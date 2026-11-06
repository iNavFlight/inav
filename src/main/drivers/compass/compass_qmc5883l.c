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
#define QMC5883L_STATUS_DRDY 0x01

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

// Targets of the non-blocking reads, filled by the bus driver in the background
static uint8_t qmc5883Status;
static uint8_t qmc5883Data[6];

// Two transfers per sample: the status byte first, the data only when DRDY confirms a fresh sample
static busReadStepResult_e qmc5883ReadStart(magDev_t * mag, bool firstStep)
{
    static bool statusStarted = false;

    if (firstStep) {
        statusStarted = false;
        qmc5883Status = 0;
    }

    if (!statusStarted) {
        if (!busReadBufStart(mag->busDev, QMC5883L_REG_STATUS, &qmc5883Status, 1)) {
            return BUS_READ_STEP_BUSY;
        }
        statusStarted = true;
        return BUS_READ_STEP_NEXT;
    }

    if ((qmc5883Status & QMC5883L_STATUS_DRDY) == 0) {
        return BUS_READ_STEP_LAST;     // nothing new to fetch, qmc5883Read() reports the miss
    }

    if (!busReadBufStart(mag->busDev, QMC5883L_REG_DATA_OUTPUT_X, qmc5883Data, sizeof(qmc5883Data))) {
        return BUS_READ_STEP_BUSY;
    }

    return BUS_READ_STEP_LAST;
}

static bool qmc5883Read(magDev_t * mag)
{
    if ((qmc5883Status & QMC5883L_STATUS_DRDY) == 0) {
        // set magData to zero for case of failed read
        mag->magADCRaw[X] = 0;
        mag->magADCRaw[Y] = 0;
        mag->magADCRaw[Z] = 0;
        return false;
    }

    mag->magADCRaw[X] = (int16_t)(qmc5883Data[1] << 8 | qmc5883Data[0]);
    mag->magADCRaw[Y] = (int16_t)(qmc5883Data[3] << 8 | qmc5883Data[2]);
    mag->magADCRaw[Z] = (int16_t)(qmc5883Data[5] << 8 | qmc5883Data[4]);

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
    mag->readStart = qmc5883ReadStart;
    mag->read = qmc5883Read;

    return true;
}
#endif
