# Week 8 laboratory, Part B — hints

Hints for the exercises of Part B of [`README.md`](README.md), to be read
when an exercise does not yield to your own attempt and before its worked
solution in [`SOLUTIONS.md`](SOLUTIONS.md). Each exercise has three
levels. Open one at a time, and stop at the first that lets you
continue: every level is complete in itself, and none contains the
finished code.

1. **The concept** — what to think about, without naming an instruction.
2. **The rule** — the instruction, register or rule involved.
3. **The shape** — the order of the steps, as an outline to fill in.

---

## Part B

### B1. Atomic operations

<details><summary>Level 1 — the concept</summary>

`spin_lock` already contains one atomic operation: it reads the lock word,
decides, and writes it back, starting again if anything came in between.
Each of the three operations has the same frame; only the computation
between the read and the write differs, and compare-and-swap may decide
not to write at all.

</details>

<details><summary>Level 2 — the rule</summary>

`ldrex` reads and starts the watch; `strex` writes only if the watch
still holds, and puts 0 (written) or 1 (not written) in a register of its
own. The arguments arrive in `r0`, `r1`, `r2` and the result leaves in
`r0`; `r0` to `r3` and `r12` may be used without saving. `clrex` ends a
watch with no store. A failed `strex` leaves the monitor Open.

</details>

<details><summary>Level 3 — the shape</summary>

Exchange: exclusive load of the old value into a free register; exclusive
store of the new value; if the store failed, back to the load; move the
old value to `r0`; return. Fetch-and-add: the same, with an add between
the load and the store. Compare-and-swap: exclusive load; compare with
`expected`; if different, `clrex`, old value to `r0`, return; otherwise
exclusive store of `desired`, back to the load if it failed, old value
to `r0`, return.

</details>

### B2. A counter with no lock

<details><summary>Level 1 — the concept</summary>

Compare-and-swap says: "write my new value only if the word still holds
the value I computed it from". An update built on it computes freely,
asks, and recomputes when the answer is no. For step 4, ask what must
happen between your read and your swap for the word to hold something
else, when only one task runs at a time.

</details>

<details><summary>Level 2 — the rule</summary>

`atomic_cas` returns the old value, and the swap happened exactly when
that value equals the `expected` you passed. In `kernel/task.c`, a task
loses the processor only when its time slice ends or when it gives it up;
a tick that does not end the slice returns to the same task.

</details>

<details><summary>Level 3 — the shape</summary>

`counter_add_fetchadd`: one call that adds 1. `counter_add_cas`: (1) read
the counter into a local variable; (2) while a compare-and-swap of that
value against `plus_one` of it does not return that value, count one
failure and read the counter again; (3) return the count.

</details>

### B3. The spinlock, in C

<details><summary>Level 1 — the concept</summary>

Test-and-set is "set the word to 1 and tell me what it was". If it was 0,
the lock is yours; if it was 1, it was taken, and setting it to 1 again
changed nothing. For step 4, follow one tick that arrives while a task
holds the lock: who runs next, what that task finds, and what it gives
away.

</details>

<details><summary>Level 2 — the rule</summary>

`atomic_exchange( lock, 1 )` is test-and-set. `spin_lock` puts its barrier
after taking the lock and `spin_unlock` before freeing it; `dmb()` gives
you the same barrier in C. In `kernel/task.c`, every switch resets
`quantum_left`, so a task that receives the processor receives a whole
slice. `QUANTUM=4` builds the same program with a slice of 4 ticks.

</details>

<details><summary>Level 3 — the shape</summary>

`tas_lock`: while test-and-set of the lock returns non-zero, yield; then
the barrier. `tas_unlock`: the barrier, then a plain store of 0. For step
4, read A's value against `COUNT_N`, and compare the four runs.

</details>

### B4. A lock that serves in turn

<details><summary>Level 1 — the concept</summary>

Think of a counter at a shop: a customer takes a number, and enters when
the display shows it. Taking a number must be indivisible; changing the
display is done by one person only, the one being served.

</details>

<details><summary>Level 2 — the rule</summary>

`atomic_fetch_add( &t->next, 1 )` returns a ticket no other call returns.
`serving` is read by the waiting task and written only by the holder.
The barriers go where they went in B3.

</details>

<details><summary>Level 3 — the shape</summary>

`ticket_lock`: take a ticket with fetch-and-add; while `serving` is not
your ticket, yield; then the barrier. `ticket_unlock`: the barrier, then
`serving` plus one. For step 4, follow the lock across two additions of
the same task, while the other task waits.

</details>
