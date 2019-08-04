/*
** SCCS ID:	@(#)scheduler.c	1.1	4/2/19
**
** File:	scheduler.c
**
** Author:	CSCI-452 class of 20185
**
** Contributor:
**
** Description:	Scheduler module implementations
*/

#define	__SP_KERNEL__

#include "common.h"

#include "scheduler.h"

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

queue_t _ready[N_PRIOS];    // the ready queue
pcb_t *_current;            // the current process

/*
** PRIVATE FUNCTIONS
*/

/*
** PUBLIC FUNCTIONS
*/

/*
** _sched_init()
**
** initialize the scheduler module
*/
void _sched_init( void ) {
    int i;
    
    // report that we're here

    __cio_puts( " SCHED" );

    // iterate through all the queue levels

    for( i = 0; i < N_PRIOS; ++i ) {

        // allocate this level's queue
        _ready[i] = _queue_alloc( NULL );

        // if that failed, we're in deep trouble
        if( _ready[i] == NULL ) {
            __cio_printf( "ERR: sched init alloc %d failed\n", i );
            _kpanic( "_sched_init", "ready queue alloc" );
        }
    }
}

/*
** _schedule(pcb)
**
** schedule a process for execution
*/
status_t _schedule( pcb_t *pcb ) {

    // if no PCB, nothing to do
    NULLPCHECK(pcb);
    
    // verify that the priority level makes sense

    uint8_t prio = pcb->priority;
    if( !VALID_PRIO(prio) ) {
        // oops - it doesn't.  this should never happen!
        __cio_printf( "bad priority in _schedule(), pid %d, prio %d\n",
	              pcb->pid, prio );
        _kpanic( "_schedule", "bad process priority value" );
    }
    
    // all's well; just add it to the indicated queue
    pcb->state = ST_READY;

    return(
         _queue_insert( _ready[prio], (void *) pcb, (void *) (pcb->pid) )
    );
}


/*
** _dispatch()
**
** give the CPU to a process
*/
void _dispatch( void ) {
    pcb_t *pcb = NULL;
    status_t status;

#ifdef CONSOLE_SHELL
    // check to see if there is any console input
    if( __cio_input_queue() > 0 ) {
        // yes - deal with it
	_shell( 'h' );
    }
#endif

    // iterate through the levels until we find one
    // that has a process.  dispatch it.

    for( int i = 0; i < N_PRIOS; ++i ) {
        if( _queue_length(_ready[i]) > 0 ) {
            status = _queue_remove( _ready[i], (void **) &pcb );
            if( status != SUCCESS ) {
               _kpanic( "_dispatch", "ready remove failed" );
            }
            _current = pcb;
            pcb->quantum_left = QUANT_STD;
            pcb->state = ST_RUNNING;
            return;
        }
    }

    /*
    ** Uh-oh...  We looked at all the queues and didn't find
    ** anyone to dispatch.  This should never happen, as there
    ** should always be an "idle" process hanging out in the
    ** bottom-most queue.
    */
    
    _kpanic( "_dispatch", "no ready process" );
}
