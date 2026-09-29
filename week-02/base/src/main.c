/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Week 2 starting point: main() prints one string from FLASH and one
 * from RAM (.data). The fully commented version is in
 * ../../solution/src/main.c.
 */

#include <stdint.h>
/* CLASS: the DEMO_DIVIDE_BY_ZERO switch goes here */

/* Print a string through semihosting: `bkpt 0xab` asks QEMU to do it. */
static void semihost_write0( const char *message )
{
    register uint32_t r0 __asm__( "r0" ) = 0x04;         /* SYS_WRITE0 */
    register const char *r1 __asm__( "r1" ) = message;   /* the string */
    __asm__ volatile ( "bkpt 0xab" : : "r"( r0 ), "r"( r1 ) : "memory" );
}

/* Initialized global: runs from RAM, initial value stored in FLASH. */
char data_message[] = "main: this text lives in .data\n";

/* CLASS: fault handlers and SVC_Handler go here */

int main( void )
{
    /* CLASS: the divide-by-zero demo goes here */

    semihost_write0( "main: hello from the base\n" );   /* literal: FLASH */
    semihost_write0( data_message );                    /* .data: RAM */

    for ( ;; )
    {
    }
}
