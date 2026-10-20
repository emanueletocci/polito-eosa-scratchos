/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Linker script for QEMU's mps2-an385 (Cortex-M3). Flat layout: each task
 * has a stack, but no partition yet. No heap.
 */

/* Physical memory regions of the board, with their access attributes.
 * The linker uses these to place sections and to catch overflow. */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 4M  /* code + read-only data, execute-in-place */
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 4M  /* data, bss, stack */
}

/* The image's entry point - the symbol whose address the debugger and
 * QEMU use as the start of execution (also entry 1 of the vector table). */
ENTRY(Reset_Handler)

_estack = ORIGIN(RAM) + LENGTH(RAM);   /* initial SP = top of RAM; the stack grows downward from here */
_Min_Stack_Size = 0x1000;              /* 4 KB reserved for the kernel stack - checked against .bss below */
TASK_STACK_SIZE = 0x400;               /* 1 KB stack for each task */
PART_SIZE = 0x1000;                    /* 4 KB per task partition */

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

    /* >>> UNIT 2: paste the task partitions here, before .data (see the slides). <<< */

/* Task A partition, 4 KB: [ stack 1 KB | A's .data + .bss | padding ] */
    .task_a_stack ALIGN(ORIGIN(RAM), 4096) (NOLOAD) : {
        _task_a_part_start = .;            /* partition start = stack bottom */
        . = . + TASK_STACK_SIZE;           /* reserve the 1 KB stack */
        _task_a_stack_top = .;             /* the stack grows down from here */
    } > RAM
    .task_a_data : {
        . = ALIGN(4);
        _task_a_data_start = .;            /* run address (RAM) */
        *task_a.o(.data .data.* .bss .bss.* COMMON)   /* all of task_a.o's variables */
        . = ALIGN(4);
        _task_a_data_end = .;
    } > RAM AT > FLASH                     /* runs in RAM, image stored in FLASH */
    _task_a_data_load = LOADADDR(.task_a_data);   /* FLASH address of the image */
    .task_a_pad (NOLOAD) : {
        . = _task_a_part_start + PART_SIZE;   /* jump to the partition end */
        _task_a_part_end = .;
    } > RAM


    .task_b_stack ALIGN(4096) (NOLOAD) : {
        _task_b_part_start = .;
        . = . + TASK_STACK_SIZE;
        _task_b_stack_top = .;
    } > RAM
    .task_b_data : {
        . = ALIGN(4);
        _task_b_data_start = .;
        *task_b.o(.data .data.* .bss .bss.* COMMON)
        . = ALIGN(4);
        _task_b_data_end = .;
    } > RAM AT > FLASH
    _task_b_data_load = LOADADDR(.task_b_data);
    .task_b_pad (NOLOAD) : {
        . = _task_b_part_start + PART_SIZE;
        _task_b_part_end = .;
    } > RAM
    ASSERT(_task_a_data_end <= _task_a_part_end, "task A data does not fit in its partition")
    ASSERT(_task_b_data_end <= _task_b_part_end, "task B data does not fit in its partition")


    _sidata = LOADADDR(.data);  /* FLASH address where .data's initial image is stored */

    /* Initialized variables: they live (run) in RAM but their initial
     * contents are stored in FLASH and copied over by Reset_Handler. */
    .data :
    {
        . = ALIGN(4);           /* word-align the start of .data */
        _sdata = .;             /* start of .data in RAM - Reset_Handler copies from here */
        *(.data*)               /* every initialized variable: kernel and tasks alike */
        . = ALIGN(4);           /* word-align the end */
        _edata = .;             /* end of .data in RAM - copy loop stops here */
    } > RAM AT > FLASH          /* run address in RAM, load address in FLASH */

    /* Zero-initialized variables: reserve RAM, no stored image. */
    .bss :
    {
        . = ALIGN(4);           /* word-align the start of .bss */
        _sbss = .;              /* start of .bss - Reset_Handler zeroes from here */
        *(.bss*)                /* every zero-initialized variable: kernel and tasks alike */
        *(COMMON)               /* legacy tentative-definition symbols */
        . = ALIGN(4);           /* word-align the end */
        _ebss = .;              /* end of .bss - zero loop stops here */
    } > RAM

}
