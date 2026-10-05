/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Text formatting: see format.h.
 */

#include "lib/format.h"

/* Write value in the given base (10 or 16) into buf. */
static unsigned utoa_base( uint32_t value, unsigned base, char *buf )
{
    char tmp[10];       /* digits come out last-first: keep them here */
    unsigned n = 0;     /* how many digits so far */
    unsigned i;

    /* Take the last digit with "% base", then drop it with "/ base".
     * A do-while makes sure that 0 prints one digit, "0". */
    do
    {
        unsigned d = value % base;                      /* last digit, 0..base-1 */
        tmp[n++] = (char) ( d < 10 ? '0' + d : 'a' + ( d - 10 ) );
        value /= base;                                  /* remove that digit */
    } while ( value != 0 );

    /* Copy the digits in reverse order, so the first digit comes first. */
    for ( i = 0; i < n; i++ )
    {
        buf[i] = tmp[n - 1 - i];
    }
    buf[n] = '\0';      /* end of string */
    return n;
}

unsigned utoa_dec( uint32_t value, char *buf )
{
    return utoa_base( value, 10, buf );
}

unsigned utoa_hex( uint32_t value, char *buf )
{
    return utoa_base( value, 16, buf );
}

/* CLASS: vformat (the % dispatch: %c %s %d %u %x %%) goes here. */
