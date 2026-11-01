/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Q2: the reference solution.
 */

#include <stdint.h>

/* (a) ⌈r / t⌉ with integers: (r + t - 1) / t. r / t alone rounds down and
 * forgets the job released inside the window but not at its start. */
uint32_t rta_interference( const uint32_t *c, const uint32_t *t, unsigned i, uint32_t r )
{
    uint32_t sum = 0;
    for ( unsigned j = 0; j < i; j++ )
    {
        sum += ( ( r + t[j] - 1 ) / t[j] ) * c[j];
    }
    return sum;
}

/* (b) The values never decrease, so the loop ends: at a fixed point, or
 * at the first value above the deadline, which is returned as it is. */
uint32_t rta_response( const uint32_t *c, const uint32_t *t, unsigned i )
{
    uint32_t r = c[i];
    for ( ;; )
    {
        uint32_t next = c[i] + rta_interference( c, t, i, r );
        if ( next == r || next > t[i] )
        {
            return next;
        }
        r = next;
    }
}

/* (c) Every task, not only the last one: a task in the middle can miss
 * while a lower one, with a long period, still meets its deadline. */
int rta_schedulable( const uint32_t *c, const uint32_t *t, unsigned n )
{
    for ( unsigned i = 0; i < n; i++ )
    {
        if ( rta_response( c, t, i ) > t[i] )
        {
            return 0;
        }
    }
    return 1;
}
