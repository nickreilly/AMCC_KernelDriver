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
ADC      EQU     Seq0 ; (falling edge triggers ADC) Det port \n\
Frame    EQU     Seq1 ; (High during reads) Det port \n\
Shutter  EQU  	 Seq3 ; \n\
URitime  EQU     Seq6 ; Pin 9 \n\
pmc      EQU     Seq9 ; Pin 17 \n\
pdata    EQU     Seq10 ; Pin 18 \n\
pdetAdr1 EQU     Seq14 ; Pin 19 \n\
pdetAdr2 EQU     Seq15 ; Pin 20 \n\
pdetAdr3 EQU     Seq11 ; Pin 21 \n\
pdetAdr4 EQU     Seq16 ; Pin 22 \n\
sArryOn1 EQU     Seq17 ; Pin 23 \n\
sArryOn2 EQU     Seq8  ; Pin 24 \n\
; some combinations of bit sequences to make coding easier \n\
ShutterOpen  EQU  Shutter ; \n\
ShutterClose  EQU  0 ; \n\
Off      EQU     $000000 ; \n\
ADCon     EQU       Off+Frame+ADC ;  \n\
ADCoff    EQU       Off+Frame ;  \n\
; some time lengths for nop waits. \n\
RSTcount EQU   @cvi(ClockRate*200*MicroSec) \n\
RowSyncCount EQU        @cvi(ClockRate*2.0*MicroSec) \n\
RPhiUnderlap EQU        @cvi(ClockRate*500*NanoSec-8) \n\
Clamp2Samp EQU  @cvi(ClockRate*5000*NanoSec-8) \n\
");



/*
 * global variables
 * Since the user almost always sets these with another parameter file and/or
 * changes them from the command prompt, these can be set to small arrays 
 * and itimes to make simulations run faster.
*/

int gb_nrows = 4;
int gb_ncols = 4;
int gb_nrowskip = 0;
int gb_ncolskip = 0;
int gb_nsamps = 1;
int gb_itime = 336;
int gb_ctstime = 26;
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

/* set the value of the SB369 DataWord
 0xa9dd4ac3 = 1010 1001 1101 1101 0100 1010 1100 0011
 This is the nominal or default mode for most of the functions
 controlled by the dataword (see pages 89-90, 69-79, 83-85,
 and 104-105 in the SB-369 user manual).
 */
int gb_sb369_DW_LS = 0x4ac3; /* Least Signigicant half of 32 bit DataWord */
int gb_sb369_DW_MS = 0xa9dd; /* Most  Signigicant half of 32 bit DataWord */

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
2080ns = 104 clock cycles
Need 4 more for "rep #" overhead when "rep rCTStime" is uncommented.
The nominal data rate is 189.655KHz PMC or 379.310KHz per pixel,
which gives 2636ns per pixel.  We normally only set CTStime to an
accuracy of 0.1us.  So, we can either drop the  "* 5" or just add
two clock cycles (40ns), which would give CTStime values in x.x40us
instead of x.x00us.
104+4-2=106
*/
int ctstime = (gb_ctstime * 5) - 106;
int tintdelay;

    if (ctstime<1)
	ctstime=1; /* don't let user set ctstime too short! */
    __asm("DEFINE  rCTStime 'N3'  \n\
         move %0,rCTStime       \n\
         nop                    \n\
         "::"D"(ctstime));

    /* The last bit (32 tint) is for the integration adjustment.  There is a
	minimum integration bit delay of 4 PMC cycles and a max of 280 PMC cycles.
	This corresponds to an integration period that covers 336 PMC cycles
	(for int bit delay = 4 PMC) and 60 PMC cycles (for int bit delay=280PMC).
    */
    tintdelay = 340 - gb_itime;  /* 336 + 4 */
    if (tintdelay<4)
	tintdelay=4;
    if (tintdelay>280)
	tintdelay=280;
    __asm("DEFINE  rIntTimeDelay 'N4'  \n\
         move %0,rIntTimeDelay       \n\
         nop                    \n\
         "::"D"(tintdelay));

    /* Put gb_ncols into DSP register  */
    __asm("DEFINE  rNCols 'N7'  \n\
         move %0,rNCols       \n\
         nop                    \n\
         "::"D"(gb_ncols));

    /* set the value of the SB369 DataWord */
    __asm("DEFINE  rSB369dwLS 'M6'  \n\
	move %0,rSB369dwLS      \n\
	"::"D"(gb_sb369_DW_LS));  /* Least Signigicant half of 32 bit DataWord */
    __asm("DEFINE  rSB369dwMS 'M7'  \n\
	move %0,rSB369dwMS      \n\
	"::"D"(gb_sb369_DW_MS)); /* Most Signigicant half of 32 bit DataWord */

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
	__asm("\n\
	rep	rCTStime	; wait for the pixels to settle \n\
	nop			; \n\
	MOVEP	#$3F8FFF,X:BCR	; add wait states? \n\
	nop		; REQUIRED wait state changes are not next cycle \n\
	movep	y:Adc0,R0	; read last pix into handy registers \n\
	nop			; \n\
	movep	R0,Y:HssTx	; send out the first pixel \n\
	rep	#18		; wait for data to be transmitted \n\
	nop			; \n\
	MOVEP	#$3F2FFF,X:BCR	; remove wait states \n\
	nop			; \n\
	movep	#ADCon,Y:DetPort ; rising edge on trigger, does nothing	\n\
	nop			; \n\
	nop			; \n\
	movep	#ADCoff,Y:DetPort ; falling edge starts conversion \n\
	; wait for sample & hold \n\
        ; ADC4325 s/h acq. time is 900ns.  ADC4320 s/h acq. time is 200ns. \n\
	; With zero nop, there is 820ns overhead. \n\
	; With rep #3 nop, there is 980ns overhead (about minimum we want). \n\
	rep	#3	; \n\
	nop		; \n\
	":::"r0");
	gb_checksum += pix1;
}


/*  I'm not sure that there is a way to just simply reset this array,
    but there is a way to leave the reset on and have no integration time,
    which requires bit 30 of the Data Word to be set to 1.

    Currently, this is a place holder until I write the correct dataword
    to do this "reset on while read" mode.
*/
/* 
void ResetArray()
{
	__asm("\n\
	move    #>RSTcount,R0 \n\
	movep   #(Off),Y:RowPort ;   \n\
	rep		R0 \n\
	nop \n\
	");
}

void RowPhi1()
{
	__asm("\n\
	nop						\n\
	");
}

void RowPhi2()
{
	__asm("\n\
	nop						\n\
	");
}
*/

/*
Technically, there is no PedSig for the SB-369 since you only ever get
a CDS image and can never do any other kind of reads.  But, use it here
just for simple consistency with past naming conventions.
*/
void PedSig()
{
int start;
register int pixsum0 __asm("r0");
/* register int pixsum1 __asm("r1"); */

    start = timer();
    gb_commandbuffer[1] = read7888(0,0); /* get start of frame temp. */
    gb_checksum = 0;

    /* We send the detector address bits to array AND clock out the array
       at the same time (see page 111 of User Manual).  Of course, the
       user must be careful with changing the selected detectors since the
       next frame read after address changes will be invalid (pg. 108).
	I don't feel like programming in the pDetAdr clocks -- too much pain.
    */
    /*  First give Dataword, which has 31 pmc.  Really Dataword is 32 bits long,
	but that last bit is the integration bit, which can be given at nearly
	any time/pmc cycle between #36 pmc and #282 pmc.  So, don't count it
	here.

	Next, clock array.  This is 4+306+1=311 pmc cycles long.
	4 pmc for minumum delay, 306 pmc for pDetAdr and 1 pmc at end.
	31+311 = 342 pmc cycles total.
    */
    /*  A lot of this is easier in C, but the overhead of the "for" and "if"
	loops is too much, so we need to do this as pure assembly.
	Original code (abreviated) was:
	for (i=0; i<311; i++) {
		if (tintdelay==i && (i<16 || i>=221)) {...}
		if (tintdelay!=i && (i<16 || i>=221)) {...}
		if (tintdelay==i && i==16) {...}
		if (tintdelay!=i && i==16) {...}
		if (tintdelay==i && i>16 && i<221) {...}
		if (tintdelay!=i && i>16 && i<221) {...}
	}
	where the {...} was assembly code for pmc clocking and ADC pulses.
	As is, this took about 2.5us per low phase of PMC and 4.5us per high
	phase of PMC during the section with ADC pulses (between 16 and 221).
	Since the longest phase determines the total time per cycle (2 x longest
	phase), then the shortest PMC cycle is about 9us, when we really want
	5.2us (or a little shorter).
    */

    __asm("\n\
	jmp	clockArray		\n\
	; 				\n\
ADCread					\n\
	rep	rCTStime		; wait for the pixels to settle \n\
	nop				\n\
	MOVEP	#$3F8FFF,X:BCR		; add wait states - transmit takes time \n\
	nop			; REQUIRED wait state changes are not next cycle \n\
	movep	y:Adc0,n0		; read last pix into handy registers \n\
	nop				\n\
	movep	n0,Y:HssTx		; send out the first pixel \n\
	rep	#18			; wait for data to be transmitted \n\
	nop				\n\
	MOVEP	#$3F2FFF,X:BCR		; remove wait states \n\
	nop				\n\
	movep	#ADCon,Y:DetPort	; rising edge on trigger, does nothing	\n\
	nop				\n\
	nop				\n\
	movep	#ADCoff,Y:DetPort 	; falling edge starts conversion \n\
	; wait for sample & hold \n\
        ; ADC4325 s/h acq. time is 900ns.  ADC4320 s/h acq. time is 200ns. \n\
	; With zero nop, there is  160/380 ns (low/high phase pmc) overhead. \n\
	; With rep #23 nop, there is 940 ns overhead (about minimum we want). \n\
	rep	#23			\n\
	nop				\n\
	; n0=current pix, r0=pixsum is used by gb_checksum at end of asm() code \n\
	cmp a,b 	(r0)+n0 ; useless compare to do parallel move r0=r0+n0 \n\
	nop				\n\
	RTS				\n\
	;				\n\
clockArray				\n\
	do rNCols,end_do_NCols		; read-out full array NCol times \n\
	    ; bit stream generator to write out each bit stored in b \n\
	    clr     b				\n\
	    nop					\n\
            ; 0xbddd4ac3 = 1011 1101 1101 1101 0100 1010 1100 0011 \n\
	    ; This is the nominal or default mode for most of the functions \n\
	    ; controlled by the dataword (see pages 89-90, 69-79, 83-85, \n\
	    ; and 104-105 in the SB-369 user manual). \n\
	    ;move #$4ac3,b0		; cannot write b0 and b1 at same time \n\
	    move rSB369dwLS,b0			\n\
	    nop 				\n\
	    nop					\n\
	    ; b0 is 0000 0000 xxxx xxxx xxxx xxxx, but not what we need, \n\
	    ; so shift left to give b0 = xxxx xxxx xxxx xxxx 0000 0000 \n\
	    rep #8 ; shift left to remove leading zeroes at bits 17-24 \n\
	    asl b				\n\
	    ;move #$bddd,b1			\n\
	    move rSB369dwMS,b1			\n\
	    nop 				\n\
	    nop					\n\
	    ; Now b = 0000 0000 0000 0000 xxxx ... xxxx 0000 0000, \n\
	    ; but not what we need, so shift left to put the bits we want \n\
	    ; at the MSB position. \n\
	    rep #16 ; shift left to remove leading zeroes at bits 56-41 \n\
	    asl b				\n\
	    ; setup do loop to write the bits	\n\
            do #29,endSerialLoop		\n\
		; Use shift left to move MSB of B into carry register (C). \n\
		asl b				\n\
		; Is the bit in carry register set, i.e. C=1?  if yes then jump \n\
		JCS endBitZero			\n\
		movep   #(Off),Y:RowPort	\n\
		rep #95				\n\
		nop				\n\
		rep	rCTStime	; user adjustable pixel timing \n\
		nop				\n\
		; pData (dataword) values are latched on rising edge of PMC \n\
		movep   #(Off+pmc),Y:RowPort	\n\
		rep #84				\n\
		nop				\n\
		rep	rCTStime		\n\
        	nop				\n\
        	jmp endBitOne 			\n\
endBitZero					\n\
        	nop 				\n\
        	movep   #(Off+pdata),Y:RowPort	\n\
		rep #95				\n\
		nop				\n\
		rep	rCTStime		\n\
		nop 				\n\
		; DataWord bits latched on rising edge of PMC \n\
		movep   #(Off+pmc+pdata),Y:RowPort \n\
		rep #88				\n\
		nop				\n\
		rep	rCTStime		\n\
		nop				\n\
endBitOne					\n\
		nop				\n\
endSerialLoop					\n\
	    ; Give bits 30 & 31 outside above do loop so we can put in an \n\
	    ; extra clock for us to keep track of integration time. \n\
	    ; The actual integration time starts 3 pmc AFTER the integration \n\
	    ; bit (bit 32 of pdata) is given and ends 2 pmc AFTER the stop bit \n\
	    ; (bit 31 of pdata) of the next frame read.  However, to make the \n\
	    ; clock easier to write, just give the URitime clock at the same \n\
	    ; time as the integration bit and end it at the next bit #30, \n\
	    ; i.e. there are 3 PMC cycles of offset for URitime clock versus \n\
	    ; the actual integration time, but total time is correct.  \n\
	    nop 				\n\
            nop 				\n\
            nop 				\n\
            nop 				\n\
            nop 				\n\
            movep   #(Off),Y:RowPort	\n\
	    rep #95				\n\
	    nop					\n\
	    rep	rCTStime			\n\
	    nop 				\n\
	    ; DataWord bits latched on rising edge of PMC \n\
	    movep   #(Off+pmc),Y:RowPort	\n\
	    movep   #(Off),Y:ColPort		; turn off URitime clock \n\
	    rep #91				\n\
	    nop					\n\
	    rep	rCTStime			\n\
	    nop					\n\
	    ; bit 30 done, next is bit 31	\n\
	    nop 				\n\
            movep   #(Off+pdata),Y:RowPort	\n\
	    rep #95				\n\
	    nop					\n\
	    rep	rCTStime			\n\
	    nop 				\n\
	    ; DataWord bits latched on rising edge of PMC \n\
	    movep   #(Off+pmc+pdata),Y:RowPort	\n\
	    rep #80				\n\
	    nop					\n\
	    rep	rCTStime			\n\
	    nop					\n\
	    ; nop				\n\
	    ; done with bit stream for data word. \n\
	    ; Now setup for the remaining 311 PMC cycles. \n\
	    ; Use A to increment counter. B to store itime delay value. Then \n\
	    ; compare A & B to see if it is the right PMC to give IntTimeBit \n\
	    clr b				\n\
	    ; clr a    rIntTimeDelay,b	; clear a and do parallel move into b \n\
	    clr a 				\n\
	    ; copy our integration time delay number into b \n\
	    move   rIntTimeDelay,b0	; need to specify b0, default is b1 \n\
	    nop					\n\
	    do #16,end_do_preADC ; Not reading pixels yet. \n\
		cmp a,b ; does a=b? if so, we need to issue integration bit \n\
		jeq preADC_int_bit ; yes, then jump to that section of code \n\
		movep #(Off),Y:RowPort		\n\
		rep #95			; make these as long as PMC with ADC \n\
		nop				\n\
		rep	rCTStime		; user adjustable pixel timing \n\
		nop				\n\
		movep #(Off+pmc),Y:RowPort	\n\
		rep #84			; make these as long as PMC with ADC \n\
		nop				\n\
		rep	rCTStime		\n\
		nop				\n\
		jmp preADC_no_intbit  ; no integration bit, jump end do loop \n\
preADC_int_bit ; Give the integration bit.	\n\
		nop				\n\
		movep #(Off+pdata),Y:RowPort	\n\
		rep #95			; make these as long as PMC with ADC \n\
		nop				\n\
		rep	rCTStime		\n\
		nop				\n\
		movep #(Off+pmc+pdata),Y:RowPort \n\
		movep   #(Off+URitime),Y:ColPort ; turn on URitime clock \n\
		rep #85			; make these as long as PMC with ADC \n\
		nop				\n\
		rep	rCTStime		\n\
		nop				\n\
preADC_no_intbit				\n\
		inc a				\n\
end_do_preADC					\n\
	    ; first pixel read is special: no ADC on low phase, yes ADC on high \n\
	    cmp a,b ; does a=b? if so, we need to issue integration bit \n\
	    jeq firstADC_int_bit ; yes, then jump to that section of code \n\
	    movep #(Off),Y:RowPort		\n\
	    rep #91				\n\
	    nop					\n\
	    rep	rCTStime		\n\
	    nop					\n\
	    movep #(Off+pmc),Y:RowPort		\n\
	    ; nop				\n\
	    jmp firstADC_done  ; no integration bit, jump to end of section \n\
firstADC_int_bit ; Give the integration bit.	\n\
	    nop					\n\
	    movep #(Off+pdata),Y:RowPort	\n\
	    rep #91				\n\
	    nop					\n\
	    rep	rCTStime		\n\
	    nop					\n\
	    movep #(Off+pmc+pdata),Y:RowPort	\n\
	    nop					\n\
firstADC_done					\n\
	    jsr ADCread				\n\
	    inc a				\n\
	    do #203,end_do_ADC ; reading pixels. \n\
		cmp a,b ; does a=b? if so, we need to issue integration bit \n\
		jeq ADC_int_bit ; yes, then jump to that section of code \n\
		movep #(Off),Y:RowPort		\n\
		nop				\n\
		jsr ADCread			\n\
		rep #6			; extra wait to make phases equal time \n\
		nop				\n\
		movep #(Off+pmc),Y:RowPort	\n\
		nop				\n\
		jsr ADCread			\n\
		jmp ADC_no_intbit  ; no integration bit, jump end do loop \n\
ADC_int_bit ; Give the integration bit. 	\n\
		nop				\n\
		movep #(Off+pdata),Y:RowPort	\n\
		nop				\n\
		jsr ADCread			\n\
		rep #6			; extra wait to make phases equal time \n\
		nop				\n\
		movep #(Off+pmc+pdata),Y:RowPort \n\
		movep   #(Off+URitime),Y:ColPort ; turn on URitime clock \n\
		nop				\n\
		jsr ADCread			\n\
		nop				; extra nop to make timing equal\n\
ADC_no_intbit					\n\
		inc a				\n\
end_do_ADC					\n\
	    ; last pixel read is special: yes ADC on low phase, no ADC on high \n\
	    cmp a,b ; does a=b? if so, we need to issue integration bit \n\
	    jeq lastADC_int_bit ; yes, then jump to that section of code \n\
	    movep #(Off),Y:RowPort		\n\
	    ;rep #37				\n\
	    nop					\n\
	    jsr ADCread				\n\
	    rep #6				\n\
	    nop					\n\
	    movep #(Off+pmc),Y:RowPort		\n\
	    rep #80				\n\
	    nop					\n\
	    rep	rCTStime		\n\
	    nop					\n\
	    jmp lastADC_done  ; no integration bit, jump to end of this section \n\
lastADC_int_bit ; Give the integration bit	\n\
	    movep #(Off+pdata),Y:RowPort	\n\
	    nop					\n\
	    jsr ADCread				\n\
	    rep #6				\n\
	    nop					\n\
	    movep #(Off+pmc+pdata),Y:RowPort	\n\
	    movep   #(Off+URitime),Y:ColPort ; turn on URitime clock \n\
	    rep #76				\n\
	    nop					\n\
	    rep	rCTStime		\n\
	    nop					\n\
lastADC_done 					\n\
	    inc a				\n\
	    nop					\n\
	    do #90,end_do_postADC ; Done reading pixels. \n\
		cmp a,b ; does a=b? if so, we need to issue integration bit \n\
		jeq postADC_int_bit ; yes then jump to that section of code \n\
		movep #(Off),Y:RowPort		\n\
		rep #95				\n\
		nop				\n\
		rep	rCTStime		\n\
		nop				\n\
		movep #(Off+pmc),Y:RowPort	\n\
		rep #84				\n\
		nop				\n\
		rep	rCTStime		\n\
		nop				\n\
		jmp postADC_no_intbit  ; no integration bit, jump end do\n\
postADC_int_bit 				\n\
		nop				\n\
		movep #(Off+pdata),Y:RowPort	\n\
		rep #95				\n\
		nop				\n\
		rep	rCTStime		\n\
		nop					\n\
		movep #(Off+pmc+pdata),Y:RowPort \n\
		movep   #(Off+URitime),Y:ColPort ; turn on URitime clock \n\
		rep #85				\n\
		nop				\n\
		rep	rCTStime		\n\
		nop				\n\
postADC_no_intbit 				\n\
		inc a				\n\
end_do_postADC					\n\
		nop				; needed for do loop nesting \n\
end_do_NCols					\n\
	nop					\n\
	jsr ADCread		; extra ADC for last pix from pipeline ADC \n\
	nop					\n\
    ":::"r0");
    gb_checksum += pixsum0;
    /* gb_checksum += pixsum1; */
    /*  Not really true checksum since lots of low value pixels or just a few
	high value 16-bit pixels will cause the 16-bit checksum to wrap around */

    /*  We cannot easily do SINGLE PIXEL reading with the SB-369, because the
	SB-369 has a fixed way of reading, which includes an internally 
	controlled integration time (based on number of pmc clocks from previous
	frame's Tint bit and current frame's Stop bit) and Sample & Hold.
	Now, we could write another version of PedSig which stops clocking pmc
	sometime during the 49th - 252nd pmc clocks (16th and 204th after the
	stop bit, respectively), but that would not sample the noise spectrum
	of the whole signal path since we would only see the Sample & Hold
	circuit, which is not really all that informative.
	If it did work, then we can use it for 1/f noise experiments and longer
	integrations on one pix.
    */

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
    int start,end;

    /* Put in initSeq so that all bits are output roughly about the same time.
       Also, need to have both FrameStart and ADC start low, which does not
       happen after entering the while(1) loop in main. */
    initSeq();
    initVars();
    /* Send FrameStart high before Ped */
    __asm("  movep  #(Off+Frame),Y:DetPort ; ");

    clearTimer();
    end = timer();
    /* wait for an edge. This adds about 1msec for wait */
    while (timer() == end)
         ;

    start = end = timer();
    end += gb_itime - 1; /* what time is it now? */
    /* First pmc cycle labeled as "0" on the page 92 is not needed. */
    /* DataZero(); */
    /* for (samps = 0 ; samps < gb_nsamps ; samps++) */
    PedSig();
    gb_pedtime = timer() - start ;

    /*  Because of the way both PYDSP and our other clock programs are written,
	commands like sscan, bscan srun, brun, pedscan, sigscan, pedrun and 
	sigrun will ALWAYS expect to receive TWO FULL FRAME reads, even though
	the ped and sig versions only need one.  So, just put another PedSig
	in here to make it work.  The alternative is re-writing the acquire
	function in pydsp and ALL the existing clock programs to fix this.
    */
    PedSig();
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

    for (samps = 0 ; samps < gb_nsamps; samps++)
    {
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
#define sb369datawordLS 31
#define sb369datawordMS 32

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
		case sb369datawordLS:
			gb_sb369_DW_LS = dataword;
			break;
		case sb369datawordMS:
			gb_sb369_DW_MS = dataword;
			break;
		default:
			break;
	    }
	}
    }
}
