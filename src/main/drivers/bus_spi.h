/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "io_types.h"
#include "rcc_types.h"
#include "bus.h"

#if defined(STM32F4) || defined(STM32F3)
    #define SPI_IO_AF_CFG      IO_CONFIG(GPIO_Mode_AF,  GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL)
    #define SPI_IO_AF_SCK_CFG  IO_CONFIG(GPIO_Mode_AF,  GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN)
    #define SPI_IO_AF_MISO_CFG IO_CONFIG(GPIO_Mode_AF,  GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_UP)
    #define SPI_IO_CS_CFG      IO_CONFIG(GPIO_Mode_OUT, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL)
#elif defined(STM32F1)
    #define SPI_IO_AF_SCK_CFG     IO_CONFIG(GPIO_Mode_AF_PP,       GPIO_Speed_50MHz)
    #define SPI_IO_AF_MOSI_CFG    IO_CONFIG(GPIO_Mode_AF_PP,       GPIO_Speed_50MHz)
    #define SPI_IO_AF_MISO_CFG    IO_CONFIG(GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz)
    #define SPI_IO_CS_CFG         IO_CONFIG(GPIO_Mode_Out_PP,      GPIO_Speed_50MHz)
#endif

bool busSpiInit(const void * hwDesc);
void busSpiProcessTxn(const void * hwDesc, BusTransaction_t * txn);
void busSpiSetSpeed(const void * hwDesc, const BusSpeed_e speed);