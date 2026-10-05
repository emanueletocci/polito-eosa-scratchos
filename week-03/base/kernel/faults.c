/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Fault handlers: silent loops that gdb can show by name.
 * They replace the weak handlers in startup.s.
 */

void HardFault_Handler( void )
{
    /* CLASS: print the fault name here */
    for ( ;; )
    {
    }
}

void MemManage_Handler( void )
{
    /* CLASS: print the fault name here */
    for ( ;; )
    {
    }
}

void BusFault_Handler( void )
{
    /* CLASS: print the fault name here */
    for ( ;; )
    {
    }
}

void UsageFault_Handler( void )
{
    /* CLASS: print the fault name here */
    for ( ;; )
    {
    }
}
