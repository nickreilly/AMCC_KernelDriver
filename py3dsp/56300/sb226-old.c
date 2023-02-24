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
Shutter  EQU  Seq3 ; \n\
FPhi1    EQU     Seq6 ; Pin 9 \n\
FPhi2    EQU     Seq7 ; Pin 10  \n\
FSync    EQU     Seq18 ; Pin 11 \n\
SPhi1    EQU     Seq9 ; Pin 17 \n\
SPhi2    EQU     Seq10 ; Pin 18 \n\
SSync    EQU     Seq14 ; Pin 19 \n\
pGlobal  EQU     Seq15 ; Pin 20 \n\
pReset   EQU     Seq11 ; Pin 21 \n\
prSExEn  EQU     Seq16 ; Pin 22 \n\
Vggcl    EQU     Seq17 ; Pin 23 \n\
ShutterOpen  EQU  Shutter ; \n\
ShutterClose  EQU  0 ; \n\
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
*/

int gb_nrows = 1024;
int gb_ncols = 1024;
int gb_nrowskip = 0;
int gb_ncolskip = 0;
int gb_nsamps = 1;
int gb_itime = 5000;
int gb_NCD = 1; /* number of coadds */
int gb_dactable[32] = {0} ;
int gb_useshutter = 1 ; /* use shutter, or use integration? */

int gb_reset = 0; /* toggle reset: 0=normal, 1=alwayson */
int gb_resetmode = 0;  /* toggle reset mode: 0=row 1=global */
int gb_sutr = 0;
int gb_onepix = 0;

int gb_frametime = 0; /* how many milliseconds for the last frame? */
int gb_pedtime = 0; /* how many milliseconds for nsamp pedestals? */

char *gb_compile_date = __DATE__ ;
char *gb_compile_time = __TIME__ ;

int gb_checksum = 0;
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
 * this function reads the converters 
 */

void readAdcs()
{
	register int pix1 __asm("r0");
	register int pix2 __asm("r1");
	__asm("\n\
    ; MOVEP #$079fe1,X:BCR ; \n\
	rep	#Clamp2Samp	     ; wait some more for pixels to settle \n\
	nop                  ;   \n\
    MOVEP #$3F8FFF,X:BCR ; add wait states? \n\
	nop                  ; REQUIRED! (wait state changes are not 'next cycle')  \n\
	movep	y:Adc0,R0	 ; read the last 2 pixels	\n\
	movep	y:Adc1,R1	 ; into a pair of handy registers. \n\
	movep	R1,Y:HssTx	 ; and send out the first pixel	  \n\
	rep	#Clamp2Samp	     ; wait for the serial xmit\n\
	nop			\n\
	movep	R0,Y:HssTx	 ; Now send out the second pixel		\n\
	":::"r0","r1");
    	gb_checksum += pix1;
    	gb_checksum += pix2;

	__asm("\n\
	rep	#Clamp2Samp	     ; wait some more for pixels to settle \n\
	nop                  ;   \n\
	movep	y:Adc2,R0	 ; read the last 2 pixels	\n\
	movep	y:Adc3,R1	 ; into a pair of handy registers. \n\
    	MOVEP #$3F2FFF,X:BCR ; remove wait states \n\
	nop
	movep	R1,Y:HssTx	 ; and send out the first pixel	  \n\
	rep	#Clamp2Samp	     ; wait for the serial xmit\n\
	nop			\n\
	movep	R0,Y:HssTx	 ; Now send out the second pixel		\n\
	movep   #ADCoff,Y:DetPort ; falling edge starts conversion \n\
	nop		;	\n\
	movep   #ADCon,Y:DetPort ; rising edge on trigger, does nothing	\n\
	":::"r0","r1");
    	gb_checksum += pix1;
    	gb_checksum += pix2;
}


/*
*/

void ResetArray()
{
	__asm("\n\
	move    #>RSTcount,R0 \n\
	movep   #(Off-FSync),Y:ColPort	; no phi, column kill bit ; acm 6-19-01 \n\
	movep   #(Off-pGlobal),Y:RowPort ;  assert pGlobal \n\
	movep   #(Off-pGlobal-pReset),Y:RowPort ;  assert pReset too \n\
	rep		R0 \n\
	nop \n\
	movep   #(Off-pGlobal),Y:RowPort ;  remove pReset \n\
	rep		R0 \n\
	nop \n\
	");
}

/*  RowSync subroutine: Do FRAME START & synch the ROW shift register */
void RowSync	()
{
	__asm("\n\
	movep   #(Off-SSync-Vggcl),Y:RowPort ; assert kill bit and clamp. \n\
	rep  #RowSyncCount ;  \n\
	nop              \n\
	movep   #(Off-Vggcl),Y:RowPort ; remove kill bit and still clamp. \n\
	rep  #RPhiUnderlap ;  \n\
	nop              \n\
	movep   #(Off-SPhi1-Vggcl),Y:RowPort ; clock bit, no row yet so clamp. \n\
	rep  #RowSyncCount ;  \n\
	nop              \n\
	movep   #(Off-Vggcl),Y:RowPort ; remove kill bit and still clamp. \n\
	rep  #RPhiUnderlap ;  \n\
	nop              \n\
	");
}

/* ColSync subroutine: Clear the bit & sync the column shifter */
void ColSync ()
{
	__asm("\n\
	movep   #(Off-FSync),Y:ColPort	; assert column kill bit \n\
	rep  #ColSyncCount				\n\
	nop								\n\
	movep   #(Off),Y:ColPort ; remove kill bit\n\
	rep  #CPhiUnderlap				\n\
	nop								\n\
	movep   #(Off-FPhi1),Y:ColPort	;   \n\
	rep  #ColSyncCount				\n\
	nop								\n\
	movep   #(Off),Y:ColPort ; remove kill bit\n\
	rep  #CPhiUnderlap				\n\
	nop								\n\
	");
}

void RowPhi2()
{
	__asm("\n\
	movep   #(Off-Vggcl),Y:RowPort ; remove last phase. \n\
	rep  #RPhiUnderlap				\n\
	nop								\n\
	movep   #(Off-SPhi2),Y:RowPort	;   \n\
	rep  #RowSyncCount				\n\
	nop								\n\
	");
}

void RowPhi1()
{
	__asm("\n\
	movep   #(Off-Vggcl),Y:RowPort ; remove last phase. \n\
	rep  #RPhiUnderlap				\n\
	nop								\n\
	movep   #(Off-SPhi1),Y:RowPort	;   \n\
	rep  #RowSyncCount				\n\
	nop								\n\
	");
}

void ColPhi1()
{
	__asm("\n\
	movep   #(Off),Y:ColPort ; Column phase Underlap \n\
	rep  #CPhiUnderlap				\n\
	nop								\n\
	movep   #(Off-FPhi1),Y:ColPort	;   \n\
	rep  #ColSyncCount				\n\
	nop								\n\
	");
}

void ColPhi2()
{
	__asm("\n\
	movep   #(Off),Y:ColPort ; Column phase Underlap \n\
	rep  #CPhiUnderlap				\n\
	nop								\n\
	movep   #(Off-FPhi2),Y:ColPort	;   \n\
	rep  #ColSyncCount				\n\
	nop								\n\
	");
}
 
int gb_commandbuffer[8] = {0}; /* data that we can get during a frame */
unsigned int gb_commandpointer; /* number of valid words in buffer. */

void txdata(int txd);

void txwait()
{
	__asm("\n\
	rep  #100			\n\
	nop								\n\
	");
}

void PedSig()
{
int row;
int col;
int ncolskip = gb_ncolskip>>2;
int ncols = ncolskip + (gb_ncols>>2);
int nrows = gb_nrows + gb_nrowskip;
int start;

	start = timer();
	gb_commandbuffer[1] = read7888(0,0); /* get start of frame temp. */
	gb_checksum = 0;
    RowSync();
    RowPhi1();
    for(row = 0; row < gb_nrowskip ; row++) /* burst furst. */
    {
        if(row & 0x01) /* if its even, do Phi 2, else do phi 1 */
            RowPhi1();
        else
            RowPhi2();
    }
    for(; row < nrows ; row++) /* for each row in the image */
    {
        if(row & 0x01) /* if its even, do Phi 2, else do phi 1 */
            RowPhi1();
        else
            RowPhi2();

        ColSync();
        ColPhi1();

        for(col = 0; col< ncolskip ; col++) /* burst furst  */
        {
            if( col & 0x01)
                ColPhi1();
            else
                ColPhi2();
        }
        for( ; col < ncols ; col++) /* each column in the image..  */
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

    readAdcs(); /* */
    txwait();
/*if we got a word.. recompute heater tweak. */
    /* if (gb_commandpointer) { 
        tweak_heater(gb_commandbuffer[0]);
    } */
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
    int j,k,images,samps;
    int width = gb_ncols>>2; /* */
    int height = gb_nrows;
    int start;
    int end;
    double data= 0;

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
		;
    if (gb_useshutter)
    {
        shutterClose();
	    gb_pedtime = gb_itime; 
    }
	for (samps = 0 ; samps < gb_nsamps ; samps++)
	{
		PedSig();
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
	initAARs();
	initSeq();
	initTTimer();


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
					clockImage();
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
				default:
					break;
			}
		}
	}

}
