/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Q5: the reference solution.
 */

#include <stdint.h>

/* (a) A strict < keeps the first job met among equal deadlines: the
 * lower index wins the tie, as the statement asks. */
unsigned edd_pick( const uint32_t *d, const uint8_t *done, unsigned n )
{
    unsigned best = n;
    for ( unsigned i = 0; i < n; i++ )
    {
        if ( !done[i] && ( best == n || d[i] < d[best] ) )
        {
            best = i;
        }
    }
    return best;
}

/* (b) Run the jobs in EDD order until job i has run: its finish time is
 * the sum of its own c and the c of every job before it. */
uint32_t edd_finish( const uint32_t *c, const uint32_t *d, unsigned n, unsigned i )
{
    uint8_t done[8] = { 0 };
    uint32_t now = 0;
    for ( ;; )
    {
        unsigned k = edd_pick( d, done, n );
        now += c[k];
        done[k] = 1;
        if ( k == i )
        {
            return now;
        }
    }
}

/* (c) Lateness is signed: a schedule that meets every deadline has a
 * negative or zero maximum, and that value is the answer. */
int32_t edd_lmax( const uint32_t *c, const uint32_t *d, unsigned n )
{
    int32_t worst = ( int32_t ) edd_finish( c, d, n, 0 ) - ( int32_t ) d[0];
    for ( unsigned i = 1; i < n; i++ )
    {
        int32_t late = ( int32_t ) edd_finish( c, d, n, i ) - ( int32_t ) d[i];
        if ( late > worst )
        {
            worst = late;
        }
    }
    return worst;
}
