/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Minimal ARM Cortex-M3 boot code, written entirely in assembly:
 * the vector table and the reset handler.
 *
 * Nothing here is C: when the CPU comes out of reset there is no C
 * runtime yet (initialized variables are not in RAM, uninitialized ones
 * are not zero). This file is where that runtime gets built by hand,
 * before main() is called. Every block below starts with the C code it
 * is equivalent to, so the assembly can be read side by side with
 * something familiar.
 *
 * The file is organized in two parts:
 *   1. the vector table  - the list of addresses the CPU reads on reset
 *                          and whenever an exception happens;
 *   2. Reset_Handler     - the first code that runs after power-on.
 */

/* ------------------------------------------------------------------ */
/* Assembler settings                                                  */
/* ------------------------------------------------------------------ */

/* `.syntax unified`: use the modern ARM assembly syntax, the one the
 * ARM reference manuals use (shared by ARM and Thumb code). */
    .syntax unified
/* `.cpu cortex-m3`: only accept instructions this CPU really has, so a
 * typo or an unsupported instruction becomes a build error. */
    .cpu cortex-m3
/* `.thumb`: emit Thumb instructions. Cortex-M CPUs can only execute
 * Thumb code; they have no "ARM mode" at all. */
    .thumb

/* ------------------------------------------------------------------ */
/* Symbols defined elsewhere                                           */
/* ------------------------------------------------------------------ */

/* `_estack` comes from scripts/mps2_m3.ld (the linker script): it is
 * an address, not a variable - the top of the stack (end of RAM).
 * `main` is defined in src/main.c.
 * The assembler treats any name it does not find in this file as
 * "defined somewhere else" and lets the linker fill in the address,
 * so they need no declaration here (unlike `extern` in C). */

/* ================================================================== */
/* 1. The vector table                                                 */
/* ================================================================== */

/* The vector table is an array of 32-bit words placed at the very
 * start of FLASH (the linker script puts section .isr_vector first,
 * at address 0x00000000).
 *   - word 0 is the initial value of the stack pointer (SP);
 *   - word 1 is the address of the first instruction to run (reset);
 *   - words 2..15 are the addresses of the handlers of the CPU's own
 *     exceptions (faults, system calls, system timer, ...).
 * When exception number N happens, the CPU reads word N and jumps
 * there.
 *
 * C equivalent:
 *     void (* const isr_vector[])(void) = {
 *         (void (*)(void)) &_estack,   // 0
 *         Reset_Handler,               // 1
 *     };
 */

/* `.section NAME,"a",%progbits`: put what follows into the section
 * called .isr_vector. "a" means "allocatable" (it takes space in the
 * final image), %progbits means "it contains real data, stored in the
 * file". The linker script uses the section name to place it. */
    .section .isr_vector,"a",%progbits
/* `.global NAME`: make the name visible to other files (and to the
 * debugger), like a non-static variable in C. */
    .global isr_vector
/* `.type NAME, %object`: tell the tools this name is data, not code. */
    .type isr_vector, %object

/* A label: `isr_vector` is the address of the first word below. */
isr_vector:
/* `.word X`: store the 32-bit value X here, like one array element. */
    .word _estack               /* 0: initial SP = top of RAM */
    .word Reset_Handler         /* 1: reset: first code after power-on */
    /* In class: you will add the remaining vector-table words here. */

/* `.size NAME, EXPR`: record how many bytes NAME occupies (here: from
 * the label to the current position `.`). Used by nm, objdump and gdb. */
    .size isr_vector, . - isr_vector

/* ================================================================== */
/* 2. Reset_Handler                                                    */
/* ================================================================== */

/* From here on it is code: put it in the normal code section. The flags
 * mean "allocatable" (a) and "executable" (x). */
    .section .text,"ax",%progbits
/* Make Reset_Handler visible to the linker (the linker script names it
 * as the ENTRY point, and the vector table above refers to it). */
    .global Reset_Handler
/* `.type NAME, %function`: tell the tools this name is code. */
    .type Reset_Handler, %function
/* `.thumb_func`: the next label is a Thumb function. The tools then set
 * bit 0 of its address wherever it is stored (e.g. in the vector
 * table); on Cortex-M that bit must be 1, meaning "Thumb code". */
    .thumb_func
Reset_Handler:

    /* In class: you will add the setup steps here, before main is
     * called (setting the stack pointer, copying .data from FLASH to
     * RAM, clearing .bss, and the rest). For now, jump straight into
     * main() with whatever the hardware left in place at reset. */

    /* C equivalent:
     *     main();
     *     for (;;) { }
     */
    bl main                     /* call main(); "branch with link" saves the return address in LR */
hang:
    b hang                      /* for (;;) {}: main should never return; if it does, stay here */

/* Literal pool. `ldr rX, =VALUE` cannot fit a full 32-bit VALUE inside
 * a 16/32-bit instruction, so the assembler stores VALUE in a nearby
 * table of constants (the "literal pool") and loads it from there.
 * `.ltorg` says "put that table here", after the last instruction,
 * where the CPU will never try to execute it as code. */
    .ltorg

/* Record Reset_Handler's size in bytes (label to here). */
    .size Reset_Handler, . - Reset_Handler
