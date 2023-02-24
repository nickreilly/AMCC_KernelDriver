/*
*/

__asm("\nCRA EQU $FFFFB5") ; /* ESSI0 Control Register A*/
__asm("\nCRB EQU $FFFFB6") ; /* ESSI0 Control Register B*/
__asm("\nSSISR EQU $FFFFB7") ; /* Sync Serial Interface Status Register */
__asm("\nRXD EQU $FFFFB8") ; /* Received data register */
__asm("\nPCRC EQU $FFFFBF") ; /* Port C (ESSI0) Control Register */
__asm("\nHCR EQU $FFFFC2") ; /* Host Control Register. 3 int enab, 2 iface status */
__asm("\nHPCR EQU $FFFFC4") ; /* Host Port Control Register */
__asm("\nHDDR EQU $FFFFC8") ; /* Host Port Data Direction Register */
__asm("\nHDR EQU $FFFFC9") ; /* Host Port Data Register */

__asm("\nPCTL EQU $FFFFFD") ; /* Phase lock loop Control Register */
__asm("\nBCR EQU $FFFFFB") ; /* Bus Control Register */
__asm("\nAAR0 EQU $FFFFF9") ; /* Address Attribute Register 0 RAM */
__asm("\nAAR1 EQU $FFFFF8") ; /* Address Attribute Register 1 FLASH */
__asm("\nAAR2 EQU $FFFFF7") ; /* Address Attribute Register 2  */
__asm("\nAAR3 EQU $FFFFF6") ; /* Address Attribute Register 3 IO */
__asm("\nIPRP EQU $FFFFFE") ; /* Address Attribute Register 3 IO */
__asm("\nPCRD 	EQU $FFFFAF"); /* Port D (ESSI_1) control register */
__asm("\nPRRD	EQU	$FFFFAE"); /* Port D (ESSI_1) direction */
__asm("\nPDRD	EQU	$FFFFAD"); /* Port D (ESSI_1) data */
/* Nov 9 2003: grab SCI (PORT E) bits as GPIO for stepper control */
/* Only 3 bits on port E.  bit0 = spare, bit1 = step, bit2 = dir */
__asm("\nPCRE 	EQU $FFFF9F"); /* control register, should bt 0 for GPIO */
__asm("\nPRRE	EQU	$FFFF9E"); /* direction bits. 1 = output */
__asm("\nPDRE	EQU	$FFFF9D"); /* Port E (SCI) data */
/*
;; Burley board registers, occupy Y memory.
*/


#ifdef MHZ80
__asm("\n\
ClockRate      EQU     80000000 \n\
");
#else
__asm("\n\
ClockRate      EQU     50000000 \n\
");
#endif

__asm("\n\
SeqPort   EQU       $FFFF88     ; all the bits at once
OrPort    EQU       $FFFF89     ; just set certain bits
NandPort  EQU       $FFFF8A     ; just clear certain bits
ToggPort  EQU       $FFFF8B     ; just toggle certain bits.
StatPort  EQU       $FFFF8C     ; status signals 2,3 \n\
DetPort   EQU       $FFFF8D     ; frame control signals, 0,1,4,5,12,13 \n\
RowPort   EQU       $FFFF8E     ; row clocks 9,10,11,14,15,16,17 \n\
ColPort   EQU       $FFFF8F     ; column clocks 6,7,8,18,19 \n\

Adc0      EQU       $FFFF90     ; Adc \n\
Adc1      EQU       $FFFF91     ; Adc \n\
HssTx EQU $FFFFD0  ;  High Speed Serial Transmit register \n\
HssTx2 EQU $FFFFD1 ;  High Speed Serial Transmit register \n\
		; 	\n\
ADC      EQU     $000001 ; (falling edge triggers ADC) Det port \n\
Frame    EQU     $000002 ; (High during reads) Det port \n\
Seq2     EQU     $000004 ; only 2 channels. No Pixmux. Stat port.
Shutter  EQU     $000008 ; shutter open / close. Stat Port\n\
Seq4     EQU     $000010 ; Det port - VddUc and VSSuc simultaneously. \n\
Seq5     EQU     $000020 ; Det port - Vpd and Vnd simulateously \n\
LSyncB   EQU     $000040 ; Line sync. Seq6 (L) Col port \n\
HClk     EQU     $000080 ; Horizontal clock. Seq7 (L) Col port \n\
MResetB  EQU     $000100 ; Main reset. Seq8 (active L) Col port \n\
                        ; pulse it low to reset serial regs to default.
VClk     EQU     $000200 ; Vertical clock. Seq9 () Row port \n\
ResetEn  EQU     $000400 ; Seq10 active hi, reset on. Row port \n\
ReadEn   EQU     $000800 ; Seq11 active hi, pix to buses. Row port \n\
Seq12    EQU	 $001000 ; Seq12 (unused) Det port IIdle and Islew  \n\
Seq13    EQU	 $002000 ; Seq13 (unused) Det port VdetCom and Vddout. \n\
FSyncB	 EQU     $004000 ; Seq14 Frame Sync. Row port \n\
Seq15	 EQU     $008000 ; Seq15 Vdd Row port \n\
CSB	 EQU     $010000 ; Seq16 CSB Row port \n\
Seq17	 EQU     $020000 ; select all rows Row port \n\
Seq18    EQU     $040000 ; Vneg protection rail. Col port?? \n\
Seq19    EQU     $080000 ; Vload Col port \n\
FrmOff   EQU     $000000 ; \n\
Off	 EQU	 $FFFFFF ; \n\
DetOff	 EQU	 $000000 ; \n\
RowOff	 EQU	 FSyncB+CSB+ReadEn ; \n\
ColOff	 EQU	 MResetB+LSyncB ; \n\
NanoSec	       EQU     0.000000001 \n\
MicroSec       EQU     0.000001 \n\
MilliSec       EQU     0.001 \n\
TenthSec       EQU     0.1 \n\
Sec            EQU     1.0 \n\
RSTcount EQU   @cvi(ClockRate*200*MicroSec) \n\
RowSyncCount EQU	@cvi(ClockRate*2.0*MicroSec) \n\
RPhiUnderlap EQU	@cvi(ClockRate*2000*NanoSec-8) \n\
RPhiRUnderlap EQU	@cvi(ClockRate*5*MicroSec-8) \n\
ColSyncCount EQU	@cvi(ClockRate*2.0*MicroSec) \n\
CPhiUnderlap EQU	@cvi(ClockRate*500*NanoSec-8) \n\
Clamp2Samp EQU	@cvi(ClockRate*5000*NanoSec-8) \n\
ADCon     EQU       Frame+ADC ;  \n\
ADCoff    EQU       Frame ;  \n\
ShutterOpen  EQU      Shutter ;  \n\
ShutterClose  EQU     $0 ;  \n\
");

/*
 * xxxB means xxx is active low (B="BAR")
 * FSYNCB is active low Frame sync. Syncs vertical shift register.
 * VCLK once per row. normally low. can overclock for framechk.
 *   vclk is synchronous, falling edge is active. advances vert shift.
 * LSYNCB is active low Line sync.. once per row, possibly sync'd with VCLK
 *   lsyncb is asynchronous, and syncs the horizontal (line) shift register.
 * HCLK, falling edge is active. Advances the horizontal shift register.
 * HRESETB-- alternate line sync, not required.
 * READEN- connect the pixels to the vertical read buses.
 * RESETEN - reset by row is default. High starts the reset. low stops 
 * MainResetBar.. pulse low to reset digital registers. can be grounded!
 * 
*/


/*
;; Timer control variables.
*/


__asm("\n\
TCSR0      EQU       $FFFF8F     ; Timer Control/Status Register \n\
TLR0       EQU       $FFFF8E     ; Timer Load Register \n\
TCPR0      EQU       $FFFF8D     ; Timer Compare Register \n\
TCR0       EQU       $FFFF8C     ; Timer Count Register \n\

TCSR1 	   EQU	     $FFFF8B     ; Timer Control/Status Register \n\
TLR1       EQU       $FFFF8A     ; Timer Load Register \n\
TCPR1      EQU       $FFFF89     ; Timer Compare Register \n\
TCR1       EQU       $FFFF88     ; Timer Count Register \n\

TCSR2      EQU       $FFFF87     ; Timer Control/Status Register \n\
TLR2       EQU       $FFFF86     ; Timer Load Register \n\
TCPR2      EQU       $FFFF85     ; Timer Compare Register \n\
TCR2       EQU       $FFFF84     ; Timer Count Register \n\

TPLR       EQU       $FFFF83     ; Timer Prescaler Load Register \n\
TPCR       EQU       $FFFF82     ; Timer Prescaler Count Register \n\
");

/*
 */
/*
* The Address Attribute Registers are documented in the 56300 Family Manual
* -- section 9.6 (port A control)
* The chip has 4 address strobes that one can place in various
* address ranges.
* You set the top 12 bits to the MSBs of the address you want to map.
* The next 4 bits are how many of these 12 you care about (so the
* smallest chunk is 4K, if you use all 12.)
*/

void initAARs()
{
    __asm(" MOVEP #$FFFC21,X:AAR3"); /* AA3 on y memory, FFF000-FFFFFF */
    __asm(" MOVEP #$0FEC11,X:AAR2"); /* AA2 on x memory, 0FE000-0FEFFF */
    __asm(" MOVEP #$D08909,X:AAR1"); /* FLASH D00000 - D07FFF 32K*/
    __asm(" MOVEP #$000811,X:AAR0"); /* SRAM x, 000000-FFFFFF 64K */
}

/* sets number of wait states. */
void initBCR()
{
   __asm(" MOVEP #$079Fe1,X:BCR"); /* 1 periph 1 SRAM, 31 eprom */
}

/* sets frequency of main oscillator */
void initPLL40()
{
   __asm(" MOVEP #$078003,X:PCTL"); /* 4+1=5,*10 = 50 Mhz */
}

void initPLL50()
{
   __asm(" MOVEP #$078004,X:PCTL"); /* 4+1=5,*10 = 50 Mhz */
}

void initPLL60()
{
   __asm(" MOVEP #$078005,X:PCTL"); /* 5+1=6,*10 = 60 Mhz */
}

void initPLL80()
{
   __asm(" MOVEP #$078007,X:PCTL"); /* 7+1=8,*10 = 80 Mhz */
}

void initPLL90()
{
   __asm(" MOVEP #$078008,X:PCTL"); /* 8+1=9,*10 = 90 Mhz */
}

void initPLL100()
{
   __asm(" MOVEP #$078009,X:PCTL"); /* 9+1=10,*10 = 100 Mhz */
}


void initSeq()
{
	__asm(" \n\
	movep   #RowOff,Y:RowPort ; \n\
	movep   #ColOff,Y:ColPort ; \n\
	movep   #DetOff,Y:DetPort ; \n\
	movep   #Seq2,Y:StatPort ; \n\
	");
}

/* Initialize triple timer module */
/* clockperiod*2 * num = time per tick * */
void initTTimer()
{
	__asm(" MOVEP #$008000,X:TCSR0");/* timer off, use prescaler. */
#ifdef MHZ80
	__asm(" MOVEP #40000,X:TPLR");/*set the TPLR to count 1 msec */
#else
	__asm(" MOVEP #43,X:TPLR");/* set the TPLR to count 2 usec */
#endif
	__asm(" MOVEP #$008001,X:TCSR0");/* timer on, use prescaler. */
}

void clearTimer()
{
	__asm(" MOVEP #$008000,X:TCSR0");/* timer off and back on. */
	__asm(" MOVEP #$008001,X:TCSR0");/* timer enable clears counter */
}

inline int timer() /* return the current timer value */
{
int volatile tcr;

	__asm(" move X:TCR0,%0":"=D"(tcr));
	return(tcr);
}

inline int tick() /* wait one timer tick */
{
	int now;
	now = timer();
	while(timer() == now)
		;
}
inline void tinytick()
{
    __asm("\n\
	rep		#86 \n\
	nop \n\
	");
}

/* SSI = Synchronous Serial Interface. */
/* we just receive with the on chip one. */
void initSSI()
{
    __asm(" MOVEP   #>$002070,X:CRB"); /* async, LSB, disable TE RE */
    __asm(" MOVEP   #>$140803,X:CRA"); /* 10 Mbps, 16 bit word */
    __asm(" MOVEP   #>$3F,X:PCRC"); /* enable ESSI port C, to host */
    __asm(" MOVEP   #>$0,X:PCRD"); /* disable ESSI port D, to 7888 chip
					  (manually clock) */
    __asm(" MOVEP   #>$2F,X:PDRD"); /* all port D bits drive hi, but bit4 */
    __asm(" MOVEP   #>$2F,X:PRRD"); /* all port D bits GPIO outs except bit4 */
    __asm(" BSET    #$11,X:CRB"); /*  enable RE */
    __asm(" BSET    #$10,X:CRB"); /* enable TE0 */

    __asm(" BSET    #$0,X:HPCR"); /* enable GPIO bits */
    __asm(" BSET    #$03,X:HDDR"); /* bit 3 is an output */
    __asm(" BSET    #$03,X:HDR"); /* bit 3 is high to enable transmit */
    __asm(" BSET    #$02,X:HDDR"); /* bit 2 is an output */
    __asm(" BCLR    #$02,X:HDR"); /* bit 2 is low to enable receive */
    __asm(" BSET    #$13,X:CRB"); /* enable RX interrupt */
    __asm(" BSET    #$02,X:IPRP"); /* crank up RX priority */
    __asm(" BSET    #$03,X:IPRP"); /* so it it very high. */
}

void initSCI_Stepper()
{
    __asm(" MOVEP   #>$00,X:PCRE"); /* disable SCI port E, to Stepper. */
    __asm(" MOVEP   #>$06,X:PDRE"); /* set out bits, so cycling pydsp won't move motor. */
    __asm(" MOVEP   #>$06,X:PRRE"); /* port E bits 1 step and 2 direction are outs. */
}

void Forward()
{
    __asm(" BSET   #$2,X:PDRE"); /* set the direction bit */
}

void Backward()
{
    __asm(" BCLR   #$2,X:PDRE"); /* clear the direction bit */
}

void stepLow()
{
    __asm(" BCLR   #$1,X:PDRE"); /* clear the step bit */
}

void stepHigh()
{
    __asm(" BSET   #$1,X:PDRE"); /* set the step bit */
}

/* move n steps. return n if move successful. */
int sciStepN(int nreq)
{
int n;
	if (nreq < 0 )
	{
		Backward();
		n = -nreq;
	}
	else if (nreq > 0)
	{
		Forward();
		n = nreq;
	}
	else
	{
		return 0 ;
	}

	if (n > 10000) /* some sort of sanity check */
	{
		return 0;
	}

	while(n--)
	{
		tick();
		tick();
		tick();
		tick();
		tick();
		tick();
		stepLow();
		tick();
		tick();
		stepHigh();
	}
	return nreq ;
}

void shutterOpen()
{
    /* __asm(" BSET    #$04,X:HDR");  bit 4 (spare, JP2) high to open shutter */
    __asm(" movep   #ShutterOpen,Y:StatPort") ;
}

void shutterClose()
{
    /* __asm(" BCLR    #$04,X:HDR"); bit 4 (spare, JP2) low to close shutter */
    __asm(" movep   #ShutterClose,Y:StatPort") ;
}

int readPDRD() /* read port D */
{
int volatile pdrd;
    __asm(" move X:PDRD,%0":"=D"(pdrd));
    return(pdrd);
}

void writePDRD(int pdrd)
{
    __asm(" move %0,X:PDRD "::"D"(pdrd));
    __asm(" rep	#100	");
    __asm(" nop ");
}

#define SFS 0x04  
#define SCK 0x08
#define SRCV 0x10
#define STDATA 0x20
#define SOFF 0x1F

/* upon entry, CS and CLK are high. */
/* request new conversion, read back previous one */
/* chan is ALREADY shifted up 11 bits. */
/* NC | 0 | a2 | a1 | a0 | ref (=0) | PM1 (=0) | PM0(=0) | 00000000b*/
void read7888(int chan)
{
    int i=16;
    /* first, we assert the chip select*/ 
    writePDRD(SOFF-SFS); 
    for ( ; i ; i--)
    {
        if (chan & 0x008000) /* bit 15 is next data*/
        {
            writePDRD(SOFF+STDATA-SFS-SCK); /* clock low & we assert data */
            writePDRD(SOFF+STDATA-SFS);  /* rising edge, 7888 gets it . */
        }
        else
        {
            writePDRD(SOFF-SFS-SCK);
            writePDRD(SOFF-SFS); 
        } 
        chan <<= 1; /* make a bit for the data back. */
        if (readPDRD() & SRCV) /* data bit back? */
            chan += 1 ; /* set it! */
    }
    /* zero */
    writePDRD(SOFF);  /* remove chip select*/
	__asm(" move %0,Y:HssTx"::"D"(chan));
}

/*
void read7888()
{__asm(" \n
    BSET	#$0,X:PDRD				;[ ] turn on temp sensor \n
	; DO		#25,END_WAIT			;[ ] wait for output to settle \n
	; JSR		M_TIMER ; wait 25 milliseconds \n  
    ; END_WAIT \n
	;MOVEP	#WAIT3,X:BCR			;[2] set wait states for ADC \n
	;MOVEP	X:TCLKS,Y:<<SEQREG 		;[4] assert /CONVST \n
	;REP		#$4						;[2] \n
	;NOP								;[HOLD] \n
	;MOVEP	X:(TCLKS+1),Y:<<SEQREG 	;[4] deassert /CONVST and wait \n
	;REP		#$50					;[2] \n
	;NOP								;[HOLD] \n
 \n
	;MOVEP 	Y:<<ADC_B,A1  			;[2] read ADC2 \n
	;MOVE	#>$3FFF,X1				;[ ] prepare 14-bit mask \n
	;AND		X1,A1					;[ ] get 14 LSBs \n
	BCLR	#$0,X:PDRD				; turn off temp sensor \n
	BCHG	#$D,A1					;[1] 2complement to binary \n
	MOVEP	#WAIT,X:BCR				; re-set wait states \n
	MOVE	A1,X:TEMP \n"
    );
}
*/

/* readAdcs reads the LAST conversions and sends them out
 while it waits for the current pixels to settle.
then, it clocks the converters for the the current pixels. */

void readAdcs()
{
	__asm("\n\
	rep	#Clamp2Samp	; wait for the pixels to settle \n\
	nop			\n\
	movep   #ADCon,Y:DetPort ; rising edge on trigger, does nothing	\n\
	movep	y:Adc0,R0	 ; read the last 2 pixels	\n\
	movep	y:Adc0,R0	 ; read the last 2 pixels	\n\
	movep	y:Adc1,R1	 ; into a pair of handy registers. \n\
	movep	y:Adc1,R1	 ; into a pair of handy registers. \n\
	movep	R1,Y:HssTx2	 ; and send out the first pixel	  \n\
	rep	#Clamp2Samp	 ; wait more for pixels \n\
	nop                      \n\
	movep   #ADCoff,Y:DetPort ; falling edge starts conversion \n\
	rep	#2 ; wait for sample & hold to kick on.	\n\
	nop		;	\n\
	movep	R0,Y:HssTx2	; Now send out the second pixel		\n\
	":::"r0","r1");
}


/*
  the reset by row needs a some stuff here.
*/

void RegZero()
{
	__asm("\n\
	movep   #(RowOff-CSB-FSyncB),Y:RowPort ; FSyncB is data. \n\
	rep	#20	; \n\
	nop \n\
	movep   #(RowOff+VClk-FSyncB-CSB),Y:RowPort ; rising edge clocks data. \n\
	rep  #20
	nop \n\
	movep   #(RowOff-FSyncB-CSB),Y:RowPort ; falling edge. move ahead.  \n\
	rep  #20
	nop \n\
	");
}

void RegOne()
{
	__asm("\n\
	movep   #(RowOff-CSB),Y:RowPort ; \n\
	rep	#30	; \n\
	nop \n\
	movep   #(RowOff+VClk-CSB),Y:RowPort ; rising edge clocks data.  \n\
	rep  #30
	nop \n\
	movep   #(RowOff-CSB),Y:RowPort ; falling edge. move ahead.  \n\
	rep  #30
	nop \n\
	");
}

void RegReset()
{
	__asm("\n\
	movep   #(ColOff-MResetB),Y:ColPort ; just to try MReset \n\
	rep	#100	; \n\
	nop \n\
	movep   #(ColOff),Y:ColPort ; \n\
	nop	\n\
	");
}

void RegDone()
{
	__asm("\n\
	movep   #(RowOff-CSB),Y:RowPort ; make sure FSyncB not asserted.\n\
	rep	#30	; \n\
	nop \n\
	movep   #(RowOff),Y:RowPort ; then remove CSB. \n\
	nop	\n\
	");
}

void ProgramRegister()
{
	int i;

	RegReset();
	for (i=0;i<3;i++)
		RegZero();

	RegOne();

	for (i=0;i<12;i++)
		RegZero();

	RegDone();
}

void ResetOn()
{
	__asm("\n\
	move    #>RSTcount,R0 \n\
	movep   #(RowOff+ResetEn),Y:RowPort	; assert reset enable \n\
	rep		R0 \n\
	nop \n\
	");
}

void ResetOff()
{
	__asm("\n\
	move    #>RSTcount,R0 \n\
	movep   #(RowOff),Y:RowPort	; remove reset enable \n\
	rep		R0 \n\
	nop \n\
	");
}

/*  RowSync subroutine: Do FRAME START & synch the ROW shift register */
void RowSync()
{
	__asm("\n\
	movep   #(RowOff-FSyncB),Y:RowPort ; assert col sync (kill bit.) \n\
	rep  #RowSyncCount ;  \n\
	nop              \n\
	movep   #(RowOff),Y:RowPort ; remove kill bit and. \n\
	rep  #RPhiUnderlap ;  \n\
	nop              \n\
	");
}

/* ColSync subroutine: Clear the bit & sync the column shifter */
void ColSync()
{
	__asm("\n\
	movep   #(ColOff-LSyncB),Y:ColPort	; assert column kill bit \n\
	rep  #ColSyncCount				\n\
	nop								\n\
	movep   #(ColOff),Y:ColPort ; remove kill bit\n\
	rep  #CPhiUnderlap				\n\
	nop								\n\
	");
}

void RowPhiReset()
{
	__asm("\n\
	movep   #(RowOff+VClk+ResetEn),Y:RowPort ; rising edge, does nothing.  \n\
	rep  #RPhiRUnderlap				\n\
	nop								\n\
	movep   #(RowOff+ResetEn),Y:RowPort	; falling edge. clocks ahead. \n\
	rep  #RPhiRUnderlap			\n\
	nop								\n\
	");
}


/* Rising edge first on VClk */
void RowPhiNoReset()
{
	__asm("\n\
	movep   #(RowOff+VClk),Y:RowPort ; rising edge, does nothing.  \n\
	rep  #RPhiUnderlap				\n\
	nop								\n\
	movep   #(RowOff),Y:RowPort	; falling edge. clocks ahead. \n\
	rep  #RPhiUnderlap			\n\
	nop								\n\
	");
}

void ColPhiLo()
{
	__asm("\n\
	movep   #(ColOff),Y:ColPort ; rising edge, does nothing.  \n\
	rep  #CPhiUnderlap				\n\
	nop								\n\
	");
}

void ColPhiHi()
{
	__asm("\n\
	movep   #(ColOff+HClk),Y:ColPort ; rising edge, does nothing.  \n\
	rep  #CPhiUnderlap				\n\
	nop								\n\
	");
}

void ReadEnOff()
{
	__asm("\n\
	movep   #(RowOff-ReadEn),Y:RowPort ; remove the read enable during itime\n\
	nop              \n\
	");
}

/* ColSync subroutine: Clear the bit & sync the column shifter */
/* Falling edge first on HClk. */
void ColPhi()
{
	__asm("\n\
	movep   #(ColOff),Y:ColPort	; falling edge, clocks ahead\n\
	rep  #CPhiUnderlap				\n\
	nop								\n\
	movep   #(ColOff+HClk),Y:ColPort ; rising edge, does nothing.  \n\
	rep  #CPhiUnderlap				\n\
	nop								\n\
	");
}

void ColSettle()
{
	__asm("\n\
	rep  #CPhiUnderlap				\n\
	nop								\n\
	rep  #CPhiUnderlap				\n\
	nop								\n\
	rep  #CPhiUnderlap				\n\
	nop								\n\
	rep  #CPhiUnderlap				\n\
	nop								\n\
	");
}

int rxfull()
{
int volatile ssisr;

	__asm(" move X:SSISR,%0":"=D"(ssisr));
	return(ssisr&0x80);
}

int volatile gb_rxdata;
int rxdata()
{
	__asm(" move X:RXD,%0":"=D"(gb_rxdata));
	return(gb_rxdata);
}

/* send a 24 bit chunk of data in two words. */
void txdata24(int txd)
{
	__asm(" move %0,Y:HssTx"::"D"(txd));
	__asm("\n\
	rep  #100			\n\
	nop								\n\
	");
	__asm(" move %0,Y:HssTx"::"D"(txd>>16));
}

inline void txdata(int txd)
{
	__asm(" move %0,Y:HssTx"::"D"(txd));
}


#define MAX_DACS 32

#define NO_DATA_COMMAND 0
#define SET_DATA_MS 1
#define SET_DATA_NS 2
#define SET_DATA_LS 3
#define SET_ADDR_MS 4
#define SET_ADDR_NS 5
#define SET_ADDR_LS 6  /* not normally used */
#define WRITE_X		7
#define WRITE_Y		8
#define WRITE_P		9
#define READ_X		10  
#define READ_Y		11
#define READ_P		12
#define WRITE_DAC	13
#define READ_ADCS	14
#define GET_STATUS  15
#define SET_NROWS   16
#define SET_NCOLS   17
#define SET_NSAMPS  18
#define SET_ITIME   19
#define CLOCK_IMAGE 20
#define WARM_BOOT   21
#define READ_TIMER  22
#define SET_NROWSKIP 23
#define SET_NCOLSKIP 24
#define READ_7888   25
#define SCI_STEP_N 26

/* NO_DATA_COMMAND high bytes: */
#define ABORT		 0 
#define READ_ADD	 1
#define READ_DATA	 2
#define RESET_ARRAY  3
#define ROW_SYNC	 4
#define ROW_PHI1	 5
#define ROW_PHI2	 6
#define COL_SYNC	 7
#define COL_PHI1	 8
#define COL_PHI2	 9
#define RESET_IMAGE	 10
#define RAMP_IMAGE	 11
#define FRAMETIME	 12
#define PEDTIME	     13

#define MHZ80
main()
{
int dacnum,dacval[MAX_DACS];
int rxvd; /* received data */
int cmdbyte; /* command byte */
static int address = 0; /* 24 bit address */
static int dataword =0; /* 24 bit dataword */
int temp = 0; /* 24 bit temp storage. */

	initBCR();
#ifdef MHZ80
	initPLL90();
#else
	initPLL50();
#endif
	initSSI();
	initSCI_Stepper();
	initAARs();
	initSeq();
	initTTimer();
	/* Do a clockImage here for easy simulation.
	   It only executes once at load time */

	while(1)
	{
		if(rxfull())
		{
            int i,j;
			rxvd=rxdata(); /* read in the data-command word */
            rxvd &= 0xffff; /* ignore top byte */
            for ( i=0; i<256; i++)
            {
                j = rxvd;
                do
                {
			        txdata(j);
                    __asm("\n\
	                rep		#200 \n\
	                nop \n\
	                ");
                }while(j--); /* */
            }
		}
	}
}
