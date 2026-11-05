/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * The "add 1 to the shared counter" step of the counter phase, and the
 * two ways of doing it with no lock (DEMO=fetchadd, DEMO=cas).
 */
#ifndef COUNTER_H
#define COUNTER_H

#include <stdint.h>

/* value + 1, slowly (see src/counter.c). */
uint32_t plus_one( uint32_t value );

/* Add 1 to *p with atomic_fetch_add. */
void counter_add_fetchadd( volatile uint32_t *p );

/* Add 1 to *p with plus_one and atomic_cas, retrying until the swap
 * happens. Returns how many attempts failed before it did. */
uint32_t counter_add_cas( volatile uint32_t *p );

#endif /* COUNTER_H */
