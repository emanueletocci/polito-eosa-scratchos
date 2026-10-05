# Week 4 laboratory — worked solutions

Solutions to every exercise are provided here. You are expected to
attempt an exercise before consulting its solution: the values an
exercise asks you to observe are the substance of the work, and reading
an observed value does not substitute for taking one.

Task document: [`README.md`](README.md). Finished code of the exercises
whose result you keep: [`solution/`](solution/). It holds A2, A3, B2,
B3, B4, B5 and B6 together. A4, B1 and B7 are experiments you make in a
copy; each is given below as the change to apply to `solution/`.

Every value, output and listing below was produced by executing the
stated command using the course toolchain (Arm GNU Toolchain 15.3,
QEMU 11.1): against `solution/`, or, for an experiment, against
`solution/` with the stated change applied. Each observed value is also
classified by what fixes it:

- **architectural** — the ARMv7-M architecture defines it; any
  Cortex-M3 produces it;
- **this build** — it depends on the code the compiler generated or on
  how fast the emulator runs, so your value may differ; the relation the
  exercise asks about holds regardless.

---

## Part A

### A1. A known-good baseline

`make run` in the copy of `week-03/solution/` prints the lines the
checkpoint lists, then one `uptime` line a second. `Ctrl-C` ends it: the
`Makefile` starts QEMU with `-monitor null -serial stdio`, which gives
the serial line no escape sequence of its own, so `Ctrl-A` `X` reaches
the program as two typed characters (it prints `key: x`).

Fix any difference now: every later exercise assumes this baseline, and
a defect inherited from here looks like a defect in the new code.

### A2. Interrupt-driven UART receive

The receive buffer has the shape of the send buffer, with the roles of
the two indices exchanged: the handler writes `rx_head`, the reader
writes `rx_tail`.

```c
#define RX_BUF_SIZE 32
static char rx_buf[RX_BUF_SIZE];
static volatile unsigned rx_head;
static volatile unsigned rx_tail;

void UART0RX_Handler( void )
{
    char c = (char) ( CMSDK_UART0->DATA & 0xFF );

    unsigned next = ( rx_head + 1 ) & ( RX_BUF_SIZE - 1 );
    if ( next != rx_tail )
    {
        rx_buf[rx_head] = c;
        rx_head = next;
    }

    CMSDK_UART0->INTSTATUS = UART_INT_RX;
}

int uart_getc( void )
{
    int c = -1;
    uint32_t saved = irq_save();
    if ( rx_head != rx_tail )
    {
        c = (unsigned char) rx_buf[rx_tail];
        rx_tail = ( rx_tail + 1 ) & ( RX_BUF_SIZE - 1 );
    }
    irq_restore( saved );
    return c;
}
```

`uart_init` adds `UART_CTRL_RXIRQEN` to `CTRL`, sets
`NVIC->IPR[UART0RX_IRQn] = 0xC0`, then sets the bit in `NVIC->ISER`.
The priority is written before the enable, so the interrupt can never
be taken at a priority nobody chose. `sys_read_impl` returns
`uart_getc()`.

`uart_getc` blocks interrupts while it moves `rx_tail`, because the
handler reads `rx_tail` to decide whether the buffer is full: without
the protection, an interrupt between the comparison and the update would
act on a stale index.

Typing ten keys one after another gives ten `key:` lines, and the
`uptime` lines continue. With `break *UART0RX_Handler` and `continue` in
gdb, the first key typed in the QEMU terminal stops the program at
`Breakpoint 1, UART0RX_Handler`. Type the keys as you normally would. A whole string written to QEMU's
input in a single burst printed only its first key here until further
input arrived; that is a property of how the emulator is fed, observed
with this driver, and it does not appear when keys are typed.

### A3. A timer driver

```c
void timer0_init( uint32_t period_ms )
{
    CMSDK_TIMER0->RELOAD = 25000UL * period_ms;
    CMSDK_TIMER0->CTRL = TIMER_CTRL_EN | TIMER_CTRL_IRQEN;
    NVIC->IPR[TIMER0_IRQn] = 0xC0;
    NVIC->ISER[TIMER0_IRQn / 32] = 1UL << ( TIMER0_IRQn % 32 );
}

void TIMER0_Handler( void )
{
    CMSDK_TIMER0->INTSTATUS = TIMER_INT_PENDING;
    ticks++;
    if ( ticks % 10 == 0 )
    {
        kprintf( "timer0: %u\n", ticks );
    }
}
```

The timer counts down at the 25 MHz processor clock, so 25 000 cycles
make a millisecond, and a 100 ms period gives ten ticks a second. The
output is:

```
timer0: 10
uptime: 1000 ms
timer0: 20
uptime: 2000 ms
```

With the acknowledge line removed, the handler runs again as soon as it
returns: the NVIC line is level-sensitive and the device's flag is still
set. The handler's priority, `0xC0`, is more urgent than SysTick's,
`0xE0`, so SysTick is never taken again: the clock stops, and with it
the `uptime` lines. Over 8 seconds the handler printed more than a
hundred `timer0:` lines and no `uptime` line.

### A4. Preemption

In a copy of your working folder. The changes to `drivers/timer.c`: the
priority of `TIMER0` becomes `0x40`, its handler reports whether the
slow handler is running, and `TIMER1` is added.

```c
static volatile int in_slow;

void TIMER0_Handler( void )
{
    CMSDK_TIMER0->INTSTATUS = TIMER_INT_PENDING;
    if ( in_slow )
    {
        kprintf( "  T0 (inside T1)\n" );
    }
    else
    {
        kprintf( "T0\n" );
    }
}

void timer1_init( void )
{
    CMSDK_TIMER1->RELOAD = 25000000UL;          /* 1 s */
    CMSDK_TIMER1->CTRL = TIMER_CTRL_EN | TIMER_CTRL_IRQEN;
    NVIC->IPR[TIMER1_IRQn] = 0xE0;
    NVIC->ISER[TIMER1_IRQn / 32] = 1UL << ( TIMER1_IRQn % 32 );
}

void TIMER1_Handler( void )
{
    CMSDK_TIMER1->INTSTATUS = TIMER_INT_PENDING;
    in_slow = 1;
    kprintf( "T1 start\n" );
    for ( volatile uint32_t i = 0; i < 100000000; i++ )
    {
    }
    kprintf( "T1 end\n" );
    in_slow = 0;
}
```

`timer1_init()` is called from `main()` after `timer0_init( 100 )`. With
the loop count above, each `T1` took about 300 ms here (this build and
this machine; adjust the count). The output:

```
T1 start
  T0 (inside T1)
  T0 (inside T1)
  T0 (inside T1)
T1 end
uptime: 1010 ms
```

`TIMER0` fires every 100 ms and is more urgent than `TIMER1`, so it
preempts the handler three times. No `uptime` line is printed between
`T1 start` and `T1 end`: the application runs below every handler. The
uptime count itself stops advancing for the same reason: SysTick has
the priority of `TIMER1`, `0xE0`, and an exception of equal priority
does not preempt a running handler, so SysTick waits, pending, until
`T1` returns.

In gdb, after `break *TIMER1_Handler` and `continue`, the low nine bits
of xPSR are the number of the active exception and `lr` holds
`EXC_RETURN`. Both are architectural:

```
(gdb) p/x $xpsr & 0x1ff
$1 = 0x19
(gdb) p/x $lr
$2 = 0xfffffff9
```

`0x19` is 25: 16 plus `TIMER1_IRQn`, 9. `0xfffffff9` returns to thread
mode on the main stack: `TIMER1_Handler` was entered from `sleep_ms`.
After `delete`, `break *TIMER0_Handler`, `continue`:

```
(gdb) p/x $xpsr & 0x1ff
$3 = 0x18
(gdb) p/x $lr
$4 = 0xfffffff1
(gdb) bt
#0  TIMER0_Handler () at drivers/timer.c:34
#1  <signal handler called>
#2  0x000004b2 in TIMER1_Handler () at drivers/timer.c:59
#3  <signal handler called>
#4  sleep_ms (ms=10) at lib/printf.c:93
```

`0x18` is 24, `16 + 8`. `0xfffffff1` returns to **handler** mode: the
interrupted code is `TIMER1_Handler` itself, and `bt` shows it below.
The addresses and line numbers are **this build**.

Swapping the two priorities, `TIMER0` at `0xE0` and `TIMER1` at `0x40`:
`TIMER0` is no longer more urgent, so it waits until `TIMER1_Handler`
returns, and no `T0` appears inside `T1`. SysTick, at `0xE0`, is less
urgent than `TIMER1`, so it waits too, and `uptime` stalls for as long
as `T1` runs.

---

## Part B

### B1. A race, and its fix

In a copy. `TIMER0` gets a 1 ms period (`timer0_init( 1 )`), its
handler adds `counter++` and `isr_hits++` after the acknowledge, and
`main()` runs, before the privilege drop:

```c
for ( int i = 0; i < 100000; i++ )
{
    uint32_t tmp = counter;
    for ( volatile int j = 0; j < 100; j++ )
    {
    }
    counter = tmp + 1;
}
CMSDK_TIMER0->CTRL = 0;
kprintf( "counter=%u expected=%u diff=%d\n", counter, 100000 + isr_hits,
         (int) ( 100000 + isr_hits - counter ) );
```

Without the protection the difference is not zero:
`counter=100001 expected=100025 diff=24` in one run, `diff=28` in
another. Each lost update is an interrupt that fell between the load
into `tmp` and the store of `tmp + 1`: the store overwrote the
handler's increment. The count varies from run to run (**this build**).
The delay loop widens the gap; without it QEMU, which translates
instructions in groups and checks for interrupts between groups, would
rarely show a loss.

With `uint32_t s = irq_save();` before `tmp = counter` and
`irq_restore( s );` after the store, every run prints `diff=0`.
`irq_save` returns the previous mask and `irq_restore` puts it back, so
the pair is correct also where interrupts were already masked, which a
plain `cpsid i` / `cpsie i` is not: `cpsie i` would enable them
unconditionally. The experiment runs privileged because `cpsid i` has
no effect in unprivileged code.

### B2. One driver, two UARTs

```c
void uart_putc_polled_on( CMSDK_UART_TypeDef *u, char c )
{
    while ( u->STATE & UART_STATE_TXBF )
    {
    }
    u->DATA = (uint32_t) (unsigned char) c;
}

void uart_putc_polled( char c )
{
    uart_putc_polled_on( CMSDK_UART0, c );
}

void uart_init_on( CMSDK_UART_TypeDef *u )
{
    u->BAUDDIV = 16;
    u->CTRL = UART_CTRL_TXEN | UART_CTRL_RXEN;
}
```

`main()` calls `uart_init_on( CMSDK_UART1 )` and sends `hello uart1\n`
one byte at a time with `uart_putc_polled_on`, before the privilege
drop. Run with the second serial option, and after quitting:

```
$ cat uart1.txt
hello uart1
```

### B3. The watchdog

```c
void wdog_init( void )
{
    CMSDK_WDOG->LOCK = WDOG_UNLOCK_KEY;
    CMSDK_WDOG->LOAD = 25000000UL;
    CMSDK_WDOG->CTRL = WDOG_CTRL_INTEN | WDOG_CTRL_RESEN;
    CMSDK_WDOG->LOCK = 0;
}

void wdog_feed( void )
{
    CMSDK_WDOG->LOCK = WDOG_UNLOCK_KEY;
    CMSDK_WDOG->INTCLR = 1;
    CMSDK_WDOG->LOCK = 0;
}

void NMI_Handler( void )
{
    kprintf( "watchdog: NMI\n" );
}
```

`wdog_init` is called from `main()` before the drop and `wdog_feed`
from `SysTick_Handler`. `NMI_Handler` replaces the weak default of the
vector table. The `LOCK` register ignores writes to the others while
locked: the key unlocks, any other value locks again.

`solution/` feeds the watchdog. Not feeding it (remove the call from
`SysTick_Handler`) gives, in the order printed:

```
scratch-os: kernel started
dropping to unprivileged mode
app: hello from unprivileged code
watchdog: NMI
timer0: 10
uptime: 1000 ms
scratch-os: kernel started
```

The counter reaches 0 after one second and raises NMI; it reloads and
starts again; the second time nobody wrote `INTCLR` before it reached 0,
so `RESEN` resets the board and the banner prints again. The cycle
repeats. Fed every 10 ms, the watchdog never expires and no NMI
appears. A `for ( ;; ) { }` in `SysTick_Handler` stops the feeding, so
the watchdog catches the stuck system, which is its purpose: the check
lives in hardware, outside the code that may hang.

### B4. Board LEDs

`FPGAIO->LED = 0x3;` in `main()`, before the drop. In gdb, stopped by
`break *SysTick_Handler` and `continue`:

```
(gdb) x/wx 0x40028000
0x40028000:	0x00000003
(gdb) p/x FPGAIO->LED
$1 = 0x3
(gdb) set var FPGAIO->LED = 0xff
(gdb) p/x FPGAIO->LED
$2 = 0x3
```

Both reads show `0x3`. The write of `0xff` leaves `0x3`: the register
has two bits, so only they are stored. You stopped in a handler first
because the CPU stopped in `app_main` is unprivileged, and the debugger's
access through the processor can fail there.

### B5. GPIO seen from the bus

```c
void gpio_set_output( unsigned pin )
{
    CMSDK_GPIO0->OUTENABLESET = 1UL << pin;
}

void gpio_write( unsigned pin, int level )
{
    if ( level )
    {
        CMSDK_GPIO0->DATAOUT |= 1UL << pin;
    }
    else
    {
        CMSDK_GPIO0->DATAOUT &= ~( 1UL << pin );
    }
}
```

`make run-trace` prints, right after `scratch-os: kernel started`:

```
cmsdk-ahb-gpio: unimplemented device write (size 4, offset 0x010, value 0x00000001)
cmsdk-ahb-gpio: unimplemented device read  (size 4, offset 0x004)
cmsdk-ahb-gpio: unimplemented device write (size 4, offset 0x004, value 0x00000001)
```

Line one is `OUTENABLESET = 1 << 0`: offset `0x010`, a write, one
access: the set register changes only its own bit. Lines two and three
are `DATAOUT |= 1`: offset `0x004`, a read, then a write of the result.
After adding `gpio_write( 1, 1 )`, the second `DATAOUT` write has value
`0x00000002`, not `0x00000003`: the read returned 0, so the OR started
from 0 and pin 0 was turned off again.

Reads return 0 because QEMU has no model for this block: it holds only
a placeholder that logs the access and answers 0. A model that kept the
latch would return it, and the result would be `0x3`. Set/clear
registers avoid the problem altogether: one write changes one bit, so
nothing is read and nothing can be lost, including when an interrupt
changes the same register between a read and a write.

### B6. Add a system call

The four places, in order:

```c
/* lib/syscalls.h */
#define SYS_LED 4
int sys_led( uint32_t bits );
```

```asm
/* lib/syscalls.s */
    .global sys_led
    .type sys_led, %function
    .thumb_func
sys_led:
    svc #4
    bx lr
    .size sys_led, . - sys_led
```

```c
/* kernel/syscall.c */
static uint32_t sys_led_impl( uint32_t a0, uint32_t a1, uint32_t a2 )
{
    (void) a1;
    (void) a2;
    FPGAIO->LED = a0 & 0x3;
    return 0;
}
/* ... and in table: */
    [SYS_LED]    = sys_led_impl,
```

`src/app.c` calls `sys_led( ( now / 1000 ) % 2 ? 1 : 2 )` once a second.
The number in the stub is a plain 4 because the assembler cannot read
the C header; the three numbers in use are 1, 2 and 3, so 4 is the next.
In gdb, with a breakpoint on `SysTick_Handler` taken when the uptime is
1500 and again when it is 2500, `p/x FPGAIO->LED` gives `0x1` then
`0x2`: alternate seconds.

With `svc #5` by mistake, the number is past the end of the table, so
`svc_dispatch` stores -1 in the result: `p/d (int) $r0` at the `bx lr`
of `sys_led` gives `-1` where the correct stub gives `0`. The LEDs stay
at `0x3`, the value `main()` wrote, and never change.

### B7. (Stretch) Pend an IRQ from software, measure its latency

In a copy. `TIMER1` is left stopped, its priority set to `0x40` and
enabled in `NVIC->ISER`; `main()`, before the drop, repeats five times:

```c
uint32_t before = SysTick->VAL;
NVIC->ISPR[TIMER1_IRQn / 32] = 1UL << ( TIMER1_IRQn % 32 );
kprintf( "latency %u\n", before - entry );
```

and `TIMER1_Handler`'s first line is `entry = SysTick->VAL;`, with
`entry` a `volatile uint32_t`. The handler needs no acknowledge: the
device line is low, so the pending bit clears on entry. SysTick counts
down, hence `before - entry`. One run printed 650, 75, 25, 25, 25
(**this build**; the numbers change from run to run, the pattern does
not). The first is larger because QEMU translates the code the first
time it runs; the rest come in coarse steps because the emulated clock
moves in steps, not per instruction. A real Cortex-M3 takes 12 cycles
from the pend to the first instruction of the handler.
