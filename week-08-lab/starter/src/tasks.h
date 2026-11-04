/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * The tasks: the code that runs unprivileged, after the kernel has
 * started. A and B take turns on the CPU (round robin); the idle task
 * runs when neither can. In week 7 A and B do the same work on data
 * they share, to show what goes wrong without synchronization.
 */
#ifndef TASKS_H
#define TASKS_H

/* Task A: adds to the shared counter, then prints lines of 'A', then
 * returns (= exit). */
void task_a( void );

/* Task B: the same as A, with lines of 'B'. */
void task_b( void );

/* The idle task: sleeps (`wfi`) for ever. Never returns. */
void idle_task( void );

/* The work of week 7. Which phases run depends on the build
 * (`make DEMO=...`, see the Makefile). Default: both phases. */

/* Phase 1, "counter": each task adds 1 to shared_counter this many times. */
#define COUNT_N 1000000
/* Phase 2, "lines": each task prints this many lines ... */
#define LINES_N 3000
/* ... of this many characters each. */
#define LINE_LEN 100

#if defined( DEMO_ATOMICTEST )
#define DO_COUNTER 0        /* neither phase: task A checks lib/atomic.s */
#define DO_LINES 0
#elif defined( DEMO_RACE ) || defined( DEMO_CPSID )
#define DO_COUNTER 0        /* lines phase only */
#define DO_LINES 1
#elif defined( DEMO_COUNTER )
#define DO_COUNTER 1        /* counter phase only */
#define DO_LINES 0
#else
#define DO_COUNTER 1        /* default (and spin): both phases */
#define DO_LINES 1
#endif

/* Around each printf. Normally they do nothing. With DEMO=cpsid they
 * try to switch interrupts off (cpsid i) and on again (cpsie i), so no
 * tick could interrupt the printf. They have NO effect here: the tasks
 * are unprivileged, and an unprivileged write to PRIMASK is ignored. */
#if defined( DEMO_CPSID )
#define PRINT_BEGIN() __asm volatile ( "cpsid i" ::: "memory" )   /* try: interrupts off */
#define PRINT_END()   __asm volatile ( "cpsie i" ::: "memory" )   /* try: interrupts on */
#else
#define PRINT_BEGIN() do { } while ( 0 )    /* nothing */
#define PRINT_END()   do { } while ( 0 )    /* nothing */
#endif

/* The lock around each shared update and each printf (week 7).
 * The race demos (DEMO=race, counter, cpsid) must show what goes
 * wrong WITHOUT a lock, so there LOCK and UNLOCK do nothing.
 * DEMO=spin takes the lock with spin_lock_busy (plain spinning), to
 * show the wasted time slices in the trace. The default build uses
 * spin_lock (spin-then-yield). */
/* Laboratory 8: how the counter phase updates shared_counter.
 *   DEMO=fetchadd - no lock: counter_add_fetchadd (src/counter.c);
 *   DEMO=cas      - no lock: counter_add_cas (src/counter.c);
 *   DEMO=ticket   - under the ticket lock counter_ticket (lib/locks.h);
 *   otherwise     - under counter_lock, through LOCK / UNLOCK.
 * COUNTER_LOCK / COUNTER_UNLOCK are what the lock builds call. */
#if defined( DEMO_TICKET )
#define COUNTER_LOCK()   ticket_lock( &counter_ticket )
#define COUNTER_UNLOCK() ticket_unlock( &counter_ticket )
#else
#define COUNTER_LOCK()   LOCK( &counter_lock )
#define COUNTER_UNLOCK() UNLOCK( &counter_lock )
#endif

#if defined( DEMO_RACE ) || defined( DEMO_COUNTER ) || defined( DEMO_CPSID )
#define LOCK( l )   do { } while ( 0 )      /* no lock: the race shows */
#define UNLOCK( l ) do { } while ( 0 )      /* no lock: the race shows */
#elif defined( DEMO_SPIN )
#define LOCK( l )   spin_lock_busy( l )     /* take it, busy waiting */
#define UNLOCK( l ) spin_unlock( l )        /* give it back */
#elif defined( DEMO_TAS )
#define LOCK( l )   tas_lock( l )           /* laboratory 8: the same lock, in C (lib/locks.c) */
#define UNLOCK( l ) tas_unlock( l )         /* give it back */
#else
#define LOCK( l )   spin_lock( l )          /* take it, yield while taken */
#define UNLOCK( l ) spin_unlock( l )        /* give it back */
#endif

#endif /* TASKS_H */
