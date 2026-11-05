/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Two locks written in C on the atomic operations of lib/atomic.h.
 *
 * The test-and-set lock: the spinlock of lib/spinlock.s, with the same
 * spinlock_t word (0 = free, 1 = taken) and the same behaviour (yield
 * while it is taken), written in C over atomic_exchange.
 *
 * The ticket lock: a lock that serves the tasks in the order they ask.
 * Two words, both 0 at the start:
 *   next    - the next ticket to hand out;
 *   serving - the ticket that may enter now.
 * Taking the lock: take a ticket (my = next, and next = my + 1, in one
 * step), then wait, yielding, until serving == my. Giving it back:
 * serving = serving + 1, which lets the next ticket in. A task that asks
 * while another waits gets a later ticket, so it cannot enter first.
 *
 * Like a spinlock, a ticket_t goes in the shared block (SHARED,
 * src/shared.h), so both tasks see the same words.
 */
#ifndef LOCKS_H
#define LOCKS_H

#include <stdint.h>
#include "lib/spinlock.h"

/* Take the lock; while it is taken, yield (task_yield) and try again. */
void tas_lock( spinlock_t *lock );

/* Give the lock back. Only the holder calls it. */
void tas_unlock( spinlock_t *lock );

typedef struct
{
    volatile uint32_t next;     /* next ticket to hand out */
    volatile uint32_t serving;  /* ticket allowed in now */
} ticket_t;

/* Take a ticket and wait for it, yielding while it is not served. */
void ticket_lock( ticket_t *t );

/* Give the lock back: serve the next ticket. Only the holder calls it. */
void ticket_unlock( ticket_t *t );

#endif /* LOCKS_H */
