# Week 7 lab — scheduling, part 2, solo

Covers week 6's topic (EDF, aperiodic/sporadic, mixed, predictability),
per the lab-lag rule — this lab happens during week 7, before that
week's lecture content. No new code this week either; this is
schedulability analysis, on paper.

## Starting point

Still `week-05/` — nothing changed in week 6, so there's no newer
folder to start from.

```sh
cd week-05
```

## Task

Working alone, no code required (a text file with your answers is
fine):

1. Given four periodic tasks:

   | Task | $C_i$ (ms) | $T_i$ (ms) |
   |---|---|---|
   | A | 1 | 5 |
   | B | 2 | 8 |
   | C | 1 | 10 |
   | D | 2 | 20 |

   Compute $U$. Is this set schedulable under RMS's sufficient test?
   Under EDF?

2. Same task set: is there a fixed-priority assignment that schedules
   it even if RMS's own sufficient test says "can't prove it"? (Hint:
   the sufficient test is not necessary — try assigning priorities by
   hand and simulating the first couple of hyperperiods.)

3. Pick one task from the set above and reclassify it as sporadic
   instead of periodic (same $C_i$, but now a *minimum* inter-arrival
   time instead of a fixed period). Does your EDF schedulability
   argument from step 1 still hold? Why or why not?

4. Looking at `scratch-os`'s actual `week-05/` dispatcher: which of
   RMS, EDF, or "none of the above" best describes what it implements?
   Justify your answer using the vocabulary from this week's slides.

## Done looks like

- Worked answers to all four questions, with the reasoning shown (not
  just the final yes/no).
