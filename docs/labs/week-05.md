# Week 5 lab — memory management, solo

Covers week 4's topic (static memory partitioning), per the lab-lag
rule — this lab happens during week 5, before that week's lecture
content.

## Starting point

```sh
git checkout week-04-end
```

## Task

Working alone:

1. Rebuild and reboot `week-04-end`; confirm `partitions OK` prints.
2. Change `TASK_COUNT`/the partition list in `scripts/mps2_m3.ld` to 5
   tasks instead of 3, and update `src/main.c`'s `extern` declarations
   and `check_partition_isolation` to match. Confirm it still reports
   `partitions OK` with all five.
3. Reproduce the "break it" demo from class yourself (see
   `live-coding-guides/week-04.md` if you want to check your reasoning
   afterward, but try it from memory first): make one partition's fill
   overrun in the direction that actually gets caught, confirm you see
   `partitions FAILED`, then explain in a comment *why* the other
   direction (overrunning forward into the next partition) doesn't get
   caught.
4. Revert your deliberate bug before finishing.

## Done looks like

- Five isolated partitions, verified at runtime.
- A working, then reverted, "break it" demonstration — plus a written
  explanation (a code comment is fine) of why only one overrun direction
  is actually caught by this particular check ordering.
