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

#include <math.h>

#include "platform.h"

#ifdef USE_MAG_AK8975

#include "build/build_config.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/bus.h"

#include "drivers/sensor.h"
#include "drivers/compass/compass.h"

#include "drivers/compass/compass_ak8975.h"

// This sensor is available in MPU-9150.

// AK8975, mag sensor address
#define AK8975_MAG_I2C_ADDRESS      0x0C

// Registers
#define AK8975_MAG_REG_WHO_AM_I     0x00
#define AK8975_MAG_REG_INFO         0x01
#define AK8975_MAG_REG_STATUS1      0x02
#define AK8975_MAG_REG_HXL          0x03
#define AK8975_MAG_REG_HXH          0x04
#define AK8975_MAG_REG_HYL          0x05
#define AK8975_MAG_REG_HYH          0x06
#define AK8975_MAG_REG_HZL          0x07
#define AK8975_MAG_REG_HZH          0x08
#define AK8975_MAG_REG_STATUS2      0x09
#define AK8975_MAG_REG_CNTL         0x0a
#define AK8975_MAG_REG_ASCT         0x0c // self test

#define AK8975A_ASAX 0x10 // Fuse ROM x-axis sensitivity adjustment value
#define AK8975A_ASAY 0x11 // Fuse ROM y-axis sensitivity adjustment value
#define AK8975A_ASAZ 0x12 // Fuse ROM z-axis sensitivity adjustment value

static bool ak8975Init(magDev_t * mag)
{
    bool ack;
    uint8_t buffer[3];
    uint8_t status;

    UNUSED(ack);

    ack = busWrite(mag->busDev, AK8975_MAG_REG_CNTL, 0x00); // power down before entering fuse mode
    delay(20);

    ack = busWrite(mag->busDev, AK8975_MAG_REG_CNTL, 0x0F); // Enter Fuse ROM access mode
    delay(10);

    ack = busReadBuf(mag->busDev, AK8975A_ASAX, &buffer[0], 3); // Read the x-, y-, and z-axis calibration values
    delay(10);

    ack = busWrite(mag->busDev, AK8975_MAG_REG_CNTL, 0x00); // power down after reading.
    delay(10);

    // Clear status registers
    ack = busRead(mag->busDev, AK8975_MAG_REG_STATUS1, &status);
    ack = busRead(mag->busDev, AK8975_MAG_REG_STATUS2, &status);

    // Trigger first measurement
    ack = busWrite(mag->busDev, AK8975_MAG_REG_CNTL, 0x01);
    return true;
}

#define BIT_STATUS1_REG_DATA_READY              (1 << 0)

#define BIT_STATUS2_REG_DATA_ERROR              (1 << 2)
#define BIT_STATUS2_REG_MAG_SENSOR_OVERFLOW     (1 << 3)

// Targets of the non-blocking reads, filled by the bus driver in the background
static uint8_t ak8975Status1;
static uint8_t ak8975Status2;
static uint8_t ak8975Data[6];

// Four transfers per sample: status, data once DRDY is set, STATUS2, then the trigger of the next measurement
static busReadStepResult_e ak8975ReadStart(magDev_t * mag, bool firstStep)
{
    static uint8_t step = 0;

    if (firstStep) {
        step = 0;
        ak8975Status1 = 0;
    }

    switch (step) {
        case 0:
            if (!busReadBufStart(mag->busDev, AK8975_MAG_REG_STATUS1, &ak8975Status1, 1)) {
                return BUS_READ_STEP_BUSY;
            }
            step = 1;
            return BUS_READ_STEP_NEXT;

        case 1:
            if ((ak8975Status1 & BIT_STATUS1_REG_DATA_READY) == 0) {
                return BUS_READ_STEP_LAST;      // measurement not finished, ak8975Read() reports the miss
            }
            if (!busReadBufStart(mag->busDev, AK8975_MAG_REG_HXL, ak8975Data, sizeof(ak8975Data))) {  // AK8975_MAG_REG_HXL to AK8975_MAG_REG_HZH
                return BUS_READ_STEP_BUSY;
            }
            step = 2;
            return BUS_READ_STEP_NEXT;

        case 2:
            if (!busReadBufStart(mag->busDev, AK8975_MAG_REG_STATUS2, &ak8975Status2, 1)) {
                return BUS_READ_STEP_BUSY;
            }
            step = 3;
            return BUS_READ_STEP_NEXT;

        default:
            // Start the next measurement; ak8975Read() parses once this write is out
            if (!busWriteStart(mag->busDev, AK8975_MAG_REG_CNTL, 0x01)) {
                return BUS_READ_STEP_BUSY;
            }
            return BUS_READ_STEP_LAST;
    }
}

static bool ak8975Read(magDev_t * mag)
{
    // set magData to zero for case of failed read
    mag->magADCRaw[X] = 0;
    mag->magADCRaw[Y] = 0;
    mag->magADCRaw[Z] = 0;

    if ((ak8975Status1 & BIT_STATUS1_REG_DATA_READY) == 0) {
        return false;
    }

    if ((ak8975Status2 & BIT_STATUS2_REG_DATA_ERROR) || (ak8975Status2 & BIT_STATUS2_REG_MAG_SENSOR_OVERFLOW)) {
        return false;
    }

    mag->magADCRaw[X] = -(int16_t)(ak8975Data[1] << 8 | ak8975Data[0]) * 4;
    mag->magADCRaw[Y] = (int16_t)(ak8975Data[3] << 8 | ak8975Data[2]) * 4;
    mag->magADCRaw[Z] = -(int16_t)(ak8975Data[5] << 8 | ak8975Data[4]) * 4;

    return true;
}

#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(magDev_t * mag)
{
    for (int retryCount = 0; retryCount < DETECTION_MAX_RETRY_COUNT; retryCount++) {
        delay(10);

        uint8_t sig = 0;
        bool ack = busRead(mag->busDev, AK8975_MAG_REG_WHO_AM_I, &sig);

        if (ack && sig == 0x48) {
            return true;
        }
    }

    return false;
}

bool ak8975Detect(magDev_t * mag)
{
    mag->busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_AK8975, mag->magSensorToUse, OWNER_COMPASS);
    if (mag->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(mag)) {
        busDeviceDeInit(mag->busDev);
        return false;
    }

    mag->init = ak8975Init;
    mag->readStart = ak8975ReadStart;
    mag->read = ak8975Read;

    return true;
}

#endif
