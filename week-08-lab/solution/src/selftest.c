/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * The check declared in src/selftest.h. Each check compares a returned
 * value or the word itself with what lib/atomic.h promises. It runs in
 * one task, with no other task touching the word, so it checks what
 * each operation computes and returns, not that it is atomic: the
 * counter phase checks that.
 */

#include "src/selftest.h"
#include "lib/atomic.h"
#include "lib/printf.h"

/* The tally lives on the task's stack: a task may touch only its own
 * memory and the shared block, not a global of this file. */
typedef struct
{
    uint32_t checks;
    uint32_t passed;
} tally_t;

/* One check: count it, and print it if it failed. */
static void check( tally_t *t, const char *what, uint32_t got, uint32_t want )
{
    t->checks++;
    if ( got == want )
    {
        t->passed++;
    }
    else
    {
        printf( "atomic test: %s is %u, expected %u\n", what, got, want );
    }
}

void atomic_selftest( void )
{
    tally_t t = { 0, 0 };
    volatile uint32_t word = 5;
    uint32_t r;

    r = atomic_exchange( &word, 9 );
    check( &t, "atomic_exchange(5 -> 9) result", r, 5 );
    check( &t, "word after atomic_exchange", word, 9 );

    r = atomic_fetch_add( &word, 3 );
    check( &t, "atomic_fetch_add(9, 3) result", r, 9 );
    check( &t, "word after atomic_fetch_add", word, 12 );

    r = atomic_cas( &word, 12, 20 );
    check( &t, "atomic_cas(12, expected 12, desired 20) result", r, 12 );
    check( &t, "word after a cas that matches", word, 20 );

    r = atomic_cas( &word, 12, 30 );
    check( &t, "atomic_cas(20, expected 12, desired 30) result", r, 20 );
    check( &t, "word after a cas that does not match", word, 20 );

    printf( "atomic test: %u of %u checks passed\n", t.passed, t.checks );
}
