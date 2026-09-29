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

#include "build/atomic.h"

#include "drivers/bus.h"
#include "drivers/bus_spi.h"
#include "drivers/bus_spi_data_ready_impl.h"
#include "drivers/time.h"
#include "dma.h"
#include "drivers/io.h"
#include "io_impl.h"
#include "drivers/nvic.h"
#include "drivers/time.h"
#include "rcc.h"

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

#ifndef SPI4_SCK_PIN
#define SPI4_NSS_PIN    PA15
#define SPI4_SCK_PIN    PB3
#define SPI4_MISO_PIN   PB4
#define SPI4_MOSI_PIN   PB5
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
#ifndef SPI4_NSS_PIN
#define SPI4_NSS_PIN NONE
#endif

#if defined(USE_SPI_DEVICE_1)
static const uint32_t spiDivisorMapFast[] = {
    LL_SPI_BAUDRATEPRESCALER_DIV256,    // SPI_CLOCK_INITIALIZATON      421.875 KBits/s
    LL_SPI_BAUDRATEPRESCALER_DIV32,     // SPI_CLOCK_SLOW               843.75 KBits/s
    LL_SPI_BAUDRATEPRESCALER_DIV16,     // SPI_CLOCK_STANDARD           6.75 MBits/s
    LL_SPI_BAUDRATEPRESCALER_DIV8,      // SPI_CLOCK_FAST               13.5 MBits/s
    LL_SPI_BAUDRATEPRESCALER_DIV4       // SPI_CLOCK_ULTRAFAST          27.0 MBits/s
};
#endif

#if defined(USE_SPI_DEVICE_2) || defined(USE_SPI_DEVICE_3) || defined(USE_SPI_DEVICE_4)
static const uint32_t spiDivisorMapSlow[] = {
    LL_SPI_BAUDRATEPRESCALER_DIV256,    // SPI_CLOCK_INITIALIZATON      210.937 KBits/s
    LL_SPI_BAUDRATEPRESCALER_DIV64,     // SPI_CLOCK_SLOW               843.75 KBits/s
    LL_SPI_BAUDRATEPRESCALER_DIV8,      // SPI_CLOCK_STANDARD           6.75 MBits/s
    LL_SPI_BAUDRATEPRESCALER_DIV4,      // SPI_CLOCK_FAST               13.5 MBits/s
    LL_SPI_BAUDRATEPRESCALER_DIV2       // SPI_CLOCK_ULTRAFAST          27.0 MBits/s
};
#endif

#if defined(STM32H7)
#include "bus_spi_stm32h7xx.h"

// Auto-resolve SPI AF per pin from the lookup table in bus_spi_stm32h7xx.h.
// Targets may still define individual SPI*_SCK/MISO/MOSI_AF values in target.h
// to override; explicit defines take priority via these #ifndef guards.
#ifdef USE_SPI_DEVICE_1
#ifndef SPI1_SCK_AF
#define SPI1_SCK_AF    SPI_PIN_AF_HELPER(1, SPI1_SCK_PIN)
#endif
#ifndef SPI1_MISO_AF
#define SPI1_MISO_AF   SPI_PIN_AF_HELPER(1, SPI1_MISO_PIN)
#endif
#ifndef SPI1_MOSI_AF
#define SPI1_MOSI_AF   SPI_PIN_AF_HELPER(1, SPI1_MOSI_PIN)
#endif
#endif

#ifdef USE_SPI_DEVICE_2
#ifndef SPI2_SCK_AF
#define SPI2_SCK_AF    SPI_PIN_AF_HELPER(2, SPI2_SCK_PIN)
#endif
#ifndef SPI2_MISO_AF
#define SPI2_MISO_AF   SPI_PIN_AF_HELPER(2, SPI2_MISO_PIN)
#endif
#ifndef SPI2_MOSI_AF
#define SPI2_MOSI_AF   SPI_PIN_AF_HELPER(2, SPI2_MOSI_PIN)
#endif
#endif

#ifdef USE_SPI_DEVICE_3
#ifndef SPI3_SCK_AF
#define SPI3_SCK_AF    SPI_PIN_AF_HELPER(3, SPI3_SCK_PIN)
#endif
#ifndef SPI3_MISO_AF
#define SPI3_MISO_AF   SPI_PIN_AF_HELPER(3, SPI3_MISO_PIN)
#endif
#ifndef SPI3_MOSI_AF
#define SPI3_MOSI_AF   SPI_PIN_AF_HELPER(3, SPI3_MOSI_PIN)
#endif
#endif

#ifdef USE_SPI_DEVICE_4
#ifndef SPI4_SCK_AF
#define SPI4_SCK_AF    SPI_PIN_AF_HELPER(4, SPI4_SCK_PIN)
#endif
#ifndef SPI4_MISO_AF
#define SPI4_MISO_AF   SPI_PIN_AF_HELPER(4, SPI4_MISO_PIN)
#endif
#ifndef SPI4_MOSI_AF
#define SPI4_MOSI_AF   SPI_PIN_AF_HELPER(4, SPI4_MOSI_PIN)
#endif
#endif

static spiDevice_t spiHardwareMap[SPIDEV_COUNT] = {
#ifdef USE_SPI_DEVICE_1
    { .dev = SPI1, .nss = IO_TAG(SPI1_NSS_PIN), .sck = IO_TAG(SPI1_SCK_PIN), .miso = IO_TAG(SPI1_MISO_PIN), .mosi = IO_TAG(SPI1_MOSI_PIN), .rcc = RCC_APB2(SPI1), .sckAF = SPI1_SCK_AF, .misoAF = SPI1_MISO_AF, .mosiAF = SPI1_MOSI_AF, .divisorMap = spiDivisorMapFast },
#else
    { .dev = NULL },    // No SPI1
#endif

#ifdef USE_SPI_DEVICE_2
    { .dev = SPI2, .nss = IO_TAG(SPI2_NSS_PIN), .sck = IO_TAG(SPI2_SCK_PIN), .miso = IO_TAG(SPI2_MISO_PIN), .mosi = IO_TAG(SPI2_MOSI_PIN), .rcc = RCC_APB1L(SPI2), .sckAF = SPI2_SCK_AF, .misoAF = SPI2_MISO_AF, .mosiAF = SPI2_MOSI_AF, .divisorMap = spiDivisorMapSlow },
#else
    { .dev = NULL },    // No SPI2
#endif

#ifdef USE_SPI_DEVICE_3
    { .dev = SPI3, .nss = IO_TAG(SPI3_NSS_PIN), .sck = IO_TAG(SPI3_SCK_PIN), .miso = IO_TAG(SPI3_MISO_PIN), .mosi = IO_TAG(SPI3_MOSI_PIN), .rcc = RCC_APB1L(SPI3), .sckAF = SPI3_SCK_AF, .misoAF = SPI3_MISO_AF, .mosiAF = SPI3_MOSI_AF, .divisorMap = spiDivisorMapSlow },
#else
    { .dev = NULL },    // No SPI3
#endif

#ifdef USE_SPI_DEVICE_4
    { .dev = SPI4, .nss = IO_TAG(SPI4_NSS_PIN), .sck = IO_TAG(SPI4_SCK_PIN), .miso = IO_TAG(SPI4_MISO_PIN), .mosi = IO_TAG(SPI4_MOSI_PIN), .rcc = RCC_APB2(SPI4), .sckAF = SPI4_SCK_AF, .misoAF = SPI4_MISO_AF, .mosiAF = SPI4_MOSI_AF, .divisorMap = spiDivisorMapSlow }
#else
    { .dev = NULL }     // No SPI4
#endif
};
#elif defined(STM32F7)
#include "bus_spi_stm32f7xx.h"

// Auto-resolve SPI AF per pin from the lookup table in bus_spi_stm32f7xx.h.
// Targets may still define individual SPI*_SCK/MISO/MOSI_AF values in target.h
// to override; explicit defines take priority via these #ifndef guards.
#ifdef USE_SPI_DEVICE_1
#ifndef SPI1_SCK_AF
#define SPI1_SCK_AF    SPI_PIN_AF_HELPER(1, SPI1_SCK_PIN)
#endif
#ifndef SPI1_MISO_AF
#define SPI1_MISO_AF   SPI_PIN_AF_HELPER(1, SPI1_MISO_PIN)
#endif
#ifndef SPI1_MOSI_AF
#define SPI1_MOSI_AF   SPI_PIN_AF_HELPER(1, SPI1_MOSI_PIN)
#endif
#endif

#ifdef USE_SPI_DEVICE_2
#ifndef SPI2_SCK_AF
#define SPI2_SCK_AF    SPI_PIN_AF_HELPER(2, SPI2_SCK_PIN)
#endif
#ifndef SPI2_MISO_AF
#define SPI2_MISO_AF   SPI_PIN_AF_HELPER(2, SPI2_MISO_PIN)
#endif
#ifndef SPI2_MOSI_AF
#define SPI2_MOSI_AF   SPI_PIN_AF_HELPER(2, SPI2_MOSI_PIN)
#endif
#endif

#ifdef USE_SPI_DEVICE_3
#ifndef SPI3_SCK_AF
#define SPI3_SCK_AF    SPI_PIN_AF_HELPER(3, SPI3_SCK_PIN)
#endif
#ifndef SPI3_MISO_AF
#define SPI3_MISO_AF   SPI_PIN_AF_HELPER(3, SPI3_MISO_PIN)
#endif
#ifndef SPI3_MOSI_AF
#define SPI3_MOSI_AF   SPI_PIN_AF_HELPER(3, SPI3_MOSI_PIN)
#endif
#endif

#ifdef USE_SPI_DEVICE_4
#ifndef SPI4_SCK_AF
#define SPI4_SCK_AF    SPI_PIN_AF_HELPER(4, SPI4_SCK_PIN)
#endif
#ifndef SPI4_MISO_AF
#define SPI4_MISO_AF   SPI_PIN_AF_HELPER(4, SPI4_MISO_PIN)
#endif
#ifndef SPI4_MOSI_AF
#define SPI4_MOSI_AF   SPI_PIN_AF_HELPER(4, SPI4_MOSI_PIN)
#endif
#endif

static spiDevice_t spiHardwareMap[] = {
#ifdef USE_SPI_DEVICE_1
    { .dev = SPI1, .nss = IO_TAG(SPI1_NSS_PIN), .sck = IO_TAG(SPI1_SCK_PIN), .miso = IO_TAG(SPI1_MISO_PIN), .mosi = IO_TAG(SPI1_MOSI_PIN), .rcc = RCC_APB2(SPI1), .sckAF = SPI1_SCK_AF, .misoAF = SPI1_MISO_AF, .mosiAF = SPI1_MOSI_AF, .divisorMap = spiDivisorMapFast },
#else
    { .dev = NULL },    // No SPI1
#endif

#ifdef USE_SPI_DEVICE_2
    { .dev = SPI2, .nss = IO_TAG(SPI2_NSS_PIN), .sck = IO_TAG(SPI2_SCK_PIN), .miso = IO_TAG(SPI2_MISO_PIN), .mosi = IO_TAG(SPI2_MOSI_PIN), .rcc = RCC_APB1(SPI2), .sckAF = SPI2_SCK_AF, .misoAF = SPI2_MISO_AF, .mosiAF = SPI2_MOSI_AF, .divisorMap = spiDivisorMapSlow },
#else
    { .dev = NULL },    // No SPI2
#endif

#ifdef USE_SPI_DEVICE_3
    { .dev = SPI3, .nss = IO_TAG(SPI3_NSS_PIN), .sck = IO_TAG(SPI3_SCK_PIN), .miso = IO_TAG(SPI3_MISO_PIN), .mosi = IO_TAG(SPI3_MOSI_PIN), .rcc = RCC_APB1(SPI3), .sckAF = SPI3_SCK_AF, .misoAF = SPI3_MISO_AF, .mosiAF = SPI3_MOSI_AF, .divisorMap = spiDivisorMapSlow },
#else
    { .dev = NULL },    // No SPI3
#endif

#ifdef USE_SPI_DEVICE_4
    { .dev = SPI4, .nss = IO_TAG(SPI4_NSS_PIN), .sck = IO_TAG(SPI4_SCK_PIN), .miso = IO_TAG(SPI4_MISO_PIN), .mosi = IO_TAG(SPI4_MOSI_PIN), .rcc = RCC_APB2(SPI4), .sckAF = SPI4_SCK_AF, .misoAF = SPI4_MISO_AF, .mosiAF = SPI4_MOSI_AF, .divisorMap = spiDivisorMapSlow }
#else
    { .dev = NULL }     // No SPI4
#endif
};
#endif

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
    spiDevice_t *spi = &(spiHardwareMap[device]);

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
        IOConfigGPIOAF(IOGetByTag(spi->sck), SPI_IO_AF_SCK_CFG_LOW, spi->sckAF);
    } else {
        IOConfigGPIOAF(IOGetByTag(spi->sck), SPI_IO_AF_SCK_CFG_HIGH, spi->sckAF);
    }
    IOConfigGPIOAF(IOGetByTag(spi->miso), SPI_IO_AF_MISO_CFG, spi->misoAF);

    IOConfigGPIOAF(IOGetByTag(spi->mosi), SPI_IO_AF_CFG, spi->mosiAF);

    if (spi->nss) {
        IOInit(IOGetByTag(spi->nss),  OWNER_SPI, RESOURCE_SPI_CS,  device + 1);
        IOConfigGPIO(IOGetByTag(spi->nss), SPI_IO_CS_CFG);
    }

    LL_SPI_Disable(spi->dev);
    LL_SPI_DeInit(spi->dev);

    LL_SPI_InitTypeDef init =
    {
        .TransferDirection = SPI_DIRECTION_2LINES,
        .Mode = SPI_MODE_MASTER,
        .DataWidth = SPI_DATASIZE_8BIT,
        .ClockPolarity = leadingEdge ? SPI_POLARITY_LOW : SPI_POLARITY_HIGH,
        .ClockPhase = leadingEdge ? SPI_PHASE_1EDGE : SPI_PHASE_2EDGE,
        .NSS = SPI_NSS_SOFT,
        .BaudRate = SPI_BAUDRATEPRESCALER_8,
        .BitOrder = SPI_FIRSTBIT_MSB,
        .CRCPoly = 7,
        .CRCCalculation = SPI_CRCCALCULATION_DISABLE,
    };

#if defined(STM32H7)
    // Prevent glitching when SPI is disabled
    LL_SPI_EnableGPIOControl(spi->dev);

    LL_SPI_SetFIFOThreshold(spi->dev, LL_SPI_FIFO_TH_01DATA);
    LL_SPI_Init(spi->dev, &init);
#else
    LL_SPI_SetRxFIFOThreshold(spi->dev, SPI_RXFIFO_THRESHOLD_QF);

    LL_SPI_Init(spi->dev, &init);
    LL_SPI_Enable(spi->dev);

    SET_BIT(spi->dev->CR2, SPI_RXFIFO_THRESHOLD);
#endif

    if (spi->nss) {
        IOHi(IOGetByTag(spi->nss));
    }

    spi->initDone = true;
    return true;
}

uint8_t spiTransferByte(SPI_TypeDef *instance, uint8_t txByte)
{
    uint8_t value = 0xFF;
    if (!spiTransfer(instance, &value, &txByte, 1)) {
        return 0xFF;
    }
    return value;
}

/**
 * Return true if the bus is currently in the middle of a transmission.
 */
bool spiIsBusBusy(SPI_TypeDef *instance)
{
#if defined(STM32H7)
    UNUSED(instance);
    // H7 doesnt really have a busy flag. its should be done when the transfer is.
    return false;
#else
    return (LL_SPI_GetTxFIFOLevel(instance) != LL_SPI_TX_FIFO_EMPTY) || LL_SPI_IsActiveFlag_BSY(instance);
#endif
}

// Bytes sent ahead of those read back: two keep the clock running, more measured no faster. The
// RX FIFO (4 bytes on F7, 8 or 16 on H7) cannot overflow, and the clock stops when nothing is left
#define SPI_BYTES_IN_FLIGHT 2

#if defined(STM32H7)
#define SPI_TX_READY(instance)  LL_SPI_IsActiveFlag_TXP(instance)
#define SPI_RX_READY(instance)  LL_SPI_IsActiveFlag_RXP(instance)
#else
#define SPI_TX_READY(instance)  LL_SPI_IsActiveFlag_TXE(instance)
#define SPI_RX_READY(instance)  LL_SPI_IsActiveFlag_RXNE(instance)
#endif

// A transfer that does not finish is abandoned leaving the SPI ready for the next one
static void spiTransferAbort(SPI_TypeDef *instance)
{
#if defined(STM32H7)
    // Left enabled, the SPI would refuse the next transfer's size, and every later one would fail
    LL_SPI_Disable(instance);
    WRITE_REG(instance->IFCR, SPI_IFCR_EOTC | SPI_IFCR_TXTFC | SPI_IFCR_UDRC | SPI_IFCR_OVRC |
              SPI_IFCR_CRCEC | SPI_IFCR_TIFREC | SPI_IFCR_MODFC | SPI_IFCR_TSERFC | SPI_IFCR_SUSPC);
#else
    // F7 stays enabled: drain what is left, or the next transfer would take it for its own.
    // Timed, not counted: the 5 bytes still queued take 0.2 ms at the slowest clock
    const timeUs_t start = micros();
    while ((LL_SPI_GetTxFIFOLevel(instance) != LL_SPI_TX_FIFO_EMPTY || LL_SPI_IsActiveFlag_BSY(instance))
        && cmpTimeUs(micros(), start) < 1000) {
    }
    for (int n = 0; n < 8 && LL_SPI_GetRxFIFOLevel(instance) != LL_SPI_RX_FIFO_EMPTY; n++) {
        (void)LL_SPI_ReceiveData8(instance);
    }
#endif
    spiTimeoutUserCallback(instance);
}

// The prefix byte (a register address, echo dropped), then len bytes, each sent while the
// earlier ones are still coming back, so the clock never stops
static bool spiTransferBackToBack(SPI_TypeDef *instance, const uint8_t *prefix, uint8_t *rxData, const uint8_t *txData, int len)
{
    const int total = len + (prefix ? 1 : 0);
    if (total == 0) {
        // On H7 a transfer size of 0 would mean one with no end
        return true;
    }

    int toSend = total;
    int toReceive = total;
    int spiTimeout = 1000;

#if defined(STM32H7)
    // The size is taken only while the SPI is disabled
    LL_SPI_Disable(instance);
    LL_SPI_SetTransferSize(instance, total);
    LL_SPI_Enable(instance);
    LL_SPI_StartMasterTransfer(instance);
#else
    SET_BIT(instance->CR2, SPI_RXFIFO_THRESHOLD);
#endif

    while (toReceive) {
        bool progress = false;

        if (toSend && toReceive - toSend < SPI_BYTES_IN_FLIGHT && SPI_TX_READY(instance)) {
            uint8_t b;
            if (prefix && toSend == total) {
                b = *prefix;
            } else {
                b = txData ? *(txData++) : 0xFF;
            }
            LL_SPI_TransmitData8(instance, b);
            toSend--;
            progress = true;
        }

        if (SPI_RX_READY(instance)) {
            const uint8_t b = LL_SPI_ReceiveData8(instance);
            if (rxData && !(prefix && toReceive == total)) {
                *(rxData++) = b;
            }
            toReceive--;
            progress = true;
        }

        if (progress) {
            spiTimeout = 1000;
        } else if ((spiTimeout--) == 0) {
            spiTransferAbort(instance);
            return false;
        }
    }

#if defined(STM32H7)
    // Bounded: a transfer that never ends must not hang the caller
    spiTimeout = 1000;
    while (!LL_SPI_IsActiveFlag_EOT(instance)) {
        if ((spiTimeout--) == 0) {
            spiTransferAbort(instance);
            return false;
        }
    }
    LL_SPI_ClearFlag_TXTF(instance);
    LL_SPI_Disable(instance);
#endif

    return true;
}

bool spiTransferRegister(SPI_TypeDef *instance, uint8_t reg, uint8_t *rxData, const uint8_t *txData, int len)
{
    return spiTransferBackToBack(instance, &reg, rxData, txData, len);
}

bool spiTransfer(SPI_TypeDef *instance, uint8_t *rxData, const uint8_t *txData, int len)
{
    return spiTransferBackToBack(instance, NULL, rxData, txData, len);
}

#if defined(USE_SPI_DATA_READY)
// Data-ready reads (see bus_spi_data_ready.c). On H7 a read that fits the SPI FIFO (16 bytes on
// SPI1-3, 8 on SPI4-6) needs no DMA: the end-of-transfer interrupt takes it all back. Longer
// reads, and all on F7 (4-byte FIFO), use two DMA streams

// Longest read, address included
#define SPI_DATA_READY_MAX  32

typedef struct {
    uint8_t reg;                        // the first register, as sent (read bit included)
    uint8_t len;                        // bytes after the address
    bool dma;
    spiDataReadyDma_t streams;
} spiDataReadyHw_t;

static spiDataReadyHw_t spiDataReadyHw[SPIDEV_COUNT];

// In memory the DMA can reach
static DMA_RAM uint8_t spiDataReadyTx[SPIDEV_COUNT][SPI_DATA_READY_MAX] __attribute__((aligned(32)));
static DMA_RAM uint8_t spiDataReadyRx[SPIDEV_COUNT][SPI_DATA_READY_MAX] __attribute__((aligned(32)));

static void spiDataReadyDmaDone(DMA_t rx)
{
    const SPIDevice device = (SPIDevice)rx->userParam;
    if (!spiDataReadyDmaEnded(rx)) {
        // The read never ends: the main loop takes it off the bus when it next needs it
        return;
    }

    uint8_t *buf = spiDataReadyRx[device];
    SCB_InvalidateDCache_by_Addr((uint32_t *)buf, SPI_DATA_READY_MAX);
    // A copy: a data-ready that came meanwhile starts the next read into the same buffer
    uint8_t data[SPI_DATA_READY_MAX];
    memcpy(data, buf + 1, spiDataReadyHw[device].len);
    spiDataReadyDone(device, data);
}

static bool spiDataReadyDmaSetup(SPIDevice device)
{
    spiDataReadyHw_t *h = &spiDataReadyHw[device];

    if (!spiDataReadyDmaInit(device, &h->streams, spiDataReadyDmaDone, device)) {
        return false;
    }
    memset(spiDataReadyTx[device], 0xFF, SPI_DATA_READY_MAX);
    spiDataReadyTx[device][0] = h->reg;
    // A no-op while DMA_RAM stays uncached, as the invalidate on the reply is
    SCB_CleanDCache_by_Addr((uint32_t *)spiDataReadyTx[device], SPI_DATA_READY_MAX);
    h->dma = true;
    return true;
}

#if defined(STM32H7)
static const IRQn_Type spiIrq[SPIDEV_COUNT] = { SPI1_IRQn, SPI2_IRQn, SPI3_IRQn, SPI4_IRQn };

static uint8_t spiFifoSize(SPI_TypeDef *instance)
{
    return (instance == SPI1 || instance == SPI2 || instance == SPI3) ? 16 : 8;
}

bool spiDataReadyHwInit(SPIDevice device, uint8_t reg, uint8_t len)
{
    SPI_TypeDef *instance = spiHardwareMap[device].dev;
    spiDataReadyHw_t *h = &spiDataReadyHw[device];
    if (!instance || len + 1 > SPI_DATA_READY_MAX) {
        return false;
    }

    h->reg = reg;
    h->len = len;
    h->dma = false;
    if (len + 1 > spiFifoSize(instance)) {
        return spiDataReadyDmaSetup(device);
    }
    HAL_NVIC_SetPriority(spiIrq[device], NVIC_PRIO_GYRO_DATA_READY, 0);
    HAL_NVIC_EnableIRQ(spiIrq[device]);
    return true;
}

void spiDataReadyHwStart(SPIDevice device)
{
    const spiDataReadyHw_t *h = &spiDataReadyHw[device];
    SPI_TypeDef *instance = spiHardwareMap[device].dev;

    LL_SPI_Disable(instance);
    // A polled transfer leaves EOT set: this read would look over before it began
    WRITE_REG(instance->IFCR, SPI_IFCR_EOTC | SPI_IFCR_TXTFC);
    LL_SPI_SetTransferSize(instance, h->len + 1);
    if (h->dma) {
        spiDataReadyDmaStart(&h->streams, &instance->TXDR, &instance->RXDR, spiDataReadyTx[device], spiDataReadyRx[device], h->len + 1);
        SET_BIT(instance->CFG1, SPI_CFG1_RXDMAEN | SPI_CFG1_TXDMAEN);
        LL_SPI_Enable(instance);
    } else {
        LL_SPI_Enable(instance);
        LL_SPI_TransmitData8(instance, h->reg);
        for (int i = 0; i < h->len; i++) {
            LL_SPI_TransmitData8(instance, 0xFF);
        }
        LL_SPI_EnableIT_EOT(instance);
    }
    LL_SPI_StartMasterTransfer(instance);
}

void spiDataReadyHwStop(SPIDevice device)
{
    const spiDataReadyHw_t *h = &spiDataReadyHw[device];
    SPI_TypeDef *instance = spiHardwareMap[device].dev;

    if (h->dma) {
        spiDataReadyDmaStop(&h->streams);
    } else {
        LL_SPI_DisableIT_EOT(instance);
    }
    LL_SPI_Disable(instance);
    // CFG1 only takes writes with the SPI disabled
    CLEAR_BIT(instance->CFG1, SPI_CFG1_RXDMAEN | SPI_CFG1_TXDMAEN);
    WRITE_REG(instance->IFCR, SPI_IFCR_EOTC | SPI_IFCR_TXTFC | SPI_IFCR_UDRC | SPI_IFCR_OVRC |
              SPI_IFCR_CRCEC | SPI_IFCR_TIFREC | SPI_IFCR_MODFC | SPI_IFCR_TSERFC | SPI_IFCR_SUSPC);
}

void spiDataReadyHwDisable(SPIDevice device)
{
    const spiDataReadyHw_t *h = &spiDataReadyHw[device];
    if (h->dma) {
        spiDataReadyDmaDisable(&h->streams);
    } else {
        HAL_NVIC_DisableIRQ(spiIrq[device]);
    }
}

static void spiDataReadyIrqHandler(SPIDevice device)
{
    SPI_TypeDef *instance = spiHardwareMap[device].dev;

    if (!LL_SPI_IsActiveFlag_EOT(instance)) {
        LL_SPI_DisableIT_EOT(instance);
        return;
    }

    // The whole read is in the receive FIFO: the echo of the address, then the data
    uint8_t data[16];
    (void)LL_SPI_ReceiveData8(instance);
    for (int i = 0; i < spiDataReadyHw[device].len; i++) {
        data[i] = LL_SPI_ReceiveData8(instance);
    }
    spiDataReadyDone(device, data);
}

#ifdef USE_SPI_DEVICE_1
void SPI1_IRQHandler(void) { spiDataReadyIrqHandler(SPIDEV_1); }
#endif
#ifdef USE_SPI_DEVICE_2
void SPI2_IRQHandler(void) { spiDataReadyIrqHandler(SPIDEV_2); }
#endif
#ifdef USE_SPI_DEVICE_3
void SPI3_IRQHandler(void) { spiDataReadyIrqHandler(SPIDEV_3); }
#endif
#ifdef USE_SPI_DEVICE_4
void SPI4_IRQHandler(void) { spiDataReadyIrqHandler(SPIDEV_4); }
#endif

#else // STM32F7

bool spiDataReadyHwInit(SPIDevice device, uint8_t reg, uint8_t len)
{
    SPI_TypeDef *instance = spiHardwareMap[device].dev;
    spiDataReadyHw_t *h = &spiDataReadyHw[device];
    if (!instance || len + 1 > SPI_DATA_READY_MAX) {
        return false;
    }

    h->reg = reg;
    h->len = len;
    return spiDataReadyDmaSetup(device);
}

// Setting the DMA requests starts the read: receive first, as the reference manual has it
void spiDataReadyHwStart(SPIDevice device)
{
    const spiDataReadyHw_t *h = &spiDataReadyHw[device];
    SPI_TypeDef *instance = spiHardwareMap[device].dev;

    // A byte left in the receive FIFO would be taken for the first one of this read
    for (int n = 0; n < 8 && LL_SPI_GetRxFIFOLevel(instance) != LL_SPI_RX_FIFO_EMPTY; n++) {
        (void)LL_SPI_ReceiveData8(instance);
    }
    spiDataReadyDmaStart(&h->streams, &instance->DR, &instance->DR, spiDataReadyTx[device], spiDataReadyRx[device], h->len + 1);
    SET_BIT(instance->CR2, SPI_CR2_RXDMAEN);
    SET_BIT(instance->CR2, SPI_CR2_TXDMAEN);
}

void spiDataReadyHwStop(SPIDevice device)
{
    const spiDataReadyHw_t *h = &spiDataReadyHw[device];
    SPI_TypeDef *instance = spiHardwareMap[device].dev;

    spiDataReadyDmaStop(&h->streams);
    CLEAR_BIT(instance->CR2, SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);

    // A read stopped half way leaves bytes behind: send them and drop what comes back, or the main
    // loop's next transfer would take it for its own
    for (int timeout = 1000; timeout && (LL_SPI_GetTxFIFOLevel(instance) != LL_SPI_TX_FIFO_EMPTY || LL_SPI_IsActiveFlag_BSY(instance)); timeout--);
    for (int n = 0; n < 8 && LL_SPI_GetRxFIFOLevel(instance) != LL_SPI_RX_FIFO_EMPTY; n++) {
        (void)LL_SPI_ReceiveData8(instance);
    }
}

void spiDataReadyHwDisable(SPIDevice device)
{
    spiDataReadyDmaDisable(&spiDataReadyHw[device].streams);
}
#endif
#endif

void spiSetSpeed(SPI_TypeDef *instance, SPIClockSpeed_e speed)
{
    SPIDevice device = spiDeviceByInstance(instance);
    LL_SPI_Disable(instance);
    LL_SPI_SetBaudRatePrescaler(instance, spiHardwareMap[device].divisorMap[speed]);
    // H7 stays disabled: each transfer sets its size, which the SPI takes only while disabled
#if !defined(STM32H7)
    LL_SPI_Enable(instance);
#endif
}

SPI_TypeDef * spiInstanceByDevice(SPIDevice device)
{
    return spiHardwareMap[device].dev;
}
