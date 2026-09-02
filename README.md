# scratch-os

The cumulative, from-scratch flat operating system built by students across
the Embedded Operating Systems course. Boot → running static tasks, on
QEMU's `mps2-an385` (ARM Cortex-M3) machine model. Static memory, static
tasks — no dynamic allocation, no virtual memory (that's what the
`freertos-labs` pillar is for).

See:
- [`../course-material/docs/three-pillar-model.md`](../course-material/docs/three-pillar-model.md) — how this pillar fits the course
- [`../course-material/docs/environment-setup.md`](../course-material/docs/environment-setup.md) — toolchain setup
- [`docs/checkpoint-tags.md`](docs/checkpoint-tags.md) — how weekly progress is tagged

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

## Weekly checkpoints

| Tag | What it adds |
|---|---|
| `week-01-end` | Toolchain pipeline proof (boot → semihosting write) |
| `week-02-end` | Explicit `naked`-asm `Reset_Handler` (same skill week 5 reuses) |
| `week-03-end` | SysTick + polled/interrupt-driven UART; `main()` becomes the idle loop |
| `week-04-end` | Static per-task memory partitions (`mps2_m3.ld`), isolation verified at runtime |
| `week-05-end` | Tick-driven cyclic-executive scheduler (3 tasks, hand-written context switch) |
| *(none)* | Week 6 is theory-only (EDF/aperiodic/mixed/predictability) — `week-05-end` stays current |
| `week-07-end` | Spinlock (`ldrex`/`strex`), fixing a real UART race observed in week 5 |

See [`docs/checkpoint-tags.md`](docs/checkpoint-tags.md) for the tagging
convention and [`docs/labs/`](docs/labs/) for each week's lab task.

## Status

Weeks 1-7 (the scratch-OS half of the course) are complete — see
`openspec/changes/author-scratch-os-content`. Weeks 8-13 are FreeRTOS
territory (`freertos-labs`), not this repo, and not yet sequenced.
