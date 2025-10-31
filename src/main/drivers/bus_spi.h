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

#include "drivers/io_types.h"
#include "drivers/rcc_types.h"
#include "drivers/dma.h"

#if defined(STM32F4)
#define SPI_IO_AF_CFG           IO_CONFIG(GPIO_Mode_AF,  GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL)
#define SPI_IO_AF_SCK_CFG       IO_CONFIG(GPIO_Mode_AF,  GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN)
#define SPI_IO_AF_MISO_CFG      IO_CONFIG(GPIO_Mode_AF,  GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_UP)
#define SPI_IO_CS_CFG           IO_CONFIG(GPIO_Mode_OUT, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL)
#elif defined(STM32F7) || defined(STM32H7)
#define SPI_IO_AF_CFG           IO_CONFIG(GPIO_MODE_AF_PP, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_NOPULL)
#define SPI_IO_AF_SCK_CFG_HIGH  IO_CONFIG(GPIO_MODE_AF_PP, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_PULLUP)
#define SPI_IO_AF_SCK_CFG_LOW   IO_CONFIG(GPIO_MODE_AF_PP, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_PULLDOWN)
#define SPI_IO_AF_MISO_CFG      IO_CONFIG(GPIO_MODE_AF_PP, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_PULLUP)
#define SPI_IO_CS_CFG           IO_CONFIG(GPIO_MODE_OUTPUT_PP, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_NOPULL)
#elif defined(AT32F43x)
#define SPI_IO_AF_CFG           IO_CONFIG(GPIO_MODE_MUX,  GPIO_DRIVE_STRENGTH_STRONGER, GPIO_OUTPUT_PUSH_PULL, GPIO_PULL_NONE)
#define SPI_IO_AF_SCK_CFG       IO_CONFIG(GPIO_MODE_MUX,  GPIO_DRIVE_STRENGTH_STRONGER, GPIO_OUTPUT_PUSH_PULL, GPIO_PULL_DOWN)
#define SPI_IO_AF_MISO_CFG      IO_CONFIG(GPIO_MODE_MUX,  GPIO_DRIVE_STRENGTH_STRONGER, GPIO_OUTPUT_PUSH_PULL, GPIO_PULL_UP)
#define SPI_IO_CS_CFG           IO_CONFIG(GPIO_MODE_OUTPUT, GPIO_DRIVE_STRENGTH_STRONGER, GPIO_OUTPUT_PUSH_PULL, GPIO_PULL_NONE)
#endif

/*
  Flash M25p16 tolerates 20mhz, SPI_CLOCK_FAST should sit around 20 or less.
*/

typedef enum {
    SPI_CLOCK_INITIALIZATON = 0,    // Lowest possible
    SPI_CLOCK_SLOW          = 1,    // ~1 MHz
    SPI_CLOCK_STANDARD      = 2,    // ~10MHz
    SPI_CLOCK_FAST          = 3,    // ~20MHz
    SPI_CLOCK_ULTRAFAST     = 4     // Highest possible
} SPIClockSpeed_e;

typedef enum SPIDevice {
    SPIINVALID = -1,
    SPIDEV_1   = 0,
    SPIDEV_2,
    SPIDEV_3,
    SPIDEV_4
} SPIDevice;

#if defined(STM32F4)
#define SPIDEV_COUNT 3
#elif defined(STM32F7) || defined(STM32H7)|| defined(AT32F43x)
#define SPIDEV_COUNT 4
#else
#define SPIDEV_COUNT 4
#endif

#if defined(AT32F43x)
typedef spi_type SPI_TypeDef;
#endif

typedef struct SPIDevice_s {
#if defined(AT32F43x)
     spi_type *dev;
#else
     SPI_TypeDef *dev;
#endif
    ioTag_t nss;
    ioTag_t sck;
    ioTag_t mosi;
    ioTag_t miso;
    rccPeriphTag_t rcc;
#if defined(STM32F7) || defined(STM32H7) || defined(AT32F43x)
    uint8_t sckAF;
    uint8_t misoAF;
    uint8_t mosiAF;
#else
    uint8_t af;
#endif
    const uint32_t * divisorMap;
    volatile uint16_t errorCount;
    bool initDone;
} spiDevice_t;

bool spiInitDevice(SPIDevice device, bool leadingEdge);

#if defined(AT32F43x)

    bool spiIsBusBusy(spi_type *instance);
    void spiSetSpeed(spi_type *instance, SPIClockSpeed_e speed);
    uint8_t spiTransferByte(spi_type *instance, uint8_t in);
    bool spiTransfer(spi_type *instance, uint8_t *rxData, const uint8_t *txData, int len);

    uint16_t spiGetErrorCounter(spi_type *instance);
    void spiResetErrorCounter(spi_type *instance);
    SPIDevice spiDeviceByInstance(spi_type *instance);
    spi_type * spiInstanceByDevice(SPIDevice device);

#else
    bool spiIsBusBusy(SPI_TypeDef *instance);
    void spiSetSpeed(SPI_TypeDef *instance, SPIClockSpeed_e speed);
    uint8_t spiTransferByte(SPI_TypeDef *instance, uint8_t in);
    bool spiTransfer(SPI_TypeDef *instance, uint8_t *rxData, const uint8_t *txData, int len);

    uint16_t spiGetErrorCounter(SPI_TypeDef *instance);
    void spiResetErrorCounter(SPI_TypeDef *instance);
    SPIDevice spiDeviceByInstance(SPI_TypeDef *instance);
    SPI_TypeDef * spiInstanceByDevice(SPIDevice device);
#endif
