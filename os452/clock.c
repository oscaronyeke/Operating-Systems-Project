/*
** SCCS ID:	@(#)clock.c	1.1	4/2/19
**
** File:   clock.c
**
** Author:   Warren R. Carithers and various CSCI-452 sections
**
** Contributor:
**
** Description:   Clock module implementation
*/

#define	__SP_KERNEL__

#include <x86arch.h>
#include <x86pic.h>
#include <x86pit.h>

#include "common.h"
#include "klib.h"

#include "clock.h"
#include "pcbs.h"
#include "queues.h"
#include "scheduler.h"
#include "sio.h"
#include "syscalls.h"

/*
** PRIVATE DEFINITIONS
*/

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

// pinwheel control variables

static int32_t _pinwheel;   // pinwheel counter
static uint32_t _pindex;   // index into pinwheel string

/*
** PUBLIC GLOBAL VARIABLES
*/

uint32_t _system_time;      // the current system time

/*
** PRIVATE FUNCTIONS
*/


/*
** _clk_isr(vector,ecode)
**
** Interrupt handler for the clock module.  Spins the pinwheel,
** wakes up sleeping processes, and handles quantum expiration
** for the current process.
*/
static void _clk_isr( int vector, int ecode ) {

    // spin the pinwheel

    ++_pinwheel;
    if( _pinwheel == (CLOCK_FREQUENCY / 10) ) {
        _pinwheel = 0;
        ++_pindex;
        // __cio_putchar_at( 79, 0, "|/-\\"[ _pindex & 3 ] );
        __cio_putchar_at( 0, 0, "|/-\\"[ _pindex & 3 ] );
    }

#if defined(STATUS)
    // Periodically, dump the queue lengths and the SIO status (along
    // with the SIO buffers, if non-empty).

    if( (_system_time % SEC_TO_TICKS(STATUS)) == 0 ) {
	__cio_printf_at( 3, 0,
	    "%3d procs:  sl/%d wt/%d rd/%d zo/%d  r %d/%d/%d/%d     ",
	    _active_procs,
            _queue_length(_sleeping), _queue_length(_waiting),
            _queue_length(_reading), _queue_length(_zombie),
	    _queue_length(_ready[0]), _queue_length(_ready[1]),
	    _queue_length(_ready[2]), _queue_length(_ready[3])
	);
        _sio_dump( true );
        // _active_dump( "Ptbl", false );
    }
#endif

    // increment the system time
    ++_system_time;

    /*
    ** wake up any sleeper whose time has come
    **
    ** we give awakened processes preference over the
    ** current process (when it is scheduled again)
    */

    qnode_t *qnode = _sleeping->head;
    while( qnode && (uint32_t) (qnode->key) <= _system_time ) {
        void *pcb;
        status_t status = _queue_remove( _sleeping, (void **) &pcb );
        if( status != SUCCESS ) {
            _kpanic( "_clk_isr", "sleep queue remove failed" );
        }
        _schedule( pcb );
        qnode = _sleeping->head;
    }

    // check the current process to see if it needs to be scheduled
    _current->quantum_left -= 1;
    if( _current->quantum_left < 1 ) {
        _schedule( _current );
        _dispatch();
    }

    // tell the PIC we're done
    __outb( PIC_MASTER_CMD_PORT, PIC_EOI );
}

/*
** PUBLIC FUNCTIONS
*/

/*
** _clk_init()
**
** initialize the clock module
*/

void _clk_init( void ) {
    uint32_t divisor;

    // announce that we got this far

    __cio_puts( " CLK" );

    // start the pinwheel

    _pinwheel = (CLOCK_FREQUENCY / 10) - 1;
    _pindex = 0;

    // return to the epoch

    _system_time = 0;

    // set the clock to tick at CLOCK_FREQUENCY Hz.

    divisor = TIMER_FREQUENCY / CLOCK_FREQUENCY;
    __outb( TIMER_CONTROL_PORT, TIMER_0_LOAD | TIMER_0_SQUARE );
    __outb( TIMER_0_PORT, divisor & 0xff );        // LSB of divisor
    __outb( TIMER_0_PORT, (divisor >> 8) & 0xff ); // MSB of divisor

    // register the ISR
    __install_isr( INT_VEC_TIMER, _clk_isr );
}
