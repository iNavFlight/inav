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
 * RP2350 SPI driver — blocking, Pico SDK hardware_spi.
 *
 * Implements the INAV bus_spi.h interface using the Pico SDK blocking API.
 * DMA is not used; all transfers complete before returning.  This is
 * sufficient for sensor bring-up (gyro, baro, flash) and can be extended
 * with DMA in a later milestone.
 *
 * Hardware mapping (Option C pin plan):
 *   INAV SPIDEV_1 → RP2350 spi0 → GP4 (MISO / PA4), GP6 (SCK / PA6), GP7 (MOSI / PA7)
 *   INAV SPIDEV_2 → RP2350 spi1 → configured via SPI2_* defines if needed
 *
 * SPI instance selection: the RP2350 assigns GPIOs to spi0 or spi1 in
 * groups of 8: GP0–7 and GP16–23 → spi0; GP8–15 and GP24–31 → spi1.
 * Formula: ((scl_gpio >> 3) & 1) ? spi1 : spi0.
 *
 * Mode:
 *   leadingEdge = true  → Mode 0 (CPOL=0, CPHA=0) — most sensors
 *   leadingEdge = false → Mode 3 (CPOL=1, CPHA=1) — some NOR flash
 *
 * MISO pull-up: enabled on init to prevent MISO from floating when
 * multiple slaves share the bus and one is deselected.
 *
 * Reference: Betaflight src/platform/PICO/bus_spi_pico.c
 *            INAV src/main/drivers/bus_spi.h
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "platform.h"

#ifdef USE_SPI

#include "common/utils.h"
#include "drivers/bus_spi.h"
#include "drivers/io.h"
#include "drivers/io_def.h"

#include "hardware/gpio.h"
#include "hardware/spi.h"

/* Speed table: SPIClockSpeed_e → baud rate (Hz) */
static const uint32_t spiClockFreq[] = {
    [SPI_CLOCK_INITIALIZATON] =   328125,   /* very slow: safe for all devices at init */
    [SPI_CLOCK_SLOW]          =  1000000,   /* 1 MHz */
    [SPI_CLOCK_STANDARD]      = 10000000,   /* 10 MHz */
    [SPI_CLOCK_FAST]          = 20000000,   /* 20 MHz */
    [SPI_CLOCK_ULTRAFAST]     = 40000000,   /* 40 MHz — RP2350 at 150 MHz easily supports this */
};

typedef struct {
    spi_inst_t       *hw;
    uint              gpio_sck;
    uint              gpio_miso;
    uint              gpio_mosi;
    volatile uint16_t errorCount;
    bool              initialised;
    bool              leadingEdge;
} rp2350SpiState_t;

static rp2350SpiState_t spiState[SPIDEV_COUNT];

/*
 * Map a SCK GPIO number to the RP2350 SPI hardware instance.
 * GP0–7 and GP16–23 → spi0; GP8–15 and GP24–31 → spi1.
 * Formula: group = gpio >> 3; spi1 if group is odd.
 */
static spi_inst_t *gpioToSpiHw(uint gpio)
{
    return ((gpio >> 3u) & 1u) ? spi1 : spi0;
}

static inline uint ioTagToGpio(ioTag_t tag)
{
    return (uint)(DEFIO_TAG_GPIOID(tag) * 16u + DEFIO_TAG_PIN(tag));
}

/* ─── Device ↔ instance mapping ──────────────────────────────────────────── */

SPIDevice spiDeviceByInstance(SPI_TypeDef *instance)
{
    if (instance == SPI1) return SPIDEV_1;
    if (instance == SPI2) return SPIDEV_2;
    return SPIINVALID;
}

SPI_TypeDef *spiInstanceByDevice(SPIDevice device)
{
    switch (device) {
    case SPIDEV_1: return SPI1;
    case SPIDEV_2: return SPI2;
    default:       return NULL;
    }
}

/* ─── Initialisation ─────────────────────────────────────────────────────── */

bool spiInitDevice(SPIDevice device, bool leadingEdge)
{
    if (device < 0 || device >= SPIDEV_COUNT) {
        return false;
    }

    ioTag_t sckTag, misoTag, mosiTag;

    switch (device) {
#ifdef USE_SPI_DEVICE_1
    case SPIDEV_1:
        sckTag  = IO_TAG(SPI1_SCK_PIN);
        misoTag = IO_TAG(SPI1_MISO_PIN);
        mosiTag = IO_TAG(SPI1_MOSI_PIN);
        break;
#endif
#ifdef USE_SPI_DEVICE_2
    case SPIDEV_2:
        sckTag  = IO_TAG(SPI2_SCK_PIN);
        misoTag = IO_TAG(SPI2_MISO_PIN);
        mosiTag = IO_TAG(SPI2_MOSI_PIN);
        break;
#endif
    default:
        return false;
    }

    const uint gpioSck  = ioTagToGpio(sckTag);
    const uint gpioMiso = ioTagToGpio(misoTag);
    const uint gpioMosi = ioTagToGpio(mosiTag);
    spi_inst_t *hw      = gpioToSpiHw(gpioSck);

    gpio_set_function(gpioSck,  GPIO_FUNC_SPI);
    gpio_set_function(gpioMosi, GPIO_FUNC_SPI);
    gpio_set_function(gpioMiso, GPIO_FUNC_SPI);

    /* Pull MISO up: prevents the line from floating when a slave is
     * deselected on a shared bus (e.g. gyro + flash on the same SPI). */
    gpio_pull_up(gpioMiso);

    /* Fast slew on SCK: reduces edge-to-edge time at high data rates. */
    gpio_set_slew_rate(gpioSck, GPIO_SLEW_RATE_FAST);

    /* Initialise hardware at a slow safe rate; callers use spiSetSpeed(). */
    spi_init(hw, spiClockFreq[SPI_CLOCK_INITIALIZATON]);

    spi_set_format(hw, 8,
        leadingEdge ? SPI_CPOL_0 : SPI_CPOL_1,
        leadingEdge ? SPI_CPHA_0 : SPI_CPHA_1,
        SPI_MSB_FIRST);

    spiState[device].hw          = hw;
    spiState[device].gpio_sck    = gpioSck;
    spiState[device].gpio_miso   = gpioMiso;
    spiState[device].gpio_mosi   = gpioMosi;
    spiState[device].leadingEdge = leadingEdge;
    spiState[device].errorCount  = 0;
    spiState[device].initialised = true;

    return true;
}

/* ─── Speed control ──────────────────────────────────────────────────────── */

void spiSetSpeed(SPI_TypeDef *instance, SPIClockSpeed_e speed)
{
    const SPIDevice device = spiDeviceByInstance(instance);
    if (device == SPIINVALID || !spiState[device].initialised) {
        return;
    }
    if ((size_t)speed >= ARRAYLEN(spiClockFreq)) {
        return;
    }
    spi_set_baudrate(spiState[device].hw, spiClockFreq[speed]);
}

/* ─── Transfers ──────────────────────────────────────────────────────────── */

bool spiTransfer(SPI_TypeDef *instance, uint8_t *rxData, const uint8_t *txData, int len)
{
    const SPIDevice device = spiDeviceByInstance(instance);
    if (device == SPIINVALID || !spiState[device].initialised || len <= 0) {
        return false;
    }

    spi_inst_t *hw = spiState[device].hw;

    if (txData && rxData) {
        spi_write_read_blocking(hw, txData, rxData, (size_t)len);
    } else if (txData) {
        spi_write_blocking(hw, txData, (size_t)len);
    } else if (rxData) {
        /* 0xFF keeps MOSI high — required by SD cards and some flash chips. */
        spi_read_blocking(hw, 0xFF, rxData, (size_t)len);
    } else {
        return false;
    }

    return true;
}

uint8_t spiTransferByte(SPI_TypeDef *instance, uint8_t in)
{
    uint8_t out;
    spiTransfer(instance, &out, &in, 1);
    return out;
}

/* ─── Status ─────────────────────────────────────────────────────────────── */

bool spiIsBusBusy(SPI_TypeDef *instance)
{
    /* Blocking implementation — transfers complete before returning. */
    UNUSED(instance);
    return false;
}

uint16_t spiGetErrorCounter(SPI_TypeDef *instance)
{
    const SPIDevice device = spiDeviceByInstance(instance);
    if (device == SPIINVALID) {
        return 0;
    }
    return spiState[device].errorCount;
}

void spiResetErrorCounter(SPI_TypeDef *instance)
{
    const SPIDevice device = spiDeviceByInstance(instance);
    if (device != SPIINVALID) {
        spiState[device].errorCount = 0;
    }
}

#endif /* USE_SPI */
