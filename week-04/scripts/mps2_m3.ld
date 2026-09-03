/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Linker script for QEMU's mps2-an385 machine (ARM Cortex-M3, MPS2
 * platform). Memory layout matches the board's actual FLASH/RAM
 * addresses and sizes.
 *
 * WEEK 4 added .task_partitions: TASK_COUNT fixed-size regions, one per
 * task, their addresses fixed at link time - not dynamically allocated
 * (see three-pillar-curriculum's static-memory scratch-OS scope; no
 * heap exists in this image at all, on purpose).
 */

/* Physical memory regions of the board, with their access attributes. */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 4M  /* code + read-only data */
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 4M  /* data, bss, partitions, stack */
}

/* Image entry point - also entry 1 of the vector table. */
ENTRY(Reset_Handler)

_estack = ORIGIN(RAM) + LENGTH(RAM);   /* initial SP = top of RAM; the stack grows downward from here */
_Min_Stack_Size = 0x1000;              /* 4 KB for the boot stack (a task's own stack is week 5's topic) */

TASK_COUNT = 3;                        /* number of task partitions to carve out */
TASK_PARTITION_SIZE = 512;             /* bytes reserved per task, fixed at link time */

SECTIONS
{
    /* Vector table first, at address 0. */
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
        _etext = .;             /* end-of-text marker */
    } > FLASH                   /* place this section in the FLASH region */

    _sidata = LOADADDR(.data);  /* FLASH address where .data's initial image is stored */

    /* Initialized variables: run in RAM, initial image copied from FLASH. */
    .data :
    {
        . = ALIGN(4);           /* word-align the start of .data */
        _sdata = .;             /* start of .data in RAM - Reset_Handler copies from here */
        *(.data*)               /* every initialized-variable input section */
        . = ALIGN(4);           /* word-align the end */
        _edata = .;             /* end of .data in RAM - copy loop stops here */
    } > RAM AT > FLASH          /* run address in RAM, load address in FLASH */

    /* Zero-initialized variables: reserve RAM, no stored image. */
    .bss :
    {
        . = ALIGN(4);           /* word-align the start of .bss */
        _sbss = .;              /* start of .bss - Reset_Handler zeroes from here */
        *(.bss*)                /* every zero-initialized input section */
        *(COMMON)               /* legacy tentative-definition symbols */
        . = ALIGN(4);           /* word-align the end */
        _ebss = .;              /* end of .bss - zero loop stops here */
    } > RAM

    /* One fixed-size, fixed-address region per task. Named per-task
     * symbols (_task0_partition_start/_end, _task1_..., _task2_...)
     * rather than a single array, so each task's boundary is visible
     * and checkable on its own - not just "somewhere in a shared
     * region." NOLOAD: reserved RAM, no image content to load here. */
    .task_partitions (NOLOAD) :
    {
        . = ALIGN(8);                                                          /* 8-byte align: matches Cortex-M stack alignment */
        _task_partitions_start = .;                                            /* first partition begins here */
        _task0_partition_start = .; . += TASK_PARTITION_SIZE; _task0_partition_end = .;  /* task 0: reserve SIZE bytes, mark both ends */
        _task1_partition_start = .; . += TASK_PARTITION_SIZE; _task1_partition_end = .;  /* task 1: the next SIZE bytes, non-overlapping by construction */
        _task2_partition_start = .; . += TASK_PARTITION_SIZE; _task2_partition_end = .;  /* task 2: the next SIZE bytes */
        _task_partitions_end = .;                                              /* end of the whole partitioned block */
    } > RAM

    __StackLimit = _estack - _Min_Stack_Size;  /* lowest address the boot stack may grow to */
    /* Link-time guard: fail the build if the partitions and the boot
     * stack would overlap, rather than corrupting memory at runtime. */
    ASSERT(__StackLimit >= _task_partitions_end, "region RAM overflowed: task partitions collide with the stack")
}
