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

#pragma once

#include "common/axis.h"
#include "common/maths.h"
#include "common/vector.h"
#include "config/parameter_group.h"
#include "drivers/accgyro/accgyro.h"
#include "sensors/sensors.h"

#define GRAVITY_CMSS    980.665f
#define GRAVITY_MSS     9.80665f

#define ACC_CLIPPING_THRESHOLD_G        15.9f
#define ACC_VIBE_FLOOR_FILT_HZ          5.0f
#define ACC_VIBE_FILT_HZ                2.0f

// Type of accelerometer used/detected
typedef enum {
    ACC_NONE = 0,
    ACC_AUTODETECT,
    ACC_MPU6000,
    ACC_MPU6500,
    ACC_MPU9250,
    ACC_BMI160,
    ACC_ICM20689,
    ACC_BMI088,
    ACC_ICM42605,
    ACC_BMI270,
    ACC_LSM6DXX,
    ACC_FAKE,
    ACC_MAX = ACC_FAKE
} accelerationSensor_e;

typedef struct {
    float min;
    float max;
} acc_extremes_t;

typedef struct acc_s {
    accDev_t dev;
    uint32_t accTargetLooptime;
    float accADCf[XYZ_AXIS_COUNT]; // acceleration in g
    float accVibeSq[XYZ_AXIS_COUNT];
    float accVibe;
    uint32_t accClipCount;
    bool isClipped;
    acc_extremes_t extremes[XYZ_AXIS_COUNT];
    float maxG;
} acc_t;

extern acc_t acc;

typedef struct accelerometerConfig_s {
    uint8_t acc_hardware;                   // Which acc hardware to use on boards with more than one device
    uint16_t acc_lpf_hz;                    // cutoff frequency for the low pass filter used on the acc z-axis for althold in Hz
    flightDynamicsTrims_t accZero;          // Accelerometer offset
    flightDynamicsTrims_t accGain;          // Accelerometer gain to read exactly 1G
    uint8_t acc_notch_hz;                   // Accelerometer notch filter frequency
    uint8_t acc_notch_cutoff;               // Accelerometer notch filter cutoff frequency
    uint8_t acc_soft_lpf_type;              // Accelerometer LPF type
    float acc_temp_correction;              // Accelerometer temperature compensation factor
} accelerometerConfig_t;

PG_DECLARE(accelerometerConfig_t, accelerometerConfig);

bool accInit(uint32_t accTargetLooptime);
bool accIsCalibrationComplete(void);
void accStartCalibration(void);
void accGetMeasuredAcceleration(fpVector3_t *measuredAcc);
const acc_extremes_t* accGetMeasuredExtremes(void);
float accGetMeasuredMaxG(void);
void updateAccExtremes(void);
void resetGForceStats(void);
void accGetVibrationLevels(fpVector3_t *accVibeLevels);
float accGetVibrationLevel(void);
uint32_t accGetClipCount(void);
bool accIsClipped(void);
void accUpdate(void);
void accSetCalibrationValues(void);
void accInitFilters(void);
bool accIsHealthy(void);
bool accGetCalibrationAxisStatus(int axis);
uint8_t accGetCalibrationAxisFlags(void);
