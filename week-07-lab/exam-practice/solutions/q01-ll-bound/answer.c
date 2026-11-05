/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Q1: the reference solution.
 */

#include <stdint.h>

/* (a) Multiply before dividing: (1000 * c) / t keeps three digits of the
 * fraction, while 1000 * (c / t) would truncate c / t to 0 or 1 first. */
uint32_t util_x1000( const uint32_t *c, const uint32_t *t, unsigned n )
{
    uint32_t u = 0;
    for ( unsigned i = 0; i < n; i++ )
    {
        u += ( 1000 * c[i] ) / t[i];
    }
    return u;
}

/* (b) The table of the statement, indexed by n (entry 0 unused). The
 * bound is "at most": U equal to U_lub(n) is still guaranteed. */
int ll_test( const uint32_t *c, const uint32_t *t, unsigned n )
{
    static const uint32_t ulub_x1000[11] = {
        0, 1000, 828, 780, 757, 743, 735, 729, 724, 721, 718
    };
    uint32_t u = util_x1000( c, t, n );
    if ( u <= ulub_x1000[n] )
    {
        return 2;
    }
    return u <= 1000 ? 1 : 0;
}
