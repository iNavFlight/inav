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
#include "drivers/bus.h"
#include "drivers/exti.h"

#include "io_impl.h"
#include "rcc.h"
#include "rx_spi.h"

#include "drivers/bus_spi.h"
#include "common/log.h"

// 10 MHz max SPI frequency
#define RX_MAX_SPI_CLK_HZ 10000000

#define ENABLE_RX() IOLo(busdev->busdev.spi.csnPin)
#define DISABLE_RX() IOHi(busdev->busdev.spi.csnPin)

static busDevice_t rxSpiDevice;
static busDevice_t *busdev = &rxSpiDevice;

static IO_t extiPin = IO_NONE;
static extiCallbackRec_t rxSpiExtiCallbackRec;
static bool extiLevel = true;

static volatile bool extiHasOccurred = false;
static volatile timeUs_t lastExtiTimeUs = 0;

bool rxSpiDeviceInit(void)
{
    busdev = busDeviceInit(BUSTYPE_SPI, DEVHW_RX_SPI, 0, OWNER_RX_SPI);

    if (!busdev) {
        return false;
    }
	
	busSetSpeed(busdev, BUS_SPEED_FAST);

    const IO_t rxCsPin = IOGetByTag(IO_TAG(RX_NSS_PIN));
    IOInit(rxCsPin, OWNER_RX_SPI, RESOURCE_RX_SPI, 0);
    IOConfigGPIO(rxCsPin, SPI_IO_CS_CFG);
	busdev->busdev.spi.csnPin = rxCsPin;

    DISABLE_RX();

    extiPin = IOGetByTag(IO_TAG(RX_SPI_EXTI_PIN));
	if (extiPin) {
        IOInit(extiPin, OWNER_RX_SPI, RESOURCE_RX_SPI, 0);
    }

	return true;
}

void rxSpiExtiHandler(extiCallbackRec_t* callback)
{
    UNUSED(callback);

    const timeUs_t extiTimeUs = microsISR();

    if (IORead(extiPin) == extiLevel) {
        lastExtiTimeUs = extiTimeUs;
        extiHasOccurred = true;
    }
}

void rxSpiExtiInit(void)
{
    if (extiPin) {
		extiLevel = false;
        EXTIHandlerInit(&rxSpiExtiCallbackRec, rxSpiExtiHandler);
        EXTIConfig(extiPin, &rxSpiExtiCallbackRec, 4, EXTI_Trigger_Rising);
        EXTIEnable(extiPin, true);
    }
}

uint8_t rxSpiTransferByte(uint8_t data)
{
	uint8_t response;
    spiBusTransfer(busdev, &response, &data, 1);
	return response;
}

void rxSpiWriteByte(uint8_t data)
{
    spiBusTransfer(busdev, NULL, &data, 1);
}

void rxSpiWriteCommand(uint8_t command, uint8_t data)
{
    spiBusWriteRegister(busdev, command, data);
}

void rxSpiWriteCommandMulti(uint8_t command, const uint8_t *data, uint8_t length)
{
    spiBusWriteBuffer(busdev, command, data, length);
}

uint8_t rxSpiReadCommand(uint8_t command, uint8_t data)
{
    UNUSED(data);
	uint8_t response;
    spiBusReadRegister(busdev, command, &response);
    return response;
}

void rxSpiReadCommandMulti(uint8_t command, uint8_t commandData, uint8_t *retData, uint8_t length)
{
	UNUSED(commandData);
    spiBusReadBuffer(busdev, command, retData, length);
}

bool rxSpiExtiConfigured(void)
{
    return extiPin != IO_NONE;
}

bool rxSpiGetExtiState(void)
{
    return IORead(extiPin);
}

bool rxSpiPollExti(void)
{
    return extiHasOccurred;
}

void rxSpiResetExti(void)
{
    extiHasOccurred = false;
}

bool rxSpiCheckIrq(void)
{
    return !IORead(extiPin);
}

#endif

