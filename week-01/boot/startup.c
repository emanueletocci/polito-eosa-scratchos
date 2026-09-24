/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * WHAT THIS FILE IS AND WHY IT EXISTS (read this before the rest)
 * -----------------------------------------------------------------
 * On a desktop OS, an executable starts already inside a running
 * environment: a C runtime has set up the stack, zeroed and
 * initialized your globals, and only then calls main(). On bare metal
 * there is no OS and no C runtime to do any of that - the very first
 * thing that runs after the chip powers on or resets is whatever sits
 * at a fixed hardware-defined address, and it starts with *nothing*
 * set up: no valid globals, no guarantee .bss is zero, nothing. This
 * file (boot/startup.c) is exactly that missing piece: hand-written
 * code that does the setup a C runtime would normally do for you,
 * before calling main(). Two things live here:
 *   1. The vector table (isr_vector[], at the bottom of this file) -
 *      a fixed array of addresses the Cortex-M3 hardware itself reads,
 *      at fixed positions, to know where to start and where to jump on
 *      each kind of exception/interrupt. Its address and layout are
 *      dictated by the ARM architecture and by mps2_m3.ld (read that
 *      file's header comment first, or alongside this one) - the
 *      linker script decides *where* this table is placed in memory,
 *      this file decides *what values* it holds.
 *   2. Reset_Handler - the function the vector table points execution
 *      to on reset. It performs the .data/.bss setup described above,
 *      by hand, using addresses (_sidata, _sdata, ...) that mps2_m3.ld
 *      computed at link time, and only then calls main().
 *
 * WEEK 1: this file is given as-is. We build it ourselves, by hand,
 * in week 2 - see course-material's slide-sources/lecture-notes/week-02.md.
 * Only enough is here to prove the toolchain pipeline works end to end.
 *
 * WEEK 3 will extend the vector table with the SysTick and UART/GPIO
 * interrupt entries this file deliberately leaves at 0 for now.
 */

/* Fixed-width integer types (uint32_t); this file needs no other libc. */
#include <stdint.h>

/* These look like ordinary uint32_t variable declarations, but they are
 * not: nothing anywhere defines storage for _estack, _sdata, etc. They
 * are symbols the linker script (scripts/mps2_m3.ld) creates by doing
 * address arithmetic (e.g. `_estack = ORIGIN(RAM) + LENGTH(RAM);`) -
 * declaring them `extern` here is a trick that lets C code take their
 * *address* (`&_estack`) and get that computed address back, without
 * ever reading or writing through them as if they held a uint32_t
 * value. Match each name below to where it's set in mps2_m3.ld to see
 * exactly which address it resolves to. */
extern uint32_t _estack;                 /* top of stack, from mps2_m3.ld */
extern uint32_t _sidata, _sdata, _edata; /* .data load addr, then run start/end */
extern uint32_t _sbss, _ebss;            /* .bss run-address start/end */

/* The C entry point, defined in src/main.c; called at the end of reset. */
extern int main( void );

/* Every unhandled exception/interrupt lands here instead of jumping to
 * garbage. A real handler for a given vector replaces this entry in the
 * table below once that topic is covered (SysTick and UART in week 3). */
void Default_Handler( void )
{
    /* Spin forever: an unexpected exception is a bug; freezing here
     * keeps the fault visible in a debugger rather than letting the CPU
     * run off into undefined behaviour. */
    for( ;; )
    {
    }
}

/* First C code to run after reset - the CPU jumps here directly (see
 * isr_vector[1] below), with no caller and no stack frame set up by
 * anyone else. That's normally a problem (a C function needs a valid
 * stack pointer before it can run at all), but Cortex-M's reset
 * sequence is special: the hardware itself reads isr_vector[0] and
 * loads it into the stack pointer register *before* fetching the first
 * instruction of Reset_Handler. So by the time this function's body
 * starts executing, SP is already valid and plain C is safe to use. */
void Reset_Handler( void )
{
    /* Copy .data's initial values from their load address in FLASH to
     * their run address in RAM, one word at a time. */
    uint32_t *src = &_sidata;  /* source cursor: .data image in FLASH */
    uint32_t *dest = &_sdata;   /* destination cursor: .data in RAM */
    while( dest < &_edata )     /* stop once the whole .data span is copied */
    {
        *dest++ = *src++;      /* copy one word, advance both cursors */
    }

    /* Zero-initialize .bss, as the C standard requires for
     * static-storage variables with no explicit initializer. */
    dest = &_sbss;             /* reuse the cursor: start of .bss in RAM */
    while( dest < &_ebss )     /* stop at the end of .bss */
    {
        *dest++ = 0;          /* clear one word, advance the cursor */
    }

    main();                    /* hand control to the C program */

    /* main() should not return in a bare-metal image; if it does,
     * stop cleanly rather than run off into undefined memory. */
    for( ;; )
    {
    }
}

/* The vector table: the CPU reads entry 0 as the initial SP and entry 1
 * as the reset address, then uses the rest to dispatch exceptions.
 * ARMv7-M requires at least these 16 entries. Peripheral interrupt
 * entries are added in week 3 once there's a handler to point them at.
 * `section(".isr_vector")` places it where mps2_m3.ld puts it first in
 * FLASH; `const` keeps it in read-only memory. */
__attribute__( ( section( ".isr_vector" ) ) )
void ( * const isr_vector[] )( void ) =
{
    (void (*)(void)) &_estack,  /* 0: initial stack pointer value */
    Reset_Handler,              /* 1: reset - first instruction after power-on */
    Default_Handler,            /* 2: NMI */
    Default_Handler,            /* 3: HardFault */
    Default_Handler,            /* 4: MemManage */
    Default_Handler,            /* 5: BusFault */
    Default_Handler,            /* 6: UsageFault */
    0, 0, 0, 0,                 /* 7-10: reserved by the architecture */
    Default_Handler,            /* 11: SVCall */
    Default_Handler,            /* 12: DebugMonitor */
    0,                          /* 13: reserved */
    Default_Handler,            /* 14: PendSV */
    Default_Handler,            /* 15: SysTick (real handler arrives in week 3) */
};
