/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * The spinlock (week 7): a lock that tasks take before they touch
 * shared data and give back right after, so only one task at a time
 * runs the code in between (the critical section).
 *
 * A lock is one word of memory:
 *   0 = free, nobody holds it;
 *   1 = taken, one task holds it.
 * Taking the lock means changing it from 0 to 1 in one step that no
 * other task can cut in two. Plain C cannot do that (a read, then a
 * write, can be split by a tick), so the functions are written in
 * assembly with the ldrex / strex instructions: see lib/spinlock.s.
 *
 * Rules for using a lock:
 *   - put it in the shared block (SHARED, src/shared.h), so both tasks
 *     see the same word; it then starts at 0, so it starts free;
 *   - every task takes it before the shared data and gives it back
 *     after, around the same data;
 *   - only the task that holds the lock gives it back.
 */
#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>

/* One lock: 0 = free, 1 = taken. volatile: the compiler must really
 * read and write memory at every use (another task changes it). */
typedef volatile uint32_t spinlock_t;

/* Take the lock; return once this task holds it.
 * If the lock is taken, the task that holds it cannot run while we
 * wait (there is one CPU), so waiting on the CPU is useless: this
 * function calls task_yield() to give the rest of the time slice to
 * the other task, and tries again at its next turn
 * ("spin-then-yield"). */
void spin_lock( spinlock_t *lock );

/* Take the lock, like spin_lock, but while the lock is taken just try
 * again and again ("busy waiting"). On one CPU the waiting task burns
 * its whole time slice for nothing. Only `make DEMO=spin` uses it, to
 * show that waste in the scheduler trace. */
void spin_lock_busy( spinlock_t *lock );

/* Give the lock back (set it to 0). Call it only when this task
 * holds the lock. */
void spin_unlock( spinlock_t *lock );

#endif /* SPINLOCK_H */
