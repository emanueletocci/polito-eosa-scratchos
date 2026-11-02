/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * The spinlock functions declared in lib/spinlock.h. A lock is one
 * word: 0 = free, 1 = taken. The pointer to it arrives in r0 (AAPCS).
 * This file will hold spin_lock_busy, spin_lock and spin_unlock.
 */

/* Same assembler settings as boot/startup.s. */
    .syntax unified             /* modern ARM assembly syntax */
    .cpu cortex-m3              /* only instructions this CPU really has */
    .thumb                      /* Thumb code (the only kind Cortex-M runs) */

    .section .text,"ax",%progbits   /* code goes into the normal code section */

    .global spin_lock_busy
    .type spin_lock_busy, %function
    .thumb_func
spin_lock_busy:
    /* Week 7, Unit 2: write spin_lock_busy here (see the slides). */
    bx lr
    .size spin_lock_busy, . - spin_lock_busy

    .global spin_lock
    .type spin_lock, %function
    .thumb_func
spin_lock:
    /* Week 7, Unit 2: write spin_lock here (see the slides). */
    bx lr
    .size spin_lock, . - spin_lock

    .global spin_unlock
    .type spin_unlock, %function
    .thumb_func
spin_unlock:
    /* Week 7, Unit 2: write spin_unlock here (see the slides). */
    bx lr
    .size spin_unlock, . - spin_unlock
