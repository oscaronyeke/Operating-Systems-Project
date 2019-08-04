/*
**	sccs id: @(#)sio.h	1.1        09/27/97
**
**	File: sio.h
**
**	Description:
**
**		This file contains a description of the registers
**		and bits associated with the serial I/O ports.
*/

#ifndef _SIO_H
#define _SIO_H


/*
** Control register functions, write register 0
*/
#define	WR0_NULL		0x00	/* null code */
#define	WR0_RESET_EXT_STATUS	0x10	/* reset external status */
#define	WR0_SEND_ABORT		0x18	/* send abort */
#define	WR0_RX_INT_NABL		0x20	/* enable rcvr int on next char */
#define	WR0_RESET_TX_INT_PEND	0x28	/* reset transmitter int pend */
#define	WR0_ERROR_RESET		0x30	/* error reset */
#define	WR0_RESET_HIGHEST_IUS	0x38	/* reset highest int under service*/

#define	WR0_SEL_R0		0x00	/* select register 0 */
#define	WR0_SEL_R1		0x01	/* select register 1 */
#define	WR0_SEL_R2		0x02	/* select register 2 */
#define	WR0_SEL_R3		0x03	/* select register 3 */
#define	WR0_SEL_R4		0x04	/* select register 4 */
#define	WR0_SEL_R5		0x05	/* select register 5 */
#define	WR0_SEL_R6		0x06	/* select register 6 */
#define	WR0_SEL_R7		0x07	/* select register 7 */
#define	WR0_SEL_R8		0x08	/* select register 8 */
#define	WR0_SEL_R9		0x09	/* select register 9 */
#define	WR0_SEL_R10		0x0A	/* select register 10 */
#define	WR0_SEL_R11		0x0B	/* select register 11 */
#define	WR0_SEL_R12		0x0C	/* select register 12 */
#define	WR0_SEL_R13		0x0D	/* select register 13 */
#define	WR0_SEL_R14		0x0E	/* select register 14 */
#define	WR0_SEL_R15		0x0F	/* select register 15 */

/*
** Control register functions, write register 1
*/
#define	WR1_RX_INT_DABL		0x00	/* disable receiver interrupts */
#define	WR1_RX_INT_FIRST_CHAR	0x08	/* interrupt on 1st char only */
#define	WR1_RX_INT_ALL_OR_SC	0x10	/* int on all chars or specl. cond. */

#define	WR1_RX_INT_SC_ONLY	0x18	/* int on special condition only */

#define	WR1_PARITY_IS_SC	0x04	/* parity is the special condition */

#define	WR1_TX_INT_NABL		0x02	/* enable transmitter interrupts */
#define	WR1_EXT_INT_NABL	0x01	/* enable external interrupts */

/*
** Control register functions, write register 2
**
** none - this is where the interrupt vector number is written
*/

/*
** Control register functions, write register 3
*/
#define	WR3_RX_5_BITS		0x00	/* receive 5 bits/char */
#define	WR3_RX_7_BITS		0x40	/* receive 7 bits/char */
#define	WR3_RX_6_BITS		0x80	/* receive 6 bits/char */
#define	WR3_RX_8_BITS		0xc0	/* receive 8 bits/char */


#define	WR3_RX_NABL		0x01	/* receiver enable */

/*
** Control register functions, write register 4
*/
#define	WR4_CLOCK_1		0x00	/* straight through mode */
#define	WR4_CLOCK_16		0x40	/* divide clock by 16 */
#define	WR4_CLOCK_32		0x80	/* divide clock by 32 */
#define	WR4_CLOCK_65		0xc0	/* divide clock by 64 */

#define	WR4_1_STOP		0x04	/* one stop bit */
#define	WR4_1_5_STOP		0x08	/* 1.5 stop bits */
#define	WR4_2_STOP		0x0c	/* two stop bits */

#define	WR4_ODD_PARITY		0x02	/* odd parity */
#define	WR4_PARITY_NABL		0x01	/* enable parity */

/*
** Control register functions, write register 5
*/
#define	WR5_DTR			0x80	/* state of DTR line */

#define	WR5_TX_5_BITS		0x00	/* send 5 bits/char */
#define	WR5_TX_7_BITS		0x20	/* send 7 bits/char */
#define	WR5_TX_6_BITS		0x40	/* send 6 bits/char */
#define	WR5_TX_8_BITS		0x60	/* send 8 bits/char */

#define	WR5_BREAK		0x10	/* send a BREAK */
#define	WR5_TX_NABL		0x08	/* enable transmission */
#define	WR5_RTS			0x02	/* state of RTS line */

/*
** Control register functions, write register 9
*/
#define	WR9_CH_A_RESET		0x80	/* reset channel A */
#define	WR9_CH_B_RESET		0x40	/* reset channel B */
#define	WR9_MASTER_INT_NABL	0x08	/* master interrupt enable */
#define	WR9_MASTER_INT_DABL	0x00	/* master interrupt disable */
#define	WR9_STATUS_HIGH		0x00	/* put intrpt stat in hi bits of vect */
#define	WR9_STATUS_LOW		0x10	/* put status in low bits of vector*/
#define	WR9_VIS			0x01	/* vector includes status */

/*
** Control register functions, write register 12
*/
#define	WR12_BAUD_9600_LO	0x0a
#define	WR12_BAUD_19200_LO	0x04

/*
** Control register functions, write register 13
*/
#define	WR13_BAUD_9600_HI	0x00
#define	WR13_BAUD_19200_HI	0x00

/*
** Control register functions, read register 0
*/
#define	RR0_BREAK		0x80	/* BREAK condition */
#define	RR0_CTS			0x20	/* CTS line */
#define	RR0_DCD			0x08	/* DCD line */
#define	RR0_TX_BUF_EMPTY	0x04	/* transmitter buffer empty */
#define	RR0_ZERO_COUNT		0x02	/* zero count reached */

#define	RR0_RX_CHAR_AVAIL	0x01	/* received character available */

/*
** Control register functions, read register 1
*/
#define	RR1_FRAMING_ERR		0x40	/* framing error */
#define	RR1_OVERRUN_ERR		0x20	/* overrun */
#define	RR1_PARITY_ERR		0x10	/* parity error */
#define	RR1_ALL_SENT		0x01	/* all chars in xmtr have been sent */

/*
** Control register functions, read register 2
**
** none - this is where you can read back the interrupt vector number
*/

/*
** Control register functions, read register 3
*/

#define	RR3_CHAN_A_RX_IP	0x20	/* channel A rx int pending */
#define	RR3_CHAN_A_TX_IP	0x10	/* channel A tx int pending */
#define	RR3_CHAN_A_EX_IP	0x08	/* channel A ext stat int pending */
#define	RR3_CHAN_B_RX_IP	0x04	/* channel B rx int pending */
#define	RR3_CHAN_B_TX_IP	0x02	/* channel B tx int pending */
#define	RR3_CHAN_B_EX_IP	0x01	/* channel A ext stat int pending */

/*
** Control register functions, read register 10
*/

#define	RR10_1CLK_MISSING	0x80    /* one clock missing */
#define	RR10_2CLK_MISSING	0x40    /* two clock missing */
#define	RR10_SDLC_LP_SENDING	0x10    /* Tx sending in SDLC mode */
#define	RR10_SDLC_ON_LP		0x02    /* SCC on loop */

/*
** Control register functions, read register 13
*/

#define	RR13_BRK_IE		0x80	/* break enable */
#define	RR13_ABRT_IER		0x80	/* abort enable */
#define	RR13_TX_URUN_IE		0x40	/* Tx underrun enable */
#define	RR13_CTS_IE		0x20	/* CTS enable */
#define	RR13_SYNC_IE		0x10	/* sync enable */
#define	RR13_DCD_IE		0x08	/* DCD enable */
#define	RR13_BRG_0_CNT_IE	0x02	/* BRG zero count enable */

/*
** Definitions kept for compatability with earlier versions of this file
*/
#define	WR0_RESET_HIGHEST_UIS	WR0_RESET_HIGHEST_IUS

#define	WR1_NABL_TX_INT		WR1_TX_INT_NABL
#define	WR1_EXT_INT_ENABLE	WR1_EXT_INT_NABL
#define	WR0_RX_IEN		WR0_RX_INT_NABL

#endif
