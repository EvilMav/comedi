


#define APCI1710_30MHZ           30
#define APCI1710_33MHZ           33
#define APCI1710_40MHZ           40


#define APCI1710_SINGLE     0
#define APCI1710_CONTINUOUS 1


#define APCI1710_CHRONO_PROGRESS_STATUS		0
#define APCI1710_CHRONO_READVALUE			1
#define APCI1710_CHRONO_CONVERTVALUE		2
#define APCI1710_CHRONO_READINTERRUPT       3

#define APCI1710_CHRONO_SET_CHANNELON       		0
#define APCI1710_CHRONO_SET_CHANNELOFF		1
#define APCI1710_CHRONO_READ_CHANNEL		2
#define APCI1710_CHRONO_READ_PORT			3

/*
+----------------------------------------------------------------------------+
|                  CHRONOMETER INISIALISATION FUNCTION   |
+----------------------------------------------------------------------------+

INT i_APCI1710_InsnConfigInitChrono(comedi_device *dev,comedi_subdevice *s,
	comedi_insn *insn,lsampl_t *data);

INT i_APCI1710_InsnWriteEnableDisableChrono(comedi_device *dev,comedi_subdevice *s,
	comedi_insn *insn,lsampl_t *data);


/*
+----------------------------------------------------------------------------+
|                       CHRONOMETER READ FUNCTION                            |
+----------------------------------------------------------------------------+
*/

INT	i_APCI1710_InsnReadChrono(comedi_device *dev,comedi_subdevice *s,
comedi_insn *insn,lsampl_t *data);


INT   i_APCI1710_GetChronoProgressStatus      (comedi_device *dev,
						 BYTE    b_ModulNbr,
						 PBYTE  pb_ChronoStatus);

INT   i_APCI1710_ReadChronoValue      (comedi_device *dev,
					 BYTE     b_ModulNbr,
					 UINT    ui_TimeOut,
					 PBYTE   pb_ChronoStatus,
					 PULONG pul_ChronoValue);

INT i_APCI1710_ConvertChronoValue     (comedi_device *dev,
					 BYTE     b_ModulNbr,
					 ULONG   ul_ChronoValue,
					 PULONG pul_Hour,
					 PBYTE   pb_Minute,
					 PBYTE   pb_Second,
					 PUINT  pui_MilliSecond,
					 PUINT  pui_MicroSecond,
					 PUINT  pui_NanoSecond);





/*
+----------------------------------------------------------------------------+
|                       CHRONOMETER DIGITAL INPUT OUTPUT FUNCTION                  |
+----------------------------------------------------------------------------+
*/

INT i_APCI1710_InsnBitsChronoDigitalIO(comedi_device *dev,comedi_subdevice *s,
	comedi_insn *insn,lsampl_t *data);

