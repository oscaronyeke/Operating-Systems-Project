/*
** SCCS ID:	@(#)queues.h	1.1	4/2/19
**
** File:	queues.h
**
** Author:	CSCI-452 class of 20185
**
** Contributor:
**
** Description:	Queue module declarations.
*/

#ifndef _QUEUES_H_
#define _QUEUES_H_

/*
** General (C and/or assembly) definitions
*/

#ifndef __SP_ASM__

/*
** Start of C-only definitions
*/

/*
** Pseudo-function to produce the length of a queue (can be replaced
** easily if the underlying queue implementation changes)
*/

#define _queue_length(q)	((q)->length)

/*
** Types
*/

/*
** Queue nodes - hold entries in our generic queues
*/

typedef struct qn_s {
    void *data;         // whatever has been queued
    void *key;          // access key
    struct qn_s *next;  // next entry in the queue
} qnode_t;

/*
** The queues themselves are fairly simple, with just head and tail
** pointers, an occupancy count, and a pointer to the comparison
** function to be used for ordering.
**
** A "queue" is a pointer to this data structure.  This allows us to easily
** pass them as reference parameters to all the manipulation functions.
*/

typedef struct q_s {
    qnode_t *head;      // first entry
    qnode_t *tail;      // last entry
    uint32_t length;    // occupancy count
    int (*compare)(void *,void *);  // comparison function for ordering
} *queue_t;

/*
** Globals
*/

/*
** Prototypes
*/

/*
** _queue_init - initialize the queue module
*/
void _queue_init( void );

/*
** _queue_alloc - allocate and initialize a queue structure
**
** note: we never deallocate these, so _queue_free() doesn't exist!
*/
queue_t _queue_alloc( int (*cmp)(void*,void*) );

/*
** _queue_insert - insert a key+data pair into a queue
*/
status_t _queue_insert( queue_t q, void *data, void *key );

/*
** _queue_remove - remove the first node from a queue
*/
status_t _queue_remove( queue_t q, void **data );

/*
** _queue_remove_by - remove a specific node from a queue
*/
status_t _queue_remove_by( queue_t q, void **result, void *data,
                           bool (*cmp)(void*,void*) );

/*
** _queue_dump(msg,que)
**
** dump the contents of the specified queue to the console
*/
void _queue_dump( const char *msg, queue_t q );

/*
** ORDERING FUNCTIONS
*/

/*
** _order_wakeup(a1,a2)
**
** used to order a queue of PCBs by wakeup time
*/
int _order_wakeup( void *a1, void *a2 );

#endif

#endif
