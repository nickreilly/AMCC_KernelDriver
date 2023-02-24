/******************************************************************************/
/*   amcc.c -- AMCC S5920 PCI Target Interface driver for custom pcb          */
/*             OCIW CCD-array readout electronics                             */
/*   (c) 2004 Greg Burley (burley@ociw.edu)                                   */
/*            Drew Moore  (acm3035@rit.edu)                                   */
/*                                                                            */
/*   Some of this driver is/was shamelessly based on the code for the         */
/*   AMCC S5933 PCI Matchmaker driver                                         */
/*   (c) 1997 Andrea Cisternino  (acister@pcape1.pi.infn.it)                  */
/*                                                                            */
/*   This program is free software; you can redistribute it and/or            */
/*   modify it under the terms of the GNU General Public License as           */
/*   published by the Free Software Foundation (www.gnu.org)                  */
/*                                                                            */
/******************************************************************************/

/* Ignore the current directory name -- this is the 64-bit version that we
   have been modifying.
 */

/*------------------------------------------------------------------------------
 *  Driver main file -- this file contains the driver code.  The hardware
 *  interface to the PCI bus is an AMCC S5920 chip.  Basically, it bridges the
 *  PCI bus to a local Add-on bus, with support for four pass-thru data regions.
 *  See S5920 PCI Target Interface Data Book from AMCC for details.
 *----------------------------------------------------------------------------*/
#ifndef __KERNEL__
# define __KERNEL__
#endif

/* #include <linux/config.h>                */ 
#if !defined (CONFIG_PCI)
# error Linux kernel needs PCI support for amcc driver
#endif

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
# error Linux kernel version > 2.6.0 required for this driver
#endif

#include <linux/fs.h>
//#include <linux/init.h>
#include <linux/module.h>
//#include <linux/kernel.h>
//#include <linux/types.h>
//#include <linux/fcntl.h>
#include <linux/sched.h>                /* current and everything */
#include <linux/interrupt.h>  

//#include <linux/slab.h>
#include <linux/pci.h>
//#include <linux/ioport.h>
//#include <linux/stat.h>
#include <linux/delay.h>
//#include <linux/spinlock.h>
//#include <asm/system.h>
//#include <asm/io.h>                     // for inb(), outb() etc. 
//#include <asm/irq.h> 
//#include <asm/pci.h>
#include <asm/dma.h>
//#include <asm/pgtable.h>
#include <asm/uaccess.h>


#include "amcc.h"                       /* S5920 definitions */

/*------------------------------------------------------------------------------
 *  Definitions
 *----------------------------------------------------------------------------*/
#define AMCC_NAME               "amcc"
#define AMCC_MAGIC              "z" 
#define AMCC_TIMEOUT            50          /* timeout in microseconds */
#define AMCC_MAX_RETRYS         10          /* number of snoozes. */

/* lock bits for concurrent access */
#define AMCC_P2A_LOCK_BIT       0
#define AMCC_A2P_LOCK_BIT       1
#define AMCC_TEST_IRQ_BIT       2

/* return codes for interrupts */
#define AMCC_IRQ_READ_OK        1
#define AMCC_IRQ_WRITE_OK       2
#define AMCC_IRQ_MASTER_ABORT   3
#define AMCC_IRQ_TARGET_ABORT   4
#define AMCC_IRQ_OMB_EMPTY      5
#define AMCC_IRQ_IMB_FULL       6

#define PCI_VENDOR_ID_AMCC      0x10e8
#define PCI_DEVICE_ID_S5920     0x5920

#define AMCC_MAJOR              125   /* Major device number requested */
#define AMCC_BASE_MINOR 	0     /* Minor device number requested */
#define AMCC_COUNT		1     /* Count of minor numbers */
#define AMCC_FIFO_SIZE          16384   /* FIFO is 16384 16-bit words */
#define P2A_BUFFER_SIZE         4096
#define A2P_BUFFER_SIZE         65536

/*------------------------------------------------------------------------------
 *  Structures
 *----------------------------------------------------------------------------*/

/* Buffer info -- holds data for Pass-Thru buffer handling. */
struct amcc_buffer {
    volatile u_int copy_size;
    volatile u_int bytes_done;
    volatile u_int size;
    char *buf;
};

// Driver private info -- holds global data for a SINGLE PCI board. 

struct amcc_priv {
//    struct pci_dev *pcidev;
    int major;                  /* Major number */
    int irq;                    /* interrupt number */
    u32 opreg_addr;             /* operation registers address BAR[0] */
    u32 opreg_len;              /* operation registers size */
    int opreg_type;             /* MEM or I/O operation registers */
    int ptnum;                  /* number of pass-thru regions */
    u_long pt1_phys_addr;       /* Pass-thru region 1 - BAR[1] */
    u_long pt1_size;            /* Pass-thru region 1 size */
    void*  pt1_virt_addr;       /* Virtual address after remapping */
    u_long pt2_phys_addr;       /* Pass-thru region 2 - BAR[2] */
    u_long pt2_size;            /* Pass-thru region 2 size */
    void*  pt2_virt_addr;       /* Virtual address after remapping */
    int hfmode;
};

/*------------------------------------------------------------------------------
 *  Static variables -- these can be seen by every process using the device.
 *  Use only one board per box, please.
 * XXX todo: this code should be able to handle multiple boards with minor changes.
 *----------------------------------------------------------------------------*/
static struct amcc_priv gb_dev;           /* Private device data */
static struct amcc_buffer p2a;         /* P2A buffer control struct */
static struct amcc_buffer a2p;         /* A2P buffer control struct */
static u_long lock = 0UL;       /* generic lock dword */
static short *phy_buf = NULL; // pointer to physical memory block
static short *phy_buf_end = NULL; // pointer to end of physical memory block. first invalid address

static short *pixqueue_head; // read() gets from here. Points to first pixel that may be read.
static short *pixqueue_tail; // irq puts to here. points to first pixel that may be written.
// read() must check that head and tail are not equal.
// irq() must never make them equal by writing pixels.


/*------------------------------------------------------------------------------
 *  Function prototypes and inline functions
 *----------------------------------------------------------------------------*/
static int amcc_probe (struct pci_dev *, const struct pci_device_id *);
static void amcc_remove (struct pci_dev *);
static int amcc_open (struct inode *, struct file *);
static int amcc_close (struct inode *, struct file *);
static ssize_t amcc_read (struct file *, char *, size_t, loff_t *);
static ssize_t amcc_write (struct file *, const char *, size_t, loff_t *);
static long amcc_ioctl (struct file *, unsigned int, unsigned long);
static irqreturn_t ociw_irq_handler (int, void *);

static struct file_operations amcc_fops =
{
    .owner = THIS_MODULE,
    .open = amcc_open,
    .release = amcc_close,
    .read = amcc_read,
    .write = amcc_write,
    .unlocked_ioctl = amcc_ioctl,
};

long ociw_physbufsize = 0; // size of physical memory we can use
//MODULE_PARM(ociw_physbufsize,"l"); // size of physical buffer in bytes
module_param(ociw_physbufsize, long, 0644);
//__MODULE_PARM_TYPE(ociw_physbufsize,"l"); // size of physical buffer in bytes

/*extern inline __u32*/
static inline __u32
amcc_read_opreg (int reg)
{
    return ((__u32) inl ((unsigned short) (gb_dev.opreg_addr + reg)));
}

/*extern inline void*/
static inline void
amcc_write_opreg (__u32 value, int reg)
{
    outl (value, (unsigned short) (gb_dev.opreg_addr + reg));
}

static void hf_ints(void) // called from ISR at high priority
{
	writel(0x00010001,(u32 *)gb_dev.pt2_virt_addr); // enable HF flag ints
    gb_dev.hfmode = 1; // since we are called at hi prio, order does not matter.
}

static void ef_ints(void) // called from READ at low priority
{
    gb_dev.hfmode = 0; // clear flag FIRST, so ISR won't think we are HF mode
	writel(0,(u32 *)gb_dev.pt2_virt_addr); // enable empty flag ints
    // we will get an int NOW if any pix are in fifo
    // good thing we cleared flag first.
}

static void ints_on(void)
{
	amcc_write_opreg ((u32) ICSR_ADDON_ENABLE,  AMCC_OP_REG_ICSR);
}

static void ints_off(void)
{
	amcc_write_opreg ((u32) 0 ,  AMCC_OP_REG_ICSR);
}


static void snooze(int i)
{
    set_current_state(TASK_INTERRUPTIBLE);
    schedule_timeout(i);
}
/*------------------------------------------------------------------------------
 *  Module initialization and remove functions.  
 *----------------------------------------------------------------------------*/
static int
amcc_probe (struct pci_dev* pcidev, const struct pci_device_id* amcc_device_id)
{
    extern struct amcc_priv gb_dev;
    int rc;
    int j;
    dev_t devno;
    struct device *device = NULL;
    static struct class *amcc_class = NULL;
    //static struct amcc_dev *amcc_devices = NULL;
    int err = 0;

    printk (KERN_DEBUG "amcc_probe\n");
    /* Register the private data area per board. Attempt to enable the device.
     * We don't use the private area in the way it's intended, yet */
    pci_set_drvdata(pcidev, &gb_dev);
    rc = pci_enable_device(pcidev);
    if (rc != 0) {
        printk (KERN_WARNING "amcc_probe: device enable failed \n");
        return(-EIO);
    }

    /* Register as a char() device with major number = 125 */
    rc = __register_chrdev(AMCC_MAJOR, AMCC_BASE_MINOR, AMCC_COUNT, AMCC_NAME, &amcc_fops);

    devno = MKDEV(AMCC_MAJOR, AMCC_BASE_MINOR);

    amcc_class = class_create(THIS_MODULE, AMCC_NAME);
    if (IS_ERR(amcc_class)) {
	    err = PTR_ERR(amcc_class);
	    //cfake_cleanup_module(devices_to_destroy);
	    return err; 
    }

    device = device_create(amcc_class, NULL, devno, NULL, AMCC_NAME "%d", AMCC_BASE_MINOR); 
    if (IS_ERR(device)) {
	    err = PTR_ERR(device);
	    printk(KERN_WARNING "[target] Error %d while trying to create %s%d",
			    err, AMCC_NAME, AMCC_BASE_MINOR);
	    return err;
    }

    if (rc < 0) {
	    printk(KERN_INFO "Registering device failed\n");
	    return rc;
    }

    if (rc == 0)
        gb_dev.major = AMCC_MAJOR;
    else
        gb_dev.major = rc;

    printk (KERN_DEBUG "amcc_probe: major = %d\n", gb_dev.major);

    if (rc == -EBUSY){
        printk (KERN_WARNING "amcc_probe: unable to get major number\n");
        return (-EIO);
    }

    /* Allocate buffers for Add-on to/from PCI bus transfers */
    p2a.buf = (char *) kmalloc(P2A_BUFFER_SIZE, GFP_KERNEL);
    p2a.size = P2A_BUFFER_SIZE;

    a2p.buf = (char *) kmalloc(A2P_BUFFER_SIZE, GFP_KERNEL);
    a2p.size = A2P_BUFFER_SIZE;

    if ((p2a.buf == NULL) || (a2p.buf == NULL)) {
        printk (KERN_WARNING "amcc_probe: no memory for Pass-thru buffers\n");
        if (a2p.buf) kfree(a2p.buf);
        if (p2a.buf) kfree(p2a.buf);
        __unregister_chrdev (gb_dev.major, AMCC_BASE_MINOR, 1, AMCC_NAME);
        return (-ENOMEM);
    }

    if (ociw_physbufsize) // physical buffer available?
    {

	    phy_buf = vmalloc(ociw_physbufsize); // alloc for pixels.
	    // check if it worked! complain if not!!
        if(!phy_buf) // null pointer? not a good sign.
        {
            // 
		    printk(KERN_WARNING "Could not allocate ociw_physbuf!!");
		    printk(KERN_WARNING "Recompile ociw.c with OCIW_DEBUG for debug printk's" );
            __unregister_chrdev (gb_dev.major, AMCC_BASE_MINOR, 1, AMCC_NAME);
            kfree(p2a.buf);
            kfree(a2p.buf);
		    return (-ENOMEM);
        }
        else
        {
            pixqueue_head = phy_buf; // read owns this pointer
            pixqueue_tail = phy_buf; // irq owns this pointer
            phy_buf_end = phy_buf+(ociw_physbufsize/sizeof(short)); // DON'T write here!!
            // queue is empty when both pointers are equal.
		    printk(KERN_WARNING "phy_buf 0x%08lx \n", (unsigned long)phy_buf);
        }   
    }
    else 
    { 
        printk (KERN_WARNING "zero physbufsize\n");
    }
    

    /* Reserve the BAR regions of the PCI device  */
    rc = pci_request_regions(pcidev, AMCC_NAME);
    if (rc == -EBUSY) {
        printk (KERN_WARNING "amcc_probe: IO request failed\n");
        return(-EIO);
    }

    /* Retrieve from PCI_BASE_ADDRESS_0 the start address and size of
     * the S5920 operations registers. */
    gb_dev.opreg_addr = pci_resource_start(pcidev, 0);
    gb_dev.opreg_len = pci_resource_len(pcidev, 0);

    /* Check for I/O mapped or Memory mapped Operation Registers */
    if (pci_resource_flags(pcidev, 0) & IORESOURCE_IO){
        gb_dev.opreg_type = PCI_BASE_ADDRESS_SPACE_IO;
        printk (KERN_DEBUG "amcc_probe: PCI BASE ADDRESS SPACE IO\n");
    }
    else if (pci_resource_flags(pcidev, 0) & IORESOURCE_MEM){
        gb_dev.opreg_type = PCI_BASE_ADDRESS_SPACE_MEMORY;
        printk (KERN_DEBUG "amcc_probe: PCI BASE ADDRESS SPACE MEM\n");
    }

    // For Pass-thru region extract the address and size of BAR         
    // PCI_BASE_ADDRESS_n, then remap into memory space. 

    gb_dev.pt1_size = pci_resource_len(pcidev, 1);
    gb_dev.pt1_phys_addr = pci_resource_start(pcidev, 1);
    gb_dev.pt1_virt_addr = ioremap (gb_dev.pt1_phys_addr, gb_dev.pt1_size);
    gb_dev.pt2_size = pci_resource_len(pcidev, 2);
    gb_dev.pt2_phys_addr = pci_resource_start(pcidev, 2);
    gb_dev.pt2_virt_addr = ioremap (gb_dev.pt2_phys_addr, gb_dev.pt2_size);
    printk (KERN_DEBUG "amcc_probe: PT[%d] Size %8d  Phys %08x  Virt %08x\n",
        1, (int)(gb_dev.pt1_size), (int)(gb_dev.pt1_phys_addr),
        (int)(gb_dev.pt1_virt_addr));

    /* Board reset -- set some defaults */
    amcc_write_opreg (RCR_RESET_MBFLAGS, AMCC_OP_REG_RCR);
    amcc_write_opreg (RCR_RESET_ADDON, AMCC_OP_REG_RCR);
    for (j=0; j<500; j++)udelay(1000);
    amcc_write_opreg ((u32) 0, AMCC_OP_REG_RCR);

    amcc_write_opreg ((u32) 0, AMCC_OP_REG_ICSR);
    amcc_write_opreg ((u32) 0, AMCC_OP_REG_PTCR);

    gb_dev.irq = pcidev->irq;  
    if ( request_irq(gb_dev.irq, 
                 &ociw_irq_handler,IRQF_SHARED | IRQF_DISABLED,"ociw",(void*)&gb_dev)<0)
        printk(KERN_WARNING"request irq failed\n");
    else {
        ef_ints();
        ints_on();
    }

    /* Output some debug info -- at power-up:                         *
     * ICSR 0x00000C0C RCR 0x00000000 MBEF 0x80000000 PTCR 0x80808080 */
    printk (KERN_DEBUG "amcc_probe: ICSR: 0x%08x  RCR : 0x%08x\n",
        amcc_read_opreg(AMCC_OP_REG_ICSR), amcc_read_opreg(AMCC_OP_REG_RCR));
    printk (KERN_DEBUG "amcc_probe: MBEF: 0x%08x  PTCR: 0x%08x\n",
        amcc_read_opreg(AMCC_OP_REG_MBEF), amcc_read_opreg(AMCC_OP_REG_PTCR));

    return (0);
}

static void
amcc_remove (struct pci_dev* pcidev)
{
    extern struct amcc_priv gb_dev;
    struct amcc_priv *priv;

    /* Free all the resources allocated by the driver */
    priv = pci_get_drvdata(pcidev);
    __unregister_chrdev (gb_dev.major, AMCC_BASE_MINOR, 1, AMCC_NAME);

    ints_off();
    free_irq (gb_dev.irq, &gb_dev);

    if (p2a.buf) kfree(p2a.buf);
    if (a2p.buf) kfree(a2p.buf);

    iounmap ((void *) (gb_dev.pt1_virt_addr));
    iounmap ((void *) (gb_dev.pt2_virt_addr));
    
    if(phy_buf)
    {
        vfree(phy_buf);
    }

    pci_release_regions(pcidev);
    pci_disable_device(pcidev);

}


/*------------------------------------------------------------------------------
 *  Standard module open() and close() functions.  
 *----------------------------------------------------------------------------*/
static int
amcc_open (struct inode *inode, struct file *file)
{
    printk (KERN_DEBUG "amcc_open : major number %d  minor number %d\n",
        imajor(inode), iminor(inode));
    return (0);
}


static int
amcc_close (struct inode *inode, struct file *file)
{
    printk (KERN_DEBUG "amcc_close: close\n");
    return(0);
}

/*------------------------------------------------------------------------------
 *  Module read() entry point.
 *  Read from the Add-on bus to PCI bus. Fill a kernel buffer with the 
 *  specified number of bytes.  Hardware specific.
 *----------------------------------------------------------------------------*/
static ssize_t
amcc_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
    extern struct amcc_priv gb_dev;
    extern struct amcc_buffer a2p;
    ssize_t  total_bytes_read = 0;
    u32 *ptr, r;
    u16 *rbuf;
    int j, k, nwords, retrys;

    r=0;
    k=0;
    j=0;
    retrys=0;


    if (count == 0) {
        return (0);
    }

    /* Is the Pass_Thru + FIFO available? */
    if (test_and_set_bit (AMCC_A2P_LOCK_BIT, &lock)) {
        printk (KERN_DEBUG "amcc: Read PT+FIFO already locked\n");
        return (-EBUSY);
    }

    /* Set up to read the device into the a2p buffer */
    ptr = (u32 *)(gb_dev.pt1_virt_addr);
    rbuf = (u16 *)a2p.buf;
    nwords = 0;

    a2p.bytes_done = 0;
    a2p.copy_size = min (count, a2p.size);

#define AMCC_ROUTINE   4

#if (AMCC_ROUTINE==0)
/*  Routine 0 -- Read until done, or until the FIFO goes empty. */
/*  If the FIFO is not ready, return immediately.               */
    for (j=0; j<a2p.copy_size/2; j++) {
        if (!(amcc_read_opreg (AMCC_OP_REG_IMB) & IMB_RX_RDY))
            break;
        rbuf[j] = (u16) readl ((u32 *)(&ptr[j]));
        a2p.bytes_done +=2;
    }
#endif

    if(!phy_buf)
    {
/*  Routine 3 -- Read until done, or until the FIFO goes empty.
 *  If the FIFO is not ready, wait for a short while.  If the FIFO
 *  is ready with data, check the programmable-almost-empty flag.
 *  Do a burst read or single read as appropriate. */ 
        j = 0;
        while (a2p.bytes_done < a2p.copy_size  && retrys < AMCC_MAX_RETRYS) {
            r = amcc_read_opreg (AMCC_OP_REG_IMB);
            if (r & IMB_RX_RDY){
                if (a2p.copy_size-a2p.bytes_done > 255 && (r & IMB_RX_PAE) ){
                    for (k=0; k<127; k++){
                        rbuf[j] = (u16) readl ((u32 *)(&ptr[k]));
                        j++;
                        a2p.bytes_done += 2;
                    }
                }
                else {
                    rbuf[j] = (u16) readl ((u32 *)(&ptr[0]));
                    a2p.bytes_done +=2;
                    j++;
                }
                retrys=0;
            }
            else {
                if (a2p.copy_size==2)
					break;
                retrys += 1;
                snooze(0);
                // udelay(1000);
            }
        }
    }
    else
    {
        j = 0;
    /*  Routine 4 -- interrupt driven.
    *  If no pixels are ready, wait for a short while. 
    *  otherwise, copy into the user's buffer */ 
        while (a2p.bytes_done < a2p.copy_size  && retrys < AMCC_MAX_RETRYS) {
            if (pixqueue_head == pixqueue_tail) { // no pixels available? 
                snooze(0);
                if (retrys)
				{
					if(gb_dev.hfmode){
                    	ef_ints(); // try this.
						printk(KERN_DEBUG"EF\n");
					}
				}
                retrys++;
            }
            else {
                rbuf[j] = (u16) *pixqueue_head;
                // NOTE: CANNOT increment pixqueue_head and then test it for wrap.
                // irq might happen while head pointer is invalid.
                // (very slim but finite chance of a wraparound bug in that case. )
                if (pixqueue_head+1 >= phy_buf_end){
                    pixqueue_head = phy_buf;
                }
                else {
                    pixqueue_head += 1; 
                }
                a2p.bytes_done +=2;
                j++;
                retrys = 0;
            }
        }
    }

    if(copy_to_user(buf, a2p.buf, a2p.bytes_done))
    {
        clear_bit (AMCC_A2P_LOCK_BIT, &lock);
        return(-EFAULT);
    }
    total_bytes_read += a2p.bytes_done;
    count -= a2p.bytes_done;

    clear_bit (AMCC_A2P_LOCK_BIT, &lock);
    return (total_bytes_read);

}

/*------------------------------------------------------------------------------
 *  Standard write() entry point.
 *  Write to the PCI-to-Add-on FIFO. Copy the user buffer into our (smaller) 
 *  kernel buffer and then perform a transfer. Hardware specific.
 *----------------------------------------------------------------------------*/
static ssize_t
amcc_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
    extern struct amcc_priv gb_dev;
    extern struct amcc_buffer p2a;
    ssize_t total_bytes_written = 0;
    u32 *ptr;
    u16 *wbuf;
    int j,k;

    if (test_and_set_bit (AMCC_P2A_LOCK_BIT, &lock)) {
        printk (KERN_DEBUG "amcc_write: Pass_Thru + FIFO already locked\n");
        return (-EBUSY);
    }

    /* Can we check to see if the Pass-Thru + FIFO is empty? */

    /* Set up to write to the device from the p2a buffer */
    ptr = (u32 *)(gb_dev.pt1_virt_addr);
    wbuf = (u16 *)p2a.buf;

    while (count > 0){
        p2a.bytes_done = 0;
        p2a.copy_size = min (count, p2a.size);
        if (copy_from_user(p2a.buf, buf, p2a.copy_size))
            return (-EFAULT);
        for (j=0; j<p2a.copy_size/2; j++){
            for (k=0; k < AMCC_TIMEOUT; k++){
                udelay(1);
                if ( !(amcc_read_opreg(AMCC_OP_REG_IMB) & IMB_TX_BUSY))
                    break;
            }
            writel ( (u32)wbuf[j], (u32 *)(&ptr[j]) );
            udelay(5);
            p2a.bytes_done += 2;
        }
        count -= p2a.bytes_done;
        total_bytes_written += p2a.bytes_done;
    }
    clear_bit (AMCC_P2A_LOCK_BIT, &lock);
    return (total_bytes_written);
}

/*------------------------------------------------------------------------------
 *  Standard module ioctl() entry point.
 *----------------------------------------------------------------------------*/
static long
amcc_ioctl (struct file *file,
    unsigned int cmd, unsigned long arg)
{
    extern struct amcc_priv gb_dev;
    u32  *addr = (u32 *) arg;
    u32  count=0;
    int rc = 0L;
    int j;

    switch (cmd) {
    case AMCC_RESET:            /* reset the whole board */
        amcc_write_opreg (RCR_RESET_MBFLAGS, AMCC_OP_REG_RCR);
        amcc_write_opreg (RCR_RESET_ADDON, AMCC_OP_REG_RCR);
        amcc_write_opreg ((u32) 0, AMCC_OP_REG_RCR);
        pixqueue_head = phy_buf; 
        pixqueue_tail = phy_buf; 
        lock = 0L;
        break;

    case AMCC_RESET_ADDON:      /* reset the Add-on bus */
        amcc_write_opreg (RCR_RESET_ADDON, AMCC_OP_REG_RCR);
        for (j=0; j<100; j++)udelay(1000); // ???
        amcc_write_opreg ((u32) 0, AMCC_OP_REG_RCR);
        pixqueue_head = phy_buf; // read owns this pointer
        pixqueue_tail = phy_buf; // irq owns this pointer
        lock = 0L;
        break;

    case AMCC_RESET_FIFO:       /* reset the PCI card external FIFO */
        for (j=0; j<AMCC_FIFO_SIZE; j++) {
            if (!(amcc_read_opreg (AMCC_OP_REG_IMB) & IMB_RX_RDY))
                break;
            (void)readl((u32 *)(gb_dev.pt1_virt_addr));
            count +=2;
        }
        pixqueue_head = phy_buf; // read owns this pointer
        pixqueue_tail = phy_buf; // irq owns this pointer
        rc = put_user (count, addr);
        break;

    case AMCC_CLR_FIFO:       /* assumes ints are reading. */
        ef_ints(); // 
        snooze(0);
        pixqueue_head = phy_buf; // read owns this pointer
        pixqueue_tail = phy_buf; // irq owns this pointer
        break;

    case AMCC_GET_FIFO:         /* read ICSR register */
		ef_ints();
        snooze(0);
		j = pixqueue_tail - pixqueue_head;
		if (j < 0)
			j += (ociw_physbufsize>>1);
		printk(KERN_DEBUG"getfifo, %d\n",j);
        rc = put_user (j , addr);
        break;

    case AMCC_GET_ICSR:         /* read ICSR register */
        rc = put_user (amcc_read_opreg (AMCC_OP_REG_ICSR), addr);
        break;

    case AMCC_GET_RCR:          /* read RCR register */
        rc = put_user (amcc_read_opreg (AMCC_OP_REG_RCR), addr);
        break;

    case AMCC_GET_PTCR:         /* read PTCR register */
        rc = put_user (amcc_read_opreg (AMCC_OP_REG_PTCR), addr);
        break;

    case AMCC_GET_MBEF:         /* read mailbox MBEF register */
        rc = put_user (amcc_read_opreg (AMCC_OP_REG_MBEF), addr);
        break;

    default:
        rc = -EINVAL;
        break;
    }
    return (rc);
}

static short * nextTail(void)
{
	short * next_tail;
    next_tail = pixqueue_tail+1; // move ahead two bytes. (== one short)
    if (next_tail >= phy_buf_end){
        next_tail = phy_buf;
    }
	return next_tail;
}

static void fifoIntoBuf(void)
// read whatever pixels we can from the fifo
// stick them into the queue.
{
    int xfers = 0;
    int r ; 
    r = amcc_read_opreg (AMCC_OP_REG_IMB);
    while(r & IMB_RX_RDY)
    {
		// read the next pixel
        *pixqueue_tail = (u16) readl ((u32 *)(gb_dev.pt1_virt_addr));
		// if we wrap, we'll certainly detect it elsewhere.
        pixqueue_tail = nextTail();
        xfers += 1;
        if (xfers>8192)
            break; // don't loop forever.
        r = amcc_read_opreg (AMCC_OP_REG_IMB);
    }
    hf_ints(); 
}

/*------------------------------------------------------------------------------
 * 
 *  Interrupt handler.
 *
 *----------------------------------------------------------------------------*/

static irqreturn_t
ociw_irq_handler (int irq, void *dev)
{
	u32 icsr = 0;

	icsr = amcc_read_opreg (AMCC_OP_REG_ICSR);

	if (!(icsr & ICSR_INT_ASSERTED)) { // is the board asking??
		return IRQ_NONE; // must be someone else on this int!
    }

	if(dev != &gb_dev){ // not called on behalf of this device?
		return IRQ_NONE; // wait till it is.
    }

	if (irq != gb_dev.irq) { // irq is not the expected irq?
		return IRQ_NONE; // hmm. that is odd.
	}

    //if (gb_dev.hfmode)
	    //fifo4KIntoBuf(); // stuff the FIFO into the buffer.
    //else
	    fifoIntoBuf(); // stuff the FIFO into the buffer.
    return IRQ_HANDLED ;
}

/*------------------------------------------------------------------------------
 *  Standard support entries.
 *----------------------------------------------------------------------------*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("(c) 2004 Greg Burley  www.ociw.edu");
MODULE_DESCRIPTION("Driver for CCD pci interface pcb");

//MODULE_PARM(ociw_physbuf,"l"); // location of physical buffer start

static struct pci_device_id id_table[] = {
    {PCI_VENDOR_ID_AMCC, PCI_DEVICE_ID_S5920, PCI_ANY_ID, PCI_ANY_ID,0,0,0},
    {0,}    // end-of-list
};

MODULE_DEVICE_TABLE(pci, id_table);

static struct pci_driver amcc_driver = {
    .name = AMCC_NAME,
    .id_table = id_table,
    .probe = amcc_probe,
    .remove = amcc_remove
};

int __init
amcc_init (void)
{
    int rc = 0;

    printk (KERN_INFO "amcc_init: AMCC S5920 Rev 2.1a for 3.13 kernel \n");
    rc = pci_register_driver(&amcc_driver);
    printk (KERN_DEBUG "amcc_init: %d\n", rc);
    return (rc) ? 0 : rc;
}

void __exit
amcc_exit (void)
{
    printk (KERN_INFO "amcc_exit: AMCC S5920 Goodbye\n");
    pci_unregister_driver(&amcc_driver);
}

module_init(amcc_init);
module_exit(amcc_exit);
