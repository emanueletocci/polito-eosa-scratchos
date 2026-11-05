/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * The spinlock functions declared in lib/spinlock.h. A lock is one
 * word: 0 = free, 1 = taken. The pointer to it arrives in r0 (AAPCS).
 *
 * The problem: taking the lock is "read the word; if it is 0, write
 * 1". In plain code a tick can come between the read and the write.
 * Then the other task also reads 0, also writes 1, and both think they
 * hold the lock.
 *
 * The answer: the exclusive pair ldrex / strex.
 *   - `ldrex r1, [r0]` reads the word like a normal load, and also
 *     tells the CPU's exclusive monitor (a tiny piece of hardware that
 *     remembers one flag) "I read this; watch it".
 *   - `strex r2, r1, [r0]` writes r1 to the word ONLY IF the monitor
 *     still says "nothing happened since the ldrex". It puts the answer
 *     in r2: 0 = written (success), 1 = not written (failure). Either
 *     way the monitor is reset.
 * So the pair is not "uninterruptible": it is "either nothing came in
 * between, or the write does not happen and we know it". On failure we
 * simply start again from ldrex.
 *
 * What clears the monitor (makes strex fail)? On the Cortex-M3, every
 * exception entry and every exception return: a SysTick, a PendSV, an
 * SVC, any interrupt, even one that does not switch tasks. So if a
 * task is preempted between its ldrex and its strex, the other task
 * runs, and when the first task comes back its strex fails and it
 * retries, reading the new value. That is why the context switch
 * (kernel/pendsv.s) needs no `clrex`: the exception itself already
 * cleared the monitor.
 *
 * Why the `dmb` barriers? `dmb` (data memory barrier) means "finish
 * every memory access written before me before starting any written
 * after me". The CPU and, above all, the compiler may otherwise move
 * reads and writes of the shared data across the lock instructions.
 *   - After a successful take: the reads and writes of the protected
 *     data must not start before the lock is ours.
 *   - Before the give-back store: the writes to the protected data
 *     must be done before another task can see the lock free.
 * The Cortex-M3 itself does not reorder much, so the dmb rarely
 * changes anything on this CPU; it is the rule every lock follows, and
 * on faster CPUs (or with several cores) it is what makes the lock
 * correct. For the C compiler, the call to a function in another file
 * is already a point it cannot move memory accesses across.
 *
 * No `clrex` here either: when spin_lock sees the lock taken, it does
 * an ldrex with no strex. That leftover "watch" is harmless: the next
 * ldrex starts a new one, and the `svc` inside task_yield clears it.
 */

/* Same assembler settings as boot/startup.s. */
    .syntax unified             /* modern ARM assembly syntax */
    .cpu cortex-m3              /* only instructions this CPU really has */
    .thumb                      /* Thumb code (the only kind Cortex-M runs) */

    .section .text,"ax",%progbits   /* code goes into the normal code section */

/* ------------------------------------------------------------------ */
/* void spin_lock(spinlock_t *lock)                                    */
/* ------------------------------------------------------------------ */
/* lock arrives in r0. Returns once this task holds the lock.
 * When the lock is taken, it calls task_yield: on one CPU the holder
 * cannot run while we wait, so we give it the CPU ("spin-then-yield").
 * C equivalent (pseudo-C, since C cannot write ldrex / strex):
 *     void spin_lock(spinlock_t *lock) {
 *         for (;;) {
 *             uint32_t value = ldrex(lock);     // read, start watching
 *             if (value != 0) {                 // taken by the other task
 *                 task_yield();                 // let the holder run
 *                 continue;                     // and try again
 *             }
 *             if (strex(lock, 1) == 0) {        // write 1, if nobody came in
 *                 break;                        // success: the lock is ours
 *             }
 *         }                                     // strex failed: try again
 *         dmb();                                // no access moves above here
 *     }
 */
    .global spin_lock           /* visible to C files */
    .type spin_lock, %function  /* it is code */
    .thumb_func                 /* it is Thumb code (bit 0 set in its address) */
spin_lock:
spin_lock_try:
    ldrex r1, [r0]              /* value = *lock, and start watching *lock */
    cmp r1, #0                  /* is value == 0 (free)? */
    bne spin_lock_wait          /* if (value != 0) goto wait: taken */
    mov r1, #1                  /* r1 = 1: the value that means "taken" */
    strex r2, r1, [r0]          /* r2 = strex(lock, 1): 0 = written, 1 = not */
    cmp r2, #0                  /* did the write happen? */
    bne spin_lock_try           /* if (r2 != 0) goto try: something came in between */
    dmb                         /* dmb(): the protected accesses start only now */
    bx lr                       /* return: the lock is ours */
spin_lock_wait:
    /* task_yield is a normal function: by the AAPCS it may change r0-r3,
     * r12 and lr. We still need r0 (the lock) and lr (where to return),
     * so save both on the stack around the call. Two words keep the
     * stack 8-byte aligned, as the AAPCS asks at a call. */
    push {r0, lr}               /* save lock and our return address */
    bl task_yield               /* task_yield(); the holder runs meanwhile */
    pop {r0, lr}                /* restore lock and our return address */
    b spin_lock_try             /* goto try: maybe the lock is free now */
    .size spin_lock, . - spin_lock /* record its size in bytes */

/* ------------------------------------------------------------------ */
/* void spin_lock_busy(spinlock_t *lock)                               */
/* ------------------------------------------------------------------ */
/* lock arrives in r0. The same as spin_lock, but while the lock is
 * taken it just tries again at once ("busy waiting"). On one CPU this
 * burns the rest of the time slice: the holder cannot run until a tick
 * switches tasks. Only `make DEMO=spin` uses it.
 * C equivalent (pseudo-C):
 *     void spin_lock_busy(spinlock_t *lock) {
 *         for (;;) {
 *             uint32_t value = ldrex(lock);     // read, start watching
 *             if (value != 0) {                 // taken by the other task
 *                 continue;                     // just try again
 *             }
 *             if (strex(lock, 1) == 0) {        // write 1, if nobody came in
 *                 break;                        // success: the lock is ours
 *             }
 *         }                                     // strex failed: try again
 *         dmb();                                // no access moves above here
 *     }
 */
    .global spin_lock_busy      /* visible to C files */
    .type spin_lock_busy, %function /* it is code */
    .thumb_func                 /* it is Thumb code (bit 0 set in its address) */
spin_lock_busy:
spin_lock_busy_try:
    ldrex r1, [r0]              /* value = *lock, and start watching *lock */
    cmp r1, #0                  /* is value == 0 (free)? */
    bne spin_lock_busy_try      /* if (value != 0) goto try: taken, ask again */
    mov r1, #1                  /* r1 = 1: the value that means "taken" */
    strex r2, r1, [r0]          /* r2 = strex(lock, 1): 0 = written, 1 = not */
    cmp r2, #0                  /* did the write happen? */
    bne spin_lock_busy_try      /* if (r2 != 0) goto try: something came in between */
    dmb                         /* dmb(): the protected accesses start only now */
    bx lr                       /* return: the lock is ours */
    .size spin_lock_busy, . - spin_lock_busy /* record its size in bytes */

/* ------------------------------------------------------------------ */
/* void spin_unlock(spinlock_t *lock)                                  */
/* ------------------------------------------------------------------ */
/* lock arrives in r0. Gives the lock back. A plain store is enough:
 * only the holder writes the word now, so nobody can cut in. The dmb
 * comes first, so the writes to the protected data are finished before
 * the other task can see the lock free.
 * C equivalent:
 *     void spin_unlock(spinlock_t *lock) {
 *         dmb();                                // finish the protected accesses
 *         *lock = 0;                            // free
 *     }
 */
    .global spin_unlock         /* visible to C files */
    .type spin_unlock, %function /* it is code */
    .thumb_func                 /* it is Thumb code (bit 0 set in its address) */
spin_unlock:
    dmb                         /* dmb(): every earlier access is done first */
    mov r1, #0                  /* r1 = 0: the value that means "free" */
    str r1, [r0]                /* *lock = 0: the lock is free */
    bx lr                       /* return */
    .size spin_unlock, . - spin_unlock /* record its size in bytes */
