/**
@verbatim

Copyright (C) 2004,2005  ADDI-DATA GmbH for the source code of this module. 
        
        ADDI-DATA GmbH 
        Dieselstrasse 3 
        D-77833 Ottersweier 
        Tel: +19(0)7223/9493-0 
        Fax: +49(0)7223/9493-92 
        http://www.addi-data-com 
        info@addi-data.com 

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

You shoud also find the complete GPL in the COPYING file accompanying this source code.

@endverbatim
*/
/*
    
  +-----------------------------------------------------------------------+
  | (C) ADDI-DATA GmbH          Dieselstraße 3       D-77833 Ottersweier  |
  +-----------------------------------------------------------------------+
  | Tel : +49 (0) 7223/9493-0     | email    : info@addi-data.com         |
  | Fax : +49 (0) 7223/9493-92    | Internet : http://www.addi-data.com   |
  +-----------------------------------------------------------------------+
  | Project     : API APCI1710    | Compiler : gcc                        |
  | Module name : TTL.C           | Version  : 2.96                       |
  +-------------------------------+---------------------------------------+
  | Project manager: Eric Stolz   | Date     :  02/12/2002                |
  +-----------------------------------------------------------------------+
  | Description :   APCI-1710 TTL I/O module                              |
  |                                                                       |
  |                                                                       |
  +-----------------------------------------------------------------------+
  |                             UPDATES                                   |
  +-----------------------------------------------------------------------+
  |   Date   |   Author  |          Description of updates                |
  +----------+-----------+------------------------------------------------+
  | 13/05/98 | S. Weber  | TTL digital input / output implementation      |
  |----------|-----------|------------------------------------------------|
  | 08/05/00 | Guinot C  | - 0400/0228 All Function in RING 0             |
  |          |           |   available                                    |
  +-----------------------------------------------------------------------+
  |          |           |                                                |
  |          |           |                                                |
  +-----------------------------------------------------------------------+
*/

/*
+----------------------------------------------------------------------------+
|                               Included files                               |
+----------------------------------------------------------------------------+
*/

#include "APCI1710_Ttl.h"

/*
+----------------------------------------------------------------------------+
| Function Name     : _INT_ i_APCI1710_InitTTLIODirection                    |
|                               (BYTE_    b_BoardHandle,                     |
|				 BYTE_    b_ModulNbr,                        |
|				 BYTE_    b_PortAMode,                       |
|				 BYTE_    b_PortBMode,                       |
|				 BYTE_    b_PortCMode,                       |
|				 BYTE_    b_PortDMode)                       |
+----------------------------------------------------------------------------+
| Task           APCI1710_TTL_INIT (using defaults)   : Configure the TTL I/O operating mode from selected     |
|                     module  (b_ModulNbr). You must calling this function be|
|                     for you call any other function witch access of TTL.   |
				 APCI1710_TTL_INITDIRECTION(user inputs for direction) 

+----------------------------------------------------------------------------+
| Input Parameters  : BYTE_ b_BoardHandle         : Handle of board APCI-1710|
|                     BYTE_ b_ModulNbr            : Module number to         |
|                                                   configure (0 to 3)  
		b_ModulNbr = (BYTE) CR_AREF(insn->chanspec);
		b_InitType = (BYTE) data[0];
		b_PortAMode	= (BYTE) data[1];
		b_PortBMode = (BYTE) data[2];
		b_PortCMode = (BYTE) data[3];
		b_PortDMode	= (BYTE) data[4];|
+----------------------------------------------------------------------------+
| Output Parameters : -                                                      |
+----------------------------------------------------------------------------+
| Return Value      : 0: No error                                            |
|                    -1: The handle parameter of the board is wrong          |
|                    -2: The module parameter is wrong                       |
|                    -3: The module is not a TTL module                      |
|		     -4: Function not available for this version             |
|		     -5: Port A mode selection is wrong                      |
|		     -6: Port B mode selection is wrong                      |
|		     -7: Port C mode selection is wrong                      |
|		     -8: Port D mode selection is wrong                      |
+----------------------------------------------------------------------------+
*/

INT i_APCI1710_InsnConfigInitTTLIO(comedi_device * dev, comedi_subdevice * s,
	comedi_insn * insn, lsampl_t * data)
{
	INT i_ReturnValue = 0;
	BYTE b_ModulNbr;
	BYTE b_InitType;
	BYTE b_PortAMode;
	BYTE b_PortBMode;
	BYTE b_PortCMode;
	BYTE b_PortDMode;

	b_ModulNbr = (BYTE) CR_AREF(insn->chanspec);
	b_InitType = (BYTE) data[0];
	i_ReturnValue = insn->n;

	/**************************/
	/* Test the module number */
	/**************************/

	if (b_ModulNbr < 4) {
	   /**************************/
		/* Test if TTL I/O module */
	   /**************************/

		if ((devpriv->s_BoardInfos.
				dw_MolduleConfiguration[b_ModulNbr] &
				0xFFFF0000UL) == APCI1710_TTL_IO) {
			switch (b_InitType) {
			case APCI1710_TTL_INIT:

				devpriv->s_ModuleInfo[b_ModulNbr].
					s_TTLIOInfo.b_TTLInit = 1;

	      /***************************/
				/* Set TTL port A to input */
	      /***************************/

				devpriv->s_ModuleInfo[b_ModulNbr].
					s_TTLIOInfo.b_PortConfiguration[0] = 0;

	      /***************************/
				/* Set TTL port B to input */
	      /***************************/

				devpriv->s_ModuleInfo[b_ModulNbr].
					s_TTLIOInfo.b_PortConfiguration[1] = 0;

	      /***************************/
				/* Set TTL port C to input */
	      /***************************/

				devpriv->s_ModuleInfo[b_ModulNbr].
					s_TTLIOInfo.b_PortConfiguration[2] = 0;

	      /****************************/
				/* Set TTL port D to output */
	      /****************************/

				devpriv->s_ModuleInfo[b_ModulNbr].
					s_TTLIOInfo.b_PortConfiguration[3] = 1;

	      /*************************/
				/* Set the configuration */
	      /*************************/

				outl(0x8,
					devpriv->s_BoardInfos.ui_Address + 20 +
					(64 * b_ModulNbr));
				break;

			case APCI1710_TTL_INITDIRECTION:

				b_PortAMode = (BYTE) data[1];
				b_PortBMode = (BYTE) data[2];
				b_PortCMode = (BYTE) data[3];
				b_PortDMode = (BYTE) data[4];

	      /********************/
				/* Test the version */
	      /********************/

				if ((devpriv->s_BoardInfos.
						dw_MolduleConfiguration
						[b_ModulNbr] & 0xFFFF) >=
					0x3230) {
		 /************************/
					/* Test the port A mode */
		 /************************/

					if ((b_PortAMode == 0)
						|| (b_PortAMode == 1)) {
		    /************************/
						/* Test the port B mode */
		    /************************/

						if ((b_PortBMode == 0)
							|| (b_PortBMode == 1)) {
		       /************************/
							/* Test the port C mode */
		       /************************/

							if ((b_PortCMode == 0)
								|| (b_PortCMode
									== 1)) {
			  /************************/
								/* Test the port D mode */
			  /************************/

								if ((b_PortDMode == 0) || (b_PortDMode == 1)) {
									devpriv->
										s_ModuleInfo
										[b_ModulNbr].
										s_TTLIOInfo.
										b_TTLInit
										=
										1;

			     /***********************/
									/* Set TTL port A mode */
			     /***********************/

									devpriv->
										s_ModuleInfo
										[b_ModulNbr].
										s_TTLIOInfo.
										b_PortConfiguration
										[0]
										=
										b_PortAMode;

			     /***********************/
									/* Set TTL port B mode */
			     /***********************/

									devpriv->
										s_ModuleInfo
										[b_ModulNbr].
										s_TTLIOInfo.
										b_PortConfiguration
										[1]
										=
										b_PortBMode;

			     /***********************/
									/* Set TTL port C mode */
			     /***********************/

									devpriv->
										s_ModuleInfo
										[b_ModulNbr].
										s_TTLIOInfo.
										b_PortConfiguration
										[2]
										=
										b_PortCMode;

			     /***********************/
									/* Set TTL port D mode */
			     /***********************/

									devpriv->
										s_ModuleInfo
										[b_ModulNbr].
										s_TTLIOInfo.
										b_PortConfiguration
										[3]
										=
										b_PortDMode;

			     /*************************/
									/* Set the configuration */
			     /*************************/

									outl((b_PortAMode << 0) | (b_PortBMode << 1) | (b_PortCMode << 2) | (b_PortDMode << 3), devpriv->s_BoardInfos.ui_Address + 20 + (64 * b_ModulNbr));
								} else {
			     /**********************************/
									/* Port D mode selection is wrong */
			     /**********************************/

									DPRINTK("Port D mode selection is wrong\n");
									i_ReturnValue
										=
										-8;
								}
							} else {
			  /**********************************/
								/* Port C mode selection is wrong */
			  /**********************************/

								DPRINTK("Port C mode selection is wrong\n");
								i_ReturnValue =
									-7;
							}
						} else {
		       /**********************************/
							/* Port B mode selection is wrong */
		       /**********************************/

							DPRINTK("Port B mode selection is wrong\n");
							i_ReturnValue = -6;
						}
					} else {
		    /**********************************/
						/* Port A mode selection is wrong */
		    /**********************************/

						DPRINTK("Port A mode selection is wrong\n");
						i_ReturnValue = -5;
					}
				} else {
		 /*******************************************/
					/* Function not available for this version */
		 /*******************************************/

					DPRINTK("Function not available for this version\n");
					i_ReturnValue = -4;
				}
				break;

				DPRINTK("\n");
			default:
				printk("Bad Config Type\n");
			}	// switch end
		} else {
	      /**********************************/
			/* The module is not a TTL module */
	      /**********************************/

			DPRINTK("The module is not a TTL module\n");
			i_ReturnValue = -3;
		}
	} else {
	   /***********************/
		/* Module number error */
	   /***********************/

		DPRINTK("Module number error\n");
		i_ReturnValue = -2;
	}

	return (i_ReturnValue);
}

/*
+----------------------------------------------------------------------------+
|                            INPUT FUNCTIONS                                 |
+----------------------------------------------------------------------------+
*/

/*
+----------------------------------------------------------------------------+
| Function Name     : _INT_   i_APCI1710_ReadTTLIOChannelValue               |
|                                       (BYTE_     b_BoardHandle,            |
|                                        BYTE_     b_ModulNbr,               |
|                                        BYTE_     b_SelectedPort,           |
|                                        BYTE_     b_InputChannel,           |
|                                        PBYTE_   pb_ChannelStatus)          |
+----------------------------------------------------------------------------+
| Task              : Read the status from selected TTL digital input        |
|                     (b_InputChannel) 
+----------------------------------------------------------------------------+
| Task              : Read the status from digital input port                |
|                     (b_SelectedPort) from selected TTL module (b_ModulNbr) |
+----------------------------------------------------------------------------+ 

+----------------------------------------------------------------------------+
| Input Parameters  : BYTE_ b_BoardHandle         : Handle of board APCI-1710|
|                     BYTE_ b_ModulNbr            : Module number to         |
|                                                   configure (0 to 7)       |
|                     BYTE_ b_SelectedPort,       : Selection from TTL I/O   |
|                                                   port (0 to 2)            |
|                                                      0 : Port A selection  |
|                                                      1 : Port B selection  |
|                                                      2 : Port C selection  |
|                                                      3 : Port D selection  |
|                     BYTE_ b_InputChannel        : Selection from digital   |
|                                                   input ( 0 to 2)        
APCI1710_TTL_READCHANNEL  
	b_ModulNbr	  = CR_AREF(insn->chanspec);
	b_SelectedPort= CR_RANGE(insn->chanspec);
	b_InputChannel= CR_CHAN(insn->chanspec);
	b_ReadType	  = (BYTE) data[0];

 APCI1710_TTL_READPORT|
	b_ModulNbr	  = CR_AREF(insn->chanspec);
	b_SelectedPort= CR_RANGE(insn->chanspec);
	b_ReadType	  = (BYTE) data[0];

+----------------------------------------------------------------------------+
| Output Parameters : data[0]

	PBYTE_  pb_ChannelStatus    : Digital input channel    |
|                                                   status                   |
|                                                   0 : Channle is not active|
|                                                   1 : Channle is active    |
+----------------------------------------------------------------------------+
| Return Value      : 0: No error                                            |
|                    -1: The handle parameter of the board is wrong          |
|                    -2: The module parameter is wrong                       |
|                    -3: The module is not a TTL module                      |
|                    -4: The selected TTL input port is wrong                |
|                    -5: The selected TTL digital input is wrong             |
|                    -6: TTL I/O not initialised                             |
+----------------------------------------------------------------------------+
*/

INT i_APCI1710_InsnBitsReadTTLIO(comedi_device * dev, comedi_subdevice * s,
	comedi_insn * insn, lsampl_t * data)
{
	INT i_ReturnValue = 0;
	DWORD dw_StatusReg;
	BYTE b_ModulNbr;
	BYTE b_SelectedPort;
	BYTE b_InputChannel;
	BYTE b_ReadType;
	PBYTE pb_ChannelStatus;
	PBYTE pb_PortValue;

	i_ReturnValue = insn->n;
	b_ReadType = (BYTE) data[0];
	b_ModulNbr = CR_AREF(insn->chanspec);
	b_SelectedPort = CR_RANGE(insn->chanspec);
	b_InputChannel = CR_CHAN(insn->chanspec);

	/**************************/
	/* Test the module number */
	/**************************/

	if (b_ModulNbr < 4) {
	   /**************************/
		/* Test if TTL I/O module */
	   /**************************/

		if ((devpriv->s_BoardInfos.
				dw_MolduleConfiguration[b_ModulNbr] &
				0xFFFF0000UL) == APCI1710_TTL_IO) {
			switch (b_ReadType) {

			case APCI1710_TTL_READCHANNEL:
				pb_ChannelStatus = (PBYTE) & data[0];
	      /********************************/
				/* Test the TTL I/O port number */
	      /********************************/

				if (((b_SelectedPort <= 2)
						&& ((devpriv->s_BoardInfos.
								dw_MolduleConfiguration
								[b_ModulNbr] &
								0xFFFF) ==
							0x3130))
					|| ((b_SelectedPort <= 3)
						&& ((devpriv->s_BoardInfos.
								dw_MolduleConfiguration
								[b_ModulNbr] &
								0xFFFF) >=
							0x3230))) {
		 /******************************************/
					/* Test the digital imnput channel number */
		 /******************************************/

					if (((b_InputChannel <= 7)
							&& (b_SelectedPort < 3))
						|| ((b_InputChannel <= 1)
							&& (b_SelectedPort ==
								3))) {
		    /******************************************/
						/* Test if the TTL I/O module initialised */
		    /******************************************/

						if (devpriv->
							s_ModuleInfo
							[b_ModulNbr].
							s_TTLIOInfo.b_TTLInit ==
							1) {
		       /***********************************/
							/* Test if TTL port used for input */
		       /***********************************/

							if (((devpriv->s_BoardInfos.dw_MolduleConfiguration[b_ModulNbr] & 0xFFFF) == 0x3130) || (((devpriv->s_BoardInfos.dw_MolduleConfiguration[b_ModulNbr] & 0xFFFF) >= 0x3230) && (devpriv->s_ModuleInfo[b_ModulNbr].s_TTLIOInfo.b_PortConfiguration[b_SelectedPort] == 0))) {
			  /**************************/
								/* Read all digital input */
			  /**************************/

								dw_StatusReg =
									inl
									(devpriv->
									s_BoardInfos.
									ui_Address
									+
									(64 * b_ModulNbr));

								*pb_ChannelStatus
									=
									(BYTE) (
									(dw_StatusReg
										>>
										(8 * b_SelectedPort)) >> b_InputChannel) & 1;
							} else {
			  /*******************************/
								/* Selected TTL I/O port error */
			  /*******************************/

								DPRINTK("Selected TTL I/O port error\n");
								i_ReturnValue =
									-4;
							}
						} else {
		       /***************************/
							/* TTL I/O not initialised */
		       /***************************/

							DPRINTK("TTL I/O not initialised\n");
							i_ReturnValue = -6;
						}
					} else {
		    /********************************/
						/* Selected digital input error */
		    /********************************/

						DPRINTK("Selected digital input error\n");
						i_ReturnValue = -5;
					}
				} else {
		 /*******************************/
					/* Selected TTL I/O port error */
		 /*******************************/

					DPRINTK("Selected TTL I/O port error\n");
					i_ReturnValue = -4;
				}
				break;

			case APCI1710_TTL_READPORT:
				pb_PortValue = (PBYTE) & data[0];
			  /********************************/
				/* Test the TTL I/O port number */
			  /********************************/

				if (((b_SelectedPort <= 2)
						&& ((devpriv->s_BoardInfos.
								dw_MolduleConfiguration
								[b_ModulNbr] &
								0xFFFF) ==
							0x3130))
					|| ((b_SelectedPort <= 3)
						&& ((devpriv->s_BoardInfos.
								dw_MolduleConfiguration
								[b_ModulNbr] &
								0xFFFF) >=
							0x3230))) {
		 /******************************************/
					/* Test if the TTL I/O module initialised */
		 /******************************************/

					if (devpriv->s_ModuleInfo[b_ModulNbr].
						s_TTLIOInfo.b_TTLInit == 1) {
		    /***********************************/
						/* Test if TTL port used for input */
		    /***********************************/

						if (((devpriv->s_BoardInfos.
									dw_MolduleConfiguration
									[b_ModulNbr]
									&
									0xFFFF)
								== 0x3130)
							|| (((devpriv->s_BoardInfos.dw_MolduleConfiguration[b_ModulNbr] & 0xFFFF) >= 0x3230) && (devpriv->s_ModuleInfo[b_ModulNbr].s_TTLIOInfo.b_PortConfiguration[b_SelectedPort] == 0))) {
		       /**************************/
							/* Read all digital input */
		       /**************************/

							dw_StatusReg =
								inl(devpriv->
								s_BoardInfos.
								ui_Address +
								(64 * b_ModulNbr));

							*pb_PortValue =
								(BYTE) (
								(dw_StatusReg >>
									(8 * b_SelectedPort)) & 0xFF);
						} else {
		       /*******************************/
							/* Selected TTL I/O port error */
		       /*******************************/

							DPRINTK("Selected TTL I/O port error\n");
							i_ReturnValue = -4;
						}
					} else {
		    /***************************/
						/* TTL I/O not initialised */
		    /***************************/

						DPRINTK("TTL I/O not initialised\n");
						i_ReturnValue = -5;
					}
				} else {
		 /*******************************/
					/* Selected TTL I/O port error */
		 /*******************************/

					DPRINTK("Selected TTL I/O port error\n");
					i_ReturnValue = -4;
				}
				break;

			default:
				printk("Bad ReadType\n");

			}	//End Switch
		} else {
	      /**********************************/
			/* The module is not a TTL module */
	      /**********************************/

			DPRINTK("The module is not a TTL module\n");
			i_ReturnValue = -3;
		}
	} else {
	   /***********************/
		/* Module number error */
	   /***********************/

		DPRINTK("Module number error\n");
		i_ReturnValue = -2;
	}

	return (i_ReturnValue);
}

/*
+----------------------------------------------------------------------------+
| Function Name     : INT i_APCI1710_InsnReadTTLIOAllPortValue(comedi_device 
*dev,comedi_subdevice *s,comedi_insn *insn,lsampl_t *data)              |
+----------------------------------------------------------------------------+
| Task              : Read the status from all digital input ports           |
|                     (port A, port B and port C) from selected TTL          |
|		      module (b_ModulNbr) 				     |
+----------------------------------------------------------------------------+
| Input Parameters  : BYTE_ b_BoardHandle         : Handle of board APCI-1710|
|                     BYTE_ b_ModulNbr            : Module number to         |
|                                                   configure (0 to 3)       |
+----------------------------------------------------------------------------+
| Output Parameters : PULONG_  pul_PortValue      : Digital TTL inputs port  |
|                                                   status                   |
+----------------------------------------------------------------------------+
| Return Value      : 0: No error                                            |
|                    -1: The handle parameter of the board is wrong          |
|                    -2: The module parameter is wrong                       |
|                    -3: The module is not a TTL module                      |
|                    -4: TTL I/O not initialised                             |
+----------------------------------------------------------------------------+
*/

INT i_APCI1710_InsnReadTTLIOAllPortValue(comedi_device * dev,
	comedi_subdevice * s, comedi_insn * insn, lsampl_t * data)
{
	INT i_ReturnValue = 0;
	DWORD dw_StatusReg;
	BYTE b_ModulNbr;
	PULONG pul_PortValue;

	b_ModulNbr = (BYTE) CR_AREF(insn->chanspec);
	i_ReturnValue = insn->n;
	pul_PortValue = (PULONG) & data[0];

	/**************************/
	/* Test the module number */
	/**************************/

	if (b_ModulNbr < 4) {
	   /**************************/
		/* Test if TTL I/O module */
	   /**************************/

		if ((devpriv->s_BoardInfos.
				dw_MolduleConfiguration[b_ModulNbr] &
				0xFFFF0000UL) == APCI1710_TTL_IO) {
	      /******************************************/
			/* Test if the TTL I/O module initialised */
	      /******************************************/

			if (devpriv->
				s_ModuleInfo[b_ModulNbr].
				s_TTLIOInfo.b_TTLInit == 1) {
		 /**************************/
				/* Read all digital input */
		 /**************************/

				dw_StatusReg = inl(devpriv->s_BoardInfos.
					ui_Address + (64 * b_ModulNbr));

		 /**********************/
				/* Test if TTL Rev1.0 */
		 /**********************/

				if ((devpriv->s_BoardInfos.
						dw_MolduleConfiguration
						[b_ModulNbr] & 0xFFFF) ==
					0x3130) {
					*pul_PortValue =
						dw_StatusReg & 0xFFFFFFUL;
				} else {
		    /**************************************/
					/* Test if port A not used for output */
		    /**************************************/

					if (devpriv->s_ModuleInfo[b_ModulNbr].
						s_TTLIOInfo.
						b_PortConfiguration[0] == 1) {
						*pul_PortValue =
							dw_StatusReg &
							0x3FFFF00UL;
					}

		    /**************************************/
					/* Test if port B not used for output */
		    /**************************************/

					if (devpriv->
						s_ModuleInfo[b_ModulNbr].
						s_TTLIOInfo.
						b_PortConfiguration[1] == 1) {
						*pul_PortValue =
							dw_StatusReg &
							0x3FF00FFUL;
					}

		    /**************************************/
					/* Test if port C not used for output */
		    /**************************************/

					if (devpriv->
						s_ModuleInfo[b_ModulNbr].
						s_TTLIOInfo.
						b_PortConfiguration[2] == 1) {
						*pul_PortValue =
							dw_StatusReg &
							0x300FFFFUL;
					}

		    /**************************************/
					/* Test if port D not used for output */
		    /**************************************/

					if (devpriv->
						s_ModuleInfo[b_ModulNbr].
						s_TTLIOInfo.
						b_PortConfiguration[3] == 1) {
						*pul_PortValue =
							dw_StatusReg &
							0xFFFFFFUL;
					}
				}
			} else {
		 /***************************/
				/* TTL I/O not initialised */
		 /***************************/
				DPRINTK("TTL I/O not initialised\n");
				i_ReturnValue = -5;
			}
		} else {
	      /**********************************/
			/* The module is not a TTL module */
	      /**********************************/
			DPRINTK("The module is not a TTL module\n");
			i_ReturnValue = -3;
		}
	} else {
	   /***********************/
		/* Module number error */
	   /***********************/
		DPRINTK("Module number error\n");
		i_ReturnValue = -2;
	}

	return (i_ReturnValue);
}

/*
+----------------------------------------------------------------------------+
|                            OUTPUT FUNCTIONS                                |
+----------------------------------------------------------------------------+
*/

/*
+----------------------------------------------------------------------------+
| Function Name     : _INT_ i_APCI1710_SetTTLIOChlOn                         |
|                               (BYTE_           b_BoardHandle,              |
|                                BYTE_           b_ModulNbr,                 |
|                                BYTE_           b_OutputChannel) 
INT i_APCI1710_InsnWriteSetTTLIOChlOnOff(comedi_device *dev,comedi_subdevice *s,
	comedi_insn *insn,lsampl_t *data)           |
+----------------------------------------------------------------------------+
| Task              : Sets or resets  the output witch has been passed with the         |
|                     parameter b_Channel. Setting an output means setting   |
|                     an ouput high.                                         |
+----------------------------------------------------------------------------+
| Input Parameters  : BYTE_ b_BoardHandle   : Handle of board APCI-1710      |
|                     BYTE_ b_ModulNbr      : Selected module number (0 to 3)|
|                     BYTE_ b_OutputChannel : Selection from digital output  |
|                                             channel (0 or 1)               |
|                                                0      : PD0                |
|                                                1      : PD1                |
|						 2 to 9 : PA                 |
|						10 to 17: PB                 |
|						18 to 25: PC                 |

  b_ModulNbr	   = CR_AREF(insn->chanspec);
	b_OutputChannel= CR_CHAN(insn->chanspec);
	ui_State	   = data[0]; // ON or OFF
+----------------------------------------------------------------------------+
| Output Parameters : -                                                      |
+----------------------------------------------------------------------------+
| Return Value      : 0: No error                                            |
|                    -1: The handle parameter of the board is wrong          |
|                    -2: The module parameter is wrong                       |
|                    -3: The module is not a TTL I/O module                  |
|                    -4: The selected digital output is wrong                |
|                    -5: TTL I/O not initialised see function                |
|                        " i_APCI1710_InitTTLIO"
+----------------------------------------------------------------------------+
*/

INT i_APCI1710_InsnWriteSetTTLIOChlOnOff(comedi_device * dev,
	comedi_subdevice * s, comedi_insn * insn, lsampl_t * data)
{
	INT i_ReturnValue = 0;
	DWORD dw_StatusReg = 0;
	BYTE b_ModulNbr;
	BYTE b_OutputChannel;
	UINT ui_State;

	i_ReturnValue = insn->n;
	b_ModulNbr = CR_AREF(insn->chanspec);
	b_OutputChannel = CR_CHAN(insn->chanspec);
	ui_State = data[0];	// ON or OFF

	/**************************/
	/* Test the module number */
	/**************************/

	if (b_ModulNbr < 4) {
	   /**************************/
		/* Test if TTL I/O module */
	   /**************************/

		if ((devpriv->s_BoardInfos.
				dw_MolduleConfiguration[b_ModulNbr] &
				0xFFFF0000UL) == APCI1710_TTL_IO) {
	      /******************************************/
			/* Test if the TTL I/O module initialised */
	      /******************************************/

			if (devpriv->s_ModuleInfo[b_ModulNbr].
				s_TTLIOInfo.b_TTLInit == 1) {
		 /***********************************/
				/* Test the TTL I/O channel number */
		 /***********************************/

				if (((b_OutputChannel <= 1)
						&& ((devpriv->s_BoardInfos.
								dw_MolduleConfiguration
								[b_ModulNbr] &
								0xFFFF) ==
							0x3130))
					|| ((b_OutputChannel <= 25)
						&& ((devpriv->s_BoardInfos.
								dw_MolduleConfiguration
								[b_ModulNbr] &
								0xFFFF) >=
							0x3230))) {
		    /****************************************************/
					/* Test if the selected channel is a output channel */
		    /****************************************************/

					if (((b_OutputChannel <= 1)
							&& (devpriv->
								s_ModuleInfo
								[b_ModulNbr].
								s_TTLIOInfo.
								b_PortConfiguration
								[3] == 1))
						|| ((b_OutputChannel >= 2)
							&& (b_OutputChannel <=
								9)
							&& (devpriv->
								s_ModuleInfo
								[b_ModulNbr].
								s_TTLIOInfo.
								b_PortConfiguration
								[0] == 1))
						|| ((b_OutputChannel >= 10)
							&& (b_OutputChannel <=
								17)
							&& (devpriv->
								s_ModuleInfo
								[b_ModulNbr].
								s_TTLIOInfo.
								b_PortConfiguration
								[1] == 1))
						|| ((b_OutputChannel >= 18)
							&& (b_OutputChannel <=
								25)
							&& (devpriv->
								s_ModuleInfo
								[b_ModulNbr].
								s_TTLIOInfo.
								b_PortConfiguration
								[2] == 1))) {
		       /************************/
						/* Test if PD0 selected */
		       /************************/

						if (b_OutputChannel == 0) {

							outl(ui_State,
								devpriv->
								s_BoardInfos.
								ui_Address +
								(64 * b_ModulNbr));
						} else {
			  /************************/
							/* Test if PD1 selected */
			  /************************/

							if (b_OutputChannel ==
								1) {

								outl(ui_State,
									devpriv->
									s_BoardInfos.
									ui_Address
									+ 4 +
									(64 * b_ModulNbr));
							} else {
								b_OutputChannel
									=
									b_OutputChannel
									- 2;

			     /********************/
								/* Read all channel */
			     /********************/

								dw_StatusReg =
									inl
									(devpriv->
									s_BoardInfos.
									ui_Address
									+
									(64 * b_ModulNbr));
								if (ui_State)	// ON
								{
									dw_StatusReg
										=
										(dw_StatusReg
										>>
										((b_OutputChannel / 8) * 8)) & 0xFF;
									dw_StatusReg
										=
										dw_StatusReg
										|
										(1
										<<
										(b_OutputChannel
											%
											8));
								} else	// Off
								{
									dw_StatusReg
										=
										(dw_StatusReg
										>>
										((b_OutputChannel / 8) * 8)) & 0xFF;
									dw_StatusReg
										=
										dw_StatusReg
										&
										(0xFF
										-
										(1 << (b_OutputChannel % 8)));

								}

			     /****************************/
								/* Set the new output value */
			     /****************************/

								outl(dw_StatusReg, devpriv->s_BoardInfos.ui_Address + 8 + ((b_OutputChannel / 8) * 4) + (64 * b_ModulNbr));
							}
						}
					} else {
		       /************************************/
						/* The selected TTL output is wrong */
		       /************************************/

						DPRINTK(" The selected TTL output is wrong\n");
						i_ReturnValue = -4;
					}
				} else {
		    /************************************/
					/* The selected TTL output is wrong */
		    /************************************/

					DPRINTK("The selected TTL output is wrong\n");
					i_ReturnValue = -4;
				}
			} else {
		 /***************************/
				/* TTL I/O not initialised */
		 /***************************/

				DPRINTK("TTL I/O not initialised\n");
				i_ReturnValue = -5;
			}
		} else {
	      /**************************************/
			/* The module is not a TTL I/O module */
	      /**************************************/

			DPRINTK("The module is not a TTL I/O module\n");
			i_ReturnValue = -3;
		}
	} else {
	   /***********************/
		/* Module number error */
	   /***********************/

		DPRINTK("Module number error\n");
		i_ReturnValue = -2;
	}

	return (i_ReturnValue);
}
