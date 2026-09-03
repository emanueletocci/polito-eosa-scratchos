/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Week 3 checkpoint: interrupts and IO.
 *  - polled UART write (busy-wait on the TX-buffer-full flag)
 *  - interrupt-driven UART write (TX-empty interrupt sends the rest of
 *    a message one character at a time)
 *  - a periodic SysTick interrupt, counted and reported over the polled
 *    UART path
 *
 * main() is no longer "run once and stop" - it becomes the idle loop
 * that everything else (week 5 onward) replaces with real scheduling.
 */

/* uint32_t, plus this course's hand-written register map. */
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
 * ever busy-waiting. `volatile` because the handler and this code both
 * touch it and an interrupt can land between any two instructions. */
static const char *volatile uart_irq_message;

/* Runs each time the UART finishes shifting out a byte and its tx buffer
 * goes empty. Sends the next character, or disables the interrupt when
 * the string is done. */
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
    CMSDK_UART0->CTRL |= UART_CTRL_TXIRQEN;              /* enable the tx-empty interrupt in the UART */

    while( CMSDK_UART0->STATE & UART_STATE_TXBF )        /* make sure the buffer has room */
    {
    }
    CMSDK_UART0->DATA = (uint32_t) *message;             /* send char 0; its completion fires the first IRQ */
}

/* Periodic tick: counted here, reported over the polled UART path so it
 * doesn't race with the interrupt-driven message above. `volatile`: the
 * handler writes it, the idle loop reads it. */
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

/* Entry point: bring up IO, prove both UART paths, then idle-loop
 * forever waking on each interrupt. */
int main( void )
{
    uart_init();                                         /* UART0 ready for output */

    uart_write_polled( "polled OK\n" );                  /* prove the polled path works */
    uart_write_interrupt_driven( "interrupt OK\n" );     /* prove the interrupt-driven path works */

    systick_init();                                      /* start the periodic tick */

    uint32_t last_reported = 0;                          /* tick_count value at the last "tick" line */
    for( ;; )
    {
        __asm__ volatile ( "wfi" );                     /* sleep until the next interrupt (the first time the CPU idles) */

        if( tick_count - last_reported >= 20 )          /* every 20 ticks... */
        {
            last_reported = tick_count;                 /* remember where we are */
            uart_write_polled( "tick\n" );             /* ...print one line */
        }
    }
}
