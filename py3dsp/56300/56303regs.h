/*
  This file defines addresses specific to the 56303 processor.
*/

/*
;; 56300 config registers, occupy X memory.
*/

__asm("\nCRA EQU $FFFFB5") ; /* ESSI0 Control Register A*/
__asm("\nCRB EQU $FFFFB6") ; /* ESSI0 Control Register B*/
__asm("\nSSISR EQU $FFFFB7") ; /* Sync Serial Interface Status Register */
__asm("\nRXD EQU $FFFFB8") ; /* Received data register */
/*__asm("\nPCRC EQU $FFFFBF") ;  Port C (ESSI0) Control Register */
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
__asm("\nPCRB 	EQU $FFFFCF"); /* Port B () control register */
__asm("\nPRRB	EQU	$FFFFCE"); /* Port B () direction */
__asm("\nPDRB	EQU	$FFFFCD"); /* Port B () data */
__asm("\nPCRC 	EQU $FFFFBF"); /* Port C (ESSI_0) control register */
__asm("\nPRRC	EQU	$FFFFBE"); /* Port C (ESSI_0) direction */
__asm("\nPDRC	EQU	$FFFFBD"); /* Port C (ESSI_0) data */
__asm("\nPCRD 	EQU $FFFFAF"); /* Port D (ESSI_1) control register */
__asm("\nPRRD	EQU	$FFFFAE"); /* Port D (ESSI_1) direction */
__asm("\nPDRD	EQU	$FFFFAD"); /* Port D (ESSI_1) data */
/* Nov 9 2003: grab SCI (PORT E) bits as GPIO for stepper control */
/* Only 3 bits on port E.  bit0 = spare, bit1 = step, bit2 = dir */
__asm("\nPCRE 	EQU $FFFF9F"); /* control register, should bt 0 for GPIO */
__asm("\nPRRE	EQU	$FFFF9E"); /* direction bits. 1 = output */
__asm("\nPDRE	EQU	$FFFF9D"); /* Port E (SCI) data */

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


