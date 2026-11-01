/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Q4: your answer. See STATEMENT.md. Write your code only between the
 * two lines that mark each gap, and leave those lines as they are.
 */

#include <stdint.h>
#include "cmsdk.h"
#include "kernel/task.h"

#define IN_SIZE 16

/* The ring of received bytes: in_head is where the next byte goes, in_tail
 * the oldest byte not yet read; the ring is empty when they are equal. */
static volatile uint8_t in_buf[IN_SIZE];
static volatile unsigned in_head;           /* written by the handler only */
static volatile unsigned in_tail;           /* written by sys_read_impl only */
static struct tcb *reader;                  /* the task waiting for input, or 0 */

/* At boot: the receive interrupt of UART0 on, at priority 0xC0. */
void input_init( void )
{
    CMSDK_UART0->CTRL |= UART_CTRL_RXIRQEN;
    NVIC->IPR[UART0RX_IRQn] = 0xC0;
    NVIC->ISER[UART0RX_IRQn / 32] = 1UL << ( UART0RX_IRQn % 32 );
}

/* The handler of the UART0 receive interrupt. */
void UART0RX_Handler( void )
{
    /* ---- gap (a) ---- */
    /* ---- end of gap (a) ---- */
    while ( CMSDK_UART0->STATE & UART_STATE_RXBF )
    {
        uint8_t c = (uint8_t) CMSDK_UART0->DATA;
        unsigned next = ( in_head + 1 ) & ( IN_SIZE - 1 );
        /* ---- gap (d) ---- */
        /* ---- end of gap (d) ---- */
    }

    if ( reader != 0 && reader->state == TASK_WAITING )
    {
        /* ---- gap (c) ---- */
        /* ---- end of gap (c) ---- */
    }
}

/* The system call sys_read, inside SVCall. */
uint32_t sys_read_impl( uint32_t a0, uint32_t a1, uint32_t a2 )
{
    (void) a0;
    (void) a1;
    (void) a2;
    if ( in_tail != in_head )
    {
        uint8_t c = in_buf[in_tail];
        in_tail = ( in_tail + 1 ) & ( IN_SIZE - 1 );
        return c;
    }
    /* ---- gap (b) ---- */
    /* ---- end of gap (b) ---- */
    return (uint32_t) -1;
}
