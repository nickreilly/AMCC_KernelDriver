/******************************************************************************
 *  pt.c -- Test program for the S5920 prototype board                        *
 *                                                                            *
 *  (c) 1998 Greg Burley                                                      *
 *                                                                            *
 *                                                                            *
 *  This program tests the S5920 prototype board using the standard           *
 *  read()/write()/ioctl() functions.                                         *
 *                                                                            *
 *  It uses the real driver ("/dev/ociw") and the PCI interface.              *
 *                                                                            *
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <asm/io.h>				/* for inl() and outl() */
#include <asm/types.h>

#include "ociw.h"

#define  SIZE		256

static int  ociw_fd = 0L;			/* device */

/*========== Main program ====================================================*/
int
main ()
{
	int r, w;
	u_int icsr, rcr, ptcr, mbef;
	u_int wb, rb;
	int  e=0,i,j;
	int  hist[16];
	u_short wbuf[SIZE], rbuf[SIZE];

    /* open the device */
	ociw_fd = open ("/etc/udev/devices/ociw0", O_RDWR);
	if (ociw_fd == -1){
		printf ("Error opening device /etc/udev/devices/ociw0 \n");
		exit(0);
	}
	else
		printf ("Device /etc/udev/devices/ociw0 opened\n");


	r = ioctl(ociw_fd, OCIW_RESET_ADDON);

	/* Scan the operation registers */
	r = ioctl(ociw_fd, OCIW_GET_ICSR , &icsr);
	r = ioctl(ociw_fd, OCIW_GET_RCR , &rcr);
	r = ioctl(ociw_fd, OCIW_GET_PTCR , &ptcr);
	r = ioctl(ociw_fd, OCIW_GET_MBEF , &mbef);
	printf("ICSR %08x  MBEF %08x  RCR %08x  PTCR %08x\n", 
			icsr, mbef, rcr, ptcr);

	while ( (r=read(ociw_fd,rbuf,2)) > 2) {
		printf("dsp_clear r=%02d  rbuf[0]=%04x \n",r,rbuf[0]);
	}

	r = ioctl(ociw_fd, OCIW_GET_ICSR , &icsr);
	r = ioctl(ociw_fd, OCIW_GET_MBEF , &mbef);
	printf("ICSR %08x  MBEF %08x\n", 
			icsr, mbef);

	/* Write, then read a small buffer */
	for (r=0; r<SIZE; r++){
		wbuf[r] = ((r & 0xff)<<0)|((r & 0xff)<<8);
		rbuf[r] = 0;
	}
	w = write(ociw_fd,wbuf,2*SIZE);
	r = read(ociw_fd,rbuf,2*SIZE);

	/* Print the read and write buffers (should be identical) */
	printf("\n");
	for (j=0; j<SIZE; j+=2){
		wb = *((u_int *)(&wbuf[j]));
		rb = *((u_int *)(&rbuf[j]));
		if (j<32)printf("wbuf %08x  rbuf %08x\n",wb, rb);
	}

	/* Check to see if read and write buffers are identical */
	for (j=0; j<SIZE; j++){
		if (wbuf[j] != rbuf[j]) {
			e++;
/*			if (e<10)printf("Error[%2d]  %04x  %04x \n",j,wbuf[j],rbuf[j]); */
		}
	}
	printf("w %04d  r %04d  errors %d\n",w,r,e);


	/* Make a histogram of bits received */
	for (j=0; j<16; j++) hist[j] = 0;
	for (i=0; i<SIZE; i++){
		for (j=0; j<16; j++){
			if (rbuf[i] & (0x1<<j)) hist[j]++;
		}
	}

	for (j=0; j<16; j++){
		printf("%4d ",hist[j]);
		if (j==7 || j==15)printf("\n");
	}


    /* close down */
	if (ociw_fd){
		close (ociw_fd);
		printf("Device /etc/udev/devices/ociw0 closed\n");
	}

    return EXIT_SUCCESS;	/* to make gcc happy */
}



