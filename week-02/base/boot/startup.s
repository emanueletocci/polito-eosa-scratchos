/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Week 2 starting point: boot code in assembly (vector table and
 * Reset_Handler). You complete it in class; the fully commented version
 * is in ../../solution/boot/startup.s.
 */

    .syntax unified
    .cpu cortex-m3
    .thumb

/* ---------------- Vector table (placed at address 0) ---------------- */

    .section .isr_vector,"a",%progbits
    .global isr_vector
    .type isr_vector, %object
isr_vector:
    .word _estack               /* 0: initial stack pointer */
    .word Reset_Handler         /* 1: first code after reset */
    /* CLASS: vector table slots 2-15 go here */
    .size isr_vector, . - isr_vector
    /* CLASS: weak handler names go here */

/* ---------------- Reset_Handler: first code after reset ------------- */

    .section .text,"ax",%progbits
    .global Reset_Handler
    .type Reset_Handler, %function
    .thumb_func
Reset_Handler:
    /* CLASS: steps 1-9 go here, before bl main */
    bl main                     /* step 10: call main() */
hang:
    b hang                      /* if main returns: stay here */
    .ltorg                      /* constants for `ldr rX, =VALUE` */
    .size Reset_Handler, . - Reset_Handler

    /* CLASS: Default_Handler goes here */
