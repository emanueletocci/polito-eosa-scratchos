# Q1. The utilisation bound of rate monotonic

A set of `n` periodic tasks is scheduled by rate monotonic (RM) on one
processor. Task `i` has an execution time `c[i]` and a period `t[i]`, and
its deadline is its period. In this question `n` is between 1 and 10,
every `c[i]` and `t[i]` is between 1 and 1000, and `c[i]` may be larger
than `t[i]`.

The utilisation is worked in thousandths, with integers only: **U×1000**
is the sum, over the tasks, of `(1000 × c[i]) / t[i]`, each term divided
with integer division (truncated). The bound `U_lub(n)` is used as these
values, also in thousandths:

| n | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
|---|---|---|---|---|---|---|---|---|---|---|
| U_lub(n) × 1000 | 1000 | 828 | 780 | 757 | 743 | 735 | 729 | 724 | 721 | 718 |

## What you write

In `answer.c`, two functions. Keep their signatures.

**(a)** `uint32_t util_x1000( const uint32_t *c, const uint32_t *t, unsigned n )`

Returns U×1000, worked as above.

**(b)** `int ll_test( const uint32_t *c, const uint32_t *t, unsigned n )`

Returns what the utilisation says about the set under RM:

- 2 if U×1000 is at most `U_lub(n)` × 1000 of the table: the set is
  schedulable by RM;
- 1 if it is above the bound but at most 1000: the bound does not
  decide;
- 0 if it is above 1000: no algorithm can schedule the set.

Use `util_x1000`.

## Examples

These show the meaning of the functions. They are not all the cases.

| call | result |
|---|---|
| `util_x1000( ( uint32_t[] ){ 1, 1, 2 }, ( uint32_t[] ){ 4, 5, 10 }, 3 )` | 650 |
| `util_x1000( ( uint32_t[] ){ 2, 4 }, ( uint32_t[] ){ 5, 7 }, 2 )` | 971 |
| `ll_test( ( uint32_t[] ){ 2, 4 }, ( uint32_t[] ){ 5, 10 }, 2 )` | 2 |
| `ll_test( ( uint32_t[] ){ 2, 4 }, ( uint32_t[] ){ 5, 7 }, 2 )` | 1 |
| `ll_test( ( uint32_t[] ){ 3, 2 }, ( uint32_t[] ){ 4, 5 }, 2 )` | 0 |
