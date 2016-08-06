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

#ifdef USE_SOFTSPI

#include "build_config.h"

#include "io.h"
#include "bus_spi.h"
#include "bus_spi_soft.h"


void softSpiInit(const softSPIDevice_t *spi)
{
    IOInit(IOGetByTag(spi->sckTag),  OWNER_SOFTSPI, RESOURCE_SPI_SCK,  SOFT_SPIDEV_1 + 1);
    IOInit(IOGetByTag(spi->misoTag), OWNER_SPI, RESOURCE_SPI_MISO, SOFT_SPIDEV_1 + 1);
    IOInit(IOGetByTag(spi->mosiTag), OWNER_SPI, RESOURCE_SPI_MOSI, SOFT_SPIDEV_1 + 1);
#ifdef SOFTSPI_NSS_PIN
    IOInit(IOGetByTag(spi->nssTag), OWNER_SPI, RESOURCE_SPI_CS, SOFT_SPIDEV_1 + 1);
#endif

#if defined(STM32F3) || defined(STM32F4)
    IOConfigGPIOAF(IOGetByTag(spi->sck),  SPI_IO_AF_CFG, 0);
    IOConfigGPIOAF(IOGetByTag(spi->miso), SPI_IO_AF_CFG, 0);
    IOConfigGPIOAF(IOGetByTag(spi->mosi), SPI_IO_AF_CFG, 0);
#ifdef SOFTSPI_NSS_PIN
    IOConfigGPIOAF(IOGetByTag(spi->nss), SPI_IO_CS_CFG, 0);
#endif
#elif defined(STM32F10X)
    IOConfigGPIO(IOGetByTag(spi->sckTag), SPI_IO_AF_SCK_CFG);
    IOConfigGPIO(IOGetByTag(spi->misoTag), SPI_IO_AF_MISO_CFG);
    IOConfigGPIO(IOGetByTag(spi->mosiTag), SPI_IO_AF_MOSI_CFG);
#ifdef SOFTSPI_NSS_PIN
    IOConfigGPIO(IOGetByTag(spi->nssTag), SPI_IO_CS_CFG);
#endif
#endif
/*    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

    // SCK as output
    GPIO_InitStructure.GPIO_Pin = dev->sck_pin;
    GPIO_Init(dev->sck_gpio, &GPIO_InitStructure);
    // MOSI as output
    GPIO_InitStructure.GPIO_Pin = dev->mosi_pin;
    GPIO_Init(dev->mosi_gpio, &GPIO_InitStructure);
    // MISO as input
    GPIO_InitStructure.GPIO_Pin = dev->miso_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(dev->miso_gpio, &GPIO_InitStructure);
#ifdef SOFTSPI_NSS_PIN
    // NSS as output
    GPIO_InitStructure.GPIO_Pin = dev->nss_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(dev->nss_gpio, &GPIO_InitStructure);
#endif*/
}

uint8_t softSpiTransferByte(const softSPIDevice_t *dev, uint8_t byte)
{
    for (int ii = 0; ii < 8; ++ii) {
        if (byte & 0x80) {
            IOHi(IOGetByTag(dev->mosiTag));
        } else {
            IOLo(IOGetByTag(dev->mosiTag));
        }
        IOHi(IOGetByTag(dev->sckTag));
        byte <<= 1;
        if (IORead(IOGetByTag(dev->misoTag)) == 1) {
            byte |= 1;
        }
        IOLo(IOGetByTag(dev->sckTag));
    }
    return byte;
}
#endif
