/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * The shared block (week 7). Until now each task had its own memory
 * and could not see the other's. Variables marked SHARED are different:
 * the linker puts them in one 1 KB block that both task A and task B
 * can read and write (MPU region 2, kernel/mpu.c). This is the only
 * place where the two tasks can touch the same variable.
 */
#ifndef SHARED_H
#define SHARED_H

#include <stdint.h>
#include "lib/spinlock.h"
#include "lib/locks.h"

/* Put a variable in the .shared section (the shared block).
 * That section is NOLOAD (scripts/mps2_m3.ld): nothing is stored in
 * FLASH for it. RAM is zero at start, so every SHARED variable starts
 * at 0. A start value in the C code (= 5) would be silently lost, so
 * do not write one. */
#define SHARED __attribute__(( section( ".shared" ) ))

/* The counter both tasks add to. volatile: the compiler must really
 * read and write memory at every use. */
extern volatile uint32_t shared_counter SHARED;

/* "I have finished the counter phase": one word per task, so each
 * task writes only its own word (no two tasks write the same word). */
extern volatile uint32_t shared_counter_done_a SHARED;
extern volatile uint32_t shared_counter_done_b SHARED;

/* "I have finished everything": the same idea, for the whole program. */
extern volatile uint32_t shared_all_done_a SHARED;
extern volatile uint32_t shared_all_done_b SHARED;

/* The locks (lib/spinlock.h). They are in the shared block, so both
 * tasks use the same word; they start at 0, which means free.
 * One lock per shared thing, so a task printing does not stop the
 * other from counting:
 *   counter_lock - held while a task updates shared_counter;
 *   uart_lock    - held while a task prints one line (the console is
 *                  shared too: a line must come out whole). */
extern spinlock_t counter_lock SHARED;
extern spinlock_t uart_lock SHARED;

/* Laboratory 8. DEMO=ticket protects shared_counter with a ticket lock
 * (lib/locks.h) instead of counter_lock. */
extern ticket_t counter_ticket SHARED;

/* Laboratory 8: how many times a counter lock has been taken, by either
 * task. Only the holder adds to it, so the lock itself protects it. A
 * task that reads it when it asks for the lock and again when it gets
 * the lock learns how many times the other task took the lock while it
 * waited: how many times it was overtaken. */
extern volatile uint32_t lock_takes SHARED;

#endif /* SHARED_H */
