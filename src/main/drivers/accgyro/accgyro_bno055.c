/*
 * This file is part of INAV.
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
#include "drivers/bus.h"
#include "drivers/time.h"
#include "build/debug.h"
#include "common/vector.h"
#include "drivers/accgyro/accgyro_bno055.h"

static busDevice_t * busDev;

static bool deviceDetect(busDevice_t * busDev)
{
    for (int retry = 0; retry < 5; retry++) {
        uint8_t sig;

        delay(150);

        bool ack = busRead(busDev, 0x00, &sig);
        if (ack) {
            return true;
        }
    };

    return false;
}

bool bno055Init(void)
{
    busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_BNO055, 0, 0);
    if (busDev == NULL) {
        DEBUG_SET(DEBUG_IMU2, 2, 1);
        return false;
    }

    if (!deviceDetect(busDev)) {
        DEBUG_SET(DEBUG_IMU2, 2, 2);
        busDeviceDeInit(busDev);
        return false;
    }

    busWrite(busDev, BNO055_ADDR_PWR_MODE, BNO055_PWR_MODE_NORMAL); //Set power mode NORMAL
    delay(25);
    busWrite(busDev, BNO055_ADDR_OPR_MODE, BNO055_OPR_MODE_NDOF); //Set operational mode NDOF
    delay(50);

    return true;
}

fpVector3_t bno055GetEurlerAngles(void)
{
    fpVector3_t eurlerAngles;

    uint8_t buf[6];
    busReadBuf(busDev, BNO055_ADDR_EUL_YAW_LSB, buf, 6);

    eurlerAngles.x = ((int16_t)((buf[3] << 8) | buf[2])) / 16;
    eurlerAngles.y = ((int16_t)((buf[5] << 8) | buf[4])) / 16;
    eurlerAngles.z = ((int16_t)((buf[1] << 8) | buf[0])) / 16;

    return eurlerAngles;
}

bno055CalibStat_t bno055GetCalibStat(void)
{
    bno055CalibStat_t stats;
    uint8_t buf;

    busRead(busDev, BNO055_ADDR_CALIB_STAT, &buf);

    stats.mag = buf & 0b00000011;
    stats.acc = (buf >> 2) & 0b00000011;
    stats.gyr = (buf >> 4) & 0b00000011;
    stats.sys = (buf >> 6) & 0b00000011;

    return stats;
}