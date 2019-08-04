/*
** File:	mqueues.c
**
** Author:	Josh Bicking
**
** Contributor:
**
** Description:	Implementation of UNIX-esque message queues for IPC
*/

#include "common.h"
#include "queues.h"
#include "mqueues.h"
#include "pcbs.h"
#include "scheduler.h"
#include "klib.h"
#include "memory.h"
#include "cio.h"

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

mqueue_t _mqueues[MQ_MAX];

/*
** PRIVATE FUNCTIONS
*/

/*
** Predicate function, given to _queue_remove_by(mq->msgs ...). Finds a node of
** a priority equal to, or higher than, the requested priority.
*/
static bool geq_prio(void* msg, void* prio) {
    return ((msg_t *) msg)->priority <= (uint8_t)(uint32_t) prio;
}

/*
** Predicate function, given to _queue_remove_by(mq->reading ...). Finds a
** blocked reader of a priority equal to, or higher than, the requested
** priority.
*/
static bool geq_proc_prio(void* pcb, void* prio) {
    return (uint8_t) ARG((pcb_t *) pcb, 4) <= (uint8_t)(uint32_t) prio;
}

/*
** PUBLIC FUNCTIONS
*/

/*
** Essentially a stub for mqueue initialization. Any setup that would need
** doing, should be done here.
*/
void _mq_init() {
  // For now, we just let the world know we exist :)
  __cio_puts( " MQUEUES" );
}

/*
** Validate a process read/write to an mqueue.
**
** - Check that the MQD is valid, and the process owns it.
** - Ensure the buffer isn't NULL.
** - Ensure a valid message length & priority.
*/
int16_t _mq_validate(pcb_t *pcb) {
  int16_t mqd = ARG(pcb, 1);
  uint8_t* buf = (uint8_t*) ARG(pcb, 2);
  uint32_t msg_len = ARG(pcb, 3);
  uint8_t prio = ARG(pcb, 4);

  if (mqd <= 0) {
    // Invalid mqd
    return E_MQ_DNE;
  }

  // Ensure the mqd is in the pcb's table of mqds
  int16_t mqd_loc = 0;
  for (uint8_t i=1; i<MQ_MAX_OPEN; i++ ) {
    if (pcb->mqds[i] == mqd) {
      mqd_loc = i;
      // Ensure the mqueue hasn't been closed
      if (_mqueues[mqd] == NULL) {
        mqd_loc = 0;
      }
      break;
    }
  }
  if (mqd_loc == 0) {
    return E_MQ_DNE;
  }

  if (msg_len > MQ_MAX_MSG_SIZE) {
    return E_MQ_BIG_SIZE;
  }

  if (buf == NULL) {
    return E_MQ_BAD_BUF;
  }

  if (!VALID_MQ_PRIO(prio)) {
    return E_BAD_PRIO;
  }

  // Input is validated. We can try to read now.
  return 0;
}

/*
** Move a message from an mqueue into a buffer.
**
** If the mqueue is empty, or no messages meet the priority requirement, the
** process is placed on the mqueue's blocked readers queue.
**
** Assumes the PCB has been validated with _mq_validate.
*/
void _mq_read(pcb_t* pcb) {
  int16_t mqd = ARG(pcb, 1);
  uint8_t* buf = (uint8_t*) ARG(pcb, 2);
  uint32_t msg_len = ARG(pcb, 3);
  uint8_t prio = ARG(pcb, 4);
  bool block = ARG(pcb, 5);
  mqueue_t mq = _mqueues[mqd];

  msg_t * msg;
  status_t status = _queue_remove_by(mq->msgs, (void **) &msg,
                                     (void *) (uint32_t) prio, geq_prio);

  if (status != SUCCESS) {
    /* Uh oh, there's nothing to read. Either the queue is empty, or nothing of
     * our priority level is there.
     *
     * How does the program want us to handle this?
     */
    if (block) {
      // Onto the reading queue you go!
      pcb->state = ST_BLOCKED;
      status_t status = _queue_insert( mq->reading, (void *) pcb, NULL);
      if (status != SUCCESS) {
        // We're out of memory, so we can't block on this.
        pcb->state = ST_RUNNING;
        RET(pcb) = E_NO_MEM;
        return;
      }
      // select a new current process
      _dispatch();
      return;
    } else {
      RET(pcb) = E_MQ_EMPTY;
      return;
    }
  }

  if (msg->size < msg_len) {
    msg_len = msg->size;
  }
  _kmemcpy(buf, msg->data, msg_len);
  _km_free(msg->data);
  RET(pcb) = msg_len;

  // We read, which means there's a free spot! Was someone waiting to write?
  if (_queue_length(mq->writing) > 0) {
    pcb_t * writer_pcb;
    status = _queue_remove(mq->writing, (void **) &writer_pcb);
    if (status != SUCCESS) {
      // This _really_ shouldn't happen, but...
      _kpanic("_mq_read", "Failed to remove a qnode from blocked writers.");
    }
    _mq_write(writer_pcb);
    _schedule(writer_pcb);
  }
  return;
}

/*
** Move a message from a buffer into an mqueue.
**
** If the mqueue is full, the process is placed on the mqueue's blocked writers
** queue.
**
** Assumes the PCB has been validated with _mq_validate.
*/
void _mq_write(pcb_t* pcb) {
  int16_t mqd = ARG(pcb, 1);
  uint8_t* buf = (uint8_t*) ARG(pcb, 2);
  uint32_t msg_len = ARG(pcb, 3);
  uint8_t prio = ARG(pcb, 4);
  bool block = ARG(pcb, 5);

  mqueue_t mq = _mqueues[mqd];
  if (_queue_length(mq->msgs) == MQ_MAX_MSGS) {
    if (block) {
      pcb->state = ST_BLOCKED;
      status_t status = _queue_insert( mq->writing, (void *) pcb, NULL);
      if (status != SUCCESS) {
        // We're out of memory, so we can't block on this.
        pcb->state = ST_RUNNING;
        RET(pcb) = E_NO_MEM;
        return;
      }
      // select a new current process
      _dispatch();
      return;
    } else {
      RET(pcb) = E_MQ_FULL;
      return;
    }
  }

  // There's a spot available, so alloc and try the write.
  msg_t * msg = _km_alloc(sizeof(msg_t));
  msg->data = _km_alloc(msg_len);
  _kmemcpy(msg->data, buf, msg_len);
  msg->size = msg_len;
  msg->priority = prio;

  status_t status = _queue_insert(mq->msgs, (void *) msg, NULL);

  if (status != SUCCESS) {
    RET(pcb) = E_NO_MEM;
    _km_free(msg->data);
    _km_free(msg);
    return;
  }
  RET(pcb) = 0;

  // Write complete. Maybe someone can read this.
  if (_queue_length(mq->reading) > 0) {
    // See if a reader is looking for something of this priority level.
    pcb_t * reader_pcb;
    status = _queue_remove_by(mq->reading, (void **) &reader_pcb,
                              (void *) (uint32_t) prio, geq_proc_prio);
    if (status == NOT_FOUND) {
        // Can't say we didn't try.
        return;
    }
    if (status != SUCCESS) {
      // Shouldn't get any other statuses, but...
      _kpanic("_mq_write", "Failed to remove a qnode from blocked readers.");
    }
    _mq_read(reader_pcb);
    _schedule(reader_pcb);
  }
}
