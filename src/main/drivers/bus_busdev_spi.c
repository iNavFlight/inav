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

bool spiBusInitHost(const busDevice_t * dev)
{
    const bool spiLeadingEdge = (dev->flags & DEVFLAGS_SPI_MODE_0);
    return spiInitDevice(dev->busdev.spi.spiBus, spiLeadingEdge);
}

void spiBusSelectDevice(const busDevice_t * dev)
{
    IOLo(dev->busdev.spi.csnPin);
    __NOP();
}

void spiBusDeselectDevice(const busDevice_t * dev)
{
    __NOP();
    IOHi(dev->busdev.spi.csnPin);
}

void spiBusSetSpeed(const busDevice_t * dev, busSpeed_e speed)
{
    const SPIClockSpeed_e spiClock[] = { SPI_CLOCK_INITIALIZATON, SPI_CLOCK_SLOW, SPI_CLOCK_STANDARD, SPI_CLOCK_FAST, SPI_CLOCK_ULTRAFAST };
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);

#ifdef BUS_SPI_SPEED_MAX
    if (speed > BUS_SPI_SPEED_MAX)
        speed = BUS_SPI_SPEED_MAX;
#endif

    spiSetSpeed(instance, spiClock[speed]);
}


bool spiBusTransfer(const busDevice_t * dev, uint8_t * rxBuf, const uint8_t * txBuf, int length)
{
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        IOLo(dev->busdev.spi.csnPin);
        __NOP();
    }

    spiTransfer(instance, rxBuf, txBuf, length);

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        __NOP();
        IOHi(dev->busdev.spi.csnPin);
    }

    return true;
}

bool spiBusTransferMultiple(const busDevice_t * dev, busTransferDescriptor_t * dsc, int count)
{
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        IOLo(dev->busdev.spi.csnPin);
        __NOP();
    }

    for (int n = 0; n < count; n++) {
        spiTransfer(instance, dsc[n].rxBuf, dsc[n].txBuf, dsc[n].length);
    }

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        __NOP();
        IOHi(dev->busdev.spi.csnPin);
    }

    return true;
}

bool spiBusWriteRegister(const busDevice_t * dev, uint8_t reg, uint8_t data)
{
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        IOLo(dev->busdev.spi.csnPin);
        delayMicroseconds(1);
    }

    spiTransferByte(instance, reg);
    spiTransferByte(instance, data);

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        delayMicroseconds(1);
        IOHi(dev->busdev.spi.csnPin);
    }

    return true;
}

bool spiBusWriteBuffer(const busDevice_t * dev, uint8_t reg, const uint8_t * data, uint8_t length)
{
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        IOLo(dev->busdev.spi.csnPin);
    }

    spiTransferByte(instance, reg);
    spiTransfer(instance, NULL, data, length);

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        IOHi(dev->busdev.spi.csnPin);
    }

    return true;
}

bool spiBusReadBuffer(const busDevice_t * dev, uint8_t reg, uint8_t * data, uint8_t length)
{
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        IOLo(dev->busdev.spi.csnPin);
    }

    spiTransferByte(instance, reg);
    spiTransfer(instance, data, NULL, length);

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        IOHi(dev->busdev.spi.csnPin);
    }

    return true;
}

bool spiBusReadRegister(const busDevice_t * dev, uint8_t reg, uint8_t * data)
{
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        IOLo(dev->busdev.spi.csnPin);
    }

    spiTransferByte(instance, reg);
    spiTransfer(instance, data, NULL, 1);

    if (!(dev->flags & DEVFLAGS_USE_MANUAL_DEVICE_SELECT)) {
        IOHi(dev->busdev.spi.csnPin);
    }

    return true;
}

bool spiBusIsBusy(const busDevice_t * dev)
{
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);
    return spiIsBusBusy(instance);
}
#endif
