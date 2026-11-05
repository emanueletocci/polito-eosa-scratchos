/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Atomic operations on one 32-bit word, callable from C.
 *
 * Each one reads the word, computes, and writes it back as ONE step for
 * every other task: if anything comes in between (an exception, and so
 * a possible switch to another task), nothing is written and the
 * operation starts again from the read. They are built on the pair
 * ldrex / strex (lib/atomic.s); the comment at the top of
 * lib/spinlock.s says what the pair guarantees.
 *
 * They are not barriers: code that protects other data with them (a
 * lock) calls dmb() itself, where spin_lock and spin_unlock put theirs.
 */
#ifndef ATOMIC_H
#define ATOMIC_H

#include <stdint.h>

/* In one step: old = *p; *p = value. Returns old.
 * With value 1 on a word that holds 0 or 1, it is test-and-set. */
uint32_t atomic_exchange( volatile uint32_t *p, uint32_t value );

/* In one step: old = *p; *p = old + n. Returns old. */
uint32_t atomic_fetch_add( volatile uint32_t *p, uint32_t n );

/* Compare-and-swap. In one step: old = *p; if old == expected, then
 * *p = desired. Returns old, so the caller knows that the swap happened
 * exactly when the returned value equals expected. When old differs,
 * nothing is written, and the operation leaves no word watched (clrex). */
uint32_t atomic_cas( volatile uint32_t *p, uint32_t expected, uint32_t desired );

/* The memory barrier (dmb), as a function. */
void dmb( void );

#endif /* ATOMIC_H */
