/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * WEEK 3 LAB STARTER - not a checkpoint.
 *
 * Derived from week-02/solution/ (boot/, scripts/ and this file). What
 * was deliberately removed from this file: the four fault handlers,
 * SVC_Handler, the divide-by-zero switch, the drop to unprivileged mode,
 * the `svc #0`, and the write to SysTick's control register. Writing
 * them back is exercises A3-A5. The Makefile is not supplied either:
 * exercise A1 has you bring the one you wrote in week-02-lab.
 *
 * As delivered, main() prints two lines and returns; Reset_Handler then
 * stops in its final loop. Every exception still reaches the weak
 * Default_Handler in boot/startup.s, which loops forever without
 * printing anything.
 *
 * It writes directly via a semihosting call instead of printf(), for
 * the reason week-02-lab exercise A4 established: this image links no
 * C library, so printf() has nothing to link against.
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
    /* `bkpt 0xab` is the Thumb semihosting trap: QEMU sees it, reads r0
     * and r1, writes the string to its console, and resumes. "memory"
     * tells the compiler this may read arbitrary memory (the string),
     * so it must not reorder earlier writes past this point. */
    __asm__ volatile ( "bkpt 0xab" : : "r"( r0 ), "r"( r1 ) : "memory" );
}

/* A global array with an initial value: unlike a string literal (which
 * lives in FLASH, in .rodata), this one is writable data, so it must
 * live in RAM. The linker script stores its initial value in FLASH,
 * and Reset_Handler's .data copy loop moves that value into RAM before
 * main() runs. */
char data_message[] = "main: this text lives in .data\n";

/* Program entry point, called from Reset_Handler once RAM is set up. */
int main( void )
{
    /* main() starts running privileged: reset always begins in
     * privileged, Thread mode. Printing here works with no extra
     * steps. */
    semihost_write0( "main: privileged hello\n" );

    /* This string lives in RAM (.data), and Reset_Handler already
     * copied its initial value there before calling main(), so it
     * prints normally. */
    semihost_write0( data_message );

    return 0;
}
