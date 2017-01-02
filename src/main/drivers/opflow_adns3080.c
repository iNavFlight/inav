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
#include <string.h>

#include <platform.h>

#if defined(USE_OPTICAL_FLOW_ADNS3080_SOFTSPI) || defined(USE_OPTICAL_FLOW_ADNS3080)

#include "system.h"
#include "gpio.h"
#include "io.h"
#include "io_impl.h"
#include "rcc.h"
#include "sensor.h"
#include "opflow.h"
#include "opflow_adns3080.h"

#include "bus_spi.h"
#include "bus_spi_soft.h"

static opflowDev_t * opflowPtr;

// Register Map for the ADNS3080 Optical OpticalFlow Sensor
#define ADNS3080_PRODUCT_ID             0x00
#define ADNS3080_MOTION                 0x02
#define ADNS3080_DELTA_X                0x03
#define ADNS3080_DELTA_Y                0x04
#define ADNS3080_SQUAL                  0x05
#define ADNS3080_CONFIGURATION_BITS     0x0A
#define ADNS3080_EXTENDED_CONFIG        0x0B
#define ADNS3080_MOTION_CLEAR           0x12
#define ADNS3080_FRAME_CAPTURE          0x13
#define ADNS3080_MOTION_BURST           0x50

#define ADNS3080_MOTION_FLAG_MOT        0x80
#define ADNS3080_MOTION_FLAG_OVF        0x10
#define ADNS3080_MOTION_FLAG_RES1600    0x01

#define ADNS3080_CONFIG_RES1600         0x10

#define ADNS3080_ECONFIG_NOPULLUP       0x04
#define ADNS3080_ECONFIG_NOAGC          0x02
#define ADNS3080_ECONFIG_FIXEDFR        0x01


// ADNS3080 hardware config
#define ADNS3080_PIXELS_X              30
#define ADNS3080_PIXELS_Y              30

// Id returned by ADNS3080_PRODUCT_ID register
#define ADNS3080_PRODUCT_ID_VALUE      0x17

#define DISABLE_ADNS3080()      {IOHi(IOGetByTag(IO_TAG(ADNS3080_NSS_PIN)));}
#define ENABLE_ADNS3080()       {IOLo(IOGetByTag(IO_TAG(ADNS3080_NSS_PIN)));}
#define DELAY_ADNS3080()        { volatile int i = 750; while (i) { i--; } }


#ifdef USE_OPTICAL_FLOW_ADNS3080_SOFTSPI
static const softSPIDevice_t softSPIDevice = {
    .sckTag = IO_TAG(ADNS3080_SCK_PIN),
    .mosiTag = IO_TAG(ADNS3080_MOSI_PIN),
    .misoTag = IO_TAG(ADNS3080_MISO_PIN),
    .nssTag = IO_TAG(ADNS3080_NSS_PIN),
};
#endif

static void ADNS3080_SpiInit(void)
{
    static bool hardwareInitialised = false;

    if (hardwareInitialised) {
        return;
    }

#ifdef USE_OPTICAL_FLOW_ADNS3080_SOFTSPI
    if (opflowPtr->hasSoftSPI) {
        softSpiInit(&softSPIDevice);
    }
    const SPIDevice flowSPIDevice = SOFT_SPIDEV_1;
#else
    const SPIDevice flowSPIDevice = spiDeviceByInstance(ADNS3080_SPI_INSTANCE);
    IOInit(IOGetByTag(IO_TAG(ADNS3080_NSS_PIN)), OWNER_SPI, RESOURCE_SPI_CS, flowSPIDevice + 1);
#endif

#if defined(STM32F10X)
    RCC_AHBPeriphClockCmd(ADNS3080_NSS_GPIO_CLK_PERIPHERAL, ENABLE);
#endif

    DISABLE_ADNS3080();

#ifdef ADNS3080_SPI_INSTANCE
    spiSetDivisor(ADNS3080_SPI_INSTANCE, SPI_9MHZ_CLOCK_DIVIDER);
#endif

    hardwareInitialised = true;
}

static uint8_t ADNS3080_TransferByte(uint8_t data)
{
#ifdef USE_OPTICAL_FLOW_ADNS3080_SOFTSPI
    if (opflowPtr->hasSoftSPI) {
        return softSpiTransferByte(&softSPIDevice, data);
    } else
#endif
    {
#ifdef ADNS3080_SPI_INSTANCE
        return spiTransferByte(ADNS3080_SPI_INSTANCE, data);
#else
        return 0;
#endif
    }
}

static uint8_t ADNS3080_WriteReg(uint8_t reg, uint8_t data)
{
    ENABLE_ADNS3080();
    ADNS3080_TransferByte(0x80 | reg);
    DELAY_ADNS3080();
    ADNS3080_TransferByte(data);
    DISABLE_ADNS3080();
    return true;
}

static uint8_t ADNS3080_ReadReg(uint8_t reg)
{
    ENABLE_ADNS3080();
    ADNS3080_TransferByte(reg);
    DELAY_ADNS3080();
    const uint8_t ret = ADNS3080_TransferByte(0xFF);
    DISABLE_ADNS3080();
    return ret;
}

static void ADNS3080_ReadBuf(uint8_t reg, uint8_t * buf, int length)
{
    ENABLE_ADNS3080();
    ADNS3080_TransferByte(reg);
    DELAY_ADNS3080();
    for (int i = 0; i < length; i++) {
        buf[i] = ADNS3080_TransferByte(0xFF);
    }
    DISABLE_ADNS3080();
}

bool opflowADNS3080Init(void)
{
    uint8_t config;

    config = ADNS3080_ReadReg(ADNS3080_CONFIGURATION_BITS);
    ADNS3080_WriteReg(ADNS3080_CONFIGURATION_BITS, config | ADNS3080_CONFIG_RES1600); // Set resolution to 1600 counts per inch

    return true;
}

bool opflowADNS3080Read(int16_t *opflowDataPtr)
{
    opflow_data_t * opflowData = (opflow_data_t*)opflowDataPtr;
    uint8_t buf[4];

    memset(opflowData, 0, sizeof(opflow_data_t));

    ADNS3080_ReadBuf(ADNS3080_MOTION_BURST, buf, 4);

    if (buf[0] & ADNS3080_MOTION_FLAG_MOT) {
        opflowData->delta.A[0] = (int8_t)buf[1];
        opflowData->delta.A[1] = (int8_t)buf[2];
        opflowData->quality = (uint8_t)buf[3];

        return true;
    }

    return false;
}

bool opflowADNS3080Detect(opflowDev_t * opflow)
{
#ifdef USE_OPTICAL_FLOW_ADNS3080_SOFTSPI
    if (!opflow->hasSoftSPI) {
        return false;
    }
#endif

    opflowPtr = opflow;

    ADNS3080_SpiInit();
    delay(25);

    // Do a dummy read to avoid clocking issues
    ADNS3080_ReadReg(ADNS3080_PRODUCT_ID);
    delay(25);

    const uint8_t reg = ADNS3080_ReadReg(ADNS3080_PRODUCT_ID);

    if (reg == ADNS3080_PRODUCT_ID_VALUE) {
        opflow->init = opflowADNS3080Init;
        opflow->read = opflowADNS3080Read;
        return true;
    }

    return false;
}

#endif
