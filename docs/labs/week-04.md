# Week 4 lab — interrupts and IO, solo

Covers week 3's topic (interrupts and IO), per the lab-lag rule — this
lab happens during week 4, before that week's lecture content.

## Starting point

```sh
cd week-03
```

## Task

Working alone:

1. Rebuild and reboot `week-03/`; confirm `polled OK`, `interrupt OK`,
   then repeating `tick` lines.
2. Add interrupt-driven UART **receive**: enable UART0 RX (`RXEN`,
   `RXIRQEN` in `CTRL`, and IRQ0 — `UARTRX0_IRQn` — in the NVIC), write
   a `UART0RX_Handler` that reads `CMSDK_UART0->DATA` and echoes the
   character straight back out via `uart_write_polled`. Extend
   `boot/startup.c`'s vector table to route IRQ0 to it instead of
   `Default_Handler`.
3. Try typing into the QEMU console (not `Ctrl-A X` — just type) and
   confirm your typed characters echo back.
4. Change the tick-report threshold (currently every 20 ticks) to
   something else and observe the reporting rate change accordingly.

## Done looks like

- Typed characters echo back over UART, driven by your own
  `UART0RX_Handler` — not polling for input.
- You can explain why RX needs both a peripheral-level enable (`RXEN`,
  `RXIRQEN`) and an NVIC-level enable, the same two-step pattern TX used.
