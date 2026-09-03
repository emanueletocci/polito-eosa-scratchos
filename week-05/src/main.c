/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Week 5 checkpoint: a tick-driven cyclic-executive scheduler, on top of
 * week 3's interrupts/IO and week 4's memory partitions.
 *  - polled UART write (busy-wait on the TX-buffer-full flag)
 *  - interrupt-driven UART write (TX-empty interrupt sends the rest of
 *    a message one character at a time)
 *  - three fixed-address, fixed-size memory partitions (mps2_m3.ld),
 *    proven isolated (week 4's check), then reused as three tasks'
 *    private stacks
 *  - three tasks, round-robin dispatched every SysTick tick, each with
 *    its own stack and its own saved register context
 *
 * main() no longer runs at all after setup - it hands off to task0 and
 * never returns. SysTick_Handler does the actual context switch: this
 * week's second just-in-time assembly dose (see course-material's
 * slide-sources/lecture-notes/week-05.md), reusing exactly the
 * ldr/mov-into-SP skill from week 2, just executed on every tick
 * instead of once at boot.
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

/* Start the periodic timer that drives the context switch below. */
static void systick_init( void )
{
    SysTick->LOAD = 0xFFFFFUL; /* reload value; the exact tick rate doesn't matter, only that it's periodic */
    SysTick->VAL = 0;          /* clear the current count so the first period is full-length */
    SysTick->CTRL = SysTick_CTRL_ENABLE | SysTick_CTRL_TICKINT | SysTick_CTRL_CLKSOURCE; /* run, interrupt, core clock */
}

/* Three fixed regions, fixed addresses, sized and placed entirely at
 * link time - see mps2_m3.ld's .task_partitions. No allocation, no
 * runtime layout decisions: exactly the static-memory scope scratch-OS
 * stays within (three-pillar-curriculum). Here they double as the three
 * tasks' private stacks. */
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

    /* Every region must still read back its own pattern. */
    int ok = partition_is( _task0_partition_start, _task0_partition_end, 0xAA )
             && partition_is( _task1_partition_start, _task1_partition_end, 0xBB )
             && partition_is( _task2_partition_start, _task2_partition_end, 0xCC );

    uart_write_polled( ok ? "partitions OK\n" : "partitions FAILED\n" ); /* report the verdict */
}

/* ---- Cyclic-executive scheduler: fixed round-robin over 3 tasks ---- */

#define TASK_COUNT 3   /* number of tasks the dispatcher cycles through */

/* A task control block. Here the entire saved state of a task IS its
 * stack pointer: everything else lives on that task's own stack. */
typedef struct
{
    uint32_t *sp; /* saved stack pointer - the entire saved context */
} tcb_t;

static tcb_t tasks[ TASK_COUNT ];          /* one TCB per task */
static volatile uint32_t current_task;     /* index of the task running now; written from asm */

/* Craft a stack frame that looks exactly like one SysTick_Handler would
 * have saved, so the very first time this task is switched IN (via the
 * normal restore path below) it's indistinguishable from resuming a
 * task that was really interrupted mid-execution. r4-r11 and r0-r3/r12
 * are dummies (a fresh task has no prior state to resume); LR is 0
 * (task functions never return); PC is the entry point; xPSR's T-bit
 * must be set - this course runs Thumb code exclusively. */
static uint32_t *task_stack_init( uint8_t *stack_top, void ( *entry )( void ) )
{
    uint32_t *sp = (uint32_t *) stack_top;  /* start at the top; the stack grows down as we push */

    *( --sp ) = 0x01000000UL;     /* xPSR: only the Thumb (T) bit set */
    *( --sp ) = (uint32_t) entry; /* PC: where the task starts executing */
    *( --sp ) = 0;                /* LR: 0, task entry functions never return */
    *( --sp ) = 0;                /* r12: dummy */
    *( --sp ) = 0;                /* r3: dummy */
    *( --sp ) = 0;                /* r2: dummy */
    *( --sp ) = 0;                /* r1: dummy */
    *( --sp ) = 0;                /* r0: dummy */

    for( int i = 0; i < 8; i++ )  /* r4-r11: eight more dummy words, the part the handler saves by hand */
    {
        *( --sp ) = 0;
    }

    return sp;                    /* the task's initial saved SP */
}

/* Per-task iteration counters; volatile because tasks are preempted at
 * any instruction and the values are inspected across switches. */
static volatile uint32_t task_run_count[ TASK_COUNT ];

/* Task 0: spin, and print occasionally so the round-robin is visible. */
void task0_entry( void )
{
    for( ;; )
    {
        if( ++task_run_count[ 0 ] % 200000 == 0 ) /* throttle output to roughly once per many switches */
        {
            uart_write_polled( "task0\n" );
        }
    }
}

/* Task 1: identical body, different index and message. */
void task1_entry( void )
{
    for( ;; )
    {
        if( ++task_run_count[ 1 ] % 200000 == 0 )
        {
            uart_write_polled( "task1\n" );
        }
    }
}

/* Task 2: identical body, different index and message. */
void task2_entry( void )
{
    for( ;; )
    {
        if( ++task_run_count[ 2 ] % 200000 == 0 )
        {
            uart_write_polled( "task2\n" );
        }
    }
}

/* The scheduling *policy*: a fixed, repeating sequence - task 0, 1, 2,
 * 0, 1, 2, ... This is the "cyclic executive" / timeline strategy from
 * this week's theory. An ordinary C function, deliberately: only the
 * context save/restore around it needs to be assembly. */
void scheduler_pick_next( void )
{
    current_task = ( current_task + 1 ) % TASK_COUNT; /* advance the index, wrapping at TASK_COUNT */
}

/* The context switch itself: assembly, because C has no way to express
 * "save every register the currently-running code might be using, onto
 * its own stack, then load a completely different set from a different
 * stack." mrs/msr and stmdb/ldmia operate purely through r0 as an
 * explicit pointer - the real SP register is never touched until the
 * final msr, so the hardware-saved exception frame this handler was
 * entered with is never at risk of being overwritten mid-switch. LR
 * (the EXC_RETURN value that makes the final `bx lr` a proper exception
 * return) is stashed in r4 across the `bl` - r4 is callee-saved, so an
 * AAPCS-conforming C function is guaranteed not to disturb it. */
__attribute__( ( naked ) )
void SysTick_Handler( void )
{
    __asm__ volatile (
        "mrs r0, msp             \n" /* r0 = the interrupted task's SP (points at the hardware-saved frame) */
        "stmdb r0!, {r4-r11}     \n" /* save current task's r4-r11 onto its own stack */

        "ldr r1, =tasks          \n" /* r1 = &tasks[0] */
        "ldr r2, =current_task   \n" /* r2 = &current_task */
        "ldr r3, [r2]            \n" /* r3 = current_task */
        "lsl r3, r3, #2          \n" /* r3 *= 4: tcb_t is one word (sp only) */
        "add r1, r1, r3          \n" /* r1 = &tasks[current_task] */
        "str r0, [r1]            \n" /* tasks[current_task].sp = r0 (save the outgoing SP) */

        "mov r4, lr              \n" /* stash EXC_RETURN - r4 survives the call below */
        "bl scheduler_pick_next  \n" /* pick the next task (may update current_task) */
        "mov lr, r4              \n" /* restore EXC_RETURN into lr */

        "ldr r1, =tasks          \n" /* recompute &tasks[current_task] with the new index */
        "ldr r2, =current_task   \n" /* r2 = &current_task */
        "ldr r3, [r2]            \n" /* r3 = current_task (possibly just changed) */
        "lsl r3, r3, #2          \n" /* r3 *= 4: one word per tcb_t */
        "add r1, r1, r3          \n" /* r1 = &tasks[current_task] */
        "ldr r0, [r1]            \n" /* r0 = tasks[current_task].sp (the incoming SP) */
        "ldmia r0!, {r4-r11}     \n" /* restore the new task's r4-r11 */
        "msr msp, r0             \n" /* switch stacks - only now does the real SP move */
        "bx lr                   \n" /* exception return: hardware pops r0-r3,r12,lr,pc,xpsr from the new stack */
    );
}

/* Launching task0 needs none of the above trickery: nothing meaningful
 * is running yet, so there's no context to preserve. Just move onto
 * task0's own stack and call it like an ordinary function - it never
 * returns. Every task after this first one is only ever entered via
 * the SysTick_Handler restore path above. */
__attribute__( ( naked ) )
void start_first_task( void )
{
    __asm__ volatile (
        "ldr r0, =_task0_partition_end \n" /* r0 = top of task0's stack region */
        "msr msp, r0                    \n" /* run on task0's stack from here on */
        "bl task0_entry                 \n" /* enter task0; it never returns */
    );
}

/* Entry point: bring up IO, prove the UART paths and partitions, build
 * the other tasks' initial stacks, then hand off to task0 - never
 * returns. */
int main( void )
{
    uart_init();                                        /* UART0 ready for output */

    uart_write_polled( "polled OK\n" );                 /* polled path works */
    uart_write_interrupt_driven( "interrupt OK\n" );    /* interrupt-driven path works */
    check_partition_isolation();                        /* partitions are isolated */

    tasks[ 1 ].sp = task_stack_init( _task1_partition_end, task1_entry ); /* pre-build task 1's saved frame */
    tasks[ 2 ].sp = task_stack_init( _task2_partition_end, task2_entry ); /* pre-build task 2's saved frame */
    current_task = 0; /* task0 is bootstrapped directly below, not via a saved context */

    systick_init();                                     /* start the tick that will drive switches */
    start_first_task();                                 /* jump into task0; does not return */

    for( ;; )                                           /* unreachable - keeps the compiler happy */
    {
    }
}
