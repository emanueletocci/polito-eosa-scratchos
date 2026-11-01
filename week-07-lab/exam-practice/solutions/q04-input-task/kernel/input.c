/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Q4: the reference solution, with the gaps filled. A ring of 16 bytes
 * filled by the receive interrupt handler and emptied by sys_read_impl();
 * the reader waits in TASK_WAITING while the ring is empty.
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

/* The handler of the UART0 receive interrupt.
 * (a) The acknowledge comes first: without it the interrupt would fire
 *     again at once, for ever, and nothing else would run. After the
 *     loop, it could clear the request of a byte that arrived during the
 *     loop, and that byte, unread, would block every byte after it.
 * (d) A full ring (15 bytes waiting) keeps its bytes and drops the new
 *     one: storing it would make in_head equal to in_tail, which means
 *     "empty", and lose all 15.
 * (c) A woken reader that outranks the running task takes the CPU at
 *     once: PendSV runs as soon as this handler returns, from a task or
 *     from the idle task alike. reader = 0: the wake-up happens once. */
void UART0RX_Handler( void )
{
    /* ---- gap (a) ---- */
    CMSDK_UART0->INTSTATUS = UART_INT_RX;
    /* ---- end of gap (a) ---- */
    while ( CMSDK_UART0->STATE & UART_STATE_RXBF )
    {
        uint8_t c = (uint8_t) CMSDK_UART0->DATA;
        unsigned next = ( in_head + 1 ) & ( IN_SIZE - 1 );
        /* ---- gap (d) ---- */
        if ( next != in_tail )
        {
            in_buf[in_head] = c;
            in_head = next;
        }
        /* ---- end of gap (d) ---- */
    }

    if ( reader != 0 && reader->state == TASK_WAITING )
    {
        /* ---- gap (c) ---- */
        reader->state = TASK_READY;
        if ( reader->priority > tcb_current->priority )
        {
            task_request_switch();
        }
        reader = 0;
        /* ---- end of gap (c) ---- */
    }
}

/* The system call sys_read, inside SVCall.
 * (b) With the ring empty, the caller waits: the handler will find it in
 *     `reader`, TASK_WAITING keeps the scheduler and the tick from
 *     choosing it, and the switch gives the CPU away now, not at the end
 *     of the slice. It returns -1; the task asks again once woken.
 * SVCall (0x80) outranks the receive interrupt (0xC0), so the handler
 * cannot run between the test of the ring above and the wait here. With
 * the receive interrupt above SVCall, a byte could arrive in between: the
 * handler would find no reader, and B would wait for the next byte with
 * one already in the ring. The question does not score this race: it
 * depends on when the byte comes. */
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
    reader = tcb_current;
    tcb_current->state = TASK_WAITING;
    task_request_switch();
    /* ---- end of gap (b) ---- */
    return (uint32_t) -1;
}
