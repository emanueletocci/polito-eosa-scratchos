# Week 4 laboratory — interrupts and I/O

This laboratory covers week 3's topic, devices, interrupts and system
calls, and extends the work done in class: you take the finished week 3
operating system and add to it the devices and the interrupt handling
the lecture introduced. Part A, in the laboratory, makes reception
interrupt-driven, writes a timer driver and observes one interrupt
preempting another. Part B, independent work, adds a race and its
repair, a second serial port, the watchdog, the board's LEDs and GPIO
block, a fourth system call, and an interrupt raised from software.
Worked solutions are in [`SOLUTIONS.md`](SOLUTIONS.md), and the
finished code in [`solution/`](solution/).

## Objectives

On completion you will have:

- replaced a polled receive path by an interrupt-driven one, with a
  ring buffer between the handler and its reader, and acknowledged the
  interrupt at the device;
- written a driver for a second device, the timer, enabling its
  interrupt at both levels, in the device and in the NVIC;
- observed, in the output and with the debugger, one interrupt preempt
  another and, in the exception number and `EXC_RETURN`, the processor
  returning to handler mode;
- demonstrated a lost update between a handler and the code it
  interrupts, and removed it with `irq_save` and `irq_restore`;
- reused one driver for two instances of a device, and driven a device
  that raises its interrupt as a non-maskable one;
- read and written device registers from the debugger, and decoded the
  accesses to a device the emulator does not model from its diagnostic
  output;
- added a system call, naming the four places a new service touches.

## Starting point

[`../week-03/solution/`](../week-03/solution/), the finished week 3
operating system. It has no copy in this folder: the exercises begin
from it exactly as published. Work on your own copy, so the original
stays as a reference. From the `scratch-os/` folder:

```sh
cp -r week-03/solution my-week4
cd my-week4
make clean
```

Every path below is relative to `my-week4/`. All register and IRQ names
come from `include/cmsdk.h`: keep it open while you work, and include it
in any file of yours that touches a register.

## Solutions

Worked solutions to every exercise are provided in
[`SOLUTIONS.md`](SOLUTIONS.md), and the finished code of the exercises
whose result you keep in [`solution/`](solution/). You are expected to
attempt each exercise before consulting its solution.

## Structure

Each exercise lists its own sub-steps and a **Checkpoint**, the
observable state that tells you it is done before you move on. Part A
is for the assisted session: it is self-contained. Part B is for work
you carry out independently afterwards.

Four rules hold for every exercise:

- **Acknowledge at the device.** The NVIC lines of this board are
  level-sensitive: the line stays active until the device's own flag is
  cleared. A UART handler writes its bit to `INTSTATUS`; a timer handler
  writes `TIMER_INT_PENDING` to its `INTSTATUS` (both write 1 to clear).
  Forget it and the handler runs again and again.
- **Two enables.** A device interrupt needs the enable bit in the device
  *and* its bit in `NVIC->ISER[n / 32]`. Set the priority byte
  `NVIC->IPR[n]` before the `ISER` write.
- **Device registers are privileged.** Write them from kernel code: a
  handler, a system call, or `main()` *before* the privilege drop. The
  application (`src/app.c`) runs unprivileged and may not touch them.
- **Experiments are made in a copy.** A4, B1 and B7 change code you do
  not keep. Make each in a copy of your working folder
  (`cp -r my-week4 my-week4-exp`, then `make clean` in it), so that
  `my-week4` stays as the previous exercise left it.

The debugger needs two terminals: `make debug` in the first one, then in
the second `arm-none-eabi-gdb build/scratch-os.axf` and
`target remote :1234`. How to obtain the debugger is in
[`environment-setup.md`](../../course-material/docs/environment-setup.md).
While the CPU is stopped in unprivileged code, gdb may fail to read
device registers: stop inside a handler first, with
`break *SysTick_Handler` and `continue`. The `*` stops on the handler's
very first instruction.

If you are stuck on a sub-step, read the solution of that exercise:
it gives the code and the reason for it.

---

## Part A — in the laboratory

### A1. A known-good baseline

1. `make run`.
2. Quit with `Ctrl-C`.

**Checkpoint:** you see, in order:

```
scratch-os: kernel started
dropping to unprivileged mode
app: hello from unprivileged code
uptime: 1000 ms
uptime: 2000 ms
...
```

and pressing a key prints `key: <the key>`. If not, fix that before
touching anything else.

### A2. Interrupt-driven UART receive

Device: `CMSDK_UART0` (`0x40004000`). IRQ: `UART0RX_IRQn` (0), handler
`UART0RX_Handler`. Today `sys_read` polls `STATE`, every 10 ms. The
UART holds one received byte: on real hardware, a byte that arrives
before the previous one was read is lost (QEMU holds the extra bytes
back for you, so you will not see the loss here). You move reception to
the interrupt, with a ring buffer between the handler and `sys_read`.

1. In `drivers/uart.c`, add an RX ring buffer: `rx_buf[RX_BUF_SIZE]`
   (a power of two, e.g. 32), `rx_head`, `rx_tail`, both `volatile`.
   Copy the shape of the TX buffer. Here the *handler* writes `rx_head`
   and the *reader* writes `rx_tail`.
2. Write `void UART0RX_Handler(void)`: read the byte from `DATA`, store
   it if the buffer is not full (drop it if it is), and acknowledge with
   `CMSDK_UART0->INTSTATUS = UART_INT_RX`. Declare it in
   `drivers/uart.h`. The vector table already names it: your function
   replaces the weak default.
3. In `uart_init`, add `UART_CTRL_RXIRQEN` to `CTRL` (keep `UART_CTRL_TXEN` and
   `UART_CTRL_RXEN`), set `NVIC->IPR[UART0RX_IRQn] = 0xC0` (same as TX), and set
   its bit in `NVIC->ISER`.
4. Add `int uart_getc(void)`: return the oldest byte from the buffer,
   or -1 if it is empty. Protect the index update with `irq_save()` /
   `irq_restore()` (`kernel/irq.h`), as `uart_write` does.
5. In `kernel/syscall.c`, make `sys_read_impl` call `uart_getc()`
   instead of `uart_getc_polled()`.

**Checkpoint:** `make run`, type quickly (several keys in a row): every
key prints `key: <the key>`, and `uptime` lines keep coming. In gdb,
`break *UART0RX_Handler`, `continue`, type a key in the QEMU terminal:
the breakpoint is hit.

### A3. A timer driver

Device: `CMSDK_TIMER0` (`0x40000000`). IRQ: `TIMER0_IRQn` (8), handler
`TIMER0_Handler`. The timer counts `VALUE` down at 25 MHz; at 0 it
reloads `RELOAD` and, if `TIMER_CTRL_IRQEN` is set, raises its IRQ.

1. Create `drivers/timer.c` and `drivers/timer.h` (the Makefile finds new
   files by itself) with `void timer0_init(uint32_t period_ms)`.
2. In it: `RELOAD = 25000 * period_ms` (25 000 cycles per ms), then
   `CTRL = TIMER_CTRL_EN | TIMER_CTRL_IRQEN`, then
   `NVIC->IPR[TIMER0_IRQn] = 0xC0` and its `ISER` bit.
3. Write `TIMER0_Handler`: acknowledge with
   `CMSDK_TIMER0->INTSTATUS = TIMER_INT_PENDING`, count the ticks in a
   `static` variable, and every 10th tick print it with `kprintf`
   (`kernel/kprintf.h`).
4. Call `timer0_init(100)` from `main()`, before the privilege drop (add
   `#include "drivers/timer.h"`).

**Checkpoint:** a `timer0: <n>` line every second, between the `uptime`
lines. Comment out the acknowledge line: the system stops printing
`uptime` (the handler runs forever). Put it back.

### A4. Preemption

Make this experiment in a copy of your working folder (see the rules
above), so that `my-week4` keeps the timer of A3.

Devices: `CMSDK_TIMER0` (fast, urgent) and `CMSDK_TIMER1`
(`0x40001000`, IRQ `TIMER1_IRQn` (9), handler `TIMER1_Handler`, slow,
not urgent). Lower priority number = more urgent. The current
priorities are SVCall `0x80`, UART `0xC0`, SysTick `0xE0`.

1. Set `NVIC->IPR[TIMER0_IRQn] = 0x40`: more urgent than everything
   above. Change its handler to print `T0` at each tick, or
   `  T0 (inside T1)` when a `static volatile int in_slow` flag is set.
2. Start `CMSDK_TIMER1` with a 1 s period (`RELOAD = 25000000`) at
   `NVIC->IPR[TIMER1_IRQn] = 0xE0`.
3. Write a deliberately slow `TIMER1_Handler`: acknowledge, set
   `in_slow = 1`, print `T1 start`, busy-wait with
   `for (volatile uint32_t i = 0; i < 100000000; i++) { }`, print
   `T1 end`, set `in_slow = 0`. Adjust the loop count until each `T1`
   takes about 300 ms on your machine.
4. In gdb: `break *TIMER1_Handler`, `continue`, then
   `p/x $xpsr & 0x1ff` (the low 9 bits of xPSR are the number of the
   active exception; gdb has no `$ipsr`) and `p/x $lr`. Then `delete`,
   `break *TIMER0_Handler`, `continue`, and the same two commands, then
   `bt`.

**Checkpoint:** the output shows `T1 start`, two or three
`  T0 (inside T1)`, then `T1 end`. In gdb, `TIMER1_Handler` shows
exception `0x19` (16 + 9) with `lr = 0xfffffff9` (return to thread
mode), and the nested `TIMER0_Handler` shows `0x18` (16 + 8) with
`lr = 0xfffffff1` (return to *handler* mode: to `TIMER1_Handler`).
`bt` shows `TIMER1_Handler` below it. Now swap the two priorities and
explain why no `T0` appears inside `T1` any more, and why `uptime` stalls
while `T1` runs (SysTick is `0xE0` too).

### Done looks like — Part A

- `make run` prints the baseline, then a `timer0: <n>` line each second
  between the `uptime` lines, and every key you type reaches the
  application through `UART0RX_Handler` and an RX ring buffer, none lost
  when you type fast.
- You have shown `TIMER0` preempting a slow `TIMER1_Handler`, in the
  output and in gdb (exception number and `EXC_RETURN`), in a copy that
  you can discard.
- `my-week4` builds and runs, with nothing left half-applied.

---

## Part B — independent work

### B1. A race, and its fix

A lost update: `main` does `counter = counter + 1` (load, add, store)
while `TIMER0_Handler` also increments `counter`. If the interrupt falls
between the load and the store, the handler's increment is overwritten.

Make this experiment in a copy of your working folder, and in `main()`
**before the privilege drop**. The fix
uses `cpsid i` (inside `irq_save`), and `cpsid i` has no effect in
unprivileged code: the experiment would show nothing.

1. Make `counter` and `isr_hits` `static volatile uint32_t`. The handler
   acknowledges, then does `counter++` and `isr_hits++`.
2. Start `TIMER0` with a 1 ms period (`RELOAD = 25000`).
3. In `main`, run 100 000 times:
   `uint32_t tmp = counter;` then
   `for (volatile int j = 0; j < 100; j++) { }` then
   `counter = tmp + 1;`. The small delay widens the gap between load and
   store. Without it you would almost never see a loss under QEMU: it
   translates groups of instructions at once and checks for interrupts
   only between groups.
4. Stop the timer (`CTRL = 0`) and print `counter`, the expected value
   `100000 + isr_hits`, and the difference.
5. Fix it: wrap the three lines in `uint32_t s = irq_save();` …
   `irq_restore(s);` and run again.

**Checkpoint:** before the fix, the difference is not 0 (a few tens of
lost updates). After the fix it is 0 every time. You can say why
`irq_save` / `irq_restore` is better here than a plain `cpsid i` /
`cpsie i`.

### B2. One driver, two UARTs

Device: `CMSDK_UART1` (`0x40005000`). IRQs: `UART1RX_IRQn` (2) and
`UART1TX_IRQn` (3), handlers `UART1RX_Handler` and `UART1TX_Handler`.
QEMU connects its serial ports to the UARTs in order: the first
`-serial` option is UART0, the second is UART1.

1. Give the polled functions a UART parameter:
   `void uart_putc_polled_on(CMSDK_UART_TypeDef *u, char c)`. Keep the
   old `uart_putc_polled(c)` as a one-line wrapper that passes
   `CMSDK_UART0`, so `kprintf` keeps working.
2. Add `uart_init_on(CMSDK_UART_TypeDef *u)` that sets `BAUDDIV` and
   `CTRL` for any UART.
3. From `main()`, before the drop, initialise `CMSDK_UART1` and send
   `hello uart1\n` on it.
4. Run with a second serial port. Either add a variable to the Makefile:
   `QEMU_EXTRA ?=` and put `$(QEMU_EXTRA)` at the end of the `QEMU :=`
   line, then

   ```sh
   make run QEMU_EXTRA="-serial file:uart1.txt"
   ```

   or type the full command by hand:

   ```sh
   qemu-system-arm -machine mps2-an385 -monitor null -nographic -serial stdio -serial file:uart1.txt -kernel build/scratch-os.axf
   ```

5. (Optional) Make the ring-buffer TX path work on UART1 too: one
   buffer per UART, and `UART1TX_Handler` calling a shared function.

**Checkpoint:** the terminal shows the usual output; after quitting,
`cat uart1.txt` prints `hello uart1`.

### B3. The watchdog

Device: `CMSDK_WDOG` (`0x40008000`). It has no IRQ slot: its interrupt
arrives as **NMI**, handler `NMI_Handler`, which nothing can mask. Its
registers ignore writes while locked: write `WDOG_UNLOCK_KEY`
(`0x1ACCE551`) to `LOCK` first, any other value to lock again.

1. Write `void NMI_Handler(void)` that prints `watchdog: NMI` with
   `kprintf`.
2. In `main()`, before the drop: unlock, `LOAD = 25000000` (1 s),
   `CTRL = WDOG_CTRL_INTEN | WDOG_CTRL_RESEN`, lock.
3. Run it. When the counter first reaches 0 it raises NMI and starts
   again. If nobody writes `INTCLR` before it reaches 0 a second time,
   it resets the board.
4. Feed it: in `SysTick_Handler` (`drivers/systick.c`) unlock, write
   `INTCLR`, lock.

**Checkpoint:** without feeding: `watchdog: NMI` about one second after
start, then about one second later the boot banner
`scratch-os: kernel started` prints again, and the cycle repeats. With
feeding: no NMI, `uptime` counts on. Now put a `for (;;) { }` inside
`SysTick_Handler` after a few seconds of uptime: the watchdog catches
the stuck system.

### B4. Board LEDs

Device: `FPGAIO` (`0x40028000`), register `LED`: one bit per LED,
1 = on. This board has two LEDs (bits 0 and 1). No IRQ. QEMU has no
window for the LEDs, so you look at them through the debugger.

1. In `main()`, before the drop, write `FPGAIO->LED = 0x3;`.
2. `make debug`; in gdb: `target remote :1234`,
   `break *SysTick_Handler`, `continue`.
3. Read it back two ways: `x/wx 0x40028000` and `p/x FPGAIO->LED`
   (the build keeps macro information, so gdb knows `FPGAIO`).
4. Write from gdb: `set var FPGAIO->LED = 0xff`, then `p/x FPGAIO->LED`.

**Checkpoint:** both reads show `0x3`. After writing `0xff` from gdb
you read back `0x3`: only the bits that exist are stored. You can say
why you stopped in a handler before reading.

### B5. GPIO seen from the bus

**What GPIO is.** A GPIO (general-purpose input/output) block drives a
row of *pins*, the metal contacts of the chip. Each pin is either an
*input* (the chip reads the voltage put on it from outside) or an
*output* (the chip drives it high or low). Its *direction* is chosen in
software. For an output, the level comes from an *output latch*: a
register bit that holds the value until it is changed. In the CMSDK
GPIO block (`CMSDK_GPIO0`, `0x40010000`), bit *n* of each register is
pin *n*:

| Offset | Register       | Meaning                                    |
|--------|----------------|--------------------------------------------|
| 0x000  | `DATA`         | read: current level of every pin           |
| 0x004  | `DATAOUT`      | output latch: the level each output drives |
| 0x010  | `OUTENABLESET` | write 1: make that pin an output           |
| 0x014  | `OUTENABLECLR` | write 1: make that pin an input            |

`OUTENABLESET` and `OUTENABLECLR` are *set/clear registers*: a 1 changes
its pin, a 0 leaves it alone. One write, no need to read first. A plain
register like `DATAOUT` needs read-modify-write to change one bit
without touching the others.

QEMU has no model for this block, only a placeholder. With `-d unimp`
(`make run-trace`) it logs every access, and every read returns 0.

1. Create `drivers/gpio.c` and `drivers/gpio.h` with
   `void gpio_set_output(unsigned pin)`, which writes
   `CMSDK_GPIO0->OUTENABLESET = 1UL << pin`, and
   `void gpio_write(unsigned pin, int level)`, which does
   `DATAOUT |= 1UL << pin` for 1 and `DATAOUT &= ~(1UL << pin)` for 0.
2. In `main()`, before the drop: `gpio_set_output(0);` then
   `gpio_write(0, 1);`.
3. `make run-trace`.
4. Decode each log line against the table: which register (offset),
   read or write, which value.
5. Add `gpio_write(1, 1);` after the first write and run again.

**Checkpoint:** right after `scratch-os: kernel started` you see:

```
cmsdk-ahb-gpio: unimplemented device write (size 4, offset 0x010, value 0x00000001)
cmsdk-ahb-gpio: unimplemented device read  (size 4, offset 0x004)
cmsdk-ahb-gpio: unimplemented device write (size 4, offset 0x004, value 0x00000001)
```

and you can explain them: `OUTENABLESET` is one write (pin 0 becomes an
output); `DATAOUT |= bit` is a read then a write. After sub-step 5, the
second `DATAOUT` write has value `0x00000002`, not `0x00000003`: the read
returned 0, so pin 0 was turned off again. Explain why reads return 0
here, and why set/clear registers are safer than read-modify-write (also
when an interrupt can change the same register between the read and the
write).

### B6. Add a system call

Goal: `sys_led(bits)` lets the application set the LEDs of B4.
A new service touches four places:

1. **Number:** in `lib/syscalls.h`, `#define SYS_LED 4` and the
   prototype `int sys_led(uint32_t bits);`.
2. **Stub:** in `lib/syscalls.s`, copy one stub as `sys_led` with
   `svc #4`. The number is written as a plain 4: the assembler cannot
   read the C header.
3. **Table:** in `kernel/syscall.c`, add `[SYS_LED] = sys_led_impl` to
   `table`.
4. **Implementation:** `sys_led_impl` writes `FPGAIO->LED = a0 & 0x3`
   and returns 0.
5. In `src/app.c`, call `sys_led(1)` and `sys_led(2)` on alternate
   seconds.

**Checkpoint:** in gdb, stopped at `break *SysTick_Handler`,
`p/x FPGAIO->LED` shows `0x1` or `0x2` depending on the second. Now make
the stub use `svc #5` by mistake: `sys_led` returns -1 and the LEDs stop
changing. Put it back.

### B7. (Stretch) Pend an IRQ from software, measure its latency

The NVIC can make an IRQ pending without the device:
`NVIC->ISPR[n / 32] = 1UL << (n % 32)`. If the IRQ is enabled and
urgent enough, its handler runs right away.

1. Use `TIMER1_IRQn` with the timer itself stopped. Enable it in
   `NVIC->ISER`. Its handler needs no device acknowledge here: the
   device line is low, so the pending bit clears on entry.
2. In `main()`, before the drop: `before = SysTick->VAL;` then the
   `ISPR` write. In `TIMER1_Handler`, first line: `entry = SysTick->VAL;`.
3. Print `before - entry` (SysTick counts *down*). Repeat five times.

**Checkpoint:** the handler runs each time, and the latency prints as a
small number of SysTick counts. The first one is larger than the rest.
The numbers come in coarse steps. You can say why: QEMU is not
cycle-accurate. It translates code the first time it runs, and its
clock moves in steps. On a real Cortex-M3 the entry takes 12 cycles.

### Done looks like — Part B

- You have shown a lost update and fixed it with `irq_save` /
  `irq_restore`, and you can say why the experiment must run privileged.
- `uart1.txt` holds what you sent on UART1 with the same driver code.
- The watchdog resets an unfed system, after an NMI, and stays quiet
  when `SysTick_Handler` feeds it.
- You can read and write the LEDs from gdb and from `sys_led`, and you
  can name the four places a new system call touches.
- You can decode a `-d unimp` GPIO line and explain the difference
  between `DATAOUT |= bit` and `OUTENABLESET = bit`.
- (Stretch) You have read the latency of a software-pended interrupt.

---

## Where this goes next

Week 4's lecture (`lecture-notes/week-04.md`) treats memory management:
how the RAM is divided, here by fixed partitions defined in the linker
script. This laboratory has prepared two things it uses.

- **Where your data lives.** The ring buffers and counters you wrote are
  static arrays and variables: the linker places them in `.bss`, in RAM,
  at addresses fixed when the image is linked. `arm-none-eabi-nm -n
  build/scratch-os.axf` lists them; `rx_buf` of A2 is one. Week 4 decides
  the same kind of placement by hand, for whole regions.
- **Addresses that are not memory.** The registers of B4 and B5 sit at
  `0x40028000` and `0x40010000`, outside RAM and flash: a load or store
  there reaches a device. B5's `-d unimp` log shows what an access to
  an unmodelled device looks like from the bus. A memory-management
  scheme has to tell the two kinds of address apart, which is the
  distinction `include/cmsdk.h` already draws.
