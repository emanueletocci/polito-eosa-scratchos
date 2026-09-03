/*
 * Embedded Operating Systems and Architecture course
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

/* uint32_t/uint8_t, plus this course's hand-written register map. */
#include <stdint.h>
#include "../include/cmsdk.h"

/* Bring UART0 up for transmit: set a valid baud divider, enable the TX. */
static void uart_init( void )
{
    CMSDK_UART0->BAUDDIV = 16;             /* 16 is the minimum divider the hardware accepts */
    CMSDK_UART0->CTRL = UART_CTRL_TXEN;    /* transmitter on; every other control bit off */
}

/* Polled: busy-wait for room in the TX buffer before every character. */
static void uart_write_polled( const char *message )
{
    while( *message )                                   /* until the terminating NUL */
    {
        while( CMSDK_UART0->STATE & UART_STATE_TXBF )   /* spin while the tx buffer is full */
        {
        }
        CMSDK_UART0->DATA = (uint32_t) *message++;      /* enqueue this char, advance the pointer */
    }
}

/* Interrupt-driven: prime the first character by hand, then the TX-empty
 * interrupt sends the rest, one character per interrupt, without main()
 * ever busy-waiting. `volatile`: handler and caller both touch it. */
static const char *volatile uart_irq_message;

/* Runs when the UART's tx buffer goes empty: send the next character,
 * or disable the interrupt once the string is done. */
void UART0TX_Handler( void )
{
    CMSDK_UART0->INTSTATUS = UART_INT_TXIRQ; /* write-1-to-clear: acknowledge this IRQ */

    if( *uart_irq_message )                              /* more characters left? */
    {
        CMSDK_UART0->DATA = (uint32_t) *uart_irq_message++; /* send one, advance */
    }
    else
    {
        CMSDK_UART0->CTRL &= ~UART_CTRL_TXIRQEN;         /* string done: stop asking for tx IRQs */
    }
}

/* Kick off an interrupt-driven send: the handler above does the rest. */
static void uart_write_interrupt_driven( const char *message )
{
    uart_irq_message = message + 1;                      /* handler starts at char 1; char 0 goes out below */

    NVIC_ISER0 |= ( 1UL << UARTTX0_IRQn );               /* enable UART0-TX at the NVIC */
    CMSDK_UART0->CTRL |= UART_CTRL_TXIRQEN;             /* enable the tx-empty interrupt in the UART */

    while( CMSDK_UART0->STATE & UART_STATE_TXBF )        /* make sure the buffer has room */
    {
    }
    CMSDK_UART0->DATA = (uint32_t) *message;             /* send char 0; its completion fires the first IRQ */
}

/* Periodic tick: counted here, reported over the polled UART path so it
 * doesn't race with the interrupt-driven message above. */
static volatile uint32_t tick_count;

/* SysTick exception handler: just advance the tick counter. */
void SysTick_Handler( void )
{
    tick_count++;
}

/* Start the periodic timer. */
static void systick_init( void )
{
    SysTick->LOAD = 0xFFFFFUL; /* reload value; the exact tick rate doesn't matter for this demo */
    SysTick->VAL = 0;          /* clear the current count so the first period is full-length */
    SysTick->CTRL = SysTick_CTRL_ENABLE | SysTick_CTRL_TICKINT | SysTick_CTRL_CLKSOURCE; /* run, interrupt, core clock */
}

/* Three fixed regions, fixed addresses, sized and placed entirely at
 * link time - see mps2_m3.ld's .task_partitions. No allocation, no
 * runtime layout decisions: exactly the static-memory scope scratch-OS
 * stays within (three-pillar-curriculum). Declared as arrays so the
 * symbol's address is the region's address. */
extern uint8_t _task0_partition_start[], _task0_partition_end[];
extern uint8_t _task1_partition_start[], _task1_partition_end[];
extern uint8_t _task2_partition_start[], _task2_partition_end[];

/* Write `pattern` to every byte in [start, end). */
static void fill_partition( uint8_t *start, uint8_t *end, uint8_t pattern )
{
    for( uint8_t *p = start; p < end; p++ )   /* walk the region byte by byte */
    {
        *p = pattern;                         /* stamp this byte */
    }
}

/* Return 1 if every byte in [start, end) still equals `pattern`, else 0. */
static int partition_is( const uint8_t *start, const uint8_t *end, uint8_t pattern )
{
    for( const uint8_t *p = start; p < end; p++ )  /* check each byte */
    {
        if( *p != pattern )                       /* any mismatch means something else wrote here */
        {
            return 0;
        }
    }
    return 1;                                     /* all bytes intact */
}

/* Fill each task's partition with a distinct pattern, then check every
 * partition still holds only its own pattern - proof that writing one
 * task's region can't touch another's. */
static void check_partition_isolation( void )
{
    fill_partition( _task0_partition_start, _task0_partition_end, 0xAA ); /* region 0 -> 0xAA */
    fill_partition( _task1_partition_start, _task1_partition_end, 0xBB ); /* region 1 -> 0xBB */
    fill_partition( _task2_partition_start, _task2_partition_end, 0xCC ); /* region 2 -> 0xCC */

    /* Every region must still read back its own pattern, untouched by
     * the other two fills. */
    int ok = partition_is( _task0_partition_start, _task0_partition_end, 0xAA )
             && partition_is( _task1_partition_start, _task1_partition_end, 0xBB )
             && partition_is( _task2_partition_start, _task2_partition_end, 0xCC );

    uart_write_polled( ok ? "partitions OK\n" : "partitions FAILED\n" ); /* report the verdict */
}

/* Entry point: bring up IO, prove the UART paths and partition
 * isolation, then idle-loop forever waking on each interrupt. */
int main( void )
{
    uart_init();                                        /* UART0 ready for output */

    uart_write_polled( "polled OK\n" );                 /* polled path works */
    uart_write_interrupt_driven( "interrupt OK\n" );    /* interrupt-driven path works */
    check_partition_isolation();                        /* partitions are isolated */

    systick_init();                                     /* start the periodic tick */

    uint32_t last_reported = 0;                         /* tick_count value at the last "tick" line */
    for( ;; )
    {
        __asm__ volatile ( "wfi" );                    /* idle until the next interrupt */

        if( tick_count - last_reported >= 20 )         /* every 20 ticks... */
        {
            last_reported = tick_count;                /* remember where we are */
            uart_write_polled( "tick\n" );            /* ...print one line */
        }
    }
}
