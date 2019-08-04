/*
** SCCS ID:	@(#)stacks.c	1.1	4/2/19
**
** File:	stacks.c
**
** Author:	CSCI-452 class of 20185
**
** Contributor:
**
** Description:	Stack-related implementations
*/

#define	__SP_KERNEL__

#include "common.h"

#include "stacks.h"

/*
** PRIVATE DEFINITIONS
*/

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

// we'll keep a queue of available Stacks to save some allocation time
static queue_t _free_stacks;	// queue of unused Stacks

/*
** PUBLIC GLOBAL VARIABLES
*/

// the OS stack
stack_t _system_stack;
uint32_t *_system_esp;

/*
** PRIVATE FUNCTIONS
*/

/*
** PUBLIC FUNCTIONS
*/

/*
** _stk_init()
**
** initializes all Stack-related data structures
*/
void _stk_init( void ) {

   // announce our presence
   __cio_puts( " STK" );

   // allocate the free Stack queue
   _free_stacks = _queue_alloc( NULL );
   if( _free_stacks == NULL ) {
      _kpanic( "_stk_init", "can't alloc Stack queue" );
   }

   // set up the system stack pointer
   //
   // this is the stack that will be used when ISRs are invoked.
   // ESP will point to the next-to-last word in the stack.
   _system_esp = ((uint32_t *) ((&_system_stack) + 1) ) - 2;

}

/*
** _stk_alloc()
**
** allocate a Stack structure
**
** returns a pointer to the Stack, or NULL on failure
*/
stack_t *_stk_alloc( void ) {
    stack_t *new;
    status_t status;

    // first, check to see if we can re-use an old one

    if( _queue_length(_free_stacks) > 0 ) {
        status = _queue_remove( _free_stacks, (void **) &new );
	if( status != SUCCESS ) {
	    _kpanic( "_stk_alloc", "Q remove failed" );
	}
    } else {
        new = _km_alloc( sizeof(stack_t) );
    }

    // clear out the space

    if( new != NULL ) {
        _kmemclr( (uint8_t *) new, sizeof(stack_t) );
    }

    return( new );
}

/*
** _stk_free(stk)
**
** deallocate a Stack
*/
void _stk_free( stack_t *stk ) {
    status_t status;

    // if no stack, nothing to do
    if( stk == NULL ) {
        return;
    }

    // save it for later re-use
    status = _queue_insert( _free_stacks, (void *) stk, NULL );
    if( status != SUCCESS ) {
        _kpanic( "_stk_free", "insert into free queue failed" );
    }
}

/*
** _stk_setup(stk,entry,argc,arg1,arg2)
**
** create a "fresh" stack
*/
context_t *_stk_setup( stack_t *stk, uint32_t entry, int argc,
                       void *arg1, void *arg2 ) {

#if TRACING_STACK
__cio_printf( "=== _stk_setup(%08x,%08x,%08x,%d,%d)", (uint32_t) stk,
             entry, argc, (uint32_t) arg1, (uint32_t) arg2 );
#endif

    // if there is no stack, we can't do anything
    if( stk == NULL ) {
        return( NULL );
    }

    // clear the stack
    _kmemclr( (void *)stk, sizeof(stack_t) );

    /*
    ** Set up the initial stack contents for a (new) user process.
    **
    ** We reserve one longword at the bottom of the stack as scratch
    ** space.  Above that, we simulate a call from exit_helper() with an
    ** argument vector by pushing the arguments and then the argument
    ** count.  We follow this up by pushing the address of the entry point
    ** of exit_helper() as a "return address".  Above that, we place a
    ** context_t area that is initialized with the standard initial register
    ** contents.
    **
    ** The low end of the stack will contain these values:
    **
    **      esp ->  ?            <- context save area
    **              ...          <- context save area
    **              ?            <- context save area
    **              exit_helper  <- return address for faked call to main()
    **              argc         <- argument count for main()
    **              arg1         <- first "command-line" argument
    **              arg2         <- second "command-line" argument
    **              0            <- last word in stack
    **
    ** When this process is dispatched, the context restore code will
    ** pop all the saved context information off the stack, leaving the
    ** "return address" on the stack as if the main() for the process
    ** had been "called" from the exit_helper() function.  When main()
    ** returns, it will "return" to the entry point of exit_helper(),
    ** which will then call exit().
    */

    // find the address of the last uint32_t in the stack
    // uint32_t *ptr = ((uint32_t *)(stk + 1)) - 1;

    // fill in the fields
    // *ptr-- = 0;                       // dummy "last word"
    // *ptr-- = (uint32_t) arg2;         // second argument
    // *ptr-- = (uint32_t) arg1;         // first argument
    // *ptr-- = (uint32_t) argc;         // argument count
    // *ptr-- = (uint32_t) exit_helper;  // dummy "return address"

    // find the address immediately following the stack
    uint32_t *ptr = ((uint32_t *)(stk + 1));

    // fill in the fields
    *--ptr = 0;                       // dummy "last word"
    *--ptr = (uint32_t) arg2;         // second argument
    *--ptr = (uint32_t) arg1;         // first argument
    *--ptr = (uint32_t) argc;         // argument count
    *--ptr = (uint32_t) exit_helper;  // dummy "return address"

    // next, create the initial runtime context for the process

    context_t *ctx = ((context_t *) ptr) - 1;
#if TRACING_STACK
__cio_printf( " context @ %08x\n", ctx );
#endif

    // initialize all the things that need it
    ctx->eflags = DEFAULT_EFLAGS;  // PPL 0, IF enabled
    ctx->eip = entry;              // initial PC
    ctx->ebp = 0;                  // end of the EBP stack frame chain

    // must also set all the segment register contents
    ctx->cs = GDT_CODE;
    ctx->ss = GDT_STACK;
    ctx->ds = GDT_DATA;
    ctx->es = GDT_DATA;
    ctx->fs = GDT_DATA;
    ctx->gs = GDT_DATA;

#if TRACING_STACK
_context_dump( "=== new context", ctx );
_stk_dump( "=== new stack", stk, 0 );
__delay(400);
#endif

    // all done!
    //
    // note that we don't assign the context pointer to
    // the relevant field in the process' PCB - whoever
    // called us is responsible for that
    return( ctx );

}

/*
** _stk_dump(msg,stk,limit)
**
** dump the contents of the provided stack, eliding duplicate lines
**
** assumes the stack size is a multiple of four words
**
** output lines begin with the 8-digit address, followed by a hex
** interpretation then a character interpretation of four words
**
** hex needs 41 bytes:
** col pos  1         2         3         4
** 1        0         0         0         0
**   xxxxxxxx  xxxxxxxx  xxxxxxxx  xxxxxxxx
**
** character needs 22 bytes:
**             1    1  2
** 1 3    8    3    8  1
**   cccc cccc cccc cccc
*/

// round up a bit
#define	HBUFSZ		48
#define	CBUFSZ		24

void _stk_dump( const char *msg, stack_t *stack, uint32_t limit ) {
    int words = sizeof(stack_t) / sizeof(uint32_t);
    int eliding = 0;
    char oldbuf[HBUFSZ], buf[HBUFSZ], cbuf[CBUFSZ];
    uint32_t addr = (uint32_t ) stack;
    uint32_t *sp = (uint32_t *) stack;
    char hexdigits[] = "0123456789ABCDEF";

    // if a limit was specified, dump only that many words

    if( limit > 0 ) {
        words = limit;
        if( (words & 0x3) != 0 ) {
            // round up to a multiple of four
            words = (words & 0xfffffffc) + 4;
        }
        // skip to the new starting point
        sp += (STACK_SIZE_U32 - words);
        addr = (uint32_t) sp;
    }

    __cio_puts( "*** stack" );
    if( msg != NULL ) {
      __cio_printf( " (%s):\n", msg );
    } else {
      __cio_puts( ":\n" );
    }

    oldbuf[0] = '\0';

    while( words > 0 ) {

        register char *bp = buf;   // start of hex field
        register char *cp = cbuf;  // start of character field
        uint32_t start_addr = addr;

        // iterate through the words for this line

        for( int i = 0; i < 4; ++i ) {
            register uint32_t curr = *sp++;
	    register uint32_t data = curr;

	    // convert the hex representation

	    // two spaces before each entry
            *bp++ = ' ';
            *bp++ = ' ';

            for( int j = 0; j < 8; ++j ) {
                uint32_t value = (data >> 28) & 0xf;
                *bp++ = hexdigits[value];
                data <<= 4;
            }

	    // now, convert the character version
	    data = curr;

	    // one space before each entry
	    *cp++ = ' ';

	    for( int j = 0; j < 4; ++j ) {
	        uint32_t value = (data >> 24) & 0xff;
	        *cp++ = (value >= ' ' && value < 0x7f) ? (char) value : '.';
	        data <<= 8;
	    }
        }
        *bp = '\0';
        *cp = '\0';

        words -= 4;
        addr += 16;

        // if this line looks like the last one, skip it

        if( _kstrcmp(oldbuf,buf) == 0 ) {
            ++eliding;
            continue;
        }

        // it's different, so print it

        // start with the address
        __cio_printf( "%08x%c", start_addr, eliding ? '*' : ' ' );
        eliding = 0;

        // print the words
        __cio_printf( "%s %s\n", buf, cbuf );

        // remember this line
        _kmemcpy( (uint8_t *) oldbuf, (uint8_t *) buf, HBUFSZ );
    }
}
