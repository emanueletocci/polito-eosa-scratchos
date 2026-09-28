# Week 5 lab — memory management, solo

Covers week 4's topic (static memory partitioning), per the lab-lag
rule — this lab happens during week 5, before that week's lecture
content.

## Starting point

```sh
cd week-04
```

## Task

Working alone. Each task has sub-steps and a checkpoint — the observable
state that says you can move on. No reference solution; ask if a
sub-step blocks you for more than a few minutes.

### 1. Baseline

1. Rebuild and reboot `week-04/`.
2. Confirm `polled OK`, `interrupt OK`, `partitions OK`, then `tick`.

**Checkpoint:** all four messages present.

### 2. Predict the layout before you change it

On paper, using `scripts/mps2_m3.ld`:

1. Write down `RAM` origin, `TASK_PARTITION_SIZE`, `TASK_COUNT`.
2. Assume `.data`+`.bss` end at `0x2000_0C40`. Compute
   `_task0_partition_start/_end` through `_task2_...`, and
   `_task_partitions_end`.
3. Compute `__StackLimit` from `_estack` and `_Min_Stack_Size`. By how
   many bytes does the `ASSERT` pass?

**Checkpoint:** you have six partition addresses and a margin figure,
written down, *before* building.

### 3. Grow to 5 partitions

1. Change `TASK_COUNT` and add `_task3_*` / `_task4_*` lines to the
   `.task_partitions` block in the linker script.
2. Update `src/main.c`: the `extern` declarations, and
   `check_partition_isolation` (two more `fill_partition` calls, two
   more `partition_is` terms, distinct patterns e.g. `0xDD`, `0xEE`).
3. Rebuild, reboot.

**Checkpoint:** `partitions OK` with five regions; and the addresses
the map file (`build/output.map`) reports for `_task4_partition_end`
match what you'd predict by extending step 2's arithmetic.

### 4. Make the `ASSERT` fire on purpose

1. Temporarily set `TASK_PARTITION_SIZE` to something absurd (e.g.
   `0x100000`) so the partitions can't fit under `__StackLimit`.
2. Rebuild.

**Checkpoint:** the build **fails** with the linker `ASSERT` message
about RAM overflow — not a runtime crash. Revert.

### 5. The "break it" demo

Reproduce the class demo (see `course-material`'s
`slide-sources/lecture-notes/week-04.md` to check your reasoning
afterward — try it from memory first):

1. Make one partition's fill overrun by one byte in the direction that
   *is* caught. Confirm `partitions FAILED`.
2. In a comment, explain why the other direction (one byte forward into
   the next partition) is **not** caught by this check ordering.
3. Revert the deliberate bug.

**Checkpoint:** a working-then-reverted `partitions FAILED`, plus the
written explanation.

## Done looks like

- Five isolated partitions, verified at runtime, with predicted
  addresses matching the map file.
- You've seen the linker `ASSERT` turn an overflow into a build error.
- A reverted "break it" demo plus a written explanation of why only one
  overrun direction is caught by this check ordering.
