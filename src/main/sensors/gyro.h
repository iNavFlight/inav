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
#include "common/time.h"
#include "config/parameter_group.h"
#include "drivers/sensor.h"
#include "flight/dynamic_gyro_notch.h"
#include "flight/secondary_dynamic_gyro_notch.h"
#if !defined(SITL_BUILD)
#include "arm_math.h"
#else
#include <math.h>
#endif

typedef enum {
    GYRO_NONE = 0,
    GYRO_AUTODETECT,
    GYRO_MPU6000,
    GYRO_MPU6500,
    GYRO_MPU9250,
    GYRO_BMI160,
    GYRO_ICM20689,
    GYRO_BMI088,
    GYRO_ICM42605,
    GYRO_BMI270,
    GYRO_LSM6DXX,
    GYRO_FAKE
   
} gyroSensor_e;

typedef enum {
    DYNAMIC_NOTCH_MODE_2D = 0,
    DYNAMIC_NOTCH_MODE_3D
} dynamicGyroNotchMode_e;

typedef enum {
    GYRO_FILTER_MODE_OFF = 0,
    GYRO_FILTER_MODE_STATIC = 1,
    GYRO_FILTER_MODE_DYNAMIC = 2,
    GYRO_FILTER_MODE_ADAPTIVE = 3
} gyroFilterMode_e;

typedef struct gyro_s {
    bool initialized;
    uint32_t targetLooptime;
    float gyroADCf[XYZ_AXIS_COUNT];
    float gyroRaw[XYZ_AXIS_COUNT];
} gyro_t;

extern gyro_t gyro;
extern dynamicGyroNotchState_t dynamicGyroNotchState;

typedef struct gyroConfig_s {
    uint16_t looptime;                      // imu loop time in us
    uint16_t  gyro_anti_aliasing_lpf_hz;
#ifdef USE_DUAL_GYRO
    uint8_t  gyro_to_use;
#endif
    uint16_t gyro_main_lpf_hz;
    uint16_t gyroDynamicLpfMinHz;
    uint16_t gyroDynamicLpfMaxHz;
    uint8_t gyroDynamicLpfCurveExpo;
#ifdef USE_DYNAMIC_FILTERS
    uint16_t dynamicGyroNotchQ;
    uint16_t dynamicGyroNotchMinHz;
    uint8_t dynamicGyroNotchEnabled;
    uint8_t dynamicGyroNotchMode;
    uint16_t dynamicGyroNotch3dQ;
#endif
#ifdef USE_GYRO_KALMAN
    uint16_t kalman_q;
    uint8_t kalmanEnabled;
#endif
    bool init_gyro_cal_enabled;
    int16_t gyro_zero_cal[XYZ_AXIS_COUNT];
    float gravity_cmss_cal;
#ifdef USE_ADAPTIVE_FILTER
    float adaptiveFilterTarget;
    uint16_t adaptiveFilterMinHz;
    uint16_t adaptiveFilterMaxHz;
    float adaptiveFilterStdLpfHz;
    float adaptiveFilterHpfHz;
    float adaptiveFilterIntegratorThresholdHigh;
    float adaptiveFilterIntegratorThresholdLow;
#endif
    uint8_t gyroFilterMode;

    uint8_t gyroLuluSampleCount;
    bool gyroLuluEnabled;
} gyroConfig_t;

PG_DECLARE(gyroConfig_t, gyroConfig);

bool gyroInit(void);
void gyroGetMeasuredRotationRate(fpVector3_t *imuMeasuredRotationBF);
void gyroUpdate(void);
void gyroFilter(void);
void gyroStartCalibration(void);
bool gyroIsCalibrationComplete(void);
bool gyroReadTemperature(void);
int16_t gyroGetTemperature(void);
int16_t gyroRateDps(int axis);
void gyroUpdateDynamicLpf(float cutoffFreq);
float averageAbsGyroRates(void);
