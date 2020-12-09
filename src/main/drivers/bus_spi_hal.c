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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <platform.h>

#include "build/debug.h"
#include "drivers/bus_spi.h"
#include "dma.h"
#include "drivers/io.h"
#include "io_impl.h"
#include "drivers/nvic.h"
#include "rcc.h"

#ifndef SPI1_NSS_PIN
#define SPI1_NSS_PIN NONE
#endif
#ifndef SPI2_NSS_PIN
#define SPI2_NSS_PIN NONE
#endif
#ifndef SPI3_NSS_PIN
#define SPI3_NSS_PIN NONE
#endif
#ifndef SPI4_NSS_PIN
#define SPI4_NSS_PIN NONE
#endif

#if defined(USE_SPI_DEVICE_1)
static const uint32_t spiDivisorMapFast[] = {
    SPI_BAUDRATEPRESCALER_256,      // SPI_CLOCK_INITIALIZATON      421.875 KBits/s
    SPI_BAUDRATEPRESCALER_32,       // SPI_CLOCK_SLOW               843.75 KBits/s
    SPI_BAUDRATEPRESCALER_16,       // SPI_CLOCK_STANDARD           6.75 MBits/s
    SPI_BAUDRATEPRESCALER_8,        // SPI_CLOCK_FAST               13.5 MBits/s
    SPI_BAUDRATEPRESCALER_4         // SPI_CLOCK_ULTRAFAST          27.0 MBits/s
};
#endif

#if defined(USE_SPI_DEVICE_2) || defined(USE_SPI_DEVICE_3) || defined(USE_SPI_DEVICE_4)
static const uint32_t spiDivisorMapSlow[] = {
    SPI_BAUDRATEPRESCALER_256,      // SPI_CLOCK_INITIALIZATON      210.937 KBits/s
    SPI_BAUDRATEPRESCALER_64,       // SPI_CLOCK_SLOW               843.75 KBits/s
    SPI_BAUDRATEPRESCALER_8,        // SPI_CLOCK_STANDARD           6.75 MBits/s
    SPI_BAUDRATEPRESCALER_4,        // SPI_CLOCK_FAST               13.5 MBits/s
    SPI_BAUDRATEPRESCALER_2         // SPI_CLOCK_ULTRAFAST          27.0 MBits/s
};
#endif

static spiDevice_t spiHardwareMap[SPIDEV_COUNT] = {
#ifdef USE_SPI_DEVICE_1
    { .dev = SPI1, .nss = IO_TAG(SPI1_NSS_PIN), .sck = IO_TAG(SPI1_SCK_PIN), .miso = IO_TAG(SPI1_MISO_PIN), .mosi = IO_TAG(SPI1_MOSI_PIN), .rcc = RCC_APB2(SPI1), .af = GPIO_AF5_SPI1, .divisorMap = spiDivisorMapFast },
#else
    { .dev = NULL },    // No SPI1
#endif
#ifdef USE_SPI_DEVICE_2
    { .dev = SPI2, .nss = IO_TAG(SPI2_NSS_PIN), .sck = IO_TAG(SPI2_SCK_PIN), .miso = IO_TAG(SPI2_MISO_PIN), .mosi = IO_TAG(SPI2_MOSI_PIN), .rcc = RCC_APB1L(SPI2), .af = GPIO_AF5_SPI2, .divisorMap = spiDivisorMapSlow },
#else
    { .dev = NULL },    // No SPI2
#endif
#ifdef USE_SPI_DEVICE_3
    { .dev = SPI3, .nss = IO_TAG(SPI3_NSS_PIN), .sck = IO_TAG(SPI3_SCK_PIN), .miso = IO_TAG(SPI3_MISO_PIN), .mosi = IO_TAG(SPI3_MOSI_PIN), .rcc = RCC_APB1L(SPI3), .af = GPIO_AF6_SPI3, .divisorMap = spiDivisorMapSlow },
#else
    { .dev = NULL },    // No SPI3
#endif
#ifdef USE_SPI_DEVICE_4
    { .dev = SPI4, .nss = IO_TAG(SPI4_NSS_PIN), .sck = IO_TAG(SPI4_SCK_PIN), .miso = IO_TAG(SPI4_MISO_PIN), .mosi = IO_TAG(SPI4_MOSI_PIN), .rcc = RCC_APB2(SPI4), .af = GPIO_AF5_SPI4, .divisorMap = spiDivisorMapSlow }
#else
    { .dev = NULL }     // No SPI4
#endif
};

static SPI_HandleTypeDef spiHandle[SPIDEV_COUNT];

SPIDevice spiDeviceByInstance(SPI_TypeDef *instance)
{
    if (instance == SPI1)
        return SPIDEV_1;

    if (instance == SPI2)
        return SPIDEV_2;

    if (instance == SPI3)
        return SPIDEV_3;

    if (instance == SPI4)
        return SPIDEV_4;

    return SPIINVALID;
}

void spiTimeoutUserCallback(SPI_TypeDef *instance)
{
    SPIDevice device = spiDeviceByInstance(instance);
    if (device == SPIINVALID) {
        return;
    }

    spiHardwareMap[device].errorCount++;
}

bool spiInitDevice(SPIDevice device, bool leadingEdge)
{
    spiDevice_t *spi = &spiHardwareMap[device];

    if (!spi->dev) {
        return false;
    }

    if (spi->initDone) {
        return true;
    }

    // Enable SPI clock
    RCC_ClockCmd(spi->rcc, ENABLE);
    RCC_ResetCmd(spi->rcc, DISABLE);

    IOInit(IOGetByTag(spi->sck),  OWNER_SPI, RESOURCE_SPI_SCK,  device + 1);
    IOInit(IOGetByTag(spi->miso), OWNER_SPI, RESOURCE_SPI_MISO, device + 1);
    IOInit(IOGetByTag(spi->mosi), OWNER_SPI, RESOURCE_SPI_MOSI, device + 1);

    if (leadingEdge) {
        IOConfigGPIOAF(IOGetByTag(spi->sck), SPI_IO_AF_SCK_CFG_LOW, spi->af);
    } else {
        IOConfigGPIOAF(IOGetByTag(spi->sck), SPI_IO_AF_SCK_CFG_HIGH, spi->af);
    }
    IOConfigGPIOAF(IOGetByTag(spi->miso), SPI_IO_AF_MISO_CFG, spi->af);
    IOConfigGPIOAF(IOGetByTag(spi->mosi), SPI_IO_AF_CFG, spi->af);

    if (spi->nss) {
        IOInit(IOGetByTag(spi->nss),  OWNER_SPI, RESOURCE_SPI_CS,  device + 1);
        IOConfigGPIO(IOGetByTag(spi->nss), SPI_IO_CS_CFG);
    }

    SPI_HandleTypeDef * hspi = &spiHandle[device];
    memset(hspi, 0, sizeof(SPI_HandleTypeDef));
    hspi->Instance = spi->dev;

    HAL_SPI_DeInit(hspi);

    hspi->Init.Mode = SPI_MODE_MASTER;
    hspi->Init.Direction = SPI_DIRECTION_2LINES;
    hspi->Init.DataSize = SPI_DATASIZE_8BIT;
    hspi->Init.NSS = SPI_NSS_SOFT;
    hspi->Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi->Init.CRCPolynomial = 7;
    hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    hspi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi->Init.TIMode = SPI_TIMODE_DISABLED;
    hspi->Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    hspi->Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;  /* Recommanded setting to avoid glitches */

    if (leadingEdge) {
        hspi->Init.CLKPolarity = SPI_POLARITY_LOW;
        hspi->Init.CLKPhase = SPI_PHASE_1EDGE;
    }
    else {
        hspi->Init.CLKPolarity = SPI_POLARITY_HIGH;
        hspi->Init.CLKPhase = SPI_PHASE_2EDGE;
    }

    if (spi->nss) {
        IOHi(IOGetByTag(spi->nss));
    }

    HAL_SPI_Init(hspi);

    spi->initDone = true;
    return true;
}

uint8_t spiTransferByte(SPI_TypeDef *instance, uint8_t txByte)
{
    uint8_t rxData;
    spiTransfer(instance, &rxData, &txByte, 1);
    return rxData;
}

/**
 * Return true if the bus is currently in the middle of a transmission.
 */
bool spiIsBusBusy(SPI_TypeDef *instance)
{
    SPIDevice device = spiDeviceByInstance(instance);
    return (spiHandle[device].State == HAL_SPI_STATE_BUSY);
}

bool spiTransfer(SPI_TypeDef *instance, uint8_t *rxData, const uint8_t *txData, int len)
{
    SPIDevice device = spiDeviceByInstance(instance);
    SPI_HandleTypeDef * hspi = &spiHandle[device];
    HAL_StatusTypeDef status;

    #define SPI_DEFAULT_TIMEOUT 10

    if (!rxData) {
        status = HAL_SPI_Transmit(hspi, txData, len, SPI_DEFAULT_TIMEOUT);
    } else if(!txData) {
        status = HAL_SPI_Receive(hspi, rxData, len, SPI_DEFAULT_TIMEOUT);
    } else {
        status = HAL_SPI_TransmitReceive(hspi, txData, rxData, len, SPI_DEFAULT_TIMEOUT);
    }

    if(status != HAL_OK) {
        spiTimeoutUserCallback(instance);
    }

    return true;
}

void spiSetSpeed(SPI_TypeDef *instance, SPIClockSpeed_e speed)
{
    SPIDevice device = spiDeviceByInstance(instance);
    SPI_HandleTypeDef * hspi = &spiHandle[device];

    HAL_SPI_DeInit(hspi);
    hspi->Init.BaudRatePrescaler = spiHardwareMap[device].divisorMap[speed];
    HAL_SPI_Init(hspi);
}

SPI_TypeDef * spiInstanceByDevice(SPIDevice device)
{
    return spiHardwareMap[device].dev;
}
