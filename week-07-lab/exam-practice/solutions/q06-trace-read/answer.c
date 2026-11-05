/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Q6: the reference solution.
 */

#include <stdint.h>

/* One record of the trace: from the tick `tick` on, the task named `to`
 * runs instead of the task named `from`. '.' is the idle task. */
struct sw { uint32_t tick; char from; char to; };

/* A periodic task: its name, its execution time c, its period t. */
struct task { char name; uint32_t c; uint32_t t; };

/* The task that runs in slot s: the last record at or before s decides. */
static char runner( const struct sw *rec, unsigned n, uint32_t s )
{
    char who = rec[0].from;
    for ( unsigned i = 0; i < n && rec[i].tick <= s; i++ )
    {
        who = rec[i].to;
    }
    return who;
}

/* (a) Slot by slot, from included to excluded. */
uint32_t run_ticks( const struct sw *rec, unsigned n, char name, uint32_t from, uint32_t to )
{
    uint32_t count = 0;
    for ( uint32_t s = from; s < to; s++ )
    {
        if ( runner( rec, n, s ) == name )
        {
            count++;
        }
    }
    return count;
}

/* (b) The job finishes at the first tick f by which the task has run
 * k * c slots in all; a switch away from the task is not a finish, since
 * the job may only have been preempted. */
uint32_t job_response( const struct sw *rec, unsigned n, uint32_t end, const struct task *tk, unsigned k )
{
    uint32_t release = ( k - 1 ) * tk->t;
    for ( uint32_t f = release + 1; f <= end; f++ )
    {
        if ( run_ticks( rec, n, tk->name, 0, f ) >= k * tk->c )
        {
            return f - release;
        }
    }
    return 0;
}

/* (c) A job misses its deadline d when it has not finished by d: no
 * finish by end, or a response time above the period. Finishing exactly
 * at d meets it. Only the first miss of each task matters. */
uint32_t first_miss( const struct sw *rec, unsigned n, uint32_t end, const struct task *tasks, unsigned m, char *who )
{
    uint32_t best = 0;
    for ( unsigned i = 0; i < m; i++ )
    {
        for ( unsigned k = 1; k * tasks[i].t <= end; k++ )
        {
            uint32_t r = job_response( rec, n, end, &tasks[i], k );
            if ( r == 0 || r > tasks[i].t )
            {
                uint32_t d = k * tasks[i].t;
                if ( best == 0 || d < best )
                {
                    best = d;
                    *who = tasks[i].name;
                }
                break;
            }
        }
    }
    return best;
}

/* (d) In each slot, the pending tasks are those with more jobs released
 * than finished; RM runs one with the shortest period, and idles only
 * when none is pending. */
int obeys_rm( const struct sw *rec, unsigned n, uint32_t end, const struct task *tasks, unsigned m )
{
    for ( uint32_t s = 0; s < end; s++ )
    {
        uint32_t tmin = 0;
        int pending[4];
        for ( unsigned i = 0; i < m; i++ )
        {
            uint32_t released = s / tasks[i].t + 1;
            uint32_t done = run_ticks( rec, n, tasks[i].name, 0, s ) / tasks[i].c;
            pending[i] = released > done;
            if ( pending[i] && ( tmin == 0 || tasks[i].t < tmin ) )
            {
                tmin = tasks[i].t;
            }
        }
        char x = runner( rec, n, s );
        int ok = ( tmin == 0 && x == '.' );
        for ( unsigned i = 0; i < m; i++ )
        {
            if ( pending[i] && tasks[i].t == tmin && tasks[i].name == x )
            {
                ok = 1;
            }
        }
        if ( !ok )
        {
            return 0;
        }
    }
    return 1;
}
