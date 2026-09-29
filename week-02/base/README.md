# week-02/base

## Starting point

This code is derived from `week-01/`. In week 1, `boot/startup.c` was
written in C, and it already did the setup a C runtime normally does
for you (copying `.data`, clearing `.bss`, calling `main()`). Here you
throw that away and build the same boot code yourself, in assembly,
one piece at a time, in `boot/startup.s`.

## What changed and why

- **`boot/startup.s`** replaces `boot/startup.c`. It is now hand-written
  assembly instead of C. Right now it holds only the two words the CPU
  absolutely needs at reset - the initial stack pointer value and the
  address of `Reset_Handler` - and a `Reset_Handler` that does nothing
  but call `main()` and then loop forever if `main()` ever returns. All
  the setup work that week 1's C version did for you (copying `.data`,
  clearing `.bss`, and more) is missing on purpose: you write it
  yourself, in class.

- **`src/main.c`** now prints two lines instead of one: a string
  literal (which lives in FLASH, in `.rodata`) and a string stored in a
  writable global array, `data_message` (which lives in RAM, in
  `.data`). Try building and running this code as it is: the first line
  prints correctly, but the second one does not show up as expected,
  because nothing has copied `.data`'s initial value from FLASH to RAM
  yet. That is exactly the gap you will close in class.

- **`Makefile`** now also has a `make debug` target, for stepping
  through the assembly you write with a debugger.

- **`scripts/mps2_m3.ld`** (the linker script) is unchanged from week 1.
  It already defines the addresses (`_estack`, `_sidata`, `_sdata`,
  `_edata`, `_sbss`, `_ebss`) that `boot/startup.s` will need.

## What you write on top of it in class

The files here are kept short on purpose. Each spot where class code goes is marked with a comment that starts with `CLASS:`, and the slides tell you which marker to replace with which snippet. For the full line-by-line explanations, open the same file in `../solution/`.

In this order:

1. Set the stack pointer yourself.
2. Copy `.data` from FLASH to RAM (the second line now prints).
3. Clear `.bss`.
4. Fill in the rest of the vector table.
5. Set the vector table address (VTOR).
6. Enable separate fault handlers (SHCSR).
7. Trap divide by zero (CCR).
8. Set exception priorities.
9. Mask interrupts during setup.
10. Write fault handlers in `main.c`, and see a divide by zero caught.
11. Write `SVC_Handler`, drop to unprivileged mode, call `svc`, and see
    a privileged-only access fault.

## Build, run, debug

Build the image:

```
make
```

Run it under QEMU:

```
make run
```

Debug it: start QEMU paused for a debugger,

```
make debug
```

then, from a second terminal, attach with GDB:

```
arm-none-eabi-gdb build/scratch-os.axf
```

(on Linux, the package is usually called `gdb-multiarch` instead), and
inside GDB:

```
target remote :1234
```

The full, finished solution is in the sibling folder, `../solution/`
(`week-02/solution/`).
