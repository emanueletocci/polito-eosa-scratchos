/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Q6: a place to try your functions. It includes your answer.c, so
 * the functions and the types of the question are in scope. Add the
 * calls you want. It checks nothing.
 */

#include "console.h"
#include "answer.c"          /* your functions, from the question folder */

int main( void )
{
    /* The first ten ticks of the example: A in 0-1 and 5-6, B in 2-4 and 7-9. */
    struct sw rec[] = { { 2, 'A', 'B' }, { 5, 'B', 'A' }, { 7, 'A', 'B' } };
    struct task tasks[] = { { 'A', 2, 5 }, { 'B', 4, 7 } };
    char who = '?';
    kprintf( "B runs %u slots in [0, 10)\n", run_ticks( rec, 3, 'B', 0, 10 ) );
    kprintf( "response of B's job 1 = %u\n", job_response( rec, 3, 10, &tasks[1], 1 ) );
    uint32_t d = first_miss( rec, 3, 10, tasks, 2, &who );
    kprintf( "first miss at %u by %c\n", d, who );
    kprintf( "obeys RM = %d\n", obeys_rm( rec, 3, 10, tasks, 2 ) );
    return 0;
}
