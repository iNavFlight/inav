/*
 * This file is part of INAV.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
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

#include "platform.h"

#include "drivers/bus.h"
#include "drivers/io.h"

static void busDevPreInit_SPI(const busDeviceDescriptor_t * descriptor)
{
    // Pre-initialize SPI device chip-select line to input with weak pull-up
    IO_t io = IOGetByTag(descriptor->busdev.spi.csnPin);
    if (io) {
        IOInit(io, OWNER_SPI_PREINIT, RESOURCE_SPI_CS, 0);
        IOConfigGPIO(io, IOCFG_IPU);
    }
}

void busInit(void)
{
    /* Pre-initialize bus devices */
    for (const busDeviceDescriptor_t * descriptor = __busdev_registry_start; (descriptor) < __busdev_registry_end; descriptor++) {
        switch (descriptor->busType) {
            case BUSTYPE_NONE:
                break;

            case BUSTYPE_I2C:
                break;

            case BUSTYPE_SPI:
                busDevPreInit_SPI(descriptor);
                break;
        }
    }
}

static bool busDevInit_I2C(busDevice_t * dev, const busDeviceDescriptor_t * descriptor)
{
    dev->busType = descriptor->busType;
    dev->busdev.i2c.i2cBus = descriptor->busdev.i2c.i2cBus;
    dev->busdev.i2c.address = descriptor->busdev.i2c.address;
    return true;
}

static bool busDevInit_SPI(busDevice_t * dev, const busDeviceDescriptor_t * descriptor, resourceOwner_e owner)
{
    dev->busType = descriptor->busType;
    dev->busdev.spi.spiBus = descriptor->busdev.spi.spiBus;
    dev->busdev.spi.csnPin = IOGetByTag(descriptor->busdev.spi.csnPin);

    if (dev->busdev.spi.csnPin) {
        IOInit(dev->busdev.spi.csnPin, owner, RESOURCE_SPI_CS, 0);
        IOConfigGPIO(dev->busdev.spi.csnPin, SPI_IO_CS_CFG);
        IOHi(dev->busdev.spi.csnPin);
        return true;
    }

    return false;
}

bool busDeviceInit(busDevice_t * dev, busType_e bus, devHardwareType_e hw, resourceOwner_e owner)
{
    for (const busDeviceDescriptor_t * descriptor = __busdev_registry_start; (descriptor) < __busdev_registry_end; descriptor++) {
        if (hw == descriptor->devHwType && (bus == descriptor->busType || bus == BUSTYPE_ANY)) {
            switch (descriptor->busType) {
                case BUSTYPE_NONE:
                    dev->busType = BUSTYPE_NONE;
                    return false;

                case BUSTYPE_I2C:
                    return busDevInit_I2C(dev, descriptor);

                case BUSTYPE_SPI:
                    return busDevInit_SPI(dev, descriptor, owner);
                    break;
            }
        }
    }

    return false;
}

/*
bool busWriteBuf(const busDevice_t * dev, uint8_t reg, uint8_t * data, uint8_t length)
{
    switch (dev->busType) {
        case BUSTYPE_NONE:
            return false;
        case BUSTYPE_SPI:
            return spiBusWriteBuffer(dev, reg & 0x7f, data, length);
        case BUSTYPE_I2C:
            return i2cBusWriteBuffer(dev, reg, data);
    }
}
*/

bool busWrite(const busDevice_t * dev, uint8_t reg, uint8_t data)
{
    switch (dev->busType) {
        case BUSTYPE_NONE:
            return false;
        case BUSTYPE_SPI:
            return spiBusWriteRegister(dev, reg & 0x7f, data);
        case BUSTYPE_I2C:
            return i2cBusWriteRegister(dev, reg, data);
    }

    return false;
}

bool busReadBuf(const busDevice_t * dev, uint8_t reg, uint8_t * data, uint8_t length)
{
    switch (dev->busType) {
        case BUSTYPE_NONE:
            return false;
        case BUSTYPE_SPI:
            return spiBusReadBuffer(dev, reg | 0x80, data, length);
        case BUSTYPE_I2C:
            return i2cBusReadBuffer(dev, reg, data, length);
    }

    return false;
}

bool busRead(const busDevice_t * dev, uint8_t reg, uint8_t * data)
{
    switch (dev->busType) {
        case BUSTYPE_SPI:
            return spiBusReadRegister(dev, reg | 0x80, data);
        case BUSTYPE_I2C:
            return i2cBusReadRegister(dev, reg, data);
        case BUSTYPE_NONE:
            return false;
    }

    return false;
}
