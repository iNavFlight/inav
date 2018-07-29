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

// This file is copied with modifications from project Deviation,
// see http://deviationtx.com

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <platform.h>

#ifdef USE_RX_SPI

#include "build/build_config.h"

#include "drivers/time.h"
#include "drivers/io.h"
#include "io_impl.h"
#include "rcc.h"
#include "rx_spi.h"

#include "drivers/bus_spi.h"

#define DISABLE_RX()    {IOHi(IOGetByTag(IO_TAG(RX_NSS_PIN)));}
#define ENABLE_RX()     {IOLo(IOGetByTag(IO_TAG(RX_NSS_PIN)));}
#ifdef RX_CE_PIN
#define RX_CE_HI()       {IOHi(IOGetByTag(IO_TAG(RX_CE_PIN)));}
#define RX_CE_LO()       {IOLo(IOGetByTag(IO_TAG(RX_CE_PIN)));}
#endif


#ifdef RX_IRQ_PIN
static IO_t rxIrqPin = IO_NONE;
#endif

void rxSpiDeviceInit()
{
    static bool hardwareInitialised = false;

    if (hardwareInitialised) {
        return;
    }

    const SPIDevice rxSPIDevice = spiDeviceByInstance(RX_SPI_INSTANCE);
    IOInit(IOGetByTag(IO_TAG(RX_NSS_PIN)), OWNER_SPI, RESOURCE_SPI_CS, rxSPIDevice + 1);

#ifdef RX_IRQ_PIN
    rxIrqPin = IOGetByTag(IO_TAG(RX_IRQ_PIN));
    IOInit(rxIrqPin, OWNER_RX, RESOURCE_NONE, 0);
    IOConfigGPIO(rxIrqPin, IOCFG_IN_FLOATING);
#endif

#ifdef RX_CE_PIN
    // CE as OUTPUT
    IOInit(IOGetByTag(IO_TAG(RX_CE_PIN)), OWNER_RX_SPI, RESOURCE_RX_CE, rxSPIDevice + 1);
#if defined(STM32F3) || defined(STM32F4)
    IOConfigGPIOAF(IOGetByTag(IO_TAG(RX_CE_PIN)), SPI_IO_CS_CFG, 0);
#endif
    RX_CE_LO();
#endif // RX_CE_PIN
    DISABLE_RX();

#ifdef RX_SPI_INSTANCE
    spiSetSpeed(RX_SPI_INSTANCE, SPI_CLOCK_STANDARD);
#endif
    hardwareInitialised = true;
}

uint8_t rxSpiTransferByte(uint8_t data)
{
#ifdef RX_SPI_INSTANCE
    return spiTransferByte(RX_SPI_INSTANCE, data);
#else
    return 0;
#endif
}

uint8_t rxSpiWriteByte(uint8_t data)
{
    ENABLE_RX();
    const uint8_t ret = rxSpiTransferByte(data);
    DISABLE_RX();
    return ret;
}

uint8_t rxSpiWriteCommand(uint8_t command, uint8_t data)
{
    ENABLE_RX();
    const uint8_t ret = rxSpiTransferByte(command);
    rxSpiTransferByte(data);
    DISABLE_RX();
    return ret;
}

uint8_t rxSpiWriteCommandMulti(uint8_t command, const uint8_t *data, uint8_t length)
{
    ENABLE_RX();
    const uint8_t ret = rxSpiTransferByte(command);
    for (uint8_t i = 0; i < length; i++) {
        rxSpiTransferByte(data[i]);
    }
    DISABLE_RX();
    return ret;
}

uint8_t rxSpiReadCommand(uint8_t command, uint8_t data)
{
    ENABLE_RX();
    rxSpiTransferByte(command);
    const uint8_t ret = rxSpiTransferByte(data);
    DISABLE_RX();
    return ret;
}

uint8_t rxSpiReadCommandMulti(uint8_t command, uint8_t commandData, uint8_t *retData, uint8_t length)
{
    ENABLE_RX();
    const uint8_t ret = rxSpiTransferByte(command);
    for (uint8_t i = 0; i < length; i++) {
        retData[i] = rxSpiTransferByte(commandData);
    }
    DISABLE_RX();
    return ret;
}

#ifdef RX_IRQ_PIN
bool rxSpiCheckIrq(void)
{
    return !IORead(rxIrqPin);
}
#endif

#endif

