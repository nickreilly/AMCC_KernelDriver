/*
;; 56300 config registers, occupy X memory.
*/

/*
;; Burley board registers, occupy Y memory.
*/


#include "56303regs.h"
#include "dspboard.h"
#include <setjmp.h>

void initAARs()
{
    __asm(" MOVEP #$FFFC21,X:AAR3"); /* AA3 on y memory, FFF000-FFFFFF */
    __asm(" MOVEP #$0FEC11,X:AAR2"); /* AA2 on x memory, 0FE000-0FEFFF */
    __asm(" MOVEP #$D08909,X:AAR1"); /* FLASH D00000 - D07FFF 32K*/
    __asm(" MOVEP #$000811,X:AAR0"); /* SRAM x, 000000-FFFFFF 64K */
}

/* SSI = Synchronous Serial Interface. */
/* we just receive with the on chip one. */
void initSSI()
{
    __asm(" MOVEP   #>$002070,X:CRB"); /* async, LSB, disable TE RE */
    __asm(" MOVEP   #>$140803,X:CRA"); /* 10 Mbps, 16 bit word */
    __asm(" MOVEP   #>$3F,X:PCRC"); /* enable ESSI port C, to host */
    __asm(" MOVEP   #>$0,X:PCRD"); /* disable ESSI port D, to 7888 chip
					  (manually clock) */
    __asm(" MOVEP   #>$2F,X:PDRD"); /* all port D bits drive hi, but bit4 */
    __asm(" MOVEP   #>$2F,X:PRRD"); /* all port D bits GPIO outs except bit4 */
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

jmp_buf abort_env;

int gb_dactable[32] = {0} ;

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

void warmBoot()
{
	__asm("JMP $000F00");
}



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


#ifdef NEVER
void readTwoAdcs(int *data)
{
	extern int gb_checksum;
	__asm("\n\
	rep	#ConvertTime; wait for conversion \n\
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
	gb_checksum += data[0];
	gb_checksum += data[1];
	__asm(" move %0,Y:HssTx"::"D"(data[0])); /* transmit pixel 1. */
	__asm("\n\
	 rep	#SerXmit ; wait for the serial xmit\n\
	nop		;	" );
	__asm(" move %0,Y:HssTx"::"D"(data[1])); /* transmit pixel 2 */
	__asm("\n\
	 rep	#SerXmit ; wait for the serial xmit\n\
	nop		;	" );
	__asm("\n\
	rep	#(Clamp2Samp-2*SerXmit) ; wait some more for pixels to settle \n\
	nop                  ;   \n\
	" );
	__asm("\n\
	movep   #ADCoff,Y:DetPort ; falling edge starts conversion \n\
	nop		;	SOC pulse should be short for LTC1608\n\
	movep   #ADCon,Y:DetPort ; rising edge on trigger, does nothing	\n\
	" );
	/* could clock array ahead here. */
}
#endif
