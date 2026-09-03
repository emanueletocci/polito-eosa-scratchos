# Week 8 lab — synchronization, solo

Covers week 7's topic (the spinlock), per the lab-lag rule — this lab
happens during week 8, the first week of the FreeRTOS pillar, before
that week's own lecture content. This is the last lab against
`scratch-os` in this course.

## Starting point

```sh
cd week-07
```

## Task

Working alone. Sub-steps and a checkpoint per task; no reference
solution.

### 1. Confirm the fix holds

1. Rebuild and reboot `week-07/`.
2. Let it run for at least a minute of wall-clock.

**Checkpoint:** every output line is exactly one of `polled OK`,
`interrupt OK`, `partitions OK`, `task0`, `task1`, `task2` — no
run-together lines.

### 2. Reproduce the bug without the lock

1. Comment out the `spinlock_acquire` / `spinlock_release` calls in
   `uart_write_polled` (leave the functions defined).
2. Rebuild, run — repeatedly, or for longer — until you see a garbled
   line.
3. Save one example (copy the exact console text into a file).

**Checkpoint:** a saved garbled line, and a note of roughly how long it
took to appear.

### 3. Put it back

1. Restore the two calls. Rebuild, run.

**Checkpoint:** clean output again over a run at least as long as the
one that caught the bug in step 2.

### 4. Is `task_run_count[]` a race?

`task_run_count[]` in `src/main.c`: three tasks, each `++`-ing its own
index, never another's.

**Checkpoint:** written reasoning for whether a lock is needed here.
Address: is `++arr[i]` atomic on Cortex-M3? Does it matter if two tasks
never touch the same index? What about the `volatile` qualifier — what
does it give you and what does it *not*?

### 5. Add a second spinlock anyway

1. Add `count_lock` and wrap the `++task_run_count[...]` in
   acquire/release in all three task functions.
2. Rebuild, run.

**Checkpoint:** output unchanged, and a comment stating what actually
changed (contention cost, ordering) and what didn't (correctness, since
step 4's reasoning already settled that) — plus whether the extra lock
could *introduce* a problem under a priority scheduler.

## Done looks like

- A saved example of the real interleaving bug, reproduced yourself,
  with a rough time-to-appear.
- Written reasoning for step 4 (correct either way, as long as the
  reasoning is sound).
- The extra spinlock from step 5, with a comment explaining its actual
  effect and its cost.
