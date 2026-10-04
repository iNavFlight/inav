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

#ifdef USE_MAG_RM3100

#include "build/build_config.h"
#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/bus_i2c.h"

#include "sensors/boardalignment.h"
#include "sensors/sensors.h"

#include "drivers/sensor.h"
#include "drivers/compass/compass.h"

#include "drivers/compass/compass_rm3100.h"

#define RM3100_REG_POLL        0x00
#define RM3100_REG_CMM         0x01
#define RM3100_REG_CCX1        0x04
#define RM3100_REG_CCX0        0x05
#define RM3100_REG_CCY1        0x06
#define RM3100_REG_CCY0        0x07
#define RM3100_REG_CCZ1        0x08
#define RM3100_REG_CCZ0        0x09
#define RM3100_REG_TMRC        0x0B
#define RM3100_REG_MX          0x24
#define RM3100_REG_MY          0x27
#define RM3100_REG_MZ          0x2A
#define RM3100_REG_BIST        0x33
#define RM3100_REG_STATUS      0x34
#define RM3100_REG_HSHAKE      0x35
#define RM3100_REG_REVID       0x36

#define RM3100_REVID           0x22

#define CCX_DEFAULT_MSB        0x00
#define CCX_DEFAULT_LSB        0xC8
#define CCY_DEFAULT_MSB        CCX_DEFAULT_MSB
#define CCY_DEFAULT_LSB        CCX_DEFAULT_LSB
#define CCZ_DEFAULT_MSB        CCX_DEFAULT_MSB
#define CCZ_DEFAULT_LSB        CCX_DEFAULT_LSB
#define CMM_DEFAULT            0x71    // Continuous mode
#define TMRC_DEFAULT           0x94


static bool deviceInit(magDev_t * mag)
{
    busWrite(mag->busDev, RM3100_REG_TMRC, TMRC_DEFAULT);

    busWrite(mag->busDev, RM3100_REG_CMM, CMM_DEFAULT);

    busWrite(mag->busDev, RM3100_REG_CCX1, CCX_DEFAULT_MSB);
    busWrite(mag->busDev, RM3100_REG_CCX0, CCX_DEFAULT_LSB);

    busWrite(mag->busDev, RM3100_REG_CCY1, CCY_DEFAULT_MSB);
    busWrite(mag->busDev, RM3100_REG_CCY0, CCY_DEFAULT_LSB);

    busWrite(mag->busDev, RM3100_REG_CCZ1, CCZ_DEFAULT_MSB);
    busWrite(mag->busDev, RM3100_REG_CCZ0, CCZ_DEFAULT_LSB);

    return true;
}

// Targets of the non-blocking reads, filled by the bus driver in the background
static uint8_t rm3100Status;
static uint8_t rm3100Report[9];     // X, Y, Z as 24 bit big endian values

// Two transfers per sample: the status byte first, the data only when a new measurement is ready
static busReadStepResult_e deviceReadStart(magDev_t * mag, bool firstStep)
{
    static bool statusStarted = false;

    if (firstStep) {
        statusStarted = false;
        rm3100Status = 0;
    }

    if (!statusStarted) {
        if (!busReadBufStart(mag->busDev, RM3100_REG_STATUS, &rm3100Status, 1)) {
            return BUS_READ_STEP_BUSY;
        }
        statusStarted = true;
        return BUS_READ_STEP_NEXT;
    }

    if ((rm3100Status & 0x80) == 0) {
        return BUS_READ_STEP_LAST;      // nothing new to fetch, deviceRead() reports the miss
    }

    if (!busReadBufStart(mag->busDev, RM3100_REG_MX, rm3100Report, sizeof(rm3100Report))) {
        return BUS_READ_STEP_BUSY;
    }

    return BUS_READ_STEP_LAST;
}

static bool deviceRead(magDev_t * mag)
{
    const uint8_t *x = &rm3100Report[0];
    const uint8_t *y = &rm3100Report[3];
    const uint8_t *z = &rm3100Report[6];

    mag->magADCRaw[X] = 0;
    mag->magADCRaw[Y] = 0;
    mag->magADCRaw[Z] = 0;

    /* Check if new measurement is ready */
    if ((rm3100Status & 0x80) == 0) {
        return false;
    }

    /* Rearrange mag data */
    const int32_t xraw = ((x[0] << 24) | (x[1] << 16) | (x[2]) << 8);
    const int32_t yraw = ((y[0] << 24) | (y[1] << 16) | (y[2]) << 8);
    const int32_t zraw = ((z[0] << 24) | (z[1] << 16) | (z[2]) << 8);

    /* Truncate to 16-bit integers and pass along */
    mag->magADCRaw[X] = (int16_t)(xraw >> 16);
    mag->magADCRaw[Y] = (int16_t)(yraw >> 16);
    mag->magADCRaw[Z] = (int16_t)(zraw >> 16);

    return true;
}

#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(magDev_t * mag)
{
    for (int retryCount = 0; retryCount < DETECTION_MAX_RETRY_COUNT; retryCount++) {
        uint8_t revid = 0;
        bool ack = busRead(mag->busDev, RM3100_REG_REVID, &revid);

        if (ack && revid == RM3100_REVID) {
            return true;
        }
    }

    return false;
}

bool rm3100MagDetect(magDev_t * mag)
{
    busSetSpeed(mag->busDev, BUS_SPEED_STANDARD);

    mag->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_RM3100, mag->magSensorToUse, OWNER_COMPASS);
    if (mag->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(mag)) {
        busDeviceDeInit(mag->busDev);
        return false;
    }

    mag->init = deviceInit;
    mag->readStart = deviceReadStart;
    mag->read = deviceRead;

    return true;
}

#endif
