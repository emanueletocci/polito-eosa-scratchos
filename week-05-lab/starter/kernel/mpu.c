/*
 * Embedded Operating Systems and Architecture course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * MPU set-up: see kernel/mpu.h.
 *
 * Prepared from week-04/solution/: the body of mpu_set_task_region was
 * taken out (exercises A1 to A3 of week-05-lab/README.md).
 *
 * Region 0 lets tasks read and run FLASH (code and constants).
 * Region 1 is meant to let a task read and write its own memory,
 * nothing else. Any other access by a task raises a MemManage fault.
 */

#include <stdint.h>
#include "cmsdk.h"
#include "kernel/mpu.h"

void mpu_init( void )
{
    /* Region 0 = FLASH, from address 0, 4 MB = 2^22 bytes.
     * Read-only for everyone, and code may run from it (no XN). */
    MPU->RBAR = 0x00000000 | MPU_RBAR_VALID | 0;       /* = 0x00000010 */
    MPU->RASR = MPU_RASR_AP( MPU_AP_RO ) | MPU_RASR_C
              | MPU_RASR_SIZE( 22 ) | MPU_RASR_ENABLE; /* = 0x0602002B */

    /* Switch the MPU on. PRIVDEFENA: the kernel (privileged) keeps the
     * normal memory map, so only tasks are restricted. */
    MPU->CTRL = MPU_CTRL_ENABLE | MPU_CTRL_PRIVDEFENA;  /* = 5 */
    __asm volatile ( "dsb" ::: "memory" );  /* finish the writes ... */
    __asm volatile ( "isb" ::: "memory" );  /* ... before the next instruction */
}

void mpu_set_task_region( uint32_t base, uint32_t size )
{
    /* Exercises A1 to A3 (week-05-lab/README.md) write this function.
     * Until they do, no region describes the task's memory, and the
     * first task cannot start. */
    (void) base;
    (void) size;
}
