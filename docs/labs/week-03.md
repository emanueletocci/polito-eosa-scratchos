# Week 3 lab — boot and CPU configuration, solo

Covers week 2's topic (the boot sequence), per the lab-lag rule — this
lab happens during week 3, before that week's lecture content.

## Starting point

```sh
git checkout week-02-end
```

## Task

Working alone:

1. Rebuild and reboot `week-02-end`, confirm it still prints correctly.
2. Read `boot/startup.c` without looking at your notes from class. Before
   touching anything, write down (a comment at the top of the file is
   fine) what each of these does, in your own words:
   - Why does `Reset_Handler` need to be `naked`?
   - What would happen if `Reset_Handler_C` ran *before* SP were valid?
   - Why does `.bss` get zeroed but `.data` gets copied from FLASH?
3. Add one more entry to `Default_Handler`'s job: instead of an empty
   infinite loop, have it write a fixed semihosting message (e.g.
   `"fault\n"`) before looping, using the same
   `semihost_write0`-style call from `src/main.c`. This isn't a real
   fault handler yet (no register dump, no fault classification — that's
   well beyond this course) — just enough that an unhandled exception
   becomes *visible* instead of silent.
4. Deliberately trigger it: temporarily point `isr_vector[]`'s
   `Reset_Handler` entry at `Default_Handler` instead, rebuild, confirm
   you now see your fault message instead of the normal output. Then
   revert.

## Done looks like

- Your written answers to the three "why" questions above.
- `Default_Handler` prints a message before looping.
- You've seen it actually fire (step 4) and reverted the change.
