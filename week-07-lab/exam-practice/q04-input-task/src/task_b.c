/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Task B, the reader. It runs unprivileged, like task A, and has the
 * higher priority (kernel/task.h). It prints every byte it reads, until
 * it reads 'q'; then it prints the scheduler trace and returns.
 */

#include <stdint.h>
#include "src/tasks.h"
#include "lib/syscalls.h"
#include "lib/printf.h"

void task_b( void )
{
    printf( "B: hello from task B\n" );
    for ( ;; )
    {
        int c;
        /* sys_read returns -1 when no byte has arrived: ask again. */
        while ( ( c = sys_read() ) < 0 )
        {
        }
        if ( c == 'q' )
        {
            break;
        }
        printf( "B: got %c (tick %u)\n", c, task_tick_count() );
    }

    /* Last action: print the trace, so the run ends with the chart. */
    trace_dump();

    /* Returning ends the task: the lr of its first frame is task_exit. */
}
