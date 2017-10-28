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

#include <math.h>

#include "platform.h"

#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/bus_spi.h"
#include "drivers/io.h"
#include "drivers/sensor.h"
#include "drivers/time.h"

#include "drivers/compass/compass.h"
#include "drivers/compass/compass_ak8963.h"
#include "drivers/compass/compass_spi_ak8963.h"

#ifdef USE_MAG_SPI_AK8963
#define DISABLE_AK8963       IOHi(ak8963CsPin)
#define ENABLE_AK8963        IOLo(ak8963CsPin)

static int16_t magGain[XYZ_AXIS_COUNT] = { 256, 256, 256 };
static IO_t ak8963CsPin = IO_NONE;

static bool ak8963WriteRegister(uint8_t reg, uint8_t data)
{
    ENABLE_AK8963;
    spiTransferByte(AK8963_SPI_INSTANCE, reg & 0x7F);
    spiTransferByte(AK8963_SPI_INSTANCE, data);
    DISABLE_AK8963;

    return true;
}

static bool ak8963ReadRegisterBuffer(uint8_t reg, uint8_t *data, uint8_t length)
{
    ENABLE_AK8963;
    spiTransferByte(AK8963_SPI_INSTANCE, reg | READ_FLAG); // read transaction
    spiTransfer(AK8963_SPI_INSTANCE, data, NULL, length);
    DISABLE_AK8963;

    return true;
}

static bool ak8963SpiInit(magDev_t *magDev)
{
    uint8_t calibration[XYZ_AXIS_COUNT];
    uint8_t status;

    UNUSED(magDev);

    ak8963WriteRegister(AK8963_MAG_REG_I2CDIS, I2CDIS_DISABLE_MASK); // disable I2C
    delay(10);

    ak8963WriteRegister(AK8963_MAG_REG_CNTL1, CNTL1_MODE_POWER_DOWN); // power down before entering fuse mode
    delay(20);

    ak8963WriteRegister(AK8963_MAG_REG_CNTL1, CNTL1_MODE_FUSE_ROM); // Enter Fuse ROM access mode
    delay(10);

    ak8963ReadRegisterBuffer(AK8963_MAG_REG_ASAX, calibration, sizeof(calibration)); // Read the x-, y-, and z-axis calibration values
    delay(10);

    magGain[X] = calibration[X] + 128;
    magGain[Y] = calibration[Y] + 128;
    magGain[Z] = calibration[Z] + 128;

    ak8963WriteRegister(AK8963_MAG_REG_CNTL1, CNTL1_MODE_POWER_DOWN); // power down after reading.
    delay(10);

    // Clear status registers
    ak8963ReadRegisterBuffer(AK8963_MAG_REG_ST1, &status, 1);
    ak8963ReadRegisterBuffer(AK8963_MAG_REG_ST2, &status, 1);

    // Trigger first measurement
    ak8963WriteRegister(AK8963_MAG_REG_CNTL1, CNTL1_BIT_16_BIT | CNTL1_MODE_ONCE);
    return true;
}

static bool ak8963SpiRead(magDev_t *magDev)
{
    uint8_t buf[7];
    uint8_t status;

    ak8963ReadRegisterBuffer(AK8963_MAG_REG_ST1, &status, 1);

    if ((status & ST1_DATA_READY) == 0) {
        return false;
    }

    ak8963ReadRegisterBuffer(AK8963_MAG_REG_HXL, &buf[0], 7);
    uint8_t status2 = buf[6];

    ak8963WriteRegister(AK8963_MAG_REG_CNTL1, CNTL1_BIT_16_BIT | CNTL1_MODE_ONCE); // start reading again

    if (status2 & ST2_MAG_SENSOR_OVERFLOW) {
        return false;
    }

    magDev->magADCRaw[X] = -parseMag(buf + 0, magGain[X]);
    magDev->magADCRaw[Y] = -parseMag(buf + 2, magGain[Y]);
    magDev->magADCRaw[Z] = -parseMag(buf + 4, magGain[Z]);

    return true;
}

bool ak8963SpiDetect(magDev_t *magDev)
{
    uint8_t sig = 0;

    ak8963CsPin = IOGetByTag(IO_TAG(AK8963_CS_PIN));

    IOInit(ak8963CsPin, OWNER_COMPASS, RESOURCE_SPI_CS, 0);
    IOConfigGPIO(ak8963CsPin, IOCFG_OUT_PP);

    DISABLE_AK8963;

    spiSetSpeed(AK8963_SPI_INSTANCE, SPI_CLOCK_STANDARD);

    ak8963WriteRegister(AK8963_MAG_REG_CNTL2, CNTL2_SOFT_RESET); // reset MAG
    delay(4);

    // check for SPI AK8963
    bool ack = ak8963ReadRegisterBuffer(AK8963_MAG_REG_WIA, &sig, 1);
    if (ack && sig == AK8963_DEVICE_ID) // 0x48 / 01001000 / 'H'
    {
        magDev->init = ak8963SpiInit;
        magDev->read = ak8963SpiRead;

        return true;
    }

    IOInit(ak8963CsPin, OWNER_SPI_PREINIT, RESOURCE_SPI_CS, 0);
    IOConfigGPIO(ak8963CsPin, IOCFG_IPU);

    return false;
}
#endif
