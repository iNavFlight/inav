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

#ifdef USE_MAG_AK8963

#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/io.h"
#include "drivers/bus.h"

#include "drivers/sensor.h"
#include "drivers/compass/compass.h"

// AK8963, mag sensor address
#define AK8963_MAG_I2C_ADDRESS          0x0C
#define AK8963_DEVICE_ID                0x48


// Registers
#define AK8963_MAG_REG_WHO_AM_I         0x00
#define AK8963_MAG_REG_INFO             0x01
#define AK8963_MAG_REG_STATUS1          0x02
#define AK8963_MAG_REG_HXL              0x03
#define AK8963_MAG_REG_HXH              0x04
#define AK8963_MAG_REG_HYL              0x05
#define AK8963_MAG_REG_HYH              0x06
#define AK8963_MAG_REG_HZL              0x07
#define AK8963_MAG_REG_HZH              0x08
#define AK8963_MAG_REG_STATUS2          0x09
#define AK8963_MAG_REG_CNTL             0x0a
#define AK8963_MAG_REG_ASCT             0x0c // self test
#define AK8963_MAG_REG_ASAX             0x10 // Fuse ROM x-axis sensitivity adjustment value
#define AK8963_MAG_REG_ASAY             0x11 // Fuse ROM y-axis sensitivity adjustment value
#define AK8963_MAG_REG_ASAZ             0x12 // Fuse ROM z-axis sensitivity adjustment value

#define READ_FLAG                       0x80

#define STATUS1_DATA_READY              0x01
#define STATUS1_DATA_OVERRUN            0x02

#define STATUS2_DATA_ERROR              0x02
#define STATUS2_MAG_SENSOR_OVERFLOW     0x03

#define CNTL_MODE_POWER_DOWN            0x00
#define CNTL_MODE_ONCE                  0x01
#define CNTL_MODE_CONT1                 0x02
#define CNTL_MODE_CONT2                 0x06
#define CNTL_MODE_SELF_TEST             0x08
#define CNTL_MODE_FUSE_ROM              0x0F

static float magGain[3] = { 1.0f, 1.0f, 1.0f };

static bool ak8963Init(magDev_t * mag)
{
    bool ack;
    UNUSED(ack);
    uint8_t calibration[3];
    uint8_t status;

    ack = busWrite(mag->busDev, AK8963_MAG_REG_CNTL, CNTL_MODE_POWER_DOWN); // power down before entering fuse mode
    delay(20);

    ack = busWrite(mag->busDev, AK8963_MAG_REG_CNTL, CNTL_MODE_FUSE_ROM); // Enter Fuse ROM access mode
    delay(10);

    ack = busReadBuf(mag->busDev, AK8963_MAG_REG_ASAX, calibration, sizeof(calibration)); // Read the x-, y-, and z-axis calibration values
    delay(10);

    magGain[X] = (((((float)(int8_t)calibration[X] - 128) / 256) + 1) * 30);
    magGain[Y] = (((((float)(int8_t)calibration[Y] - 128) / 256) + 1) * 30);
    magGain[Z] = (((((float)(int8_t)calibration[Z] - 128) / 256) + 1) * 30);

    ack = busWrite(mag->busDev, AK8963_MAG_REG_CNTL, CNTL_MODE_POWER_DOWN); // power down after reading.
    delay(10);

    // Clear status registers
    ack = busRead(mag->busDev, AK8963_MAG_REG_STATUS1, &status);
    ack = busRead(mag->busDev, AK8963_MAG_REG_STATUS2, &status);

    ack = busWrite(mag->busDev, AK8963_MAG_REG_CNTL, CNTL_MODE_ONCE);

    return true;
}

static bool ak8963Read(magDev_t * mag)
{
    bool ack = false;
    bool readResult = false;
    uint8_t buf[7];

    ack = busRead(mag->busDev, AK8963_MAG_REG_STATUS1, &buf[0]);

    if (!ack || (buf[0] & STATUS1_DATA_READY) == 0) {
        return false;
    }

    ack = busReadBuf(mag->busDev, AK8963_MAG_REG_HXL, &buf[0], 7);

    if (!ack || (buf[6] & STATUS2_DATA_ERROR) || (buf[6] & STATUS2_MAG_SENSOR_OVERFLOW)) {
        return false;
    }

    mag->magADCRaw[X] = (int16_t)(buf[1] << 8 | buf[0]) * magGain[X];
    mag->magADCRaw[Y] = (int16_t)(buf[3] << 8 | buf[2]) * magGain[Y];
    mag->magADCRaw[Z] = (int16_t)(buf[5] << 8 | buf[4]) * magGain[Z];

    ack = busWrite(mag->busDev, AK8963_MAG_REG_CNTL, CNTL_MODE_ONCE);

    return readResult;
}

#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(magDev_t * mag)
{
    for (int retryCount = 0; retryCount < DETECTION_MAX_RETRY_COUNT; retryCount++) {
        delay(10);

        uint8_t sig = 0;
        bool ack = busRead(mag->busDev, AK8963_MAG_REG_WHO_AM_I, &sig);

        if (ack && sig == AK8963_DEVICE_ID) {
            return true;
        }
    }

    return false;
}

bool ak8963Detect(magDev_t * mag)
{
    mag->busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_AK8963, mag->magSensorToUse, OWNER_COMPASS);
    if (mag->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(mag)) {
        busDeviceDeInit(mag->busDev);
        return false;
    }

    mag->init = ak8963Init;
    mag->read = ak8963Read;

    return true;
}
#endif
