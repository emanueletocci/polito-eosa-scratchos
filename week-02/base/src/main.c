/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Starting point for this week: main() prints two lines. The first one
 * is a plain string literal, which lives in FLASH and needs nothing set
 * up to be readable. The second one is stored in a global array with an
 * initial value, which lives in RAM - its initial contents sit in FLASH
 * until something copies them over, and right now nothing does that yet.
 *
 * It writes text directly via a semihosting call instead of printf():
 * our hand-rolled Reset_Handler doesn't perform the C-runtime init that
 * newlib's semihosting-backed stdio (rdimon) expects, so routing through
 * printf silently produces no output. Direct semihosting sidesteps libc
 * entirely, which this freestanding build does not link against anyway.
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

/* A string literal like this one is `const` data: the compiler places
 * it in .rodata, which the linker script puts in FLASH right next to
 * the code. It needs no copying to be read - it is already sitting at
 * the address it will be read from, from the very first instruction. */

/* A global array with an initial value: unlike the string literal
 * above, this one is *writable* data, so it must live in RAM (FLASH
 * cannot be written to at runtime). The linker script stores its
 * initial value ("main: this text lives in .data\n") in FLASH, in the
 * .data section, and something must copy that value into RAM before
 * this array reads back what you expect. Nothing does that copy yet -
 * so, for now, this array's content in RAM is whatever happened to
 * already be there. */
char data_message[] = "main: this text lives in .data\n";

/* Program entry point, called from Reset_Handler once the CPU jumps
 * here. */
int main( void )
{
    /* This string lives in FLASH (.rodata): reading it works with no
     * setup at all, so this line always prints correctly. */
    semihost_write0( "main: hello from the base\n" );

    /* This string lives in RAM (.data): until something copies its
     * initial value from FLASH to RAM, this line prints garbage or
     * nothing meaningful. */
    semihost_write0( data_message );

    /* main() has nothing left to do: loop here forever rather than
     * return (a bare-metal program has nowhere to return to). */
    for ( ;; )
    {
    }
}
