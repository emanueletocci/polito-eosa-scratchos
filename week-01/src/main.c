/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Week 1 checkpoint: proves the toolchain pipeline (cross-compile, link,
 * boot under QEMU) works end to end. No OS behavior yet - that starts
 * in week 2. Output goes over ARM semihosting, not real UART (week 3's
 * topic).
 *
 * This writes directly via a semihosting call instead of printf():
 * routing through printf fails at LINK time, not at run time. Depending
 * on whether the link searches a C library at all, it fails either on
 * printf itself (with -nostdlib) or on the syscall stubs newlib's stdio
 * needs - _write, _read, _sbrk - which this image does not provide.
 * Even had it linked, newlib's semihosting-backed stdio (rdimon) expects
 * C-runtime init our hand-rolled Reset_Handler never performs, so
 * supplying the library alone would not be enough. Direct semihosting
 * sidesteps libc entirely - appropriate for week 1, before any C-runtime
 * topic has been covered anyway.
 */

/* Only uint32_t is needed here; no other header is pulled in. */
#include <stdint.h>

/* Emit a NUL-terminated string over the ARM semihosting debug channel.
 * Semihosting is a convention where the target traps to the debugger
 * (here QEMU) to ask it to perform host I/O on the target's behalf. */
static void semihost_write0( const char *message )
{
    /* Semihosting call number 0x04 (SYS_WRITE0) must be in r0. The
     * `register ... __asm__("r0")` binding forces the value into that
     * exact register, which the calling convention does not otherwise
     * guarantee. */
    register uint32_t r0 __asm__( "r0" ) = 0x04;
    /* The pointer to the string must be in r1 for SYS_WRITE0. */
    register const char *r1 __asm__( "r1" ) = message;

    /* bkpt is an ARM instruction that halts the CPU's normal execution
     * and enters Debug state - normally used so an external debugger
     * (GDB, Keil MDK, ...) can inspect registers and memory. Its
     * immediate operand (here 0xab) is just a tag a debugger can use to
     * tell one breakpoint from another; it carries no meaning to the
     * hardware itself.
     *
     * Semihosting repurposes that trap: instead of just stopping, the
     * debugger (here QEMU, acting as the debug host) recognizes 0xab,
     * reads the request number from r0 and its argument from r1, performs
     * the requested operation on the host machine, then resumes the
     * target. "semi" (half) + "hosting" - part of the task (deciding
     * what to print) runs on the target CPU, the other half (actually
     * writing to a console or file) runs on the host. It lets a board
     * with no UART or SD card still print messages or read/write files
     * during debugging.
     *
     * Reading the line: `__asm__` embeds raw assembly in C; `volatile`
     * tells the compiler not to delete or reorder this instruction even
     * though it has no C-visible result. The three fields after the
     * instruction string are, in order:
     *   - outputs (before the first colon): none here.
     *   - inputs (before the second colon): "r"(r0), "r"(r1) - place the
     *     C variables r0 and r1 into (any) registers before this
     *     instruction runs, which is what actually pins them to the
     *     physical r0/r1 the `register` declarations above requested.
     *   - clobbers (after the second colon): "memory" - warns the
     *     compiler this instruction may read memory it can't see
     *     (the string), so it must not move earlier memory writes to
     *     after this point. */
    __asm__ volatile ( "bkpt 0xab" : : "r"( r0 ), "r"( r1 ) : "memory" );
}

/* Program entry point, called from Reset_Handler once RAM is set up. */
int main( void )
{
    /* The whole point of week 1: if this line's text appears in the
     * QEMU console, the compile -> link -> boot pipeline works. */
    semihost_write0( "toolchain OK\n" );
    /* Return to Reset_Handler, which then halts in its trailing spin
     * loop (a bare-metal main() has nowhere to "exit" to). */
    return 0;
}
