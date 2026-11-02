/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * System calls, kernel side: find which service `svc #n` asked for,
 * run it, and hand the result back to the caller.
 *
 * Everything here runs inside the SVCall exception, so it is
 * privileged: it may touch the drivers and the hardware registers.
 *
 * One service does not return to its caller: SYS_START_TASKS (main()
 * starts the first task). It enters a task instead, so svc_dispatch()
 * returns the EXC_RETURN value that ends the exception (see
 * kernel/svc_entry.s).
 *
 * Services that give the CPU away (yield, delay, delay_until, exit) do
 * not switch here: they change the task's state and make PendSV pending
 * (task_request_switch()). PendSV runs right after this exception ends.
 */

#include <stdint.h>
#include "kernel/syscall.h"
#include "lib/syscalls.h"
#include "drivers/uart.h"
#include "drivers/systick.h"
#include "kernel/task.h"
#include "kernel/trace.h"

/* Every service: up to three arguments (the caller's r0, r1, r2) and
 * one result (the caller's r0). */
typedef uint32_t ( *syscall_fn )( uint32_t a0, uint32_t a1, uint32_t a2 );

/* Is [p, p + len) fully inside [start, end)?
 * Written as `len <= end - p` so that p + len can never wrap around. */
static int in_range( uint32_t p, uint32_t len, uint32_t start, uint32_t end )
{
    return p >= start && p <= end && len <= end - p;
}

/* SYS_WRITE: a0 = buf, a1 = len. Put as many bytes as fit into the UART
 * send buffer and return how many were taken. It never waits: the UART
 * interrupt that makes room is less urgent than SVCall. */
static uint32_t sys_write_impl( uint32_t a0, uint32_t a1, uint32_t a2 )
{
    (void) a2;                                  /* not used */
    const char *buf = ( const char * ) a0;      /* the number is really an address */
    unsigned len = a1;

    /* Accept the buffer only inside the caller's partition or FLASH. */
    const struct tcb *t = tcb_current;
    if ( t != 0
         && !in_range( a0, len, t->part_start, t->part_end )
         && !in_range( a0, len, 0x00000000, 0x00400000 ) )
        return ( uint32_t ) -1;

    return uart_write( buf, len );              /* bytes accepted, 0..len */
}

/* SYS_READ: no arguments. Return one received byte, or -1 if none. */
static uint32_t sys_read_impl( uint32_t a0, uint32_t a1, uint32_t a2 )
{
    (void) a0;                                  /* not used */
    (void) a1;
    (void) a2;
    return ( uint32_t ) uart_getc_polled();
}

/* SYS_UPTIME: no arguments. Return the milliseconds since start. */
static uint32_t sys_uptime_impl( uint32_t a0, uint32_t a1, uint32_t a2 )
{
    (void) a0;                                  /* not used */
    (void) a1;
    (void) a2;
    return systick_uptime_ms();
}

/* The scheduler services answer -1 before the first task starts. */

/* SYS_YIELD: give up the rest of the time slice; the task stays READY. */
static uint32_t sys_yield_impl( uint32_t a0, uint32_t a1, uint32_t a2 )
{
    (void) a0;                                  /* not used */
    (void) a1;
    (void) a2;
    if ( tcb_current == 0 )
        return ( uint32_t ) -1;
    task_request_switch();                      /* PendSV runs after this exception */
    return 0;
}

/* SYS_DELAY: a0 = ticks. Block the caller for that many ticks from now. */
static uint32_t sys_delay_impl( uint32_t a0, uint32_t a1, uint32_t a2 )
{
    (void) a1;                                  /* not used */
    (void) a2;
    if ( tcb_current == 0 )
        return ( uint32_t ) -1;
    if ( a0 == 0 )
        task_request_switch();                  /* delay(0) = yield */
    else
        task_block_until( tick_count + a0 );    /* READY again at tick now + a0 */
    return 0;
}

/* SYS_DELAY_UNTIL: a0 = last_wake (address in the task's partition),
 * a1 = period. Block until tick *last_wake + period and store it in
 * *last_wake: the wake-ups stay periodic whatever the work takes. */
static uint32_t sys_delay_until_impl( uint32_t a0, uint32_t a1, uint32_t a2 )
{
    (void) a2;                                  /* not used */
    const struct tcb *t = tcb_current;
    if ( t == 0 )
        return ( uint32_t ) -1;

    /* The kernel WRITES *last_wake: only inside the caller's partition,
     * and aligned to 4. */
    if ( !in_range( a0, sizeof( uint32_t ), t->part_start, t->part_end )
         || ( a0 & 3 ) != 0 )
        return ( uint32_t ) -1;

    uint32_t *last_wake = ( uint32_t * ) a0;    /* the number is really an address */
    uint32_t wake = *last_wake + a1;            /* the next wake-up tick */
    *last_wake = wake;                          /* remembered for the next period */

    /* Wrap-safe test: a late task does not block at all. */
    if ( ( int32_t ) ( wake - tick_count ) > 0 )
        task_block_until( wake );
    return 0;
}

/* SYS_EXIT: the calling task is FINISHED and never chosen again. */
static uint32_t sys_exit_impl( uint32_t a0, uint32_t a1, uint32_t a2 )
{
    (void) a0;                                  /* not used */
    (void) a1;
    (void) a2;
    if ( tcb_current == 0 )
        return ( uint32_t ) -1;
    task_finish_current();                      /* PendSV switches away after this exception */
    return 0;
}

/* SYS_TICK_COUNT: the ticks since the scheduler started (1 tick = 10 ms). */
static uint32_t sys_tick_count_impl( uint32_t a0, uint32_t a1, uint32_t a2 )
{
    (void) a0;                                  /* not used */
    (void) a1;
    (void) a2;
    return tick_count;
}

/* SYS_TRACE_DUMP: print the scheduler trace (kernel/trace.c). */
static uint32_t sys_trace_dump_impl( uint32_t a0, uint32_t a1, uint32_t a2 )
{
    (void) a0;                                  /* not used */
    (void) a1;
    (void) a2;
    trace_print();
    return 0;
}

/* The dispatch table: table[n] is the function for service n. Entry 0
 * and SYS_START_TASKS are empty: svc_dispatch handles the latter. */
static const syscall_fn table[] = {
    [SYS_WRITE]       = sys_write_impl,
    [SYS_READ]        = sys_read_impl,
    [SYS_UPTIME]      = sys_uptime_impl,
    [SYS_EXIT]        = sys_exit_impl,
    [SYS_YIELD]       = sys_yield_impl,
    [SYS_DELAY]       = sys_delay_impl,
    [SYS_DELAY_UNTIL] = sys_delay_until_impl,
    [SYS_TRACE_DUMP]  = sys_trace_dump_impl,
    [SYS_TICK_COUNT]  = sys_tick_count_impl,
};

/* How many entries the table has (here 11: numbers 0..10). */
#define TABLE_SIZE ( sizeof( table ) / sizeof( table[0] ) )

/* EXC_RETURN bit 2: 1 if the caller was on the process stack (a task),
 * 0 if it was on the main stack (main(), the kernel). */
#define EXC_RETURN_PSP_BIT 0x4u

/* frame points at the 8 registers the hardware pushed (r0-r3, r12, lr,
 * pc, xPSR). exc_return is the EXC_RETURN of this exception; the
 * function returns the one SVC_Handler must use to end it. */
uint32_t svc_dispatch( uint32_t *frame, uint32_t exc_return )
{
    /* Which service? n is the low byte of the `svc #n` instruction,
     * 2 bytes before the saved pc (frame[6]). */
    uint8_t n = ( ( uint8_t * ) frame[6] )[-2];

    /* Did a task ask (process stack), or main() (main stack)? */
    int from_task = ( exc_return & EXC_RETURN_PSP_BIT ) != 0;

    /* SYS_START_TASKS: kernel only; a task gets -1 below. */
    if ( n == SYS_START_TASKS && !from_task && tcb_current == 0 )
    {
        return task_start_first();      /* enter the first task */
    }

    /* An unknown number, or 0, or an empty table entry: answer -1. */
    if ( n >= TABLE_SIZE || table[n] == 0 )
    {
        frame[0] = ( uint32_t ) -1;
        return exc_return;              /* back to the caller */
    }

    /* Run the service; its result goes into the saved r0. */
    frame[0] = table[n]( frame[0], frame[1], frame[2] );
    return exc_return;                  /* back to the caller, as it came */
}
