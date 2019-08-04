/*
** File:	mqueues.h
**
** Author:	Josh Bicking
**
** Contributor:
**
** Description: Implementation of UNIX-esque message queues for IPC
*/

#ifndef _MQUEUES_H_
#define _MQUEUES_H_

#include "common.h"
#include "queues.h"
#include "pcbs.h"

/*
** Types
*/

typedef struct msg_s {
    uint32_t  size;
    uint8_t   priority;
    uint8_t * data;
} msg_t;

typedef struct mqueue_s {
    char * key;      // How userspace identifies the mqueue
    queue_t msgs;    // Underlying queue of message nodes
    queue_t reading; // Who's waiting to read from this queue?
    queue_t writing; // Who's waiting to write to this queue?
} *mqueue_t;

/*
** Globals
*/

/*
** An mqueue descriptor is an index into this array. This is to avoid giving
** memory addresses as descriptors.
**
** An mqd of 0 is invalid (to avoid null checking errors), so MAX_MQS-1 mqds may
** be given.
*/
extern mqueue_t _mqueues[];

/*
** Prototypes
*/

void _mq_init(void);
void _mq_read(pcb_t* pcb);
void _mq_write(pcb_t* pcb);
int16_t _mq_validate(pcb_t* pcb);

#endif
