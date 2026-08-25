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

## Status

Foundational scaffolding only — the actual weekly content (topic
sequencing, boot code, task implementations) is authored in a follow-up
change once the course's 13-topic sequence is decided.
