/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * The counter updates declared in src/counter.h.
 */

#include "src/counter.h"
#include "lib/atomic.h"

/* The "add 1" step of the counter update, as a separate function.
 * Why: the call, the return and the small loop split the update into
 * separate steps (load, add, store), as on a real CPU, where a tick
 * can arrive between any two instructions. QEMU only takes an
 * interrupt between blocks of straight-line code, so in one short
 * expression the update would never be cut. */
uint32_t plus_one( uint32_t value )
{
    for ( volatile uint32_t pause = 0; pause < 10; pause++ )
    {
        /* a short pause: it makes the gap between load and store longer */
    }
    return value + 1;                           /* the add */
}

void counter_add_fetchadd( volatile uint32_t *p )
{
    atomic_fetch_add( p, 1 );                   /* read, add 1, write: one step */
}

uint32_t counter_add_cas( volatile uint32_t *p )
{
    uint32_t failed = 0;
    uint32_t old = *p;                          /* the value the update starts from */
    while ( atomic_cas( p, old, plus_one( old ) ) != old )  /* changed meanwhile: not written */
    {
        failed++;
        old = *p;                               /* start again from the new value */
    }
    return failed;
}
