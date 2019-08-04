/*
** SCCS ID:	@(#)system.c	1.1	4/2/19
**
** File:	system.c
**
** Author:	CSCI-452 class of 20185
**
** Contributor: Oscar Onyeke
**
** Description:	miscellaneous OS support functions
*/

#define	__SP_KERNEL__

#include "common.h"

#include "system.h"
#include "queues.h"
#include "clock.h"
#include "pcbs.h"
#include "bootstrap.h"
#include "syscalls.h"
#include "cio.h"
#include "sio.h"
#include "scheduler.h"
#include "mqueues.h"
#include "BuildFileSys.h

// need init() address
#include "users.h"

/*
** PRIVATE DEFINITIONS
*/

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

/*
** PUBLIC GLOBAL VARIABLES
*/

/*
** PRIVATE FUNCTIONS
*/

/*
** PUBLIC FUNCTIONS
*/

/*
** _sysinit - system initialization routine
**
** Called by the startup code immediately before returning into the
** first user process.
*/
void _sysinit( void ) {
    pcb_t *pcb;

    /*
    ** BOILERPLATE CODE - taken from basic framework
    **
    ** Initialize interrupt stuff.
    */

    __init_interrupts();  // IDT and PIC initialization

    /*
    ** Console I/O system.
    */

#ifdef INCLUDE_SHELL
    __cio_init( _shell );  // use _shell() for console input
#else
    __cio_init( NULL );    // no console callback routine
#endif
#ifdef TRACE_CX
    __cio_setscroll( 0, 7, 99, 99 );
    __cio_puts_at( 0, 6, "================================================================================" );
#endif

    /*
    ** 20185-SPECIFIC CODE STARTS HERE
    */

    /*
    ** Initialize various OS modules
    **
    ** Other modules (clock, SIO, syscall, etc.) are expected to
    ** install their own ISRs in their initialization routines.
    */

    __cio_puts( "System initialization starting\n" );
    __cio_puts( "------------------------------\n" );

    __cio_puts( "Modules:" );

    _km_init();      // kernel memory system (must be first)
    _queue_init();   // queues (must be second)

    _clk_init();     // clock
    _pcb_init();     // processes
    _sched_init();   // scheduler
    _sio_init();     // serial i/o
    _stk_init();     // stacks
    _sys_init();     // system calls
//    _mq_init();      // message queues
    _sys_buildfs(); // file system

    __cio_puts( "\nModule initialization complete\n" );
    __cio_puts( "------------------------------\n" );
    __delay( 200 );  // about 5 seconds

    /*
    ** Create the initial process
    **
    ** Code largely stolen from _sys_fork() and _sys_exec(); if
    ** either or both of those routines change, SO MUST THIS!!!
    */

    // allocate a PCB and stack

    pcb = _pcb_alloc();
    if( pcb == NULL ) {
        _kpanic( "_init", "init() pcb alloc failed" );
    }

    pcb->stack = _stk_alloc();
    if( pcb->stack == NULL ) {
        _kpanic( "_init", "init() stack alloc failed" );
    }

    // initialize the stack with the standard context

    pcb->context = _stk_setup( pcb->stack, (uint32_t) init, 0, 0, 0 );
    if( pcb->context == NULL ) {
        _kpanic( "_init", "init() stack setup failed" );
    }

    // _pcb_dump( "init()", pcb );
    // _stk_dump( "init() stack", pcb->stack, 8 );
    // __delay( 2000 );  // about 50 seconds

    // set up various PCB fields
    pcb->pid = pcb->ppid = PID_INIT;
    pcb->priority = PRIO_SYS;
    pcb->children = 0;

    // remember it
    _init_pcb = pcb;

    // put it on the ready queue
    _schedule( pcb );

    // this is our first active process

    _ptable[0] = pcb;
    _active_procs = 1;

    /*
    ** Create the idle() process
    **
    ** Philosophical question: should we create it here, or
    ** should we let init() create it?
    **
    ** We opt for the latter, as it means less work for us. :-)
    */

    /*
    ** Turn on the SIO receiver (the transmitter will be turned
    ** on/off as characters are being sent)
    */

    _sio_enable( SIO_RX );

    // dispatch the first user process

    _dispatch();

    /*
    ** END OF 20185-SPECIFIC CODE
    **
    ** Finally, report that we're all done.
    */

    __cio_puts( "System initialization complete.\n" );
    __cio_puts( "-------------------------------\n" );
}

#ifdef CONSOLE_SHELL
/*
** _shell - extremely simple shell for handling console input
**
** Called whenever we want to take input from the console and
** act upon it (e.g., for debugging the kernel)
*/
void _shell( int ch ) {

    // clear the input buffer
    ch = __cio_getchar();

    __cio_puts( "\nInteractive mode ('x' to exit)\n? " );
    ch = __cio_getchar();

    // loop until we get an "exit" indicator

    while( 1 ) {

        if( ch == 'x' || ch == EOT ) {
            break;
        }

        // c_printf( "%c\n", ch );

        switch( ch ) {

	case '\r': // ignore CR and LF
	case '\n':
	    break;

        case 'q':  // dump the queues
            _queue_dump( "Sleep queue", _sleeping );
            _queue_dump( "Waiting queue", _waiting );
            _queue_dump( "Reading queue", _reading );
            _queue_dump( "Zombie queue", _zombie );
            _queue_dump( "Ready queue 0", _ready[PRIO_SYS] );
            _queue_dump( "Ready queue 1", _ready[PRIO_USER_H] );
            _queue_dump( "Ready queue 2", _ready[PRIO_USER_S] );
            _queue_dump( "Ready queue 3", _ready[PRIO_DEFERRED] );
	    break;

	case 'a':  // dump the active table
	    _active_dump( "\nActive processes", false );
	    break;

	case 'p':  // dump the active table and all PCBs
	    _active_dump( "\nActive processes", true );
	    break;

	case 'c':  // dump context info for all active PCBs
	    _context_dump_all( "\nContext dump" );
	    break;

	case 's':  // dump stack info for all active PCBS
	    __cio_puts( "\nActive stacks (w/5-sec. delays):\n" );
	    for( int i = 0; i < MAX_PROCS; ++i ) {
	        if( _ptable[i] != NULL ) {
	            pcb_t *pcb = _ptable[i];
		    __cio_printf( "pid %5d: ", pcb->pid );
		    __cio_printf( "EIP %08x, ", pcb->context->eip );
		    _stk_dump( NULL, pcb->stack, 12 );
		    __delay( 200 );
		}
            }
	    break;
	 
	default:
	    __cio_printf( "shell: unknown request '%c'\n", ch );

	    // FALL THROUGH

	case 'h':  // help message
	    __cio_puts( "\nCommands:\n" );
	    __cio_puts( "   a  -- dump the active table\n" );
	    __cio_puts( "   c  -- dump contexts for active processes\n" );
	    __cio_puts( "   h  -- this message\n" );
	    __cio_puts( "   p  -- dump the active table and all PCBs\n" );
	    __cio_puts( "   q  -- dump the queues\n" );
	    __cio_puts( "   s  -- dump stacks for active processes\n" );
	    __cio_puts( "   x  -- exit\n" );
	    break;
        }

        __cio_puts( "\n? " );
        ch = __cio_getchar();
    }

    __cio_puts( "\nExiting interactive mode\n\n" );
}
#endif
