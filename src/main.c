/*
 * Embedded Operating Systems course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Week 1 checkpoint: proves the toolchain pipeline (cross-compile, link,
 * boot under QEMU) works end to end. No OS behavior yet - that starts
 * in week 2. Output goes over ARM semihosting, not real UART (week 3's
 * topic).
 *
 * This writes directly via a semihosting call instead of printf(): our
 * hand-rolled Reset_Handler doesn't perform the C-runtime init newlib's
 * semihosting-backed stdio (rdimon) expects, so routing through printf
 * silently produces no output. Direct semihosting sidesteps libc
 * entirely - appropriate for week 1, before any C-runtime topic has
 * been covered anyway.
 */

#include <stdint.h>

static void semihost_write0( const char *message )
{
    register uint32_t r0 __asm__( "r0" ) = 0x04; /* SYS_WRITE0 */
    register const char *r1 __asm__( "r1" ) = message;
    __asm__ volatile ( "bkpt 0xab" : : "r"( r0 ), "r"( r1 ) : "memory" );
}

int main( void )
{
    semihost_write0( "toolchain OK\n" );
    return 0;
}
