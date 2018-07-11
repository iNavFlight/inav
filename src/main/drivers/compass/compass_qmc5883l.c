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

#ifdef USE_MAG_QMC5883

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

#include "drivers/compass/compass_qmc5883l.h"

#define QMC5883L_MAG_I2C_ADDRESS     0x0D

// Registers
#define QMC5883L_REG_CONF1 0x09
#define QMC5883L_REG_CONF2 0x0A

// data output rates for 5883L
#define QMC5883L_ODR_10HZ (0x00 << 2)
#define QMC5883L_ODR_50HZ  (0x01 << 2)
#define QMC5883L_ODR_100HZ (0x02 << 2)
#define QMC5883L_ODR_200HZ (0x03 << 2)

// Sensor operation modes
#define QMC5883L_MODE_STANDBY 0x00
#define QMC5883L_MODE_CONTINUOUS 0x01

#define QMC5883L_RNG_2G (0x00 << 4)
#define QMC5883L_RNG_8G (0x01 << 4)

#define QMC5883L_OSR_512 (0x00 << 6)
#define QMC5883L_OSR_256 (0x01 << 6)
#define QMC5883L_OSR_128	(0x10 << 6)
#define QMC5883L_OSR_64	(0x11	<< 6)

#define QMC5883L_RST 0x80

#define QMC5883L_REG_DATA_OUTPUT_X 0x00
#define QMC5883L_REG_STATUS 0x06

#define QMC5883L_REG_ID 0x0D
#define QMC5883_ID_VAL 0xFF

static bool qmc5883Init(magDev_t * mag)
{
    bool ack = true;

    ack = ack && busWrite(mag->busDev, 0x0B, 0x01);
    // ack = ack && i2cWrite(busWrite(mag->busDev, 0x20, 0x40);
    // ack = ack && i2cWrite(busWrite(mag->busDev, 0x21, 0x01);
    ack = ack && busWrite(mag->busDev, QMC5883L_REG_CONF1, QMC5883L_MODE_CONTINUOUS | QMC5883L_ODR_200HZ | QMC5883L_OSR_512 | QMC5883L_RNG_8G);

    return ack;
}

static bool qmc5883Read(magDev_t * mag)
{
    uint8_t status;
    uint8_t buf[6];

    // set magData to zero for case of failed read
    mag->magADCRaw[X] = 0;
    mag->magADCRaw[Y] = 0;
    mag->magADCRaw[Z] = 0;

    bool ack = busRead(mag->busDev, QMC5883L_REG_STATUS, &status);
    if (!ack || (status & 0x04) == 0) {
        return false;
    }

    ack = busReadBuf(mag->busDev, QMC5883L_REG_DATA_OUTPUT_X, buf, 6);
    if (!ack) {
        return false;
    }

    mag->magADCRaw[X] = (int16_t)(buf[1] << 8 | buf[0]);
    mag->magADCRaw[Y] = (int16_t)(buf[3] << 8 | buf[2]);
    mag->magADCRaw[Z] = (int16_t)(buf[5] << 8 | buf[4]);

    return true;
}

#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(magDev_t * mag)
{
    for (int retryCount = 0; retryCount < DETECTION_MAX_RETRY_COUNT; retryCount++) {
        // Must write reset first  - don't care about the result
        busWrite(mag->busDev, QMC5883L_REG_CONF2, QMC5883L_RST);
        delay(30);

        uint8_t sig = 0;
        bool ack = busRead(mag->busDev, QMC5883L_REG_ID, &sig);

        if (ack && sig == QMC5883_ID_VAL) {
            // Should be in standby mode after soft reset and sensor is really present
            // Reading ChipID of 0xFF alone is not sufficient to be sure the QMC is present

            ack = busRead(mag->busDev, QMC5883L_REG_CONF1, &sig);
            if (ack && sig == QMC5883L_MODE_STANDBY) {
                return true;
            }
        }
    }

    return false;
}

bool qmc5883Detect(magDev_t * mag)
{
    mag->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_QMC5883, mag->magSensorToUse, OWNER_COMPASS);
    if (mag->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(mag)) {
        busDeviceDeInit(mag->busDev);
        return false;
    }

    mag->init = qmc5883Init;
    mag->read = qmc5883Read;

    return true;
}
#endif
