




   #define APCI1710_30MHZ           30
   #define APCI1710_33MHZ           33
   #define APCI1710_40MHZ           40


#define APCI1710_PWM_INIT			0
#define APCI1710_PWM_GETINITDATA	1

#define APCI1710_PWM_DISABLE        0
#define APCI1710_PWM_ENABLE			1
#define APCI1710_PWM_NEWTIMING      2 




INT	i_APCI1710_InsnConfigPWM(comedi_device *dev,comedi_subdevice *s,
comedi_insn *insn,lsampl_t *data);

INT   i_APCI1710_InitPWM      (comedi_device *dev,
				 BYTE     b_ModulNbr,
				 BYTE    b_PWM,
				 BYTE     b_ClockSelection,
				 BYTE     b_TimingUnit,
				 ULONG   ul_LowTiming,
				 ULONG   ul_HighTiming,
				 PULONG pul_RealLowTiming,
				 PULONG pul_RealHighTiming);

INT  i_APCI1710_GetPWMInitialisation (comedi_device *dev,
					 BYTE     b_ModulNbr,
					 BYTE     b_PWM,
					 PBYTE   pb_TimingUnit,
					 PULONG pul_LowTiming,
					 PULONG pul_HighTiming,
					 PBYTE   pb_StartLevel,
					 PBYTE   pb_StopMode,
					 PBYTE   pb_StopLevel,
					 PBYTE   pb_ExternGate,
					 PBYTE   pb_InterruptEnable,
					 PBYTE   pb_Enable);




INT	i_APCI1710_InsnWritePWM(comedi_device *dev,comedi_subdevice *s,
comedi_insn *insn,lsampl_t *data);

INT i_APCI1710_EnablePWM    (comedi_device *dev,
				 BYTE  b_ModulNbr,
				 BYTE  b_PWM,
				 BYTE  b_StartLevel,
				 BYTE  b_StopMode,
				 BYTE  b_StopLevel,
				 BYTE  b_ExternGate,
				 BYTE  b_InterruptEnable);

INT i_APCI1710_SetNewPWMTiming	(comedi_device *dev,
					 BYTE     b_ModulNbr,
					 BYTE     b_PWM,
					 BYTE     b_TimingUnit,
					 ULONG   ul_LowTiming,
					 ULONG   ul_HighTiming);

INT i_APCI1710_DisablePWM   (comedi_device *dev,
				 BYTE  b_ModulNbr,
				 BYTE  b_PWM);

INT i_APCI1710_InsnReadGetPWMStatus(comedi_device *dev,comedi_subdevice *s,
	comedi_insn *insn,lsampl_t *data);

INT i_APCI1710_InsnBitsReadPWMInterrupt(comedi_device *dev,comedi_subdevice *s,
	comedi_insn *insn,lsampl_t *data);
