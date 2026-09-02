# Lab tasks

This is where the scratch-OS pillar's weekly lab task descriptions live —
not in `course-material`, since every lab is tied to a specific checkpoint
tag in this same repository (see
[`../checkpoint-tags.md`](../checkpoint-tags.md)). This satisfies the
`course-repository-structure` requirement that scratch-OS lab tasks live
inside `scratch-os` itself.

## Convention

- Filename: `week-NN.md`, e.g. `week-03.md` — the lab assigned during
  week NN (which, per the lab-lag rule, covers week NN-1's lecture
  topic).
- Each lab doc SHALL state:
  - **Starting point**: the checkpoint tag the student checks out before
    starting (`week-(NN-1)-end`).
  - **Task**: what to build, as a variation or extension of that week's
    live-coding target — not a reproduction of it.
  - **Done looks like**: an observable, checkable outcome (e.g. "the
    console prints X," "task Y runs every N ticks," "the race is no
    longer reproducible after 100 runs").
- No reference solution is authored (a deliberate choice — see
  `openspec/changes/author-scratch-os-content/design.md`). If that
  changes for a specific week, the solution lives alongside the task
  description in the same file, clearly separated.
- Full visibility applies here too: lab docs for weeks not yet reached in
  class are visible in the repo like everything else (see
  `course-repository-structure`'s full-visibility policy) — there is no
  gating.

## Week 1

There is no week-1 lab doc — week 1 has no lab (see
`weekly-schedule-structure`'s week-1 exception).
