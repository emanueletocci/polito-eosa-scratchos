# week-07/base

## Starting point

This folder is where you start in class. It is a preemptive round-robin
operating system with two tasks, A and B, plus an idle task. The two
tasks now share some memory and run the same program on it. It builds
and runs from the first minute, but it has a problem: the tasks take a
lock before they touch the shared data, and the lock does nothing yet.
So the shared data gets corrupted. You write the lock live, following
the slides. The finished code is in `../solution/`, with full
line-by-line comments.

## How this folder was prepared

It starts from `week-05/solution/`. Week 6 had no code, so this is the
next step after week 5. These changes were made so that class time goes
to the lock, not to plumbing:

- The week 5 demos (`coop`, `idle`) and task B's week 4 demos (`rogue`,
  `badptr`, `overflow`) were removed. The `QUANTUM` setting stays.
- A shared region was added. In `scripts/mps2_m3.ld` the section
  `.shared` sits at `0x20003000`, is 1 KB long, and is `NOLOAD`: nothing
  is stored in FLASH for it, and RAM is zero at start, so every shared
  variable starts at 0 (a start value in the C code would be lost). In
  `kernel/mpu.c`, MPU region 2 gives both tasks read and write access
  to it. Variables go there with the `SHARED` marker in `src/shared.h`;
  they are defined in `src/shared.c`.
- Tasks A and B run the same program, in two phases (`src/task_a.c`,
  `src/task_b.c`):
  1. **counter**: each task adds 1 to `shared_counter`, 1 000 000
     times. The total must be 2 000 000.
  2. **lines**: each task prints 3000 lines of 100 letters (`A` or
     `B`). Every line must come out whole.
  The add in the counter phase is the small function `plus_one`. It has
  a short pause on purpose. QEMU only takes an interrupt between blocks
  of instructions, so without the pause the gap between the load and the
  store of the counter would be too short for a tick to land in it. The
  pause makes the gap long enough to show the race.
- Four `DEMO` builds were added (see below).
- `lib/spinlock.h` declares the lock functions and `src/tasks.h` defines
  the `LOCK` and `UNLOCK` macros. The tasks already call them around
  every update of the counter and every `printf`. In the base the
  functions do nothing, so nothing is protected.

## What is given, and why

You read this code in the Inspect slides. You do not type it.

- **`src/shared.h`, `src/shared.c`** define the shared variables:
  `shared_counter`, the "done" flags of the two tasks, and two locks,
  `counter_lock` and `uart_lock`.
- **`src/tasks.h`** holds the phase sizes (`COUNT_N`, `LINES_N`,
  `LINE_LEN`) and the `LOCK`, `UNLOCK`, `PRINT_BEGIN` and `PRINT_END`
  macros.
- **`src/task_a.c` and `src/task_b.c`** are the tasks, with the two
  phases. The last task to finish prints the total and calls
  `trace_dump()`.
- **`lib/spinlock.h`** is the interface of the lock: the type
  `spinlock_t` and the functions `spin_lock`, `spin_lock_busy` and
  `spin_unlock`.
- **`scripts/mps2_m3.ld`, `kernel/mpu.c`** hold the shared region and
  MPU region 2.
- **`kernel/`, `lib/`, `drivers/`, `boot/`, `src/main.c`, `src/idle.c`**
  and the `Makefile` are the week 5 system: scheduler, trace, system
  calls, `printf`, UART and boot code.

## What you will write in class

Only the lock, in `lib/spinlock.s`. Each function is a label with a
single `bx lr` and a marker comment (`/* Week 7, Unit 2: write ... here
(see the slides). */`). In the order of the slides:

1. `spin_lock_busy` and `spin_unlock`. Then run
   `make DEMO=spin run`: the output is correct, and the trace shows how
   much time a waiting task wastes.
2. `spin_lock` (spin-then-yield: while the lock is taken, give the CPU
   away with `task_yield`). Then run `make run`.

Until you do, `make run` builds and ends with `all tasks finished`, but
the lock does nothing: the counter total is below 2 000 000 and some
lines are mixed.

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

To draw the scheduler trace that the last task prints, save the output,
stop QEMU with `Ctrl-C` once you see `all tasks finished`, and give the
file to the schedule simulator:

```
make run | tee output.txt
python3 ../../../course-material/tools/schedsim/schedsim.py trace output.txt
```

## DEMO

`DEMO` builds one variation of the program:

```
make DEMO=race run
make DEMO=counter run
make DEMO=cpsid run
make DEMO=spin run
```

- `race`: the lines phase only, no lock. The lines of A and B get mixed.
- `counter`: the counter phase only, no lock. Updates get lost: the
  total is below 2 000 000.
- `cpsid`: like `race`, with each `printf` wrapped in `cpsid i` and
  `cpsie i`. It does not help: the tasks are unprivileged, so the write
  to the interrupt mask is ignored and the lines still get mixed.
- `spin`: both phases, with the lock taken by `spin_lock_busy` (plain
  busy waiting). It needs your lock to be written.

Without `DEMO`, both phases run and the lock is `spin_lock`.
`QUANTUM` is the length of a time slice in ticks (one tick is 10 ms;
the default is 5): `make run QUANTUM=2`.

Each variation builds into its own folder, `build/demo-<word>/` or
`build/quantum-<n>/`, so builds never mix. Use that image with gdb and
`nm`, for example `build/demo-race/scratch-os.axf`.

The full, finished solution is in the sibling folder, `../solution/`.
