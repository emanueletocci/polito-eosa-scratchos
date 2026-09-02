/*
 * Embedded Operating Systems course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Week 4 checkpoint: static per-task memory partitioning, on top of
 * week 3's interrupts and IO.
 *  - polled UART write (busy-wait on the TX-buffer-full flag)
 *  - interrupt-driven UART write (TX-empty interrupt sends the rest of
 *    a message one character at a time)
 *  - a periodic SysTick interrupt, counted and reported over the polled
 *    UART path
 *  - three fixed-address, fixed-size memory partitions (mps2_m3.ld),
 *    proven isolated from each other by filling each with a distinct
 *    byte pattern and checking none bled into its neighbor
 *
 * main() is no longer "run once and stop" - it becomes the idle loop
 * that everything else (week 5 onward) replaces with real scheduling.
 */

#include <stdint.h>
#include "../include/cmsdk.h"

static void uart_init( void )
{
    CMSDK_UART0->BAUDDIV = 16;                 /* minimum valid divider */
    CMSDK_UART0->CTRL = UART_CTRL_TXEN;
}

/* Polled: busy-wait for room in the TX buffer before every character. */
static void uart_write_polled( const char *message )
{
    while( *message )
    {
        while( CMSDK_UART0->STATE & UART_STATE_TXBF )
        {
        }
        CMSDK_UART0->DATA = (uint32_t) *message++;
    }
}

/* Interrupt-driven: prime the first character by hand, then the TX-empty
 * interrupt sends the rest, one character per interrupt, without main()
 * ever busy-waiting. */
static const char *volatile uart_irq_message;

void UART0TX_Handler( void )
{
    CMSDK_UART0->INTSTATUS = UART_INT_TXIRQ; /* acknowledge */

    if( *uart_irq_message )
    {
        CMSDK_UART0->DATA = (uint32_t) *uart_irq_message++;
    }
    else
    {
        CMSDK_UART0->CTRL &= ~UART_CTRL_TXIRQEN; /* nothing left to send */
    }
}

static void uart_write_interrupt_driven( const char *message )
{
    uart_irq_message = message + 1; /* first char goes out by hand below */

    NVIC_ISER0 |= ( 1UL << UARTTX0_IRQn );
    CMSDK_UART0->CTRL |= UART_CTRL_TXIRQEN;

    while( CMSDK_UART0->STATE & UART_STATE_TXBF )
    {
    }
    CMSDK_UART0->DATA = (uint32_t) *message; /* triggers the first TX-empty IRQ */
}

/* Periodic tick: counted here, reported over the polled UART path so it
 * doesn't race with the interrupt-driven message above. */
static volatile uint32_t tick_count;

void SysTick_Handler( void )
{
    tick_count++;
}

static void systick_init( void )
{
    SysTick->LOAD = 0xFFFFFUL; /* free-running count; exact rate doesn't matter here */
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_ENABLE | SysTick_CTRL_TICKINT | SysTick_CTRL_CLKSOURCE;
}

/* Three fixed regions, fixed addresses, sized and placed entirely at
 * link time - see mps2_m3.ld's .task_partitions. No allocation, no
 * runtime layout decisions: exactly the static-memory scope scratch-OS
 * stays within (three-pillar-curriculum). */
extern uint8_t _task0_partition_start[], _task0_partition_end[];
extern uint8_t _task1_partition_start[], _task1_partition_end[];
extern uint8_t _task2_partition_start[], _task2_partition_end[];

static void fill_partition( uint8_t *start, uint8_t *end, uint8_t pattern )
{
    for( uint8_t *p = start; p < end; p++ )
    {
        *p = pattern;
    }
}

static int partition_is( const uint8_t *start, const uint8_t *end, uint8_t pattern )
{
    for( const uint8_t *p = start; p < end; p++ )
    {
        if( *p != pattern )
        {
            return 0;
        }
    }
    return 1;
}

/* Fill each task's partition with a distinct pattern, then check every
 * partition still holds only its own pattern - proof that writing one
 * task's region can't touch another's. */
static void check_partition_isolation( void )
{
    fill_partition( _task0_partition_start, _task0_partition_end, 0xAA );
    fill_partition( _task1_partition_start, _task1_partition_end, 0xBB );
    fill_partition( _task2_partition_start, _task2_partition_end, 0xCC );

    int ok = partition_is( _task0_partition_start, _task0_partition_end, 0xAA )
             && partition_is( _task1_partition_start, _task1_partition_end, 0xBB )
             && partition_is( _task2_partition_start, _task2_partition_end, 0xCC );

    uart_write_polled( ok ? "partitions OK\n" : "partitions FAILED\n" );
}

int main( void )
{
    uart_init();

    uart_write_polled( "polled OK\n" );
    uart_write_interrupt_driven( "interrupt OK\n" );
    check_partition_isolation();

    systick_init();

    uint32_t last_reported = 0;
    for( ;; )
    {
        __asm__ volatile ( "wfi" ); /* idle until the next interrupt */

        if( tick_count - last_reported >= 20 )
        {
            last_reported = tick_count;
            uart_write_polled( "tick\n" );
        }
    }
}
