/*
**	sccs id: @(#)r4600.h	1.4        11/11/99
**
**	File:	r4600.h
**
**	Description:
**
**		Definitions of constants and macros for use
**		with MIPS R4600 devices and registers.
**
*/

#ifndef _R4600_H
#define	_R4600_H

/*******************************************
** Other architecture-specific include files
*******************************************/

#include <sys/IP22.h>
#include <sys/hpc3.h>

/***********************
** System initialization
***********************/

#define	KSEG1_BASE		0xa0000000

#define	GE_VECTOR	(*(int*)(0xa0001000+0x14))
#define	TLBMISS_VECTOR	(*(int*)(0xa0001000+0x18))
#define	FLUSH_CACHES	((void (*)())(*(int*)((*(int*)(0xa0001000+0x20))+0x88)))

/***********
** Debugging
***********/

#define	DB_PR	((void (*)())(*(int*)((*(int*)(0xa0001000+0x20))+0x6c)))
#define	DEBUG_PRINT(STRING,LEN)	{ int n; DB_PR(1,STRING,LEN,&n); }

/************
** Exceptions
************/

#define	N_EXCEPTIONS              32

/**********************************
** Bit definitions in CP0 registers
**********************************/

/*
** Status Register (CP0:12)
**
**     Field	Bit(s)	Name
**     -----	------	---------------------------------------------------
**	CU	31..28	Coprocessor Usability bits (CP3, CP2, CP1, CP0)
**	RP	27	Reduced Power (0=full speed, 1=slower clock)
**	FR	26	Floating Registers (0=16, 1=32)
**	RE	25	Reverse Endian
**	DS	24-16	Diagnostic Status
**	IM	15-8	Interrupt Mask
**	KX	7	Kernel eXtended addressing (0=32-bit, 1=64-bit)
**	SX	6	Supervisor eXtended addressing (0=32-bit, 1=64-bit)
**	UX	5	User eXtended addressing (0=32-bit, 1=64-bit)
**	KSU	4-3	Kernel-Supervisor-User mode (00, 01, 10; 11 unused)
**	ERL	2	ERror Level (0=normal, 1=error)
**	EXL	1	EXception Level (0=normal, 1=exception)
**	IE	0	Interrupt Enable (0=disable all, 1=enable per IM)
*/
#define	SR_CU			0xf0000000
#define	SR_RP			0x08000000
#define SR_FR			0x04000000
#define SR_RE			0x02000000
#define SR_DS			0x01ff0000
#define SR_IM			0x0000ff00
#define SR_KX			0x00000080
#define SR_SX			0x00000040
#define SR_UX			0x00000020
#define SR_KSU			0x00000018
#define SR_ERL			0x00000004
#define SR_EXL			0x00000002
#define SR_IE			0x00000001

/* IM bits */
#define	INT_ENABLE_MASK		0x0000ff00
#define	INT_7_ENABLE		0x00008000
#define	INT_6_ENABLE		0x00004000
#define	INT_5_ENABLE		0x00002000
#define	INT_4_ENABLE		0x00001000
#define	INT_3_ENABLE		0x00000800
#define	INT_2_ENABLE		0x00000400
#define	INT_1_ENABLE		0x00000200
#define	INT_0_ENABLE		0x00000100

#define	CLOCK_ENABLE 		INT_7_ENABLE
#define	LEVEL1_ENABLE		INT_3_ENABLE

#define	INTERRUPT_CODE		0x00000000
#define	SYSCALL_CODE		0x00000020
#define	TRAP_CODE		0x00000034

#define	MASTER_INT_ENABLE	SR_IE

#define	USER_MODE		0xf0000010
#define	SUPERVISOR_MODE		0xf0000008
#define	KERNEL_MODE		0xf0000000

/*
** Cause Register (CP0:13)
**
**     Field	Bit(s)	Name
**     -----	------	---------------------------------------------------
**	BD	31	last exception was in a Branch Delay slot (1=yes)
**	-	30	must be zero
**	CE	29-28	Coprocessor unusable Exception - CP unit referenced
**	-	27-16	must be zero
**	IP	15-8	Interrupt Pending (bits correspond to IM in SR)
**	-	7	must be zero
**	EXC	6-2	EXception Code
**	-	1-0	must be zero
**
*/
#define	EXC_CODE_MASK		0x0000007c

#define	EXC_INTERRUPT		0x00000000
#define	EXC_TLB_MOD		0x00000004
#define	EXC_TLB_LOAD_IF		0x00000008
#define	EXC_TLB_STORE		0x0000000c
#define	EXC_ADDR_ERR_LOAD_IF	0x00000010
#define	EXC_ADDR_ERR_STORE	0x00000014
#define	EXC_BUS_ERROR_IF	0x00000018
#define	EXC_BUS_ERROR_DATA	0x0000001c
#define	EXC_SYSCALL		0x00000020
#define	EXC_BREAKPOINT		0x00000024
#define	EXC_RES_INSTR		0x00000028
#define	EXC_CO_UNUSABLE		0x0000002c
#define	EXC_ARITH_OVERFLOW	0x00000030
#define	EXC_TRAP		0x00000034
#define	EXC_VC_INSTR		0x00000038
#define	EXC_FLOATING_PT		0x0000003c
/* Codes 16 (0x40) through 22 (0x58) reserved */
#define	EXC_WATCH		0x0000005c
/* Codes 24 (0x60) through 30 (0x78) reserved */
#define	EXC_VC_DATA		0x0000007c

#define	INT_PEND_MASK		0x0000ff00
#define	INT_7_PENDING		0x00008000
#define	INT_6_PENDING		0x00004000
#define	INT_5_PENDING		0x00002000
#define	INT_4_PENDING		0x00001000
#define	INT_3_PENDING		0x00000800
#define	INT_2_PENDING		0x00000400
#define	INT_1_PENDING		0x00000200
#define	INT_0_PENDING		0x00000100

#define	CLOCK_PENDING		INT_7_PENDING
#define	LEVEL1_PENDING		INT_3_PENDING
#define	SIO_PENDING		INT_3_PENDING
#define	LEVEL0_PENDING		INT_2_PENDING

/*************************
** Memory-mapped registers
*************************/

/* Local Interrupt registers and masks */
#define	LOCAL0_STATUS_REG	(KSEG1_BASE | (0x1fbd9880+3))
#define	LOCAL0_MASK_REG		(KSEG1_BASE | (0x1fbd9884+3))
#define	LOCAL1_STATUS_REG	(KSEG1_BASE | (0x1fbd9888+3))
#define	LOCAL1_MASK_REG		(KSEG1_BASE | (0x1fbd988c+3))

#define	MAP_STATUS_REG		(KSEG1_BASE | (0x1fbd9890+3))
#define	MAP_MASK0_REG		(KSEG1_BASE | (0x1fbd9894+3))
#define	MAP_MASK1_REG		(KSEG1_BASE | (0x1fbd9898+3))
#define	MAP_POLARITY_REG	(KSEG1_BASE | (0x1fbd989c+3))

/* bit set in Local Status Registers when SIO/KBM has interrupted */
#define	LSR0_INT_MASK		0x80
#define	LSR1_INT_MASK		0x08

/* bits set in Map Status Register when SIO/KBM has interrupted */
#define	MSR_SIO_INT_MASK	0x20
#define	MSR_KBM_INT_MASK	0x10

/* enable SIO interrupts in Local Mask Registers and Map Mask Registers */
#define	SIO_LMR0_INT_ENABLE	LSR0_INT_MASK
#define	SIO_LMR1_INT_ENABLE	LSR1_INT_MASK
#define	SIO_MMR0_INT_ENABLE	0x20
#define	SIO_MMR1_INT_ENABLE	0x20

/* enable KBM (keyboard/mouse) interrupts in LMRs and MMRs */
#define	KBM_L0_INT_ENABLE	0x90
#define	KBM_L1_INT_ENABLE	0x18

/* Map Polarity Register definitions */
#define	MAP_POL_SIO_ENABLE	0xcf

/*********
** Devices
*********/

#define	N_DEVS			3

/* Device numbers. */
#define	DEV_A			0
#define	DEV_B			1
#define	DEV_DEBUG		2

#define	CHAN_A			DEV_A
#define	CHAN_B			DEV_B

/* Serial Communication Controller addresses */
#define	SIO_CTRL_A		(KSEG1_BASE | (HPC3_SERIAL1_CMD+3))
#define	SIO_DATA_A		(KSEG1_BASE | (HPC3_SERIAL1_DATA+3))
#define	SIO_CTRL_B		(KSEG1_BASE | (HPC3_SERIAL0_CMD+3))
#define	SIO_DATA_B		(KSEG1_BASE | (HPC3_SERIAL0_DATA+3))

/* Keyboard/Mouse Communication Controller addresses */
#define	KBM_MOUSE_0		(KSEG1_BASE | (HPC3_KBD_MOUSE0+3))
#define	KBM_MOUSE_1		(KSEG1_BASE | (HPC3_KBD_MOUSE1+3))

/* Macros for SCC interaction */
#define	SCC_FLUSHBUS		*(volatile int *)(HPC3_INTSTAT_ADDR | KSEG1_BASE)
#define	READ_SCC(P)		( Delay(1), *((char *)P) )
#define	WRITE_SCC(P, V)		{ Delay(1); *((char *)P)=(V); SCC_FLUSHBUS; }

/* Macros for device register interaction */
#define	READ_REG(P)		( Delay(1), *((char *)P) )
#define	WRITE_REG(P, V)		{ Delay(1); *((char *)P)=(V); }

#endif
