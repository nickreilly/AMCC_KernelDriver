/********************************************************************************** 
 * 
 *     DEFINES for 56302/3 interrupts 
 * 
 *     Derived from .asm file (Last .asm update: June 11 1995).
 * 
 ********************************************************************************** */


#if	defined(I_VEC)
	/*leave user definition as is. */
#else
#define	I_VEC	(0x0)	
#endif

/*------------------------------------------------------------------------ 
 * Non-Maskable interrupts 
 *------------------------------------------------------------------------ */
#define	I_RESET	(I_VEC+0x00)	/* Hardware RESET */
#define	I_STACK	(I_VEC+0x02)	/* Stack Error */
#define	I_ILL	(I_VEC+0x04)	/* Illegal Instruction */
#define	I_DBG	(I_VEC+0x06)	/* Debug Request       */
#define	I_TRAP	(I_VEC+0x08)	/* Trap */
#define	I_NMI	(I_VEC+0x0A)	/* Non Maskable Interrupt */

/*------------------------------------------------------------------------ 
 * Interrupt Request Pins 
 *------------------------------------------------------------------------ */
#define	I_IRQA	(I_VEC+0x10)	/* IRQA */
#define	I_IRQB	(I_VEC+0x12)	/* IRQB */
#define	I_IRQC	(I_VEC+0x14)	/* IRQC */
#define	I_IRQD	(I_VEC+0x16)	/* IRQD */

/*------------------------------------------------------------------------ 
 * DMA Interrupts 
 *------------------------------------------------------------------------ */
#define	I_DMA0	(I_VEC+0x18)	/* DMA Channel 0 */
#define	I_DMA1	(I_VEC+0x1A)	/* DMA Channel 1 */
#define	I_DMA2	(I_VEC+0x1C)	/* DMA Channel 2 */
#define	I_DMA3	(I_VEC+0x1E)	/* DMA Channel 3 */
#define	I_DMA4	(I_VEC+0x20)	/* DMA Channel 4 */
#define	I_DMA5	(I_VEC+0x22)	/* DMA Channel 5 */

/*------------------------------------------------------------------------ 
 * Timer Interrupts 
 *------------------------------------------------------------------------ */
#define	I_TIM0C	(I_VEC+0x24)	/* TIMER 0 compare */
#define	I_TIM0OF	(I_VEC+0x26)	/* TIMER 0 overflow */
#define	I_TIM1C	(I_VEC+0x28)	/* TIMER 1 compare */
#define	I_TIM1OF	(I_VEC+0x2A)	/* TIMER 1 overflow */
#define	I_TIM2C	(I_VEC+0x2C)	/* TIMER 2 compare */
#define	I_TIM2OF	(I_VEC+0x2E)	/* TIMER 2 overflow */

/*------------------------------------------------------------------------ 
 * ESSI Interrupts 
 *------------------------------------------------------------------------ */
#define	I_SI0RD	(I_VEC+0x30)	/* ESSI0 Receive Data */
#define	I_SI0RDE	(I_VEC+0x32)	/* ESSI0 Receive Data With Exception Status */
#define	I_SI0RLS	(I_VEC+0x34)	/* ESSI0 Receive last slot */
#define	I_SI0TD	(I_VEC+0x36)	/* ESSI0 Transmit data */
#define	I_SI0TDE	(I_VEC+0x38)	/* ESSI0 Transmit Data With Exception Status */
#define	I_SI0TLS	(I_VEC+0x3A)	/* ESSI0 Transmit last slot */
#define	I_SI1RD	(I_VEC+0x40)	/* ESSI1 Receive Data */
#define	I_SI1RDE	(I_VEC+0x42)	/* ESSI1 Receive Data With Exception Status */
#define	I_SI1RLS	(I_VEC+0x44)	/* ESSI1 Receive last slot */
#define	I_SI1TD	(I_VEC+0x46)	/* ESSI1 Transmit data */
#define	I_SI1TDE	(I_VEC+0x48)	/* ESSI1 Transmit Data With Exception Status */
#define	I_SI1TLS	(I_VEC+0x4A)	/* ESSI1 Transmit last slot */

/*------------------------------------------------------------------------ 
 * SCI Interrupts 
 *------------------------------------------------------------------------ */
#define	I_SCIRD	(I_VEC+0x50)	/* SCI Receive Data  */
#define	I_SCIRDE	(I_VEC+0x52)	/* SCI Receive Data With Exception Status */
#define	I_SCITD	(I_VEC+0x54)	/* SCI Transmit Data */
#define	I_SCIIL	(I_VEC+0x56)	/* SCI Idle Line */
#define	I_SCITM	(I_VEC+0x58)	/* SCI Timer */

/*------------------------------------------------------------------------ 
 * HOST Interrupts 
 *------------------------------------------------------------------------ */
#define	I_HRDF	(I_VEC+0x60)	/* Host Receive Data Full */
#define	I_HTDE	(I_VEC+0x62)	/* Host Transmit Data Empty */
#define	I_HC	(I_VEC+0x64)	/* Default Host Command */

/*------------------------------------------------------------------------ 
 * INTERRUPT ENDING ADDRESS 
 *------------------------------------------------------------------------ */
#define	I_INTEND	(I_VEC+0xFF)	/* last address of interrupt vector space */
