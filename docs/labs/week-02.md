# Week 2 lab — toolchain, solo

Covers week 1's topic (the toolchain pipeline), per the lab-lag rule —
this lab happens during week 2, before that week's lecture content.

## Starting point

```sh
cd week-01
```

## Task

Working alone (ask for help if you're stuck — that's what this session
is for):

1. Build and boot `week-01/` exactly as shown in class:
   ```sh
   make run
   ```
   Confirm you see `toolchain OK` printed, then exit QEMU (`Ctrl-A`, `X`).
2. Change the message in `src/main.c` to something of your own choosing,
   rebuild, and confirm your new message prints instead.
3. Deliberately break the build once, on purpose, then fix it — pick one:
   - Remove a semicolon or brace from `src/main.c` and read the compiler
     error.
   - Point the `Makefile`'s `-T` flag at a linker script that doesn't
     exist and read the linker error.
   
   The point isn't the bug — it's getting comfortable reading
   cross-compiler and linker error messages before you need to debug a
   real one.

## Done looks like

- Your own message prints correctly when you run `make run`.
- You can describe, in your own words, what each of the three pipeline
  stages (compile, link, run-under-QEMU) is doing.
