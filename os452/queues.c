/*
** SCCS ID:	@(#)queues.c	1.1	4/2/19
**
** File:	queues.c
**
** Author:	CSCI-452 class of 20185
**
** Contributor:
**
** Description:	Queue module
*/

#define	__SP_KERNEL__

#include "common.h"

#include "queues.h"

/*
** PRIVATE DEFINITIONS
*/

// number of qnodes to preallocate (4 * MAX_PROCS)

#define	N_QNODES	(MAX_PROCS << 2)

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

// list of allocated but not currently used qnodes
static qnode_t *_free_qnodes;

/*
** PUBLIC GLOBAL VARIABLES
*/

/*
** PRIVATE FUNCTIONS
*/

/*
** These functions manage qnodes.  They are local to the queue
** module, and should not be callable by the rest of the system.
*/

/*
** _qnode_alloc - allocate a qnode and initialize it
*/
static qnode_t *_qnode_alloc( void *data, void *key ) {
    qnode_t *new;
    
    // see if we have any already-allocated ones hanging
    // around; if not, carve another one from the heap

    if( _free_qnodes ) {

        // yes!  recycling saves!
        new = _free_qnodes;
        _free_qnodes = new->next;

    } else {

        // nope - grab a new one
        new = _km_alloc( sizeof(qnode_t) );
        if( new == NULL ) {
            return( NULL );
        }

    }
    
    // fill in the space we just grabbed
    new->data = data;
    new->key = key;
    new->next = NULL;
    
    // and return it
    return( new );
}

/*
** _qnode_free - return a qnode to the wild
*/
static void _qnode_free( qnode_t *old ) {

    if( old == NULL ) {
        _kpanic( "_qnode_free", "attempt to free NULL" );
    }
    
    // add this one to the beginning of the free list
    old->next = _free_qnodes;
    _free_qnodes = old;
}

/*
** _qnode_init - preallocate some number of qnodes
*/
static void _qnode_init( void ) {
    _free_qnodes = NULL;
    for( int i = 0; i < N_QNODES; ++i ) {
        qnode_t *qn = _km_alloc( sizeof(qnode_t) );
        _qnode_free( qn );
    }
}

/*
** PUBLIC FUNCTIONS
*/

/*
** _queue_init - initialize the queue module
*/
void _queue_init( void ) {

    // announce that we were called
    __cio_puts( " QUEUE" );
    
    // set up the initial collection of qnodes
    _qnode_init();

    // whatever else we need to do here
}

/*
** _queue_alloc - allocate and initialize a queue structure
**
** note: we never deallocate these, so _queue_free() doesn't exist!
*/
queue_t _queue_alloc( int (*cmp)(void*,void*) ) {
    queue_t new;
    
    new = (queue_t) _km_alloc( sizeof(struct q_s) );
    if( new != NULL ) {
        new->head = new->tail = NULL;
        new->length = 0;
        new->compare = cmp;
    }
    
    return( new );
}

/*
** _queue_insert - insert a key+data pair into a queue
*/
status_t _queue_insert( queue_t q, void *data, void *key ) {
    qnode_t *qnode;
    
    // SANITY CHECK
    // NULL q:  BAD_PARAM
    NULLPCHECK(q);
    
    // allocate and initialize the qnode
    qnode = _qnode_alloc( data, key );
    if( qnode == NULL ) {
        return( BAD_ALLOC );
    }
    
    // first case:  insert into an empty queue
    if( _queue_length(q) == 0 ) {
        q->head = q->tail = qnode;
        q->length = 1;
        return( SUCCESS );
    }
    
    // second case:  insert into a FIFO (i.e., unordered) queue
    if( q->compare == NULL ) {
        q->tail->next = qnode;
        q->tail = qnode;
        q->length += 1;
        return( SUCCESS );
    }
    
    // third case:  insert into an ordered queue

    // traverse the queue looking for the insertion point
    qnode_t *prev = NULL;
    qnode_t *curr = q->head;
    while( curr && q->compare(curr->data,qnode->data) <= 0 ) {
        prev = curr;
        curr = curr->next;
    }
    
    //  prev  curr  meaning
    //  ====  ====  ================
    //   0     0    ERROR
    //   0     !0   add at front of list
    //   !0    0    add at end of list
    //   !0    !0   add in middle of list
    
    // link to the new node's successor
    qnode->next = curr;  // always
    
    // now, connect the predecessor to this node

    if( prev == NULL ) {

        // there is no predecessor

        if( curr == NULL ) {
            // this should never happen!
            _kpanic( "_queue_insert", "no prev or curr in non-empty Q" );
        }

        // add at front
        q->head = qnode;

    } else {

        // there is a predecessor

        prev->next = qnode;

        // if there isn't a successor, we're the new "last node"
        if( curr == NULL ) {
            q->tail = qnode;
        }

    }
    
    // the queue is now longer
    q->length += 1;
    
    return( SUCCESS );
}

/*
** _queue_remove - remove the first node from a queue
*/
status_t _queue_remove( queue_t q, void **data ) {
    qnode_t *node;
    
    // SANITY CHECK
    NULLPCHECK(q);
    
    // can't get water out of a stone!
    if( _queue_length(q) < 1 ) {
        return( EMPTY );
    }
    
    // take the first node from the queue
    node = q->head;
    q->head = node->next;

    // decrement the length; if it's zero,
    // this was the only node in the queue

    if( (q->length -= 1) == 0 ) {
        q->tail = NULL;
    }
    
    // return the data
    *data = node->data;

    // free the qnode
    _qnode_free( node );
    
    return( SUCCESS );
}

/*
** _queue_remove_by - remove a specific node from a queue
*/
status_t _queue_remove_by( queue_t q, void **result, void *data,
                           bool (*cmp)(void*,void*) ) {
    qnode_t *curr, *prev;
    
    // SANITY CHECK
    NULLPCHECK(q);
    
    // can't get water out of a stone!
    if( _queue_length(q) < 1 ) {
        return( EMPTY );
    }

    // locate the indicated node in the queue
    prev = NULL;
    curr = q->head;

    while( curr && !cmp(curr->data,data) ) {
        prev = curr;
        curr = curr->next;
    }
    
    // did we find it?
    if( curr == NULL ) {
        return( NOT_FOUND );
    }

    // found it - return the data pointer
    *result = curr->data;

    // unlink this node from the qnode list
    if( prev ) {  // first entry?
        prev->next = curr->next;  // no
    } else {
        q->head = curr->next; // yes
    }

    q->length -= 1;

    // was this the only node in the list?
    if( q->head == NULL ) {
        q->tail = NULL;  // yes
    }

    // free the qnode
    _qnode_free( curr );
    
    return( SUCCESS );
}

/*
** _queue_dump(msg,q)
**
** dump the contents of a queue to the console
*/
void _queue_dump( const char *msg, queue_t q ) {
    qnode_t *tmp;
    int i;

    // report on this queue
    __cio_printf( "%s: ", msg );
    if( q == NULL ) {
        __cio_puts( "NULL???\n" );
        return;
    }

    // first, the basic data
    __cio_printf( "head %08x tail %08x %d items",
                  (uint32_t) q->head, (uint32_t) q->tail, q->length );

    // next, how the queue is ordered
    if( q->compare ) {
        __cio_printf( " cmp %08x\n", (uint32_t) q->compare );
    } else {
        __cio_puts( " FIFO\n" );
    }

    // if there are members in the queue, dump the first five data pointers
    if( q->length > 0 ) {
        __cio_puts( " data: " );
        i = 0;
        for( tmp = q->head; i < 5 && tmp != NULL; ++i, tmp = tmp->next ) {
            __cio_printf( " [%08x]", (uint32_t) tmp->data );
        }

        if( tmp != NULL ) {
            __cio_puts( " ..." );
        }

        __cio_puts( "\n" );
    }
}

/*
** ORDERING FUNCTIONS
*/

/*
** _order_wakeup(a1,a2)
**
** used to order a queue of PCBs by wakeup time
*/
int _order_wakeup( void *a1, void *a2 ) {

    // parameters will be pointers to PCBS

    const pcb_t *p1 = (const pcb_t *) a1;
    const pcb_t *p2 = (const pcb_t *) a2;

    // wakeup times are unsigned 32-bit integers

    if( p1->wakeup < p2->wakeup ) {
        return( -1 );
    } else if( p1->wakeup == p2->wakeup ) {
        return( 0 );
    } else {
        return( 1 );
    }
}
