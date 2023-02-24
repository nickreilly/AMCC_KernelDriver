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

__asm("\n\
Shutter  EQU  	 Seq3 ; \n\
; FPhi1    EQU     Seq6 ; Pin 9 \n\
; FPhi2    EQU     Seq7 ; Pin 10  \n\
; FSync    EQU     Seq18 ; Pin 11 \n\
TCK      EQU     Seq9 ; Pin 17 \n\
IMODE0   EQU     Seq10 ; Pin 18 \n\
IMODE1   EQU     Seq14 ; Pin 19 \n\
TRSTN    EQU     Seq15 ; Pin 20 \n\
TX       EQU     Seq11 ; Pin 21 \n\
PD       EQU     Seq16 ; Pin 22 \n\
TDI      EQU     Seq17 ; Pin 23 \n\
TMS      EQU     Seq8 ; Pin 24 \n\
ShutterOpen  EQU  Shutter ; \n\
ShutterClose  EQU  0 ; \n\

; Off      EQU     $FFFFFF ; Choose one: off=high \n\
; Off      EQU     $000000 ; or off=low \n\
Off      EQU     $000000+TRSTN ; TRSTN is active low \n\

RSTcount	EQU	@cvi(ClockRate*10*MicroSec) ; Reset, need > 2usec \n\
; Want to do TCK fast, but RC filter & line cap make us run slower than 100ns \n\
TCKCount	EQU	@cvi(ClockRate*500*NanoSec) ; TCK wait time \n\
; RowSyncCount	EQU	@cvi(ClockRate*2.0*MicroSec) \n\
; RPhiUnderlap	EQU	@cvi(ClockRate*2000*NanoSec-8) \n\
; RPhiRUnderlap	EQU	@cvi(ClockRate*5*MicroSec-8) \n\
; ColSyncCount	EQU	@cvi(ClockRate*2.0*MicroSec) \n\
; CPhiUnderlap	EQU	@cvi(ClockRate*500*NanoSec-8) \n\
; Clamp2Samp	EQU	@cvi(ClockRate*5000*NanoSec-8) ; don't hard code this! \n\
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

int gb_nrows = 1;
int gb_ncols = 16;
int gb_nrowskip = 0;
int gb_ncolskip = 0;
int gb_nsamps = 2;
int gb_itime = 4;
int gb_ctstime = 100;  /* clamp-to-sample time -- pix enable to ADC sample time */
/* int gb_NCD = 1; */ /* number of coadds */
int gb_dactable[32] = {0} ;
int gb_useshutter = 0 ; /* use shutter, or use integration? */



int gb_sutr = 0;  /* Use sample-up-the-ramp to take image data */
int gb_onepix = 0; /* Goto and stay on one pixel for time series data */

int gb_frametime = 0; /* how many milliseconds for the last frame? */
int gb_pedtime = 0; /* how many milliseconds for nsamp pedestals? */

char *gb_compile_date = __DATE__ ;
char *gb_compile_time = __TIME__ ;

int gb_checksum = 0;

#include <setjmp.h>

jmp_buf abort_env;

int checkForCommand(void);
/*
 * Functions to set up the board.
*/

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
  /* The desired ctstime is given by user (gb_ctstime).  The number of nop's
  to perform is given by this equation, where the first number converts from 
  the user given 0.1us to number of clock cycles (50MHz) and the last number 
  is the overhead in the readADCs() function, i.e. equivalent number of nop's 
  to complete instructions between the pixel enable and the conversion of that
  enabled pixel.  This is NOT the pixel-to-pixel time, which actually includes 
  the A/D s/h acquisition time.  As mentioned below in the readADCs() function,
  the current pixel is sampled and held before the next pixel is enabled.  
  Then once next pixel is enabled, the previous pixel's AD conversion will 
  happen during that next CTStime.  So, there is a minimum CTStime which must 
  be both greater than the overhead in the equation below and greater than the
  time it takes the AD to do an actual convert.

  See the readADCs() function!
  */
  int ctstime = (gb_ctstime * 5) - 87;
  __asm("DEFINE  rCTStime 'N3'  \n\
         move %0,rCTStime       \n\
         nop                    \n\
         "::"D"(ctstime));
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

/*
  this function reads the converters 
  readADCs first reads the LAST conversions and sends them out
  while it waits for the current pixels to settle.
  then, it clocks the converters for the the current pixels
  to start their conversion.  The ADC is a "pipeline" converter.

  Table of performance for ADs [needed for timing of readADC() func.]

		Sample&Hold	A/D		
		Acquisition	Conversion	Speed
  ADC4325	900ns		1.1us		500KHz
  ADC4322	200ns		300ns		2.0MHz
  LTC1608	400ns		1.5us		500KHz
*/
/*  Fairchild only has two outputs.
To read it out as fast as possible, we need to use the
two outputs of the sensor.  However, the designers decided that 
we don't get to have valid data on BOTH outputs simultaneously,
i.e. we get Pixel 0 on one TCK clock on the even output, but then
have to wait until the next TCK clock to get Pixel 1 on the odd
output.  But, the worst part is that Pixel 0's value is not valid
after the TCK to get Pixel 1, and we therefore can't use just one
AD convert pulse to read both pixels.  Although an argument can be
made for using one switched AD to read both outputs, in my opinion, 
this is a design flaw of the Fairchild sensor.
*/
void readEvenADC()
{
	register int pix1 __asm("r0");
	__asm("\n\
	rep	rCTStime	; wait for the pixels to settle \n\
	nop			; \n\
	MOVEP	#$3F8FFF,X:BCR	; add wait states? \n\
	nop		; REQUIRED wait state changes are not next cycle \n\
	movep	#ADCon,Y:DetPort ; rising edge on trigger, does nothing	\n\
	movep	y:Adc0,R0	; read last pixel into handy register. \n\
	nop			; \n\
	movep	R0,Y:HssTx	; send out the first pixel \n\
	rep	#18	        ; wait for data to be transmitted \n\
	nop                     ; \n\
	MOVEP	#$3F2FFF,X:BCR	; remove wait states \n\
	nop			; \n\
	movep	#ADCoff,Y:DetPort ; falling edge starts conversion. \n\
	; This pixel convert will be transmitted to computer on the next call \n\
	; of this function. \n\
	; wait for sample & hold \n\
	rep	#18  ; With one nop there is about 420ns overhead before the \n\
	nop	     ; next TCK transitions down. Unfortunately, overhead on \n\
		     ; the readOddADC is larger, so we need to make them equal \n\
		     ; This is about 860ns. \n\
	":::"r0");
    	gb_checksum += pix1;
}

void readOddADC()
{
	register int pix2 __asm("r1");
	__asm("\n\
	rep	rCTStime	; \n\
	nop			; \n\
	MOVEP	#$3F8FFF,X:BCR	; add wait states? \n\
	nop		; REQUIRED wait state changes are not next cycle \n\
	movep	#ADCon,Y:DetPort ; \n\
	movep	y:Adc1,R1	; \n\
	nop			; \n\
	movep	R1,Y:HssTx	; \n\
	rep	#18	        ; \n\
	nop                     ; \n\
	MOVEP	#$3F2FFF,X:BCR	; remove wait states \n\
	nop			; \n\
	movep	#ADCoff,Y:DetPort ; \n\
	nop	; Larger overhead on Odd due to C-code for-loop, so fewer nop. \n\
	":::"r1");
    	gb_checksum += pix2;
}

/* Reset all pixels in the array. */
void ResetArray()
{
int resetindex;

	__asm("\n\
	movep	#(Off),Y:RowPort ;  All off \n\
	rep	#TCKCount \n\
	nop	\n\
	movep	#(Off+TX),Y:RowPort ;  Turn on TX for reset on next TCK \n\
	rep	#TCKCount \n\
	nop	\n\
	movep   #(Off+TX+TCK),Y:RowPort ;  rising edge of TCK reads TX value \n\
	rep	#TCKCount \n\
	nop	\n\
	movep	#(Off+TX),Y:RowPort ; \n\
	rep	#TCKCount \n\
	nop	\n\
	");
	/* Resetting takes 400 TCK clock cycles, see Fig 8 and N10 in Table 12 */
	for (resetindex = 0 ; resetindex < 400 ; resetindex++)
	{
		__asm("\n\
		movep	#(Off),Y:RowPort ;  All off \n\
		rep	#TCKCount \n\
		nop	\n\
		movep   #(Off+TCK),Y:RowPort ;  Pulse TCK once \n\
		rep	#TCKCount \n\
		nop	\n\
		");
	}

}

/* Reset the chip's internal registers (modes) */
void SystemReset()
{
	__asm("\n\
	move	#>RSTcount,R0 \n\
	; movep	#0,Y:DetPort ; Frame sync to scope \n\
	movep	#(Off),Y:RowPort ;  All off \n\
	rep	#TCKCount \n\
	nop	\n\
	movep   #(Off+TCK),Y:RowPort ;  Pulse TCK once \n\
	rep	#TCKCount \n\
	nop	\n\
	movep   #(Off),Y:RowPort ; \n\
	rep	#TCKCount \n\
	nop	\n\
	movep	#(Off-TRSTN),Y:RowPort ; TRSTN is on = active low \n\
	rep	R0 \n\
	nop	\n\
	movep   #(Off-TRSTN+TCK),Y:RowPort ;  \n\
	rep	#TCKCount \n\
	nop	\n\
	movep   #(Off),Y:RowPort ; \n\
	rep	#TCKCount \n\
	nop	\n\
	; movep	#Frame,Y:DetPort ; Frame sync to scope\n\
	; nop	\n\
	");
}

/*  Set Acquisition mode.  See Table 3 in Fairchild data sheet. */
void AcqMode()
{
	__asm("\n\
	movep	#(Off),Y:RowPort ;  \n\
	rep	#TCKCount ;	\n\
	nop		\n\
	movep	#(Off+PD),Y:RowPort ; PD=1 + TX=0 is MRDI mode \n\
	rep	#TCKCount ;	\n\
	nop		\n\
	movep	#(Off+PD+TCK),Y:RowPort ; \n\
	rep	#TCKCount ;	\n\
	nop		\n\
	movep	#(Off),Y:RowPort ;  \n\
	rep	#TCKCount ;	\n\
	nop		\n\
	");
}

/*  Set Interface mode.  See Table 4 in Fairchild data sheet. */
void InterfaceMode()
{
	__asm("\n\
	movep	#(Off),Y:RowPort ;  \n\
	rep	#TCKCount ;	\n\
	nop		\n\
	movep	#(Off+IMODE1),Y:RowPort ; IMODE0=0 + IMODE1=1 is dual output \n\
	rep	#TCKCount ;	\n\
	nop		\n\
	movep	#(Off+IMODE1+TCK),Y:RowPort ; \n\
	rep	#TCKCount ;	\n\
	nop		\n\
	movep	#(Off),Y:RowPort ;  \n\
	rep	#TCKCount ;	\n\
	nop		\n\
	");
}

/* Reset the shift register */
void ShiftRegSync()
{
	__asm("\n\
	movep	#(Off+TDI),Y:RowPort ; Turn on TDI for Sync on next TCK \n\
	rep	#TCKCount				\n\
	nop						\n\
	movep	#(Off+TDI+TCK),Y:RowPort ; Now Shift Reg is sync'd \n\
	rep	#TCKCount				\n\
	nop						\n\
	");
}

/* Clock through pixels with TCK */
void PixelPhi()
{
	__asm("\n\
	movep	#(Off),Y:RowPort ;  \n\
	rep	#TCKCount			\n\
	nop					\n\
	movep	#(Off+TCK),Y:RowPort	;   \n\
	rep	#TCKCount			\n\
	nop					\n\
	");
}

 
int gb_commandbuffer[8] = {0}; /* data that we can get during a frame */
unsigned int gb_commandpointer; /* number of valid words in buffer. */


void txwait()
{
	__asm("\n\
	rep  #100			\n\
	nop								\n\
	");
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
/* int row; */
int col;
int ncolskip = gb_ncolskip>>1; /* Bit shift by one (divide by 2) for #outputs */
int ncols = ncolskip + (gb_ncols>>1); 
/* int nrows = gb_nrows + gb_nrowskip; */
int start;

	start = timer();
	gb_commandbuffer[1] = read7888(0,0); /* get start of frame temp. */
	gb_checksum = 0;

	/* only one row in the image! */
	ShiftRegSync();
	/* need two dummy TCK */
	PixelPhi();
	PixelPhi();

	for(col = 0; col< ncolskip ; col++) /* burst furst  */
	{
		PixelPhi();
	}
	for( ; col < ncols ; col++) /* each column in the image..  */
        {
	    PixelPhi();  /* Clock to next pixel. */
            /* now read the converters and shuffle off the pixels */
            /* down the high speed serial line*/
	    readEvenADC();
	    PixelPhi();
	    readOddADC();
        }
	checkForCommand();
	/* could do a fixed delay here with optional per-row communication */
	/* could treat each row as a mini-frame, too. */

	readEvenADC(); /* This is done just to transmit the last pixels that */
	readOddADC();  /* were converted by the previous convert pulse. */
	/* need two dummy TCK for 2052 clock cycles. */
	PixelPhi();
	PixelPhi();

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
    gb_commandpointer = 0;
    gb_commandbuffer[0] = 0;
    gb_frametime = timer() - start;
} 

/* SSI = Synchronous Serial Interface. */
/* we just receive with the on chip one. */
#ifdef NEVER
void initSSI()
{
    __asm(" MOVEP   #>$002070,X:CRB"); /* async, LSB, disable TE RE */
    __asm(" MOVEP   #>$140803,X:CRA"); /* 10 Mbps, 16 bit word */
    __asm(" MOVEP   #>$3F,X:PCRC"); /* enable ESSI */
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

#endif
/*
* The Address Attribute Registers are documented in the 56300 Family Manual
* -- section 9.6 (port A control)
* The chip has 4 address strobes that one can place in various
* address ranges.
* You set the top 12 bits to the MSBs of the address you want to map.
* The next 4 bits are how many of these 12 you care about (so the
* smallest chunk is 4K, if you use all 12.)
*/

#ifdef NEVER
void initAARs()
{
    __asm(" MOVEP #$FFFC21,X:AAR3"); /* AA3 on y memory, FFF000-FFFFFF */
    __asm(" MOVEP #$0FEC11,X:AAR2"); /* AA2 on x memory, 0FE000-0FEFFF */
    __asm(" MOVEP #$D08909,X:AAR1"); /* FLASH D00000 - D07FFF 32K*/
    __asm(" MOVEP #$000811,X:AAR0"); /* SRAM x, 000000-FFFFFF 64K */
}
#endif

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
    /* Send FrameStart high before Ped */
    __asm("  movep  #($000000+Frame),Y:DetPort ; ");
    initVars();
    SystemReset();
    AcqMode();
    InterfaceMode();
    ResetArray();

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
    SystemReset();
    AcqMode();
    InterfaceMode();
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
#define RESET_IMAGE 28 /* delete - no longer used */
#define RESET_MODE 29 /* delete - no longer used */
#define SET_CTSTIME 30

/* NO_DATA_COMMAND high bytes: */
#define ABORT		 0 
#define READ_ADD	 1
#define READ_DATA	 2
#define RESET_ARRAY	 3
#define ROW_SYNC	 4
#define ROW_PHI1	 5
#define ROW_PHI2	 6
#define COL_SYNC	 7
#define COL_PHI1	 8
#define COL_PHI2	 9
#define RAMP_IMAGE	 11
#define FRAMETIME	 12
#define PEDTIME	     	 13
#define SINGLE_PIXEL 	 14
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
	initAARs();
	initSeq();
	initTTimer();
	gb_commandpointer = 0;
	/* Do a clockImage here to make simulation easier.
	   It only executes once at load time */
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
					writeDac(data_middle_byte>>8,dataword,1);
					break;
				case READ_ADCS:
					/* readADCs(); */
					readEvenADC();
					readOddADC();
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
				case SET_CTSTIME:
					gb_ctstime = dataword;
					break;
				default:
					break;
			}
		}
	}

}
