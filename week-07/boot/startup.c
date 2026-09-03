/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Minimal ARM Cortex-M3 boot code: vector table + reset handler.
 *
 * Reset_Handler is deliberately split into a naked asm stub (explicit
 * stack-pointer setup) and a plain C continuation - see course-material's
 * slide-sources/lecture-notes/week-02.md for why. The same ldr/mov-into-SP
 * pattern used here comes back, repeated many times instead of once,
 * in week 5's context switch.
 *
 * WEEK 3 extended this vector table with the SysTick core-exception
 * entry and the UART0-TX external-interrupt entry (src/main.c has both
 * handlers). External interrupts start at index 16 (after the 16 core
 * exception entries) - see slide-sources/lecture-notes/week-03.md.
 */

/* Fixed-width integer types (uint32_t); this file needs no other libc. */
#include <stdint.h>

/* Symbols the linker script defines - addresses, not variables. Declared
 * extern here so C can take their address; their value comes from
 * mps2_m3.ld at link time. */
extern uint32_t _estack;                 /* top of stack, from mps2_m3.ld */
extern uint32_t _sidata, _sdata, _edata; /* .data load addr, then run start/end */
extern uint32_t _sbss, _ebss;            /* .bss run-address start/end */

/* The C entry point, defined in src/main.c; called at the end of reset. */
extern int main( void );
/* Interrupt handlers defined in src/main.c, referenced by the table below. */
extern void SysTick_Handler( void );  /* periodic core-timer exception */
extern void UART0TX_Handler( void );  /* UART0 transmit-buffer-empty IRQ */

/* Every unhandled exception/interrupt lands here instead of jumping to
 * garbage. A real handler for a given vector replaces this entry in the
 * table below once that topic is covered. */
void Default_Handler( void )
{
    /* Spin forever: an unexpected exception is a bug; freezing here
     * keeps the fault visible in a debugger rather than letting the CPU
     * run off into undefined behaviour. */
    for( ;; )
    {
    }
}

/* Explicit stack-pointer setup, in assembly, before any C code runs.
 * On Cortex-M this is technically redundant - hardware already loaded
 * SP from isr_vector[0] before this function's own code starts - but
 * taking control of it explicitly here is the same skill week 5's
 * context switch needs, where SP genuinely does need to move at
 * runtime. `naked` means no compiler-generated prologue/epilogue, so
 * we control every instruction that runs before SP is set. */
__attribute__( ( naked ) )
void Reset_Handler( void )
{
    __asm volatile ( "ldr r0, =_estack" );   /* r0 = address of the top of stack */
    __asm volatile ( "mov sp, r0" );         /* install it as the active stack pointer */
    __asm volatile ( "bl Reset_Handler_C" ); /* branch-and-link into the C continuation */
}

/* The rest of reset, now that SP is known-good: safe to use plain C
 * (locals, function calls) from here on. */
void Reset_Handler_C( void )
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

/* The vector table. Entry 0 is the initial SP, entry 1 the reset
 * address, entries 2-15 the core exceptions. ARMv7-M requires at least
 * the first 16 entries (core exceptions). From index 16 on, these are
 * external (peripheral) interrupts, in the order the MPS2/CMSDK board
 * defines them - we only need index 17 (UART0 TX) so far; index 16
 * (UART0 RX) is a placeholder until a future week needs it.
 * `section(".isr_vector")` + `const` place it first in FLASH, read-only. */
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
    SysTick_Handler,            /* 15: SysTick - the periodic tick added this week */
    Default_Handler,            /* 16 / IRQ0: UART0 RX (unused so far) */
    UART0TX_Handler,            /* 17 / IRQ1: UART0 TX - drives interrupt-driven output */
};
