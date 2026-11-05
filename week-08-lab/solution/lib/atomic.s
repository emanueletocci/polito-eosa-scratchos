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
    ldrex r2, [r0]              /* old = *p, and start watching *p */
    strex r3, r1, [r0]          /* *p = value, if nothing came in between; r3 = 0 stored, 1 not */
    cmp r3, #0                  /* did the write happen? */
    bne atomic_exchange         /* if not, start again */
    mov r0, r2                  /* return old */
    bx lr
    .size atomic_exchange, . - atomic_exchange /* record its size in bytes */

/* ------------------------------------------------------------------ */
/* uint32_t atomic_fetch_add(volatile uint32_t *p, uint32_t n)         */
/* ------------------------------------------------------------------ */
    .global atomic_fetch_add    /* visible to C files */
    .type atomic_fetch_add, %function /* it is code */
    .thumb_func                 /* it is Thumb code (bit 0 set in its address) */
atomic_fetch_add:
    ldrex r2, [r0]              /* old = *p, and start watching *p */
    add r3, r2, r1              /* r3 = old + n */
    strex r12, r3, [r0]         /* *p = old + n, if nothing came in between */
    cmp r12, #0                 /* did the write happen? */
    bne atomic_fetch_add        /* if not, start again */
    mov r0, r2                  /* return old */
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
    ldrex r3, [r0]              /* old = *p, and start watching *p */
    cmp r3, r1                  /* old == expected? */
    bne cas_differs             /* no: give up, write nothing */
    strex r12, r2, [r0]         /* *p = desired, if nothing came in between */
    cmp r12, #0                 /* did the write happen? */
    bne atomic_cas              /* if not, start again: old may have changed */
    mov r0, r3                  /* return old (== expected: swapped) */
    bx lr
cas_differs:
    clrex                       /* stop watching *p: no strex will follow this ldrex */
    mov r0, r3                  /* return old (!= expected: not swapped) */
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
