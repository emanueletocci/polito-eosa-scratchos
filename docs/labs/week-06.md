# Week 6 lab — scheduling, part 1, solo

Covers week 5's topic (the cyclic-executive dispatcher), per the
lab-lag rule — this lab happens during week 6, before that week's
lecture content.

## Starting point

```sh
git checkout week-05-end
```

## Task

Working alone:

1. Rebuild and reboot `week-05-end`; confirm `task0`/`task1`/`task2`
   cycle.
2. Add a 4th task. You'll need to: add a 4th partition to
   `scripts/mps2_m3.ld`, bump `TASK_COUNT`, write `task3_entry`, add its
   `task_stack_init` call in `main()`. `scheduler_pick_next` shouldn't
   need to change at all — explain in a comment why not.
3. Change the schedule from plain round-robin to a fixed **table**
   instead of a modulo counter — e.g. `{0, 1, 2, 0, 1, 3}` (task 0 runs
   twice as often as 2 and 3). This is closer to a "real" cyclic
   executive, where the table is designed offline to meet each task's
   rate requirement, not just cycled evenly.
4. Try deliberately corrupting one task's saved context (e.g.
   temporarily have `task_stack_init` reserve one word too few) and
   observe what breaks. Revert it. This isn't about fixing anything —
   it's about seeing what a wrong stack frame actually looks like when
   it goes wrong.

## Done looks like

- 4 tasks, isolated stacks, dispatched from a fixed table (not just
  modulo round-robin).
- A comment explaining why `scheduler_pick_next` didn't need to change
  to add a 4th task (hint: what does it actually know about *how many*
  tasks there are, versus what `main()` knows?).
