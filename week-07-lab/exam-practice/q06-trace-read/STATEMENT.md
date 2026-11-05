# Q6. Reading a trace

A kernel records every task switch: from the tick `tick` on, the task
`to` runs instead of the task `from`. The records of one run are in an
array, oldest first, as this structure, already defined in `answer.c`:

```c
struct sw { uint32_t tick; char from; char to; };
```

A task is named by one capital letter, and `'.'` is the idle task. The
processor time is cut into **slots**: slot `s` is the time from tick `s`
to tick `s + 1`. The task that runs in slot `s` is the `to` of the last
record whose tick is at most `s`; before the first record, `from` of the
first record runs. Two records may have the same tick: the later one
decides. The run is observed from tick 0 up to the tick `end`, excluded.

The tasks are periodic, all released together at 0, and the deadline of
every task is its period:

```c
struct task { char name; uint32_t c; uint32_t t; };
```

Job `k` of a task (`k` = 1, 2, ...) is released at `(k - 1) × t` and has
the deadline `k × t`. A task runs its jobs one after the other: job `k`
**finishes** at the end of the slot in which the task has run, counted
from tick 0, `k × c` slots in all. Its **response time** is its finish
minus its release.

In this question there are 1 to 64 records, with ticks below 10000 in
non-decreasing order, `end` is at most 10000 and after the tick of the
last record, and there are 1 to 4 tasks.

## What you write

In `answer.c`, four functions. Keep their signatures.

**(a)** `uint32_t run_ticks( const struct sw *rec, unsigned n, char name, uint32_t from, uint32_t to )`

Returns the number of slots `s` with `from <= s < to` in which the task
`name` runs, for the `n` records `rec`.

**(b)** `uint32_t job_response( const struct sw *rec, unsigned n, uint32_t end, const struct task *tk, unsigned k )`

Returns the response time of job `k` of the task `*tk`, or 0 if that job
has not finished by `end`. Use `run_ticks`.

**(c)** `uint32_t first_miss( const struct sw *rec, unsigned n, uint32_t end, const struct task *tasks, unsigned m, char *who )`

Over the `m` tasks, finds the earliest deadline, at most `end`, that a
job misses: the job has not finished by that deadline. Returns that
deadline and writes the name of the task to `*who`. If two tasks miss at
the same tick, the one earlier in `tasks` is named. If no job misses a
deadline that is at most `end`, returns 0 and leaves `*who` unchanged.
Use `job_response`.

**(d)** `int obeys_rm( const struct sw *rec, unsigned n, uint32_t end, const struct task *tasks, unsigned m )`

A task is **pending** in slot `s` if it has a job released at or before
tick `s` that has not finished by tick `s`. Returns 1 if, in every slot
from 0 to `end - 1`, the task that runs is a pending task with the
shortest period among the pending tasks (any one of them, if several
have that period), and the idle task runs only when no task is pending.
Otherwise returns 0.

## Examples

These show the meaning of the functions. They are not all the cases. They
use the first ten ticks of one run, with the tasks A (`c` 2, `t` 5) and B
(`c` 4, `t` 7): A runs in slots 0-1 and 5-6, B in 2-4 and 7-9.

| call | result |
|---|---|
| `run_ticks( ( struct sw[] ){ { 2, 'A', 'B' }, { 5, 'B', 'A' }, { 7, 'A', 'B' } }, 3, 'B', 0, 10 )` | 6 |
| `run_ticks( ( struct sw[] ){ { 2, 'A', 'B' }, { 5, 'B', 'A' }, { 7, 'A', 'B' } }, 3, 'A', 3, 6 )` | 1 |
| `job_response( ( struct sw[] ){ { 2, 'A', 'B' }, { 5, 'B', 'A' }, { 7, 'A', 'B' } }, 3, 10, &( struct task[] ){ { 'A', 2, 5 }, { 'B', 4, 7 } }[1], 1 )` | 8 |
| `first_miss( ( struct sw[] ){ { 2, 'A', 'B' }, { 5, 'B', 'A' }, { 7, 'A', 'B' } }, 3, 10, ( struct task[] ){ { 'A', 2, 5 }, { 'B', 4, 7 } }, 2, &( char ){ 0 } )` | 7 |
| `obeys_rm( ( struct sw[] ){ { 2, 'A', 'B' }, { 5, 'B', 'A' }, { 7, 'A', 'B' } }, 3, 10, ( struct task[] ){ { 'A', 2, 5 }, { 'B', 4, 7 } }, 2 )` | 1 |
