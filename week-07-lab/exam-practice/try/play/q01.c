/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Q1: a place to try your functions. It includes your answer.c, so
 * the functions of the question are in scope. Add the calls you want.
 * It checks nothing.
 */

#include "console.h"
#include "answer.c"          /* your functions, from the question folder */

int main( void )
{
    uint32_t c[] = { 1, 1, 2 };
    uint32_t t[] = { 4, 5, 10 };
    kprintf( "util_x1000 = %u\n", util_x1000( c, t, 3 ) );
    kprintf( "ll_test = %d\n", ll_test( c, t, 3 ) );
    return 0;
}
