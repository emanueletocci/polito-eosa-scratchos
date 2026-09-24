# scratch-os

The cumulative, from-scratch flat operating system you build across the
Embedded Operating Systems and Architecture course. Boot → running static tasks, on
QEMU's `mps2-an385` (ARM Cortex-M3) machine model. Static memory, static
tasks — no dynamic allocation, no virtual memory (that's what the
`freertos-labs` pillar is for).

See:
- [`../course-material/docs/three-pillar-model.md`](../course-material/docs/three-pillar-model.md) — how this pillar fits the course
- [`../course-material/docs/environment-setup.md`](../course-material/docs/environment-setup.md) — toolchain setup
- [`docs/checkpoint-tags.md`](docs/checkpoint-tags.md) — how weekly progress is organized

## License

Code in this repository is licensed under the [MIT License](LICENSE).
Every source file carries a header per
[`code-header-template.md`](../course-material/docs/code-header-template.md).

## Building and running

```sh
make          # build build/scratch-os.axf
make run      # build, then boot it in QEMU
```

Requires the toolchain — see
[`environment-setup.md`](../course-material/docs/environment-setup.md).

## Weekly folders

| Folder | What it adds |
|---|---|
| `week-01/` | Toolchain pipeline proof (boot → semihosting write) |
| `week-02/` | Explicit `naked`-asm `Reset_Handler` (same skill week 5 reuses) |
| `week-03/` | SysTick + polled/interrupt-driven UART; `main()` becomes the idle loop |
| `week-04/` | Static per-task memory partitions (`mps2_m3.ld`), isolation verified at runtime |
| `week-05/` | Tick-driven cyclic-executive scheduler (3 tasks, hand-written context switch) |
| *(none)* | Week 6 is theory-only (EDF/aperiodic/mixed/predictability) — `week-05/` stays current |
| `week-07/` | Spinlock (`ldrex`/`strex`), fixing a real UART race observed in week 5 |

Each folder is a complete, standalone snapshot — `cd week-04 && make run`
works on its own. See [`docs/checkpoint-tags.md`](docs/checkpoint-tags.md)
for the folder convention.

## Lab folders

Beside the checkpoints sit `week-02-lab/` through `week-08-lab/`, one per
lab. A lab folder's number is the **calendar week the lab runs in**, and
per the lab-lag rule it covers the *previous* week's topic — so
`week-04-lab/` runs in week 4, covers week 3, and its `starter/` derives
from `week-03/`. A lab folder therefore does **not** pair with the
`week-NN/` next to it, and you never need to work out which checkpoint to
copy: the prepared starting code is already inside the lab folder.

Week 6 produced no code, so there is no `week-06/` — both `week-06-lab/`
and `week-07-lab/` start from `week-05/`.

| Folder | What it holds |
|---|---|
| `week-NN-lab/README.md` | the task: Part A, for the laboratory session, and Part B, for independent work afterwards |
| `week-NN-lab/starter/` | what you edit — prepared, with the lab's own work removed |
| `week-NN-lab/solution/` | the finished state; `make run` works |
| `week-NN-lab/SOLUTIONS.md` | worked answers, published immediately |

The labs are being converted to this format one week at a time. At
present only `week-02-lab/` is complete; the rest hold their task text
and, until converted, have no `starter/`, no `solution/` and no
`SOLUTIONS.md`. The status table in
[`docs/labs/README.md`](docs/labs/README.md) records where each one
stands.

Where solutions are published, they are verified by execution first —
each program built and run from a clean tree, and every command the
documents quote actually executed.

`starter/` and `solution/` are **not checkpoints**: nothing later builds
on them, and they are not cumulative. See
[`docs/labs/README.md`](docs/labs/README.md) for the lab index and
format, and [`docs/checkpoint-tags.md`](docs/checkpoint-tags.md) for why
the distinction matters.
