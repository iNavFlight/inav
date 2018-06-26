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
#ifdef USE_DMA_SPI_DEVICE
#ifndef GYRO_READ_TIMEOUT
    #define GYRO_READ_TIMEOUT 20
#endif //GYRO_READ_TIMEOUT
#include "drivers/dma_spi.h"
#include "drivers/time.h"
#endif //USE_DMA_SPI_DEVICE

void spiBusSetSpeed(const busDevice_t * dev, busSpeed_e speed)
{
    const SPIClockSpeed_e spiClock[] = { SPI_CLOCK_INITIALIZATON, SPI_CLOCK_SLOW, SPI_CLOCK_STANDARD, SPI_CLOCK_FAST, SPI_CLOCK_ULTRAFAST };
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);
    spiSetSpeed(instance, spiClock[speed]);
}


bool spiBusTransferMultiple(const busDevice_t * dev, busTransferDescriptor_t * dsc, int count)
{
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);

    IOLo(dev->busdev.spi.csnPin);
    __NOP();

    for (int n = 0; n < count; n++) {
        spiTransfer(instance, dsc[n].rxBuf, dsc[n].txBuf, dsc[n].length);
    }

    __NOP();
    IOHi(dev->busdev.spi.csnPin);

    return true;
}

bool spiBusTransfer(const busDevice_t * dev, const uint8_t *txData, uint8_t *rxData, int length)
{

    #ifdef USE_DMA_SPI_DEVICE
        (void)(dev);
        uint32_t timeoutCheck = millis();
        memcpy(dmaTxBuffer, (uint8_t *)txData, length);
        dmaSpiTransmitReceive(dmaTxBuffer, dmaRxBuffer, length, 1);
        while(dmaSpiReadStatus != DMA_SPI_READ_DONE)
        {
            if(millis() - timeoutCheck > GYRO_READ_TIMEOUT)
            {
                //GYRO_READ_TIMEOUT ms max, read failed, cleanup spi and return 0
                dmaSpicleanupspi();
                return false;
            }
        }
        memcpy((uint8_t *)rxData, dmaRxBuffer, length);
    #else
        IOLo(dev->busdev.spi.csnPin);
        spiTransfer(bus->busdev_u.spi.instance, txData, rxData, length);
        IOHi(dev->busdev.spi.csnPin);
    #endif

    return true;
}

bool spiBusWriteRegister(const busDevice_t * dev, uint8_t reg, uint8_t data)
{
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);

    IOLo(dev->busdev.spi.csnPin);
    delayMicroseconds(1);
    spiTransferByte(instance, reg);
    spiTransferByte(instance, data);
    IOHi(dev->busdev.spi.csnPin);
    delayMicroseconds(1);

    return true;
}

bool spiBusWriteBuffer(const busDevice_t * dev, uint8_t reg, const uint8_t * data, uint8_t length)
{
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);

    IOLo(dev->busdev.spi.csnPin);
    spiTransferByte(instance, reg);
    spiTransfer(instance, NULL, data, length);
    IOHi(dev->busdev.spi.csnPin);

    return true;
}

bool spiBusReadBuffer(const busDevice_t * dev, uint8_t reg, uint8_t * data, uint8_t length)
{
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);

    IOLo(dev->busdev.spi.csnPin);
    spiTransferByte(instance, reg);
    spiTransfer(instance, data, NULL, length);
    IOHi(dev->busdev.spi.csnPin);

    return true;
}

bool spiBusReadRegister(const busDevice_t * dev, uint8_t reg, uint8_t * data)
{
    SPI_TypeDef * instance = spiInstanceByDevice(dev->busdev.spi.spiBus);

    IOLo(dev->busdev.spi.csnPin);
    spiTransferByte(instance, reg);
    spiTransfer(instance, data, NULL, 1);
    IOHi(dev->busdev.spi.csnPin);

    return true;
}
#endif
