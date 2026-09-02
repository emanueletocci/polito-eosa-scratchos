/*
 * Embedded Operating Systems course
 * Licensed under the MIT License. See LICENSE in the repository root.
 *
 * Contact: Stefano Di Carlo <stefano.dicarlo@polito.it>
 *
 * Minimal register definitions for the peripherals scratch-os uses on
 * QEMU's mps2-an385 machine (ARM Cortex-M3 core + CMSDK peripherals).
 * Hand-written from the ARMv7-M architecture reference and the MPS2
 * board's own documented memory map - not copied from any vendored
 * CMSIS header. Only what we actually use; not a general CMSDK library.
 */

#ifndef SCRATCH_OS_CMSDK_H
#define SCRATCH_OS_CMSDK_H

#include <stdint.h>

/* ---- SysTick (ARMv7-M core peripheral, fixed address on every Cortex-M) --- */

typedef struct
{
    volatile uint32_t CTRL;  /* 0x00 control/status: ENABLE, TICKINT, CLKSOURCE, COUNTFLAG */
    volatile uint32_t LOAD;  /* 0x04 reload value (24-bit) */
    volatile uint32_t VAL;   /* 0x08 current value; any write clears it */
    volatile uint32_t CALIB; /* 0x0C calibration (unused here) */
} SysTick_TypeDef;

#define SysTick             ( ( SysTick_TypeDef * ) 0xE000E010UL )
#define SysTick_CTRL_ENABLE     ( 1UL << 0 )
#define SysTick_CTRL_TICKINT    ( 1UL << 1 )
#define SysTick_CTRL_CLKSOURCE  ( 1UL << 2 )

/* ---- NVIC (ARMv7-M core peripheral) ---- */

#define NVIC_ISER0          ( *( volatile uint32_t * ) 0xE000E100UL )

/* ---- CMSDK UART0 (MPS2 board peripheral) ---- */

typedef struct
{
    volatile uint32_t DATA;      /* 0x00 */
    volatile uint32_t STATE;     /* 0x04: bit0 TXBF (tx buffer full), bit1 RXBF */
    volatile uint32_t CTRL;      /* 0x08: bit0 TXEN, bit2 TXIRQEN */
    volatile uint32_t INTSTATUS; /* 0x0C: write 1 to clear (INTCLEAR alias) */
    volatile uint32_t BAUDDIV;   /* 0x10 */
} CMSDK_UART_TypeDef;

#define CMSDK_UART0          ( ( CMSDK_UART_TypeDef * ) 0x40004000UL )
#define UART_STATE_TXBF      ( 1UL << 0 ) /* tx buffer full */
#define UART_STATE_RXBF      ( 1UL << 1 ) /* rx buffer full (a byte is waiting) */
#define UART_CTRL_TXEN       ( 1UL << 0 )
#define UART_CTRL_RXEN       ( 1UL << 1 )
#define UART_CTRL_TXIRQEN    ( 1UL << 2 )
#define UART_CTRL_RXIRQEN    ( 1UL << 3 )
#define UART_INT_TXIRQ       ( 1UL << 0 )
#define UART_INT_RXIRQ       ( 1UL << 1 )

/* IRQn used as an NVIC input, per the board's interrupt map (positions
 * 0 and 1 in the external-interrupt array, i.e. isr_vector[16 + N]). */
#define UARTRX0_IRQn 0
#define UARTTX0_IRQn 1

#endif /* SCRATCH_OS_CMSDK_H */
