;** LOAD2.ASM  V2  uses HSSTX on burley board for checksum. ******************
;
;   Version:
;       $Id: load2.asm,v 1.1 2003/07/02 20:52:21 dsp Exp $
;
;   Revisions:
;       $Log: load2.asm,v $
;       Revision 1.1  2003/07/02 20:52:21  dsp
;       this file is the source code for the bootloader on the dsp board.
;
;
;       ROUTINE TO PROGRAM SSI AND FETCH S-RECORDS -- BOOTS FROM EEPROM
;   COMPILE WITH  ASM56300 -A -B -V  filename
;                 SREC -B -A3 -OP:2 filename 
;   then add      S20A000000003000000000C5 as 2nd line of .s file
;   then load into eeprom
;*****************************************************************************
        PAGE    110,60,1,1
        TABS    4

;*****************************************************************************
;       DEFINITIONS & POINTERS
;*****************************************************************************
START           EQU     $000F00    ; program start location
;TXD             EQU     $FFFFBC   ; unused. ESSI0 Transmit Data Register 0
RXD             EQU     $FFFFB8    ; ESSI0 Receive Data Register
SSISR           EQU     $FFFFB7    ; ESSI0 Status Register
CRB             EQU     $FFFFB6    ; ESSI0 Control Register B
CRA             EQU     $FFFFB5    ; ESSI0 Control Register A
PCRC            EQU     $FFFFBF    ; Port C (ESSI_0) control register
PCTL            EQU     $FFFFFD    ; PLL control register
; ADD SOME NEW HARDWARE..
BCR		EQU     $FFFFFB    ; Bus Control Register
AAR0            EQU     $FFFFF9    ; Address Attribute Register 0
AAR1            EQU     $FFFFF8    ; Address Attribute Register 1
AAR2            EQU     $FFFFF7    ; Address Attribute Register 2
AAR3            EQU     $FFFFF6    ; Address Attribute Register 3
HssTx           EQU     $FFFFD0    ; High speed serial transmit register
HPCR            EQU     $FFFFC4    ; Host Port Control Register
HDDR            EQU     $FFFFC8    ; Host Port Data Direction Register
HDR             EQU     $FFFFC9    ; Host Port Data Register
IPRP            EQU     $FFFFFE    ; Address Attribute Register 3 IO



; The received 16-bit words are arranged in the following order:
; [0] number of words (exluding first two header words)
; [1] memory space code X=1 Y=2 P=4 END=8
; [2] start address (bits 15..0)
; [3] start address (bits 23..16), or coded as zero

                ORG             P:$0
                JMP             START

                ORG             P:START
PLL_SETUP       MOVEP   #$078007,X:PCTL  ; set PLL parameters 80 MHz
		; PEN, PSTP, XTLD, XTLR (not sure of XTLR.. no xtal anyway)
;;;;;PLL_SETUP   MOVEP   #$038000,X:PCTL  ; set PLL parameters, 10 MHz pass thru
		; (no PEN, pass clock straight thru!)
SSI_SETUP       MOVEP   #>$002070,X:CRB  ; async, LSB, disable TE RE
                MOVEP   #>$140803,X:CRA  ; 10 Mbps, 16 bit word
                MOVEP   #>$3F,X:PCRC     ; enable ESSI
                BSET    #$11,X:CRB       ; enable RE
                BSET    #$10,X:CRB       ; enable TE0                    
           BSET    #$0,X:HPCR ; enable GPIO bits */
           BSET    #$03,X:HDDR ;  /* bit 3 is an output */
           BSET    #$03,X:HDR ;  /* bit 3 is high to enable transmit */
           BSET    #$02,X:HDDR ;  /* bit 2 is an output */
           BCLR    #$02,X:HDR ;  /* bit 2 is low to enable receive */
	;           BSET    #$13,X:CRB ;  /* enable RX interrupt */
    ;       BSET    #$02,X:IPRP ;  /* crank up RX priority */
    ;       BSET    #$03,X:IPRP ;  /* so it it very high. */

            MOVEP	#$3f2fff,X:BCR	; 1 wait state on AA3 - 0 not allowed
SET_AA3		MOVEP	#$FFFC21,X:AAR3	; put AA3 on Y memory, FFF000-FFFFFF

; SET_AA2		MOVEP	#$0FEC11,X:AAR2	; put AA2 on x memory, 0FE000-0FEFFF
; SET_AA1		MOVEP	#$D08909,X:AAR1	; put FLASH ??, D08000 to D0FFFF
; SET_AA0		MOVEP	#$000811,X:AAR0	; put SRAM on x memory, 000000-FFFFFF

GET_SREC        CLR     B       #$0,N0           ; clear checksum, offset
                JSR     READ_SSI         ; get X,Y,P,END and word count 
                MOVE    A0,X0            ; save mem space/word count
                JSR     READ_SSI         ; get starting address
                MOVE    A0,R0            ; R0 contains address
                MOVE    #>$FF,A1         ; get word count
                AND     X0,A1            ; A1 contains word count
                JSET    #$13,X0,FINISH   ; found END record

GET_WORDS       DO      A1,END_LOAD
                JSR     READ_SSI  ; get 3 byte word
X_LOAD          JCLR    #$10,X0,Y_LOAD     ; check X flag
                MOVE    A0,X:(R0)+
Y_LOAD          JCLR    #$11,X0,P_LOAD     ; check Y flag
                MOVE    A0,Y:(R0)+
P_LOAD          JCLR    #$12,X0,NO_LOAD    ; check P flag
                MOVEM   A0,P:(R0)+
NO_LOAD         NOP
END_LOAD        
                JCLR    #$6,X:SSISR,* ; wait for TDE to go high
                MOVE    B1,Y:HssTx    ; write checksum to Burley HssTx
                JMP     GET_SREC      ; go get next S-record

READ_SSI        JCLR    #$7,X:SSISR,*         ; wait for RDF to go high
                MOVE    X:RXD,X1      ; read from ESSI
                ADD     X1,B    X1,A1 ; add to checksum
                ASR     #$10,A,A      ;
                JCLR    #$7,X:SSISR,* ; wait for RDRF to go high
                MOVE    X:RXD,X1      ; read from SCI
                ADD     X1,B    X1,A1 ; add to checksum
                ASR     #$8,A,A       ; 24-bit word in A0
END_READ        RTS

FINISH          JMP     (R0)                  ;begin executing loaded program

