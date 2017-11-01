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

#if defined(USE_SPI)

#include "drivers/io.h"
#include "drivers/bus.h"
#include "drivers/bus_spi.h"

bool spiBusWriteRegister(const busDevice_t * dev, uint8_t reg, uint8_t data)
{
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);

    IOLo(dev->busdev.spi.csnPin);
    spiTransferByte(instance, reg);
    spiTransferByte(instance, data);
    IOHi(dev->busdev.spi.csnPin);

    return true;
}

bool spiBusReadBuffer(const busDevice_t * dev, uint8_t reg, uint8_t * data, uint8_t length)
{
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);

    IOLo(dev->busdev.spi.csnPin);
    spiTransferByte(instance, reg);
    spiTransfer(instance, NULL, data, length);
    IOHi(dev->busdev.spi.csnPin);

    return true;
}

bool spiBusReadRegister(const busDevice_t * dev, uint8_t reg, uint8_t * data)
{
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);

    IOLo(dev->busdev.spi.csnPin);
    spiTransferByte(instance, reg);
    spiTransfer(instance, NULL, data, 1);
    IOHi(dev->busdev.spi.csnPin);

    return true;
}
#endif
