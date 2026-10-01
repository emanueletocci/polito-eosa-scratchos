# Week 6 lab — scheduling, part 1, solo

Covers week 5's topic (the cyclic-executive dispatcher), per the
lab-lag rule: the lab is held during week 6 and extends the work done
in week 5.

## Starting point

`week-05/` — this lab covers week 5's topic, so it builds on week 5's
checkpoint, not on the `week-06/` folder its own number suggests. There
is no `week-06/`: week 6 is theory-only and adds no code, so `week-05/`
stays current.

```sh
cd week-05
```

## Task

Working alone. Sub-steps and a checkpoint per task; no reference
solution.

### 1. Baseline

1. Rebuild and reboot `week-05/`.
2. Confirm `polled OK`, `interrupt OK`, `partitions OK`, then
   `task0`/`task1`/`task2` cycling.

**Checkpoint:** all three tasks visibly take turns.

### 2. Add a 4th task

1. Add a 4th partition to `scripts/mps2_m3.ld` (`_task3_*`), bump
   `TASK_COUNT`.
2. Write `task3_entry` (copy the shape of `task2_entry`, new index and
   message).
3. Add the `tasks[3].sp = task_stack_init(_task3_partition_end,
   task3_entry);` line in `main()`.
4. Build, run.

**Checkpoint:** `task3` now appears in the rotation, and
`scheduler_pick_next` was **not** edited. In a comment, say why not —
what does it know about task *count* vs. what `main()` knows?

### 3. Trace one switch by hand

Before changing the policy: pick the moment task 1 → task 2. Using the
week-5 "instruction-by-instruction" slide as a template, write down, in
a comment, the value of `r0` after each asm line of `SysTick_Handler`
for that switch, given a plausible starting `MSP`.

**Checkpoint:** a comment block with 8 lines, one per instruction,
ending at `bx lr`.

### 4. Replace modulo with a real table

1. Change `scheduler_pick_next` to step through a fixed array, e.g.
   `{0, 1, 2, 0, 1, 3}`, wrapping at its length.
2. Build, run.

**Checkpoint:** task 0 and task 1 appear about twice as often as tasks
2 and 3 — a hand-designed rate, not an even split.

### 5. Break a stack frame on purpose

1. Temporarily make `task_stack_init` reserve **one word too few**
   (e.g. loop `i < 7` instead of `i < 8`).
2. Build, run, observe.
3. Revert.

**Checkpoint:** you've seen what a misaligned/short saved frame does at
the first switch into that task (wrong PC, hard fault, or garbage) —
and can describe which word ended up in which register.

## Done looks like

- 4 tasks, isolated stacks, dispatched from a fixed **table** (not just
  modulo round-robin).
- A comment explaining why `scheduler_pick_next` didn't need to change
  to add a 4th task.
- A by-hand instruction trace of one real context switch.
- A reverted "short stack frame" experiment with a written description
  of what broke and why.
