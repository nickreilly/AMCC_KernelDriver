// Driver routines for CCD controller digital signal processor interface 
//
// Version:
//     $Id: sload.c,v 1.1 2003/08/14 02:40:17 dsp Exp $
//
// Revisions:
//     $Log: sload.c,v $
//     Revision 1.1  2003/08/14 02:40:17  dsp
//     initial UR versions.
//     The rit versions are included in the header.
//
//
//     Revision 1.6  2002/11/24 19:54:33  drew
//     latest stuff.
//
//     Revision 1.5  2002/09/11 13:16:31  lars
//     Changed reboot to cold_boot
//
//     Revision 1.4  2002/08/05 16:28:52  lars
//     Added newline after the clear_fifo message gets called.
//     The checksum is still commented out.
//
//     Revision 1.3  2002/07/29 15:53:14  lars
//     Now using the ociw.h from ../ociwpci
//
//     Revision 1.2  2002/07/17 14:12:11  lars
//     Added CVS Header
//
//     
//
#define DSPDVR_C

#include <stdio.h>
#include <unistd.h> // read and write
#include <fcntl.h> // ushort
#include <sys/ioctl.h> // ioctl stuff.
#include <linux/types.h> // __u32
#include "../ociwpci/ociw.h"  // hardware stuff..

//
//#include <stdlib.h>
//#include <string.h>
//#include <sys/time.h>
//#include <stdlib.h>
// #include <sys/types.h>
//#include <termio.h>
// #include "shared.h"
//#include "motxer.h"

/*========== definitions ====================================================*/
/* Define the DSP address locations for the following control parameters */

#define  MAX_RETRYS		50			/* max consecutive retrys */
#define  TIMEOUT		20			/* read time-out (u_sec) */
#define  SLEEP			20			/* Sleep time at start-up (* 100 msec) */

#define  WLEN  			4			/* Word length is 4 bytes for SSI */

/*=============== function prototypes =======================================*/
int dsp_coldboot(void);
int dsp_clear_fifo(int fd);
int dsp_init(char *srec_file, int dsp_fd);

//static void usleep(int u_secs);

/*========== local variables ================================================*/
//static int dsp; -> moved to shared.h
// static struct termios tty, save_tty;

/*	sci         file descriptor for SPARC serial port (ttya)     */
/*	ssi         file descriptor for SSI high speed serial link   */
/*	srec_file	file containing S-records to load into DSP       */

//  static int xmem[1000];
//  static int ymem[1000];
//  static int reg;

/*========== Open/configure/close the SPARC serial port =====================*/ 

void dsp_reset(int dsp)
{
	int r;

	// Reset the DSP through the PCI interface or by toggling
	// the serial port RTS -- then wait for PLL to lock 
	printf("resetting DSP\n");
	r = ioctl(dsp, OCIW_RESET);  // reset the whole board
	printf("done. wait for PLL\n");
	sleep(1.0); // wait a second.
/*
	printf("Cold booting the DSP\n");

	r = ioctl(dsp, OCIW_RESET);  // reset the whole board

	// r = ioctl(dsp, OCIW_RESET_ADDON); // just reset addon bus

	printf("Waiting for PLL to lock\n");
	for (j=0; j < SLEEP; j++) usleep(100000);
	return(dsp);
*/
}


int
dsp_clear_fifo(dsp)
{
	int r, icsr;
	u_short rbuf[1];

	int n=0;

	/* Clean out any stray words in read FIFO.  On /RESET, the DSP   */

	r = ioctl(dsp, OCIW_GET_ICSR , &icsr);
	
	while ( (r=read(dsp,rbuf,2)) > 0) {
		n++;
	}
	printf("read %d words from fifo!\n", n);
	return(dsp);
}


/*========== Set up and load the S-Records into the DSP =====================*/
int  
dsp_init(char *srec_file , int dsp)
{
	FILE  *fp;
	char srec[525], cmd[3];
	int	srec_count=0, retrys=0; 
	int	j,r, scan_index, byte_count, bc;
	int	checksum=0, nwords=0; 
	int num, addr, val, mem_space = 0;

	u_short wbuf[150], rbuf[1];
	u_char  *buf = (u_char *)wbuf;
	int nbyte = 2;

	/* Reset the DSP, wait for PLL lock and open the s-record file */
//	dsp_reset();

	/* Clear out any stray words in the read FIFO from DSP reset   */
	dsp_clear_fifo(dsp);
	
	if( (fp = fopen( srec_file, "r")) != NULL) {
		printf("Loading s-records from file: %s\n", srec_file);
	}
	else {
		printf("Could not open file: %s\n", srec_file);
		return(0);
	}

	while(fgets(srec,100,fp)!=NULL){ // read S file, 1 line, till newline char.
		if (srec[0] != 'S') continue; // first char must be S!!
		srec_count++;

		sscanf(srec,"%2s %2x",cmd,&num);  // first 2 chars into cmd. 
				// convert next 2 chars from ascii hex into a number.
				// num is number of chars in line.
		scan_index=4; // we are done with the first 4 chars.

		nwords = (num-3)/3; // subtract off 3 address bytes,
							// and 3 bytes per word.

		/* Scan the two byte (S0 or S1) or three byte (S2) address field */
		switch(srec[1]) {
		case '0': // S0 changes address mem_space..
			sscanf(&srec[scan_index],"%4x",&addr);
			scan_index+=4;
			mem_space = (addr&0xff);	
			break;
		case '1':
			sscanf(&srec[scan_index],"%4x",&addr);
			scan_index+=4;
			break;
		case '2': // S2 does not change mem_space!
			sscanf(&srec[scan_index],"%6x",&addr); 
			scan_index+=6;
			break;
		case '8':
			sscanf(&srec[scan_index],"%6x",&addr); 
			scan_index+=6;
			mem_space = 0x08;	
			break;
		case '9':
			sscanf(&srec[scan_index],"%4x",&addr); 
			mem_space = 0x08;	
			scan_index+=4;
			break;
		default:
			break;
		}

		// printf(" mem space: %x \n" , mem_space);

/*	 buf[0] = number of words, excluding first two header words            *
 *	    [1] = dummy byte, coded as zero                                    *
 *	    [2] = memory space code X=1, Y=2, P=4, END=8                       *
 *      [ ]                                                                *
 *	    [0] = start address (bits 8..0)                                    *
 *	    [1] = start address (bits 15..8)                                   *
 *	    [2] = start address (bits 23..16)                                  *
 *      [ ]                                                                */

		/* Arrange the outgoing bytes in the write buffer */
		bc = 0;										/* byte counter */		
		buf[bc++] = (u_char)(nwords>>0 & 0xff);		/* number of 24-bit words*/
		buf[bc++] = (u_char)(nwords>>8 & 0xff);	
		buf[bc++] = (u_char)(mem_space & 0xff);		/* memory space */
		if (nbyte==2) buf[bc++] = 0;

		buf[bc++] = (u_char)(addr>>0 & 0xff);		/* address bits  7..0 */
		buf[bc++] = (u_char)(addr>>8 & 0xff);		/* address bits 15..8 */
		buf[bc++] = (u_char)(addr>>16 & 0xff);		/* address bits 23..16 */
		if (nbyte==2) buf[bc++]=0;

		while (scan_index<=2*num){
			sscanf(&srec[scan_index],"%6x",&val);
			buf[bc++] = (val>>0 & 0xff);
			buf[bc++] = (val>>8 & 0xff);
			buf[bc++] = (val>>16 & 0xff);
			if (nbyte==2) buf[bc++]=0;
			scan_index+=6;
		}
		byte_count = bc;

		/* Calculate checksum */
		checksum = 0;
//		printf("write");
		for (j=0; j<byte_count/nbyte; j++)
		{
			 checksum += wbuf[j];
//			printf(" %x" , wbuf[j]);
		}
//		printf("bytes: %d \n", byte_count);

		checksum &= 0xffff;
		if (nbyte==1) checksum &= 0xff; 

		/* 	Write s-record to the dsp, and read back checksum */
		if (srec[1] == '1' || srec[1]=='2')	{	
			rbuf[0] = ~checksum;
			while (rbuf[0] != checksum)
			{
				write(dsp,wbuf,byte_count);
  
				while (retrys<MAX_RETRYS){
					if ((r=read(dsp,rbuf,nbyte)) > 0) break;
					retrys++;
					usleep(TIMEOUT);
					printf("f");
				}
//				printf("r=%d  rbuf=%04x  checksum=%04x  retrys=%3d\n",
//					 r, rbuf[0], checksum, retrys);
				if(retrys++ >= MAX_RETRYS) {
					printf("SCI comm retry failure\n");
					fclose(fp);
					return(-srec_count);
				}
 			}
			retrys=0;
		}
		/* write last record + wait */ 
 		if (srec[1] == '8' || srec[1] == '9')
		{
			nbyte=0;
			while ((r=read(dsp,rbuf,2)) > 0)
			{
				printf("rbuf: %04x\n",rbuf[0]);
				if(++nbyte == 10)
					break;
			}
			printf("about to start DSP\n");
			write(dsp,wbuf,byte_count);
			usleep(10*TIMEOUT);
		}

	}// end of read from file loop.

	nbyte=0;
	while ((r=read(dsp,rbuf,2)) > 0)
	{
		printf("rbuf: %04x\n",rbuf[0]);
		if(++nbyte == 10)
			break;
	}
	/* Clean up after the DSP as it restarts its peripherals */
	dsp_clear_fifo(dsp);

	if(srec_count == 0)
		printf("DSP NOT initialized, %d S-records processed.\n",srec_count);
	else
		printf("DSP initialized, %d S-records processed.\n",srec_count);

	fclose(fp);
	return(srec_count);

}

/*
int dsp_run()
{
	dsp_open();
	dsp_reset();
	dsp_init("srec/clk226c.s");
	dsp_close();
	return(0);
}
*/
