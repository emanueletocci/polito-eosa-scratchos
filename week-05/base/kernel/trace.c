/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * The scheduler trace: see kernel/trace.h.
 *
 * trace_record_switch() runs inside PendSV and trace_dump() inside
 * SVCall. Both are exceptions that never interrupt each other in the
 * middle of their work (PendSV only runs when SVCall is over), so the
 * buffer needs no protection.
 */

#include <stdint.h>
#include "kernel/trace.h"
#include "kernel/task.h"
#include "kernel/kprintf.h"

/* CLASS: trace record type goes here */

/* CLASS: ring buffer goes here */

void trace_record_switch( uint8_t from, uint8_t to )
{
    (void) from;                            /* not used until the class code */
    (void) to;
    /* CLASS: trace_record_switch body goes here */
}

/* CLASS: Gantt helpers go here */

void trace_print( void )
{
    /* CLASS: trace_print body goes here */
}
