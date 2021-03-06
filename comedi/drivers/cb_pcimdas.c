/*
    comedi/drivers/cb_pcimdas.c
    Comedi driver for Computer Boards PCIM-DAS1602/16 & PCIe-DAS1602/16

    COMEDI - Linux Control and Measurement Device Interface
    Copyright (C) 2000 David A. Schleef <ds@schleef.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/
/*
Driver: cb_pcimdas
Description: Measurement Computing PCI Migration series boards
Devices: [ComputerBoards] PCIM-DAS1602/16 (cb_pcimdas), PCIe-DAS1602/16
Author: Richard Bytheway
Updated: Mon, 13 Oct 2014 11:17:08 +0000
Status: experimental

Written to support the PCIM-DAS1602/16 and PCIe-DAS1602/16.

Configuration Options:
    [0] - PCI bus number
    [1] - PCI slot number

Developed from cb_pcidas and skel by Richard Bytheway (mocelet@sucs.org).
Only supports DIO, AO and simple AI in it's present form.
No interrupts, multi channel or FIFO AI, although the card looks like it could
support this.

http://www.mccdaq.com/PDFs/Manuals/pcim-das1602-16.pdf
http://www.mccdaq.com/PDFs/Manuals/pcie-das1602-16.pdf
*/

#include <linux/comedidev.h>

#include <linux/delay.h>

#include "comedi_pci.h"
#include "plx9052.h"
#include "8255.h"

//#define CBPCIMDAS_DEBUG
#undef CBPCIMDAS_DEBUG

/* Registers for the PCIM-DAS1602/16 and PCIe-DAS1602/16 */

// sizes of io regions (bytes)
#define BADR0_SIZE 2		//??
#define BADR1_SIZE 4
#define BADR2_SIZE 6
#define BADR3_SIZE 16
#define BADR4_SIZE 4

//DAC Offsets
#define ADC_TRIG 0
#define DAC0_OFFSET 2
#define DAC1_OFFSET 4

//AI and Counter Constants
#define MUX_LIMITS 0
#define MAIN_CONN_DIO 1
#define ADC_STAT 2
#define ADC_CONV_STAT 3
#define ADC_INT 4
#define ADC_PACER 5
#define BURST_MODE 6
#define PROG_GAIN 7
#define CLK8254_1_DATA 8
#define CLK8254_2_DATA 9
#define CLK8254_3_DATA 10
#define CLK8254_CONTROL 11
#define USER_COUNTER 12
#define RESID_COUNT_H 13
#define RESID_COUNT_L 14

static const comedi_lrange cb_pcimdas_ai_bip_range = {
	4, {
		BIP_RANGE(10),
		BIP_RANGE(5),
		BIP_RANGE(2.5),
		BIP_RANGE(1.25)
	}
};

static const comedi_lrange cb_pcimdas_ai_uni_range = {
	4, {
		UNI_RANGE(10),
		UNI_RANGE(5),
		UNI_RANGE(2.5),
		UNI_RANGE(1.25)
	}
};

/* Board description */
typedef struct cb_pcimdas_board_struct {
	const char *name;
	unsigned short device_id;
	int ai_se_chans;	// Inputs in single-ended mode
	int ai_diff_chans;	// Inputs in differential mode
	int ai_bits;		// analog input resolution
	int ai_speed;		// fastest conversion period in ns
	int ao_nchan;		// number of analog out channels
	int ao_bits;		// analogue output resolution
	int has_ao_fifo;	// analog output has fifo
	int ao_scan_speed;	// analog output speed for 1602 series (for a scan, not conversion)
	int fifo_size;		// number of samples fifo can hold
	int dio_bits;		// number of dio bits
	int has_dio;		// has DIO
	const comedi_lrange *ranges;
} cb_pcimdas_board;

static const cb_pcimdas_board cb_pcimdas_boards[] = {
	{
	      name:	"PCIM-DAS1602/16",
	      device_id:0x56,
	      ai_se_chans:16,
	      ai_diff_chans:8,
	      ai_bits:	16,
	      ai_speed:10000,	//??
	      ao_nchan:2,
	      ao_bits:	12,
	      has_ao_fifo:0,	//??
	      ao_scan_speed:10000,
			//??
	      fifo_size:1024,
	      dio_bits:24,
	      has_dio:	1,
//              ranges:         &cb_pcimdas_ranges,
	},
	{
	      name:	"PCIe-DAS1602/16",
	      device_id:0x115,
	      ai_se_chans:16,
	      ai_diff_chans:8,
	      ai_bits:	16,
	      ai_speed:10000,	//??
	      ao_nchan:2,
	      ao_bits:	12,
	      has_ao_fifo:0,	//??
	      ao_scan_speed:10000,
			//??
	      fifo_size:1024,
	      dio_bits:24,
	      has_dio:	1,
//              ranges:         &cb_pcimdas_ranges,
	},
};

/* This is used by modprobe to translate PCI IDs to drivers.  Should
 * only be used for PCI and ISA-PnP devices */
static DEFINE_PCI_DEVICE_TABLE(cb_pcimdas_pci_table) = {
	{PCI_VENDOR_ID_COMPUTERBOARDS, 0x0056, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{PCI_VENDOR_ID_COMPUTERBOARDS, 0x0115, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{0}
};

MODULE_DEVICE_TABLE(pci, cb_pcimdas_pci_table);


/*
 * Useful for shorthand access to the particular board structure
 */
#define thisboard ((const cb_pcimdas_board *)dev->board_ptr)

/* this structure is for data unique to this hardware driver.  If
   several hardware drivers keep similar information in this structure,
   feel free to suggest moving the variable to the comedi_device struct.  */
typedef struct {
	int data;

	// would be useful for a PCI device
	struct pci_dev *pci_dev;

	//base addresses
	unsigned long BADR0;
	unsigned long BADR1;
	unsigned long BADR2;
	unsigned long BADR3;
	unsigned long BADR4;

	/* Used for AO readback */
	lsampl_t ao_readback[2];

	// Used for DIO
	unsigned short int port_a;	// copy of BADR4+0
	unsigned short int port_b;	// copy of BADR4+1
	unsigned short int port_c;	// copy of BADR4+2
	unsigned short int dio_mode;	// copy of BADR4+3

} cb_pcimdas_private;

/*
 * most drivers define the following macro to make it easy to
 * access the private structure.
 */
#define devpriv ((cb_pcimdas_private *)dev->private)

/*
 * The comedi_driver structure tells the Comedi core module
 * which functions to call to configure/deconfigure (attach/detach)
 * the board, and also about the kernel module that contains
 * the device code.
 */
static int cb_pcimdas_attach(comedi_device * dev, comedi_devconfig * it);
static int cb_pcimdas_detach(comedi_device * dev);
static comedi_driver driver_cb_pcimdas = {
      driver_name:"cb_pcimdas",
      module:THIS_MODULE,
      attach:cb_pcimdas_attach,
      detach:cb_pcimdas_detach,
};

static int cb_pcimdas_ai_rinsn(comedi_device * dev, comedi_subdevice * s,
	comedi_insn * insn, lsampl_t * data);
static int cb_pcimdas_ao_winsn(comedi_device * dev, comedi_subdevice * s,
	comedi_insn * insn, lsampl_t * data);
static int cb_pcimdas_ao_rinsn(comedi_device * dev, comedi_subdevice * s,
	comedi_insn * insn, lsampl_t * data);

/*
 * Attach is called by the Comedi core to configure the driver
 * for a particular board.  If you specified a board_name array
 * in the driver structure, dev->board_ptr contains that
 * address.
 */
static int cb_pcimdas_attach(comedi_device * dev, comedi_devconfig * it)
{
	comedi_subdevice *s;
	struct pci_dev *pcidev;
	int index;
	unsigned int aichanstat;
	//int i;

	printk("comedi%d: cb_pcimdas: ", dev->minor);

/*
 * Allocate the private structure area.
 */
	if (alloc_private(dev, sizeof(cb_pcimdas_private)) < 0)
		return -ENOMEM;

/*
 * Probe the device to determine what device in the series it is.
 */
	printk("\n");

	for (pcidev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, NULL);
		pcidev != NULL;
		pcidev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, pcidev)) {
		// is it not a computer boards card?
		if (pcidev->vendor != PCI_VENDOR_ID_COMPUTERBOARDS)
			continue;
		// loop through cards supported by this driver
		for (index = 0; index < ARRAY_SIZE(cb_pcimdas_boards); index++) {
			if (cb_pcimdas_boards[index].device_id !=
				pcidev->device)
				continue;
			// was a particular bus/slot requested?
			if (it->options[0] || it->options[1]) {
				// are we on the wrong bus/slot?
				if (pcidev->bus->number != it->options[0] ||
					PCI_SLOT(pcidev->devfn) !=
					it->options[1]) {
					continue;
				}
			}
			devpriv->pci_dev = pcidev;
			dev->board_ptr = cb_pcimdas_boards + index;
			goto found;
		}
	}

	printk("No supported ComputerBoards/MeasurementComputing card found on "
		"requested position\n");
	return -EIO;

      found:

	printk("Found %s on bus %i, slot %i\n", cb_pcimdas_boards[index].name,
		pcidev->bus->number, PCI_SLOT(pcidev->devfn));

	if (comedi_pci_enable(pcidev, "cb_pcimdas")) {
		printk(" Failed to enable PCI device and request regions\n");
		return -EIO;
	}

	devpriv->BADR0 = pci_resource_start(devpriv->pci_dev, 0);
	devpriv->BADR1 = pci_resource_start(devpriv->pci_dev, 1);
	devpriv->BADR2 = pci_resource_start(devpriv->pci_dev, 2);
	devpriv->BADR3 = pci_resource_start(devpriv->pci_dev, 3);
	devpriv->BADR4 = pci_resource_start(devpriv->pci_dev, 4);

#ifdef CBPCIMDAS_DEBUG
	printk("devpriv->BADR0 = 0x%lx\n", devpriv->BADR0);
	printk("devpriv->BADR1 = 0x%lx\n", devpriv->BADR1);
	printk("devpriv->BADR2 = 0x%lx\n", devpriv->BADR2);
	printk("devpriv->BADR3 = 0x%lx\n", devpriv->BADR3);
	printk("devpriv->BADR4 = 0x%lx\n", devpriv->BADR4);
#endif

// Dont support IRQ yet
//      // get irq
//      if(comedi_request_irq(devpriv->pci_dev->irq, cb_pcimdas_interrupt, IRQF_SHARED, "cb_pcimdas", dev ))
//      {
//              printk(" unable to allocate irq %u\n", devpriv->pci_dev->irq);
//              return -EINVAL;
//      }
//      dev->irq = devpriv->pci_dev->irq;

	//Initialize dev->board_name
	dev->board_name = thisboard->name;

/*
 * Allocate the subdevice structures.  alloc_subdevice() is a
 * convenient macro defined in comedidev.h.
 */
	if (alloc_subdevices(dev, 3) < 0)
		return -ENOMEM;

	s = dev->subdevices + 0;
	//dev->read_subdev=s;
	// analog input subdevice
	s->type = COMEDI_SUBD_AI;
	/* check status of hardware switches */
	aichanstat = inb(devpriv->BADR3 + 2);
	if (aichanstat & 0x20) {
		/* single-ended mode */
		s->subdev_flags = SDF_READABLE | SDF_GROUND;
		s->n_chan = thisboard->ai_se_chans;
	} else {
		/* differential mode */
		s->subdev_flags = SDF_READABLE | SDF_DIFF;
		s->n_chan = thisboard->ai_diff_chans;
	}
	s->maxdata = (1 << thisboard->ai_bits) - 1;
	/* supported ranges depend on state of Bip/Uni switch */
	if (aichanstat & 0x40)
		s->range_table = &cb_pcimdas_ai_uni_range;
	else
		s->range_table = &cb_pcimdas_ai_bip_range;
	s->len_chanlist = 1;	// This is the maximum chanlist length that
	// the board can handle
	s->insn_read = cb_pcimdas_ai_rinsn;

	s = dev->subdevices + 1;
	// analog output subdevice
	s->type = COMEDI_SUBD_AO;
	s->subdev_flags = SDF_WRITABLE;
	s->n_chan = thisboard->ao_nchan;
	s->maxdata = 1 << thisboard->ao_bits;
	s->range_table = &range_unknown;	//ranges are hardware settable, but not software readable.
	s->insn_write = &cb_pcimdas_ao_winsn;
	s->insn_read = &cb_pcimdas_ao_rinsn;

	s = dev->subdevices + 2;
	/* digital i/o subdevice */
	if (thisboard->has_dio) {
		subdev_8255_init(dev, s, NULL, devpriv->BADR4);
	} else {
		s->type = COMEDI_SUBD_UNUSED;
	}

	printk("attached\n");

	return 1;
}

/*
 * _detach is called to deconfigure a device.  It should deallocate
 * resources.
 * This function is also called when _attach() fails, so it should be
 * careful not to release resources that were not necessarily
 * allocated by _attach().  dev->private and dev->subdevices are
 * deallocated automatically by the core.
 */
static int cb_pcimdas_detach(comedi_device * dev)
{
#ifdef CBPCIMDAS_DEBUG
	if (devpriv) {
		printk("devpriv->BADR0 = 0x%lx\n", devpriv->BADR0);
		printk("devpriv->BADR1 = 0x%lx\n", devpriv->BADR1);
		printk("devpriv->BADR2 = 0x%lx\n", devpriv->BADR2);
		printk("devpriv->BADR3 = 0x%lx\n", devpriv->BADR3);
		printk("devpriv->BADR4 = 0x%lx\n", devpriv->BADR4);
	}
#endif
	printk("comedi%d: cb_pcimdas: remove\n", dev->minor);
	if (dev->irq)
		comedi_free_irq(dev->irq, dev);
	if (devpriv) {
		if (devpriv->pci_dev) {
			if (devpriv->BADR0) {
				comedi_pci_disable(devpriv->pci_dev);
			}
			pci_dev_put(devpriv->pci_dev);
		}
	}

	return 0;
}

/*
 * "instructions" read/write data in "one-shot" or "software-triggered"
 * mode.
 */
static int cb_pcimdas_ai_rinsn(comedi_device * dev, comedi_subdevice * s,
	comedi_insn * insn, lsampl_t * data)
{
	int n, i;
	unsigned int d;
	unsigned int busy;
	unsigned int chan = CR_CHAN(insn->chanspec);
	unsigned int range = CR_RANGE(insn->chanspec);
	unsigned short chanlims;

	// only support sw initiated reads from a single channel

	//configure for sw initiated read
	d = inb(devpriv->BADR3 + 5);
	if ((d & 0x03) > 0) {	//only reset if needed.
		d = d & 0xfd;
		outb(d, devpriv->BADR3 + 5);
	}
	outb(0x01, devpriv->BADR3 + 6);	//set bursting off, conversions on
	outb(range, devpriv->BADR3 + 7);	//set range

	// write channel limits to multiplexer, set Low (bits 0-3) and High (bits 4-7) channels to chan.
	chanlims = chan | (chan << 4);
	outb(chanlims, devpriv->BADR3 + 0);

	/* convert n samples */
	for (n = 0; n < insn->n; n++) {
		/* trigger conversion */
		outw(0, devpriv->BADR2 + 0);

#define TIMEOUT 1000		//typically takes 5 loops on a lightly loaded Pentium 100MHz,
		//this is likely to be 100 loops on a 2GHz machine, so set 1000 as the limit.

		/* wait for conversion to end */
		for (i = 0; i < TIMEOUT; i++) {
			busy = inb(devpriv->BADR3 + 2) & 0x80;
			if (!busy)
				break;
		}
		if (i == TIMEOUT) {
			printk("timeout\n");
			return -ETIMEDOUT;
		}
		/* read data */
		d = inw(devpriv->BADR2 + 0);

		/* mangle the data as necessary */
		//d ^= 1<<(thisboard->ai_bits-1); // 16 bit data from ADC, so no mangle needed.

		data[n] = d;
	}

	/* return the number of samples read/written */
	return n;
}

static int cb_pcimdas_ao_winsn(comedi_device * dev, comedi_subdevice * s,
	comedi_insn * insn, lsampl_t * data)
{
	int i;
	int chan = CR_CHAN(insn->chanspec);

	/* Writing a list of values to an AO channel is probably not
	 * very useful, but that's how the interface is defined. */
	for (i = 0; i < insn->n; i++) {
		switch (chan) {
		case 0:
			outw(data[i] & 0x0FFF, devpriv->BADR2 + DAC0_OFFSET);
			break;
		case 1:
			outw(data[i] & 0x0FFF, devpriv->BADR2 + DAC1_OFFSET);
			break;
		default:
			return -1;
		}
		devpriv->ao_readback[chan] = data[i];
	}

	/* return the number of samples read/written */
	return i;
}

/* AO subdevices should have a read insn as well as a write insn.
 * Usually this means copying a value stored in devpriv. */
static int cb_pcimdas_ao_rinsn(comedi_device * dev, comedi_subdevice * s,
	comedi_insn * insn, lsampl_t * data)
{
	int i;
	int chan = CR_CHAN(insn->chanspec);

	for (i = 0; i < insn->n; i++)
		data[i] = devpriv->ao_readback[chan];

	return i;
}

/*
 * A convenient macro that defines init_module() and cleanup_module(),
 * as necessary.
 */
COMEDI_PCI_INITCLEANUP(driver_cb_pcimdas, cb_pcimdas_pci_table);
