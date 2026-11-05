# Week 8 laboratory — two kernels, and the limits of a spinlock

Laboratory 8 covers week 7's topic, synchronization and the first
FreeRTOS tasks, and extends the work done in class on both kernels. Its
goal is to measure what the week 7 spinlock does and does not guarantee:
on FreeRTOS, where the same lock meets task priorities, and on the
scratch OS, where it is compared with an update that needs no lock and
with a lock that serves the tasks in turn. Part A, in the laboratory,
is FreeRTOS work, in
[`freertos-labs/week-08-lab/`](../../freertos-labs/week-08-lab/README.md).
Part B, independent practice, is the last work on the scratch OS, in
this folder. Hints for Part B are in [`HINTS.md`](HINTS.md), worked
solutions in [`SOLUTIONS.md`](SOLUTIONS.md), and the finished code in
[`solution/`](solution/).

## Objectives

On completion you will have:

- run the program of the scratch OS, with its spinlock unchanged, on
  FreeRTOS, and compared the two kernels on it by measurement;
- made a task of higher priority wait for ever on a spinlock held by a
  task of lower priority, and explained it from the state read in the
  debugger;
- written periodic FreeRTOS tasks under Rate Monotonic priorities and
  shown that their measured response times agree with response-time
  analysis and with `schedsim`;
- (Part B) written atomic exchange, fetch-and-add and compare-and-swap
  in assembly with the exclusive pair;
- (Part B) updated a shared counter correctly with no lock, in two ways;
- (Part B) rewritten the spinlock in C on your atomic exchange, measured
  that neither version bounds waiting, and built on fetch-and-add a
  ticket lock that does, at a cost you have measured.

## Part A — in the laboratory

Part A is in
[`freertos-labs/week-08-lab/README.md`](../../freertos-labs/week-08-lab/README.md),
with its own starting point, hints and solutions: the program of this
kernel run on FreeRTOS, a task priority that turns the spinlock into an
endless wait, and three periodic tasks under Rate Monotonic.

---

## Part B — independent practice

### Starting point

[`starter/`](starter/), a prepared copy of
[`../week-07/solution/`](../week-07/solution/): tasks A and B, the
shared counter and the lines, the spinlock of `lib/spinlock.s`. You
write a small library of atomic operations and three things built on
it. These files are new; the functions you write are present, with
empty bodies and a comment that says what each must do:

- `lib/atomic.h` and `lib/atomic.s`: `atomic_exchange`,
  `atomic_fetch_add` and `atomic_cas` (yours, in assembly), and `dmb`, a
  barrier callable from C (given).
- `src/counter.h` and `src/counter.c`: `plus_one`, moved here from the
  task files (given), and `counter_add_fetchadd` and `counter_add_cas`
  (yours).
- `lib/locks.h` and `lib/locks.c`: `tas_lock` and `tas_unlock`, and the
  ticket lock `ticket_t`, `ticket_lock` and `ticket_unlock` (yours).
- `src/selftest.c`: a check of the three atomic operations (given).

Five new demonstrations select what runs (see the `Makefile`):
`DEMO=atomictest` runs the check; `DEMO=fetchadd` and `DEMO=cas` update
the counter with no lock, through the two functions of `src/counter.c`;
`DEMO=tas` replaces `spin_lock` with `tas_lock` on both locks; and
`DEMO=ticket` protects the counter with a ticket lock. At the end of its
counter phase each task prints the tick and, in the lock builds, the
largest number of times the other task took the counter lock while this
one waited for it. That count uses `lock_takes`, a count of the takes in
the shared block. The `Makefile` has a target `run-exact`, which runs in
exact time, as in Part A.

Work in `starter/`. Every path below is relative to it.

### B1. Atomic operations

`lecture-notes/week-07.md`, "Hardware help: atomic read-modify-write",
observes that between `ldrex` and `strex` you can compute anything, so
the pair gives test-and-set, an atomic increment, and more. This
exercise writes three of them.

1. Read `lib/atomic.h`. Run `make run-exact DEMO=atomictest` and record
   how many of the checks pass with the empty functions.
2. Write `atomic_exchange` and `atomic_fetch_add` in `lib/atomic.s`, each
   an `ldrex` / `strex` loop that returns the old value. Use only
   registers the caller does not expect you to preserve.
3. Write `atomic_cas`. When the old value differs from `expected`, it
   writes nothing, and it must leave no word watched: end that path
   with `clrex`.
4. Run the check again until every check passes. Then answer: why must
   a failed `strex` send the loop back to the `ldrex` and not just retry
   the `strex`; and why does `atomic_cas`, after a failed `strex`, compare
   again instead of storing at once?

**Checkpoint:** every check of `DEMO=atomictest` passes, and both
questions are answered.

### B2. A counter with no lock

1. Write `counter_add_fetchadd` in `src/counter.c`: one call. Run
   `make run-exact DEMO=fetchadd` and record the final counter and the
   tick at which each task finishes counting.
2. Write `counter_add_cas`: read the counter, compute the new value with
   `plus_one`, and swap it in with `atomic_cas`; if the swap did not
   happen, start again from the value now in the counter. Return how
   many attempts failed. Run `make run-exact DEMO=cas` and record the
   same values and the failure counts.
3. Run `make run-exact`, with the spinlock, and fill one table with the
   three runs.
4. Explain the table. Why is `DEMO=fetchadd` not a fair comparison with
   the lock, and `DEMO=cas` is? What, on one processor, makes a
   compare-and-swap fail, and why is a tick that comes between the read
   and the swap not enough?

**Checkpoint:** two correct counters with no lock, one table of three
runs, and the failures explained.

### B3. The spinlock, in C

1. Write `tas_lock` and `tas_unlock` in `lib/locks.c`: the test-and-set
   lock of the week 7 notes over `atomic_exchange`, yielding while the
   lock is taken, with `dmb` where `spin_lock` and `spin_unlock` put
   their barriers.
2. Run `make run-exact DEMO=tas`. Check that the counter is right and
   that no line is mixed.
3. Compare the largest overtaking of each task with that of
   `make run-exact`. Then run both again with a time slice of 4 ticks
   (`make run-exact QUANTUM=4` and `make run-exact DEMO=tas QUANTUM=4`)
   and compare again.
4. Explain what the value of task A in `DEMO=tas` means, and what the
   four runs together say about the waiting either lock allows.

**Checkpoint:** a correct lock in C, and its waiting compared with
`spin_lock` under two time slices.

### B4. A lock that serves in turn

1. Write `ticket_lock` and `ticket_unlock` in `lib/locks.c` as
   `lib/locks.h` describes: take a ticket with `atomic_fetch_add` on
   `next`, wait, yielding, until `serving` is your ticket, and give the
   lock back by serving the next ticket, with the barriers of B3.
2. Run `make run-exact DEMO=ticket`. Record the final counter, the
   largest overtaking of each task, and the tick at which each task
   finishes counting, and compare them with `make run-exact`.
3. Explain why the waiting is now bounded, and why giving `serving` its
   new value needs no atomic operation.
4. State what the bound costs, and why it costs that on one processor.

**Checkpoint:** waiting bounded by one take, measured; the price of the
bound, measured and explained.

### Done looks like — Part B

- B1: three atomic operations in assembly that pass every check.
- B2: two counters with no lock, compared with the lock in one table.
- B3: the spinlock in C, correct, and its waiting compared with
  `spin_lock` under two time slices.
- B4: a ticket lock whose waiting is bounded, and its cost explained.

---

## Where this goes next

Week 8 treats synchronization in FreeRTOS. This laboratory has prepared
two things it uses.

- **Waiting that the kernel manages.** In Part A the waiting task stayed
  ready and kept the processor; in B4 every hand-over of the lock cost a
  yield. Both are the price of a wait that only the waiting task knows
  about. A kernel that knows a task is waiting can stop choosing it, and
  choose it again when the lock is given back.
- **A measured schedule.** In Part A you checked a FreeRTOS schedule tick
  by tick against analysis; the same method checks any program whose
  tasks wait for each other.
