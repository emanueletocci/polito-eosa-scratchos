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

## Week 1

There is no `week-00` folder — `week-01/` starts from an empty/minimal
project scaffold, since nothing precedes it.

## Week 6

There is no `week-06/` folder. Week 6 is theory-only (no new
implementation work) — `week-05/` stays the checkpoint week 7 (and week
7's own lab) build on.
