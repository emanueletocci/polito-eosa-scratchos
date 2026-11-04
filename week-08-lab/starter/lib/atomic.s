/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * The atomic operations declared in lib/atomic.h. Arguments arrive in
 * r0, r1, r2 and the result goes back in r0 (AAPCS); r3 and r12 are free
 * to use without saving. Each operation is an ldrex / strex loop, as in
 * spin_lock (lib/spinlock.s): read and watch, compute, store if nothing
 * came in between, otherwise start again.
 */

/* Same assembler settings as lib/spinlock.s. */
    .syntax unified             /* modern ARM assembly syntax */
    .cpu cortex-m3              /* only instructions this CPU really has */
    .thumb                      /* Thumb code (the only kind Cortex-M runs) */

    .section .text,"ax",%progbits   /* code goes into the normal code section */

/* ------------------------------------------------------------------ */
/* uint32_t atomic_exchange(volatile uint32_t *p, uint32_t value)      */
/* ------------------------------------------------------------------ */
    .global atomic_exchange     /* visible to C files */
    .type atomic_exchange, %function /* it is code */
    .thumb_func                 /* it is Thumb code (bit 0 set in its address) */
atomic_exchange:
    /* B1: old = *p; *p = value, in one step. Return old. For now it
     * does nothing and returns 0. */
    mov r0, #0                  /* return 0 */
    bx lr
    .size atomic_exchange, . - atomic_exchange /* record its size in bytes */

/* ------------------------------------------------------------------ */
/* uint32_t atomic_fetch_add(volatile uint32_t *p, uint32_t n)         */
/* ------------------------------------------------------------------ */
    .global atomic_fetch_add    /* visible to C files */
    .type atomic_fetch_add, %function /* it is code */
    .thumb_func                 /* it is Thumb code (bit 0 set in its address) */
atomic_fetch_add:
    /* B1: old = *p; *p = old + n, in one step. Return old. For now it
     * does nothing and returns 0. */
    mov r0, #0                  /* return 0 */
    bx lr
    .size atomic_fetch_add, . - atomic_fetch_add /* record its size in bytes */

/* ------------------------------------------------------------------ */
/* uint32_t atomic_cas(volatile uint32_t *p, uint32_t expected,        */
/*                     uint32_t desired)                               */
/* ------------------------------------------------------------------ */
    .global atomic_cas          /* visible to C files */
    .type atomic_cas, %function /* it is code */
    .thumb_func                 /* it is Thumb code (bit 0 set in its address) */
atomic_cas:
    /* B1: old = *p; if old == expected, *p = desired, in one step; when
     * old differs, write nothing and stop watching *p. Return old. For
     * now it does nothing and returns 0. */
    mov r0, #0                  /* return 0 */
    bx lr
    .size atomic_cas, . - atomic_cas /* record its size in bytes */

/* ------------------------------------------------------------------ */
/* void dmb(void)                                                      */
/* ------------------------------------------------------------------ */
    .global dmb                 /* visible to C files */
    .type dmb, %function        /* it is code */
    .thumb_func                 /* it is Thumb code (bit 0 set in its address) */
dmb:
    dmb                         /* memory accesses before it complete before those after it */
    bx lr
    .size dmb, . - dmb          /* record its size in bytes */
