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
pmc      EQU     Seq6  ; Pin 9  \n\
pdata    EQU     Seq7  ; Pin 10 \n\
vpa1     EQU     Seq9  ; Pin 17 \n\
vpa2     EQU     Seq10 ; Pin 18 \n\
vna1     EQU     Seq14 ; Pin 19 \n\
vna2     EQU     Seq15 ; Pin 20 \n\
vpd      EQU     Seq11 ; Pin 21 \n\
vnd      EQU     Seq16 ; Pin 22 \n\
vpout    EQU     Seq17 ; Pin 23 \n\
vnout    EQU     Seq8  ; Pin 24 \n\
; some combinations of bit sequences to make coding easier \n\
ShutterOpen  EQU  Shutter ; \n\
ShutterClose  EQU  0 ; \n\
Off      EQU     $000000 ; \n\
ADCon     EQU       Off+Frame+ADC ;  \n\
ADCoff    EQU       Off+Frame ;  \n\
; some time lengths for nop waits. \n\
RSTcount EQU   @cvi(ClockRate*200*MicroSec) \n\
RowSyncCount EQU        @cvi(ClockRate*2.0*MicroSec) \n\
PMCdataLow EQU        @cvi(ClockRate*1160*NanoSec-8) \n\
PMCwait    EQU  @cvi(ClockRate*3340*NanoSec-8) \n\
Clamp2Samp EQU  @cvi(ClockRate*5000*NanoSec-8) \n\
");



/*
 * global variables
 * Since the user almost always sets these with another parameter file and/or
 * changes them from the command prompt, these can be set to small arrays 
 * and itimes to make simulations run faster.
*/

int gb_nrows = 4;
int gb_ncols = 24;
int gb_nrowskip = 0;
int gb_ncolskip = 0;
int gb_nsamps = 2;
int gb_itime = 20;
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

/* set the value of the SB339 DataWord 
For 16-bit chunks:
 0x8807 = 1000 1000 0000 0111  Most Significant, bits 0-15, with reset
 0x8007 = 1000 0000 0000 0111  Most Significant, bits 0-15, NO reset
 0xe1a6 = 1110 0001 1010 0110  Middle Significant, bits 16-31 default currents
 0xc000 = 1100 0000 0000 0000  Middle Significant, bits 16-31, lowest currents
 0x0001 = 0000 0000 0000 0001  Least Significant, bits 32-47

For 24-bit chunks:
 0x8807e1 = 1000 1000 0000 0111 1110 0001  Most Significant, bits 0-23
 0xa60001 = 1010 0110 0000 0000 0000 0001  Least Significant, bits 24-47
 
The above is the nominal or default mode for most of the functions
 controlled by the dataword (see the SB-339 user manual).

For lowest currents:
 0x8807c0 = 1000 1000 0000 0111 1100 0000
 0x1      = 0000 0000 0000 0000 0000 0001 
 */
long gb_DW_MostSig = 0x8807e1;
long gb_DW_LeastSig = 0xa60001;

extern int gb_commandbuffer[];
extern unsigned int gb_commandpointer;
extern int gb_dactable[];

void initSeq()
{
	__asm("\n\
	movep   #Off,Y:RowPort ; \n\
	movep   #Off,Y:ColPort ; \n\
	movep   #Off,Y:DetPort ; \n\
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
4540ns = 227 clock cycles
Need 4 more for "rep #" overhead when "rep rCTStime" is uncommented.
227+4=231
*/
int ctstime = (gb_ctstime * 5) - 231;
int tintdelay;

    if (ctstime<1)
	ctstime=1; /* don't let user set ctstime too short! */
    __asm("DEFINE  rCTStime 'R4'  \n\
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
}


void DataZero()
{
    __asm("\n\
        movep #(Off),Y:ColPort ;  \n\
        rep  rCTStime	; this is a user adjustable wait time	\n\
	nop							\n\
	; PMCwait makes these data PMC take same amount of time \n\
	; as pixel PMC with ADC pulses in PedSig() 		\n\
	rep  #PMCwait 						\n\
	nop							\n\
        rep  #PMCdataLow   ; this makes DataZero phases equal time	\n\
	nop							\n\
	movep #(Off+pmc),Y:ColPort	; pdata=0 is now read by ROIC  \n\
	rep  rCTStime						\n\
	nop							\n\
	rep  #PMCwait 						\n\
	nop							\n\
    ");
}

void DataOne()
{
    __asm("\n\
        movep #(Off+pdata),Y:ColPort ;  \n\
        rep  rCTStime	; this is a user adjustable wait time	\n\
	nop							\n\
	; PMCwait makes these data PMC take same amount of time \n\
	; as pixel PMC with ADC pulses in PedSig() 		\n\
	rep  #PMCwait 						\n\
	nop							\n\
        rep  #PMCdataLow   ; this makes DataZero phases equal time	\n\
	nop							\n\
	movep #(Off+pmc+pdata),Y:ColPort  ; pdata=1 is now read by ROIC  \n\
	rep  rCTStime						\n\
	nop							\n\
	rep  #PMCwait 						\n\
	nop							\n\
    ");
}

void DataWord_withReset()
{
    int i;
    /* set the value of the 48-bit SB339 DataWord.  
       Note N[0-7] are 24-bit registers, and our DSP command word has
       a max of 24-bits -- see dsp.senddsp() and ociw.data24().  
       So break into 24-bit or less chunks.  */
    /* DataWord Most Significant bits 0-23 */
    /*__asm("DEFINE  rDWMostSig 'N0'  \n\
	move %0,rDWMostSig      \n\
	"::"D"(gb_DW_MostSig)); 
    */
    /* DataWord Least Significant bits 24-47 */
    /*__asm("DEFINE  rDWLeastSig 'N1'  \n\
	move %0,rDWLeastSig      \n\
	"::"D"(gb_DW_LeastSig));  
    */

    /* Make a test pattern starting at the 24th bit, the one at far left */
    long testbit = 1;
    testbit<<=23; 
    /*  We need to guarantee a reset here, so 5th bit MUST be 1.  
	So just do the first 4 bits...
     */
    for (i=0; i<4; i++) {
	if (gb_DW_MostSig & testbit)
	{
	    /* this bit is one */
	    DataOne();
	}
	else
	{
	    DataZero();
	}
	testbit>>=1; /* shift the test pattern to the next bit to the right */
    }
    __asm("\n\
	rep  #17 ; add extra nop for equal time phase on PMC 	\n\
	nop								\n\
    ");
    /* Then do the 5th bit (Reset bit=1) separately */
    DataOne();
    testbit>>=1; /* shift the test pattern to the next bit to the right */
    i++;
    /* Next do the 6th-24th bits. */
    for (; i<24; i++) {
	if (gb_DW_MostSig & testbit)
	{
	    /* this bit is one */
	    DataOne();
	}
	else
	{
	    DataZero();
	}
	testbit>>=1; /* shift the test pattern to the next bit to the right */
    }
    /* Now, repeat for the 25-48th bits.  
       Remember, we are still only looking at a 24-bit word here. */
    testbit = 1;
    testbit<<=23; 
    for (i=0; i<24; i++) {
	if (gb_DW_LeastSig & testbit)
	{
	    /* this bit is one */
	    DataOne();
	}
	else
	{
	    DataZero();
	}
	testbit = testbit>>1; /* shift the test pattern to the next bit to the right */
    }
    __asm("movep #(Off),Y:ColPort "); /* end with pmc and pdata low */
}

void DataWord_noReset()
{
    int i;
    /* Make a test pattern starting at the 24th bit, the one at far left */
    long testbit = 1;
    testbit<<=23; 
    /*  We need to guarantee NO reset here, so 5th bit MUST be 0.  
	So just do the first 4 bits...
     */
    for (i=0; i<4; i++) {
	if (gb_DW_MostSig & testbit)
	{
	    /* this bit is one */
	    DataOne();
	}
	else
	{
	    DataZero();
	}
	testbit>>=1; /* shift the test pattern to the next bit to the right */
    }
    __asm("\n\
	rep  #17 ; add extra nop for equal time phase on PMC 	\n\
	nop								\n\
    ");
    /* Then do the 5th bit (Reset bit=0) separately */
    DataZero();
    testbit>>=1; /* shift the test pattern to the next bit to the right */
    i++;
    /* Next do the 6th-24th bits. */
    for (; i<24; i++) {
	if (gb_DW_MostSig & testbit)
	{
	    /* this bit is one */
	    DataOne();
	}
	else
	{
	    DataZero();
	}
	testbit>>=1; /* shift the test pattern to the next bit to the right */
    }
    /* Now, repeat for the 25-48th bits.  
       Remember, we are still only looking at a 24-bit word here. */
    testbit = 1;
    testbit<<=23; 
    for (i=0; i<24; i++) {
	if (gb_DW_LeastSig & testbit)
	{
	    /* this bit is one */
	    DataOne();
	}
	else
	{
	    DataZero();
	}
	testbit>>=1; /* shift the test pattern to the next bit to the right */
    }
    __asm("movep #(Off),Y:ColPort "); /* end with pmc and pdata low */
}


void PedSig()
{
int row=0;
int col=0;
int ncols = gb_ncols>>2; /* bit shift to right by 2, i.e. divide by 4 */
int nrows = gb_nrows;
int start;

    start = timer();
    gb_commandbuffer[1] = read7888(0,0); /* get start of frame temp. */
    gb_checksum = 0;

    /*  After the Serial Data Word, there needs to be 215.5 - 48 = 167.5 pmc
	cycles before we get pixels output.  Somewhere in there, there also
	needs to be an IntStart pulse (pdata high for one pmc cycle and then
	low until next serial data word).  Not sure it matters when....
     */
    for (col=0; col < 100 ; col++)
    {
	DataZero();
    __asm("\n\
	rep #27 ; add extra time on high side of PMC for equal time phases \n\
	nop				\n\
    ");
    }
    DataOne(); /* This is IntStart bit */
    col++;
    __asm("\n\
	rep #23 ; add extra time on high side of PMC for equal time phases \n\
	nop				\n\
    ");
    for (; col < 167 ; col++)
    {
	DataZero();
    __asm("\n\
	rep #27 ; add extra time on high side of PMC for equal time phases \n\
	nop				\n\
    ");
    }
    /* Now the last pmc is only a half cycle. */
    __asm("\n\
	nop				\n\
	nop				\n\
	nop				\n\
        movep #(Off),Y:ColPort ;	\n\
        rep  rCTStime			\n\
	nop				\n\
	rep #20				\n\
	nop				\n\
	rep  #PMCwait 						\n\
	nop							\n\
    ");

    /* First pixel starts on a half cycle, when pmc is high. */
    for (row=0; row < nrows ; row++)
    {
	/* Each period of pmc contains two pixels */
	for (col=0; col < ncols ; col+=2)
	{
	    __asm("movep #(Off+pmc),Y:ColPort ");
	    readAdcs();
	    __asm("\n\
		rep #14 ; add a few nops to even out two phases \n\
		nop \n\
		movep #(Off),Y:ColPort 		\n\
		nop \n\
		nop \n\
 		nop \n\
		nop \n\
		");
	    readAdcs();
	}
	/* One extra pmc period between rows - inverse of a DataZero() call */
	__asm("\n\
            movep #(Off+pmc),Y:ColPort   	\n\
            rep  rCTStime			\n\
	    nop					\n\
	    rep  #PMCwait 			\n\
	    nop					\n\
	    rep #54				\n\
	    nop					\n\
	    movep #(Off),Y:ColPort		\n\
	    rep  rCTStime			\n\
	    nop					\n\
	    rep  #PMCwait 			\n\
	    nop					\n\
	    rep #22				\n\
	    nop					\n\
	");
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
    int start,end,samps;

    /* Put in initSeq so that all bits are output roughly about the same time.
       Also, need to have both FrameStart and ADC start low, which does not
       happen after entering the while(1) loop in main. */
    initSeq();
    initVars();

    clearTimer();
    end = timer();
    /* wait for an edge. This adds about 1msec for wait */
    while (timer() == end)
         ;

    start = end = timer();
    end += gb_itime - 1; /* what time is it now? */

    /* Send FrameStart high before Ped */
    __asm("  movep  #(Off+Frame),Y:DetPort ; ");

    /* First sample needs a reset */
    DataWord_withReset();
    PedSig();
    /* Read the 2nd through Nth sample. */
    for (samps = 1 ; samps < gb_nsamps ; samps++)
    {
	DataWord_noReset();
	PedSig();
    }
    gb_pedtime = timer() - start ;

    while( !((end - timer())&0x800000) ) /* wait till itime is done */
    {
	checkForCommand();
	/* long integrations are trouble for temp control!  We will need
	to arrange in advance for N comm frames during itimes. */
    }
    /* Now read the last group of Nsamps in the Fowler-N-N image. */
    for (samps = 0 ; samps < gb_nsamps ; samps++)
    {
	DataWord_noReset();
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
    /* ResetArray(); */
    clearTimer();

    end = timer(); /* what time is it now? */
    while (timer() == end) /*  wait for an edge. */
        ;
    /* First sample needs a reset */
    DataWord_withReset();
    PedSig();
    for (samps = 1 ; samps < gb_nsamps; samps++)
    {
	while( !((end - timer())&0x800000) ) /* wait till itime is done */
		checkForCommand();
	DataWord_noReset();
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

/* continue the set of command codes from dspboard.h with specific ones for
this clock program and ROIC. */
#define sb339datawordLS 31
#define sb339datawordMS 32

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
					/* __asm(" move %0,Y:HssTx \n\
						 "::"S"(address));
					*/
					break;
				case READ_DATA:
					__asm(" move %0,Y:HssTx "::"S"(dataword));
					break;
				case RESET_ARRAY:
					/* ResetArray(); */
					break;
				case ROW_SYNC:
					/* RowSync(); */
					break;
				case ROW_PHI1:
					/* RowPhi1(); */
					break;
				case ROW_PHI2:
					/* RowPhi2(); */
					break;
				case COL_SYNC:
					/* ColSync(); */
					break;
				case COL_PHI1:
					/* ColPhi1(); */
					break;
				case COL_PHI2:
					/* ColPhi2(); */
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
			__asm(" move %0,X:(%1) "::"S"(dataword),"A"(address));
			break;
		case WRITE_Y:
			address &= 0xffff00;
			address |= rxvd>>8;
			__asm(" move %0,Y:(%1) "::"S"(dataword),"A"(address));
			break;
		case WRITE_P:
			address &= 0xffff00;
			address |= rxvd>>8;
			__asm(" move %0,P:(%1) "::"S"(dataword),"A"(address));
			break;
		case READ_X:
			address &= 0xffff00;
			address |= rxvd>>8;
			__asm(" move X:(%0),%1	\n\
				move %1,Y:HssTx	\n\
				"::"A"(address),"S"(temp)); 
			break;
		case READ_Y:
			address &= 0xffff00;
			address |= rxvd>>8;
			__asm(" move Y:(%0),%1		\n\
				move %1,Y:HssTx		\n\
				"::"A"(address),"S"(temp)); 
			break;
		case READ_P:
			address &= 0xffff00;
			address |= rxvd>>8;
			__asm(" move P:(%0),%1		\n\
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
			__asm(" move X:TCR0,%0	\n\
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
		case sb339datawordLS:
			gb_DW_LeastSig = dataword;
			break;
		case sb339datawordMS:
			gb_DW_MostSig = dataword;
			break;
		default:
			break;
	    }
	}
    }
}
