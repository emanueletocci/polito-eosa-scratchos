/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Task B, periodic: every B_PERIOD ticks (kernel/task.h) from tick 0,
 * a job of 4 ticks of processor time, 5 jobs in all.
 */

#include <stdint.h>
#include "src/tasks.h"
#include "kernel/task.h"
#include "lib/syscalls.h"

/* C ticks of processor time: count the distinct ticks seen while
 * running. The kernel switches only at a tick, so a task that is
 * preempted does not count the ticks it did not run. */
static void work( uint32_t c )
{
    uint32_t seen = task_tick_count(), n = 1;
    while ( n <= c )
    {
        uint32_t now = task_tick_count();
        if ( now != seen )
        {
            seen = now;
            n++;
        }
    }
}

void task_b( void )
{
    uint32_t last = 0;                      /* job 1 released at tick 0 */
    for ( unsigned k = 1; k <= 5; k++ )
    {
        work( 4 );
        task_delay_until( &last, B_PERIOD );
    }

    /* Last action: print the trace, with the misses. */
    trace_dump();
}
