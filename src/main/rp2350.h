/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

/*
 * rp2350.h — chip-level type stubs for the RP2350 port.
 *
 * Included by platform.h in place of the vendor HAL headers that INAV's
 * generic driver layer expects on STM32 (stm32f7xx.h, stm32h7xx.h, …).
 * Only the fields and constants actually referenced by shared INAV code
 * are defined; everything else is a void* placeholder.
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

/* ── Chip unique ID ────────────────────────────────────────────────────────── */

#define U_ID_0 0
#define U_ID_1 1
#define U_ID_2 2

extern uint32_t SystemCoreClock;

/* ── Timer ─────────────────────────────────────────────────────────────────── */

typedef struct { void* dummy; } TIM_TypeDef;
typedef struct { void* dummy; } TIM_OCInitTypeDef;

/* ── DMA ───────────────────────────────────────────────────────────────────── */

typedef struct { void* dummy; } DMA_TypeDef;
typedef struct { void* dummy; } DMA_Channel_TypeDef;

/* ── SPI ───────────────────────────────────────────────────────────────────── */

typedef struct { void* dummy; } SPI_TypeDef;

/* RP2350-only identity tokens — NOT hardware register addresses.
 * Used only by bus_spi_rp2350.c to select spi0/spi1 by INAV device index.
 * Do not use in shared INAV driver code. */
#define SPI1 ((SPI_TypeDef *)0x0001)
#define SPI2 ((SPI_TypeDef *)0x0002)

/* ── I2C ───────────────────────────────────────────────────────────────────── */

typedef struct { void* dummy; } I2C_TypeDef;

/* ── GPIO ──────────────────────────────────────────────────────────────────── */

typedef struct
{
    uint32_t IDR;
    uint32_t ODR;
    uint32_t BSRR;
    uint32_t BRR;
} GPIO_TypeDef;

#define GPIOA_BASE ((intptr_t)0x0001)

/* ── USART / UART ──────────────────────────────────────────────────────────── */

typedef struct { uint32_t dummy; } USART_TypeDef;

#define USART1 ((USART_TypeDef *)0x0001)
#define USART2 ((USART_TypeDef *)0x0002)
#define USART3 ((USART_TypeDef *)0x0003)
#define USART4 ((USART_TypeDef *)0x0004)
#define USART5 ((USART_TypeDef *)0x0005)
#define USART6 ((USART_TypeDef *)0x0006)
#define USART7 ((USART_TypeDef *)0x0007)
#define USART8 ((USART_TypeDef *)0x0008)

/* Aliases for code that uses UARTx names rather than USARTx names. */
#define UART4 ((USART_TypeDef *)0x0004)
#define UART5 ((USART_TypeDef *)0x0005)
#define UART7 ((USART_TypeDef *)0x0007)
#define UART8 ((USART_TypeDef *)0x0008)

/* ── Misc HAL enums ────────────────────────────────────────────────────────── */

typedef enum { RESET = 0, SET = !RESET } FlagStatus, ITStatus;
typedef enum { DISABLE = 0, ENABLE = !DISABLE } FunctionalState;
typedef enum { TEST_IRQ = 0 } IRQn_Type;
typedef enum {
    EXTI_Trigger_Rising         = 0x08,
    EXTI_Trigger_Falling        = 0x0C,
    EXTI_Trigger_Rising_Falling = 0x10
} EXTITrigger_TypeDef;
