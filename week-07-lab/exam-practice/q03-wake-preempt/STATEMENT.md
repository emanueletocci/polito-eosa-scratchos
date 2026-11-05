# Q3. A higher priority that cannot wait

The kernel of this question is preemptive with fixed priorities: tasks A
and B, and the idle task when neither can run. When the kernel switches,
it chooses the READY application task with the highest priority, and
takes turns among tasks of equal priority. It switches only when the time
slice of the running task ends (5 ticks) or when the running task yields,
blocks or exits. So a task of higher priority that becomes READY while
another task runs waits until the end of that task's slice.

This folder holds three files of the kernel.

- `kernel/task.h` is given. Every task control block has a `priority`,
  and the file defines `A_PRIO` and `B_PRIO`, the priorities of tasks A
  and B (1 and 2). A higher number is a higher priority.
- `src/task_b.c` is given. Task B is periodic: it blocks until its next
  release, every 7 ticks from its first run, does one tick of work, and
  does so five times. Task A is busy: it never blocks.
- `kernel/task.c` is your answer.

## What you write

In `kernel/task.c`, after your change:

**(a)** When a tick makes a task READY and that task has a strictly
higher priority than the application task that is running, the kernel
switches to it at that tick, without waiting for the end of the slice.

**(b)** When a tick makes a task READY whose priority is equal to or
lower than that of the running task, nothing changes: the woken task
waits, as it does now.

Nothing else changes: which task is chosen at a switch, the time slice,
and what happens when the idle task runs.

## Example

With `A_PRIO` 1 and `B_PRIO` 2, the first lines of the trace are

```text
T 5 A B
T 5 B A
T 12 A B
T 13 B A
T 19 A B
```

B becomes READY at ticks 12 and 19 and runs at once. With the given
`kernel/task.c`, the third line is `T 15 A B`: B waits for the end of A's
slice.
