# Q7. A deadline monitor

The kernel of this question is preemptive with fixed priorities: at a
switch it chooses the READY application task with the highest priority,
and a task that wakes with a higher priority than the running one takes
the CPU at that tick. Tasks A and B are periodic, released together at
tick 0, and the deadline of each job is the next release of its task:

| task | C (ticks) | T (ticks) | priority |
|---|---|---|---|
| A | 2 | 5 | 2 |
| B | 4 | 7 | 1 |

The priorities follow Rate Monotonic. Each job does `C` ticks of work and
then calls `task_delay_until( &last, T )`, so a task that has finished its
job is BLOCKED until its next release. A does 7 jobs, B does 5, and B
prints the trace when it is done.

This folder holds the kernel files of this question.

- `kernel/task.h` is given. Every task control block has a `period` (0
  for a task that is not periodic), set from `A_PERIOD` and `B_PERIOD`,
  and a field `next_release` that the kernel does not use yet.
- `kernel/trace.c` is given. `trace_record_miss( task, tick )` records
  that the task with index `task` in the task table missed a deadline at
  `tick`; the trace dump prints each record as a line `M <tick> <name>`,
  after the `T` lines and before `TRACE END`.
- `src/task_a.c` and `src/task_b.c` are given: the two periodic tasks.
- `kernel/task.c` is your answer.

## What you write

In `kernel/task.c`, the kernel detects deadline misses. After your
change:

**(a)** The kernel knows the next release of every periodic task: the
first is one period after tick 0, and each one is followed by the next.

**(b)** At the tick of a release of a periodic task, if the task's
previous job has not finished, the kernel records a miss with
`trace_record_miss`, at that tick. A task that has exited has no more
jobs and no more misses.

**(c)** Nothing else changes: the tasks are scheduled exactly as before,
and the `T` lines of the dump are the same as without your change.

**(d)** A set of tasks that meets every deadline has no `M` line.

## Example

With the given periods, B misses its deadline at 7 and at every release
after it. The dump ends with

```text
M 7 B
M 14 B
M 21 B
M 28 B
TRACE END 35
```

as an analysis of the set by Rate Monotonic predicts: B's first job,
released at 0, ends at 8, after its deadline 7.

A job of the given tasks ends a few instructions after the tick that ends
its last tick of work, when the task calls `task_delay_until`. So a job
whose response time is exactly its period ends just after its deadline,
and this kernel records it as a miss. Keep it in mind if you try other
periods.
