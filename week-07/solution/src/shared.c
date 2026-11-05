/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * The shared variables declared in src/shared.h. All of them are in
 * the .shared section, so both tasks can use them. No start values:
 * the section is NOLOAD, so they start at 0.
 */

#include "src/shared.h"

volatile uint32_t shared_counter SHARED;            /* the total both tasks add to */
volatile uint32_t shared_counter_done_a SHARED;     /* 1 when A has finished the counter phase */
volatile uint32_t shared_counter_done_b SHARED;     /* 1 when B has finished the counter phase */
volatile uint32_t shared_all_done_a SHARED;         /* 1 when A has finished everything */
volatile uint32_t shared_all_done_b SHARED;         /* 1 when B has finished everything */
spinlock_t counter_lock SHARED;                     /* protects shared_counter; 0 = free */
spinlock_t uart_lock SHARED;                        /* protects the console; 0 = free */
