/*
;; This file contains code to run on the Burley / OCIW science DSP
;; by Drew Moore, RIT.
*/
/*
// Version:                                                              
//      $Id: hawaii.c,v 1.12 2004/05/11 02:06:33 drew Exp $
//                                                                           
// Revisions:                                         
//      $Log: hawaii.c,v $
//
*/

#include "56303regs.h"
#include "dspboard.h"

__asm("\n\
Shutter  EQU  Seq3 ; \n\
LSyncB   EQU     Seq6 ; \n\
HClk     EQU     Seq7 ; \n\
MResetB  EQU     Seq8 ; \n\
VClk     EQU     Seq9 ; \n\
ResetEn  EQU     Seq10 ; \n\
ReadEn   EQU     Seq11 ; \n\
FSyncB   EQU     Seq14 ; \n\
CSB      EQU     Seq16 ; \n\
pGlobal  EQU     Seq17 ; \n\
ShutterOpen  EQU  Shutter ; \n\
ShutterClose  EQU  0 ; \n\
RowOff   EQU FSyncB+CSB+ReadEn ; \n\
ColOff   EQU MResetB+LSyncB ; \n\
DetOff   EQU 0 ; \n\
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
 * global variables gb_ prefix means 'global'
 * Since the user almost always sets these with another parameter file and/or
 * changes them from the command prompt, these can be set to small arrays 
 * and itimes to make simulations run faster.
 */

#include <setjmp.h>

int gb_nrows = 4;
int gb_ncols = 8;
int gb_nrowskip = 0;
int gb_ncolskip = 0;
int gb_nsamps = 1;
int gb_itime = 1;
int gb_NCD = 1; /* number of coadds */
int gb_dactable[32] = {0} ;
int gb_useshutter = 0 ; /* use shutter, or use integration? */
int gb_reset = 0; /* toggle reset: 0=normal, 1=alwayson */
int gb_resetmode = 0;  /* toggle reset mode: 0=row 1=global */
int gb_sutr = 0;
int gb_onepix = 0;
int gb_frametime = 0; /* how many milliseconds for the last frame? */
int gb_pedtime = 0; /* how many milliseconds for nsamp pedestals? */

char *gb_compile_date = __DATE__ ;
char *gb_compile_time = __TIME__ ;

int gb_checksum = 0;

jmp_buf abort_env;

int checkForCommand(void);
/*
 * Functions to set up the board.
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

void initAARs();
void initSSI();

/* sets number of wait states. */
void initBCR()
{
   __asm(" MOVEP #$079fe1,X:BCR"); /* 1 periph 1 SRAM, 31 eprom */
}

/* sets frequency of main oscillator */
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
void initTTimer()
{
	__asm(" MOVEP #$008000,X:TCSR0");/* timer off, use prescaler. */
#ifdef MHZ80
	__asm(" MOVEP #40000,X:TPLR");/*set the TPLR to count 1 msec */
#else
	__asm(" MOVEP #25000,X:TPLR");/*set the TPLR to count 1 msec */
#endif
	__asm(" MOVEP #$008001,X:TCSR0");/* timer on, use prescaler. */
}

void clearTimer()
{
	__asm(" MOVEP #$008000,X:TCSR0");/* timer off and back on. */
	__asm(" MOVEP #$008001,X:TCSR0");/* timer enable clears counter */
}

int timer() /* return the current timer value */
{
int volatile tcr;

	__asm(" move X:TCR0,%0":"=D"(tcr));
	return(tcr);
}

int tick() /* wait one timer tick */
{
	int now;
	now = timer();
	while(timer() == now)
		;
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
static int read7888inner(int chan)
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
	return chan;
}

int read7888(int chan, int nextchan)
/* 
 * ONLY this routine should be used to read the 7888 converter!
 * (lastchan MUST reflect the current state of the real hardware mux.)
 * CAREFUL, chan & nextchan are real_channel<<11!!
 * chan is the desired channel for THIS read.
 * nextchan is the desired channel for the NEXT read.
 * if the last channel was different than this read.
 * do an extra read to set the channel properly.
 * set nextchan for the next read.
 * the conversion is always done NOW.
 */
{
	static int lastchan = -1; /* init to an impossible value. */
	if (chan != lastchan)
	{
		(void)read7888inner(chan);
	}
	lastchan = nextchan;
	return read7888inner(nextchan);
}
/* readAdcs reads the LAST conversions and sends them out
 while it waits for the current pixels to settle.
then, it clocks the converters for the the current pixels. */

void readAdcs()
{
	register int pix1 __asm("r0");
	register int pix2 __asm("r1");
	__asm("\n\
	movep   #ADC,Y:NandPort ; falling edge starts conversion \n\
	nop		;	SOC pulse should be short for LTC1608\n\
	movep   #ADC,Y:OrPort ; rising edge on trigger, does nothing	\n\
	" );
	__asm("\n\
    	MOVEP #$3F8FFF,X:BCR ; add wait state for pixel read. \n\
	nop                  ; REQUIRED! (wait state changes are not 'next cycle')  \n\
	" );
	__asm("\n\
	rep	#Clamp2Samp	; wait for the pixels to settle \n\
	nop			\n\
	movep	y:Adc0,R0	 ; read the last 2 pixels	\n\
	movep	y:Adc1,R1	 ; into a pair of handy registers. \n\
	nop
	movep	R0,Y:HssTx2	 ; and send out the first pixel	  \n\
	rep	#Clamp2Samp	 ; wait more for pixels \n\
	nop		;	\n\
	movep	R1,Y:HssTx2	; Now send out the second pixel		\n\
	":::"r0","r1");
    gb_checksum += pix1;
    gb_checksum += pix2;
}

void readFourAdcs()
{
	register int pix1 __asm("r0");
	register int pix2 __asm("r1");
	__asm("\n\
	movep   #ADC,Y:NandPort ; falling edge starts conversion \n\
	nop		;	SOC pulse should be short for LTC1608\n\
	movep   #ADC,Y:OrPort ; rising edge on trigger, does nothing	\n\
	" );
	/* could clock array ahead here. */
	__asm("\n\
	rep	#80; wait for conversion \n\
	nop                  ;   \n\
	" );
	__asm("\n\
    	MOVEP #$3F8FFF,X:BCR ; add wait state for pixel read. \n\
	nop                  ; REQUIRED! (wait state changes are not 'next cycle')  \n\
	" );
	__asm("\n\
	movep	y:Adc0,R0	 ; read these 2 pixels	\n\
	movep	y:Adc1,R1	 ; into pixel data \n\
	movep	R0,Y:HssTx	 ; and send out the first pixel	  \n\
	rep	#Clamp2Samp	 ; wait more for pixels \n\
	nop                      \n\
	movep	R1,Y:HssTx	; Now send out the second pixel		\n\
	rep	#Clamp2Samp	 ; wait more for pixels \n\
	nop                      \n\
	movep	y:Adc2,R0	 ; read these 2 pixels	\n\
	movep	y:Adc3,R1	 ; into pixel data \n\
	movep	R0,Y:HssTx	 ; and send out the first pixel	  \n\
	rep	#Clamp2Samp	 ; wait more for pixels \n\
	nop                      \n\
	movep	R1,Y:HssTx	; Now send out the second pixel		\n\
	rep	#Clamp2Samp	 ; wait more for pixels \n\
	nop                      \n\
	":::"r0","r1");
}


/*
  the reset by row needs some stuff here.
*/

void RegZero()
{
	__asm("\n\
	movep   #(RowOff-CSB-FSyncB),Y:RowPort ; FSyncB is data. \n\
	rep	#25	; \n\
	nop \n\
	movep   #(RowOff+VClk-FSyncB-CSB),Y:RowPort ; rising edge clocks data. \n\
	rep  #25
	nop \n\
	movep   #(RowOff-FSyncB-CSB),Y:RowPort ; falling edge. move ahead.  \n\
	rep  #25
	nop \n\
	");
}

void RegOne()
{
	__asm("\n\
	movep   #(RowOff-CSB),Y:RowPort ; \n\
	rep	#25	; \n\
	nop \n\
	movep   #(RowOff+VClk-CSB),Y:RowPort ; rising edge clocks data.  \n\
	rep  #25
	nop \n\
	movep   #(RowOff-CSB),Y:RowPort ; falling edge. move ahead.  \n\
	rep  #25
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
        rep #50 \n\
	nop	\n\
	");
}

void RegDone()
{
	__asm("\n\
	movep   #(RowOff-CSB),Y:RowPort ; make sure FSyncB not asserted.\n\
	rep	#25	; \n\
	nop \n\
	movep   #(RowOff),Y:RowPort ; then remove CSB. \n\
	rep     #25	\n\
        nop     \n\
	");
}


void RegHoriDir()
{
	int i;	
	/* Set all output scan directions to 0 = left to right */
	/* 0001 0000 0000 0000 */
	for (i=0;i<3;i++)
		RegZero();

	RegOne();

	for (i=0;i<12;i++)
		RegZero();
	RegDone();
}

void RegOutputBuf()
{
	int i;	
	/* Enable the source follower buffer for output pad B 100KHz */
	/* 0100 0000 0001 0010 */
	RegZero();
	RegOne();
	for (i=0;i<9;i++)
		RegZero();
	RegOne();
	RegZero();
	RegZero();
	RegOne();
	RegZero();
	
	RegDone();
}

void RegNormalMode()
{
	int i;	
	/* reg is used for many things, but here it just enables global reset */
	/* 0101 0000 1000 0000 */
	RegZero();
	RegOne();
	RegZero();
	RegOne();
	for (i=0;i<4;i++)
		RegZero();
	RegOne();
	for (i=0;i<7;i++)
		RegZero();

	RegDone();
}


void ProgramRegister()
{
	RegReset();

	RegHoriDir();
	/* RegOutputBuf(); */ /* Uncomment if using Pad B outputs on mux */
	if (gb_resetmode)
	{
	    RegNormalMode();
	}
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
        if (gb_reset)
	{
	  __asm("\n\
	  movep   #(RowOff-FSyncB+ResetEn),Y:RowPort ; assert sync (kill bit) \n\
	  rep  #RowSyncCount ;  \n\
	  nop              \n\
	  movep   #(RowOff+ResetEn),Y:RowPort ; remove kill bit and. \n\
	  rep  #RPhiUnderlap ;  \n\
	  nop              \n\
	  ");
	}
	else 
	{
	  __asm("\n\
	  movep   #(RowOff-FSyncB),Y:RowPort ; assert sync (kill bit) \n\
	  rep  #RowSyncCount ;  \n\
	  nop              \n\
	  movep   #(RowOff),Y:RowPort ; remove kill bit and. \n\
	  rep  #RPhiUnderlap ;  \n\
	  nop              \n\
	  ");
	}
}

void RowPhiReset()
{
	__asm("\n\
	movep   #(RowOff+VClk+ResetEn),Y:RowPort ; rising edge, does nothing. \n\
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

void RowPhi()
{
	if (gb_reset)
	{
		RowPhiReset();
	}
	else 
	{
		RowPhiNoReset();
	}
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
        ;  Need extra col pulse after sync to setup shift reg and, since the
        ;  slew current goes to the enabled pixel plus the very next one, this
        ;  starts that current flowing to the very first pixel in the row.
        ;  May want to add nops at end to make sure first pix sees good Islew.
        movep   #(ColOff+HClk),Y:ColPort ; rising edge, does nothing.  \n\
	rep  #CPhiUnderlap				\n\
	nop								\n\
	movep   #(ColOff),Y:ColPort ; falling edge, but no pix enabled yet \n\
	rep  #CPhiUnderlap				\n\
	nop
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

void ReadEnOff()
{
        if (gb_reset)
	{
	  __asm("\n\
	  movep   #(RowOff-ReadEn+ResetEn),Y:RowPort  \n\
          ; remove the read enable during itime       \n\
	  nop              \n\
	  ");
	}
	else 
	{
	  __asm("\n\
	  movep   #(RowOff-ReadEn),Y:RowPort    \n\
          ; remove the read enable during itime \n\
	  nop              \n\
	  ");
	}
}

void ResetArray()
{
int i;
	RowSync();
	ResetOn();
	for (i=0;i<(gb_nrows+gb_nrowskip);i++)
	{
		RowPhiReset();
		RowPhiReset();
	}
	if (!gb_reset )
	{
		RowPhiNoReset(); 
		ResetOff();/* not continually resetting? remove now. */
	}
}

/*
 * current impl = only one word can be received during frame.
 * if it is a 0, = abort.
 * if it is 
*/
int gb_commandbuffer[8] = {0}; /* data that we can get during a frame */
unsigned int gb_commandpointer; /* number of valid words in buffer. */

/* send a 24 bit chunk of data in two words. */
void txwait()
{
	__asm("\n\
	rep  #100			\n\
	nop								\n\
	");
}

void writeBias(int dacnum, int dacval)
/*
    To write a bias voltage you must write
*/
{
#define hd_dacdata ((int *)0xFFFF97)
#define hd_bias0 ((int *)0xFFFF94)
#define hd_bias1 ((int *)0xFFFF95)

	if(dacnum < 0 || dacnum > 31)
		return;
    __asm(" MOVEP #$3F8FFF,X:BCR"); /* 4 wait states needed to write the dac */
    __asm(" MOVEP #$3F2FFF,X:BCR"); /* 1 wait states needed otherwise */
}

void writeDac(int dacnum, int dacval, int persist)
{
#define hd_dac ((int *)0xFFFF00)

	if(dacnum < 0 || dacnum > 31)
		return;
    __asm(" MOVEP #$3F8FFF,X:BCR"); /* 4 wait states needed to write the dac */
	hd_dac[dacnum] = dacval;
    __asm(" MOVEP #$3F2FFF,X:BCR"); /* 1 wait states needed otherwise */
	if (persist)
		gb_dactable[dacnum] = dacval;
}

void tweak_heater(int rxvd_tweak);

void txdata24(int txd)
{
	__asm(" move %0,Y:HssTx"::"D"(txd));
    txwait();
	__asm(" move %0,Y:HssTx"::"D"(txd>>16));
}

void txdata(int txd)
{
	__asm(" move %0,Y:HssTx"::"D"(txd));
}

void PedSig()
{
int row;
int col;
int ncolskip = gb_ncolskip>>1;
int ncols = ncolskip + (gb_ncols>>1);
int nrows = gb_nrows + gb_nrowskip;
	int start;
	start = timer();
	gb_commandbuffer[1] = read7888(0,0); /* get start of frame temp. */
	gb_checksum = 0;
	RowSync();  /* clear the row shifter.  */
	RowPhi(); /* first one is single clock! */

	for(row = 0; row < gb_nrowskip ; row++) /* burst furst. */
	{
		RowPhi();
		RowPhi(); /* double clock, array is bonded every other pixel. */
	}
	if(gb_onepix)
	{
		ColSync();
		ColPhiHi();  /* HClock Hi.. */
	
		for(col = 0; col< ncolskip ; col++) /* burst furst  */
		{
			ColPhi();
			ColPhi(); /* double clock */
		}
	}
	for(; row < nrows ; row++) /* for each row in the image */
	{
		if(gb_onepix)
		{
			for( ; col < ncols ; col++) /* each column in image. */
			{
				ColSettle();
				readAdcs(); /* Read same pixel over and over */
			}
		}
		else
		{
			ColSync();
			ColPhiHi();  /* HClock Hi.. */
	
			for(col = 0; col< ncolskip ; col++) /* burst furst  */
			{
				ColPhi();
				ColPhi(); /* double clock */
			}
			for( ; col < ncols ; col++) /* each column in image. */
			{
				ColPhi(); /*  */
				if (col == 0)
					ColSettle();
				/* now read the converters and shuffle off the */
				/* pixels down the high speed serial line */
				readAdcs(); /* */
				ColPhi(); /* double clock HCLK */
			}
			ColPhi(); /* might not be needed.? */
			ColPhiLo(); 

			RowPhi();
			RowPhi();  /* double clock */
		}
		checkForCommand();
		/* could do a fixed delay here with optional per-row communication */
		/* could treat each row as a mini-frame, too. */
	}
	readAdcs(); /* */
    txwait();
	if (gb_commandpointer) { /*if we got a word.. recompute heater tweak. */
		tweak_heater(gb_commandbuffer[0]);
	}
	gb_commandbuffer[2] = read7888(0,1<<11); /* get end of frame temp. */
	gb_commandbuffer[3] = read7888(1<<11,3<<11); /* get the bias current */
	gb_commandbuffer[4] = read7888(3<<11,0); /* get the bias voltage */
	gb_commandbuffer[5] = gb_dactable[8]; /* read dactable for heater*/
	gb_commandbuffer[6] = gb_dactable[9]; /* read dactable for other heat rail.*/
    txdata(gb_commandbuffer[0]); 
    txwait();
    txdata(gb_commandbuffer[1]);
    txwait();
    txdata(gb_commandbuffer[2]);
    txwait();
    txdata(gb_commandbuffer[3]);
    txwait();
    txdata(gb_commandbuffer[4]);
    txwait();
    txdata(gb_commandbuffer[5]);
    txwait();
    txdata(gb_commandbuffer[6]);
    txwait();
    txdata(gb_checksum);
    txwait();
    gb_commandpointer = 0;
	gb_commandbuffer[0] = 0;
	gb_frametime = timer() - start;
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
	gb_rxdata &= 0x00ffff;
	return(gb_rxdata);
}


void getStatus()
{
	/* Fill up x mem with data from the DSP */
	/* __asm(" move %0,X:(%1)"::"S"(data),"A"(add)); */

	/* Put nrows in x:0 */
	__asm(" move %0,X:0"::"S"(gb_ncols));

	/* Put ncols in x:1 */
	__asm(" move %0,X:1"::"S"(gb_nrows));

	/* Put nsamps in x:2 */
	__asm(" move %0,X:2"::"S"(gb_nsamps));

	/* Put sampmod in X:3 */
	__asm(" move %0,X:3"::"S"(gb_itime));

}

void clockImage()
{
    int samps;
	int start;
    int end;
    /* Put in initSeq so that all bits are output roughly about the same time.
       Also, need to have both FrameStart and ADC start low, which does not
       happen after entering the while(1) loop in main. */
    initSeq();
    
    ProgramRegister();
    ResetArray();
    /* Send FrameStart high before Ped */
    __asm("  movep  #(DetOff+Frame),Y:DetPort ; ");

    clearTimer();
    end = timer(); /* what time is it now? */
    while (timer() == end) /* wait for an edge. This adds about 1msec for wait */
        ;

	start = end = timer();
	end += gb_itime - 1; /* what time is it now? */
    for (samps = 0 ; samps < gb_nsamps ; samps++)
    {
	    PedSig();  /* do ped */
    }
    ReadEnOff();
	gb_pedtime = timer() - start ;

    if (gb_useshutter)
    {
        end = timer(); /* what time is it now? */
        while (timer() == end) /*  wait for an edge. */
            ;
	end = timer() + gb_itime - 1; /* what time is it now? */
        shutterOpen();
    }
        
	while( !((end - timer())&0x800000) ) /* wait till itime is done */
	{
		checkForCommand();
		/* long integrations are trouble for temp control! */
		/* we will need to arrange in advance for 
			N comm frames during itimes. */
	}

    if (gb_useshutter)
    {
        shutterClose();
    }
    for (samps = 0 ; samps < gb_nsamps ; samps++)
	{
	    PedSig();  /* do sig */
	}
    /* Put in initSeq just to see FrameStart go low at end of frame.*/
    initSeq();
    /* ReadEnOff(); */
}


void clockRampImage()
{
    int samps, end;
    ProgramRegister();
    ResetArray();
    clearTimer();

    end = timer(); /* what time is it now? */
    while (timer() == end) /*  wait for an edge. */
        ;

    for (samps = 0 ; samps < gb_nsamps; samps++)
    {
	  if (samps) /** if it's not the first frame:  */
      while( !((end - timer())&0x800000) ) /* wait till itime is done */
			checkForCommand();
	  end = timer();
	  end += gb_itime - 1; /* */
	  PedSig();
	  ReadEnOff();
    }
}


void warmBoot()
{
	__asm("JMP $000F00");
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
#define TWEAK_HEATER 27
#define RESET_IMAGE 28
#define RESET_MODE 29

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

#define RAMP_IMAGE	 11
#define FRAMETIME	 12
#define PEDTIME	     13
#define SINGLE_PIXEL 14

void tweak_heater(int rxvd_tweak)
/* assume the top byte is zero. */
{
	int heater_tweak;
	int new_heater;
	if ( (rxvd_tweak & 0xff) != TWEAK_HEATER) /* */
		return;
	heater_tweak = rxvd_tweak >> 8 ; /* 0-255 */
	heater_tweak -= (gb_dactable[8]&0xff);  /* -255 - 255 */
	if (heater_tweak < -128) 
		heater_tweak += 256;
	else if (heater_tweak > 127) 
		heater_tweak -= 256;
	/* this code will set heater rail 8 to the nearest value that
		has the same low 8 bits as the received tweak.
		this allows the host to make absolute reqests.
		it will shift rail 9 up or down in parallel. */
	new_heater = gb_dactable[8] + heater_tweak;
	writeDac(8, new_heater, 1);
	new_heater = gb_dactable[9] + heater_tweak;
	writeDac(9, new_heater, 1);
}

int checkForCommand()
{
/* check for abort = 0x0000 command during clocking. 
	if abort, longjump back to main loop. 
	if some other request, make a note of it somewhere.*/
	if(rxfull())
	{
		(void) rxdata() ; 
		/* longjmp(abort_env, 1); */
		if ((gb_rxdata&0xFFFF) == 0 )
		{
			longjmp(abort_env, 1);
		}
		else
		{
			/* make a note of what was received. */
			/* commands can come over in list fashion!*/
			/* need to either make a list*/
			if (gb_commandpointer == 0)
			{
				gb_commandbuffer[0] = gb_rxdata;
				gb_commandpointer = 1;
			}
			/* or process right here. */
			/* PC side MUST pace data properly. */
			/* we CANNOT checksum mid-stream. */
			/* */
			return 1;
		}	
	}
	return 0 ;
}

int doCommand()
{
	/* DO NOT USE.  */
	int dacval;
	int dacnum;
	/* only if everything is perfect do we change a dac.*/
	if (gb_commandpointer != 3) /* got the exact number of words expected*/
		return;
	if (gb_commandbuffer[0]&0x0000ff != SET_DATA_NS)
		return;
	if (gb_commandbuffer[1]&0x0000ff != SET_DATA_LS)
		return;
	if (gb_commandbuffer[2]&0x0000ff != WRITE_DAC)
		return;

	dacval = (gb_commandbuffer[0]&0x00ff00) | ((gb_commandbuffer[1]>>8)&0x0000ff);
	dacnum = (gb_commandbuffer[2]>>8)&0x0000ff;
	writeDac(dacnum,dacval,0);
	gb_commandpointer = 0; /* we are done with the command buffer */
}

main()
{
int dacnum,dacval[MAX_DACS];
int rxvd; /* received data */
int cmdbyte; /* command byte */
int data_middle_byte; /* data byte */
static int address = 0; /* 24 bit address */
static int dataword =0; /* 24 bit dataword */
int temp = 0; /* 24 bit temp storage. */

	initBCR();
#ifdef MHZ80
	initPLL80();
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
	gb_commandpointer = 0;
	clockImage();
	
	if(setjmp(abort_env) != 0) 
	{
		__asm(" movep  #$FFFF,Y:HssTx");  
	}

	while(1)
	{
		if(rxfull())
		{
			rxvd=rxdata(); /* read in the data-command word */
			/* RXVD = 24 bits = |00|DATAbyte|COMMANDbyte| */
			cmdbyte = rxvd & 0x0000ff; /* low byte is command */
			data_middle_byte = rxvd & 0x00ff00; /* middle byte is data */
			/* we only receive 16 bit data, so most sig byte is dont care. */
			switch (cmdbyte)
			{
				case NO_DATA_COMMAND:
					switch(data_middle_byte>>8)
					{
						case ABORT:
							__asm(" movep  #$FFFF,Y:HssTx");  
							break;
						case READ_ADD:
							txdata(address);
							/* __asm(" move  %0,Y:HssTx"::"S"(address));  */
							break;
						case READ_DATA:
							__asm(" move  %0,Y:HssTx"::"S"(dataword)); 
							break;
						case RESET_ARRAY:
							ResetArray();
							break;
						case ROW_SYNC:
							RowSync();
							break;
						case ROW_PHI1:
							RowPhi();
							break;
						case ROW_PHI2:
							RowPhi();
							break;
						case COL_SYNC:
							ColSync();
							break;
						case COL_PHI1:
							ColPhi();
							break;
						case COL_PHI2:
							ColPhi();
							break;
						case RAMP_IMAGE:
							gb_sutr = 1;
							break;
						case FRAMETIME:
						        txwait();
							txdata(gb_frametime);
							break;
						case PEDTIME:
						        txwait();
							txdata24(gb_pedtime);
							break;
						case SINGLE_PIXEL:
							gb_onepix = 1;
							break;
						default:
							break;
					}
					break;
				case SET_DATA_MS:
					dataword &= 0x00ffff;
					dataword |= data_middle_byte<<8;
					break;
				case SET_DATA_NS:
					dataword &= 0xff00ff;
					dataword |= data_middle_byte;
					break;	
				case SET_DATA_LS:
					dataword &= 0xffff00;
					dataword |= data_middle_byte>>8;
					break;	
				case SET_ADDR_MS:
					address &= 0x00ffff;
					address |= data_middle_byte<<8;
					break;
				case SET_ADDR_NS:
					address &= 0xff00ff;
					address |= data_middle_byte;
					break;
				case SET_ADDR_LS:
					address &= 0xffff00;
					address |= data_middle_byte>>8;
					break;
				case WRITE_X:
					address &= 0xffff00;
					address |= data_middle_byte>>8;
					__asm(" move %0,X:(%1)"::"S"(dataword),"A"(address));
					break;
				case WRITE_Y:
					address &= 0xffff00;
					address |= data_middle_byte>>8;
					__asm(" MOVEP #$3F8FFF,X:BCR"); /* 4 waitstates */
					__asm(" move %0,Y:(%1)"::"S"(dataword),"A"(address));
					__asm(" MOVEP #$3F2FFF,X:BCR"); /* 0 ow */
					break;
				case WRITE_P:
					address &= 0xffff00;
					address |= data_middle_byte>>8;
					__asm("\
						move %0,P:(%1)	\n\
					"::"S"(dataword),"A"(address));
					break;
				case READ_X:
					address &= 0xffff00;
					address |= data_middle_byte>>8;
					__asm("\
						move X:(%0),%1	\n\
						move %1,Y:HssTx	\n\
					"::"A"(address),"S"(temp)); 
					break;
				case READ_Y:
					address &= 0xffff00;
					address |= data_middle_byte>>8;
					__asm("\
						move Y:(%0),%1		\n\
						move %1,Y:HssTx		\n\
					"::"A"(address),"S"(temp)); 
					break;
				case READ_P:
					address &= 0xffff00;
					address |= data_middle_byte>>8;
					__asm("\
						move P:(%0),%1		\n\
						move %1,Y:HssTx		\n\
					"::"A"(address),"S"(temp)); 
					break;
				case WRITE_DAC:
					writeDac(data_middle_byte>>8,dataword,1);
					break;
				case READ_ADCS:
					{
					readFourAdcs();
					break;
					}
				case GET_STATUS:
					getStatus();
					break;
				case SET_NROWS:
					gb_nrows = dataword;
					break;
				case SET_NCOLS:
					gb_ncols = dataword;
					break;
				case SET_NSAMPS:
					gb_nsamps = dataword;
					break;
				case SET_ITIME:
					gb_itime = dataword;
					break;
				case CLOCK_IMAGE:
					if (gb_sutr)
						clockRampImage();
					else
						clockImage();
					gb_sutr=0;
					gb_onepix = 0;
					break;
				case WARM_BOOT:
					__asm("JMP $000F00");
					/* break not needed. */
				case READ_TIMER:
					__asm("\
						move X:TCR0,%0	\n\
						move %0,Y:HssTx	\n\
					"::"S"(temp)); 
                    break;
				case SET_NROWSKIP:
					gb_nrowskip = dataword;
					break;
				case SET_NCOLSKIP:
					gb_ncolskip = dataword;
					break;
				case READ_7888:
					__asm(" move %0,Y:HssTx"::"D"(read7888(data_middle_byte,data_middle_byte)));
					break;
				case SCI_STEP_N:
					txdata( sciStepN(dataword)) ;
					break;
				case TWEAK_HEATER:
					tweak_heater(rxvd);
					break;
			        case RESET_IMAGE:
					gb_reset = dataword;
					break;
				case RESET_MODE:
					gb_resetmode = dataword;
					break;
				default:
					break;
			}
		}
	}

}
