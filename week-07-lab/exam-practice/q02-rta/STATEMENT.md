# Q2. Response-time analysis

A set of `n` periodic tasks runs on one processor with fixed priorities,
and the deadline of every task is its period. Task `i` has an execution
time `c[i]` and a period `t[i]`. The arrays are sorted by priority:
task 0 has the highest priority, task `n - 1` the lowest. In this question
`n` is between 1 and 10, and every `c[i]` and `t[i]` is between 1 and 1000.

The response time `R` of the first job of task `i` (all tasks released
together at 0) is found by iteration:

- start with `R = c[i]`;
- the next value is `c[i]` plus the interference of the higher-priority
  tasks in a window of length `R`: each task `j` with `j < i` releases
  `⌈R / t[j]⌉` jobs in it (the division rounded **up**), each of `c[j]`
  units;
- stop when the next value equals `R`: that is the response time;
- stop also when the next value is above `t[i]`: the deadline is
  missed, and that first value above `t[i]` is the response time.

## What you write

In `answer.c`, three functions. Keep their signatures.

**(a)** `uint32_t rta_interference( const uint32_t *c, const uint32_t *t, unsigned i, uint32_t r )`

Returns the interference on task `i` in a window of length `r` (`r` at
least 1): the sum, over the tasks `j < i`, of `⌈r / t[j]⌉ × c[j]`. For
task 0 it is 0.

**(b)** `uint32_t rta_response( const uint32_t *c, const uint32_t *t, unsigned i )`

Returns the response time of task `i`, by the iteration above. Use
`rta_interference`.

**(c)** `int rta_schedulable( const uint32_t *c, const uint32_t *t, unsigned n )`

Returns 1 if every one of the `n` tasks has a response time at most its
period, and 0 otherwise. Use `rta_response`.

## Examples

These show the meaning of the functions. They are not all the cases.

| call | result |
|---|---|
| `rta_interference( ( uint32_t[] ){ 1, 2, 3 }, ( uint32_t[] ){ 4, 6, 12 }, 2, 7 )` | 6 |
| `rta_response( ( uint32_t[] ){ 1, 2, 3 }, ( uint32_t[] ){ 4, 6, 12 }, 2 )` | 10 |
| `rta_response( ( uint32_t[] ){ 2, 4 }, ( uint32_t[] ){ 5, 7 }, 1 )` | 8 |
| `rta_schedulable( ( uint32_t[] ){ 1, 2, 3 }, ( uint32_t[] ){ 4, 6, 12 }, 3 )` | 1 |
| `rta_schedulable( ( uint32_t[] ){ 2, 4 }, ( uint32_t[] ){ 5, 7 }, 2 )` | 0 |
