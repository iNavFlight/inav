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

#ifdef USE_MAG_LIS3MDL

#include "common/utils.h"
#include "common/axis.h"

#include "drivers/time.h"
#include "drivers/sensor.h"

#include "drivers/compass/compass.h"

#include "compass_lis3mdl.h"

#define LIS3MDL_MAG_I2C_ADDRESS     0x1E
#define LIS3MDL_MAG_I2C_ADDRESS2    0x1C // LIS3MDL has two possible addresses, selected by a physical pin
#define LIS3MDL_DEVICE_ID           0x3D

#define LIS3MDL_REG_WHO_AM_I        0x0F

#define LIS3MDL_REG_CTRL_REG1       0x20
#define LIS3MDL_REG_CTRL_REG2       0x21
#define LIS3MDL_REG_CTRL_REG3       0x22
#define LIS3MDL_REG_CTRL_REG4       0x23
#define LIS3MDL_REG_CTRL_REG5       0x24

#define LIS3MDL_REG_STATUS_REG      0x27

#define LIS3MDL_REG_OUT_X_L         0x28
#define LIS3MDL_REG_OUT_X_H         0x29
#define LIS3MDL_REG_OUT_Y_L         0x2A
#define LIS3MDL_REG_OUT_Y_H         0x2B
#define LIS3MDL_REG_OUT_Z_L         0x2C
#define LIS3MDL_REG_OUT_Z_H         0x2D

#define LIS3MDL_TEMP_OUT_L          0x2E
#define LIS3MDL_TEMP_OUT_H          0x2F

#define LIS3MDL_INT_CFG             0x30
#define LIS3MDL_INT_SRC             0x31
#define LIS3MDL_THS_L               0x32
#define LIS3MDL_THS_H               0x33

// CTRL_REG1
#define LIS3MDL_TEMP_EN             0x80  // Default 0
#define LIS3MDL_OM_LOW_POWER        0x00  // Default
#define LIS3MDL_OM_MED_PROF         0x20
#define LIS3MDL_OM_HI_PROF          0x40
#define LIS3MDL_OM_ULTRA_HI_PROF    0x60
#define LIS3MDL_DO_0_625            0x00
#define LIS3MDL_DO_1_25             0x04
#define LIS3MDL_DO_2_5              0x08
#define LIS3MDL_DO_5                0x0C
#define LIS3MDL_DO_10               0x10  // Default
#define LIS3MDL_DO_20               0x14
#define LIS3MDL_DO_40               0x18
#define LIS3MDL_DO_80               0x1C
#define LIS3MDL_FAST_ODR            0x02

// CTRL_REG2
#define LIS3MDL_FS_4GAUSS           0x00  // Default
#define LIS3MDL_FS_8GAUSS           0x20
#define LIS3MDL_FS_12GAUSS          0x40
#define LIS3MDL_FS_16GAUSS          0x60
#define LIS3MDL_REBOOT              0x08
#define LIS3MDL_SOFT_RST            0x04

// CTRL_REG3
#define LIS3MDL_LP                  0x20  // Default 0
#define LIS3MDL_SIM                 0x04  // Default 0
#define LIS3MDL_MD_CONTINUOUS       0x00  // Default
#define LIS3MDL_MD_SINGLE           0x01
#define LIS3MDL_MD_POWERDOWN        0x03

// CTRL_REG4
#define LIS3MDL_ZOM_LP              0x00  // Default
#define LIS3MDL_ZOM_MP              0x04
#define LIS3MDL_ZOM_HP              0x08
#define LIS3MDL_ZOM_UHP             0x0C
#define LIS3MDL_BLE                 0x02  // Default 0

// CTRL_REG5
#define LIS3MDL_FAST_READ           0x80  // Default 0
#define LIS3MDL_BDU                 0x40  // Default 0

// Targets of the non-blocking reads, filled by the bus driver in the background
static uint8_t lis3mdlStatus;
static uint8_t lis3mdlData[6];

#define LIS3MDL_STATUS_ZYXDA    0x08

// Two transfers per sample: the status byte first, the data only when ZYXDA confirms a fresh sample
static busReadStepResult_e lis3mdlReadStart(magDev_t * mag, bool firstStep)
{
    static bool statusStarted = false;

    if (firstStep) {
        statusStarted = false;
        lis3mdlStatus = 0;
    }

    if (!statusStarted) {
        if (!busReadBufStart(mag->busDev, LIS3MDL_REG_STATUS_REG, &lis3mdlStatus, 1)) {
            return BUS_READ_STEP_BUSY;
        }
        statusStarted = true;
        return BUS_READ_STEP_NEXT;
    }

    if (!(lis3mdlStatus & LIS3MDL_STATUS_ZYXDA)) {
        return BUS_READ_STEP_LAST;      // nothing new to fetch, lis3mdlRead() reports the miss
    }

    if (!busReadBufStart(mag->busDev, LIS3MDL_REG_OUT_X_L, lis3mdlData, sizeof(lis3mdlData))) {
        return BUS_READ_STEP_BUSY;
    }

    return BUS_READ_STEP_LAST;
}

static bool lis3mdlRead(magDev_t * mag)
{
    const uint8_t *buf = lis3mdlData;

    if (!(lis3mdlStatus & LIS3MDL_STATUS_ZYXDA)) {
        mag->magADCRaw[X] = 0;
        mag->magADCRaw[Y] = 0;
        mag->magADCRaw[Z] = 0;
        return false;
    }

    mag->magADCRaw[X] = (int16_t)(buf[1] << 8 | buf[0]) / 4;
    mag->magADCRaw[Y] = (int16_t)(buf[3] << 8 | buf[2]) / 4;
    mag->magADCRaw[Z] = (int16_t)(buf[5] << 8 | buf[4]) / 4;

    return true;
}

#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(magDev_t * mag)
{
    for (int retryCount = 0; retryCount < DETECTION_MAX_RETRY_COUNT; retryCount++) {
        delay(10);

        uint8_t sig = 0;
        bool ack = busRead(mag->busDev, LIS3MDL_REG_WHO_AM_I, &sig);

        if (ack && sig == LIS3MDL_DEVICE_ID) {
            return true;
        }
    }

    return false;
}

static bool lis3mdlInit(magDev_t *mag)
{
	busWrite(mag->busDev, LIS3MDL_REG_CTRL_REG2, LIS3MDL_FS_4GAUSS);   // Configuration Register 2
    busWrite(mag->busDev, LIS3MDL_REG_CTRL_REG1, LIS3MDL_TEMP_EN | LIS3MDL_OM_ULTRA_HI_PROF | LIS3MDL_DO_80);   // Configuration Register 1
    busWrite(mag->busDev, LIS3MDL_REG_CTRL_REG5, LIS3MDL_BDU);   // Configuration Register 5
    busWrite(mag->busDev, LIS3MDL_REG_CTRL_REG4, LIS3MDL_ZOM_UHP);   // Configuration Register 4
    busWrite(mag->busDev, LIS3MDL_REG_CTRL_REG3, 0x00);   // Configuration Register 3

    return true;
}

bool lis3mdlDetect(magDev_t * mag)
{
    mag->busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_LIS3MDL, mag->magSensorToUse, OWNER_COMPASS);
    if (mag->busDev == NULL) {
        return false;
    }

// TODO: Add check for second possible I2C address

    if (!deviceDetect(mag)) {
        busDeviceDeInit(mag->busDev);
        return false;
    }

    mag->init = lis3mdlInit;
    mag->readStart = lis3mdlReadStart;
    mag->read = lis3mdlRead;

    return true;
}

#endif
