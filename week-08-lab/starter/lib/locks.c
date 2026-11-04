/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * The locks declared in lib/locks.h. The barriers are where spin_lock
 * and spin_unlock (lib/spinlock.s) put theirs: after taking, before
 * giving back.
 */

#include "lib/locks.h"
#include "lib/atomic.h"
#include "lib/syscalls.h"

void tas_lock( spinlock_t *lock )
{
    /* B3: take the lock with atomic_exchange; yield while it is taken. */
    ( void ) lock;
}

void tas_unlock( spinlock_t *lock )
{
    /* B3: give the lock back. */
    ( void ) lock;
}

void ticket_lock( ticket_t *t )
{
    /* B4: take a ticket with atomic_fetch_add; yield until it is served. */
    ( void ) t;
}

void ticket_unlock( ticket_t *t )
{
    /* B4: serve the next ticket. */
    ( void ) t;
}
