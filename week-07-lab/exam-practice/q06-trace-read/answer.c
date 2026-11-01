/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Q6: your answer. See STATEMENT.md.
 */

#include <stdint.h>

/* One record of the trace: from the tick `tick` on, the task named `to`
 * runs instead of the task named `from`. '.' is the idle task. */
struct sw { uint32_t tick; char from; char to; };

/* A periodic task: its name, its execution time c, its period t. */
struct task { char name; uint32_t c; uint32_t t; };

/* (a) */
uint32_t run_ticks( const struct sw *rec, unsigned n, char name, uint32_t from, uint32_t to )
{
    ( void ) rec;
    ( void ) n;
    ( void ) name;
    ( void ) from;
    ( void ) to;
    return 0;                   /* replace with your code */
}

/* (b) */
uint32_t job_response( const struct sw *rec, unsigned n, uint32_t end, const struct task *tk, unsigned k )
{
    ( void ) rec;
    ( void ) n;
    ( void ) end;
    ( void ) tk;
    ( void ) k;
    return 0;                   /* replace with your code */
}

/* (c) */
uint32_t first_miss( const struct sw *rec, unsigned n, uint32_t end, const struct task *tasks, unsigned m, char *who )
{
    ( void ) rec;
    ( void ) n;
    ( void ) end;
    ( void ) tasks;
    ( void ) m;
    ( void ) who;
    return 0;                   /* replace with your code */
}

/* (d) */
int obeys_rm( const struct sw *rec, unsigned n, uint32_t end, const struct task *tasks, unsigned m )
{
    ( void ) rec;
    ( void ) n;
    ( void ) end;
    ( void ) tasks;
    ( void ) m;
    return 0;                   /* replace with your code */
}
