# Week 8 laboratory, Part B — worked solutions

Solutions to the exercises of Part B of [`README.md`](README.md); Part A
has its own, in
[`freertos-labs/week-08-lab/SOLUTIONS.md`](../../freertos-labs/week-08-lab/SOLUTIONS.md).
You are expected to attempt an exercise before consulting its solution,
and to read the hints in [`HINTS.md`](HINTS.md) before this file: the
values an exercise asks you to measure are the substance of the work, and
reading a measured value does not substitute for taking one.

Finished code: [`solution/`](solution/). Every value below was produced
by running the stated command with the course toolchain (Arm GNU
Toolchain 15.3, QEMU 11.1), in exact time (`make run-exact`). In exact
time the tick numbers are the same at every run; they depend on the code
the compiler generated, so a different compiler version may shift them,
and the relations the exercises ask about hold regardless.

---

### B1. Atomic operations

**Step 1.** With the empty functions, `make run-exact DEMO=atomictest`
ends with:

```text
atomic test: 0 of 8 checks passed
```

Each empty function returns 0 and writes nothing, so every result and
every value of the word is wrong.

**Steps 2 and 3.** `lib/atomic.s`:

```asm
atomic_exchange:
    ldrex r2, [r0]              /* old = *p, and start watching *p */
    strex r3, r1, [r0]          /* *p = value, if nothing came in between; r3 = 0 stored, 1 not */
    cmp r3, #0                  /* did the write happen? */
    bne atomic_exchange         /* if not, start again */
    mov r0, r2                  /* return old */
    bx lr

atomic_fetch_add:
    ldrex r2, [r0]              /* old = *p, and start watching *p */
    add r3, r2, r1              /* r3 = old + n */
    strex r12, r3, [r0]         /* *p = old + n, if nothing came in between */
    cmp r12, #0                 /* did the write happen? */
    bne atomic_fetch_add        /* if not, start again */
    mov r0, r2                  /* return old */
    bx lr

atomic_cas:
    ldrex r3, [r0]              /* old = *p, and start watching *p */
    cmp r3, r1                  /* old == expected? */
    bne cas_differs             /* no: give up, write nothing */
    strex r12, r2, [r0]         /* *p = desired, if nothing came in between */
    cmp r12, #0                 /* did the write happen? */
    bne atomic_cas              /* if not, start again: old may have changed */
    mov r0, r3                  /* return old (== expected: swapped) */
    bx lr
cas_differs:
    clrex                       /* stop watching *p: no strex will follow this ldrex */
    mov r0, r3                  /* return old (!= expected: not swapped) */
    bx lr
```

`r0` to `r3` and `r12` are the registers a C caller does not expect a
function to preserve, so the three functions save nothing. Each needs one
register for the old value and one for the answer of `strex`, distinct
from the address and from the value stored; `atomic_fetch_add` and
`atomic_cas` need `r12` for that answer because `r0` to `r3` are taken.

**Step 4.**

```text
atomic test: 8 of 8 checks passed
```

A failed `strex` means that something came in between, and the monitor is
now Open: a second `strex` without a new `ldrex` would fail as well, and
the old value read before may no longer be the value in the word. Only a
new `ldrex` both reads the current value and starts a new watch. For the
same reason `atomic_cas` compares again after a failed `strex`: the word
may have been changed by whatever came in between, and storing `desired`
over a value that is no longer `expected` would be the lost update the
operation exists to prevent.

---

### B2. A counter with no lock

**Steps 1 and 2.** `src/counter.c`:

```c
void counter_add_fetchadd( volatile uint32_t *p )
{
    atomic_fetch_add( p, 1 );                   /* read, add 1, write: one step */
}

uint32_t counter_add_cas( volatile uint32_t *p )
{
    uint32_t failed = 0;
    uint32_t old = *p;                          /* the value the update starts from */
    while ( atomic_cas( p, old, plus_one( old ) ) != old )  /* changed meanwhile: not written */
    {
        failed++;
        old = *p;                               /* start again from the new value */
    }
    return failed;
}
```

The new value must be computed from the value the swap expects, so
`old` is read once and passed both to `plus_one` and, as `expected`, to
`atomic_cas`.

**Step 3.** `make run-exact DEMO=fetchadd`, `make run-exact DEMO=cas` and
`make run-exact`:

```text
A: counter done at tick 83
B: counter done at tick 157
counter=2000000 expected=2000000 (B last)

A: counter done at tick 393, compare-and-swap failed 26 times
B: counter done at tick 467, compare-and-swap failed 25 times
counter=2000000 expected=2000000 (B last)

A: counter done at tick 391, overtaken at most 228601 times
B: counter done at tick 446, overtaken at most 228601 times
counter=2000000 expected=2000000 (B last)
```

| update | counter | A counting done | B counting done | failed swaps (A / B) |
|---|---|---|---|---|
| `atomic_fetch_add` (`DEMO=fetchadd`) | 2000000 | 83 | 157 | — |
| compare-and-swap (`DEMO=cas`) | 2000000 | 393 | 467 | 26 / 25 |
| `spin_lock` (`make run-exact`) | 2000000 | 391 | 446 | — |

**Step 4.** All three are correct. `DEMO=fetchadd` is not a fair
comparison: it does not call `plus_one`, so its additions do less work,
and most of the difference from the other two is that work. The
compare-and-swap version keeps `plus_one` between the read and the swap,
as the lock version keeps it inside the lock, so the two do the same work
per addition; they finish within a few percent of each other.

On one processor the counter can change between this task's read and its
swap only if the other task runs in between, that is, if this task is
switched out inside that window. Each failure is one such switch. A tick
that comes inside the window but does not end the time slice changes
nothing in the counter, so the swap succeeds. A tick that comes between
the `ldrex` and the `strex` inside `atomic_cas` makes that `strex` fail,
but the function retries it itself, and the caller does not count it.

---

### B3. The spinlock, in C

**Step 1.** `lib/locks.c`:

```c
void tas_lock( spinlock_t *lock )
{
    while ( atomic_exchange( lock, 1 ) != 0 )   /* it was taken: not ours */
    {
        task_yield();                           /* let the holder run */
    }
    dmb();                                      /* the protected accesses start only now */
}

void tas_unlock( spinlock_t *lock )
{
    dmb();                                      /* the protected writes are done first */
    *lock = 0;                                  /* free: only the holder writes here */
}
```

`atomic_exchange( lock, 1 )` is test-and-set: it returns 0 exactly when
the lock was free and this call took it. Writing 1 over a lock that was
already 1 changes nothing. Giving the lock back needs no atomic
operation: only the holder writes the word then.

**Steps 2 and 3.** `make run-exact DEMO=tas`:

```text
B: counter done at tick 256, overtaken at most 63305 times
A: counter done at tick 544, overtaken at most 1000000 times
counter=2000000 expected=2000000 (A last)
```

The counter is right, and every line of the output is a whole line. With
a time slice of 4 ticks:

```text
make run-exact QUANTUM=4
B: counter done at tick 360, overtaken at most 182875 times
A: counter done at tick 464, overtaken at most 182875 times

make run-exact DEMO=tas QUANTUM=4
B: counter done at tick 456, overtaken at most 118210 times
A: counter done at tick 544, overtaken at most 118210 times
```

| largest overtaking (A / B) | quantum 5 | quantum 4 |
|---|---|---|
| `spin_lock` | 228601 / 228601 | 182875 / 182875 |
| `tas_lock` | 1000000 / 63305 | 118210 / 118210 |

**Step 4.** Task A's 1000000 under `DEMO=tas` is every addition of task
B: A asked for the lock once, and entered only after B had finished its
whole counter phase. A was not blocked; at each of its turns it found the
lock taken and yielded, and B, which held the lock when the tick came,
received a whole new time slice. Neither lock remembers who asked first,
so which task the ticks catch inside the critical section decides the
waiting, and that depends only on where the ticks fall: with a slice of 4
ticks the same `tas_lock` lets A wait 118210 takes, fewer than
`spin_lock`. The two locks follow the same rule, and neither bounds
waiting: the four runs give four different maxima, one of them the
starvation of a task for a whole phase.

---

### B4. A lock that serves in turn

**Step 1.** `lib/locks.c`:

```c
void ticket_lock( ticket_t *t )
{
    uint32_t my = atomic_fetch_add( &t->next, 1 );  /* take a ticket */
    while ( t->serving != my )                  /* not my turn yet */
    {
        task_yield();                           /* let the holder run */
    }
    dmb();                                      /* the protected accesses start only now */
}

void ticket_unlock( ticket_t *t )
{
    dmb();                                      /* the protected writes are done first */
    t->serving = t->serving + 1;                /* serve the next ticket: only the holder writes here */
}
```

**Step 2.** `make run-exact DEMO=ticket`:

```text
A: counter done at tick 1278, overtaken at most 1 times
B: counter done at tick 1354, overtaken at most 1 times
counter=2000000 expected=2000000 (B last)
```

| | counter | overtaken at most (A / B) | A counting done | B counting done |
|---|---|---|---|---|
| `spin_lock` (`make run-exact`) | 2000000 | 228601 / 228601 | 391 | 446 |
| ticket lock (`DEMO=ticket`) | 2000000 | 1 / 1 | 1278 | 1354 |

**Step 3.** `atomic_fetch_add` hands each request a different ticket, in
the order the requests were made, and the lock admits tickets in that
order. A task that asks while the other waits receives a later ticket, so
it cannot enter first: with two tasks, the other task can take the lock
at most once while this one waits. `serving` is written only by the task
that holds the lock, so its update is never concurrent with another
write; the waiting task only reads it, and a stale value costs it one more
turn of the wait.

**Step 4.** The counting takes about three times as long. Once both tasks
hold a ticket, each unlock serves the other task's ticket, and the task
that gave the lock back takes a new ticket behind it. On one processor
the other task is not running: the task that just gave the lock back can
only yield to it. The lock then changes hands at every addition, and
every change costs a switch, where the spinlock let the holder add for a
whole time slice.

With the empty functions of the starter the same build prints
`counter=1341682 expected=2000000 (B last)`: without a lock, updates are
lost.
