/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Task B. It runs unprivileged, so it only uses system calls
 * (lib/syscalls.h) and the helpers built on them (lib/printf.h).
 *
 * Week 7: tasks B and A run the same program on shared data. Two
 * phases (see src/tasks.h for which ones a build runs):
 *   1. counter: add 1 to shared_counter COUNT_N times. Together the
 *      tasks must reach 2 * COUNT_N.
 *   2. lines: print LINES_N lines of LINE_LEN characters, all 'B'.
 *      Each line must come out whole.
 * Without a lock (DEMO=race, counter, cpsid) updates get lost and lines
 * get mixed. The other builds take a spinlock (lib/spinlock.h) around
 * each update and each printf: LOCK / UNLOCK in src/tasks.h.
 * The last task to finish prints the result and the scheduler trace.
 *
 * Builds (see the Makefile):
 *     make run               - both phases, with the lock (spin-then-yield)
 *     make DEMO=counter run  - counter phase only, no lock
 *     make DEMO=race run     - lines phase only, no lock
 *     make DEMO=cpsid run    - lines phase, printf wrapped in cpsid/cpsie
 *     make DEMO=spin run     - both phases, lock with busy waiting
 * Task B's week 4 demos (rogue, badptr, overflow) are no longer part of
 * this week.
 */

#include <stdint.h>
#include "src/tasks.h"
#include "src/shared.h"
#include "lib/syscalls.h"
#include "lib/printf.h"

#if DO_COUNTER      /* only the counter phase needs it */
/* The "add 1" step of the counter update, as a separate function.
 * Why: the call, the return and the small loop split the update into
 * separate steps (load, add, store), as on a real CPU, where a tick
 * can arrive between any two instructions. QEMU only takes an
 * interrupt between blocks of straight-line code, so in one short
 * expression the update would never be cut. */
static uint32_t plus_one( uint32_t value )
{
    for ( volatile uint32_t pause = 0; pause < 10; pause++ )
    {
        /* a short pause: it makes the gap between load and store longer */
    }
    return value + 1;                           /* the add */
}
#endif

void task_b( void )
{
    LOCK( &uart_lock );                         /* the console is ours */
    printf( "B: hello from task B\n" );
    UNLOCK( &uart_lock );                       /* the other task may print now */

#if DO_COUNTER
    /* Phase 1: add 1 to the shared counter, as a plain load, add,
     * store (-O0). Without the lock, if a tick switches to the other
     * task between the load and the store, the other task's update is
     * overwritten: it is lost. With the lock, the other task cannot
     * start its own update until this one is stored. */
    for ( uint32_t i = 0; i < COUNT_N; i++ )
    {
        LOCK( &counter_lock );                          /* only one task at a time below */
        shared_counter = plus_one( shared_counter );    /* load, add 1, store */
        UNLOCK( &counter_lock );                        /* the other task may update now */
    }

    /* Say "my counter phase is done". Only this task writes this word. */
    shared_counter_done_b = 1;
    /* If the other task is done too, this is the last one: print the
     * total. It must be 2 * COUNT_N: without the lock it is less. */
    if ( shared_counter_done_a )
    {
        LOCK( &uart_lock );                     /* the console is ours */
        printf( "counter=%u expected=%u (B last)\n", shared_counter, 2 * COUNT_N );
        UNLOCK( &uart_lock );                   /* the other task may print now */
    }
#endif

#if DO_LINES
    /* Phase 2: print LINES_N lines, each LINE_LEN times the letter 'B'.
     * The line is built on this task's own stack (it is private). */
    char line[ LINE_LEN + 1 ];                  /* the characters + the end mark */
    for ( uint32_t k = 0; k < LINE_LEN; k++ )
    {
        line[ k ] = 'B';                    /* fill the line with one letter */
    }
    line[ LINE_LEN ] = '\0';                    /* end of the string */

    for ( uint32_t n = 0; n < LINES_N; n++ )
    {
        LOCK( &uart_lock );                     /* the console is ours */
        PRINT_BEGIN();                          /* only for DEMO=cpsid */
        printf( "%s\n", line );                 /* one whole line */
        PRINT_END();                            /* only for DEMO=cpsid */
        UNLOCK( &uart_lock );                   /* the other task may print now */
    }
#endif

    /* Say "I am done with everything". */
    shared_all_done_b = 1;
    /* The last task to finish prints the scheduler trace, so the run
     * ends with the chart. */
    if ( shared_all_done_a )
    {
        trace_dump();
    }

    /* Returning ends the task: the lr of its first frame is task_exit. */
}
