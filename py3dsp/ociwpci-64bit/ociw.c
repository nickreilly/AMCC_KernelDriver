/******************************************************************************/
/*	  OCIW.c -- S5920 PCI Target OCIW Camera Interface driver				  */
/*	                                                                          */
/*     This driver is shamelessly based on the code for the                   */
/*                                                                            */
/*     Simple AMCC S5933 PCI Matchmaker driver                                */
/*     (c) 1997 Andrea Cisternino  (acister@pcape1.pi.infn.it)                */
/*      modified for OCIW by Greg Burley  Sept 2000                           */
/*      modified for University of Rochester, by Drew Moore, July 2001        */
/*      modified for University of Rochester, by Lars Avery, July 2002        */
/*      set tab = 4 spaces                                                    */
/*                                                                            */
/*
 *
 *  Version:
 *    $Id: ociw.c,v 1.16 2004/05/11 02:09:28 drew Exp $
 *  Revisions:
 *    $Log: ociw.c,v $
 *    Revision 1.16  2004/05/11 02:09:28  drew
 *    added support for dropping pre-frame pixels.
 *    (could add post frame pixels too.)`
 *
 *    Revision 1.15  2004/02/23 20:59:27  dsp
 *    commented out some of the continuous yakking at device driver.
 *
 *    Revision 1.14  2003/12/29 04:05:17  drew
 *    removed some annoying printk statements that were flooding
 *    /var/log/messages.
 *    removing them all using #ifdef OCIW_DEBUG didnt work..
 *    first, it did't remove them (I did #if wrong)
 *    then it caused the module to crash. still need
 *    to figure that one out.
 *
 *    Revision 1.13  2003/12/01 22:20:28  drew
 *    mostly commented out debug stuff.
 *    can be conditionally included with
 *    compile flag.
 *    (IOCTL might have been better?)
 *
 *    Revision 1.12  2003/08/14 01:39:44  dsp
 *    bug fix, (issue 0003) phy_buf == 0 needs error return.
 *    also, remap_page_range needs extra arg for 2.4.20-8
 *
 *    Revision 1.11  2003/03/19 21:55:41  anonymous
 *    Major checkin of changes that were done at RIT.
 *    Improvements: dest buffers are fixed size.
 *    Memory mapping is straightened out.
 *    2 processes can open driver.
 *
 *    Revision 1.10  2003/02/16 19:14:23  anonymous
 *    no real change, just getting current.
 *    also added a header.
 *
 *
 */

//      -------- history from RIT development ---------
//      Revision 1.12  2002/11/24 15:40:01  drew
//      minor changes, not really a big difference.
//      Next changes: programmable offset?
//      different divisor than nsamps?
//      floating point?
//      generalised coefficients?
//      fixed destination buffer locations?
//
//      Revision 1.11  2002/10/29 07:51:53  drew
//      Bug was floating point exception when nrows and ncols equal to 0.
//
//      Revision 1.10  2002/10/05 11:57:38  drew
//
//      there was a bug when pixtogo didn't fall below 4096 and you
//      aborted the scan and then did coldboot - loadsrec.
//      The load_srec would hang.
//      explicitly clearing the int enable
//      at reset seems to have fixed this problem.
//      Not sure what was happening, but ints off at reset is pretty sensible.
//
//      Revision 1.9  2002/09/26 10:05:42  lars
//      Added ability to set image mode.  Can now select between sig-ped, sig, ped,
//      accumulated sig, or accumulated ped.
//
//      Revision 1.8  2002/08/27 09:59:42  lars
//      Added the wait queue, an interruptible sleep, and a wait
//
//      Revision 1.7  2002/08/12 15:23:32  lars
//      removed a double mmap call to fix segmentation fault
//
//      Revision 1.6  2002/08/01 12:14:46  lars
//      Client can now get size of their dest image using OCIW_GET_MMAPSIZE
//      This size will be used when the client changes the row,col,and samples.
//      Also the CopyFowler code was updated to copy to phy_buf.
//
//      Revision 1.5  2002/07/29 15:24:08  lars
//      Added numerous printk statements for debugging.
//      Look for lines with the word remove after it to find printk lines added.
//
//      Revision 1.4  2002/07/17 16:14:46  lars
//      Only allow 1 person to create.  Still has m-unmapping problem on close
//
//      Revision 1.3  2002/07/15 10:54:16  lars
//      Fixed commenting in file header
//
//      Revision 1.2  2002/07/15 10:45:49  lars
//      Added a CVS header
//                                                              
/*                                                                            */
/******************************************************************************/

/*------------------------------------------------------------------------------
 * 
 *  Driver main file -- this file contains the driver code.
 *
 *----------------------------------------------------------------------------*/
#define OCIW_DEBUG 0
#define KERNVER2_4	1  // use better way to compile for different kernels??

#ifndef __KERNEL__
# define __KERNEL__
#endif

#include <linux/config.h>					   /* for CONFIG_PCI */

#ifdef MODULE
# include <linux/module.h>
# include <linux/version.h>
#else
# define MOD_INC_USE_COUNT
# define MOD_DEC_USE_COUNT
#endif

#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
//#include <linux/vmalloc.h> // acm, for private buffer.. ..not used anymore.
#include <linux/pci.h>
#include <linux/ioport.h>
#include <linux/stat.h>
#include <linux/delay.h>
// #include <linux/tqueue.h> // acm, for waking up a sleeping user. not used.
#include <asm/io.h>							 /* for inb(), outb() etc. */
#include <asm/system.h>
#include <asm/pgtable.h>
#include <asm/uaccess.h>
#include <asm/string.h> // acm, for memcpy
#include <asm/irq.h> // acm, for enable_irq

#include "ociw.h"							   /* S5920 definitions */

// these variables are exposed to the outside world
// and can be changed at load time to configure memory.
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
MODULE_PARM(ociw_physbuf,"l"); // location of physical buffer start
MODULE_PARM(ociw_physbufsize,"l"); // size of physical buffer in bytes
MODULE_PARM(ociw_invertvid,"i"); //  flip the sign of video?
MODULE_PARM(ociw_readstrategy,"i"); // slow, med, fast read code..
MODULE_PARM(ociw_maxpix,"l"); // dest buffers, max size in pixels.
MODULE_PARM(ociw_preframepix,"i"); // number of ignoreable pre-frame pixels
MODULE_PARM(ociw_region2pix,"i"); // number pixels in region 2 (block copy)

/*-- Defines ---------------------------------------------------------*/

#define OCIW_NAME			"ociw"
#define OCIW_RW_TIMEOUT		(HZ * 5)		/* 5 sec. */
#define OCIW_TIMEOUT		50		  /* timeout in microseconds */

/* lock bits for concurrent access */
#define OCIW_P2A_LOCK_BIT	   0
#define OCIW_A2P_LOCK_BIT	   1
#define OCIW_TEST_IRQ_BIT	   2

/* Miscellaneous */
#define OCIW_POLL_DELAY		 20UL	/* in usecs */

/* return codes for interrupts, unused */
/*
#define OCIW_IRQ_READ_OK		1
#define OCIW_IRQ_WRITE_OK		2
#define OCIW_IRQ_MASTER_ABORT   3
#define OCIW_IRQ_TARGET_ABORT   4
#define OCIW_IRQ_OMB_EMPTY		5
#define OCIW_IRQ_IMB_FULL		6
*/

#define PCI_DEVICE_ID_AMCC_S5920	0x5920

#define OCIW_MAJOR				  125		 /* Major device number requested */
#define OCIW_BUFFER_SIZE	16384

#define MIN(a,b)			(((a) < (b)) ? (a) : (b))

/* Different Imaging Modes */
#define	SIGMINUSPED 0
#define	SIG			1
#define	PED			2
#define	ACCSIG		3
#define	ACCPED		4


/*-- Structures ------------------------------------------------------*/

/*  Pass-thru info -- holds information about a single pass-thru region. */
struct s5920_pt {
	u_int	   size;		   /* dimensions */
	u_int	   phys_addr;		  /* real address */
	char	   *virt_addr;		  /* after ioremap call */
};

/* Hardware info -- holds global data for a single PCI board. */
struct ociw_s5920 {
	struct pci_dev *pcidev;
	int	 major;
	u_long	  op_regs;		/* this is BADDR[0] */
	int	 op_regs_type;   /* MEM or I/O operation registers */
	u_char	  irq;			/* interrupt number */
	u_char	  ptnum;		  /* number of pass-thru regions */
	struct s5920_pt pts[5];		 /* pass-thru regions */
	int	 timeout;		/* timeout for async waits */
};

/* Buffer info -- holds data for Pass-Thru buffer handling. */
struct ociw_buffer {
	volatile int   irq_code;
	volatile u_int copy_size;
	volatile u_int bytes_done;
	volatile u_int size;
	u_int  async;
	char   *buf;
};

// might be able to eliminate ociw_buffer


// we allocate physical memory that lilo.conf reserves for us. append="mem=xx"
long ociw_physbuf = 0x6000000; // address of physical memory we can grab
long ociw_physbufsize = 0x2000000; // size of physical memory we can use
int ociw_readstrategy = 1; // 1 is a medium speed strategy. 0=slow, 2=fast
int ociw_invertvid =  0 ; //
int ociw_maxpix =  0x100000 ; // 1024 * 1024
int ociw_preframepix = 0; // number of pixels to toss at the start of each frame.
int ociw_region2pix = 4096; // number pixels in region 2 (block copy)

// next line is not like above 5 (which are module parameters)
int ociw_imagemode = SIGMINUSPED; // initialize CopyFowler mode 

/*------------------------------------------------------------------------------
 *
 *  Static variables.
 *  These can be seen by every process using the device.
 *
 *----------------------------------------------------------------------------*/

struct ociw_s5920 ociw_dev;				 /* General data */

struct ociw_buffer p2a;		 /* P2A buffer control struct */
struct ociw_buffer a2p;		 /* A2P buffer control struct */

static u_long lock = 0UL;				   /* generic lock dword */

static u32  badrs[6] =
{
	PCI_BASE_ADDRESS_0,				 /* PCI bus operations register */
	PCI_BASE_ADDRESS_1,
	PCI_BASE_ADDRESS_2,
	PCI_BASE_ADDRESS_3,
	PCI_BASE_ADDRESS_4,
	PCI_BASE_ADDRESS_5,				 /* not used in S5920 */
};

// our destination images are all 16 bits, we put them first.
// user specifies how many of these there are.
// our pedestal and signal images are 32 bits each. they go next.
// the raw images are also 16 bits each.
// we put them last, and fill our physical memory with them.
static short *phy_buf; // pointer to physical memory block, and first dest image
static long *pedstart; // the first pixel in the fowler pedestal image block.
static long *sigstart; // the first pixel in the fowler signal image block.
static short *rawbuf; // pointer to first of NBUFS raw frames.
static short nrows = 1024; // number of rows in image
static short ncols = 1024; // number of column in image
static short nimages = 2; // number of destination images. (src and bkg.)
static short nsamps = 1; // number of sigs and/or peds (nsamps = nsigs = npeds)
static short sampmode = 8; // sample mode, 8 = Fowler. Not used.
static short imagenum = 0; // which destination image. 0 <= imagenum < nimages


static short ociw_count = 0;
// interrupt routine puts images into one of several input buffers.
// which are contiguous in memory.
// each input buffer has a base location, current location
// and a pix to go before it is filled.
// When a buffer is filled, the interrupt routine needs to
// go to a different buffer, and wake up the bottom half so that
// it can empty the current buffer..

static long NBUFS = 10; // 20 Mb for raw images, 12 Mb for fowler image
static short volatile latest_buf;  //  which buffer interrupt routine owns.
//static short bhstop;  //  which buffer bottom half should stop at.
static short bhstart;  //  which buffer bottom half should start at.
static short *curbuf; // next empty spot in buffer where int routine writes to.
static long bufpixtogo; // number of pixels till input buffer fills.

// variables related to current state of fowler averaging.
// static short currow; // what row we are at now (0 to nrows-1)
// static short curcol; // what column we are at now.
static short cursamp; // which frame we are in (0 to nsamps-1)
static short dosigs; // starts at 0 for peds, goes to 1 for sigs.

// more like a global flag..
static long pixtogo; // total real pixels to go. bottom half owns it..
static long rawpixtogo; // total pixels including prepixels to go.
						// fifoIntoBuf owns it.
static long we_wrapped;
static struct tq_struct ociw_task;

//Blocking info
wait_queue_head_t frameq;
DECLARE_WAIT_QUEUE_HEAD(frameq);

wait_queue_head_t wq;
DECLARE_WAIT_QUEUE_HEAD(wq);

/*------------------------------------------------------------------------------
 * 
 *  Flags.
 *  These variables can be overridden at load time by the insmod cmd.
 *
 *----------------------------------------------------------------------------*/

int vid = PCI_VENDOR_ID_AMCC;
int did = PCI_DEVICE_ID_AMCC_S5920;

/*------------------------------------------------------------------------------
 * 
 *  Function prototypes
 *
 *----------------------------------------------------------------------------*/

int   init_module (void);
void  cleanup_module (void);

static ssize_t ociw_read (struct file *, char *, size_t, loff_t *);
static ssize_t ociw_write (struct file *, const char *, size_t, loff_t *);
static int ociw_ioctl (struct inode *, struct file *, unsigned int, 
	unsigned long);
static int ociw_open (struct inode *, struct file *);
static int ociw_release (struct inode *, struct file *);
static int ociw_mmap (/*struct inode *inode,*/ struct file *file, struct vm_area_struct *vma);

static int  ociw_init_dev (struct ociw_s5920 *, struct pci_dev *);
static void ociw_free_dev (void);

static void ociw_irq_handler (int, void *, struct pt_regs *);
static void ociw_bottom_half(void*);


/*------------------------------------------------------------------------------
 *
 *  Linux kernel main entry point.
   For a definition of its fields see <linux/fs.h>.
 *
 *----------------------------------------------------------------------------*/

static struct file_operations ociw_fops =
{
	open:	   ociw_open,
	release:	ociw_release,
	read:	   ociw_read,
	write:	  ociw_write,
	ioctl:	  ociw_ioctl,
	mmap:			ociw_mmap,
};

/*-----------------------------------------------------------------------------
 * 
 *  Inline Functions
 *
 *----------------------------------------------------------------------------*/

extern inline __u32
ociw_read_opreg (int reg)
{
	extern struct ociw_s5920 ociw_dev;
	#ifdef OCIW_DEBUG
	// printk(KERN_WARNING"ociw: ociw_read_opreg\n");//remove
	#endif

	if (ociw_dev.op_regs_type == PCI_BASE_ADDRESS_SPACE_IO)
		return ((__u32) inl ((unsigned short) (ociw_dev.op_regs + reg)));
	else
		return (readl ((ociw_dev.op_regs + reg)));
}

extern inline void
ociw_write_opreg (__u32 value, int reg)
{
	extern struct ociw_s5920 ociw_dev;
#ifdef OCIW_DEBUG
	// printk(KERN_WARNING"ociw: ociw_write_opreg\n");//remove
#endif

	if (ociw_dev.op_regs_type == PCI_BASE_ADDRESS_SPACE_IO)
		outl (value, (unsigned short) (ociw_dev.op_regs + reg));
	else
		writel (value, (ociw_dev.op_regs + reg));
}

void hf_ints()
{
#ifdef OCIW_DEBUG
	// printk(KERN_WARNING"ociw: ociw_hf_ints\n");//remove
#endif

	writel(0xffffffffL,(u32 *)(ociw_dev.pts[2]).virt_addr); // enable HF flag ints
}

void ef_ints()
{
#ifdef OCIW_DEBUG
	printk(KERN_WARNING"ociw: ociw_ef_ints\n");//remove
#endif

	writel(0,(u32 *)(ociw_dev.pts[2]).virt_addr); // enable HF flag ints
}


// before we start reading pixels we need to do this stuff..
/*
void setup_ints()
{
	hf_ints();

	ociw_write_opreg ((u32) 0, AMCC_OP_REG_ICSR);
	// printk (KERN_WARNING "ociw_init: ICSR: 0x%08lx\n",
			// ociw_read_opreg (AMCC_OP_REG_ICSR));
	ociw_write_opreg ((u32) ICSR_ADDON_ENABLE,  AMCC_OP_REG_ICSR);
}
*/
/*------------------------------------------------------------------------------
 * 
 *  Standard module initialization function.
 *  This function scans the PCI bus looking for the right board. Support
 *  functions can be found in file support.c
 *
 *----------------------------------------------------------------------------*/
int
init_module (void)
{

	extern struct ociw_s5920 ociw_dev;

	extern struct ociw_buffer p2a;
	extern struct ociw_buffer a2p;

	struct pci_dev *pcidev = NULL;
	int  result = 0L;

	printk (KERN_WARNING "ociw_init >>> START <<<\n");
	printk(KERN_WARNING"ociw: ociw_init_module\n");//remove

	memset (&ociw_dev, 0, sizeof (ociw_dev));
	memset (&p2a, 0, sizeof (p2a));
	memset (&a2p, 0, sizeof (a2p));

	if (!pci_present())
		return(-ENODEV);

	if ((pcidev = pci_find_device((u_short)vid, (u_short)did, pcidev))){

		result = register_chrdev(OCIW_MAJOR, OCIW_NAME, &ociw_fops);
		if (result == 0)
			ociw_dev.major = OCIW_MAJOR;
		else
			ociw_dev.major = result;
		printk (KERN_WARNING "ociw_dev.major = %d\n", ociw_dev.major);

		if (ociw_dev.major == -EBUSY){
			printk (KERN_WARNING "ociw: unable to get major number\n");
			return (-EIO);
		}
#ifdef OCIW_DEBUG
		printk (KERN_WARNING "pcidev->resource[0].flags = %08lx \n", 
						pcidev->resource[0].flags);
#endif
		ociw_dev.pcidev = pcidev;
		ociw_dev.timeout = OCIW_RW_TIMEOUT;
		result = ociw_init_dev(&ociw_dev, pcidev);
	}

#ifdef OCIW_DEBUG
	printk (KERN_INFO "ociw: OCIW S5920 Rev 0.5 \n");
#endif
	return (result);
}

/*------------------------------------------------------------------------------
 * 
 *  Standard module release function.
 *
 *----------------------------------------------------------------------------*/

void
cleanup_module (void)
{
#ifdef OCIW_DEBUG
	printk(KERN_WARNING"ociw: ociw_cleanupModule\n");//remove
#endif

	ociw_free_dev ();
	printk (KERN_WARNING "\nociw: >>> STOP <<<\n");
}

/*------------------------------------------------------------------------------
 * 
 *  Standard open() entry point.
 *  It simply increments the module usage count.
 *
 *----------------------------------------------------------------------------*/

static int
ociw_open (struct inode *inode, struct file *file)
{
#ifdef OCIW_DEBUG
	printk (KERN_WARNING "ociw: open\n");//remove
#endif

	/* Get the minor device number in case there is more than one  *
	 * physical device using the driver				*/

#ifdef OCIW_DEBUG
	printk (KERN_WARNING "Device minor number %d.%d\n", 
		inode->i_rdev >>8, inode->i_rdev & 0xff);
#endif

	/* Do some access control here. Only allow 2 devices open */ 

	if (ociw_count > 1) return -ENODEV; /* 1 device only */
	
	MOD_INC_USE_COUNT;
	ociw_count++;
	return (0);
}

/*------------------------------------------------------------------------------
 * 
 *  Standard release() entry point.
 *  This function is called by the close() system call.
 *
 *----------------------------------------------------------------------------*/

static int
ociw_release (struct inode *inode, struct file *file)
{
#ifdef OCIW_DEBUG
	printk (KERN_WARNING "ociw: close\n");
#endif
	MOD_DEC_USE_COUNT;
	ociw_count--;
	return(0);
}

/*------------------------------------------------------------------------------
 * 
 *  Standard read() entry point.
 *  It is used to read from the Add-on bus to PCI bus. 
 *  It fills a kernel buffer with the specified number of bytes.
 *
 *  Blocking + Non-Blocking I/O options need to be worked on.
 *  The read() code is hardware specific.
 *
 *----------------------------------------------------------------------------*/

// struct wait_queue *wq = NULL;

static ssize_t 
ociw_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{

		extern struct ociw_s5920 ociw_dev;
        extern struct ociw_buffer a2p;
        ssize_t  total_bytes_read = 0;
        u32 *ptr, *pcnt;
        u16 *rbuf;
        int nwords;
		int j;
		
#ifdef OCIW_DEBUG
		// printk(KERN_WARNING"ociw: ociw_read\n");//remove
#endif

        if (count == 0)
                return (0);

        /* Is the Pass_Thru + FIFO available? */
        if (test_and_set_bit (OCIW_A2P_LOCK_BIT, &lock)) {
#ifdef OCIW_DEBUG
                printk (KERN_WARNING "ociw: Read PT+FIFO already locked\n");
#endif
                return (-EBUSY);
        }

        /* Set the async flag correctly -- we don't do much with this yet. *
     * Eventually we will implement blocking + non-blocking mode.      */
        if (file->f_flags & O_NONBLOCK)
                a2p.async = 1L;
        else
                a2p.async = 0L;

        /* Set up to read the device into the a2p buffer */
        ptr = (u32 *)(ociw_dev.pts[1]).virt_addr;
        pcnt = (u32 *)(ociw_dev.pts[2]).virt_addr;
        rbuf = (u16 *)a2p.buf;
        nwords = 0;

        a2p.bytes_done = 0;
        a2p.copy_size = MIN (count, a2p.size);

/*  Routine 1 -- Read until done, or until the FIFO goes empty. */
/*  If the FIFO is not ready, return immediately.               */
	for (j=0; j<a2p.copy_size/2; j++) {
            if (!(ociw_read_opreg (AMCC_OP_REG_IMB) & IMB_RX_RDY))
                    break;
            rbuf[j] = (u16) readl ((u32 *)(&ptr[j]));
            a2p.bytes_done +=2;
    }

    if(copy_to_user(buf, a2p.buf, a2p.bytes_done))
            return(-EFAULT);
    total_bytes_read += a2p.bytes_done;
    count -= a2p.bytes_done;

    clear_bit (OCIW_A2P_LOCK_BIT, &lock);
    return (total_bytes_read);
}
//

/*------------------------------------------------------------------------------
 * 
 *  Standard write() entry point.
 *  It is used to write to the PCI-to-Add-on FIFO. It copies the user
 *  buffer into our (smaller) kernel buffer and then performs a transfer.
 *  The write() code is hardware specific.
 *----------------------------------------------------------------------------*/
static ssize_t 
ociw_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	extern struct ociw_s5920 ociw_dev;
	extern struct ociw_buffer p2a;
	ssize_t total_bytes_written = 0;
	u32 *ptr;
	u16 *wbuf;
	int j,k;

#ifdef OCIW_DEBUG
	// printk(KERN_WARNING"ociw: ociw_write\n");//remove
#endif

	/* Is the Pass-Thru + FIFO available? */
	if (test_and_set_bit (OCIW_P2A_LOCK_BIT, &lock)) {
#ifdef OCIW_DEBUG
		printk (KERN_WARNING "ociw_write: Pass_Thru + FIFO already locked\n");
#endif
		return (-EBUSY);
	}				   

	/* Set the async flag correctly -- we don't do much with this yet. *
	 * Someday we will implement blocking + non-blocking mode.	 */
	if (file->f_flags & O_NONBLOCK)
		p2a.async = 1L;
	else
		p2a.async = 0L;

	/* Can we check to see if the Pass-Thru + FIFO is empty? */

	/* Set up to write to the device from the p2a buffer */
	ptr = (u32 *)(ociw_dev.pts[1]).virt_addr;
	wbuf = (u16 *)p2a.buf;

	/* Start write -- after each write, wait for EPLD to transmit the data  */
	/* ie wait for hardware flag to set, then check flag before each write  */
	/* to prevent overwriting the previous data as it is being transmitted  */
	while (count > 0){
		p2a.bytes_done = 0;
		p2a.copy_size = MIN (count, p2a.size);
		if (copy_from_user(p2a.buf, buf, p2a.copy_size))
			return (-EFAULT);
		for (j=0; j<p2a.copy_size/2; j++){
			for (k=0; k < OCIW_TIMEOUT; k++){
				if ( !(ociw_read_opreg(AMCC_OP_REG_IMB) & IMB_TX_BUSY))
					break;
				udelay(1);
			}
			writel ( (u32)wbuf[j], (u32 *)(&ptr[j]) );
			udelay(5);
			p2a.bytes_done += 2;
		}
		count -= p2a.bytes_done;
		total_bytes_written += p2a.bytes_done;
	}
	clear_bit (OCIW_P2A_LOCK_BIT, &lock);

	return (total_bytes_written);
}


void clearpix();
void emptyFifo();

/*------------------------------------------------------------------------------
 * 
 *  Standard ioctl() entry point.
 *
 *----------------------------------------------------------------------------*/
static int
ociw_ioctl (struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
/*	  extern struct ociw_s5920 ociw_dev; */
	extern u_long lock;
	int ret = 0L;
	int j;
#ifdef OCIW_DEBUG
	// printk(KERN_WARNING"ociw: ociw_ioctl\n");
#endif

	switch (cmd) {
	case OCIW_RESET:			/* reset the whole board */
		ociw_write_opreg (RCR_RESET_MBFLAGS, AMCC_OP_REG_RCR);
		ociw_write_opreg (RCR_RESET_ADDON, AMCC_OP_REG_RCR);
		udelay(1000);
		ociw_write_opreg ((u32) 0, AMCC_OP_REG_RCR);
		ociw_write_opreg ((u32) 0, AMCC_OP_REG_ICSR); // no ints yet
		lock = 0L;
		break;

	case OCIW_RESET_ADDON:	  /* reset the Add-on bus */
		ociw_write_opreg (RCR_RESET_ADDON, AMCC_OP_REG_RCR);
		udelay(1000);
		ociw_write_opreg ((u32) 0, AMCC_OP_REG_RCR);
		ociw_write_opreg ((u32) 0, AMCC_OP_REG_ICSR); // no ints yet
		lock = 0L;
		break;

	case OCIW_RESET_FIFO:	   /* reset the PCI to Add-On internal FIFO */
		ociw_write_opreg (RCR_RESET_FIFO, AMCC_OP_REG_RCR);
		lock = 0L;
		break;

	case OCIW_RESET_MBFLAGS:	/* reset the maibox flags */
		ociw_write_opreg (RCR_RESET_MBFLAGS, AMCC_OP_REG_RCR);
		break;

	case OCIW_GET_ICSR:			 /* read ICSR register */
		{
		u32  *addr = (u32 *) arg;

		ret = put_user (ociw_read_opreg (AMCC_OP_REG_ICSR), addr);
		}
		break;

	case OCIW_GET_RCR:			  /* read RCR register */
		{
		u32  *addr = (u32 *) arg;

		ret = put_user (ociw_read_opreg (AMCC_OP_REG_RCR), addr);
		}
		break;


	case OCIW_GET_PTCR:			 /* read PTCR register */
		{
		u32  *addr = (u32 *) arg;

		ret = put_user (ociw_read_opreg (AMCC_OP_REG_PTCR), addr);
		}
		break;

	case OCIW_GET_MBEF:			 /* read mailbox MBEF register */
		{
		u32  *addr = (u32 *) arg;

		ret = put_user (ociw_read_opreg (AMCC_OP_REG_MBEF), addr);
		if (ret == -EFAULT) return (ret);
		}
		break;


	case OCIW_SET_PTCR:			 /* write to PTCR register */
		{

		u32  ptcr;

		ret = get_user (ptcr, (u32 *) arg);
		if (ret == -EFAULT) 
			return (ret);
		ociw_write_opreg (ptcr, AMCC_OP_REG_PTCR);
		}
		break;

	case OCIW_SET_P2ABUF:	   /* change P2A buffer size */
		ret = -EINVAL;
		break;

	case OCIW_SET_A2PBUF:	   /* change A2P buffer size */
		ret = -EINVAL;
		break;

	case OCIW_WRITE_MB:			 /* write to mailbox */
		ret = -EINVAL;
		break;

	case OCIW_READ_MB:			  /* read from mailbox */
		ret = -EINVAL;
		break;

	case OCIW_SET_OMBIRQ:	   /* set IRQ on OMB empty */
		ret = -EINVAL;
		break;
 
	case OCIW_SET_IMBIRQ:	   /* set IRQ on IMB full */
		ret = -EINVAL;
		break;

	case OCIW_CLR_MBIRQ:		/* clear MB IRQ flags */
		ret = -EINVAL;
		break;

	case OCIW_TEST:			 /* miscellaneous tests */
		{
		u32 *ptr, *pcnt, tmp, imb;
		u32 buf[1];

		buf[0] = 0xaabb5522;
		tmp = 0;

		ptr = (u32 *)(ociw_dev.pts[1]).virt_addr;
		pcnt = (u32 *)(ociw_dev.pts[2]).virt_addr;

		imb = ociw_read_opreg (AMCC_OP_REG_IMB);
#ifdef OCIW_DEBUG
		printk (KERN_WARNING "ociw_tmp  imb %08x\n ", imb);
		printk (KERN_WARNING "ociw_tmp  rxf %08x\n ", (IMB_TX_BUSY & imb));
#endif

		writel ( (u32)buf[0], (u32 *)(ptr));
		udelay(1);

		imb = ociw_read_opreg (AMCC_OP_REG_IMB);
#ifdef OCIW_DEBUG
		printk (KERN_WARNING "ociw_tmp  imb %08x\n ", imb);
		printk (KERN_WARNING "ociw_tmp  rxf %08x\n ", (IMB_TX_BUSY & imb));
#endif

		}
		break;
	case OCIW_INTS_ON:			/* turn on interrupts */
		// (start a new frame acquisition is more like it.)
#ifdef OCIW_DEBUG
		// printk(KERN_WARNING"Int on called\n");
#endif
		// first, compute an appropriate nbufs.
		// divide phys buffer size (in bytes)
		//  by size of image (in bytes) to get nbufs.
		if (nrows == 0 || ncols == 0) // no divide by zero please!!
			break; 
        // subtract off the destination images
        // and the 32 bit fowler signal and pedestals..
        // each is fixed in size at ociw_maxpix.
        // divide by number of bytes in current image...
		NBUFS = (ociw_physbufsize - 2*(4+nimages)*ociw_maxpix)
					/(2*(ociw_preframepix+nrows*ncols));
		// first nimage locations are destination buffers.
		// and fowler image, which comes next,
		// is twice as big as a regular image
		// since it is 32 bits, not 16 bits..
		// NBUFS -= (nimages + 4); // subtract off dest and fowler bufs
		//NBUFS -=1 ; // and some more room for good measure!
		// 4 in line above is 2 buffers (ped & sig) which are 
		// twice as big as ordinary buffers (they are 32 bits)
		// NBUFS better be 2 or greater!
		pedstart = (long *)(&phy_buf[nimages*ociw_maxpix]);
		sigstart = &pedstart[ociw_maxpix]; // pedstart points to long!
		rawbuf = (short *)&sigstart[ociw_maxpix] ; // sigstart, too.
		// clear the pixels in the fowler buffer.  
		// actually, bottom half handles this.
		// clearpix(); // needs pedstart and sigstart to run properly!

		ociw_write_opreg (0,  AMCC_OP_REG_ICSR); // Needed?
		emptyFifo(); // make sure no pix in FIFO. // added as debug 
		hf_ints(); /* half full flag interrupts */

		// clear the current frame state varibles.
		cursamp=0; // we have no samples yet.
		dosigs=0; // we are doing peds first.
		latest_buf=0; // start at the first buffer.
		bhstart=0; // start at the top.
		we_wrapped = 0;	// clear the error flag
		curbuf=&rawbuf[0]; // init the current buffer.
		bufpixtogo = (ociw_preframepix + nrows*ncols);
					// size of buffer might be handy global.
		pixtogo = 2*nsamps*nrows*ncols; // how we know when to stop.
		rawpixtogo = 2*nsamps*(ociw_preframepix+nrows*ncols); 
								// how we know when to stop.
#ifdef OCIW_DEBUG
		// printk(KERN_WARNING" ints about to go on\n");
#endif
		ociw_write_opreg ((u32) ICSR_ADDON_ENABLE,  AMCC_OP_REG_ICSR);
		lock = 0L;
		break;

	case OCIW_SET_NROWS:
		ret = get_user (nrows, (short *) arg);
		if (nrows <= 0 )
			nrows = 1 ; // can't set to zero!
#ifdef OCIW_DEBUG
		// printk(KERN_WARNING"nrows %d", nrows);
#endif
		if (ret == -EFAULT) 
			return (ret);
		break;
	case OCIW_SET_NCOLS:
		ret = get_user (ncols, (short *) arg);
		if (ncols <= 0 )
			ncols = 1 ; // can't set to zero!
#ifdef OCIW_DEBUG
		// printk(KERN_WARNING"ncols %d", ncols);
#endif
		if (ret == -EFAULT) 
			return (ret);
		break;
	case OCIW_SET_NSAMPS:
		ret = get_user (nsamps, (short *) arg);
		if (nsamps <= 0 )
			nsamps = 1 ; // can't set to zero!
#ifdef OCIW_DEBUG
		// printk(KERN_WARNING"nsamps %d", nsamps);
#endif
		if (ret == -EFAULT) 
			return (ret);
		break;
	case OCIW_GET_DONE:
		{
		u32  *addr = (u32 *) arg;
			
		// user wants to know how many pix to go.
		// if there is no hope of a HF int.
		// and we are still waiting..
		if(rawpixtogo<=ociw_region2pix && rawpixtogo >0) 
		{
			// if there are pixels in the fifo, read them.
#ifdef OCIW_DEBUG
			// printk(KERN_WARNING"manually empty fifo!\n");
#endif
			while((ociw_read_opreg (AMCC_OP_REG_IMB) & IMB_RX_RDY))
			{
				// we cannot easily use the OCIW FIFO
				// if we want to use the empty flag. OCIW reads ahead.
				// so copy from single region instead.
		   		*curbuf++ = (short)readl((u32*)(ociw_dev.pts[1]).virt_addr);

				bufpixtogo--; // that's one leSs pixel to go in this buff.`
				if(!bufpixtogo) // no more pixels in this buffer??
				{
					bufpixtogo = ociw_preframepix + nrows*ncols; 
								// we're in the next buffer
					latest_buf++;  // so move our buffer pointer ahead..
					if (latest_buf == NBUFS)
					{
						latest_buf = 0;
						curbuf = rawbuf;
					}
				}
				rawpixtogo--;	// thats one less pixel overall, too.
				if(!rawpixtogo) // if there are no more pixels.
				{
					// tell the bottom half to run
#ifdef OCIW_DEBUG
			//		printk(KERN_WARNING"queue up BH for last time!\n");
			//		printk(KERN_WARNING" latest_buf %d\n",latest_buf);
#endif
					//ociw_bottom_half(NULL);
					queue_task(&ociw_task, &tq_immediate);
					mark_bh(IMMEDIATE_BH);
					break;
				}
			}
		}
		else
		{
			// either rawpixtogo is zero already
			// (last int could have hit it exactly..)
			// or we have a while to go.
			// put user on snooze for some data
			if (pixtogo > 0)
				interruptible_sleep_on_timeout(&wq,1);
		}	
		if(we_wrapped)
		{
			we_wrapped = 0; // clear the flag
			ret = put_user(-1,addr);
		}
		else if (pixtogo == 0 )
		{
			if((ociw_read_opreg (AMCC_OP_REG_IMB) & IMB_RX_RDY))
				ret = put_user(-2,addr); // too many pixels!
			else
				ret = put_user(pixtogo,addr); // just enough pixels
		}
		else
			ret = put_user(pixtogo,addr);
		if (ret == -EFAULT)
			return (ret);
		}
		break;
	case OCIW_SET_IMAGE: // tell which image the fowler image is copied to
		ret = get_user (imagenum, (short *) arg);
		if (ret == -EFAULT) 
			return (ret);
        if (imagenum >= nimages)
                imagenum = nimages-1;
        if (imagenum < 0)
                imagenum = 0;
		break;
	case OCIW_SET_SAMPMODE:
		ret = get_user (sampmode, (short *) arg);
		if (ret == -EFAULT) 
			return (ret);
		break;
	case OCIW_GET_PHYSBUFSIZE:
		{
		u32  *addr = (u32 *) arg;
		ret = put_user (ociw_physbufsize, addr);
		}
#ifdef OCIW_DEBUG
		printk(KERN_WARNING"ociw_GET_BUFSIZE:\n"); // remove
		printk(KERN_WARNING"phy_buf 0x%08lx \n", phy_buf);
		printk(KERN_WARNING"ociw_physbuf 0x%08lx \n", ociw_physbuf);
		printk(KERN_WARNING"buf size 0x%08lx \n", ociw_physbufsize);
#endif
		break;
	case OCIW_GET_IMAGESIZE:
		{
		u32 *addr = (u32 *) arg;
		ret = put_user(ociw_maxpix*2, addr);
		}
#ifdef OCIW_DEBUG
		printk(KERN_WARNING"ociw_GET_MMAPSIZE:\n"); // remove
		printk(KERN_WARNING"bytesperbuf 0x%08lx \n", nrows*ncols*2);
#endif
		break;
	case OCIW_SET_IMAGEMODE:
		ret = get_user(ociw_imagemode, (int *)arg);
		if (ret == -EFAULT)
			return (ret);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return (ret);
}


/*------------------------------------------------------------------------------
 * 
 *  Standard mmap() entry point.
 *  Very simple for the moment.
 *
 *----------------------------------------------------------------------------*/

void simple_vma_open(struct vm_area_struct *area)
{
    MOD_INC_USE_COUNT;
}

void simple_vma_close(struct vm_area_struct *area)
{
    MOD_DEC_USE_COUNT;
}

static struct vm_operations_struct simple_vm_ops = {
        simple_vma_open,
        simple_vma_close
};


static int
ociw_mmap (struct file *file, struct vm_area_struct *vma)
{
	// map the entire physical chunk we have into user space.
	// (user size and placement are variable by insmod.)
	// vma, virtual, physical, size, protections
//# for natasha, 2.4.20--
//	if(remap_page_range(vma->vm_start, ociw_physbuf, ociw_physbufsize, vma->vm_page_prot ))
//# for itchy, 2.4.20-8 (if UTS_RELEASE == "2.4.20-8")
	if(remap_page_range(vma, vma->vm_start, ociw_physbuf, ociw_physbufsize, vma->vm_page_prot ))
	{
#ifdef OCIW_DEBUG
		printk(KERN_WARNING"remap err\n");
#endif
		return(-ENXIO);
	}
    vma->vm_ops = &simple_vm_ops;
    MOD_INC_USE_COUNT;
	return (0);
}
/*------------------------------------------------------------------------------
 * 
 *  Standard mmap() entry point.
 *  Very simple for the moment.
 *
 *----------------------------------------------------------------------------*/


/*
static int
ociw_mmap (struct file *file, struct vm_area_struct *vma)
{
	long user_mmapsize;
	printk(KERN_WARNING"ociw: ociw_mmap\n"); // remove
	printk(KERN_WARNING"phy_buf 0x%08lx \n", phy_buf);
	printk(KERN_WARNING"ociw_physbuf 0x%08lx \n", ociw_physbuf);
	printk(KERN_WARNING"buf size 0x%08lx \n", ociw_physbufsize);
	printk(KERN_WARNING"vma->vm_start = 0x%08lx \n", vma->vm_start);
	printk(KERN_WARNING"vma->vm_end = 0x%08lx \n", vma->vm_end);
	user_mmapsize = vma->vm_end - vma->vm_start;
	printk(KERN_WARNING"user_mmapsize = 0x%08lx \n", user_mmapsize);
	// map the entire physical chunk we have into user space.
	// (should make the user size and placement variable by insmod.)
	//if(remap_page_range(vma->vm_start,0x6000000, 0x2000000,vma->vm_page_prot))
	if(remap_page_range(vma->vm_start,ociw_physbuf, user_mmapsize, vma->vm_page_prot))
	{
		printk(KERN_WARNING"remap err\n");
		return(-EAGAIN);
	}

	return (0);
}
*/

/*------------------------------------------------------------------------------
 *
 *  Device initialization.
 *  This function initializes the board. It requests an interrupt and
 *  reserves an IO region for the operation registers. The Major number
 *  has already been allocated by the main init function.
 *
 *----------------------------------------------------------------------------*/

static int
ociw_init_dev (struct ociw_s5920 *dev, struct pci_dev *pcidev)
{

	extern struct ociw_buffer p2a;
	extern struct ociw_buffer a2p;
	u_long  ret;
	int  count;
	int i;

#ifdef OCIW_DEBUG
	printk(KERN_WARNING"ociw: ociw_init_dev\n");//remove
#endif

	/* Buffer allocation */
	p2a.buf = (char *) kmalloc(OCIW_BUFFER_SIZE, GFP_KERNEL);
	p2a.size = OCIW_BUFFER_SIZE;

	a2p.buf = (char *) kmalloc(OCIW_BUFFER_SIZE, GFP_KERNEL);
	a2p.size = OCIW_BUFFER_SIZE;

	phy_buf = ioremap(ociw_physbuf, ociw_physbufsize); // alloc for pixels.
	// check if it worked! complain if not!!
    if(!phy_buf) // null pointer? not a good sign.
    {
		ociw_free_dev (); 
		printk(KERN_WARNING"Could not allocate ociw_physbuf!!");
		printk(KERN_WARNING"Recompile ociw.c with OCIW_DEBUG for debug printk's" );
		return (-ENOMEM);
    }
	printk(KERN_WARNING"phy_buf 0x%08lx \n", phy_buf);
	printk(KERN_WARNING"ociw_physbuf 0x%08lx \n", ociw_physbuf);
	printk(KERN_WARNING"buf size 0x%08lx \n", ociw_physbufsize);
/*
	p2a.buf = (char *) __get_free_pages (GFP_KERNEL, OCIW_BUFFER_ORDER);
	p2a.order = OCIW_BUFFER_ORDER;

	a2p.buf = (char *) __get_free_pages (GFP_KERNEL, OCIW_BUFFER_ORDER);
	a2p.order = OCIW_BUFFER_ORDER;
*/
	if ((p2a.buf == NULL) || (a2p.buf == NULL)) {
		printk (KERN_WARNING "ociw: no memory for Pass-Thru buffers\n");
		ociw_free_dev ();
		return (-ENOMEM);
	}

	/* The following section retrieves from PCI_BASE_ADDRESS_0 the address
	 * of the S5920 operations registers. It then registers the IO address
	 * range with the system (ie 32 DWORD).				 */

#ifdef KERNVER2_4
 	printk (KERN_WARNING "ociw: pcidev_resource[0].start = 0x%08lx\n",
		pcidev->resource[0].start);
#else
	printk (KERN_WARNING "ociw: pcidev_base_address[0] = 0x%08lx\n",
		pcidev->base_address[0]);
	printk (KERN_WARNING "ociw: pcidev_base_address[1] = 0x%08lx\n",
		pcidev->base_address[1]);
	printk (KERN_WARNING "ociw: pcidev_base_address[2] = 0x%08lx\n",
		pcidev->base_address[2]);
	printk (KERN_WARNING "ociw: pcidev_base_address[3] = 0x%08lx\n",
		pcidev->base_address[3]);
	printk (KERN_WARNING "ociw: pcidev_base_address[4] = 0x%08lx\n",
		pcidev->base_address[4]);
	printk (KERN_WARNING "ociw: pcidev_base_address[5] = 0x%08lx\n",
		pcidev->base_address[5]);
#endif

#ifdef KERNVER2_4
	/* Check for I/O or Memory Mapped Operation Registers */
	// (the OCIW board is memory mapped. More generally, could be either.)
	// if (pcidev->resource[0].start & PCI_BASE_ADDRESS_SPACE){ <-- wrong
	// the new resource structure has separated out the usage flags!
	if (pcidev->resource[0].flags & IORESOURCE_IO){
		dev->op_regs=(pcidev->resource[0].start & PCI_BASE_ADDRESS_IO_MASK);
		ret = check_region (dev->op_regs, AMCC_OP_REG_SIZE);
		if (ret == -EBUSY) {
#ifdef OCIW_DEBUG
			printk (KERN_WARNING "ociw: I/O space already requested at 0x%04lx\n", dev->op_regs);
#endif
			dev->op_regs = 0;
			ociw_free_dev ();
			return (-EIO);
		}
		request_region (dev->op_regs, AMCC_OP_REG_SIZE, OCIW_NAME);
		dev->op_regs_type = PCI_BASE_ADDRESS_SPACE_IO;
#ifdef OCIW_DEBUG
		printk (KERN_WARNING "ociw: IO_MAP BADR[0] = 0x%08lx\n",dev->op_regs);
#endif
	}
	else {
		dev->op_regs=(pcidev->resource[0].start & PCI_BASE_ADDRESS_MEM_MASK);
		dev->op_regs = (u_long) ioremap (dev->op_regs, AMCC_OP_REG_SIZE);
		dev->op_regs_type = PCI_BASE_ADDRESS_SPACE_MEMORY;
#ifdef OCIW_DEBUG
		printk (KERN_WARNING "ociw: MEM_MAP BADR[0] = 0x%08lx\n",dev->op_regs);
#endif
	}
#else

	/* Check for I/O or Memory Mapped Operation Registers */

	if (pcidev->base_address[0] & PCI_BASE_ADDRESS_SPACE){
		dev->op_regs=(pcidev->base_address[0] & PCI_BASE_ADDRESS_IO_MASK);
		ret = check_region (dev->op_regs, AMCC_OP_REG_SIZE);
		if (ret == -EBUSY) {
#ifdef OCIW_DEBUG
			printk (KERN_WARNING "ociw: I/O space already requested at 0x%04lx\n", dev->op_regs);
#endif
			dev->op_regs = 0;
			ociw_free_dev ();
			return (-EIO);
		}
		request_region (dev->op_regs, AMCC_OP_REG_SIZE, OCIW_NAME);
		dev->op_regs_type = PCI_BASE_ADDRESS_SPACE_IO;
#ifdef OCIW_DEBUG
		printk (KERN_WARNING "ociw: IO_MAP BADR[0] = 0x%08lx\n",dev->op_regs);
#endif
	}
	else {
		dev->op_regs=(pcidev->base_address[0] & PCI_BASE_ADDRESS_MEM_MASK);
		dev->op_regs = (u_long) ioremap (dev->op_regs, AMCC_OP_REG_SIZE);
		dev->op_regs_type = PCI_BASE_ADDRESS_SPACE_MEMORY;
#ifdef OCIW_DEBUG
		printk (KERN_WARNING "ociw: MEM_MAP BADR[0] = 0x%08lx\n",dev->op_regs);
#endif
	}
#endif

	/* For each Pass-Thru region, extract information from the PCI base
	 * addresses registers and remap each area into memory space.
	 * As per the PCI spec, as soon as one base address is unused
	 * (ie set to 0) any subsequent address is also unused.	  */

#ifdef OCIW_DEBUG
	printk (KERN_WARNING "ociw: Getting configuration for PT regions\n");

	for (count = 1; count <=4; count++){
#ifdef KERNVER2_4
		printk(KERN_WARNING "ociw: resource[%d].start = %08lx \n",
			count, pcidev->resource[count].start);
#else
		printk(KERN_WARNING "ociw: PT[%d] base_address[%08lx] \n",
			count, pcidev->base_address[count]);
#endif
	}

#endif
	dev->ptnum = 0;
	for (count = 1; count <= 4; count++) {
		u32  val;
		u32  mask;
#ifdef KERNVER2_4
		if (pcidev->resource[count].start == 0) {
#ifdef OCIW_DEBUG
		printk(KERN_WARNING "ociw: PT[%d]  blank -- not mapped\n",count);
#endif
			break;
		}

		if (pcidev->resource[count].start & PCI_BASE_ADDRESS_SPACE_IO) {
#ifdef OCIW_DEBUG
			printk(KERN_WARNING "ociw: We only support memory mapped PT \n");
#endif
			continue;
		}
#else
		if (pcidev->base_address[count] == 0) {
#ifdef OCIW_DEBUG
		printk(KERN_WARNING "ociw: PT[%d]  blank -- not mapped\n",count);
#endif
			break;
		}

		if (pcidev->base_address[count] & PCI_BASE_ADDRESS_SPACE_IO) {
#ifdef OCIW_DEBUG
			printk(KERN_WARNING "ociw: We only support memory mapped PT \n");
#endif
			continue;
		}
#endif
		cli ();
		pci_read_config_dword(pcidev, badrs[count], &val);
		pci_write_config_dword(pcidev, badrs[count], ~0);
		pci_read_config_dword(pcidev, badrs[count], &mask);
		pci_write_config_dword(pcidev, badrs[count], val);
		sti();

#ifdef OCIW_DEBUG
		printk (KERN_WARNING "ociw: pt[%d]  mask %08lx  val  %08lx\n",
			count, mask, val);
#endif

		mask &= PCI_BASE_ADDRESS_MEM_MASK;

		dev->ptnum++;

		(dev->pts[count]).size = (~mask) + 1;
#ifdef KERNVER2_4
		(dev->pts[count]).phys_addr =
			(pcidev->resource[count].start & PCI_BASE_ADDRESS_MEM_MASK);
#else
		(dev->pts[count]).phys_addr =
			(pcidev->base_address[count] & PCI_BASE_ADDRESS_MEM_MASK);
#endif
		(dev->pts[count]).virt_addr = 
			ioremap ((dev->pts[count]).phys_addr,(dev->pts[count]).size);

#ifdef OCIW_DEBUG
		printk (KERN_WARNING "ociw: PT[%d]  Size %8d  Phys %08lx  Virt %08lx\n",
			(count), 
			(dev->pts[count]).size,
			(dev->pts[count]).phys_addr, 
			(dev->pts[count]).virt_addr);
#endif
	}

	/* Board reset -- set some defaults */
#ifdef OCIW_DEBUG
	printk (KERN_WARNING "ociw: I/O space at 0x%04lx\n", dev->op_regs);
#endif
	ociw_write_opreg (RCR_RESET_MBFLAGS, AMCC_OP_REG_RCR);
	ociw_write_opreg (RCR_RESET_ADDON, AMCC_OP_REG_RCR);
	for (i=0; i<100; i++)
		udelay(10000);

	ociw_write_opreg ((u32) 0, AMCC_OP_REG_RCR);
	ociw_write_opreg ((u32) 0, AMCC_OP_REG_ICSR); // no ints yet

	// set up our bottom half
	ociw_task.routine = ociw_bottom_half;
	ociw_task.data = (void *) 0;
	// ociw_task.next = 0;

	// 
	pcibios_read_config_byte(pcidev->bus->number
			, pcidev->devfn,PCI_INTERRUPT_LINE,&ociw_dev.irq);

#ifdef OCIW_DEBUG
	printk(KERN_WARNING"pcibios irq: %d\n", ociw_dev.irq);
#endif
        // try 
	if ( request_irq(ociw_dev.irq, 
                ociw_irq_handler,SA_INTERRUPT | SA_SHIRQ ,"ociw",&ociw_dev)<0)
    {
#ifdef OCIW_DEBUG
	        printk(KERN_WARNING"request_irq failed\n");
#endif
    }
    else
#ifdef OCIW_DEBUG
		printk(KERN_WARNING"SA_SHIRQ|SA_INTERRUPT OK ociw irq: %d\n", ociw_dev.irq);
#endif
	ociw_write_opreg ((u32) 0, AMCC_OP_REG_PTCR);
	/* Power up: ICSR 0x00000C0C RCR 0x00000000 PTCR 0x80808080 */

#ifdef OCIW_DEBUG
	printk (KERN_WARNING "ociw_init: ICSR: 0x%08lx\n", 
			ociw_read_opreg (AMCC_OP_REG_ICSR));
	printk (KERN_WARNING "ociw_init:  RCR: 0x%08lx\n",
			ociw_read_opreg (AMCC_OP_REG_RCR));
	printk (KERN_WARNING "ociw_init: MBEF: 0x%08lx\n", 
			ociw_read_opreg (AMCC_OP_REG_MBEF));
	printk (KERN_WARNING "ociw_init: PTCR: 0x%08lx\n", 
			ociw_read_opreg (AMCC_OP_REG_PTCR));

	printk (KERN_WARNING "ociw_init: function terminated successfully\n");
#endif

	return (0);
}

/*------------------------------------------------------------------------------
 * 
 *  Device finalization.
 *  It selectively frees all the resources allocated by the driver.
 *
 *----------------------------------------------------------------------------*/

static void
ociw_free_dev ()
{
	extern struct ociw_s5920 ociw_dev;
	extern struct ociw_buffer p2a;
	extern struct ociw_buffer a2p;
	int count = 0L;
#ifdef OCIW_DEBUG
	printk(KERN_WARNING"ociw: ociw_free_dev\n");//remove
#endif

	/* reset the ICSR register */
	ociw_write_opreg ((u32) 0, AMCC_OP_REG_ICSR);
	if(phy_buf)
		iounmap(phy_buf); // don't forget to free our page map entries!!.

	if (ociw_dev.major)
		unregister_chrdev (ociw_dev.major, OCIW_NAME);

	if (ociw_dev.op_regs) {
		if (ociw_dev.op_regs_type == PCI_BASE_ADDRESS_SPACE_IO)
			release_region (ociw_dev.op_regs, AMCC_OP_REG_SIZE);
		else
			iounmap ((void *)ociw_dev.op_regs);		 /* gsb added (void *) */
	}

	// don't leave interrupts on, or you are asking for trouble!
	ociw_write_opreg ((u32) 0, AMCC_OP_REG_ICSR);

	/* Free IRQ */
	if (ociw_dev.irq)
		free_irq (ociw_dev.irq, &ociw_dev);

	/* Free pass-thru regions -- S5920 has PT[1] thru PT[4] */
	for (count = 1; count <= 4; count++){
		if ((ociw_dev.pts[count]).virt_addr != NULL) 
			iounmap ((ociw_dev.pts[count]).virt_addr);
	}

	/* Free Pass-Thru + FIFO buffers */
	if (p2a.buf) 
		kfree (p2a.buf);

	if (a2p.buf)
		kfree (a2p.buf);
}


/*
// ok, clear out the pixels.
// well, all the ones in the fowler frame.
// ACM.. can eliminate, first sample is assigned not added.
//
void clearpix()
{
	int i;
	long *fowframe;
	// printk(KERN_WARNING"ociw: ociw_clearpix\n");//remove

	// clear the pedestal image.
	i=nrows*ncols;
	fowframe=pedstart;
	while(i--)
	{
		*fowframe++ = 0;
	}

	// clear the signal image.
	i=nrows*ncols;
	fowframe=sigstart;
	while(i--)
	{
		*fowframe++ = 0;
	}
	printk (KERN_WARNING "ociw: cleared pixels\n");
}

*/

// copy the fowler image into its 16 bit destination, after
// dividing by nsamps

void CopyFowler()
{
	int i;
	short *dest16;
	long *fowped;
	long *fowsig;

#ifdef OCIW_DEBUG
	// printk(KERN_WARNING"ociw: ociw_CopyFowler\n");// remove
#endif

	fowped=pedstart;
	fowsig=sigstart;

	dest16 = &phy_buf[imagenum*ociw_maxpix];
	i=nrows*ncols;
			
	switch(ociw_imagemode) {
	case SIGMINUSPED:
        if (ociw_invertvid)
		    while(i--)
		    {
			    *dest16++ = (short) (((*fowped++) - (*fowsig++))/nsamps);
		    }
        else
		    while(i--)
		    {
			    *dest16++ = (short) (((*fowsig++) - (*fowped++))/nsamps);
		    }
		break;
	case SIG:
		while(i--)
		{
			*dest16++ = (short) ((*fowsig++)/nsamps);
		}
		break;
	case PED: //Just the pedestal
		while(i--)
		{
			*dest16++ = (short) ((*fowped++)/nsamps);
		}
		break;
	case ACCSIG:  //accumulated signal
		while(i--)
		{
			*dest16++ = (short) (*fowsig++);
		}
		break;
	case ACCPED:  //accumulated signal
		while(i--)
		{
			*dest16++ = (short) (*fowped++);
		}
		break;
	default:
		break;
	}
}

// the bottom half does the image processing.
// it wakes up when a whole frame is waiting.
// this makes the logic pretty easy!!


/*
void checkFrame(ushort expected)
{
	int i;
	long *fowframe;
	long firstpix;
	long diffs = 0 ;
	u_short diff;

	if(expected==0)
	{
	fowframe = fowstart;
	for(i=0;i<20;i++)
		printk(KERN_WARNING" %ld\n",fowframe[i]);
	firstpix = fowframe[(nrows*ncols)-100];
	i = nrows*ncols;
	while(i--)
	{
		if(firstpix != *fowframe++)
			diffs++;
	}
	printk(KERN_WARNING"CF0diffs: %ld \n", diffs);
	return;
	}

	// we are  expecting a cdiff.
	fowframe = fowstart;
	i = nrows*ncols;
	diffs = 0;

	while(--i)
	{
		diff = fowframe[1]-fowframe[0];
		fowframe++;
		if (diff != expected)
		{
			// printk(KERN_WARNING"CF diff at %d\n", nrows*ncols - i);
			diffs++;
		}
	}
	printk(KERN_WARNING"CFdiffs: %ld \n", diffs);
}
*/

// useful for debugging intermediate results
// with the sequence generating diagnostic hardware mode.
// check this sequence for increasing values

int checkPix(u_short * pix, int npix,u_short expected)
{
	u_short diff;
	int diffs;
	diffs=0;
#ifdef OCIW_DEBUG
	printk(KERN_WARNING"ociw: ociw_checkPix\n");//remove
#endif

	while (--npix)
	{
		diff=pix[1]-pix[0];
		pix++;
		if(diff != expected)
		{
			diffs++;
#ifdef OCIW_DEBUG
			printk(KERN_WARNING"BH diff at %d\n", nrows*ncols-npix);
#endif
		}
	}
#ifdef OCIW_DEBUG
	printk(KERN_WARNING"BHdiffs: %ld \n", diffs);
#endif
	return diffs;
}

void emptyFifo()
{
#ifdef OCIW_DEBUG
	// printk(KERN_WARNING"ociw: ociw_emptyFifo\n");//remove
#endif
    int i;
    short dummy;

    for (i=0; i<ociw_region2pix; i++)
	    dummy  = (short)readl((u32*)(ociw_dev.pts[1]).virt_addr);

    // this might cause trouble!!
    // memcpy seems to run too fast for PCI.. or maybe it was another bug.
	//memcpy(rawbuf, (u32 *)(ociw_dev.pts[2]).virt_addr ,4096*2); 
	//memcpy(rawbuf, (u32 *)(ociw_dev.pts[2]).virt_addr ,4096*2); 
	//memcpy(rawbuf, (u32 *)(ociw_dev.pts[2]).virt_addr ,4096*2); 
}

// the bottom half takes one or more raw images
// and adds them to or subtracts them from the accumulated image.
// it uses and modifies bhstart to know where the source buffer is
// and uses latest_buf to know where to stop.
// it modifies cursamp and dosigs, which basically
// tell it which frame in the fowler cycle we are on.
void ociw_bottom_half(void* unused)
{
	int i;
	long *fowframe;
	short *bhbuf;
	// this routine is where the pre-pix pixels are tossed.
	static int called = 0;

#ifdef OCIW_DEBUG
	// printk(KERN_WARNING"ociw: ociw_bottom_half\n");//remove
#endif

	// check to see if bottom half recurses.
	if(called)
		we_wrapped =1;
	called++;

#ifdef OCIW_DEBUG
	//printk(KERN_WARNING"bhstart: %d\n",bhstart);
#endif

	// latest_buf is volatile!
	// (if ociw_preframepix )
	while(bhstart != latest_buf) // while we aren't putting pix in the latest buf.
	{
		if (ociw_preframepix) // any pixels to skip?
		{
			// if so, then move the real pixels.
			// I know, its inefficient to move them this
			// extra time. It isn't that bad.
			// memcopy might work here.
			// incoming raw buffers are farther
			// into the physical buffer.
			// copy them closer now.
			short *bhsrc; // pointer to full frame buffer upon entry
			short *bhdest; // pointer to full frame buffer upon exit 
			i = nrows*ncols;
			bhsrc = &rawbuf[ociw_preframepix + bhstart*(ociw_preframepix + nrows*ncols)];
			bhdest = &rawbuf[bhstart*nrows*ncols];
			while (i--)
				*bhdest++ = *bhsrc++;;
		}

		bhbuf = &rawbuf[bhstart*nrows*ncols];
		i = nrows*ncols;
		if(dosigs)
			fowframe = sigstart;// we are either doing signals..
		else
			fowframe = pedstart;// or we are doing pedestals.
		// COOL! we have a whole new frame to add in.

		// need to modify to handle coadds!

/*
		if(ociw_invertvid) // to invert vid, just flip bits.
		{
			if(cursamp == 0)
				while(i--) 		// (assign frame for first sample.)
					*fowframe++ = (int)(~(*bhbuf++));
			else
				while(i--) 		// (add in the frame for other samples.)
					*fowframe++ += (int)(~(*bhbuf++));
		}
		else
*/
		
		if(cursamp == 0)
			while(i--) 		// (assign frame for first sample.)
				*fowframe++ = *bhbuf++;
		else
			while(i--) 		// (add in the frame for other samples.)
				*fowframe++ += *bhbuf++;

		// now, we are done with bhstart for this buffer.
		// move to the next buffer before we forget.
		bhstart++;
		if (bhstart == NBUFS)
			bhstart=0;

		cursamp++; // we are ahead 1 sample.

		// debug...
/*		if(dosigs)
			checkFrame(cursamp-nsamps);
		else
			checkFrame(-cursamp);
*/
		if(cursamp == nsamps) // was it the last sample?
		{
			cursamp = 0;
			if(dosigs)
			{
				// we are done!! bail this place.
		//		checkPix(rawbuf+(2*nsamps-1)*nrows*ncols,nrows*ncols,1);
						// look at raw data
#ifdef OCIW_DEBUG
	//			printk(KERN_WARNING "last sigs. yippee\n");
#endif
				// checkFrame(0);
				//wake_up(&wq);
				//wake_up_interruptible(&wq);
				CopyFowler();
			}
			else
			{
#ifdef OCIW_DEBUG
		//		printk(KERN_WARNING "sigs\n");
#endif
				dosigs  = 1; //  TRUE, we are doing signals now.
#ifdef OCIW_DEBUG
		//		printk(KERN_WARNING "peds. signals now!\n");
#endif
			}
		}
		pixtogo -= nrows*ncols;
	}
	called--;
}

// fifoIntoBuf is a subroutine of the interrupt routine.
// basically, it does all the work.
// we just got a half full interrupt.
// pull the pixels out of the FIFO and put them into 
// the input buffers.
// if any buffer is filled (may fill several for tiny frame size),
// wake up the bottom half and move on to the next buffer.
//

void fifoIntoBuf()
{
	int pixleft;
#ifdef OCIW_DEBUG
	// printk(KERN_WARNING"ociw: ociw_fifoIntoBuf\n");//remove
#endif

	// while the fifo is half full
	while  (!(ociw_read_opreg(AMCC_OP_REG_IMB) & IMB_NOT_HF))
	{
		// do the transfer, copying ociw_region2pix pixels
		// (might overrun last buffer and copy into padding area, but so what?
		// that is what is there for.)
		// ociw_region2pix is the memory size of pass thru region 2.
		// which does prefetch of the FIFO.

		// ociw_region2pix/2 Dwords 
		// == ociw_region2pix*2 bytes 
		// == ociw_region2pix pixels
		//if(bufpixtogo >= ociw_region2pix)
		// how many more pixels in the NBUFS string of buffers?
		pixleft = (ociw_preframepix + nrows*ncols)*NBUFS  
				// total number of pix in input buffer
				- (curbuf-rawbuf); // minus how many we used so far.
		// where is the next frame boundary?
		// curbuf is next empty spot.
		// curbuf - rawbuf is number of pixels written in rawbuf space so far.
		// prepix might occur at the beginning or end of a fifo block of
		// when the prepix are read, what happens?
		// prepix reading must be associated with rolling to a new frame.
		if(pixleft >= ociw_region2pix) // room to copy ? do it!
		{
			if(ociw_readstrategy==2) // fastest so far
			{
				memcpy(curbuf, (u32 *)(ociw_dev.pts[2]).virt_addr ,ociw_region2pix*2);
			}
			else if(ociw_readstrategy==1) // medium. use prefetch but not memcpy
			{
				int i;
				for (i=0; i<(ociw_region2pix>>1); i++)
		   			((long *)curbuf)[i] =
						 readl(((u32*)(ociw_dev.pts[2]).virt_addr)+i);
			}
			else // slowest.. 
			{
				int i;
				for (i=0; i<ociw_region2pix; i++)
		 		 	curbuf[i] = 
						(short)readl((u32*)(ociw_dev.pts[1]).virt_addr);
			}
			curbuf +=  ociw_region2pix; // so move ahead in curbuf
		}
		else // we will wrap around somewhere!
		{
			if(ociw_readstrategy==2) // fastest so far
			{
				// fill up what we have left...
				memcpy(curbuf, (u32 *)(ociw_dev.pts[2]).virt_addr ,pixleft*2);
				// and start over at the beginning
				memcpy(rawbuf, (u32 *)(((short *)(ociw_dev.pts[2]).virt_addr)
					+bufpixtogo) ,(ociw_region2pix-pixleft)*2);
			}
			else if(ociw_readstrategy==1) // medium speed
			{
				int i;
				// fill up what we have left...
				for (i=0; i<pixleft/2; i++)
		   			((long *)curbuf)[i] =
						 readl(((u32*)(ociw_dev.pts[2]).virt_addr)+i);
				// and start over at the beginning
				for (i=0; i<(ociw_region2pix-pixleft)/2; i++)
		   			((long *)rawbuf)[i] =
						 readl(((u32*)(ociw_dev.pts[2]).virt_addr)+i+(pixleft/2));
			}
			else // slowest
			{
				int i;
				// fill up what we have left...
				for (i=0; i<pixleft; i++)
		  			curbuf[i] = (short)readl((u32*)(ociw_dev.pts[1]).virt_addr);
				// and start over at the beginning
				for (i=0; i<(ociw_region2pix-pixleft); i++)
		  			rawbuf[i] = (short)readl((u32*)(ociw_dev.pts[1]).virt_addr);
			}
			curbuf = rawbuf + (ociw_region2pix-pixleft);
		}

		bufpixtogo -= ociw_region2pix; // we just read in ociw_region2pix pixels

		if(bufpixtogo <= 0) // did we fill up the buffer?
		{
			// well, we are in a different buffer now.
			// but it might not be the next one..
			// a buffer could be smaller than half a fifo.
			// we could have filled up several.
			do
			{
				++latest_buf; // advance to the next buffer.
				if (latest_buf == NBUFS) // are we at the last buffer?
				{
					latest_buf = 0; // first, roll around latest_buf.
				}
				if(latest_buf == bhstart)
					we_wrapped=1;
				bufpixtogo += (ociw_preframepix +nrows*ncols); // move to the next buffer.
			}
			while (bufpixtogo <= 0); // no more room in this buffer?
			// ok, we are now in a new buffer with room in it.

			// queue up the bottom half!
			queue_task(&ociw_task, &tq_immediate);
			mark_bh(IMMEDIATE_BH);
		}

		// no more interrupts expected ??
		rawpixtogo -= ociw_region2pix; //  reduces total pixels we are expecting
		if(rawpixtogo <= ociw_region2pix)
		{
#ifdef OCIW_DEBUG
			// printk(KERN_WARNING"INTSOFF\n");
#endif
			ociw_write_opreg ((u32) 0, AMCC_OP_REG_ICSR);
		
			//Data is done being written so interrupt the sleep
			wake_up_interruptible(&wq); //?? is this correct?
		}
	}
}

/*------------------------------------------------------------------------------
 * 
 *  Interrupt handler.
 *
 *----------------------------------------------------------------------------*/
static void
ociw_irq_handler (int irq, void *dev, struct pt_regs *regs)
{
	u32 icsr;

	icsr = ociw_read_opreg (AMCC_OP_REG_ICSR);

	if (!(icsr & ICSR_INT_ASSERTED)) { // is the board asking??
		return; // must be someone else on this int!
    }

	if(dev != &ociw_dev){ // probably not needed?
		return; // bet it never happens.
    }


	if (irq != ociw_dev.irq) { // extremely unlikely??
#ifdef OCIW_DEBUG
		printk (KERN_WARNING "ociw: irq: spurious interrupt!\n");
#endif
		return; // never seen this either
	}				   
	if(rawpixtogo <= ociw_region2pix)
#ifdef OCIW_DEBUG
		printk(KERN_WARNING"unexpected ociw irq!");
#endif
	fifoIntoBuf(); // stuff the FIFO into the buffer.
}
