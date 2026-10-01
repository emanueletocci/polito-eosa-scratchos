# Week 7 lab — scheduling, part 2, solo

Covers week 6's topic (EDF, aperiodic/sporadic, mixed, predictability),
per the lab-lag rule: the lab is held during week 7 and extends the
work done in week 6. No new code this week either; this is
schedulability analysis, on paper.

## Starting point

Still `week-05/` — week 6 is theory-only and adds no code, so there is
no `week-06/` folder and no newer checkpoint to start from. This is the
second lab in a row to build on `week-05/`.

```sh
cd week-05
```

## Task

Working alone, no code required (a text file with your answers is
fine). Each question lists what a complete answer contains — that's the
checkpoint.

### 1. Utilization and the two standard tests

Given four periodic tasks ($D_i = T_i$):

| Task | $C_i$ (ms) | $T_i$ (ms) |
|---|---|---|
| A | 1 | 5 |
| B | 2 | 8 |
| C | 1 | 10 |
| D | 2 | 20 |

1. Compute $U$ (show the fractions and the sum).
2. RMS sufficient test: state the $n=4$ bound and whether $U$ clears it.
3. EDF: schedulable? State the exact condition you're using.

**Complete answer:** three numbers ($U$, the bound, yes/no each) with
the arithmetic shown.

### 2. When the sufficient test can't decide

Same set. If step 1's RMS bound test is inconclusive (or even if it
isn't), run **response-time analysis** for the lowest-priority task (D):
iterate $R = C_D + \sum_{j \in hp(D)} \lceil R/T_j \rceil C_j$ to a
fixed point.

**Complete answer:** the iteration written out line by line, the fixed
point $R_D$, and the verdict $R_D \le D_D$.

### 3. Reclassify one task as sporadic

Pick one task and make it **sporadic**: same $C_i$, but $T_i$ is now a
*minimum* inter-arrival time, not a fixed period.

1. Does your EDF argument from step 1 still hold? Why / why not?
2. Does the RMS analysis from step 2 change?

**Complete answer:** a yes/no with a one-paragraph justification for
each of the two sub-questions.

### 4. An overload case

Construct a 3-task set with $U > 1$. Sketch the EDF timeline over the
first hyperperiod-ish window.

**Complete answer:** the task set, the timeline, and a sentence naming
which job misses first and why later jobs then cascade.

### 5. Classify `scratch-os`'s own dispatcher

Looking at `week-05/`'s actual `scheduler_pick_next` + `SysTick_Handler`:

**Complete answer:** which of RMS / EDF / cyclic-executive / "none"
it is, justified with this week's vocabulary, plus the worst-case
ready-to-running latency in terms of the SysTick period.

## Done looks like

- Worked answers to all five questions with the reasoning shown (not
  just the final yes/no), each meeting its "complete answer" bar.
