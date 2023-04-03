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

#ifdef USE_MAG_MAG3110

#include "build/build_config.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/bus.h"

#include "sensors/boardalignment.h"
#include "sensors/sensors.h"

#include "drivers/sensor.h"
#include "drivers/compass/compass.h"

#include "drivers/compass/compass_mag3110.h"


// Registers
#define MAG3110_MAG_I2C_ADDRESS     0x0E
#define MAG3110_MAG_REG_STATUS       0x00
#define MAG3110_MAG_REG_HXL          0x01
#define MAG3110_MAG_REG_HXH          0x02
#define MAG3110_MAG_REG_HYL          0x03
#define MAG3110_MAG_REG_HYH          0x04
#define MAG3110_MAG_REG_HZL          0x05
#define MAG3110_MAG_REG_HZH          0x06
#define MAG3110_MAG_REG_WHO_AM_I     0x07
#define MAG3110_MAG_REG_SYSMODE      0x08
#define MAG3110_MAG_REG_CTRL_REG1    0x10
#define MAG3110_MAG_REG_CTRL_REG2    0x11

static bool mag3110Init(magDev_t * mag)
{
    bool ack = busWrite(mag->busDev, MAG3110_MAG_REG_CTRL_REG1, 0x01); //  active mode 80 Hz ODR with OSR = 1
    if (!ack) {
        return false;
    }

    delay(20);

    ack = busWrite(mag->busDev, MAG3110_MAG_REG_CTRL_REG2, 0xA0); // AUTO_MRST_EN + RAW
    if (!ack) {
        return false;
    }

    return true;
}

#define BIT_STATUS_REG_DATA_READY               (1 << 3)

static bool mag3110Read(magDev_t * mag)
{
    uint8_t status;
    uint8_t buf[6];

    // set magData to zero for case of failed read
    mag->magADCRaw[X] = 0;
    mag->magADCRaw[Y] = 0;
    mag->magADCRaw[Z] = 0;

    bool ack = busRead(mag->busDev, MAG3110_MAG_REG_STATUS, &status);
    if (!ack || (status & BIT_STATUS_REG_DATA_READY) == 0) {
        return false;
    }

    ack = busReadBuf(mag->busDev, MAG3110_MAG_REG_HXL, buf, 6);
    if (!ack) {
        return false;
    }

    mag->magADCRaw[X] = (int16_t)(buf[0] << 8 | buf[1]);
    mag->magADCRaw[Y] = (int16_t)(buf[2] << 8 | buf[3]);
    mag->magADCRaw[Z] = -(int16_t)(buf[4] << 8 | buf[5]);  // Z is down, not up in this sensor

    return true;
}

#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(magDev_t * mag)
{
    for (int retryCount = 0; retryCount < DETECTION_MAX_RETRY_COUNT; retryCount++) {
        delay(10);

        uint8_t sig = 0;
        bool ack = busRead(mag->busDev, MAG3110_MAG_REG_WHO_AM_I, &sig);

        if (ack && sig == 0xC4) {
            return true;
        }
    }

    return false;
}

bool mag3110detect(magDev_t * mag)
{
    mag->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_MAG3110, mag->magSensorToUse, OWNER_COMPASS);
    if (mag->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(mag)) {
        busDeviceDeInit(mag->busDev);
        return false;
    }

    mag->init = mag3110Init;
    mag->read = mag3110Read;

    return true;
}
#endif
