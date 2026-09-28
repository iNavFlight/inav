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
#include <build/debug.h>

#if defined(USE_SPI)

#include "drivers/io.h"
#include "drivers/bus.h"
#include "drivers/bus_spi.h"
#include "drivers/time.h"

void spiChipSelectSetupDelay(void)
{
    // CS->CLK delay, MPU6000 - 8ns
    delayNanos(8);
}

void spiChipSelectHoldTime(void)
{
    // CLK->CS delay, MPU6000 - 500ns
    delayNanos(500);
}

bool spiBusInitHost(const busDevice_t * dev)
{
    const bool spiLeadingEdge = (dev->flags & DEVFLAGS_SPI_MODE_0);
    return spiInitDevice(dev->busdev.spi.spiBus, spiLeadingEdge);
}

// Select takes the bus, deselect gives it back: data-ready reads take turns with these
void spiBusSelectDevice(const busDevice_t * dev)
{
    spiBusAcquire(dev->busdev.spi.spiBus);
    IOLo(dev->busdev.spi.csnPin);
    spiChipSelectSetupDelay();
}

void spiBusDeselectDevice(const busDevice_t * dev)
{
    spiChipSelectHoldTime();
    IOHi(dev->busdev.spi.csnPin);
    spiBusRelease(dev->busdev.spi.spiBus);
}

void spiBusSetSpeed(const busDevice_t * dev, busSpeed_e speed)
{
    const SPIClockSpeed_e spiClock[] = { SPI_CLOCK_INITIALIZATON, SPI_CLOCK_SLOW, SPI_CLOCK_STANDARD, SPI_CLOCK_FAST, SPI_CLOCK_ULTRAFAST };

#if defined(AT32F43x)
    spi_type * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);
#else
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);
#endif

#ifdef BUS_SPI_SPEED_MAX
    if (speed > BUS_SPI_SPEED_MAX)
        speed = BUS_SPI_SPEED_MAX;
#endif

    spiBusAcquire(dev->busdev.spi.spiBus);
    spiSetSpeed(instance, spiClock[speed]);
    spiBusRelease(dev->busdev.spi.spiBus);
}


bool spiBusTransfer(const busDevice_t * dev, uint8_t * rxBuf, const uint8_t * txBuf, int length)
{
#if defined(AT32F43x)
    spi_type * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);
#else
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);
#endif

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        spiBusSelectDevice(dev);
    }

    spiTransfer(instance, rxBuf, txBuf, length);

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        spiBusDeselectDevice(dev);
    }

    return true;
}

bool spiBusTransferMultiple(const busDevice_t * dev, busTransferDescriptor_t * dsc, int count)
{
#if defined(AT32F43x)
    spi_type * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);
#else
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);
#endif

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        spiBusSelectDevice(dev);
    }

    for (int n = 0; n < count; n++) {
        spiTransfer(instance, dsc[n].rxBuf, dsc[n].txBuf, dsc[n].length);
    }

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        spiBusDeselectDevice(dev);
    }

    return true;
}

// Address and data in one transfer: on H7 starting and ending one costs about as much as the bytes
#if defined(AT32F43x)
static void spiBusTransferRegister(spi_type * instance, uint8_t reg, uint8_t * rxData, const uint8_t * txData, int len)
#else
static void spiBusTransferRegister(SPI_TypeDef * instance, uint8_t reg, uint8_t * rxData, const uint8_t * txData, int len)
#endif
{
#if defined(STM32H7) || defined(STM32F7)
    spiTransferRegister(instance, reg, rxData, txData, len);
#else
    spiTransferByte(instance, reg);
    spiTransfer(instance, rxData, txData, len);
#endif
}

bool spiBusWriteRegister(const busDevice_t * dev, uint8_t reg, uint8_t data)
{
#if defined(AT32F43x)
    spi_type * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);
#else
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);
#endif

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        spiBusSelectDevice(dev);
    }

    spiBusTransferRegister(instance, reg, NULL, &data, 1);

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        spiBusDeselectDevice(dev);
    }

    return true;
}

bool spiBusWriteBuffer(const busDevice_t * dev, uint8_t reg, const uint8_t * data, uint8_t length)
{
#if defined(AT32F43x)
    spi_type * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);
#else
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);
#endif

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        spiBusSelectDevice(dev);
    }

    spiBusTransferRegister(instance, reg, NULL, data, length);

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        spiBusDeselectDevice(dev);
    }

    return true;
}

bool spiBusReadBuffer(const busDevice_t * dev, uint8_t reg, uint8_t * data, uint8_t length)
{
#if defined(AT32F43x)
    spi_type * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);
#else
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);
#endif

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        spiBusSelectDevice(dev);
    }

    spiBusTransferRegister(instance, reg, data, NULL, length);

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        spiBusDeselectDevice(dev);
    }

    return true;
}

bool spiBusReadRegister(const busDevice_t * dev, uint8_t reg, uint8_t * data)
{
#if defined(AT32F43x)
    spi_type * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);
#else
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);
#endif

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        spiBusSelectDevice(dev);
    }

    spiBusTransferRegister(instance, reg, data, NULL, 1);

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        spiBusDeselectDevice(dev);
    }

    return true;
}

bool spiBusIsBusy(const busDevice_t * dev)
{
#if defined(AT32F43x)
    spi_type * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);
#else
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);
#endif
    return spiIsBusBusy(instance);
}
#endif
