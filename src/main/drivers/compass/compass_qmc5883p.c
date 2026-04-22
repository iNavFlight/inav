/*
 * This file is part of INAV Project.
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

#include <stdbool.h>
#include <stdint.h>

#include <math.h>

#include "platform.h"

#ifdef USE_MAG_QMC5883P

#include "build/build_config.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/bus_i2c.h"

#include "sensors/boardalignment.h"
#include "sensors/sensors.h"

#include "drivers/sensor.h"
#include "drivers/compass/compass.h"

#include "drivers/compass/compass_qmc5883p.h"

#define QMC5883P_MAG_I2C_ADDRESS     0x2C

#define QMC5883P_REG_ID 0x00
#define QMC5883P_ID_VAL 0x80

#define QMC5883P_REG_DATA_OUTPUT_X 0x01
#define QMC5883P_DATA_BYTES 6

#define QMC5883P_REG_CONF1 0x0A

#define QMC5883P_CONF1_MODE_SUSPEND    0x00
#define QMC5883P_CONF1_MODE_NORMAL     0x01
#define QMC5883P_CONF1_MODE_SINGLE     0x02
#define QMC5883P_CONF1_MODE_CONTINUOUS 0x03

#define QMC5883P_CONF1_ODR_10HZ  (0x00 << 2)
#define QMC5883P_CONF1_ODR_50HZ  (0x01 << 2)
#define QMC5883P_CONF1_ODR_100HZ (0x02 << 2)
#define QMC5883P_CONF1_ODR_200HZ (0x03 << 2)

#define QMC5883P_CONF1_OSR1_8 (0x00 << 4)
#define QMC5883P_CONF1_OSR1_4 (0x01 << 4)
#define QMC5883P_CONF1_OSR1_2 (0x02 << 4)
#define QMC5883P_CONF1_OSR1_1 (0x03 << 4)

#define QMC5883P_CONF1_OSR2_1 (0x00 << 6)
#define QMC5883P_CONF1_OSR2_2 (0x01 << 6)
#define QMC5883P_CONF1_OSR2_4 (0x02 << 6)
#define QMC5883P_CONF1_OSR2_8 (0x03 << 6)


#define QMC5883P_REG_CONF2 0x0B

#define QMC5883P_CONF2_SET_RESET_ON     0x00
#define QMC5883P_CONF2_SET_ON_RESET_OFF 0x01
#define QMC5883P_CONF2_SET_RESET_OFF    0x02

#define QMC5883P_CONF2_RNG_30G (0x00 << 2)
#define QMC5883P_CONF2_RNG_12G (0x01 << 2)
#define QMC5883P_CONF2_RNG_8G  (0x02 << 2)
#define QMC5883P_CONF2_RNG_2G  (0x03 << 2)

#define QMC5883P_CONF2_SELF_TEST 0x40

#define QMC5883P_CONF2_RESET 0x80


#define QMC5883P_REG_STATUS 0x09
#define QMC5883P_STATUS_DRDY_MASK 0x01
#define QMC5883P_STATUS_OVFL_MASK 0x02

// This register has no definition in the datasheet and only mentioned in an example
#define QMC5883P_REG_DATA_SIGN 0x29
#define QMC5883P_DATA_SIGN_MAGIC_VALUE 0x06

static bool qmc5883pInit(magDev_t * mag)
{
    bool ack = true;

    ack = ack && busWrite(mag->busDev, QMC5883P_REG_CONF2, QMC5883P_CONF2_RESET);

    delay(30);

    ack = ack && busWrite(mag->busDev, QMC5883P_REG_DATA_SIGN, QMC5883P_DATA_SIGN_MAGIC_VALUE);
    ack = ack && busWrite(mag->busDev, QMC5883P_REG_CONF2, QMC5883P_CONF2_RNG_8G);
    ack = ack && busWrite(mag->busDev,
                          QMC5883P_REG_CONF1,
                          QMC5883P_CONF1_MODE_CONTINUOUS |
                          QMC5883P_CONF1_ODR_200HZ       |
                          QMC5883P_CONF1_OSR1_8          |
                          QMC5883P_CONF1_OSR2_8);

    return ack;
}

static bool qmc5883pRead(magDev_t * mag)
{
    uint8_t status;
    uint8_t buf[QMC5883P_DATA_BYTES];

    // set magData to zero for case of failed read
    mag->magADCRaw[X] = 0;
    mag->magADCRaw[Y] = 0;
    mag->magADCRaw[Z] = 0;

    bool ack = busRead(mag->busDev, QMC5883P_REG_STATUS, &status);
    if (!ack || (status & QMC5883P_STATUS_DRDY_MASK) == 0) {
        return false;
    }

    ack = busReadBuf(mag->busDev, QMC5883P_REG_DATA_OUTPUT_X, buf, QMC5883P_DATA_BYTES);
    if (!ack) {
        return false;
    }

    /*
    Initially, this sensor provided the data like this:
    mag->magADCRaw[X] = (int16_t)(buf[1] << 8 | buf[0]);
    mag->magADCRaw[Y] = (int16_t)(buf[3] << 8 | buf[2]);
    mag->magADCRaw[Z] = (int16_t)(buf[5] << 8 | buf[4]);

    But QMC5883P has a different reference point and axis directions, compared to QMC5883L.
    As QMC5883P is designed to be a drop-in replacement for QMC5883L, apply alignment at
    readout to obtain the same readings in the same position. In particular, it does
    the same transformation to the data, as the CW270_DEG_FLIP option:
        dest[X] = -y;
        dest[Y] = -x;
        dest[Z] = -z;
    */
    mag->magADCRaw[X] = -(int16_t)(buf[3] << 8 | buf[2]);
    mag->magADCRaw[Y] = -(int16_t)(buf[1] << 8 | buf[0]);
    mag->magADCRaw[Z] = -(int16_t)(buf[5] << 8 | buf[4]);

    return true;
}

#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(magDev_t * mag)
{
    for (int retryCount = 0; retryCount < DETECTION_MAX_RETRY_COUNT; retryCount++) {
        // Must write reset first  - don't care about the result
        busWrite(mag->busDev, QMC5883P_REG_CONF2, QMC5883P_CONF2_RESET);
        delay(30);

        uint8_t sig = 0;
        bool ack = busRead(mag->busDev, QMC5883P_REG_ID, &sig);

        if (ack && sig == QMC5883P_ID_VAL) {
            return true;
        }
    }

    return false;
}

bool qmc5883pDetect(magDev_t * mag)
{
    mag->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_QMC5883P, mag->magSensorToUse, OWNER_COMPASS);
    if (mag->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(mag)) {
        busDeviceDeInit(mag->busDev);
        return false;
    }

    mag->init = qmc5883pInit;
    mag->read = qmc5883pRead;

    return true;
}
#endif
