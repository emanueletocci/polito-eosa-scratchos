# Week 4 lab — interrupts and IO, solo

Covers week 3's topic (interrupts and IO), per the lab-lag rule — this
lab happens during week 4, before that week's lecture content.

## Starting point

```sh
cd week-03
```

## Task

Working alone. Each numbered task lists its own sub-steps and a
"checkpoint" — the observable state that tells you it's done before you
move on. No reference solution is provided; if you're stuck for more
than a few minutes on a sub-step, ask.

### 1. Get back to a known-good baseline

1. Rebuild and reboot `week-03/`.
2. Confirm the output, in order: `polled OK`, `interrupt OK`, then
   `tick` repeating.

**Checkpoint:** you see the three messages and a steady stream of
`tick`. If not, fix that before touching anything else.

### 2. Add interrupt-driven UART receive

Build it up in this order — test-compile after each sub-step:

1. Declare the handler `void UART0RX_Handler(void)` and leave its body
   empty for now. Compile.
2. In `boot/startup.c`, replace the `Default_Handler` at the **IRQ0**
   slot (`isr_vector[16 + UARTRX0_IRQn]`) with `UART0RX_Handler`. Add
   the `extern` declaration next to the existing ones. Compile.
3. Enable the receiver: set `UART_CTRL_RXEN | UART_CTRL_RXIRQEN` in
   `CMSDK_UART0->CTRL` (keep the TX bits that are already there), and
   set the IRQ0 bit in `NVIC_ISER0`. Do this in `uart_init` or a new
   `uart_rx_init` — your choice.
4. Fill in the handler: read one byte from `CMSDK_UART0->DATA`,
   acknowledge the RX interrupt in `INTSTATUS` (the RX bit, not the TX
   bit), and echo the byte back with `uart_write_polled` (build a
   2-char NUL-terminated buffer, or add a small `uart_putc`).

**Checkpoint:** it still builds, and boot output is unchanged from
task 1 (you haven't broken TX).

### 3. Prove RX works

1. `make run`, then type characters into the QEMU console — just type,
   don't press `Ctrl-A`.
2. Each character you type should appear once, echoed by your handler.

**Checkpoint:** typed characters echo back, and `tick` lines keep
coming in between — RX is interrupt-driven, not blocking the idle loop.

### 4. Observe the tick rate as a parameter

1. Change the tick-report threshold (currently `>= 20`) to `>= 5`, then
   to `>= 100`. Rebuild and run each time.

**Checkpoint:** the visible `tick` rate changes proportionally, and you
can say why (the SysTick period didn't change — only how many periods
pass between reports).

### 5. (Stretch) A second peripheral, on paper

Using the GPIO register sketch from the week-3 slides, write — in a
comment block, not compiled — the three lines that would configure pin 0
as an output and drive it high. Note next to it *why* you can't verify
this in headless QEMU.

## Done looks like

- Typed characters echo back over UART, driven by your own
  `UART0RX_Handler` — not polling for input.
- Boot output (`polled OK` / `interrupt OK` / `tick`) is unchanged: your
  RX additions didn't disturb the existing checkpoint behaviour.
- You can explain why RX needs **both** a peripheral-level enable
  (`RXEN`, `RXIRQEN`) **and** an NVIC-level enable — the same two-step
  pattern TX used — and where the RX acknowledge differs from TX.
