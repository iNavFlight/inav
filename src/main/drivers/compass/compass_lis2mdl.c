/*
 * This file is part of INAV Project.
 *
 * INAV Project is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV Project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#ifdef USE_MAG_LIS2MDL

#include "common/axis.h"

#include "drivers/time.h"
#include "drivers/sensor.h"

#include "drivers/compass/compass.h"
#include "drivers/compass/compass_lis2mdl.h"

// LIS2MDL, IIS2MDC, LSM303AGR and LSM303AH are firmware and pin-to-pin compatible solutions.

#define LIS2MDL_DEVICE_ID           0x40

#define LIS2MDL_REG_WHO_AM_I        0x4F
#define LIS2MDL_REG_CFG_REG_A       0x60
#define LIS2MDL_REG_CFG_REG_B       0x61
#define LIS2MDL_REG_CFG_REG_C       0x62
#define LIS2MDL_REG_STATUS_REG      0x67
#define LIS2MDL_REG_OUTX_L          0x68

// CFG_REG_A
#define LIS2MDL_MD_CONTINUOUS       0x00
#define LIS2MDL_ODR_100HZ           0x0C
#define LIS2MDL_COMP_TEMP_EN        0x80

// CFG_REG_B
#define LIS2MDL_OFF_CANC            0x02

// CFG_REG_C
#define LIS2MDL_BDU                 0x10

// STATUS_REG
#define LIS2MDL_STATUS_ZYXDA        0x08

static bool lis2mdlInit(magDev_t *mag)
{
    bool ack = true;

    ack = ack && busWrite(mag->busDev, LIS2MDL_REG_CFG_REG_A, LIS2MDL_MD_CONTINUOUS | LIS2MDL_ODR_100HZ | LIS2MDL_COMP_TEMP_EN);
    ack = ack && busWrite(mag->busDev, LIS2MDL_REG_CFG_REG_B, LIS2MDL_OFF_CANC);
    ack = ack && busWrite(mag->busDev, LIS2MDL_REG_CFG_REG_C, LIS2MDL_BDU);

    return ack;
}

static bool lis2mdlRead(magDev_t *mag)
{
    uint8_t status = 0;
    uint8_t buf[6];

    mag->magADCRaw[X] = 0;
    mag->magADCRaw[Y] = 0;
    mag->magADCRaw[Z] = 0;

    bool ack = busRead(mag->busDev, LIS2MDL_REG_STATUS_REG, &status);
    if (!ack || !(status & LIS2MDL_STATUS_ZYXDA)) {
        return false;
    }

    ack = busReadBuf(mag->busDev, LIS2MDL_REG_OUTX_L, buf, 6);
    if (!ack) {
        return false;
    }

    const int16_t x = (int16_t)(buf[1] << 8 | buf[0]);
    const int16_t y = (int16_t)(buf[3] << 8 | buf[2]);
    const int16_t z = (int16_t)(buf[5] << 8 | buf[4]);

    // Adapt LIS2MDL left-handed frame to common sensor axis orientation, matching Betaflight.
    mag->magADCRaw[X] = -x;
    mag->magADCRaw[Y] = y;
    mag->magADCRaw[Z] = z;

    return true;
}

#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(magDev_t *mag)
{
    for (int retryCount = 0; retryCount < DETECTION_MAX_RETRY_COUNT; retryCount++) {
        delay(10);

        uint8_t sig = 0;
        const bool ack = busRead(mag->busDev, LIS2MDL_REG_WHO_AM_I, &sig);

        if (ack && sig == LIS2MDL_DEVICE_ID) {
            return true;
        }
    }

    return false;
}

bool lis2mdlDetect(magDev_t *mag)
{
    mag->busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_LIS2MDL, mag->magSensorToUse, OWNER_COMPASS);
    if (mag->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(mag)) {
        busDeviceDeInit(mag->busDev);
        return false;
    }

    mag->init = lis2mdlInit;
    mag->read = lis2mdlRead;

    return true;
}

#endif
