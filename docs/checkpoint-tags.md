# Weekly folders

`scratch-os` is a cumulative codebase: the flat, static OS you build
grows week over week. Each week's ending state lives in its own folder
on the `main` branch — `week-01/`, `week-02/`, and so on — so you can
open any prior week's code directly, without checking out a tag or
resetting anything.

## Convention

- Folder format: `week-NN/`, e.g. `week-02/`, `week-03/`.
- Each `week-NN/` folder is a complete, standalone snapshot of that
  week's course-authored code (`boot/`, `src/`, `scripts/`, `include/`
  where applicable, and its own `Makefile`) — it builds and runs on its
  own, without depending on any other week's folder.
- Shared material that doesn't change week to week — `docs/labs/`,
  this repository's `README.md`, `LICENSE`, `.gitignore` — stays at the
  repository root, not duplicated into every folder.
- **No branch protection, no hidden folders.** Every week's folder and
  the full commit history remain visible at all times — there is no
  separate "solutions" repo or gated branch.

## Using a checkpoint

```sh
cd week-04   # everything you need for week 4's state lives here
make run     # build and boot it in QEMU
```

## Lab folders are not checkpoints

`week-NN-lab/` folders sit beside the checkpoints but are not part of
this cumulative line. Specifically:

- `week-NN-lab/starter/` is a **prepared** copy of an earlier
  checkpoint, with whatever that lab asks you to write removed. It is
  not that checkpoint.
- `week-NN-lab/solution/` is the end state of one lab's exercises, which
  are variations and extensions rather than a reproduction of the
  course's own line of development. It is not cumulative, nothing
  carries forward from it, and **no later week ever starts from it**.
- A later week's checkpoint or lab always derives from a `week-NN/`
  folder, never from a lab folder.

A lab folder's number is the calendar week the lab runs in, so it does
not pair with the `week-NN/` beside it — `week-04-lab/` runs in week 4
and derives from `week-03/`. See
[`labs/README.md`](labs/README.md).

## Week 1

There is no `week-00` folder — `week-01/` starts from an empty/minimal
project scaffold, since nothing precedes it.

## Week 6

There is no `week-06/` folder. Week 6 is theory-only (no new
implementation work) — `week-05/` stays the checkpoint week 7 (and week
7's own lab) build on.
