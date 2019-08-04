/*
** SCCS ID:	@(#)pcbs.c	1.1	4/2/19
**
** File:	pcbs.c
**
** Author:	CSCI-452 class of 20185
**
** Contributor:
**
** Description:	PCB-related implementations
*/

#define	__SP_KERNEL__

#include "common.h"

#include "pcbs.h"

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

// we'll keep a queue of available PCBs to save some allocation time
queue_t _free_pcbs;    // queue of unused PCBs

// process table - one entry for each "active" process
pcb_t *_ptable[MAX_PROCS];

// number of "active" processes
uint32_t _active_procs;

// initial user process
pcb_t *_init_pcb;

// next available PID
int32_t _next_pid;

// table of priority names
char _prios[N_PRIOS][4] = { "SY", "UH", "US", "DF" };

// table of state names
char _states[N_STATES][4] = {
    "UNU", "NEW", "RDY", "RUN", "SLE", "BLK", "WTG", "ZOM"
};

/*
** PRIVATE FUNCTIONS
*/

/*
** PUBLIC FUNCTIONS
*/

/*
** _pcb_init()
**
** initializes all PCB-related data structures
*/
void _pcb_init( void ) {

    // announce our presence
    __cio_puts( " PCB" );

    // allocate the free PCB queue
    _free_pcbs = _queue_alloc( NULL );
    if( _free_pcbs == NULL ) {
        _kpanic( "_pcb_init", "can't alloc PCB queue" );
    }

    // initialize the process table and related variables
    _active_procs = 0;
    for( int i = 0; i < MAX_PROCS; ++i ) {
        _ptable[i] = NULL;
    }

    // PID for the first (non-init) process
    _next_pid = PID_FIRSTUSER;

}

/*
** _pcb_alloc()
**
** allocate a PCB structure
**
** returns a pointer to the PCB, or NULL on failure
*/
pcb_t *_pcb_alloc( void ) {
    pcb_t *new;
    status_t status;

    // first, check to see if we can re-use an old one

    if( _queue_length(_free_pcbs) > 0 ) {
        status = _queue_remove( _free_pcbs, (void **) &new );
            if( status != SUCCESS ) {
                _kpanic( "_pcb_alloc", "Q remove failed" );
            }
    } else {
        new = _km_alloc( sizeof(pcb_t) );
    }

    // clear out the space

    _kmemclr( (uint8_t *) new, sizeof(pcb_t) );

    return( new );
}

/*
** _pcb_free(pcb)
**
** deallocate a PCB
*/
void _pcb_free( pcb_t *pcb ) {
    status_t status;

    if( pcb == NULL ) {
        _kpanic( "_pcb_free", "NULL argument" );
    }

    // ensure it can't be found by a PID or state search
    pcb->pid = -1;
    pcb->ppid = -1;
    pcb->state = ST_UNUSED;

    // save it for later re-use
    status = _queue_insert( _free_pcbs, (void *) pcb, NULL );
    if( status != SUCCESS ) {
        _kpanic( "_pcb_free", "cannot insert in free PCB queue" );
    }
}

/*
** _pcb_locate(pcb,cmp)
**
** locate a PCB using the supplied comparison function
**
** this comparison function takes two PCB pointers as parameters
** and returns TRUE if they contain the correct values (e.g., PIDs
** match, or one's PID matches the other's PPID)
**
** the supplied 'pcb' parameter will always be given to the
** comparison function as the FIRST parameter.
*/
pcb_t *_pcb_locate( pcb_t *pcb, bool (*cmp)(void*,void*) ) {
    int i, checked;

#if TRACING_PCB
__cio_printf( "pcb locate: pcb %08x", pcb );
#endif
    i = checked = 0;
    while( i < MAX_PROCS && checked < _active_procs ) {
        if( _ptable[i] != NULL ) {
            if( cmp(pcb,_ptable[i]) ) {
#if TRACING_PCB
__cio_printf( " found %08x\n", _ptable[i] );
#endif
                return( _ptable[i] );
            }
            ++checked;
        }
        ++i;
    }
#if TRACING_PCB
__cio_puts( " not found???\n" );
#endif
    return( NULL );
}

/*
** _pcb_cleanup(pcb)
**
** reclaim all resources from this process
*/
void _pcb_cleanup( pcb_t *pcb ) {

	// if the parameter is NULL, don't do anything
        NULLPQCHECK(pcb);

	// get the process table index for this PCB
        uint8_t ix = pcb->pt_index;

        // sanity check #2
        if( ix >= MAX_PROCS ) {
                _kpanic( "_pcb_cleanup", "bad _ptable index for PCB" );
        }

	// sanity check #3
        if( _ptable[ix] != pcb ) {
                _kpanic( "_pcb_cleanup", "incorrect _ptable index for PCB" );
        }

	// remove this from the "active processes" list
        _ptable[ix] = NULL;
        _active_procs -= 1;

	// release all the allocated space for this process
        _stk_free( pcb->stack );
        _pcb_free( pcb );
}

/*
** _pcb_dump(msg,pcb)
**
** dump the contents of this PCB to the console
*/
void _pcb_dump( const char *msg, pcb_t *pcb ) {

    __cio_printf( "%s @ %08x: ", msg, (uint32_t) pcb );
    if( pcb == NULL ) {
        __cio_puts( " NULL???\n" );
        return;
    }

    __cio_printf( " pids %d/%d state %d", pcb->pid, pcb->ppid, pcb->state );

    __cio_printf( "\n kids %d prio %d ticks %d xit %d",
                  pcb->children, pcb->priority, pcb->quantum_left,
                  pcb->exitstatus );

    __cio_printf( "\n context %08x stack %08x\n",
                  (uint32_t) pcb->context,
                  (uint32_t) pcb->stack );
}

/*
** _context_dump(msg,context)
**
** dump the contents of this process context to the console
*/
void _context_dump( const char *msg, context_t *context ) {

    __cio_printf( "%s @ %08x: ", msg, (uint32_t) context );
    if( context == NULL ) {
        __cio_puts( " NULL???\n" );
        return;
    }

    __cio_printf( "\n      ss %08x  gs %08x  fs %08x  es %08x\n",
                  context->ss, context->gs, context->fs, context->es );
    __cio_printf( "      ds %08x edi %08x esi %08x ebp %08x\n",
                  context->ds, context->edi, context->esi, context->ebp );
    __cio_printf( "     esp %08x ebx %08x edx %08x ecx %08x\n",
                  context->esp, context->ebx, context->edx, context->ecx );
    __cio_printf( "     eax %08x vec %08x cod %08x eip %08x\n",
                  context->eax, context->vector, context->code, context->eip );
    __cio_printf( "      cs %08x efl %08x\n", context->cs, context->eflags );
}

/*
** _context_dump_all(msg)
**
** dump the process context for all active processes
*/
void _context_dump_all( const char *msg ) {

    __cio_printf( "%s: ", msg );
    __cio_printf( "%d active processes\n", _active_procs );

    if( _active_procs < 1 ) {
        return;
    }

    int n = 0;
    for( int i = 0; i < MAX_PROCS; ++i ) {
        if( _ptable[i] != NULL ) {
	    ++n;
	    context_t *c = _ptable[i]->context;
	    __cio_printf( "%2d[%2d]: ", n, i );
	    __cio_printf( "ss %04x gs %04x fs %04x es %04x ds %04x cs %04x\n",
	                  c->ss, c->gs, c->fs, c->es, c->ds, c->cs );
	    __cio_printf( "  edi %08x esi %08x ebp %08x esp %08x\n",
	                  c->edi, c->esi, c->ebp, c->esp );
	    __cio_printf( "  ebx %08x edx %08x ecx %08x eax %08x\n",
	                  c->ebx, c->edx, c->ecx, c->eax );
	    __cio_printf( "  vec %08x cod %08x eip %08x eflags %08x\n",
	                  c->vector, c->code, c->eip, c->eflags );
	}
    }
}

/*
** _active_dump(msg,all)
**
** dump the contents of the "active processes" table
*/
void _active_dump( const char *msg, bool all ) {

    if( msg ) {
        __cio_printf( "%s: ", msg );
    }

    int used = 0;
    int empty = 0;

    for( int i = 0; i < MAX_PROCS; ++i ) {
        if( _ptable[i] != NULL ) {

            // if not dumping everything, add commas if needed
            if( !all && used ) {
                __cio_putchar( ',' );
            }

            // things that are always printed
            __cio_printf( " #%d: %d/%d", i, _ptable[i]->pid, _ptable[i]->ppid );
            if( VALID_STATE(_ptable[i]->state) ) {
                __cio_printf( " %s", _states[_ptable[i]->state] );
            } else {
                __cio_printf( " ?(%d)", _ptable[i]->state );
            }
	    if( VALID_PRIO(_ptable[i]->priority) ) {
	        __cio_printf( " pr %s", _prios[_ptable[i]->priority] );
	    } else {
                __cio_printf( " pr ?(%d)", _ptable[i]->priority );
	    }

            // do we want more info?
            if( all ) {
                __cio_printf( " stk %08x EIP %08x\n",
		              (uint32_t)_ptable[i]->stack,
			      _ptable[i]->context->eip );
            }

            // one more non-empty slot
            ++used;

        } else {

            ++empty;

        }
    }

    // only need this if we're doing one-line output
    if( !all ) {
        __cio_putchar( '\n' );
    }

    // make sure we saw the correct number of slots in the table
    if( (used + empty) != MAX_PROCS ) {
        __cio_printf( "Table size %d, used %d + empty %d = %d???\n",
                      MAX_PROCS, used, empty, used + empty );
    }
}

/*
** Comparison functions for _pcb_locate()
*/

/*
** _find_parent_of(p1,p2)
**
** returns TRUE if p2 is the parent of p1
*/
bool _find_parent_of( register void *p1, register void *p2 ) {
    return( ((pcb_t *)p1)->ppid == ((pcb_t *)p2)->pid );
}

/*
** _find_child_of(p1,p2)
**
** returns TRUE if p2 is the child of p1
*/
bool _find_child_of( register void *p1, register void *p2 ) {
    return( ((pcb_t *)p1)->pid == ((pcb_t *)p2)->ppid );
}

/*
** _find_process(p1,p2)
**
** returns TRUE if p1 and p2 have the same pid
*/
bool _find_process( register void *p1, register void *p2 ) {
    return( ((pcb_t *)p1)->pid == ((pcb_t *)p2)->pid );
}

/*
** _find_zombie_child(p1,p2)
**
** returns TRUE if p2 is a child of p1 and is a zombie
*/
bool _find_zombie_child( register void *p1, register void *p2 ) {
    return( ((pcb_t *)p1)->pid == ((pcb_t *)p2)->ppid &&
            ((pcb_t *)p2)->state == ST_ZOMBIE );
}

/*
** _find_by_pid(pid,p2)
**
** returns TRUE if 'pid' is the PID of p2
*/
bool _find_by_pid( register void *pid, register void *p2 ) {
    return( (int32_t) pid == ((pcb_t *)p2)->pid );
}
