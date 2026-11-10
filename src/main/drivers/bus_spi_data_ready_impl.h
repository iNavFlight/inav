/*
 * This file is part of INAV.
 *
 * INAV is free software. You can redistribute this software
 * and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV is distributed in the hope that they will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

// What a family's SPI driver provides to bus_spi_data_ready.c

// Gets the bus ready for reads of reg and the len bytes after it. False if it can not
bool spiDataReadyHwInit(SPIDevice device, uint8_t reg, uint8_t len);
// Starts a read. The device is already selected
void spiDataReadyHwStart(SPIDevice device);
// Takes a read off the bus, finished or not. The device is deselected after
void spiDataReadyHwStop(SPIDevice device);
// No more reads on this bus: the interrupts that end them go off
void spiDataReadyHwDisable(SPIDevice device);

// From the family's interrupt that ended a read, with a copy of the bytes after the address
void spiDataReadyDone(SPIDevice device, const uint8_t *data);
// Counts a transfer that never ended on this bus (the family's SPI driver has it)
#if defined(STM32H7) || defined(STM32F7)
void spiTimeoutUserCallback(SPI_TypeDef *instance);
#else
uint32_t spiTimeoutUserCallback(SPI_TypeDef *instance);
#endif

#if defined(STM32F4) || defined(STM32F7) || defined(STM32H7)
#include "drivers/dma.h"

// Reads longer than the SPI FIFO go through two DMA streams (bus_spi_data_ready_dma.c)
typedef struct {
    DMA_t rx;
    DMA_t tx;
#if defined(STM32F4) || defined(STM32F7)
    uint32_t rxChannel;                 // the stream's channel for this SPI
    uint32_t txChannel;
#endif
} spiDataReadyDma_t;

// Takes a free stream for each direction. done() runs at the end of each read, or when the
// receive stream fails. False if the bus has no free pair
bool spiDataReadyDmaInit(SPIDevice device, spiDataReadyDma_t *dma, dmaCallbackHandlerFuncPtr done, uint32_t userParam);
// n bytes from tx to the SPI's transmit register, and n from its receive register to rx
void spiDataReadyDmaStart(const spiDataReadyDma_t *dma, volatile void *txReg, volatile void *rxReg, const uint8_t *tx, uint8_t *rx, uint16_t n);
void spiDataReadyDmaStop(const spiDataReadyDma_t *dma);
// In done(): true if the read ended, false if the stream failed. Clears the stream's flags
bool spiDataReadyDmaEnded(DMA_t rx);
void spiDataReadyDmaDisable(const spiDataReadyDma_t *dma);
#endif
