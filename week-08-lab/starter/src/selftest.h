/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * A check of the atomic operations (lib/atomic.h), with one task, on a
 * word of its own stack: DEMO=atomictest.
 */
#ifndef SELFTEST_H
#define SELFTEST_H

/* Run the checks, print one line per failed check and a summary. */
void atomic_selftest( void );

#endif /* SELFTEST_H */
