# week-03/base

## Starting point

This folder is where you start in class. It is a small operating system
with the hard-to-type parts already in place and the interesting parts
left empty. You fill the empty parts in live, following the slides. The
finished code is in `../solution/`, with full line-by-line comments.

## What is given, and why

Class time should go to this week's topics, so the following is ready:

- **`Makefile`** builds every source folder and has three targets:
  `run`, `debug` and `run-trace`.
- **`scripts/mps2_m3.ld`** is the linker script. It places the code in
  FLASH and the data, `.bss` and stack in RAM.
- **`include/cmsdk.h`** is the complete register map of the board:
  UART, SysTick, NVIC, SCB, timers, watchdog, GPIO, FPGAIO and the
  interrupt numbers. You read and use it; you do not type it.
- **`boot/startup.s`** has the full CPU setup (stack, `.data`, `.bss`,
  vector table address, fault handlers, priorities) and the complete
  vector table: the 16 core slots and the 32 device-interrupt slots,
  with their weak default handlers. You read it; you do not type it.
- **API headers** (`drivers/uart.h`, `drivers/systick.h`,
  `kernel/irq.h`, `kernel/kprintf.h`, `kernel/syscall.h`,
  `lib/format.h`, `lib/printf.h`, `lib/syscalls.h`, `src/app.h`) say what
  each part will offer, so the files can call each other from the start.
- **Number helpers** in `lib/format.c` turn numbers into text. The `%`
  dispatch is yours to write.
- **Silent fault handlers** in `kernel/faults.c` are named loops that
  gdb can show. You make them print.
- **`src/main.c`** runs privileged, does nothing, and prints nothing.
  The first visible output is the one you write.

## Folder by folder

| Folder | Given | You write |
|---|---|---|
| `boot/` | `startup.s`: CPU setup, 16 core slots, 32 device-interrupt slots and their weak default handlers | nothing: you read it |
| `kernel/` | `irq.h`, `kprintf.h`, `syscall.h`; silent `faults.c`; marker files `irq.s`, `svc_entry.s`, `syscall.c`, `kprintf.c` | `irq_save`/`irq_restore`, `SVC_Handler`, the system-call table and services, `kprintf`, the fault messages |
| `drivers/` | `uart.h`, `systick.h`; marker files `uart.c`, `systick.c` | the UART driver (polled, then ring buffer and interrupt) and the SysTick tick |
| `lib/` | headers; number helpers in `format.c`; marker files `printf.c`, `syscalls.s` | `vformat`, the `svc` stubs, `printf`, `sleep_ms` |
| `include/` | `cmsdk.h`, complete | nothing |
| `src/` | `main.c` (empty privileged loop), `app.h`, marker file `app.c` | kernel start-up in `main.c`, the application in `app.c` |
| `scripts/` | `mps2_m3.ld` | nothing |

A marker file is a nearly empty file with a comment that starts with
`CLASS:`. The slides tell you which marker to replace with which code.

## What you will write in class

Unit 1:

1. Read the board's memory map from the QEMU monitor.
2. Touch a device QEMU does not model and watch it report the access.
3. See the compiler remove a hardware read, then fix it with `volatile`.
4. Send the first byte through the UART.
5. Echo received bytes by polling.
6. Write `vformat` and `kprintf` for formatted kernel output.
7. Make the fault handlers print their names.

Unit 2:

1. Write `kernel/irq.s`, your first assembly functions called from C.
2. Watch `r0` cross a call in gdb.
3. Write the SysTick driver: a 10 ms tick and an uptime counter.
4. Print the uptime every second.
5. Catch an exception frame in gdb.
6. Read the 32 device-interrupt slots in the vector table.
7. Write the UART transmit ring buffer, driven by interrupts.
8. Send 160 bytes through a 64-byte buffer.

Unit 3:

1. Write `SVC_Handler` in `kernel/svc_entry.s`.
2. Write the system-call table and services in `kernel/syscall.c`.
3. Write the user stubs in `lib/syscalls.s` and make your first system
   call.
4. Find the service number in gdb.
5. Write `printf` and `sleep_ms` in `lib/printf.c`.
6. Write the application in `src/app.c` and the kernel start in
   `src/main.c`.
7. Check in gdb that the application runs unprivileged.
8. Add a debug print inside the kernel and see why the kernel keeps
   its own `kprintf`.
9. Make the kernel wait for buffer space and see why it must never
   wait inside a system call.

## Build, run, debug, run-trace

Build the image:

```
make
```

Run it under QEMU (quit with `Ctrl-A` then `x`):

```
make run
```

Run it and log every access to a device QEMU does not implement:

```
make run-trace
```

Debug it. Start QEMU paused, waiting for a debugger:

```
make debug
```

Then, from a second terminal, attach GDB:

```
arm-none-eabi-gdb build/scratch-os.axf
```

(on Linux the package is usually called `gdb-multiarch`), and inside
GDB:

```
target remote :1234
```

Remove the generated files with `make clean`.

The full, finished solution is in the sibling folder, `../solution/`.
