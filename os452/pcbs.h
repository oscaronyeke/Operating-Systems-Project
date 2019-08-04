/*
** SCCS ID:	@(#)pcbs.h	1.1	4/2/19
**
** File:	pcbs.h
**
** Author:	CSCI-452 class of 20185
**
** Contributor:
**
** Description:	PCB-related declarations
*/

#ifndef _PCBS_H_
#define _PCBS_H_

#include "common.h"

/*
** General (C and/or assembly) definitions
*/

// the init process is always PID 1
#define	PID_INIT	1

// other user processes have PIDs beginning at 100
#define	PID_FIRSTUSER	100

#ifndef __SP_ASM__

/*
** Start of C-only definitions
*/

// REG(pcb,x) -- access a specific register in a process context

#define	REG(pcb,x)	((pcb)->context->x)

// RET(pcb) -- access return value register in a process context

#define	RET(pcb)	((pcb)->context->eax)

// ARG(pcb,n) -- access argument #n from the indicated process
//
// ARG(pcb,0) --> return address
// ARG(pcb,1) --> first parameter
// ARG(pcb,2) --> second parameter
// etc.
//
// ASSUMES THE STANDARD 32-BIT ABI, WITH PARAMETERS PUSHED ONTO THE
// STACK.  IF THE PARAMETER PASSING MECHANISM CHANGES, SO MUST THIS!

#define	ARG(pcb,n)	( ( (uint32_t *) (((pcb)->context) + 1) ) [(n)] )

/*
** Types
*/

///
// process priorities
//
// these are defined in common.h (so that user processes can see
// them), but we define some CPP macros here that are based on
// the priority values.  These depend on the settings in common.h!
///

#define	N_PRIOS		(LAST_PRIO + 1)

#define	VALID_PRIO(n)	((n) >= PRIO_SYS && (n) <= LAST_PRIO)

///
// process states
//
// Important:  new states should be added immediately before ST_ZOMBIE
// so that ST_ZOMBIE is always the last entry in the set of states;
// this ensures that the subsequent CPP macros have the correct values.
///

enum states_e {
    ST_UNUSED = 0,
    ST_NEW = 1,
    ST_READY = 2,
    ST_RUNNING = 3,
    ST_SLEEPING = 4,
    ST_BLOCKED = 5,
    ST_WAITING = 6,
    ST_ZOMBIE
};

// range of states a process can actually be in
#define	FIRST_STATE	ST_NEW
#define	LAST_STATE	ST_ZOMBIE

#define	N_STATES	(LAST_STATE+1)

#define	VALID_STATE(n)	((n) >= ST_UNUSED && (n) <= LAST_STATE)

///
// process context structure
//
// NOTE:  the order of data members here depends on the
// register save code in isr_stubs.S!!!!
///

typedef struct context {
   uint32_t ss;
   uint32_t gs;
   uint32_t fs;
   uint32_t es;
   uint32_t ds;
   uint32_t edi;
   uint32_t esi;
   uint32_t ebp;
   uint32_t esp;
   uint32_t ebx;
   uint32_t edx;
   uint32_t ecx;
   uint32_t eax;
   uint32_t vector;
   uint32_t code;
   uint32_t eip;
   uint32_t cs;
   uint32_t eflags;
} context_t;

// include these here after the context definition

#include "stacks.h"
#include "queues.h"

///
// process control block
//
// members are ordered by size to eliminate padding between
// members; however, the whole structure will be filled out
// to a multiple of four bytes by the compiler
///

typedef struct pcb_s {
   // 32-bit items
   context_t  *context;		// context save area pointer
   stack_t    *stack;		// per-process runtime stack
   int32_t    pid;		// unique process id
   int32_t    ppid;		// PID of parent
   int32_t    exitstatus;	// exit status for zombies
   uint32_t   wakeup;		// wakeup time for sleeping process

   // 16-bit items
   uint16_t   children;		// count of currently-active children

   // 8-bit items
   uint8_t    state;		// current process state
   uint8_t    priority;		// scheduling priority
   uint8_t    quantum_left;	// remaining execution quantum
   uint8_t    pt_index;		// index into process table

   // 8-bit arrays
   int8_t    mqds[MQ_MAX_OPEN]; // message queue descriptors belonging to the process
} pcb_t;

/*
** Globals
*/

extern queue_t _free_pcbs;		// queue of unused PCBs

extern pcb_t *_ptable[MAX_PROCS];	// table of "active" processes
extern uint32_t _active_procs;		// number of "active" processes

extern pcb_t *_init_pcb;		// initial user process

extern int32_t _next_pid;		// next available PID

extern char _prios[N_PRIOS][4];		// table of priority names
extern char _states[N_STATES][4];	// table of state names

/*
** Prototypes
*/

/*
** _pcb_init()
**
** initializes all PCB-related data structures
*/
void _pcb_init( void );

/*
** _pcb_alloc()
**
** allocate a PCB structure
**
** returns a pointer to the PCB, or NULL on failure
*/
pcb_t *_pcb_alloc( void );

/*
** _pcb_free(pcb)
**
** deallocate a PCB
*/
void _pcb_free( pcb_t *pcb );

/*
** _pcb_locate(pcb,cmp)
**
** locate a PCB using the supplied comparison function
**
** the comparison function takes two PCB pointers as void* parameters
** and returns TRUE if they contain the correct values (e.g., PIDs
** match, or one's PID matches the other's PPID)
*/
pcb_t *_pcb_locate( pcb_t *pcb, bool (*cmp)(void*,void*) );

/*
** _pcb_cleanup(pcb)
**
** reclaim all resources from this process
*/
void _pcb_cleanup( pcb_t *pcb );

/*
** _pcb_dump(msg,pcb)
**
** dump the contents of this PCB to the console
*/
void _pcb_dump( const char *msg, pcb_t *pcb );

/*
** _context_dump(msg,context)
**
** dump the contents of this process context to the console
*/
void _context_dump( const char *msg, context_t *context );

/*
** _context_dump_all(msg)
**
** dump the process context for all active processes
*/
void _context_dump_all( const char *msg );

/*
** _active_dump(msg,all)
**
** dump the contents of the "active processes" table
*/
void _active_dump( const char *msg, bool all );

/*
** Comparison functions for _pcb_locate()
*/

/*
** _find_parent_of(p1,p2)
**
** returns TRUE if p2 is the parent of p1
*/
bool _find_parent_of( void *p1, void *p2 );

/*
** _find_child_of(p1,p2)
**
** returns TRUE if p2 is the child of p1
*/
bool _find_child_of( void *p1, void *p2 );

/*
** _find_process(p1,p2)
**
** returns TRUE if p1 and p2 have the same PID
*/
bool _find_process( void *p1, void *p2 );

/*
** _find_zombie_child(p1,p2)
**
** returns TRUE if p2 is a child of p1 and is a zombie
*/
bool _find_zombie_child( void *p1, void *p2 );

/*
** _find_by_pid(pid,p2)
**
** returns TRUE if 'pid' is the PID of p2
*/
bool _find_by_pid( void *pid, void *p2 );

#endif

#endif
