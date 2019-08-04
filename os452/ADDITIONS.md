# Team 4

This document includes implementation and design rationale for our additions to
the baseline operating system.

# Message queues

Message queues allow for inter-process communication. User processes may open
(create), close (destroy), read from them, and write to them.

## API

### Limits and variables

Message queues have limits to keep userspace programs from running amok.

  - Max number of mqueues that can exist
  - Max number of mqueues a process can open at once
  - Max message size
  - Max number of messages in an mqueue

These are all configurable via definitions in `mqueues.h`.

### Creating and deleting message queues

#### Creating

Message queues are created with the `mq_open` system call, requiring a
NUL-terminated string of appropriate length. This will create a message queue of
that name, if it doesn't exist. It will also return a message queue descriptor
(mqd): this is what gives this process access to the message queue in other
syscalls.

#### Deleting

Message queues are deleted with the `mq_close` system call. Any process that
opened the message queue is allowed to close it. Any processes waiting on the
message queue will return with an error.

After closing the message queue, the mqd is invalid.

### Sending & receiving messages

The `mq_write` and `mq_read` syscalls handle sending and receiving of messages.
These syscalls both require a valid mqd, and a buffer containing (or, that will
contain) the message. The caller may select block and priority preferences.

#### Message priorities

Messages have a priority level from 0-4 (0 most important, 4 least important).
Priority levels do not change queue order, but allow a reader to filter out
lower priority messages. For example, calling `mq_read` with a priority of `1`
will return a message of priority `=< 1`, if one is available.

#### Blocking

When userspace reads or writes to an mqueue, they have the option to block on
that operation. Typically, a read on an empty mqueue would fail with an empty
error. However, when blocking, the process will be blocked until a message is
written.

Blocks can still fail: for example, the mqueue may close while a process is
blocking. So userspace should still check for errors from a blocked read or
write.

## Internals

### Structure and design

Under the hood, created message queue pointers are stored in a global `_mqueues`
array. An array keeps a fixed limit on the number of message queues in a system,
to help prevent abuse by user programs.

PCBs have valid message queue descriptors, stored in an `mqds` array. *An mqd is
an index of the `_mqueues` array*: this avoids giving memory addresses to
processes. It also allowed indirect reference without playing with pointers:
since virtual memory development was proceeding at the same time, not messing
with user/kernel space memory locations was a must.

By default, `mqds` is all zeroes, and since 0 is NULL, *index 0 of `_mqueues` is
invalid*. Implementation pretty much doesn't touch 0 at all in these arrays, just
to avoid indexing errors.

### Data types

Important data types include `mqueue_t` and `msg_t`.

`mqueue_t` contains the name of the mqueue, the underlying list of messages, and
two queues for any processes blocked on the mqueue. Pointers to these made up
the entries of `_mqueues`.

`msg_t` is what resides on an `mqueue_t`'s `msgs` queue. These hold the actual
data to be sent, priority information, and the size of the data they're holding.

### Validation

The underlying syscall implementations for `mq_write` and `mq_read`
(`_sys_mq_write` and `_sys_mq_read`) have a similar prodecure for input
validation. For this reason, argument validation for these syscalls is moved to
`_mq_validate`. Any PCB is run through this before being passed to `_mq_write`
or `_mq_read` respectively, which do the actual adding/removing/blocking.

Validation for `_sys_mq_open` and `_sys_mq_close` are done directly in the
functions themselves, as their validation is more unique.

### Priorities

The priority of a message is stored within a `msg_t`.

When adding a message to an mqueue, priority is ignored, and the message is
added to the end of the queue. This allows a reader to take the "oldest"
message, and disregard priority.

When reading a message from an mqueue, desired priority is compared with
priority of each message, using the `queue_t` structure's `_queue_remove_by`
function. `geq_prio` compares a desired priority number against messages, and
`geq_proc_prio` compares the priority in a blocked process's PCB. In this sense,
priority is treated like a threshold. If no messages meet the priority
requirements, it's as if the queue is empty: there are no messages available.

### Blocking

If a user process has indicated it should block on an empty/full mqueue, and
it's unable to read/write, it will be placed on the `mqueue_t`'s `reading` or
`writing` block queue.

This queue is observed once the opposite operation occurs. As an example:
  - A process blocks on a write. The queue is full, so the PCB is placed on the
    mqueue's `writing` queue.
  - Another process reads from the mqueue.
  - After the read, as part of `_mq_read`, the `writing` queue is observed, and
    the writer is dequeued. Its message is written, and the process is
    scheduled.

One key difference occurs between dequeuing from `reading` and `writing`. In
`reading`, the queue is traversed, looking for the first reader that accepts a
priority of the message just written. In the case of `writing`, however, the
writer is always dequeued, as a free space is available.

## Experiences

### A single block queue per mqueue

Only 1 queue was used for both processes blocked on read, and on write.
 - If we're at 1 message on the queue and blocked > 0, processes are waiting to
   read. Dequeue, read, place in buffer, and schedule.
 - If we're at MQ_MAX_MSGS-1 messages and blocked > 0, processes are waiting to
   write. Dequeue, write to message queue, and schedule.

This falls apart when priorities are added. Take the following example:
 - Fill an mqueue with priority 4 messages
 - Read a priority 1 message, with blocking. This process is placed on the block
   queue as a reader.
 - Another process writes to the mqueue. It's placed on the block queue are a
   writer.

#### Current implementation: separate reading and writing queues

While requiring more memory, separate reading and writing queues prevent the
above issue.
  - Upon reading any message, the writing queue is dequeued.
  - Upon writing a message, the reading queue *may be* dequeued, searching for a
    reader with >= priority than the message we just wrote.

### `typedef x y` vs `typedef x *y`

Upon first glance, I didn't notice the `queue_t` type was defined as *a pointer
to* a `struct queue_s`, and not directly as a `struct queue_s`. The initial
`mqueue_t` type was initialized with `_km_alloc(sizeof(queue_t))`, which is only
the size of a pointer. Amazingly, this worked fine when an `mqueue_t` was only a
key, a queue of messages, and a blocked queue.

When exchanging a single blocked queue for a reader and writer queue, however,
these queues became NULL right after they were initialized. The memory had been
overwritten, as the correct size wasn't allocated.

### The beauty of flags

I opted to use an int and a boolean for read/write options in syscalls, but this
soon became cumbersome. C doesn't include an easy way to express optional arguments.
After implementation, I realized specifying a priority of 4 for each read/write
call would soon become annoying for a user who didn't care about message
priority.

If I had time to refactor, I would have left a single byte for priority and
blocking. I would move the lowest priority level to 0 (all 0 bits), the highest
to 7 (three 1s), and use the highest 3 bits to express priority. Then the last
bit would indicate non-blocking or blocking, with 0 or 1. I could then expose
this through the user library, with flags such as `O_PRIO_0` - `O_PRIO_4`,
`O_NOBLOCK`, and `O_BLOCK`. A priority flag could then be ORed with a block
flag.
