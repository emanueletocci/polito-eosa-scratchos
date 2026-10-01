# Lab tasks

Each lab lives in its own folder at the root of this repository —
[`week-02-lab/`](../../week-02-lab/) through
[`week-08-lab/`](../../week-08-lab/) — beside the `week-NN/` checkpoints,
because every lab is tied to code in this same repository. This file is
the index and the format reference; the labs themselves are not here.

| Lab | Covers | Starts from | Format |
|:--|:--|:--|:--|
| [`week-02-lab/`](../../week-02-lab/) | week 1 — toolchain | `week-01/` | new |
| [`week-03-lab/`](../../week-03-lab/) | week 2 — boot | `week-02/solution/` | moved only |
| [`week-04-lab/`](../../week-04-lab/) | week 3 — interrupts and IO | `week-03/` | moved only |
| [`week-05-lab/`](../../week-05-lab/) | week 4 — memory | `week-04/` | moved only |
| [`week-06-lab/`](../../week-06-lab/) | week 5 — scheduling, part 1 | `week-05/` | moved only |
| [`week-07-lab/`](../../week-07-lab/) | week 6 — scheduling, part 2 | `week-05/` | moved only |
| [`week-08-lab/`](../../week-08-lab/) | week 7 — synchronization | `week-07/` | moved only |

**Labs marked "new" follow the format below in full.** The rest have been
moved into their folders with their task text unchanged, and are being
converted one week at a time; until a lab is converted it has no
`starter/`, no `solution/` and no `SOLUTIONS.md`, and its exercises
carry no time estimates.

## Numbering

A lab folder's number is the **calendar week the lab runs in**, the same
as every other week-numbered thing in this course. Per the lab-lag rule
it covers the *previous* week's topic, so `week-04-lab/` runs in week 4,
covers week 3, and starts from `week-03/`.

That means a lab folder does **not** pair with the `week-NN/` beside it.
You never have to work this out: the starting code is already inside the
lab folder, in `starter/`.

There is no week-1 lab — week 1 has no lab slot (see
[`Class-Schedule.md`](../../../course-material/Class-Schedule.md)). Week
6 is theory-only and produced no code, so there is no `week-06/`
checkpoint and both `week-06-lab/` and `week-07-lab/` start from
`week-05/`.

## Layout

```
week-NN-lab/
  README.md      the task: Part A, Part B, budgets, what "done" looks like
  starter/       what you edit — a prepared copy of the earlier checkpoint
  solution/      the finished state; `make run` works
  SOLUTIONS.md   worked answers to every exercise
```

An analysis-only lab (one with no code) has no `starter/` or
`solution/`, and says so.

`starter/` is **prepared, not copied**: whatever the lab asks you to
write has been removed from it. If a file you expect is missing, that is
the exercise, and the README says so. Never edit a `week-NN/` checkpoint
folder — those are the artifact of record for the lectures, and
`solution/` is not one of them (see
[`../checkpoint-tags.md`](../checkpoint-tags.md)).

## Two parts

Every laboratory divides its exercises in two:

- **Part A — in the laboratory.** Intended for completion during the
  assisted LABINF session, with an assistant available. Self-contained:
  Part A can be completed and the session concluded without beginning
  Part B.
- **Part B — independent work.** Designed to be carried out afterwards,
  unassisted. It requires nothing beyond the repositories, your
  toolchain, and the published solutions.

Exercises marked optional extend the material beyond what the
laboratory requires.

## Exercises and solutions

Exercises are labelled `A1`, `A2`, … in Part A and `B1`, `B2`, … in Part
B. `SOLUTIONS.md` uses the same labels in the same order.

**Solutions are provided with the laboratory**, not withheld or
released subsequently. A solution presents the reasoning by which the
answer is reached rather than the answer alone, and every figure,
diagnostic and disassembly listing quoted in one was produced by
executing the code in `solution/`.

They are provided because Part B is unassisted: work that cannot be
checked cannot readily be learned from. You are expected to attempt an
exercise before consulting its solution — the evidence an exercise asks
you to gather is the substance of the work, and reading a recorded
measurement does not substitute for taking one.

Full visibility applies as everywhere else in this course: labs for
weeks not yet reached are visible in the repository like everything
else.

## Working from the course material

Every exercise names the material it expects you to work from — a
lecture-note section, a tutorial, a self-assessment question. Anything
an exercise *requires* is in the lecture notes or in the lab document
itself; tutorials are
[optional and not examined](../../../course-material/tutorials/), so a
lab will cite one for depth but never depend on it.

Each lab ends with **Where this goes next**, naming what the coming
week's lecture builds on what you just did. Part B is meant to be
preparation as much as revision.

## How the solutions are checked

Every figure, diagnostic, address and listing quoted in a `SOLUTIONS.md`
was produced by executing the command against that lab's `solution/`,
not written from expectation. Every command printed in a lab is executed
from the directory the document places you in, and every `solution/`
program is built and run from a clean tree before the lab is published
and again after any later edit.

You can therefore treat a quoted figure as a measurement. If one does
not reproduce on your machine, that is worth reporting.

## For authors

- One folder per lab, `week-NN-lab/`, `NN` being the calendar week it
  runs in. Add it to the index table above.
- Divide the exercises so that Part A is completable within the
  timetabled session and Part B is designed for independent work. The
  calibration behind that division is an authoring convention and is
  **not published**: no time estimate belongs in a lab document, in
  `SOLUTIONS.md`, in this index, or in the schedule.
- Prepare `starter/` by removing exactly what the lab asks for, and
  record in a header comment which checkpoint it came from and what was
  taken out.
- Build `solution/` and run it before quoting any output in
  `SOLUTIONS.md`. Never write what you expect a command to print; write
  what it printed. Run the authoring toolchain's lab verification before
  publishing and again after every later edit; it must exit clean.
- Keep the anchors `A<n>`/`B<n>` identical across `README.md`,
  `SOLUTIONS.md` and any comment in `starter/` or `solution/`.
- Move a lab's row from "moved only" to "new" in the table above only
  once it satisfies all of the above.
