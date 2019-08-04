/*
** SCCS ID: @(#)common.h    1.1 4/2/19
**
** File:    common.h
**
** Author:  CSCI-452 class of 20185
**
** Contributor: Josh Bicking
**
** Description: Common definitions for the baseline system.
*/

#ifndef _COMMON_H_
#define _COMMON_H_

/*
** General (C and/or assembly) definitions
*/

// generic null pointer (defined as a purist would define
// it, which means it's also usable in assembly code)

#define NULL    0

// predefined I/O stream identifiers

#define STR_CONS    0
#define STR_SIO     1

// maximum number of simultaneous processes the system will support

#define MAX_PROCS   25

#ifndef __SP_ASM__

/*
** Start of C-only definitions
*/

// time-related definitions

// clock interrupts per second

#define CLOCK_FREQUENCY     1000
#define TICKS_PER_MS        1

// conversion functions to go between seconds, ms, and ticks

#define SEC_TO_MS(n)            ((n) * 1000)
#define MS_TO_TICKS(n)          ((n))
#define SEC_TO_TICKS(n)         (MS_TO_TICKS(SEC_TO_MS(n)))
#define TICKS_TO_SEC(n)         ((n) / CLOCK_FREQUENCY)
#define TICKS_TO_SEC_ROUNDED(n) (((n)+(CLOCK_FREQUENCY-1)) / CLOCK_FREQUENCY)

/*
** Types
*/

// pull in the standard system type definitions
#include "types.h"

// return values for system calls

#define E_SUCCESS     (0)
#define E_ERROR       (-1)
#define E_BAD_PID     (-2)
#define E_BAD_PRIO    (-3)
#define E_BAD_STREAM  (-4)
#define E_NO_PROCS    (-5)
#define E_NO_CHILDREN (-6)
#define E_NO_DATA     (-7)
#define E_BAD_CODE    (-8)
#define E_NO_MEM      (-9)

// Mqueue-specific
#define E_MQS_MAX     (-10)
#define E_MQDS_MAX    (-11)
#define E_MQ_DNE      (-12)
#define E_MQ_BIG_KEY  (-13)
#define E_MQ_FULL     (-14)
#define E_MQ_EMPTY    (-15)
#define E_MQ_BIG_SIZE (-16)
#define E_MQ_BAD_BUF  (-17)

///
// process priorities (defined here because user processes need them)
//
// Our ready queue operates as a four-level MLQ structure which uses
// process priority to select the appropriate queue level.
//
// Important:  new priorities should be added immediately before
// PRIO_DEFERRED so that PRIO_DEFERRED is always the last entry in the
// set of priorities; this ensures that the subsequent CPP macros have
// the correct values.
///

enum prio_e {
    PRIO_SYS = 0,   // system processes
    PRIO_USER_H = 1,    // high-priority user processes
    PRIO_USER_S = 2,    // standard-priority user processes
    PRIO_DEFERRED   // lowest-priority processes
};

// range of priorities
#define FIRST_PRIO      PRIO_SYS
#define LAST_PRIO       PRIO_DEFERRED

/*
** Message queue definitions
*/
#define MQ_MAX          128   // Max # of mqueues that can exist at a time
#define MQ_MAX_OPEN     8     // Max # of mqueues a process can open at once
#define MQ_MAX_KEY_LEN  31    // Max # of characters (excluding \0) a key can be

#define MQ_MAX_MSG_SIZE 4096  // Max bytes of one message
#define MQ_MAX_MSGS     128   // Max # of messages in one queue

// mqueue priorities
#define MQ_PRIO_HIGHEST  0
#define MQ_PRIO_LOWEST   4
#define VALID_MQ_PRIO(x) (x >= MQ_PRIO_HIGHEST && x <= MQ_PRIO_LOWEST)

/*
** Additional OS-only or user-only things
*/

#ifdef __SP_KERNEL__

/*
** OS-only includes and definitions
*/

// The OS also needs the standard system headers

#include "cio.h"
#include "memory.h"
#include "support.h"
#include "system.h"
#include "klib.h"

// pseudo-functions for (e.g.) parameter checking

#define NULLPCHECK(p)   if( (p) == NULL ) return( BAD_PARAM );
#define NULLPQCHECK(p)  if( (p) == NULL ) return;

// tracing options
//
// these bits are set when defining the TRACE macro in the Makefile

#ifdef TRACE

// bits for selecting desired trace
#define TRACE_PCB       0x01
#define TRACE_SYSRET    0x02
#define TRACE_EXIT      0x04
#define TRACE_STACK     0x08
#define TRACE_SIO_ISR   0x10
#define TRACE_SIO_WR    0x20
#define TRACE_SYSCALLS  0x40

// expressions for testing trace options
#define TRACING_PCB         ((TRACE & TRACE_PCB) != 0)
#define TRACING_SYSRET      ((TRACE & TRACE_SYSRET) != 0)
#define TRACING_EXIT        ((TRACE & TRACE_EXIT) != 0)
#define TRACING_STACK       ((TRACE & TRACE_STACK) != 0)
#define TRACING_SIO_ISR     ((TRACE & TRACE_SIO_ISR) != 0)
#define TRACING_SIO_WR      ((TRACE & TRACE_SIO_WR) != 0)
#define TRACING_SYSCALLS    ((TRACE & TRACE_SYSCALLS) != 0)

#else

#define TRACING_PCB         0
#define TRACING_SYSRET      0
#define TRACING_EXIT        0
#define TRACING_STACK       0
#define TRACING_SIO_ISR     0
#define TRACING_SIO_WR      0
#define TRACING_SYSCALLS    0

#endif

#else

// User code needs the user library headers

#include "ulib.h"

#endif

#endif

#endif
