# week-05/base

## Starting point

This folder is where you start in class. It is the finished operating
system from week 4, extended with a complete preemptive round-robin
scheduler: tasks A and B take turns on the CPU, and an idle task runs
when neither can. It builds and runs from the first minute. The one
thing it cannot do is show you who used the CPU: the scheduler trace is
empty. You write the trace live, following the slides. The finished code
is in `../solution/`, with full line-by-line comments.

## How this folder was prepared

It starts from `week-04/solution/`. These changes were made so that
class time goes to observing and tracing the scheduler, not to plumbing:

- Each task has a task control block, `struct tcb`, in the table
  `tasks[]`, with a state (READY, RUNNING, BLOCKED, FINISHED).
- A new idle task, `src/idle.c`, has its own partition in
  `scripts/mps2_m3.ld`.
- `task_exit_current` was replaced by the scheduler functions in
  `kernel/task.c`; the context switch is the new `PendSV_Handler` in
  `kernel/pendsv.s`.
- `SysTick_Handler` calls `task_increment_tick`, and PendSV and SysTick
  get the least urgent priority in `boot/startup.s`.
- New system calls: yield, delay, delay_until, tick_count and
  trace_dump (`lib/syscalls.h`, `lib/syscalls.s`, `kernel/syscall.c`).
- `spin_ticks` (busy work for a number of ticks) was added to
  `lib/printf.c`.
- The `Makefile` gained `QUANTUM` and the `coop` and `idle` demos.
- `kernel/trace.c` is an empty skeleton, and the call to the trace in
  `task_switch_context` was removed. Everything else about the trace is
  given.

## What is given, and why

You read this code in the Inspect slides. You do not type it.

- **`kernel/task.h` and `kernel/task.c`** hold `struct tcb`, the table
  `tasks[]`, `tcb_current`, `tick_count`, and the functions
  `task_init_stack` (builds a task's first stack), `task_start_first`,
  `task_increment_tick` (count the tick, wake tasks, end the time slice),
  `task_request_switch` (make PendSV pending), `task_block_until`,
  `task_finish_current` and `task_switch_context` (round robin).
- **`kernel/pendsv.s`** has `PendSV_Handler`: it saves r4-r11, stores the
  stack pointer in the TCB, calls `task_switch_context`, and restores the
  next task.
- **`kernel/syscall.c`, `lib/syscalls.s`, `lib/syscalls.h`** hold the
  system calls, including `SYS_TRACE_DUMP`: `sys_trace_dump_impl` calls
  `trace_print`, and the user stub `trace_dump` runs `svc #9`.
- **`src/task_a.c` and `src/task_b.c`** are the tasks. Task B ends by
  calling `trace_dump()`. They also hold the `DEMO` blocks.
- **`src/idle.c`** has `idle_task`, which sleeps with `wfi`.
- **`kernel/trace.h`** is the interface you implement:
  `trace_record_switch` and `trace_print`.
- **`Makefile`**, the drivers and the rest of `kernel/` and `lib/` are as
  in week 4, apart from the changes above.

## What you will write in class

Only the trace. In `kernel/trace.c`, in the order of the slides, at each
marker comment:

1. `/* CLASS: trace record type goes here */`: `struct trace_record`.
2. `/* CLASS: ring buffer goes here */`: `TRACE_SIZE`, `buffer` and
   `count`.
3. `/* CLASS: trace_record_switch body goes here */`: the body of
   `trace_record_switch`.
4. `/* CLASS: Gantt helpers go here */`: `stored`, `oldest_plus`,
   `owner_at`, `GANTT_COLUMNS` and `draw_gantt`.
5. `/* CLASS: trace_print body goes here */`: the body of `trace_print`.

Then, in `kernel/task.c`, at `/* CLASS: trace hook goes here
(task_switch_context) */`, the call to `trace_record_switch` that records
a switch whenever the task really changes.

Until you do, `make run` shows A and B interleaving and ends with
`all tasks finished`, but `trace_dump()` prints nothing.

## Build, run, debug

Build the image:

```
make
```

Run it under QEMU (quit with `Ctrl-C`):

```
make run
```

Debug it. Start QEMU paused, waiting for a debugger:

```
make debug
```

Then, from a second terminal, attach GDB:

```
arm-none-eabi-gdb build/scratch-os.axf
```

(on Linux the package is usually called `gdb-multiarch`), and inside
GDB:

```
target remote :1234
```

Log every access to a device QEMU does not implement with
`make run-trace`. Remove the generated files with `make clean`.

## QUANTUM and DEMO

`QUANTUM` is the length of a time slice in ticks (one tick is 10 ms; the
default is 5):

```
make run QUANTUM=2
```

`DEMO` builds one deliberate variation:

```
make run DEMO=coop
make run DEMO=idle
make run DEMO=rogue
make run DEMO=badptr
make run DEMO=overflow
```

- `coop`: no preemption (`QUANTUM` is forced to 0) and task A never
  yields, so task B starves.
- `idle`: both tasks sleep with `task_delay` and `task_delay_until`, so
  the idle task runs.
- `rogue`, `badptr`, `overflow`: task B misbehaves, and the MPU stops it
  (the week 4 demos).

Each variation builds into its own folder, `build/demo-<word>/` or
`build/quantum-<n>/`, so builds never mix. Use that image with gdb and
`nm`, for example `build/demo-coop/scratch-os.axf`.

The full, finished solution is in the sibling folder, `../solution/`.
