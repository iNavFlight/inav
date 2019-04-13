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

#include <platform.h>

#ifdef USE_SPI

#include "drivers/bus_spi.h"
#include "drivers/exti.h"
#include "drivers/io.h"
#include "drivers/io_impl.h"
#include "drivers/rcc.h"

/* for F30x processors */
#if defined(STM32F303xC)
#ifndef GPIO_AF_SPI1
#define GPIO_AF_SPI1    GPIO_AF_5
#endif
#ifndef GPIO_AF_SPI2
#define GPIO_AF_SPI2    GPIO_AF_5
#endif
#ifndef GPIO_AF_SPI3
#define GPIO_AF_SPI3    GPIO_AF_6
#endif
#endif

#ifndef SPI1_SCK_PIN
#define SPI1_NSS_PIN    PA4
#define SPI1_SCK_PIN    PA5
#define SPI1_MISO_PIN   PA6
#define SPI1_MOSI_PIN   PA7
#endif

#ifndef SPI2_SCK_PIN
#define SPI2_NSS_PIN    PB12
#define SPI2_SCK_PIN    PB13
#define SPI2_MISO_PIN   PB14
#define SPI2_MOSI_PIN   PB15
#endif

#ifndef SPI3_SCK_PIN
#define SPI3_NSS_PIN    PA15
#define SPI3_SCK_PIN    PB3
#define SPI3_MISO_PIN   PB4
#define SPI3_MOSI_PIN   PB5
#endif

#ifndef SPI1_NSS_PIN
#define SPI1_NSS_PIN NONE
#endif
#ifndef SPI2_NSS_PIN
#define SPI2_NSS_PIN NONE
#endif
#ifndef SPI3_NSS_PIN
#define SPI3_NSS_PIN NONE
#endif

#if defined(STM32F3)
#if defined(USE_SPI_DEVICE_1)
static const uint16_t spiDivisorMapFast[] = {
    SPI_BaudRatePrescaler_256,    // SPI_CLOCK_INITIALIZATON      281.25 KBits/s
    SPI_BaudRatePrescaler_128,    // SPI_CLOCK_SLOW               562.5 KBits/s
    SPI_BaudRatePrescaler_8,      // SPI_CLOCK_STANDARD           9.0 MBits/s
    SPI_BaudRatePrescaler_4,      // SPI_CLOCK_FAST               18.0 MBits/s
    SPI_BaudRatePrescaler_4       // SPI_CLOCK_ULTRAFAST          18.0 MBits/s
};
#endif

#if defined(USE_SPI_DEVICE_2) || defined(USE_SPI_DEVICE_3)
static const uint16_t spiDivisorMapSlow[] = {
    SPI_BaudRatePrescaler_256,    // SPI_CLOCK_INITIALIZATON      140.625 KBits/s
    SPI_BaudRatePrescaler_64,     // SPI_CLOCK_SLOW               562.5 KBits/s
    SPI_BaudRatePrescaler_4,      // SPI_CLOCK_STANDARD           9.0 MBits/s
    SPI_BaudRatePrescaler_2,      // SPI_CLOCK_FAST               18.0 MBits/s
    SPI_BaudRatePrescaler_2       // SPI_CLOCK_ULTRAFAST          18.0 MBits/s
};
#endif

static spiDevice_t spiHardwareMap[] = {
#ifdef USE_SPI_DEVICE_1
    { .dev = SPI1, .nss = IO_TAG(SPI1_NSS_PIN), .sck = IO_TAG(SPI1_SCK_PIN), .miso = IO_TAG(SPI1_MISO_PIN), .mosi = IO_TAG(SPI1_MOSI_PIN), .rcc = RCC_APB2(SPI1), .af = GPIO_AF_SPI1, .divisorMap = spiDivisorMapFast },
#else
    { .dev = NULL },    // No SPI1
#endif
#ifdef USE_SPI_DEVICE_2
    { .dev = SPI2, .nss = IO_TAG(SPI2_NSS_PIN), .sck = IO_TAG(SPI2_SCK_PIN), .miso = IO_TAG(SPI2_MISO_PIN), .mosi = IO_TAG(SPI2_MOSI_PIN), .rcc = RCC_APB1(SPI2), .af = GPIO_AF_SPI2, .divisorMap = spiDivisorMapSlow },
#else
    { .dev = NULL },    // No SPI2
#endif
#ifdef USE_SPI_DEVICE_3
    { .dev = SPI3, .nss = IO_TAG(SPI3_NSS_PIN), .sck = IO_TAG(SPI3_SCK_PIN), .miso = IO_TAG(SPI3_MISO_PIN), .mosi = IO_TAG(SPI3_MOSI_PIN), .rcc = RCC_APB1(SPI3), .af = GPIO_AF_SPI3, .divisorMap = spiDivisorMapSlow },
#else
    { .dev = NULL },    // No SPI3
#endif
    { .dev = NULL },    // No SPI4
};
#elif defined(STM32F4)
#if defined(USE_SPI_DEVICE_1)
static const uint16_t spiDivisorMapFast[] = {
    SPI_BaudRatePrescaler_256,    // SPI_CLOCK_INITIALIZATON      328.125 KBits/s
    SPI_BaudRatePrescaler_128,    // SPI_CLOCK_SLOW               656.25 KBits/s
    SPI_BaudRatePrescaler_8,      // SPI_CLOCK_STANDARD           10.5 MBits/s
    SPI_BaudRatePrescaler_4,      // SPI_CLOCK_FAST               21.0 MBits/s
    SPI_BaudRatePrescaler_2       // SPI_CLOCK_ULTRAFAST          42.0 MBits/s
};
#endif

#if defined(USE_SPI_DEVICE_2) || defined(USE_SPI_DEVICE_3)
static const uint16_t spiDivisorMapSlow[] = {
    SPI_BaudRatePrescaler_256,    // SPI_CLOCK_INITIALIZATON      164.062 KBits/s
    SPI_BaudRatePrescaler_64,     // SPI_CLOCK_SLOW               656.25 KBits/s
    SPI_BaudRatePrescaler_4,      // SPI_CLOCK_STANDARD           10.5 MBits/s
    SPI_BaudRatePrescaler_2,      // SPI_CLOCK_FAST               21.0 MBits/s
    SPI_BaudRatePrescaler_2       // SPI_CLOCK_ULTRAFAST          21.0 MBits/s
};
#endif

static spiDevice_t spiHardwareMap[] = {
#ifdef USE_SPI_DEVICE_1
    { .dev = SPI1, .nss = IO_TAG(SPI1_NSS_PIN), .sck = IO_TAG(SPI1_SCK_PIN), .miso = IO_TAG(SPI1_MISO_PIN), .mosi = IO_TAG(SPI1_MOSI_PIN), .rcc = RCC_APB2(SPI1), .af = GPIO_AF_SPI1, .divisorMap = spiDivisorMapFast },
#else
    { .dev = NULL },    // No SPI1
#endif
#ifdef USE_SPI_DEVICE_2
    { .dev = SPI2, .nss = IO_TAG(SPI2_NSS_PIN), .sck = IO_TAG(SPI2_SCK_PIN), .miso = IO_TAG(SPI2_MISO_PIN), .mosi = IO_TAG(SPI2_MOSI_PIN), .rcc = RCC_APB1(SPI2), .af = GPIO_AF_SPI2, .divisorMap = spiDivisorMapSlow },
#else
    { .dev = NULL },    // No SPI2
#endif
#ifdef USE_SPI_DEVICE_3
    { .dev = SPI3, .nss = IO_TAG(SPI3_NSS_PIN), .sck = IO_TAG(SPI3_SCK_PIN), .miso = IO_TAG(SPI3_MISO_PIN), .mosi = IO_TAG(SPI3_MOSI_PIN), .rcc = RCC_APB1(SPI3), .af = GPIO_AF_SPI3, .divisorMap = spiDivisorMapSlow },
#else
    { .dev = NULL },    // No SPI3
#endif
    { .dev = NULL },    // No SPI4
};
#else
#error "Invalid CPU"
#endif

SPIDevice spiDeviceByInstance(SPI_TypeDef *instance)
{
    if (instance == SPI1)
        return SPIDEV_1;

    if (instance == SPI2)
        return SPIDEV_2;

    if (instance == SPI3)
        return SPIDEV_3;

    return SPIINVALID;
}

bool spiInitDevice(SPIDevice device, bool leadingEdge)
{
    spiDevice_t *spi = &(spiHardwareMap[device]);

    if (!spi->dev) {
        return false;
    }

    if (spi->initDone) {
        return true;
    }

    // Enable SPI clock
    RCC_ClockCmd(spi->rcc, ENABLE);
    RCC_ResetCmd(spi->rcc, ENABLE);

    IOInit(IOGetByTag(spi->sck),  OWNER_SPI, RESOURCE_SPI_SCK,  device + 1);
    IOInit(IOGetByTag(spi->miso), OWNER_SPI, RESOURCE_SPI_MISO, device + 1);
    IOInit(IOGetByTag(spi->mosi), OWNER_SPI, RESOURCE_SPI_MOSI, device + 1);

#if defined(STM32F3) || defined(STM32F4)
    if (leadingEdge) {
        IOConfigGPIOAF(IOGetByTag(spi->sck),  SPI_IO_AF_SCK_CFG, spi->af);
        IOConfigGPIOAF(IOGetByTag(spi->miso), SPI_IO_AF_MISO_CFG, spi->af);
        IOConfigGPIOAF(IOGetByTag(spi->mosi), SPI_IO_AF_CFG, spi->af);
    } else {
        IOConfigGPIOAF(IOGetByTag(spi->sck),  SPI_IO_AF_CFG, spi->af);
        IOConfigGPIOAF(IOGetByTag(spi->miso), SPI_IO_AF_CFG, spi->af);
        IOConfigGPIOAF(IOGetByTag(spi->mosi), SPI_IO_AF_CFG, spi->af);
    }

    if (spi->nss) {
        IOConfigGPIOAF(IOGetByTag(spi->nss), SPI_IO_CS_CFG, spi->af);
    }
#endif

    // Init SPI hardware
    SPI_I2S_DeInit(spi->dev);

    SPI_InitTypeDef spiInit;
    spiInit.SPI_Mode = SPI_Mode_Master;
    spiInit.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spiInit.SPI_DataSize = SPI_DataSize_8b;
    spiInit.SPI_NSS = SPI_NSS_Soft;
    spiInit.SPI_FirstBit = SPI_FirstBit_MSB;
    spiInit.SPI_CRCPolynomial = 7;
    spiInit.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;

    if (leadingEdge) {
        // SPI_MODE0
        spiInit.SPI_CPOL = SPI_CPOL_Low;
        spiInit.SPI_CPHA = SPI_CPHA_1Edge;
    } else {
        // SPI_MODE3
        spiInit.SPI_CPOL = SPI_CPOL_High;
        spiInit.SPI_CPHA = SPI_CPHA_2Edge;
    }

#ifdef STM32F303xC
    // Configure for 8-bit reads.
    SPI_RxFIFOThresholdConfig(spi->dev, SPI_RxFIFOThreshold_QF);
#endif

    SPI_Init(spi->dev, &spiInit);
    SPI_Cmd(spi->dev, ENABLE);

    if (spi->nss) {
        // Drive NSS high to disable connected SPI device.
        IOHi(IOGetByTag(spi->nss));
    }

    spi->initDone = true;
    return true;
}

uint32_t spiTimeoutUserCallback(SPI_TypeDef *instance)
{
    SPIDevice device = spiDeviceByInstance(instance);
    if (device == SPIINVALID) {
        return -1;
    }
    spiHardwareMap[device].errorCount++;
    return spiHardwareMap[device].errorCount;
}

// return uint8_t value or -1 when failure
uint8_t spiTransferByte(SPI_TypeDef *instance, uint8_t data)
{
    uint16_t spiTimeout = 1000;

    while (SPI_I2S_GetFlagStatus(instance, SPI_I2S_FLAG_TXE) == RESET)
        if ((spiTimeout--) == 0)
            return spiTimeoutUserCallback(instance);

#ifdef STM32F303xC
    SPI_SendData8(instance, data);
#else
    SPI_I2S_SendData(instance, data);
#endif
    spiTimeout = 1000;
    while (SPI_I2S_GetFlagStatus(instance, SPI_I2S_FLAG_RXNE) == RESET)
        if ((spiTimeout--) == 0)
            return spiTimeoutUserCallback(instance);

#ifdef STM32F303xC
    return ((uint8_t)SPI_ReceiveData8(instance));
#else
    return ((uint8_t)SPI_I2S_ReceiveData(instance));
#endif
}

/**
 * Return true if the bus is currently in the middle of a transmission.
 */
bool spiIsBusBusy(SPI_TypeDef *instance)
{
#ifdef STM32F303xC
    return SPI_GetTransmissionFIFOStatus(instance) != SPI_TransmissionFIFOStatus_Empty || SPI_I2S_GetFlagStatus(instance, SPI_I2S_FLAG_BSY) == SET;
#else
    return SPI_I2S_GetFlagStatus(instance, SPI_I2S_FLAG_TXE) == RESET || SPI_I2S_GetFlagStatus(instance, SPI_I2S_FLAG_BSY) == SET;
#endif

}

bool spiTransfer(SPI_TypeDef *instance, uint8_t *out, const uint8_t *in, int len)
{
    uint16_t spiTimeout = 1000;

    instance->DR;
    while (len--) {
        uint8_t b = in ? *(in++) : 0xFF;
        while (SPI_I2S_GetFlagStatus(instance, SPI_I2S_FLAG_TXE) == RESET) {
            if ((spiTimeout--) == 0)
                return spiTimeoutUserCallback(instance);
        }
#ifdef STM32F303xC
        SPI_SendData8(instance, b);
#else
        SPI_I2S_SendData(instance, b);
#endif
        spiTimeout = 1000;
        while (SPI_I2S_GetFlagStatus(instance, SPI_I2S_FLAG_RXNE) == RESET) {
            if ((spiTimeout--) == 0)
                return spiTimeoutUserCallback(instance);
        }
#ifdef STM32F303xC
        b = SPI_ReceiveData8(instance);
#else
        b = SPI_I2S_ReceiveData(instance);
#endif
        if (out)
            *(out++) = b;
    }

    return true;
}

void spiSetSpeed(SPI_TypeDef *instance, SPIClockSpeed_e speed)
{
#define BR_CLEAR_MASK 0xFFC7
    SPIDevice device = spiDeviceByInstance(instance);
    if (device == SPIINVALID) {
        return;
    }

    SPI_Cmd(instance, DISABLE);

    uint16_t tempRegister = instance->CR1;
    tempRegister &= BR_CLEAR_MASK;
    tempRegister |= spiHardwareMap[device].divisorMap[speed];
    instance->CR1 = tempRegister;

    SPI_Cmd(instance, ENABLE);
}

uint16_t spiGetErrorCounter(SPI_TypeDef *instance)
{
    SPIDevice device = spiDeviceByInstance(instance);
    if (device == SPIINVALID) {
        return 0;
    }
    return spiHardwareMap[device].errorCount;
}

void spiResetErrorCounter(SPI_TypeDef *instance)
{
    SPIDevice device = spiDeviceByInstance(instance);
    if (device != SPIINVALID) {
        spiHardwareMap[device].errorCount = 0;
    }
}

SPI_TypeDef * spiInstanceByDevice(SPIDevice device)
{
    return spiHardwareMap[device].dev;
}
#endif // USE_SPI
