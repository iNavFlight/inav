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
#include <string.h>
#include <math.h>

#include "platform.h"

FILE_COMPILE_FOR_SPEED

#include "build/build_config.h"
#include "build/debug.h"

#include "common/axis.h"
#include "common/calibration.h"
#include "common/filter.h"
#include "common/log.h"
#include "common/maths.h"
#include "common/utils.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"
#include "config/feature.h"

#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_mpu.h"
#include "drivers/accgyro/accgyro_mpu3050.h"
#include "drivers/accgyro/accgyro_mpu6000.h"
#include "drivers/accgyro/accgyro_mpu6050.h"
#include "drivers/accgyro/accgyro_mpu6500.h"
#include "drivers/accgyro/accgyro_mpu9250.h"

#include "drivers/accgyro/accgyro_lsm303dlhc.h"
#include "drivers/accgyro/accgyro_l3g4200d.h"
#include "drivers/accgyro/accgyro_l3gd20.h"
#include "drivers/accgyro/accgyro_adxl345.h"
#include "drivers/accgyro/accgyro_mma845x.h"
#include "drivers/accgyro/accgyro_bma280.h"
#include "drivers/accgyro/accgyro_bmi160.h"
#include "drivers/accgyro/accgyro_icm20689.h"
#include "drivers/accgyro/accgyro_fake.h"
#include "drivers/io.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#include "io/beeper.h"
#include "io/statusindicator.h"

#include "scheduler/scheduler.h"

#include "sensors/boardalignment.h"
#include "sensors/gyro.h"
#include "sensors/sensors.h"

#include "flight/gyroanalyse.h"
#include "flight/rpm_filter.h"
#include "flight/dynamic_gyro_notch.h"

#ifdef USE_HARDWARE_REVISION_DETECTION
#include "hardware_revision.h"
#endif

FASTRAM gyro_t gyro; // gyro sensor object

#define MAX_GYRO_COUNT          1

STATIC_UNIT_TESTED gyroDev_t gyroDev[MAX_GYRO_COUNT];  // Not in FASTRAM since it may hold DMA buffers
STATIC_FASTRAM int16_t gyroTemperature[MAX_GYRO_COUNT];
STATIC_FASTRAM_UNIT_TESTED zeroCalibrationVector_t gyroCalibration[MAX_GYRO_COUNT];

STATIC_FASTRAM filterApplyFnPtr gyroLpfApplyFn;
STATIC_FASTRAM filter_t gyroLpfState[XYZ_AXIS_COUNT];

STATIC_FASTRAM filterApplyFnPtr gyroLpf2ApplyFn;
STATIC_FASTRAM filter_t gyroLpf2State[XYZ_AXIS_COUNT];

STATIC_FASTRAM filterApplyFnPtr notchFilter1ApplyFn;
STATIC_FASTRAM void *notchFilter1[XYZ_AXIS_COUNT];

#ifdef USE_DYNAMIC_FILTERS

EXTENDED_FASTRAM gyroAnalyseState_t gyroAnalyseState;
EXTENDED_FASTRAM dynamicGyroNotchState_t dynamicGyroNotchState;

#endif

PG_REGISTER_WITH_RESET_TEMPLATE(gyroConfig_t, gyroConfig, PG_GYRO_CONFIG, 10);

PG_RESET_TEMPLATE(gyroConfig_t, gyroConfig,
    .gyro_lpf = GYRO_LPF_42HZ,      // 42HZ value is defined for Invensense/TDK gyros
    .gyro_soft_lpf_hz = 60,
    .gyro_soft_lpf_type = FILTER_BIQUAD,
    .gyro_align = ALIGN_DEFAULT,
    .gyroMovementCalibrationThreshold = 32,
    .looptime = 1000,
    .gyroSync = 1,
    .gyro_to_use = 0,
    .gyro_notch_hz = 0,
    .gyro_notch_cutoff = 1,
    .gyro_stage2_lowpass_hz = 0,
    .gyro_stage2_lowpass_type = FILTER_BIQUAD,
    .dynamicGyroNotchRange = DYN_NOTCH_RANGE_MEDIUM,
    .dynamicGyroNotchQ = 120,
    .dynamicGyroNotchMinHz = 150,
    .dynamicGyroNotchEnabled = 0,
);

STATIC_UNIT_TESTED gyroSensor_e gyroDetect(gyroDev_t *dev, gyroSensor_e gyroHardware)
{
    dev->gyroAlign = ALIGN_DEFAULT;

    switch (gyroHardware) {
    case GYRO_AUTODETECT:
        FALLTHROUGH;

#ifdef USE_IMU_MPU6050
    case GYRO_MPU6050:
        if (mpu6050GyroDetect(dev)) {
            gyroHardware = GYRO_MPU6050;
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_IMU_L3G4200D
    case GYRO_L3G4200D:
        if (l3g4200dDetect(dev)) {
            gyroHardware = GYRO_L3G4200D;
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_IMU_MPU3050
    case GYRO_MPU3050:
        if (mpu3050Detect(dev)) {
            gyroHardware = GYRO_MPU3050;
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_IMU_L3GD20
    case GYRO_L3GD20:
        if (l3gd20Detect(dev)) {
            gyroHardware = GYRO_L3GD20;
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_IMU_MPU6000
    case GYRO_MPU6000:
        if (mpu6000GyroDetect(dev)) {
            gyroHardware = GYRO_MPU6000;
            break;
        }
        FALLTHROUGH;
#endif

#if defined(USE_IMU_MPU6500)
    case GYRO_MPU6500:
        if (mpu6500GyroDetect(dev)) {
            gyroHardware = GYRO_MPU6500;
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_IMU_MPU9250
    case GYRO_MPU9250:
        if (mpu9250GyroDetect(dev)) {
            gyroHardware = GYRO_MPU9250;
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_IMU_BMI160
    case GYRO_BMI160:
        if (bmi160GyroDetect(dev)) {
            gyroHardware = GYRO_BMI160;
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_IMU_ICM20689
    case GYRO_ICM20689:
        if (icm20689GyroDetect(dev)) {
            gyroHardware = GYRO_ICM20689;
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_IMU_FAKE
    case GYRO_FAKE:
        if (fakeGyroDetect(dev)) {
            gyroHardware = GYRO_FAKE;
            break;
        }
        FALLTHROUGH;
#endif

    default:
    case GYRO_NONE:
        gyroHardware = GYRO_NONE;
    }

    return gyroHardware;
}

static void initGyroFilter(filterApplyFnPtr *applyFn, filter_t state[], uint8_t type, uint16_t cutoff)
{
    *applyFn = nullFilterApply;
    if (cutoff > 0) {
        switch (type) 
        {
            case FILTER_PT1:
                *applyFn = (filterApplyFnPtr)pt1FilterApply;
                for (int axis = 0; axis < 3; axis++) {
                    pt1FilterInit(&state[axis].pt1, cutoff, getLooptime()* 1e-6f);
                }
                break;
            case FILTER_BIQUAD:
                *applyFn = (filterApplyFnPtr)biquadFilterApply;
                for (int axis = 0; axis < 3; axis++) {
                    biquadFilterInitLPF(&state[axis].biquad, cutoff, getLooptime());
                }
                break;
        }
    }
}

static void gyroInitFilters(void)
{
    STATIC_FASTRAM biquadFilter_t gyroFilterNotch_1[XYZ_AXIS_COUNT];
    notchFilter1ApplyFn = nullFilterApply;
    
    initGyroFilter(&gyroLpf2ApplyFn, gyroLpf2State, gyroConfig()->gyro_stage2_lowpass_type, gyroConfig()->gyro_stage2_lowpass_hz);
    initGyroFilter(&gyroLpfApplyFn, gyroLpfState, gyroConfig()->gyro_soft_lpf_type, gyroConfig()->gyro_soft_lpf_hz);

    if (gyroConfig()->gyro_notch_hz) {
        notchFilter1ApplyFn = (filterApplyFnPtr)biquadFilterApply;
        for (int axis = 0; axis < 3; axis++) {
            notchFilter1[axis] = &gyroFilterNotch_1[axis];
            biquadFilterInitNotch(notchFilter1[axis], getLooptime(), gyroConfig()->gyro_notch_hz, gyroConfig()->gyro_notch_cutoff);
        }
    }
}

bool gyroInit(void)
{
    memset(&gyro, 0, sizeof(gyro));

    // Set inertial sensor tag (for dual-gyro selection)
#ifdef USE_DUAL_GYRO
    gyroDev[0].imuSensorToUse = gyroConfig()->gyro_to_use;
#else
    gyroDev[0].imuSensorToUse = 0;
#endif

    // Detecting gyro0
    gyroSensor_e gyroHardware = gyroDetect(&gyroDev[0], GYRO_AUTODETECT);
    if (gyroHardware == GYRO_NONE) {
        gyro.initialized = false;
        detectedSensors[SENSOR_INDEX_GYRO] = GYRO_NONE;
        return true;
    }

    // Gyro is initialized
    gyro.initialized = true;
    detectedSensors[SENSOR_INDEX_GYRO] = gyroHardware;
    sensorsSet(SENSOR_GYRO);

    // Driver initialisation
    gyroDev[0].lpf = gyroConfig()->gyro_lpf;
    gyroDev[0].requestedSampleIntervalUs = gyroConfig()->looptime;
    gyroDev[0].sampleRateIntervalUs = gyroConfig()->looptime;
    gyroDev[0].initFn(&gyroDev[0]);

    // initFn will initialize sampleRateIntervalUs to actual gyro sampling rate (if driver supports it). Calculate target looptime using that value
    gyro.targetLooptime = gyroConfig()->gyroSync ? gyroDev[0].sampleRateIntervalUs : gyroConfig()->looptime;

    // At this poinrt gyroDev[0].gyroAlign was set up by the driver from the busDev record
    // If configuration says different - override
    if (gyroConfig()->gyro_align != ALIGN_DEFAULT) {
        gyroDev[0].gyroAlign = gyroConfig()->gyro_align;
    }

    gyroInitFilters();
#ifdef USE_DYNAMIC_FILTERS
    dynamicGyroNotchFiltersInit(&dynamicGyroNotchState);
    gyroDataAnalyseStateInit(
        &gyroAnalyseState, 
        gyroConfig()->dynamicGyroNotchMinHz,
        gyroConfig()->dynamicGyroNotchRange,
        getLooptime()
    );
#endif
    return true;
}

void gyroStartCalibration(void)
{
    if (!gyro.initialized) {
        return;
    }

    zeroCalibrationStartV(&gyroCalibration[0], CALIBRATING_GYRO_TIME_MS, gyroConfig()->gyroMovementCalibrationThreshold, false);
}

bool gyroIsCalibrationComplete(void)
{
    if (!gyro.initialized) {
        return true;
    }

    return zeroCalibrationIsCompleteV(&gyroCalibration[0]) && zeroCalibrationIsSuccessfulV(&gyroCalibration[0]);
}

STATIC_UNIT_TESTED void performGyroCalibration(gyroDev_t *dev, zeroCalibrationVector_t *gyroCalibration)
{
    fpVector3_t v;

    // Consume gyro reading
    v.v[0] = dev->gyroADCRaw[0];
    v.v[1] = dev->gyroADCRaw[1];
    v.v[2] = dev->gyroADCRaw[2];

    zeroCalibrationAddValueV(gyroCalibration, &v);

    // Check if calibration is complete after this cycle
    if (zeroCalibrationIsCompleteV(gyroCalibration)) {
        zeroCalibrationGetZeroV(gyroCalibration, &v);
        dev->gyroZero[0] = v.v[0];
        dev->gyroZero[1] = v.v[1];
        dev->gyroZero[2] = v.v[2];

        LOG_D(GYRO, "Gyro calibration complete (%d, %d, %d)", dev->gyroZero[0], dev->gyroZero[1], dev->gyroZero[2]);
        schedulerResetTaskStatistics(TASK_SELF); // so calibration cycles do not pollute tasks statistics
    }
    else {
        dev->gyroZero[0] = 0;
        dev->gyroZero[1] = 0;
        dev->gyroZero[2] = 0;
    }
}

/*
 * Calculate rotation rate in rad/s in body frame
 */
void gyroGetMeasuredRotationRate(fpVector3_t *measuredRotationRate)
{
    for (int axis = 0; axis < 3; axis++) {
        measuredRotationRate->v[axis] = DEGREES_TO_RADIANS(gyro.gyroADCf[axis]);
    }
}

static bool FAST_CODE NOINLINE gyroUpdateAndCalibrate(gyroDev_t * gyroDev, zeroCalibrationVector_t * gyroCal, float * gyroADCf)
{
    // range: +/- 8192; +/- 2000 deg/sec
    if (gyroDev->readFn(gyroDev)) {
        if (zeroCalibrationIsCompleteV(gyroCal)) {
            int32_t gyroADCtmp[XYZ_AXIS_COUNT];

            // Copy gyro value into int32_t (to prevent overflow) and then apply calibration and alignment
            gyroADCtmp[X] = (int32_t)gyroDev->gyroADCRaw[X] - (int32_t)gyroDev->gyroZero[X];
            gyroADCtmp[Y] = (int32_t)gyroDev->gyroADCRaw[Y] - (int32_t)gyroDev->gyroZero[Y];
            gyroADCtmp[Z] = (int32_t)gyroDev->gyroADCRaw[Z] - (int32_t)gyroDev->gyroZero[Z];

            // Apply sensor alignment
            applySensorAlignment(gyroADCtmp, gyroADCtmp, gyroDev->gyroAlign);
            applyBoardAlignment(gyroADCtmp);

            // Convert to deg/s and store in unified data
            gyroADCf[X] = (float)gyroADCtmp[X] * gyroDev->scale;
            gyroADCf[Y] = (float)gyroADCtmp[Y] * gyroDev->scale;
            gyroADCf[Z] = (float)gyroADCtmp[Z] * gyroDev->scale;

            return true;
        } else {
            performGyroCalibration(gyroDev, gyroCal);

            // Reset gyro values to zero to prevent other code from using uncalibrated data
            gyroADCf[X] = 0.0f;
            gyroADCf[Y] = 0.0f;
            gyroADCf[Z] = 0.0f;

            return false;
        }
    } else {
        // no gyro reading to process
        return false;
    }
}

void FAST_CODE NOINLINE gyroUpdate()
{
    if (!gyro.initialized) {
        return;
    }

    if (!gyroUpdateAndCalibrate(&gyroDev[0], &gyroCalibration[0], gyro.gyroADCf)) {
        return;
    }

    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        // At this point gyro.gyroADCf contains unfiltered gyro value [deg/s]
        float gyroADCf = gyro.gyroADCf[axis];

        DEBUG_SET(DEBUG_GYRO, axis, lrintf(gyroADCf));

#ifdef USE_RPM_FILTER
        DEBUG_SET(DEBUG_RPM_FILTER, axis, gyroADCf);
        gyroADCf = rpmFilterGyroApply(axis, gyroADCf);
        DEBUG_SET(DEBUG_RPM_FILTER, axis + 3, gyroADCf);
#endif

        gyroADCf = gyroLpf2ApplyFn((filter_t *) &gyroLpf2State[axis], gyroADCf);
        gyroADCf = gyroLpfApplyFn((filter_t *) &gyroLpfState[axis], gyroADCf);
        gyroADCf = notchFilter1ApplyFn(notchFilter1[axis], gyroADCf);

#ifdef USE_DYNAMIC_FILTERS
        if (dynamicGyroNotchState.enabled) {
            gyroDataAnalysePush(&gyroAnalyseState, axis, gyroADCf);
            DEBUG_SET(DEBUG_DYNAMIC_FILTER, axis, gyroADCf);
            gyroADCf = dynamicGyroNotchFiltersApply(&dynamicGyroNotchState, axis, gyroADCf);
            DEBUG_SET(DEBUG_DYNAMIC_FILTER, axis + 3, gyroADCf);
        }
#endif
        gyro.gyroADCf[axis] = gyroADCf;
    }

#ifdef USE_DYNAMIC_FILTERS
    if (dynamicGyroNotchState.enabled) {
        gyroDataAnalyse(&gyroAnalyseState);

        if (gyroAnalyseState.filterUpdateExecute) {
            dynamicGyroNotchFiltersUpdate(
                &dynamicGyroNotchState, 
                gyroAnalyseState.filterUpdateAxis, 
                gyroAnalyseState.filterUpdateFrequency
            );
        }
    }
#endif

}

bool gyroReadTemperature(void)
{
    if (!gyro.initialized) {
        return false;
    }

    // Read gyro sensor temperature. temperatureFn returns temperature in [degC * 10]
    if (gyroDev[0].temperatureFn) {
        return gyroDev[0].temperatureFn(&gyroDev[0], &gyroTemperature[0]);
    }

    return false;
}

int16_t gyroGetTemperature(void)
{
    if (!gyro.initialized) {
        return 0;
    }

    return gyroTemperature[0];
}

int16_t gyroRateDps(int axis)
{
    if (!gyro.initialized) {
        return 0;
    }

    return lrintf(gyro.gyroADCf[axis]);
}

bool gyroSyncCheckUpdate(void)
{
    if (!gyro.initialized) {
        return false;
    }

    if (!gyroDev[0].intStatusFn) {
        return false;
    }

    return gyroDev[0].intStatusFn(&gyroDev[0]);
}
