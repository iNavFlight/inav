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

static bool deviceRead(magDev_t * mag)
{
    uint8_t status;

#pragma pack(push, 1)
    struct {
        uint8_t x[3];
        uint8_t y[3];
        uint8_t z[3];
    } rm_report;
#pragma pack(pop)

    mag->magADCRaw[X] = 0;
    mag->magADCRaw[Y] = 0;
    mag->magADCRaw[Z] = 0;

    /* Check if new measurement is ready */
    bool ack = busRead(mag->busDev, RM3100_REG_STATUS, &status);

    if (!ack || (status & 0x80) == 0) {
        return false;
    }

    ack = busReadBuf(mag->busDev, RM3100_REG_MX, (uint8_t *)&rm_report, sizeof(rm_report));
    if (!ack) {
        return false;
    }

    int32_t xraw;
    int32_t yraw;
    int32_t zraw;

    /* Rearrange mag data */
    xraw = ((rm_report.x[0] << 24) | (rm_report.x[1] << 16) | (rm_report.x[2]) << 8);
    yraw = ((rm_report.y[0] << 24) | (rm_report.y[1] << 16) | (rm_report.y[2]) << 8);
    zraw = ((rm_report.z[0] << 24) | (rm_report.z[1] << 16) | (rm_report.z[2]) << 8);

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
    mag->read = deviceRead;

    return true;
}

#endif
