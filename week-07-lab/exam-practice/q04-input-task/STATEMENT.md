# Q4. A task that waits for input

The kernel of this question is preemptive with fixed priorities: at a
switch it chooses the READY application task with the highest priority.
Task A is busy and never blocks; task B, of higher priority, reads the
console byte by byte with `sys_read` and prints what it reads, until it
reads `q`. When `sys_read` returns -1, B calls it again.

The console input is interrupt driven. The handler of the UART0 receive
interrupt puts every byte that arrives in a ring of 16 bytes, and
`sys_read`, inside SVCall, takes the oldest byte from the ring. A task
that finds the ring empty waits in the state `TASK_WAITING`, in which the
scheduler does not choose it and the tick does not wake it.

This folder holds the kernel files of this question. All are given;
`kernel/input.c`, with the handler, `sys_read_impl` and `input_init`, is
given with four gaps, marked `gap (a)` to `gap (d)`. Your answer is that
file with the gaps filled. Write only between the two lines that mark a
gap, and leave those lines as they are.

## What you write

Fill each gap so that, after your change:

**(a)** After the handler has run, the receive interrupt does not run
again until another byte has arrived, and every byte that arrives is
returned by `sys_read` once, in the order of arrival.

**(b)** When no byte is waiting, the task that calls `sys_read` gives up
the processor at once, and the other tasks run as if it were not there,
until a byte arrives.

**(c)** When a byte arrives and a task is waiting for input, the task
becomes READY; if it has a higher priority than the task that is
running, it runs at once, at the tick at which the byte arrived, and not
at the end of the running task's time slice. This holds also when the
idle task is running.

**(d)** When 15 bytes are waiting to be read, a byte that arrives is
dropped, and the 15 are kept.

Nothing else in the file changes.

## Example

A's work takes 20 ticks when A is alone. After your change, the run
prints `task A exited (tick 20)` even though B, of higher priority, has
been waiting for a key since the start. A key pressed while A runs is
printed by B with the tick at which it arrived, and the trace has a
switch from A to B at that same tick.
