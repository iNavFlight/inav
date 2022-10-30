/*
 * This file is part of Cleanflight, Betaflight and INAV.
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
#include <string.h>

#include <platform.h>

#include "build/build_config.h"
#include "build/debug.h"
#include "common/utils.h"

#include "drivers/io.h"
#include "drivers/bus.h"
#include "drivers/time.h"
#include "drivers/barometer/barometer.h"

#include "drivers/barometer/barometer_2smpb_02b.h"

#if defined(USE_BARO) && defined(USE_BARO_B2SMPB)

#define BARO_2SMBP_I2C_ADDRESS 0x70

#define BARO_2SMBP_CHIP_ID 0x5C

#define REG_CHIP_ID 0xD1
#define REG_RESET 0xE0

#define REG_COE_PR11 0xA0
#define REG_COE_PR21 0xA3
#define REG_COE_PR31 0xA5
#define REG_COE_TEMP11 0xA7
#define REG_COE_TEMP21 0xA9
#define REG_COE_TEMP31 0xAB
#define REG_COE_PTAT11 0xAD
#define REG_COE_PTAT21 0xB1
#define REG_COE_PTAT31 0xB3

#define REG_IIR_CNT 0xF1
#define REG_DEVICE_STAT 0xF3
#define REG_CTRL_MEAS 0xF4
#define REG_IO_SETUP 0xF5
#define REG_PRESS_TXD2 0xF7

// Value for CTRL_MEAS with 4x temperature averaging, 32x perssure, forced mode
#define REG_CLT_MEAS_VAL_TAVG4X_PAVG32X_FORCED ((0x03 << 5) | (0x05 << 2) | 0x01)

// IIR coefficient setting 8x
#define REG_IIR_CNT_VAL_8X 0x03

typedef struct {
    float aa;
    float ba;
    int32_t ca;
    float ap;
    float bp;
    int32_t cp;
    float at;
    float bt;
    float ct;
} calibrationCoefficients_t;

typedef struct {
    calibrationCoefficients_t   calib;
    float                       pressure;       // Pa
    float                       temperature;    // DegC
} baroState_t;

static baroState_t  baroState;
static uint8_t baroDataBuf[6];


static int32_t readSignedRegister(busDevice_t * busDev, uint8_t reg, uint8_t nBytes)
{
    uint8_t buf[3];
    uint32_t rawValue = 0;

    busReadBuf(busDev, reg, &buf[0], nBytes);

    for (int i=0; i<nBytes; i++) {
        rawValue += (uint32_t)buf[i] << (8 * (nBytes - i - 1));
    }

    // 2's complement
    if (rawValue & ((int32_t)1 << (8 * nBytes - 1))) {
        // Negative
        return ((int32_t)rawValue) - ((int32_t)1 << (8 * nBytes));
    }
    else {
        return rawValue;
    }
}

static int32_t getSigned24bitValue(uint8_t * pData)
{
    uint32_t raw;

    raw = (((uint32_t)pData[0] << 16) | ((uint32_t)pData[1] << 8) | (uint32_t)pData[2]) - ((uint32_t)1 << 23);

    return raw;
}

static bool deviceConfigure(busDevice_t * busDev)
{
    /** Note: Chip reset causes I2C error due missing ACK. This causes interrupt based read (busReadRegisterBufferStart)
        to not work (read stops due to error flags). It works fine without chip reset. **/

    //busWrite(busDev, REG_RESET, 0xE6);

    // No need to write IO_SETUP register: default values are fine

    // Read calibration coefficients and scale them
    baroState.calib.aa = (4.2e-4f  * readSignedRegister(busDev, REG_COE_PTAT31, 2)) / 32767;
    baroState.calib.ba = (8.0e0f  * readSignedRegister(busDev, REG_COE_PTAT21, 2)) / 32767 - 1.6e2f;
    baroState.calib.ca = readSignedRegister(busDev, REG_COE_PTAT11, 3);
    baroState.calib.ap = (3.0e-5f  * readSignedRegister(busDev, REG_COE_PR31, 2)) / 32767;
    baroState.calib.bp = (10 * readSignedRegister(busDev, REG_COE_PR21, 2)) / 32767 + 3.0e1f;
    baroState.calib.cp = readSignedRegister(busDev, REG_COE_PR11, 3);
    baroState.calib.at = (8.0e-11f  * readSignedRegister(busDev, REG_COE_TEMP31, 2)) / 32767;
    baroState.calib.bt = (1.6e-6f  * readSignedRegister(busDev, REG_COE_TEMP21, 2)) / 32767 - 6.6e-6f;
    baroState.calib.ct = (8.5e-3f  * readSignedRegister(busDev, REG_COE_TEMP11, 2)) / 32767 + 4.0e-2f;

    // Configure IIR filter
    busWrite(busDev, REG_IIR_CNT, REG_IIR_CNT_VAL_8X);

    return true;
}

#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(busDevice_t * busDev)
{
    for (int retry = 0; retry < DETECTION_MAX_RETRY_COUNT; retry++) {
        uint8_t chipId;

        busRead(busDev, REG_CHIP_ID, &chipId);

        if (chipId == BARO_2SMBP_CHIP_ID) {
            return true;
        }

        delay(50);
    };

    return false;
}

static bool b2smpbStartUP(baroDev_t *baro)
{
    // start a forced measurement
    return busWrite(baro->busDev, REG_CTRL_MEAS, REG_CLT_MEAS_VAL_TAVG4X_PAVG32X_FORCED);
}

static bool b2smpbGetUP(baroDev_t *baro)
{
    int32_t dtp;
    float tr, pl, tmp;

    if (!busReadBuf(baro->busDev, REG_PRESS_TXD2, &baroDataBuf[0], 6)) {
        return false;
    }

    // Calculate compensated temperature
    dtp = getSigned24bitValue(&baroDataBuf[3]);

    tmp = baroState.calib.ba * baroState.calib.ba;
    tr = (-1 * baroState.calib.ba - sqrtf(tmp - 4 * baroState.calib.aa * (baroState.calib.ca - dtp))) / (2 * baroState.calib.aa);
    baroState.temperature = tr / 256;

    // Calculate raw pressure
    dtp = getSigned24bitValue(&baroDataBuf[0]);

    tmp = baroState.calib.bp * baroState.calib.bp;
    pl = (sqrtf(tmp - 4 * baroState.calib.ap * (baroState.calib.cp - dtp)) - baroState.calib.bp) / (2 * baroState.calib.ap);

    // Calculate temperature compensated pressure
    tmp = tr * tr;
    baroState.pressure = pl / (baroState.calib.at * tmp + baroState.calib.bt * tr + baroState.calib.ct + 1);

    return true;
}

static bool deviceCalculate(baroDev_t *baro, int32_t *pressure, int32_t *temperature)
{
    UNUSED(baro);

    if (pressure) {
        *pressure = baroState.pressure;
    }

    if (temperature) {
        *temperature = (baroState.temperature * 100);   // to centidegrees
    }

    return true;
}

bool baro2SMPB02BDetect(baroDev_t *baro)
{
    baro->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_B2SMPB, 0, OWNER_BARO);

    if (baro->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(baro->busDev)) {
        busDeviceDeInit(baro->busDev);
        return false;
    }

    if (!deviceConfigure(baro->busDev)) {
        busDeviceDeInit(baro->busDev);
        return false;
    }

    baro->up_delay = 35000; // measurement takes 33.7 ms with 4x / 32x averaging
    baro->start_up = b2smpbStartUP;
    baro->get_up = b2smpbGetUP;

    baro->ut_delay = 0;
    baro->start_ut = NULL;
    baro->get_ut = NULL;

    baro->calculate = deviceCalculate;

    return true;
}

#endif
