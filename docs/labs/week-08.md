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

Working alone:

1. Rebuild and reboot `week-07/`; run it long enough to convince
   yourself the UART output stays clean.
2. Temporarily remove the `spinlock_acquire`/`spinlock_release` calls
   from `uart_write_polled` and reproduce the interleaving bug yourself
   — run it repeatedly (or for longer) until you actually see it. Save
   one example of the garbled output.
3. Put the spinlock back, confirm the bug is gone again.
4. `task_run_count[]` (in `src/main.c`) is itself a shared array — three
   tasks each increment their own index, never touching each other's.
   Is a spinlock needed there too? Write down your reasoning (this is a
   real question with a real answer, not a trick).
5. Add a second spinlock protecting `task_run_count[]` regardless of
   your answer to (4), and explain in a comment what changes and what
   doesn't.

## Done looks like

- A saved example of the real interleaving bug, reproduced yourself.
- Written reasoning for step 4 (correct either way, as long as the
  reasoning is sound).
- The extra spinlock from step 5, with a comment explaining its actual
  effect.
