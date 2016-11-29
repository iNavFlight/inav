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

#include "bus.h"
#include "bus_spi.h"
#include "exti.h"
#include "io.h"
#include "io_impl.h"
#include "rcc.h"

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

typedef struct SPIDevice_s {
    SPI_TypeDef *dev;
    ioTag_t nss;
    ioTag_t sck;
    ioTag_t mosi;
    ioTag_t miso;
    rccPeriphTag_t rcc;
    uint8_t af;
    bool sdcard;
    bool nrf24l01;
    volatile uint16_t errorCount;
} spiDevice_t;

static spiDevice_t spiHardwareMap[] = {
#if defined(STM32F1)
    { .dev = SPI1, .nss = IO_TAG(SPI1_NSS_PIN), .sck = IO_TAG(SPI1_SCK_PIN), .miso = IO_TAG(SPI1_MISO_PIN), .mosi = IO_TAG(SPI1_MOSI_PIN), .rcc = RCC_APB2(SPI1), .af = 0 },
    { .dev = SPI2, .nss = IO_TAG(SPI2_NSS_PIN), .sck = IO_TAG(SPI2_SCK_PIN), .miso = IO_TAG(SPI2_MISO_PIN), .mosi = IO_TAG(SPI2_MOSI_PIN), .rcc = RCC_APB1(SPI2), .af = 0 },
#else
    { .dev = SPI1, .nss = IO_TAG(SPI1_NSS_PIN), .sck = IO_TAG(SPI1_SCK_PIN), .miso = IO_TAG(SPI1_MISO_PIN), .mosi = IO_TAG(SPI1_MOSI_PIN), .rcc = RCC_APB2(SPI1), .af = GPIO_AF_SPI1 },
    { .dev = SPI2, .nss = IO_TAG(SPI2_NSS_PIN), .sck = IO_TAG(SPI2_SCK_PIN), .miso = IO_TAG(SPI2_MISO_PIN), .mosi = IO_TAG(SPI2_MOSI_PIN), .rcc = RCC_APB1(SPI2), .af = GPIO_AF_SPI2 },
#endif
#if defined(STM32F3) || defined(STM32F4)
    { .dev = SPI3, .nss = IO_TAG(SPI3_NSS_PIN), .sck = IO_TAG(SPI3_SCK_PIN), .miso = IO_TAG(SPI3_MISO_PIN), .mosi = IO_TAG(SPI3_MOSI_PIN), .rcc = RCC_APB1(SPI3), .af = GPIO_AF_SPI3 }
#endif
};

typedef enum SPIDevice {
    SPIINVALID = -1,
    SPIDEV_1   = 0,
    SPIDEV_2,
    SPIDEV_3,
    SPIDEV_MAX = SPIDEV_3,
} SPIDevice;

static SPIDevice spiDeviceByInstance(SPI_TypeDef *instance)
{
    if (instance == SPI1)
        return SPIDEV_1;

    if (instance == SPI2)
        return SPIDEV_2;

    if (instance == SPI3)
        return SPIDEV_3;

    return SPIINVALID;
}

static void spiInitDevice(SPIDevice device)
{
    SPI_InitTypeDef spiInit;

    spiDevice_t *spi = &(spiHardwareMap[device]);

#ifdef SDCARD_SPI_INSTANCE
    if (spi->dev == SDCARD_SPI_INSTANCE) {
        spi->sdcard = true;
    }
#endif
#ifdef RX_SPI_INSTANCE
    if (spi->dev == RX_SPI_INSTANCE) {
        spi->nrf24l01 = true;
    }
#endif

    // Enable SPI clock
    RCC_ClockCmd(spi->rcc, ENABLE);
    RCC_ResetCmd(spi->rcc, ENABLE);

    IOInit(IOGetByTag(spi->sck),  OWNER_SPI, RESOURCE_SPI_SCK,  device + 1);
    IOInit(IOGetByTag(spi->miso), OWNER_SPI, RESOURCE_SPI_MISO, device + 1);
    IOInit(IOGetByTag(spi->mosi), OWNER_SPI, RESOURCE_SPI_MOSI, device + 1);

#if defined(STM32F3) || defined(STM32F4)
    if (spi->sdcard || spi->nrf24l01) {
        IOConfigGPIOAF(IOGetByTag(spi->sck),  SPI_IO_AF_SCK_CFG, spi->af);
        IOConfigGPIOAF(IOGetByTag(spi->miso), SPI_IO_AF_MISO_CFG, spi->af);
        IOConfigGPIOAF(IOGetByTag(spi->mosi), SPI_IO_AF_CFG, spi->af);
    }
    else {
        IOConfigGPIOAF(IOGetByTag(spi->sck),  SPI_IO_AF_CFG, spi->af);
        IOConfigGPIOAF(IOGetByTag(spi->miso), SPI_IO_AF_CFG, spi->af);
        IOConfigGPIOAF(IOGetByTag(spi->mosi), SPI_IO_AF_CFG, spi->af);
    }

    if (spi->nss) {
        IOConfigGPIOAF(IOGetByTag(spi->nss), SPI_IO_CS_CFG, spi->af);
    }
#endif
#if defined(STM32F10X)
    IOConfigGPIO(IOGetByTag(spi->sck), SPI_IO_AF_SCK_CFG);
    IOConfigGPIO(IOGetByTag(spi->miso), SPI_IO_AF_MISO_CFG);
    IOConfigGPIO(IOGetByTag(spi->mosi), SPI_IO_AF_MOSI_CFG);

    if (spi->nss) {
        IOConfigGPIO(IOGetByTag(spi->nss), SPI_IO_CS_CFG);
    }
#endif

    // Init SPI hardware
    SPI_I2S_DeInit(spi->dev);

    spiInit.SPI_Mode = SPI_Mode_Master;
    spiInit.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    spiInit.SPI_DataSize = SPI_DataSize_8b;
    spiInit.SPI_NSS = SPI_NSS_Soft;
    spiInit.SPI_FirstBit = SPI_FirstBit_MSB;
    spiInit.SPI_CRCPolynomial = 7;
    spiInit.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;

    if (spi->sdcard || spi->nrf24l01) {
        spiInit.SPI_CPOL = SPI_CPOL_Low;
        spiInit.SPI_CPHA = SPI_CPHA_1Edge;
    } else {
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
}

static uint32_t spiTimeoutUserCallback(SPI_TypeDef *instance)
{
    SPIDevice device = spiDeviceByInstance(instance);
    if (device == SPIINVALID)
        return -1;
    spiHardwareMap[device].errorCount++;
    return spiHardwareMap[device].errorCount;
}

#define SPI_TIMEOUT_US      50
bool spiBusTransfer(SPI_TypeDef * instance, BusTransaction_t * txn, uint32_t currentTime)
{
    uint8_t b;

    if (txn->busBytesRemaining == 0)
        return true;

    switch(txn->busSequence) {
        case 0:
            instance->DR;
            txn->busTimeoutUs = currentTime;
            txn->busSequence++;

        case 1:
            if (SPI_I2S_GetFlagStatus(instance, SPI_I2S_FLAG_TXE) == RESET) {
                if ((currentTime - txn->busTimeoutUs) > SPI_TIMEOUT_US) {
                    spiTimeoutUserCallback(instance);
                    return true;
                }

                return false;
            }

            b = txn->busTxBufPtr ? *(txn->busTxBufPtr++) : 0xFF;
#ifdef STM32F303xC
            SPI_SendData8(instance, b);
#else
            SPI_I2S_SendData(instance, b);
#endif
            txn->busTimeoutUs = currentTime;
            txn->busSequence++;

        case 2:
            if (SPI_I2S_GetFlagStatus(instance, SPI_I2S_FLAG_RXNE) == RESET) {
                if ((currentTime - txn->busTimeoutUs) > SPI_TIMEOUT_US) {
                    spiTimeoutUserCallback(instance);
                    return true;
                }

                return false;
            }

#ifdef STM32F303xC
            b = SPI_ReceiveData8(instance);
#else
            b = SPI_I2S_ReceiveData(instance);
#endif
            if (txn->busRxBufPtr) {
                *(txn->busRxBufPtr++) = b;
            }
            txn->busBytesRemaining--;
            txn->busBytesCompleted++;
            txn->busSequence++;

        case 3:
            if (txn->busBytesRemaining > 0) {
                txn->busSequence = 1;
            }
            else {
                return true;
            }
    }

    return false;
}

bool busSpiInit(const void * hwDesc)
{
    SPIDevice spiDev = spiDeviceByInstance((SPI_TypeDef *) hwDesc);

    if (spiDev == SPIINVALID) {
        return false;
    }

    spiInitDevice(spiDev);
    return true;
}

void busSpiProcessTxn(const void * hwDesc, BusTransaction_t * txn, uint32_t currentTime)
{
    switch (txn->state) {
        case TXN_QUEUED:
        case TXN_BUSY_SETUP:
            IOLo((IO_t)txn->device);

            if (txn->command_bytes != 0) {
                busSetupTransfer(txn, NULL, txn->command, txn->command_bytes);
                txn->state = TXN_BUSY_COMMAND;
            }
            else if (txn->payload_bytes != 0) {
                if (txn->type == BUS_READ) {
                    busSetupTransfer(txn, txn->payload, NULL, txn->payload_bytes);
                }
                else {
                    busSetupTransfer(txn, NULL, txn->payload, txn->payload_bytes);
                }
                txn->state = TXN_BUSY_PAYLOAD;
            }
            else {
                txn->state = TXN_BUSY_COMPLETE;
            }
            break;

        case TXN_BUSY_COMMAND:
            if (spiBusTransfer((SPI_TypeDef *)hwDesc, txn, currentTime)) {
                if (txn->payload_bytes != 0) {
                    if (txn->type == BUS_READ) {
                        busSetupTransfer(txn, txn->payload, NULL, txn->payload_bytes);
                    }
                    else {
                        busSetupTransfer(txn, NULL, txn->payload, txn->payload_bytes);
                    }
                    txn->state = TXN_BUSY_PAYLOAD;
                }
                else {
                    txn->state = TXN_BUSY_COMPLETE;
                }
            }
            break;

        case TXN_BUSY_PAYLOAD:
            if (spiBusTransfer((SPI_TypeDef *)hwDesc, txn, currentTime)) {
                txn->completed_bytes = txn->busBytesCompleted;
                txn->state = TXN_BUSY_COMPLETE;
            }
            break;

        case TXN_BUSY_COMPLETE:
            IOHi((IO_t)txn->device);
            txn->state = TXN_DONE;
            break;

        default:    // Driver shouldn't receive TXN descriptor which is IDLE -> do nothing
            break;
    }
}

void busSpiSetSpeed(const void * hwDesc, const BusSpeed_e speed)
{
#define BR_CLEAR_MASK 0xFFC7

    SPI_TypeDef * instance = (SPI_TypeDef *)hwDesc;
    uint16_t tempRegister;

    SPI_Cmd(instance, DISABLE);
    tempRegister = instance->CR1;

    switch (speed) {
        case BUS_SPEED_LOWEST:
            tempRegister &= BR_CLEAR_MASK;
            tempRegister |= SPI_BaudRatePrescaler_256;
            break;
#if defined(STM32F4)
        case BUS_SPEED_SLOW:
            tempRegister &= BR_CLEAR_MASK;
            tempRegister |= SPI_BaudRatePrescaler_128;  //00.65625 MHz
            break;
        case BUS_SPEED_STANDARD:
            tempRegister &= BR_CLEAR_MASK;
            tempRegister |= SPI_BaudRatePrescaler_8;    //10.50000 MHz
            break;
        case BUS_SPEED_FAST:
            tempRegister &= BR_CLEAR_MASK;
            tempRegister |= SPI_BaudRatePrescaler_4;    //21.00000 MHz
            break;
        case BUS_SPEED_ULTRAFAST:
            tempRegister &= BR_CLEAR_MASK;
            tempRegister |= SPI_BaudRatePrescaler_2;    //42.00000 MHz
            break;
#else
        case BUS_SPEED_SLOW:
            tempRegister &= BR_CLEAR_MASK;
            tempRegister |= SPI_BaudRatePrescaler_128;  //00.56250 MHz
            break;
        case BUS_SPEED_STANDARD:
            tempRegister &= BR_CLEAR_MASK;
            tempRegister |= SPI_BaudRatePrescaler_4;    //09.00000 MHz
            break;
        case BUS_SPEED_FAST:
            tempRegister &= BR_CLEAR_MASK;
            tempRegister |= SPI_BaudRatePrescaler_2;    //18.00000 MHz
            break;
        case BUS_SPEED_ULTRAFAST:
            tempRegister &= BR_CLEAR_MASK;
            tempRegister |= SPI_BaudRatePrescaler_2;    //18.00000 MHz
            break;
#endif
    }

    instance->CR1 = tempRegister;

    SPI_Cmd(instance, ENABLE);
}
