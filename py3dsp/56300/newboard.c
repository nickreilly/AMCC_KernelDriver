/*
;; This file contains code to run on the Burley / OCIW science DSP
;; by Drew Moore, RIT.
*/
/*
// Version:                                                              
//      $Id: newboard.c,v 1.1 2004/02/14 17:07:50 drew Exp $
//                                                                           
// Revisions:                                         
//      $Log: newboard.c,v $
*/

#include "56303regs.h"
#include "dspboard.h"

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

int gb_nrows = 4;
int gb_ncols = 8;
int gb_nrowskip = 0;
int gb_ncolskip = 0;
int gb_nsamps = 1;
int gb_itime = 1;
int gb_NCD = 1; /* number of coadds */
int gb_dactable[32] = {0} ;
int gb_useshutter = 0 ; /* use shutter, or use integration? */
int gb_reset = 0;
int gb_sutr = 0;
int gb_frametime = 0; /* how many milliseconds for the last frame? */
int gb_pedtime = 0; /* how many milliseconds for nsamp pedestals? */

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

#include <setjmp.h>

/* 
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
then, it clocks the converters for the the current pixels.
TODO: modify so accepts R/W arg array which is pointer to data block.
while waiting, data is sent out the serial XMIT line
at end of routine, settled pixels are acquired and put into array.
This allows caller to do something with conversions and
also to specify transmission words. */

void readTwoAdcs(int *data)
{
	__asm(" move %0,Y:HssTx"::"D"(data[0])); /* transmit pixel 1. */
	/* Possible concern about having output shifter active while sampling? NAH. */
	__asm("\n\
	 rep	#90 ; wait for the serial xmit\n\
	nop		;	" );
	__asm(" move %0,Y:HssTx"::"D"(data[1])); /* transmit pixel 2 */
	__asm("\n\
	 rep	#90 ; wait for the serial xmit\n\
	nop		;	" );
	__asm("\n\
	rep	#90 ; wait some more for pixels to settle \n\
	nop                  ;   \n\
	" );
	__asm("\n\
	movep   #ADCoff,Y:DetPort ; falling edge starts conversion \n\
	nop		;	SOC pulse should be short for LTC1608\n\
	movep   #ADCon,Y:DetPort ; rising edge on trigger, does nothing	\n\
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
	movep	y:Adc0,%0	 ; read these 2 pixels	\n\
	movep	y:Adc1,%1	 ; into pixel data \n\
	":"=D"(data[0]),"=D"(data[1]));
}

void readAdcs()
{
	__asm("\n\
    ; MOVEP #$079fe1,X:BCR ; \n\
	rep	#Clamp2Samp	     ; wait some more for pixels to settle \n\
	nop                  ;   \n\
    MOVEP #$3F8FFF,X:BCR ; add wait states? \n\
	nop                  ; REQUIRED! (wait state changes are not 'next cycle')  \n\
	movep	y:Adc0,R0	 ; read the last 2 pixels	\n\
	movep	y:Adc1,R1	 ; into a pair of handy registers. \n\
    MOVEP #$3F2FFF,X:BCR ; remove wait states \n\
	movep	R1,Y:HssTx	 ; and send out the first pixel	  \n\
	rep	#Clamp2Samp	     ; wait for the serial xmit\n\
	nop			\n\
	movep	R0,Y:HssTx	 ; Now send out the second pixel		\n\
	movep   #ADCoff,Y:DetPort ; falling edge starts conversion \n\
	nop		;	\n\
	movep   #ADCon,Y:DetPort ; rising edge on trigger, does nothing	\n\
	":::"r0","r1");
}


/*
  reset by row needs some stuff here.
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

#define MAXCOMMAND 4
int gb_commandbuffer[4]; /* data that we can get during a frame */
unsigned int gb_commandpointer; /* number of valid words in buffer. */

void txwait()
{
	__asm("\n\
	rep  #100			\n\
	nop								\n\
	");
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
int pixels[2] = {0x55aa, 0xaa55} ;
int gb_checksum;
	int start;
	start = timer();
	gb_checksum=0;

	RowSync();  /* clear the row shifter.  */
	RowPhi(); /* first one is single clock! */

	for(row = 0; row < gb_nrowskip ; row++) /* burst furst. */
	{
		RowPhi(); /* double clock, array is bonded every other pixel. */
		RowPhi();
	}
	for(; row < nrows ; row++) /* for each row in the image */
	{
		ColSync();
		ColPhiHi();  /* HClock Hi.. */

		for(col = 0; col< ncolskip ; col++) /* burst furst  */
		{
			ColPhi(); /* double clock */
			ColPhi();
		}
		for( ; col < ncols ; col++) /* each column in the image..  */
		{
			ColPhi(); /* double clock HCLK */
			ColPhi(); /* may want one short, since readAdcs takes time.*/
			if (col == 0)
				ColSettle();
			/* now read the converters and shuffle off the pixels */
			/* down the high speed serial line*/
			gb_checksum += pixels[0];
			gb_checksum += pixels[1];
			readTwoAdcs(pixels); /* */
		}
		ColPhi(); /* might not be needed.? */
		ColPhiLo(); 

		RowPhi(); /* double clock */
		RowPhi(); 
	}
	gb_checksum += pixels[0];
	gb_checksum += pixels[1];
	readTwoAdcs(pixels); /* */
    txwait();
    txdata(gb_commandbuffer[0]);
    txwait();
    txdata(gb_commandbuffer[1]);
    txwait();
    txdata(gb_commandbuffer[2]);
    txwait();
    txdata(gb_checksum);
    txwait();
    gb_commandpointer = 0;
    gb_commandbuffer[0] += 1;

	gb_frametime = timer() - start;
} 

void writeDac(int dacnum, int dacval)
{
#define hd_dac ((int *)0xFFFF00)

	if(dacnum < 0 || dacnum > 31)
		return;
    __asm(" MOVEP #$3F8FFF,X:BCR"); /* 4 wait states needed to write the dac */
	hd_dac[dacnum] = dacval;
    __asm(" MOVEP #$3F2FFF,X:BCR"); /* 1 wait states needed otherwise */
	gb_dactable[dacnum] = dacval;

}

int rxfull()
{
int volatile ssisr;

	__asm(" move X:SSISR,%0":"=D"(ssisr));
	return(ssisr&0x80);
}

int rxdata()
{
int volatile rxd;
	__asm(" move X:RXD,%0":"=D"(rxd));
	return(rxd);
}


/* send a 24 bit chunk of data in two words. */
void txdata24(int txd)
{
	__asm(" move %0,Y:HssTx"::"D"(txd));
    txwait();
	__asm(" move %0,Y:HssTx"::"D"(txd>>16));
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
	gb_pedtime = timer() - start;

    if (gb_useshutter)
    {
        end = timer(); /* what time is it now? */
        while (timer() == end) /*  wait for an edge. */
            ;
	end = timer() + gb_itime - 1; /* what time is it now? */
        shutterOpen();
    }
        
	while( !((end - timer())&0x800000) ) /* wait till itime is done */
		;

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
		  ;
	  end = timer() + gb_itime - 1; /* what time is it now? */
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
#define PEDTIME		 13

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
	clockImage();

	while(1)
	{
		if(rxfull())
		{
			rxvd=rxdata(); /* read in the data-command word */
			cmdbyte = rxvd & 0x0000ff; /* low byte is command */
			rxvd &= 0x00ff00; /* next byte is data. */
			/* we only receive 16 bit data, so most sig byte is dont care. */
			switch (cmdbyte)
			{
				case NO_DATA_COMMAND:
					switch(rxvd>>8)
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
						case RESET_IMAGE:
							gb_reset = 1;
							break;
						case RAMP_IMAGE:
							gb_sutr = 1;
							break;
						case FRAMETIME:
							txdata(gb_frametime);
							break;
						case PEDTIME:
                            txwait();
							txdata24(gb_pedtime);
							break;
						default:
							break;
					}
					break;
				case SET_DATA_MS:
					dataword &= 0x00ffff;
					dataword |= rxvd<<8;
					break;
				case SET_DATA_NS:
					dataword &= 0xff00ff;
					dataword |= rxvd;
					break;	
				case SET_DATA_LS:
					dataword &= 0xffff00;
					dataword |= rxvd>>8;
					break;	
				case SET_ADDR_MS:
					address &= 0x00ffff;
					address |= rxvd<<8;
					break;
				case SET_ADDR_NS:
					address &= 0xff00ff;
					address |= rxvd;
					break;
				case SET_ADDR_LS:
					address &= 0xffff00;
					address |= rxvd>>8;
					break;
				case WRITE_X:
					address &= 0xffff00;
					address |= rxvd>>8;
					__asm(" move %0,X:(%1)"::"S"(dataword),"A"(address));
					break;
				case WRITE_Y:
					address &= 0xffff00;
					address |= rxvd>>8;
					__asm(" move %0,Y:(%1)"::"S"(dataword),"A"(address));
					break;
				case WRITE_P:
					address &= 0xffff00;
					address |= rxvd>>8;
					__asm("\
						move %0,P:(%1)	\n\
					"::"S"(dataword),"A"(address));
					break;
				case READ_X:
					address &= 0xffff00;
					address |= rxvd>>8;
					__asm("\
						move X:(%0),%1	\n\
						move %1,Y:HssTx	\n\
					"::"A"(address),"S"(temp)); 
					break;
				case READ_Y:
					address &= 0xffff00;
					address |= rxvd>>8;
					__asm("\
						move Y:(%0),%1		\n\
						move %1,Y:HssTx		\n\
					"::"A"(address),"S"(temp)); 
					break;
				case READ_P:
					address &= 0xffff00;
					address |= rxvd>>8;
					__asm("\
						move P:(%0),%1		\n\
						move %1,Y:HssTx		\n\
					"::"A"(address),"S"(temp)); 
					break;
				case WRITE_DAC:
					writeDac(rxvd>>8,dataword);
					break;
				case READ_ADCS:
					readAdcs();
					break;
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
					gb_reset = 0;
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
					read7888(rxvd);
					break;
				case SCI_STEP_N:
					txdata( sciStepN(dataword)) ;
					break;
				default:
					break;
			}
		}
	}

}
