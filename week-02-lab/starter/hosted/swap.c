/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * WEEK 2 LAB STARTER - not a checkpoint.
 *
 * Exercise A3. This file builds and runs as supplied; the three pieces
 * marked TODO are yours to complete, in the order the lab gives them.
 * Build and run it before changing anything, and note what it does.
 */

#include <stdio.h>

/* Supplied by newlib's semihosting library, and declared here because
 * it appears in no public header. It establishes the file descriptors
 * that stdio writes through. */
extern void initialise_monitor_handles(void);

void swap_c(int *a, int *b)
{
    int temp;
    temp = *a;
    *a = *b;
    *b = temp;
}

/* (A3, step 3): exchange the values *a and *b, in ARM assembly.
 *
 * `naked` suppresses the compiler-generated prologue and epilogue, so
 * the body is exactly the instructions below and nothing else - which
 * is why the closing `bx lr` is already present and must remain last.
 * Add your instructions before it.
 *
 * The first argument arrives in r0 and the second in r1; r2 and r3 are
 * scratch registers you may use freely. */

// r0 = a  (indirizzo di x)
// r1 = b  (indirizzo di y)
__attribute__((naked)) void swap_asm(int *a, int *b)
{
    __asm__ volatile(
        "ldr r2, [r0]\n" /* r2 = *a        */
        "ldr r3, [r1]\n" /* r3 = *b        */
        "str r3, [r0]\n" /* *a = r3        */
        "str r2, [r1]\n" /* *b = r2        */
        "bx  lr\n");
}

int main(void)
{
    int x, y;

    initialise_monitor_handles();

    x = 1;
    y = 2;
    printf("C   before: x=%d y=%d\n", x, y);
    swap_c(&x, &y);
    printf("C   after:  x=%d y=%d\n", x, y);

    x = 1;
    y = 2;
    printf("asm before: x=%d y=%d\n", x, y);
    swap_asm(&x, &y);
    printf("asm after:  x=%d y=%d\n", x, y);

    return 0;
}
