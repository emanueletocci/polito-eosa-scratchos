/*
 * Embedded Operating Systems and Architecture course
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

/* Include guard: keep the definitions single even if included twice. */
#ifndef SCRATCH_OS_CMSDK_H
#define SCRATCH_OS_CMSDK_H

/* uint32_t, used for every register field below. */
#include <stdint.h>

/* ---- SysTick (ARMv7-M core peripheral, fixed address on every Cortex-M) --- */

/* Register block laid out to match the hardware: the four words at
 * 0xE000E010 in the order the CPU exposes them. `volatile` because the
 * hardware, not this code, changes their contents. */
typedef struct
{
    volatile uint32_t CTRL;  /* 0x00 control/status: ENABLE, TICKINT, CLKSOURCE, COUNTFLAG */
    volatile uint32_t LOAD;  /* 0x04 reload value (24-bit) */
    volatile uint32_t VAL;   /* 0x08 current value; any write clears it */
    volatile uint32_t CALIB; /* 0x0C calibration (unused here) */
} SysTick_TypeDef;

/* Access the block by casting its architectural base address to the struct. */
#define SysTick             ( ( SysTick_TypeDef * ) 0xE000E010UL )
#define SysTick_CTRL_ENABLE     ( 1UL << 0 ) /* start the counter */
#define SysTick_CTRL_TICKINT    ( 1UL << 1 ) /* raise the SysTick exception on wrap */
#define SysTick_CTRL_CLKSOURCE  ( 1UL << 2 ) /* use the core clock (not the reference clock) */

/* ---- NVIC (ARMv7-M core peripheral) ---- */

/* Interrupt Set-Enable Register 0: writing bit N enables external IRQ N. */
#define NVIC_ISER0          ( *( volatile uint32_t * ) 0xE000E100UL )

/* ---- CMSDK UART0 (MPS2 board peripheral) ---- */

/* The five UART0 registers, in hardware order, starting at 0x40004000. */
typedef struct
{
    volatile uint32_t DATA;      /* 0x00 read = RX byte, write = TX byte */
    volatile uint32_t STATE;     /* 0x04: bit0 TXBF (tx buffer full), bit1 RXBF */
    volatile uint32_t CTRL;      /* 0x08: bit0 TXEN, bit2 TXIRQEN */
    volatile uint32_t INTSTATUS; /* 0x0C: write 1 to clear (INTCLEAR alias) */
    volatile uint32_t BAUDDIV;   /* 0x10 baud-rate divider */
} CMSDK_UART_TypeDef;

/* UART0's base address on this board. */
#define CMSDK_UART0          ( ( CMSDK_UART_TypeDef * ) 0x40004000UL )
#define UART_STATE_TXBF      ( 1UL << 0 ) /* tx buffer full - wait before writing DATA */
#define UART_STATE_RXBF      ( 1UL << 1 ) /* rx buffer full (a byte is waiting) */
#define UART_CTRL_TXEN       ( 1UL << 0 ) /* enable the transmitter */
#define UART_CTRL_RXEN       ( 1UL << 1 ) /* enable the receiver */
#define UART_CTRL_TXIRQEN    ( 1UL << 2 ) /* raise an IRQ when the tx buffer drains */
#define UART_CTRL_RXIRQEN    ( 1UL << 3 ) /* raise an IRQ when a byte arrives */
#define UART_INT_TXIRQ       ( 1UL << 0 ) /* tx interrupt pending / write-1-to-clear bit */
#define UART_INT_RXIRQ       ( 1UL << 1 ) /* rx interrupt pending / write-1-to-clear bit */

/* IRQn used as an NVIC input, per the board's interrupt map (positions
 * 0 and 1 in the external-interrupt array, i.e. isr_vector[16 + N]). */
#define UARTRX0_IRQn 0
#define UARTTX0_IRQn 1

#endif /* SCRATCH_OS_CMSDK_H */
