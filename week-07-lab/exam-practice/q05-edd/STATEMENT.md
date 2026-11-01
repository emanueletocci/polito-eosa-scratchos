# Q5. Earliest due date

`n` jobs all arrive at time 0 on one processor. Job `i` needs `c[i]` units
of processor time and has the absolute deadline `d[i]`. The jobs are
scheduled by **earliest due date** (EDD): one after the other, each run to
its end, in order of non-decreasing deadline; between jobs with the same
deadline, the one with the lower index goes first. The first job starts
at 0.

A job's lateness is its finish time minus its deadline: negative or 0 if
it meets the deadline, positive if it is late.

In this question `n` is between 1 and 8, every `c[i]` is between 1 and
100, and every `d[i]` between 1 and 1000.

## What you write

In `answer.c`, three functions. Keep their signatures.

**(a)** `unsigned edd_pick( const uint32_t *d, const uint8_t *done, unsigned n )`

`done[i]` is 1 for a job that has already run and 0 for one that has
not; at least one has not. Returns the index of the job that EDD runs
next among those that have not run.

**(b)** `uint32_t edd_finish( const uint32_t *c, const uint32_t *d, unsigned n, unsigned i )`

Returns the time at which job `i` finishes in the EDD schedule. Use
`edd_pick`.

**(c)** `int32_t edd_lmax( const uint32_t *c, const uint32_t *d, unsigned n )`

Returns the maximum lateness of the `n` jobs in the EDD schedule: the
largest of their latenesses, which may be negative. Use `edd_finish`.

## Examples

These show the meaning of the functions. They are not all the cases.

| call | result |
|---|---|
| `edd_pick( ( uint32_t[] ){ 3, 10, 7, 8, 5 }, ( uint8_t[] ){ 0, 0, 0, 0, 0 }, 5 )` | 0 |
| `edd_pick( ( uint32_t[] ){ 3, 10, 7, 8, 5 }, ( uint8_t[] ){ 1, 0, 0, 0, 0 }, 5 )` | 4 |
| `edd_finish( ( uint32_t[] ){ 1, 1, 1, 3, 2 }, ( uint32_t[] ){ 3, 10, 7, 8, 5 }, 5, 3 )` | 7 |
| `edd_lmax( ( uint32_t[] ){ 2, 3, 1 }, ( uint32_t[] ){ 2, 3, 4 }, 3 )` | 2 |
