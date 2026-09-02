/*
 * Embedded Operating Systems course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Minimal ARM Cortex-M3 boot code: vector table + reset handler.
 *
 * Reset_Handler is deliberately split into a naked asm stub (explicit
 * stack-pointer setup) and a plain C continuation - see
 * live-coding-guides/week-02.md for why. The same ldr/mov-into-SP
 * pattern used here comes back, repeated many times instead of once,
 * in week 5's context switch.
 *
 * WEEK 3 will extend the vector table with the SysTick and UART/GPIO
 * interrupt entries this file deliberately leaves at 0 for now.
 */

#include <stdint.h>

extern uint32_t _estack;               /* top of stack, from mps2_m3.ld */
extern uint32_t _sidata, _sdata, _edata; /* .data load/run addresses */
extern uint32_t _sbss, _ebss;            /* .bss run addresses */

extern int main( void );

/* Every unhandled exception/interrupt lands here instead of jumping to
 * garbage. A real handler for a given vector replaces this entry in the
 * table below once that topic is covered (SysTick and UART in week 3). */
void Default_Handler( void )
{
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
    __asm volatile ( "ldr r0, =_estack" );
    __asm volatile ( "mov sp, r0" );
    __asm volatile ( "bl Reset_Handler_C" );
}

void Reset_Handler_C( void )
{
    /* Copy .data's initial values from flash to RAM. */
    uint32_t *src = &_sidata;
    uint32_t *dest = &_sdata;
    while( dest < &_edata )
    {
        *dest++ = *src++;
    }

    /* Zero-initialize .bss. */
    dest = &_sbss;
    while( dest < &_ebss )
    {
        *dest++ = 0;
    }

    main();

    /* main() should not return in a bare-metal image; if it does,
     * stop cleanly rather than run off into undefined memory. */
    for( ;; )
    {
    }
}

/* ARMv7-M requires at least these 16 entries. Peripheral interrupt
 * entries are added in week 3 once there's a handler to point them at. */
__attribute__( ( section( ".isr_vector" ) ) )
void ( * const isr_vector[] )( void ) =
{
    (void (*)(void)) &_estack,  /* initial stack pointer */
    Reset_Handler,
    Default_Handler,            /* NMI */
    Default_Handler,            /* HardFault */
    Default_Handler,            /* MemManage */
    Default_Handler,            /* BusFault */
    Default_Handler,            /* UsageFault */
    0, 0, 0, 0,                 /* reserved */
    Default_Handler,            /* SVCall */
    Default_Handler,            /* DebugMonitor */
    0,                          /* reserved */
    Default_Handler,            /* PendSV */
    Default_Handler,            /* SysTick */
};
