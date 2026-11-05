/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Q5: a place to try your functions. It includes your answer.c, so
 * the functions of the question are in scope. Add the calls you want.
 * It checks nothing.
 */

#include "console.h"
#include "answer.c"          /* your functions, from the question folder */

int main( void )
{
    uint32_t c[] = { 1, 1, 1, 3, 2 };
    uint32_t d[] = { 3, 10, 7, 8, 5 };
    uint8_t done[] = { 0, 0, 0, 0, 0 };
    kprintf( "first job = %u\n", edd_pick( d, done, 5 ) );
    kprintf( "job 3 finishes at %u\n", edd_finish( c, d, 5, 3 ) );
    kprintf( "Lmax = %d\n", edd_lmax( c, d, 5 ) );
    return 0;
}
