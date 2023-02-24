
/****************************************************************************/ 
/*     ociw.h -- OCIW S5920 PCI Target Interface driver                    */
/*                                                                          */
/*     This driver is shamelessly based on the code for the                 */
/*                                                                          */
/*     Simple AMCC S5933 PCI Matchmaker driver                              */
/*     (c) 1997 Andrea Cisternino  (acister@pcape1.pi.infn.it)              */
/*                                                                          */
/****************************************************************************/

//
//
//  Version:
//    $Id: ociw.h,v 1.5 2003/03/19 21:57:19 anonymous Exp $
//  Revisions:
//    $Log: ociw.h,v $
//    Revision 1.5  2003/03/19 21:57:19  anonymous
//    just changed some #defines to be clearer.
//
//    Revision 1.4  2003/03/10 23:28:17  dsp
//
//    added a few more #defines.
//
//
//
//
//
//  

#ifndef _OCIW_H_
#define _OCIW_H_

/****************************************************************************/
/* AMCC Operation Register Offsets                                          */
/****************************************************************************/

#define AMCC_OP_REG_OMB                 0x0C    /* Outgoing Mailbox register */
#define AMCC_OP_REG_IMB                 0x1C    /* Incoming Mailbox register */
#define AMCC_OP_REG_APTA                0x28    /* Add-on Pass-Thru address */
#define AMCC_OP_REG_APTD                0x2c    /* Add-on Pass-Thru data */
#define AMCC_OP_REG_MBEF                0x34    /* Mailbox Empty/Full register */
#define AMCC_OP_REG_ICSR                0x38    /* Interrupt Control/Status register*/
#define AMCC_OP_REG_RCR                 0x3c    /* Reset Control register */
#define AMCC_OP_REG_PTCR                0x60    /* Pass-Thru Configuration register */

#define AMCC_FIFO_DEPTH_DWORD   8               /* Internal S5920 FIFOs */
#define AMCC_FIFO_DEPTH_BYTES   (8 * sizeof (u32))

/****************************************************************************/
/* AMCC Operation Registers Size - PCI                                      */
/****************************************************************************/

#define AMCC_OP_REG_SIZE                128             /* in bytes */

/****************************************************************************/
/* AMCC Operation Registers Flags                                           */
/****************************************************************************/

/* PCI Reset Control/Status Register (RCR) */
#define RCR_NV_CTRL_MASK                0xffff0000
#define RCR_NV_ACC_MASK                 0xe0000000
#define RCR_NV_AD_MASK                  0x00ff0000
#define RCR_RESET_MBFLAGS               0x08000000
#define RCR_RESET_FIFO                  0x02000000
#define RCR_RESET_ADDON                 0x01000000

/* PCI Interrupt Control/Status Register (ICSR) */
#define ICSR_INT_STATUS_MASK    0x00ff0000
#define ICSR_INT_SEL_MASK               0x0000ffff
#define ICSR_INT_ASSERTED               0x00800000
#define ICSR_ADDON_INT                  0x00400000
#define ICSR_IMB_INT                    0x00020000
#define ICSR_OMB_INT                    0x00010000
#define ICSR_ADDON_ENABLE               0x00002000
#define ICSR_IMB_ENABLE                 0x00001000
#define ICSR_IMB_BYTE                   0x00000300
#define ICSR_OMB_ENABLE                 0x00000010
#define ICSR_OMB_BYTE                   0x00000003
#define ICSR_MB_INT_MASK                0x00001F1F

/* PCI Pass-thru Configuration register */
#define PTCR_PTADDR_ENABLE              0x00000080
#define PTCR_ENDIAN_CONVERT             0x00000040
#define PTCR_WR_FIFO_ENABLE             0x00000020
#define PTCR_RD_FIFO_ENABLE             0x00000018
#define PTCR_WAIT_STATES                0x00000007

/* Mailbox Empty/Full Status Register */
#define MBEF_IN_STATUS_MASK             0xf0000000
#define MBEF_OUT_STATUS_MASK    0x0000f000

/* Incoming Mailbox Flags -- Application specific */
#define IMB_TX_BUSY                             0x01000000
#define IMB_RX_RDY                              0x02000000
#define IMB_NOT_HF				0x04000000
#define IMB_NOT_FULL				0x08000000


// the MAGIC number keeps other clients from accidentally  using 
// this driver incorrectly.
#define OCIW_MAGIC 'z'

/*-- Ioctls ----------------------------------------------------------*/

/* general */
#define OCIW_RESET                      _IO(OCIW_MAGIC,  0)
#define OCIW_RESET_ADDON        _IO(OCIW_MAGIC,  1)
#define OCIW_RESET_FIFO         _IO(OCIW_MAGIC,  2)
#define OCIW_RESET_MBFLAGS      _IO(OCIW_MAGIC,  3)

#define OCIW_GET_ICSR           _IOR(OCIW_MAGIC, 4, __u32)
#define OCIW_GET_RCR            _IOR(OCIW_MAGIC, 5, __u32)
#define OCIW_GET_MBEF           _IOW(OCIW_MAGIC, 6, __u32)
#define OCIW_GET_PTCR           _IOR(OCIW_MAGIC, 7, __u32)

#define OCIW_SET_PTCR           _IOW(OCIW_MAGIC, 8, __u32)
#define OCIW_SET_P2ABUF         _IOW(OCIW_MAGIC, 9, u_long)
#define OCIW_SET_A2PBUF         _IOW(OCIW_MAGIC,10, u_long)

/* Mailboxes */
#define OCIW_READ_MB            _IOR(OCIW_MAGIC,11, __u32)
#define OCIW_WRITE_MB           _IOW(OCIW_MAGIC,12, __u32)
#define OCIW_SET_OMBIRQ         _IO(OCIW_MAGIC, 13)
#define OCIW_SET_IMBIRQ         _IO(OCIW_MAGIC, 14)
#define OCIW_CLR_MBIRQ          _IO(OCIW_MAGIC, 15)

/* Misc or User-specific */
#define OCIW_GET_IRQTIME        _IOR(OCIW_MAGIC, 16, long)
#define OCIW_TEST                       _IOWR(OCIW_MAGIC,17, long)
#define OCIW_INTS_ON		_IO(OCIW_MAGIC, 18)
#define OCIW_SET_NROWS		_IOW(OCIW_MAGIC, 19,u_short)
#define OCIW_SET_NCOLS		_IOW(OCIW_MAGIC, 20,u_short)
#define OCIW_SET_NSAMPS		_IOW(OCIW_MAGIC, 21,u_short)
#define OCIW_GET_DONE		_IOR(OCIW_MAGIC, 22,u_short)
#define OCIW_GET_FRAME		_IOR(OCIW_MAGIC, 23,u_short)
#define OCIW_SET_SAMPMODE	_IOW(OCIW_MAGIC, 23,u_short)
#define OCIW_SET_IMAGE		_IOW(OCIW_MAGIC, 24,u_short)
#define OCIW_GET_PHYSBUFSIZE    _IOR(OCIW_MAGIC, 25,long)
#define OCIW_GET_IMAGESIZE   _IOR(OCIW_MAGIC, 26,long)
#define OCIW_SET_IMAGEMODE  _IOW(OCIW_MAGIC, 27,int)
#define OCIW_INTS_OFF		_IO(OCIW_MAGIC, 28)


#endif  /* _OCIW_H_ */
