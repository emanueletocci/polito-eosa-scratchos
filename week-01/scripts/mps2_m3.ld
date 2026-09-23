/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * WHAT THIS FILE IS AND WHY IT EXISTS (read this before the rest)
 * -----------------------------------------------------------------
 * When you compile a .c file, GCC produces a .o object file whose code
 * and data don't yet have final memory addresses - it's up to the
 * *linker* to merge all the .o files into one image and decide where
 * everything goes. On a desktop OS you never see this: the OS loader
 * picks addresses for you at run time. On bare metal there is no OS,
 * so the addresses must be nailed down once, at link time, matching
 * the real hardware exactly - get it wrong and the CPU fetches its
 * first instruction from the wrong place, or the stack overwrites your
 * code, and nothing runs. A linker script is the file that tells the
 * linker (ld, invoked here through gcc via `-T`, see the Makefile)
 * those addresses: which physical memories exist on this board, and
 * which compiled "section" (a named chunk of code or data - .text for
 * code, .data for initialized variables, .bss for zero-initialized
 * ones, and so on - these names come from the compiler, not from us)
 * goes into which memory, at which address.
 *
 * This particular script targets QEMU's mps2-an385 machine (an
 * emulated Cortex-M3 board, MPS2 platform) - the -machine option the
 * Makefile's `run` target passes to qemu-system-arm. Its FLASH/RAM
 * addresses and sizes below match exactly what that emulated board
 * reports, which is why they look arbitrary but must not be changed.
 *
 * Deliberately minimal for week 1: one stack, no per-task partitions
 * (that's week 4's topic), no heap (nothing here allocates dynamically -
 * see three-pillar-curriculum's static-memory scratch-OS scope).
 */

/* Every real CPU memory address on this board lives in one of two
 * regions: nonvolatile FLASH, which keeps its contents with no power
 * and is where code and constants are stored so they survive a reset,
 * and volatile RAM, which is fast to read/write but forgets everything
 * when power is lost - which is why variables that change at run time
 * must live there instead. (rx)/(rwx) list what each region allows:
 * read/execute for FLASH (code runs directly from it - no separate
 * "load into RAM first" step needed on this simple platform), and
 * read/write/execute for RAM. The linker uses these bounds to place
 * sections and to catch a build that doesn't fit the real hardware. */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 4M  /* code + read-only data, execute-in-place */
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 4M  /* data, bss, stack */
}

/* The image's entry point - the symbol whose address the debugger and
 * QEMU use as the start of execution (also entry 1 of the vector table,
 * defined in boot/startup.c - read that file next). This is metadata
 * for the tools, not an instruction that runs on the CPU. */
ENTRY(Reset_Handler)

/* These three lines just do arithmetic on addresses and store the
 * results under names - nothing is allocated or written yet. Each name
 * (_estack, _Min_Stack_Size, ...) becomes a linker-defined symbol: an
 * address that C code can read via `extern uint32_t _estack;` (see
 * boot/startup.c) even though no such variable was ever declared in C.
 * ORIGIN(RAM)/LENGTH(RAM) refer back to the MEMORY block above. */
_estack = ORIGIN(RAM) + LENGTH(RAM);   /* initial SP = top of RAM; the stack grows downward from here */
_Min_Stack_Size = 0x1000;              /* 4 KB reserved for the stack - checked against the heap below */
_Min_Heap_Size = 0x1000;               /* 4 KB reserved for the heap - only newlib's stdio buffering would touch it */

/* SECTIONS is the heart of the script: it walks through the sections
 * every input .o file was compiled with and says, one by one, where
 * each one is placed in the final image and which symbols mark its
 * boundaries. `.` (the "location counter") is the linker's current
 * output address - it advances automatically as sections are placed,
 * and you can read or force-align it explicitly, as the ALIGN() calls
 * below do. */
SECTIONS
{
    /* Vector table first, at address 0, so the CPU finds the initial SP
     * and reset vector where the architecture requires them. */
    .isr_vector :
    {
        . = ALIGN(4);            /* vector table must be word-aligned */
        KEEP(*(.isr_vector))     /* KEEP: never garbage-collect it, even though nothing "calls" it */
        . = ALIGN(4);            /* keep the location counter aligned for what follows */
    } > FLASH                    /* the vector table lives at the base of FLASH */

    /* Executable code and read-only data. */
    .text :
    {
        . = ALIGN(4);           /* word-align the start of this section */
        *(.text*)               /* all code from every input object */
        *(.rodata*)             /* string literals, const tables */
        . = ALIGN(4);           /* word-align the end so _etext is aligned */
        _etext = .;             /* end-of-text marker (unused here, handy when debugging) */
    } > FLASH                   /* place this section in the FLASH region */

    _sidata = LOADADDR(.data);  /* FLASH address where .data's initial image is stored */

    /* Initialized variables (e.g. `int x = 5;`) are the one case where
     * a section needs two different addresses at once, which is why
     * this rule ends in `> RAM AT > FLASH` instead of a single `> RAM`
     * or `> FLASH`: the variables must be read/writable at *run* time,
     * so they need RAM addresses (`> RAM`) - but their initial values
     * (5, in the example) must still be stored in FLASH so they aren't
     * lost when power is off (`AT > FLASH`, the *load* address). Since
     * FLASH is only readable, nothing can write `x`'s RAM copy for us
     * before main() runs except our own code - that's exactly the copy
     * loop in Reset_Handler (boot/startup.c), which uses _sidata,
     * _sdata and _edata below to copy FLASH -> RAM one word at a time,
     * very early, before main() can touch any initialized variable. */
    .data :
    {
        . = ALIGN(4);           /* word-align the start of .data */
        _sdata = .;             /* start of .data in RAM - Reset_Handler copies from here */
        *(.data*)               /* every initialized-variable input section */
        . = ALIGN(4);           /* word-align the end */
        _edata = .;             /* end of .data in RAM - copy loop stops here */
    } > RAM AT > FLASH          /* run address in RAM, load address in FLASH */

    /* Zero-initialized variables (e.g. `int y;` or `static int y;`) are
     * simpler: the C standard says they must start at 0, so there is no
     * value worth storing in FLASH for them - the image would just be
     * bigger for no reason. Instead this section only reserves RAM
     * space (no `AT >` here, and no data for it in the .axf image);
     * Reset_Handler is responsible for actually writing the zeros,
     * using _sbss/_ebss below, before main() runs. */
    .bss :
    {
        . = ALIGN(4);           /* word-align the start of .bss */
        _sbss = .;              /* start of .bss - Reset_Handler zeroes from here */
        *(.bss*)                /* every zero-initialized input section */
        *(COMMON)               /* legacy tentative-definition symbols */
        . = ALIGN(4);           /* word-align the end */
        _ebss = .;              /* end of .bss - zero loop stops here */
    } > RAM

    /* Heap region: reserved so the overflow check below is meaningful,
     * even though week 1's code never calls malloc. */
    .heap :
    {
        . = ALIGN(8);          /* malloc alignment on this ABI is 8 */
        PROVIDE( end = . );     /* `end`/`_end`: the symbol newlib's sbrk uses as the heap base */
        PROVIDE( _end = . );    /* same address under the alternate spelling */
        _heap_bottom = .;      /* record the heap's low address */
        . = . + _Min_Heap_Size; /* advance the location counter to reserve the heap span */
        _heap_top = .;         /* record the heap's high address (checked below) */
    } > RAM                    /* the heap lives in RAM */

    __StackLimit = _estack - _Min_Stack_Size;  /* lowest address the stack may grow to */
    /* Link-time guard: fail the build if the reserved heap and stack
     * would overlap, rather than corrupting memory at runtime. */
    ASSERT(__StackLimit >= _heap_top, "region RAM overflowed: heap collides with the stack")
}
