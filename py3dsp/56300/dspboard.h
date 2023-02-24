/*
;; This file defines registers specific to
  the dsp board and the other addressable boards
  in the system.
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
Adc2      EQU       $FFFF92     ; Adc \n\
Adc3      EQU       $FFFF93     ; Adc \n\
HssTx EQU $FFFFD0  ;  High Speed Serial Transmit register \n\
HssTx2 EQU $FFFFD1 ;  High Speed Serial Transmit register \n\

Seq0     EQU     $000001 ; (falling edge triggers ADC) Det port \n\
Seq1     EQU     $000002 ; (High during reads) Det port \n\
Seq2     EQU     $000004 ; only 2 channels. No Pixmux. Stat port.
Seq3     EQU     $000008 ; shutter open / close. Stat Port\n\
Seq4     EQU     $000010 ; Det port - VddUc and VSSuc simultaneously. \n\
Seq5     EQU     $000020 ; Det port - Vpd and Vnd simulateously \n\
Seq6     EQU     $000040 ; Seq6 (L) Col port \n\
Seq7     EQU     $000080 ; Seq7 (L) Col port \n\
Seq8     EQU     $000100 ; Seq8 (L) Col port \n\
Seq9     EQU     $000200 ; Seq9 (L) Row port \n\
Seq10    EQU     $000400 ; Seq10 (L) Row port \n\
Seq11    EQU     $000800 ; Seq11 (active L) Row port \n\
Seq12    EQU	 $001000 ; Seq12 (unused) Det port IIdle and Islew  \n\
Seq13    EQU	 $002000 ; Seq13 (unused) Det port VdetCom and Vddout. \n\
Seq14    EQU     $004000 ; Seq14 (clamp)  Row port \n\
Seq15    EQU     $008000 ; Seq15 clamp voltage Row port \n\
Seq16    EQU     $010000 ; Row port \n\
Seq17    EQU     $020000 ; Row port \n\
Seq18    EQU     $040000 ; Seq18 Col port?? \n\
Seq19    EQU     $080000 ; Seq19 Col port \n\

NanoSec	       EQU     0.000000001 \n\
MicroSec       EQU     0.000001 \n\
MilliSec       EQU     0.001 \n\
TenthSec       EQU     0.1 \n\
Sec            EQU     1.0 \n\
");

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
#define SET_CTSTIME 30

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



