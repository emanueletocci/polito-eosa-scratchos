/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Q2: a place to try your functions. It includes your answer.c, so
 * the functions of the question are in scope. Add the calls you want.
 * It checks nothing.
 */

#include "console.h"
#include "answer.c"          /* your functions, from the question folder */

int main( void )
{
    uint32_t c[] = { 1, 2, 3 };
    uint32_t t[] = { 4, 6, 12 };
    kprintf( "interference on task 2 in 7 = %u\n", rta_interference( c, t, 2, 7 ) );
    kprintf( "R of task 2 = %u\n", rta_response( c, t, 2 ) );
    kprintf( "schedulable = %d\n", rta_schedulable( c, t, 3 ) );
    return 0;
}
