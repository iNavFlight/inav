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

#include "build/build_config.h"
#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/filter.h"
#include "common/utils.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

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
#include "drivers/accgyro/accgyro_fake.h"
#include "drivers/io.h"
#include "drivers/logging.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#include "io/beeper.h"
#include "io/statusindicator.h"

#include "scheduler/scheduler.h"

#include "sensors/boardalignment.h"
#include "sensors/gyro.h"
#include "sensors/sensors.h"

#ifdef USE_HARDWARE_REVISION_DETECTION
#include "hardware_revision.h"
#endif

FASTRAM gyro_t gyro; // gyro sensor object

STATIC_UNIT_TESTED gyroDev_t gyroDev0;  // Not in FASTRAM since it may hold DMA buffers
STATIC_FASTRAM int16_t gyroTemperature0;

typedef struct gyroCalibration_s {
    int32_t g[XYZ_AXIS_COUNT];
    stdev_t var[XYZ_AXIS_COUNT];
    uint16_t calibratingG;
} gyroCalibration_t;

STATIC_FASTRAM_UNIT_TESTED gyroCalibration_t gyroCalibration;
STATIC_FASTRAM int32_t gyroADC[XYZ_AXIS_COUNT];

STATIC_FASTRAM filterApplyFnPtr softLpfFilterApplyFn;
STATIC_FASTRAM void *softLpfFilter[XYZ_AXIS_COUNT];

#ifdef USE_GYRO_NOTCH_1
STATIC_FASTRAM filterApplyFnPtr notchFilter1ApplyFn;
STATIC_FASTRAM void *notchFilter1[XYZ_AXIS_COUNT];
#endif
#ifdef USE_GYRO_NOTCH_2
STATIC_FASTRAM filterApplyFnPtr notchFilter2ApplyFn;
STATIC_FASTRAM void *notchFilter2[XYZ_AXIS_COUNT];
#endif

PG_REGISTER_WITH_RESET_TEMPLATE(gyroConfig_t, gyroConfig, PG_GYRO_CONFIG, 2);

PG_RESET_TEMPLATE(gyroConfig_t, gyroConfig,
    .gyro_lpf = GYRO_LPF_42HZ,      // 42HZ value is defined for Invensense/TDK gyros
    .gyro_soft_lpf_hz = 60,
    .gyro_align = ALIGN_DEFAULT,
    .gyroMovementCalibrationThreshold = 32,
    .looptime = 1000,
    .gyroSync = 0,
    .gyro_to_use = 0,
    .gyro_soft_notch_hz_1 = 0,
    .gyro_soft_notch_cutoff_1 = 1,
    .gyro_soft_notch_hz_2 = 0,
    .gyro_soft_notch_cutoff_2 = 1
);

STATIC_UNIT_TESTED gyroSensor_e gyroDetect(gyroDev_t *dev, gyroSensor_e gyroHardware)
{
    dev->gyroAlign = ALIGN_DEFAULT;

    switch (gyroHardware) {
    case GYRO_AUTODETECT:
        FALLTHROUGH;

#ifdef USE_GYRO_MPU6050
    case GYRO_MPU6050:
        if (mpu6050GyroDetect(dev)) {
            gyroHardware = GYRO_MPU6050;
#ifdef GYRO_MPU6050_ALIGN
            dev->gyroAlign = GYRO_MPU6050_ALIGN;
#endif
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_GYRO_L3G4200D
    case GYRO_L3G4200D:
        if (l3g4200dDetect(dev)) {
            gyroHardware = GYRO_L3G4200D;
#ifdef GYRO_L3G4200D_ALIGN
            dev->gyroAlign = GYRO_L3G4200D_ALIGN;
#endif
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_GYRO_MPU3050
    case GYRO_MPU3050:
        if (mpu3050Detect(dev)) {
            gyroHardware = GYRO_MPU3050;
#ifdef GYRO_MPU3050_ALIGN
            dev->gyroAlign = GYRO_MPU3050_ALIGN;
#endif
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_GYRO_L3GD20
    case GYRO_L3GD20:
        if (l3gd20Detect(dev)) {
            gyroHardware = GYRO_L3GD20;
#ifdef GYRO_L3GD20_ALIGN
            dev->gyroAlign = GYRO_L3GD20_ALIGN;
#endif
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_GYRO_MPU6000
    case GYRO_MPU6000:
        if (mpu6000GyroDetect(dev)) {
            gyroHardware = GYRO_MPU6000;
#ifdef GYRO_MPU6000_ALIGN
            dev->gyroAlign = GYRO_MPU6000_ALIGN;
#endif
            break;
        }
        FALLTHROUGH;
#endif

#if defined(USE_GYRO_MPU6500)
    case GYRO_MPU6500:
        if (mpu6500GyroDetect(dev)) {
            gyroHardware = GYRO_MPU6500;
#ifdef GYRO_MPU6500_ALIGN
            dev->gyroAlign = GYRO_MPU6500_ALIGN;
#endif
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_GYRO_MPU9250
    case GYRO_MPU9250:
        if (mpu9250GyroDetect(dev)) {
            gyroHardware = GYRO_MPU9250;
#ifdef GYRO_MPU9250_ALIGN
            dev->gyroAlign = GYRO_MPU9250_ALIGN;
#endif
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_FAKE_GYRO
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

    addBootlogEvent6(BOOT_EVENT_GYRO_DETECTION, BOOT_EVENT_FLAGS_NONE, gyroHardware, 0, 0, 0);

    if (gyroHardware != GYRO_NONE) {
        detectedSensors[SENSOR_INDEX_GYRO] = gyroHardware;
        sensorsSet(SENSOR_GYRO);
    }
    return gyroHardware;
}

bool gyroInit(void)
{
    memset(&gyro, 0, sizeof(gyro));

    // Set inertial sensor tag (for dual-gyro selection)
#ifdef USE_DUAL_GYRO
    gyroDev0.imuSensorToUse = gyroConfig()->gyro_to_use;
#else
    gyroDev0.imuSensorToUse = 0;
#endif

    if (gyroDetect(&gyroDev0, GYRO_AUTODETECT) == GYRO_NONE) {
        return false;
    }

    // Driver initialisation
    gyroDev0.lpf = gyroConfig()->gyro_lpf;
    gyroDev0.requestedSampleIntervalUs = gyroConfig()->looptime;
    gyroDev0.sampleRateIntervalUs = gyroConfig()->looptime;
    gyroDev0.initFn(&gyroDev0);

    // initFn will initialize sampleRateIntervalUs to actual gyro sampling rate (if driver supports it). Calculate target looptime using that value
    gyro.targetLooptime = gyroConfig()->gyroSync ? gyroDev0.sampleRateIntervalUs : gyroConfig()->looptime;

    if (gyroConfig()->gyro_align != ALIGN_DEFAULT) {
        gyroDev0.gyroAlign = gyroConfig()->gyro_align;
    }

    gyroInitFilters();
    return true;
}

void gyroInitFilters(void)
{
    STATIC_FASTRAM biquadFilter_t gyroFilterLPF[XYZ_AXIS_COUNT];
    softLpfFilterApplyFn = nullFilterApply;
#ifdef USE_GYRO_NOTCH_1
    STATIC_FASTRAM biquadFilter_t gyroFilterNotch_1[XYZ_AXIS_COUNT];
    notchFilter1ApplyFn = nullFilterApply;
#endif
#ifdef USE_GYRO_NOTCH_2
    STATIC_FASTRAM biquadFilter_t gyroFilterNotch_2[XYZ_AXIS_COUNT];
    notchFilter2ApplyFn = nullFilterApply;
#endif

    if (gyroConfig()->gyro_soft_lpf_hz) {
        softLpfFilterApplyFn = (filterApplyFnPtr)biquadFilterApply;
        for (int axis = 0; axis < 3; axis++) {
            softLpfFilter[axis] = &gyroFilterLPF[axis];
            biquadFilterInitLPF(softLpfFilter[axis], gyroConfig()->gyro_soft_lpf_hz, getGyroUpdateRate());
        }
    }

#ifdef USE_GYRO_NOTCH_1
    if (gyroConfig()->gyro_soft_notch_hz_1) {
        notchFilter1ApplyFn = (filterApplyFnPtr)biquadFilterApply;
        for (int axis = 0; axis < 3; axis++) {
            notchFilter1[axis] = &gyroFilterNotch_1[axis];
            biquadFilterInitNotch(notchFilter1[axis], getGyroUpdateRate(), gyroConfig()->gyro_soft_notch_hz_1, gyroConfig()->gyro_soft_notch_cutoff_1);
        }
    }
#endif

#ifdef USE_GYRO_NOTCH_2
    if (gyroConfig()->gyro_soft_notch_hz_2) {
        notchFilter2ApplyFn = (filterApplyFnPtr)biquadFilterApply;
        for (int axis = 0; axis < 3; axis++) {
            notchFilter2[axis] = &gyroFilterNotch_2[axis];
            biquadFilterInitNotch(notchFilter2[axis], getGyroUpdateRate(), gyroConfig()->gyro_soft_notch_hz_2, gyroConfig()->gyro_soft_notch_cutoff_2);
        }
    }
#endif
}

void gyroSetCalibrationCycles(uint16_t calibrationCyclesRequired)
{
    gyroCalibration.calibratingG = calibrationCyclesRequired;
}

STATIC_UNIT_TESTED bool isCalibrationComplete(gyroCalibration_t *gyroCalibration)
{
    return gyroCalibration->calibratingG == 0;
}

bool gyroIsCalibrationComplete(void)
{
    return gyroCalibration.calibratingG == 0;
}

static bool isOnFinalGyroCalibrationCycle(gyroCalibration_t *gyroCalibration)
{
    return gyroCalibration->calibratingG == 1;
}

static bool isOnFirstGyroCalibrationCycle(gyroCalibration_t *gyroCalibration)
{
    return gyroCalibration->calibratingG == CALIBRATING_GYRO_CYCLES;
}

STATIC_UNIT_TESTED void performGyroCalibration(gyroDev_t *dev, gyroCalibration_t *gyroCalibration, uint8_t gyroMovementCalibrationThreshold)
{
    for (int axis = 0; axis < 3; axis++) {
        // Reset g[axis] at start of calibration
        if (isOnFirstGyroCalibrationCycle(gyroCalibration)) {
            gyroCalibration->g[axis] = 0;
            devClear(&gyroCalibration->var[axis]);
        }

        // Sum up CALIBRATING_GYRO_CYCLES readings
        gyroCalibration->g[axis] += dev->gyroADCRaw[axis];
        devPush(&gyroCalibration->var[axis], dev->gyroADCRaw[axis]);

        // gyroZero is set to zero until calibration complete
        dev->gyroZero[axis] = 0;

        if (isOnFinalGyroCalibrationCycle(gyroCalibration)) {
            const float stddev = devStandardDeviation(&gyroCalibration->var[axis]);
            // check deviation and start over if the model was moved
            if (gyroMovementCalibrationThreshold && stddev > gyroMovementCalibrationThreshold) {
                gyroSetCalibrationCycles(CALIBRATING_GYRO_CYCLES);
                return;
            }
            // calibration complete, so set the gyro zero values
            dev->gyroZero[axis] = (gyroCalibration->g[axis] + (CALIBRATING_GYRO_CYCLES / 2)) / CALIBRATING_GYRO_CYCLES;
        }
    }

    if (isOnFinalGyroCalibrationCycle(gyroCalibration)) {
        DEBUG_TRACE_SYNC("Gyro calibration complete (%d, %d, %d)", dev->gyroZero[0], dev->gyroZero[1], dev->gyroZero[2]);
        schedulerResetTaskStatistics(TASK_SELF); // so calibration cycles do not pollute tasks statistics
    }
    gyroCalibration->calibratingG--;
}

#ifdef USE_ASYNC_GYRO_PROCESSING
STATIC_FASTRAM float accumulatedRates[XYZ_AXIS_COUNT];
STATIC_FASTRAM timeUs_t accumulatedRateTimeUs;

static void gyroUpdateAccumulatedRates(timeDelta_t gyroUpdateDeltaUs)
{
    accumulatedRateTimeUs += gyroUpdateDeltaUs;
    const float gyroUpdateDelta = gyroUpdateDeltaUs * 1e-6f;
    for (int axis = 0; axis < 3; axis++) {
        accumulatedRates[axis] += gyro.gyroADCf[axis] * gyroUpdateDelta;
    }
}
#endif

/*
 * Calculate rotation rate in rad/s in body frame
 */
void gyroGetMeasuredRotationRate(fpVector3_t *measuredRotationRate)
{
#ifdef USE_ASYNC_GYRO_PROCESSING
    const float accumulatedRateTime = accumulatedRateTimeUs * 1e-6;
    accumulatedRateTimeUs = 0;
    for (int axis = 0; axis < 3; axis++) {
        measuredRotationRate->v[axis] = DEGREES_TO_RADIANS(accumulatedRates[axis] / accumulatedRateTime);
        accumulatedRates[axis] = 0.0f;
    }
#else
    for (int axis = 0; axis < 3; axis++) {
        measuredRotationRate->v[axis] = DEGREES_TO_RADIANS(gyro.gyroADCf[axis]);
    }
#endif
}

void gyroUpdate(timeDelta_t gyroUpdateDeltaUs)
{
    // range: +/- 8192; +/- 2000 deg/sec
    if (gyroDev0.readFn(&gyroDev0)) {
        if (isCalibrationComplete(&gyroCalibration)) {
            // Copy gyro value into int32_t (to prevent overflow) and then apply calibration and alignment
            gyroADC[X] = (int32_t)gyroDev0.gyroADCRaw[X] - (int32_t)gyroDev0.gyroZero[X];
            gyroADC[Y] = (int32_t)gyroDev0.gyroADCRaw[Y] - (int32_t)gyroDev0.gyroZero[Y];
            gyroADC[Z] = (int32_t)gyroDev0.gyroADCRaw[Z] - (int32_t)gyroDev0.gyroZero[Z];
            applySensorAlignment(gyroADC, gyroADC, gyroDev0.gyroAlign);
            applyBoardAlignment(gyroADC);
        } else {
            performGyroCalibration(&gyroDev0, &gyroCalibration, gyroConfig()->gyroMovementCalibrationThreshold);
            // Reset gyro values to zero to prevent other code from using uncalibrated data
            gyro.gyroADCf[X] = 0.0f;
            gyro.gyroADCf[Y] = 0.0f;
            gyro.gyroADCf[Z] = 0.0f;
            // still calibrating, so no need to further process gyro data
            return;
        }
    } else {
        // no gyro reading to process
        return;
    }

    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        float gyroADCf = (float)gyroADC[axis] * gyroDev0.scale;

        DEBUG_SET(DEBUG_GYRO, axis, lrintf(gyroADCf));

        gyroADCf = softLpfFilterApplyFn(softLpfFilter[axis], gyroADCf);

        DEBUG_SET(DEBUG_NOTCH, axis, lrintf(gyroADCf));

#ifdef USE_GYRO_NOTCH_1
        gyroADCf = notchFilter1ApplyFn(notchFilter1[axis], gyroADCf);
#endif
#ifdef USE_GYRO_NOTCH_2
        gyroADCf = notchFilter2ApplyFn(notchFilter2[axis], gyroADCf);
#endif
        gyro.gyroADCf[axis] = gyroADCf;
    }

#ifdef USE_ASYNC_GYRO_PROCESSING
    // Accumulate gyro readings for better IMU accuracy
    gyroUpdateAccumulatedRates(gyroUpdateDeltaUs);
#endif
}

bool gyroReadTemperature(void)
{
    // Read gyro sensor temperature. temperatureFn returns temperature in [degC * 10]
    if (gyroDev0.temperatureFn) {
        return gyroDev0.temperatureFn(&gyroDev0, &gyroTemperature0);
    }

    return false;
}

int16_t gyroGetTemperature(void)
{
    return gyroTemperature0;
}

int16_t gyroRateDps(int axis)
{
    return lrintf(gyro.gyroADCf[axis] / gyroDev0.scale);
}

bool gyroSyncCheckUpdate(void)
{
    if (!gyroDev0.intStatusFn)
        return false;

    return gyroDev0.intStatusFn(&gyroDev0);
}
