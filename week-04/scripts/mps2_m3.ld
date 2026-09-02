/*
 * Embedded Operating Systems course
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

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 4M
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 4M
}

ENTRY(Reset_Handler)

_estack = ORIGIN(RAM) + LENGTH(RAM);   /* stack grows down from top of RAM */
_Min_Stack_Size = 0x1000;              /* 4KB - the boot stack (not a task's own stack yet - that's week 5) */

TASK_COUNT = 3;
TASK_PARTITION_SIZE = 512; /* bytes reserved per task, fixed at link time */

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /* One fixed-size, fixed-address region per task. Named per-task
     * symbols (_task0_partition_start/_end, _task1_..., _task2_...)
     * rather than a single array, so each task's boundary is visible
     * and checkable on its own - not just "somewhere in a shared
     * region." NOLOAD: reserved RAM, no image content to load here. */
    .task_partitions (NOLOAD) :
    {
        . = ALIGN(8);
        _task_partitions_start = .;
        _task0_partition_start = .; . += TASK_PARTITION_SIZE; _task0_partition_end = .;
        _task1_partition_start = .; . += TASK_PARTITION_SIZE; _task1_partition_end = .;
        _task2_partition_start = .; . += TASK_PARTITION_SIZE; _task2_partition_end = .;
        _task_partitions_end = .;
    } > RAM

    __StackLimit = _estack - _Min_Stack_Size;
    ASSERT(__StackLimit >= _task_partitions_end, "region RAM overflowed: task partitions collide with the stack")
}
