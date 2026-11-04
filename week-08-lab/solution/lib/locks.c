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
    while ( atomic_exchange( lock, 1 ) != 0 )   /* it was taken: not ours */
    {
        task_yield();                           /* let the holder run */
    }
    dmb();                                      /* the protected accesses start only now */
}

void tas_unlock( spinlock_t *lock )
{
    dmb();                                      /* the protected writes are done first */
    *lock = 0;                                  /* free: only the holder writes here */
}

void ticket_lock( ticket_t *t )
{
    uint32_t my = atomic_fetch_add( &t->next, 1 );  /* take a ticket */
    while ( t->serving != my )                  /* not my turn yet */
    {
        task_yield();                           /* let the holder run */
    }
    dmb();                                      /* the protected accesses start only now */
}

void ticket_unlock( ticket_t *t )
{
    dmb();                                      /* the protected writes are done first */
    t->serving = t->serving + 1;                /* serve the next ticket: only the holder writes here */
}
