/*
;; This file contains code to run on the Burley / OCIW science DSP
;; with modifications by Drew Moore, RIT.
*/
/*
// Version:                                                              
//      $Id:$
//                                                                           
// Revisions:                                         
//      $Log:$
// 
//  Modified Apr 8 to run at UR on new controller there.
//
*/

#include "56303regs.h"
#include "dspboard.h"

#include <setjmp.h>

extern jmp_buf abort_env;

__asm("\n\
ADC      EQU     Seq0  ; (falling edge triggers ADC) Det port \n\
Frame    EQU     Seq1  ; (High during reads) Det port \n\
Shutter  EQU  	 Seq3  ; \n\
pScanCol EQU     Seq5  ; Pin 2\n\
vResetG  EQU     Seq12 ; Pin 3 \n\
; SyncCol  EQU     Seq6  ; Pin 9 \n\
PhiC1    EQU     Seq7  ; Pin 10  \n\
PhiC2    EQU     Seq18 ; Pin 11 \n\
; SyncRow  EQU     Seq9  ; Pin 17 \n\
PhiR1    EQU     Seq10 ; Pin 18 \n\
PhiR2    EQU     Seq14 ; Pin 19 \n\
; pGlobal  EQU     Seq15 ; Pin 20 \n\
pResetR  EQU     Seq11 ; Pin 21 \n\
vRowOn   EQU     Seq16 ; Pin 22 \n\
vRowOff  EQU     Seq17 ; Pin 23 \n\
vggcl    EQU     Seq8  ; Pin 24 \n\
; some combinations of bit sequences to make coding easier \n\
ShutterOpen  EQU  Shutter ; \n\
ShutterClose  EQU  0 ; \n\
Off      EQU     $FFFFFF ; \n\
DetOff   EQU     Off-Frame-ADC ; \n\
ADCon    EQU     DetOff+Frame+ADC ;  \n\
ADCoff   EQU     DetOff+Frame ;  \n\
; some time lengths for nop waits. \n\
RSTcount      EQU  @cvi(ClockRate*200*MicroSec) \n\
RowSyncCount  EQU  @cvi(ClockRate*2.0*MicroSec) \n\
RPhiUnderlap  EQU  @cvi(ClockRate*2000*NanoSec-8) \n\
RPhiRUnderlap EQU  @cvi(ClockRate*5*MicroSec-8) \n\
ColSyncCount  EQU  @cvi(ClockRate*2.0*MicroSec) \n\
CPhiUnderlap  EQU  @cvi(ClockRate*500*NanoSec-8) \n\
RefPixSettle  EQU  @cvi(ClockRate*30*MicroSec-8) \n\
");

/*
;; 56300 config registers, occupy X memory.
*/

/*
;; Burley board registers, occupy Y memory.
*/

/*
;; Timer control variables.
*/


/*
 * global variables
 * Since the user almost always sets these with another parameter file and/or
 * changes them from the command prompt, these can be set to small arrays 
 * and itimes to make simulations run faster.
*/

int gb_nrows = 8;
int gb_ncols = 32;
int gb_nrowskip = 16;
int gb_ncolskip = 64;
int gb_nsamps = 2;
int gb_itime = 4;
int gb_ctstime = 100;
/* int gb_NCD = 1; number of coadds */

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


extern int gb_commandbuffer[];
extern unsigned int gb_commandpointer;
extern int gb_dactable[];

void initSeq()
{
	__asm("\n\
	movep   #Off,Y:RowPort ; \n\
	movep   #Off,Y:ColPort ; \n\
	movep   #DetOff,Y:DetPort ; \n\
	movep   #Seq2,Y:StatPort ; \n\
	");
}

/* Initialize some detector variables */
int initVars()
{
/* The desired ctstime is given by user (gb_ctstime).
There are two definitions of CTStime (Clamp-To-Sample Time):
1) Old UR definition for 56001 DSP - time between the selection of a
pixel and the start of conversion for that pixel.  This is the logical
definition for the CTS name, but it makes it difficult for the user
to accurately set the time between pixels since the user would have
to know and understand all the overhead for pixel transitions.
2) New UR definition for 56303 DSP - time between two successive
ADC pulses.  This comes from the desire to have the convert pulses
occur at exactly 10us intervals (100KHz pixel rate).  Thus the clock
writer must account for the overhead once (user friendly).

The number of nop's to perform is given by this equation, where the 
first number converts from the user given 0.1us to number of clock 
cycles (50MHz) and the last number is the overhead in the readAdcs()
function, i.e. equivalent number of nop's to complete instructions 
between the rep rCTStime nop and the selection of the next pixel.

With the "rep rCTStime" commented out in readADC() the minimum time
between ADC pulses is:
7440ns = 372 clock cycles
Need 4 more for "rep #" overhead when "rep rCTStime" is uncommented.
372+4=376
*/
  int ctstime = (gb_ctstime * 5) - 376;
  __asm("DEFINE  rCTStime 'N3'  \n\
         move %0,rCTStime       \n\
         nop                    \n\
         "::"D"(ctstime));
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

/*
 readAdcs reads the LAST conversions and sends them out
 while it waits for the current pixels to settle.
 then, it clocks the converters for the the current pixels.
 */

void readAdcs()
{
	register int pix1 __asm("r0");
	register int pix2 __asm("r1");
	register int pix3 __asm("r2");
	register int pix4 __asm("r3");
	__asm("\n\
	rep	rCTStime	; wait for the pixels to settle \n\
	nop			; \n\
	MOVEP #$3F8FFF,X:BCR ; add wait states? \n\
	nop		; REQUIRED wait state changes are not next cycle \n\
	movep	y:Adc0,R0	; read last 4 pix into 4 handy registers \n\
	nop			; \n\
	movep	y:Adc1,R1	; \n\
	nop			; \n\
	movep	y:Adc2,R2	; \n\
	nop			; \n\
	movep	y:Adc3,R3	; \n\
	nop			; \n\
	movep	R0,Y:HssTx	; send out the first pixel \n\
	rep	#18		; wait for data to be transmitted \n\
	nop			; \n\
        movep	R1,Y:HssTx	; Now send out the second pixel \n\
	rep	#18		; wait for data to be transmitted \n\
	nop			; \n\
	movep	R2,Y:HssTx	; and send out the third pixel	  \n\
	rep	#18		; wait for the serial xmit\n\
	nop			; \n\
	movep	R3,Y:HssTx	; Now send out the fourth pixel	\n\
	rep	#18		; wait for the serial xmit\n\
	nop			; \n\
	MOVEP #$3F2FFF,X:BCR ; remove wait states \n\
	nop			; \n\
	movep	#ADCon,Y:DetPort ; rising edge on trigger, does nothing	\n\
	nop			; \n\
	nop			; \n\
	movep	#ADCoff,Y:DetPort ; falling edge starts conversion \n\
	; wait for sample & hold \n\
        ; ADC4325 s/h acq. time is 900ns.  ADC4320 s/h acq. time is 200ns. \n\
	; With zero nop, there is about 1.2us overhead. \n\
	; rep	#8	; \n\
	nop		; \n\
	":::"r0","r1","r2","r3");
    	gb_checksum += pix1;
    	gb_checksum += pix2;
    	gb_checksum += pix3;
    	gb_checksum += pix4;
        /*  Not really true checksum since lots of low value pixels or just a few
	    high value 16-bit pixels will cause the 16-bit checksum to wrap around */
}


/*
*/

/*  RowSync subroutine: Do FRAME START & synch the ROW shift register */
void RowSync()
{
	__asm("\n\
	movep  #(Off-PhiR1-PhiR2-vRowOn),Y:RowPort ; assert kill bit. \n\
	rep  #RowSyncCount ;  \n\
	nop              \n\
	movep  #(Off-PhiR1-vRowOn),Y:RowPort ; remove kill bit, R2 first. \n\
	rep  #RPhiUnderlap ;  \n\
	nop              \n\
	");
}

void RowPhi1()
{
	__asm("\n\
	movep   #(Off-vRowOn),Y:RowPort ; remove last phase. \n\
	rep  #RPhiUnderlap				\n\
	nop								\n\
	movep   #(Off-PhiR1-vRowOn),Y:RowPort	;  select next row \n\
	rep  #RowSyncCount				\n\
	nop								\n\
	");
}

void RowPhi2()
{
	__asm("\n\
	movep   #(Off-vRowOn),Y:RowPort ; remove last phase. \n\
	rep  #RPhiUnderlap				\n\
	nop								\n\
	movep   #(Off-PhiR2-vRowOn),Y:RowPort	;  select next row \n\
	rep  #RowSyncCount				\n\
	nop								\n\
	");
}

/* 
VRowOn and VRowOff go to the unit cell row enable FET.  Which one goes 
to a particular row's set of row enable FETs is determined by the row 
shift register which controls a transmission gate for the VRowOn and 
VRowOff voltages.  This is done to reduce the noise in the unit cell 
since both voltages can be RC filtered better than a clock can be.
VRowOn is set high during a normal row-by-row reset.  This disables
the row enable FET, which keeps the output bus from seeing the reset 
voltage of the unit cells, which in turn reduces the odd-even row effect.
 */
void RowPhi1Reset()
{
	/* Normal Row Reset with VRowOn set high (off). */
	__asm("\n\
	movep #(Off-pResetR),Y:RowPort  ; remove last phase. \n\
	rep  #RPhiUnderlap				\n\
	nop								\n\
	movep #(Off-PhiR1-pResetR),Y:RowPort  ; select next row \n\
	rep  #RowSyncCount				\n\
	nop								\n\
	");
}

void RowPhi2Reset()
{
	__asm("\n\
	movep #(Off-pResetR),Y:RowPort  ; remove last phase. \n\
	rep  #RPhiUnderlap				\n\
	nop								\n\
	movep #(Off-PhiR2-pResetR),Y:RowPort  ; select next row \n\
	rep  #RowSyncCount				\n\
	nop								\n\
	");
}

void RowPhi1RstRead()
{
	/* Row Reset with VRowOn set low (on) for reading reset level.
	   DO NOT USE THESE!  This array resets by row pairs.  So the
	   first row is reset when you read it, but the second row is
	   not being reset when you read it.
	
	   Instead use the global reset (full array reset).
	*/
	__asm("\n\
	movep #(Off-vRowOn-pResetR),Y:RowPort  ; remove last phase. \n\
	rep  #RPhiUnderlap				\n\
	nop								\n\
	movep #(Off-PhiR1-vRowOn-pResetR),Y:RowPort  ; select next row \n\
	rep  #RowSyncCount				\n\
	nop								\n\
	");
}

void RowPhi2RstRead()
{
	__asm("\n\
	movep #(Off-vRowOn-pResetR),Y:RowPort  ; remove last phase. \n\
	rep  #RPhiUnderlap				\n\
	nop								\n\
	movep #(Off-PhiR2-vRowOn-pResetR),Y:RowPort  ; select next row \n\
	rep  #RowSyncCount				\n\
	nop								\n\
	");
}

/* ColSync subroutine: Clear the bit & sync the column shifter */
void ColSync ()
{
	__asm("\n\
	movep  #(Off-PhiC1-PhiC2),Y:ColPort  ; assert column kill bit \n\
	rep  #ColSyncCount				  \n\
	nop						  \n\
	movep  #(Off-PhiC1),Y:ColPort ; remove kill bit   \n\
	rep  #CPhiUnderlap				  \n\
	nop						  \n\
	");
}

void ColPhi1()
{
	__asm("\n\
	movep #(Off),Y:ColPort ; Column phase Underlap (remove previous) \n\
	rep  #CPhiUnderlap				\n\
	nop								\n\
	movep #(Off-PhiC1),Y:ColPort	; Select next column  \n\
	rep  #ColSyncCount				\n\
	nop								\n\
	");
}

void ColPhi2()
{
	__asm("\n\
	movep #(Off),Y:ColPort ; Column phase Underlap (remove previous) \n\
	rep  #CPhiUnderlap				\n\
	nop								\n\
	movep #(Off-PhiC2),Y:ColPort	; Select next column  \n\
	rep  #ColSyncCount				\n\
	nop								\n\
        ; Need some extra nops (5) to make ColPhi2 the same time length as ColPhi1 \n\
	; This is due to overhead of the C-code if-else in PedSig()  \n\
	nop \n\
	nop \n\
	nop \n\
	nop \n\
	nop \n\
	");
}

void ResetOn()
{
	__asm("\n\
	move  #>RSTcount,R5 \n\
	movep #(Off-pResetR),Y:RowPort ; assert reset enable, vRowOn high \n\
	rep		R5 \n\
	nop \n\
	");
}

void ResetOff()
{
	__asm("\n\
	move    #>RSTcount,R5 \n\
	movep   #(Off),Y:RowPort	; remove reset enable \n\
	rep		R5 \n\
	nop \n\
	movep   #(Off-vRowOn),Y:RowPort	; enable vRowOn \n\
	rep		R5 \n\
	nop \n\
	");
}

void ResetGlobal()
{
	__asm("\n\
	move    #>RSTcount,R5 \n\
	movep   #(DetOff-vResetG),Y:DetPort	; remove reset enable \n\
	rep	R5 \n\
	nop \n\
	rep	R5 \n\
	nop \n\
	");
}

void ResetGlobalOff()
{
	__asm("\n\
	move    #>RSTcount,R5 \n\
	movep   #(DetOff),Y:DetPort	; remove reset enable \n\
	rep	R5 \n\
	nop \n\
	");
}

void ResetArray()
{
int i;
	RowSync();
	if (!gb_resetmode) /* Which reset mode do we want? */
	{   /* We are doing row-by-row reset. */
	    ResetOn();
	    for (i=0;i<(gb_nrows+gb_nrowskip);i++)
	    {
                if(i & 0x01) /* if it is odd, do Phi1, else do Phi2 */
		    RowPhi1Reset();
		else
		    RowPhi2Reset();

	    }
	    /* Reset always on using row reset doesn't work. This array
		resets by row pairs.  So the first row is reset when you
		read it, but the second row is not being reset when you
		read it.
	    if (!gb_reset )
	    { 
            */
	    ResetOff(); /* not continually resetting? remove now. */
	    /* } */
	}
	else /* Ok, we are doing global (full frame) reset. */
	{
	    ResetGlobal();
	    /* NOTE:
		Global reset always on also doesn't work.  There is too
		much overhead when adding IF statements to the readAdcs()
		routine to check if we want to always keep resetting. 
		The best fix is to change the cable wiring (and then 
		this clock) to put vResetG on the RowPort instead of
		the DetPort.
	    */
	    if (!gb_reset )
	    {
		ResetGlobalOff(); /* not continually resetting. */
	    }
	}
}

 

void PedSig()
{
int row=0;
int col=0;
int ncolskip = gb_ncolskip>>2; /* bit shift to right by 2, i.e. divide by 4 */
int ncols = gb_ncols>>2;
int nrows = gb_nrows + gb_nrowskip;
int start;

    start = timer();
    gb_commandbuffer[1] = read7888(0,0); /* get start of frame temp. */
    gb_checksum = 0;
    
    RowSync();
    for(row = 0; row < gb_nrowskip ; row++) /* burst furst. */
    {
        if(row & 0x01) /* if it is odd, do Phi1, else do phi2 */
            RowPhi1();
        else
            RowPhi2();
    }
    if(gb_onepix)
    {
	ColSync();

        for(col = 0; col< ncolskip ; col++) /* burst furst  */
        {
            if( col & 0x01)
                ColPhi1();
            else
                ColPhi2();
        }
    }
    for(; row < nrows ; row++) /* for each row in the image */
    {
	if(gb_onepix)
	{
	    for(col=0 ; col < ncols ; col++) /* same pix is read ncol times */
	    {
		readAdcs();
	    }
	}
	else
	{
	    if(row & 0x01) /* if it is odd, do Phi1, else do phi2 */
		RowPhi1();
	    else
		RowPhi2();

	    ColSync();
            /* 
            Reference pixels (beginning and end of each row) have 4 times
            higher capacitance than active pixels, which requires 4 times
	    longer settling times.  Always do the first pixel of each
            row here.  
	    I should also use a longer settling time for the last pixel 
	    in each row, which is also a reference pixel with 4 times C.
	    However, to do that, I would need another IF statement which
	    would add extra overhead to the last pixel read in all 
	    sub-array reads.
            Sometimes you may not want to read the reference pixels at the
            beginning of each row.  If that is what you want, then you
            need to copy this clock and modify it (delete this part). 
            */
            ColPhi2();
            __asm("\n\
		rep  #RefPixSettle				\n\
		nop						\n\
	    ");
            readAdcs();
	    /* If we are skipping column pixels, then no readAdcs() */
            /* Start at col=1, not 0 since ref-pix is already done above 
	       and we need to start the skipping on the right clock phase.  
	       Also, we want to have the total number of skips correct.  
	       So also add 1 the ending ncolskip number. */
	    for(col = 1; col< ncolskip+1 ; col++) /* burst furst  */
	    {
		if( col & 0x01)
		    ColPhi1();
		else
		    ColPhi2();
	    }
            /* Start at col=1, not 0 since ref-pix is already done above. */
	    for(col = 1 ; col < ncols ; col++) /* each column in the image */
	    {
		if( col & 0x01)
		    ColPhi1();
		else
		    ColPhi2();
	    /* now read the converters and shuffle off the pixels */
	    /* down the high speed serial line*/
		readAdcs();
	    }
	}
    }

    readAdcs(); /* One more readAdcs to send out last converted pixels */

    checkForCommand();
    /* could do a fixed delay here with optional per-row communication */
    /* could treat each row as a mini-frame, too. */

    txwait();
    /* if we got a word.. recompute heater tweak. */
    if (gb_commandpointer) { 
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

void clockImage()
{
    int samps;
    int start,end;

    /* Put in initSeq so that all bits are output roughly about the same time.
       Also, need to have both FrameStart and ADC start low, which does not
       happen after entering the while(1) loop in main. */
    initSeq();
    initVars();
    ResetArray();
    /* Send FrameStart high before Ped */
    __asm("  movep  #(DetOff+Frame),Y:DetPort ; ");

    clearTimer();
    end = timer();
    while (timer() == end) /* wait for an edge. This adds about 1msec for wait */
        ;
    start = end = timer();
    end += gb_itime - 1; /* what time is it now? */
    for (samps = 0 ; samps < gb_nsamps ; samps++)
    {
	PedSig();
    }
    gb_pedtime = timer() - start ; 
    if (gb_useshutter)
    {
	end = timer(); /* what time is it now? */
        while (timer() == end) /*  wait for an edge. */
            ;
	end = timer() + gb_itime; /* what time is it now? */
        shutterOpen();
    }
        
    while( !((end - timer())&0x800000) ) /* wait till itime is done */
    {
	checkForCommand();
	/* long integrations are trouble for temp control!  We will need
	to arrange in advance for N comm frames during itimes. */
    }
    if (gb_useshutter)
    {
        shutterClose();
	gb_pedtime = gb_itime; 
    }
    for (samps = 0 ; samps < gb_nsamps ; samps++)
    {
	PedSig();
    }
    /* Put in initSeq just to see FrameStart go low at end of frame.*/
    initSeq();
}


void clockRampImage()
{
    int samps, end;
    /* Put in initSeq so that all bits are output roughly about the same time.
       Also, need to have both FrameStart and ADC start low, which does not
       happen after entering the while(1) loop in main. */
    initSeq();
    initVars();
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
    }
    /* Put in initSeq just to see FrameStart go low at end of frame.*/
    initSeq();
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
					__asm(" movep #$FFFF,Y:HssTx");  
					break;
				case READ_ADD:
					txdata(address);
					/* __asm(" move %0,Y:HssTx "::"S"(address)); */
					break;
				case READ_DATA:
					__asm(" move %0,Y:HssTx "::"S"(dataword)); 
					break;
				case RESET_ARRAY:
					ResetArray();
					break;
				case ROW_SYNC:
					RowSync();
					break;
				case ROW_PHI1:
					RowPhi1();
					break;
				case ROW_PHI2:
					RowPhi2();
					break;
				case COL_SYNC:
					ColSync();
					break;
				case COL_PHI1:
					ColPhi1();
					break;
				case COL_PHI2:
					ColPhi2();
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
			__asm(" move %0,Y:(%1) "::"S"(dataword),"A"(address));
			break;
		case WRITE_P:
			address &= 0xffff00;
			address |= rxvd>>8;
			__asm("	move %0,P:(%1) "::"S"(dataword),"A"(address));
			break;
		case READ_X:
			address &= 0xffff00;
			address |= rxvd>>8;
			__asm("	move X:(%0),%1	\n\
				move %1,Y:HssTx	\n\
				"::"A"(address),"S"(temp)); 
			break;
		case READ_Y:
			address &= 0xffff00;
			address |= rxvd>>8;
			__asm("	move Y:(%0),%1		\n\
				move %1,Y:HssTx		\n\
				"::"A"(address),"S"(temp)); 
			break;
		case READ_P:
			address &= 0xffff00;
			address |= rxvd>>8;
			__asm("	move P:(%0),%1		\n\
				move %1,Y:HssTx		\n\
				"::"A"(address),"S"(temp)); 
			break;
		case WRITE_DAC:
			writeDac(data_middle_byte>>8,dataword,1);
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
			gb_onepix = 0;
			break;
		case WARM_BOOT:
			__asm("JMP $000F00");
			/* break not needed. */
		case READ_TIMER:
			__asm("	move X:TCR0,%0	\n\
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
			__asm(" move %0,Y:HssTx \n\
			    "::"D"(read7888(data_middle_byte,data_middle_byte)));
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
		case SET_CTSTIME:
			gb_ctstime = dataword;
			break;
		default:
			break;
	    }
	}
    }
}
