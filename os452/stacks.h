/*
** SCCS ID:	@(#)stacks.h	1.1	4/2/19
**
** File:	stacks.h
**
** Author:	CSCI-452 class of 20185
**
** Contributor:
**
** Description:	Stack-related declarations
*/

#ifndef _STACKS_H_
#define _STACKS_H_

/*
** General (C and/or assembly) definitions
*/

#ifndef __SP_ASM__

/*
** Start of C-only definitions
*/

// number of 32-bit unsigned integers in a stack

#define	STACK_SIZE_U32		1024

// total size of a stack

#define	STACK_SIZE		(STACK_SIZE_U32 * sizeof(uint32_t))

/*
** Types
*/

///
// stack structure
//
// not particularly impressive, but c'est la vie
///

typedef uint32_t stack_t[STACK_SIZE_U32];

/*
** Globals
*/

// the OS stack
extern stack_t _system_stack;
extern uint32_t *_system_esp;

/*
** Prototypes
*/

/*
** _stk_init()
**
** initializes all Stack-related data structures
*/
void _stk_init( void );

/*
** _stk_alloc()
**
** allocate a Stack structure
**
** returns a pointer to the Stack, or NULL on failure
*/
stack_t *_stk_alloc( void );

/*
** _stk_free(stk)
**
** deallocate a Stack
*/
void _stk_free( stack_t *stk );

/*
** _stk_setup(stk,entry,argc,arg1,arg2)
**
** create a "fresh" stack
*/
context_t *_stk_setup( stack_t *stk, uint32_t entry, int argc,
                       void *arg1, void *arg2 );

/*
** _stk_dump(msg,stk,limit)
**
** dump the contents of this Stack to the console
*/
void _stk_dump( const char *msg, stack_t *stk, uint32_t limit );

#endif

#endif
