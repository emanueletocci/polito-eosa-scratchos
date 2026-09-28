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
| `week-02/` | Boot code written in pure assembly (`boot/startup.s`: vector table, reset handler, CPU setup), fault and SVC handlers, and a privileged → unprivileged demo |
| `week-03/` | SysTick + polled/interrupt-driven UART; `main()` becomes the idle loop |
| `week-04/` | Static per-task memory partitions (`mps2_m3.ld`), isolation verified at runtime |
| `week-05/` | Tick-driven cyclic-executive scheduler (3 tasks, hand-written context switch) |
| *(none)* | Week 6 is theory-only (EDF/aperiodic/mixed/predictability) — `week-05/` stays current |
| `week-07/` | Spinlock (`ldrex`/`strex`), fixing a real UART race observed in week 5 |

**About the weekly folders:** from week 2 on, you will find two versions of each `week-NN/` folder.
The main `week-NN/` folder holds the complete solution that we build together in class.
You can also look at `week-NN/base/`, which contains the starting code where the live coding begins — each has its own Makefile and builds independently.
The `base/README.md` explains how that starting code was prepared so you can understand the foundation before we expand it.

Each folder is a complete, standalone snapshot — `cd week-04 && make run`
works on its own. See [`docs/checkpoint-tags.md`](docs/checkpoint-tags.md)
for the folder convention and [`docs/labs/`](docs/labs/) for each week's
lab task.
