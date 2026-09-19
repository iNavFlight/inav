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
#include "drivers/accgyro/accgyro_mpu6000.h"
#include "drivers/accgyro/accgyro_mpu6500.h"
#include "drivers/accgyro/accgyro_mpu9250.h"

#include "drivers/accgyro/accgyro_bmi088.h"
#include "drivers/accgyro/accgyro_bmi160.h"
#include "drivers/accgyro/accgyro_bmi270.h"
#include "drivers/accgyro/accgyro_icm20689.h"
#include "drivers/accgyro/accgyro_icm42605.h"
#include "drivers/accgyro/accgyro_icm45686.h"
#include "drivers/accgyro/accgyro_icm40609d.h"
#include "drivers/accgyro/accgyro_lsm6dxx.h"
#include "drivers/accgyro/accgyro_fake.h"
#include "drivers/io.h"

#include "fc/config.h"
#include "fc/runtime_config.h"
#include "fc/rc_controls.h"
#include "fc/settings.h"

#include "io/beeper.h"
#include "io/statusindicator.h"

#include "scheduler/scheduler.h"

#include "sensors/boardalignment.h"
#include "sensors/gyro.h"
#include "sensors/sensors.h"

#include "flight/gyroanalyse.h"
#include "flight/rpm_filter.h"
#include "flight/kalman.h"
#include "flight/adaptive_filter.h"

#ifdef USE_HARDWARE_REVISION_DETECTION
#include "hardware_revision.h"
#endif

FASTRAM gyro_t gyro; // gyro sensor object

#ifdef USE_DUAL_GYRO
#define MAX_GYRO_COUNT 2
// Highest IMU tag any in-tree target uses: most are 0 and 1, AETH743Basic is 0 and 2
#define MAX_GYRO_SENSOR_TAG 2
#else
#define MAX_GYRO_COUNT 1
#endif

// gyroDev[0] flies the aircraft, whichever IMU gyro_to_use picked. On a dual-IMU board
// with gyro_secondary_enabled, gyroDev[1] is the other one, read only for the log
#define GYRO_PRIMARY    0
#define GYRO_SECONDARY  1

STATIC_UNIT_TESTED gyroDev_t gyroDev[MAX_GYRO_COUNT];  // Not in FASTRAM since it may hold DMA buffers
STATIC_FASTRAM int16_t gyroTemperature[MAX_GYRO_COUNT];
STATIC_FASTRAM_UNIT_TESTED zeroCalibrationVector_t gyroCalibration[MAX_GYRO_COUNT];

STATIC_FASTRAM filterApplyFnPtr gyroLpfApplyFn;
STATIC_FASTRAM filter_t gyroLpfState[XYZ_AXIS_COUNT];

STATIC_FASTRAM filterApplyFnPtr gyroLpf2ApplyFn;
STATIC_FASTRAM filter_t gyroLpf2State[XYZ_AXIS_COUNT];

// Cached calibration status to eliminate function call in hot path, one entry per sensor
STATIC_FASTRAM bool gyroCalibrationComplete[MAX_GYRO_COUNT];

#ifdef USE_DUAL_GYRO
// Set by the blackbox while a log is being written, the only time the secondary is wanted
STATIC_FASTRAM bool gyroSecondaryLogging;
#endif

STATIC_FASTRAM filterApplyFnPtr gyroLuluApplyFn;
STATIC_FASTRAM filter_t gyroLuluState[XYZ_AXIS_COUNT];

#ifdef USE_DYNAMIC_FILTERS

EXTENDED_FASTRAM gyroAnalyseState_t gyroAnalyseState;
EXTENDED_FASTRAM dynamicGyroNotchState_t dynamicGyroNotchState;
EXTENDED_FASTRAM secondaryDynamicGyroNotchState_t secondaryDynamicGyroNotchState;

#endif

PG_REGISTER_WITH_RESET_TEMPLATE(gyroConfig_t, gyroConfig, PG_GYRO_CONFIG, 12);

PG_RESET_TEMPLATE(gyroConfig_t, gyroConfig,
    .gyro_anti_aliasing_lpf_hz = SETTING_GYRO_ANTI_ALIASING_LPF_HZ_DEFAULT,
    .looptime = SETTING_LOOPTIME_DEFAULT,
#ifdef USE_DUAL_GYRO
    .gyro_to_use = SETTING_GYRO_TO_USE_DEFAULT,
    .gyro_secondary_enabled = SETTING_GYRO_SECONDARY_ENABLED_DEFAULT,
#endif
    .gyro_main_lpf_hz = SETTING_GYRO_MAIN_LPF_HZ_DEFAULT,
    .gyroDynamicLpfMinHz = SETTING_GYRO_DYN_LPF_MIN_HZ_DEFAULT,
    .gyroDynamicLpfMaxHz = SETTING_GYRO_DYN_LPF_MAX_HZ_DEFAULT,
    .gyroDynamicLpfCurveExpo = SETTING_GYRO_DYN_LPF_CURVE_EXPO_DEFAULT,
#ifdef USE_DYNAMIC_FILTERS
    .dynamicGyroNotchQ = SETTING_DYNAMIC_GYRO_NOTCH_Q_DEFAULT,
    .dynamicGyroNotchMinHz = SETTING_DYNAMIC_GYRO_NOTCH_MIN_HZ_DEFAULT,
    .dynamicGyroNotchEnabled = SETTING_DYNAMIC_GYRO_NOTCH_ENABLED_DEFAULT,
    .dynamicGyroNotchMode = SETTING_DYNAMIC_GYRO_NOTCH_MODE_DEFAULT,
    .dynamicGyroNotch3dQ = SETTING_DYNAMIC_GYRO_NOTCH_3D_Q_DEFAULT,
#endif
#ifdef USE_GYRO_KALMAN
    .kalman_q = SETTING_SETPOINT_KALMAN_Q_DEFAULT,
    .kalmanEnabled = SETTING_SETPOINT_KALMAN_ENABLED_DEFAULT,
#endif
    .init_gyro_cal_enabled = SETTING_INIT_GYRO_CAL_DEFAULT,
    .gyro_zero_cal = {SETTING_GYRO_ZERO_X_DEFAULT, SETTING_GYRO_ZERO_Y_DEFAULT, SETTING_GYRO_ZERO_Z_DEFAULT},
    .gravity_cmss_cal = SETTING_INS_GRAVITY_CMSS_DEFAULT,
#ifdef USE_ADAPTIVE_FILTER
    .adaptiveFilterTarget = SETTING_GYRO_ADAPTIVE_FILTER_TARGET_DEFAULT,
    .adaptiveFilterMinHz = SETTING_GYRO_ADAPTIVE_FILTER_MIN_HZ_DEFAULT,
    .adaptiveFilterMaxHz = SETTING_GYRO_ADAPTIVE_FILTER_MAX_HZ_DEFAULT,
    .adaptiveFilterStdLpfHz = SETTING_GYRO_ADAPTIVE_FILTER_STD_LPF_HZ_DEFAULT,
    .adaptiveFilterHpfHz = SETTING_GYRO_ADAPTIVE_FILTER_HPF_HZ_DEFAULT,
    .adaptiveFilterIntegratorThresholdHigh = SETTING_GYRO_ADAPTIVE_FILTER_INTEGRATOR_THRESHOLD_HIGH_DEFAULT,
    .adaptiveFilterIntegratorThresholdLow  = SETTING_GYRO_ADAPTIVE_FILTER_INTEGRATOR_THRESHOLD_LOW_DEFAULT,
#endif
    .gyroFilterMode = SETTING_GYRO_FILTER_MODE_DEFAULT,
    .gyroLuluSampleCount = SETTING_GYRO_LULU_SAMPLE_COUNT_DEFAULT,
    .gyroLuluEnabled = SETTING_GYRO_LULU_ENABLED_DEFAULT
);

STATIC_UNIT_TESTED gyroSensor_e gyroDetect(gyroDev_t *dev, gyroSensor_e gyroHardware)
{
    dev->gyroAlign = ALIGN_DEFAULT;

    switch (gyroHardware) {
    case GYRO_AUTODETECT:
        FALLTHROUGH;

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

#ifdef USE_IMU_BMI088
    case GYRO_BMI088:
        if (bmi088GyroDetect(dev)) {
            gyroHardware = GYRO_BMI088;
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

#ifdef USE_IMU_ICM42605
    case GYRO_ICM42605:
        if (icm42605GyroDetect(dev)) {
            gyroHardware = GYRO_ICM42605;
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_IMU_BMI270
    case GYRO_BMI270:
        if (bmi270GyroDetect(dev)) {
            gyroHardware = GYRO_BMI270;
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_IMU_LSM6DXX
    case GYRO_LSM6DXX:
        if (lsm6dGyroDetect(dev)) {
            gyroHardware = GYRO_LSM6DXX;
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_IMU_ICM45686
    case GYRO_ICM45686:
        if (icm45686GyroDetect(dev)) {
            gyroHardware = GYRO_ICM45686;
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_IMU_ICM40609D
    case GYRO_ICM40609D:
        if (icm40609dGyroDetect(dev)) {
            gyroHardware = GYRO_ICM40609D;
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

static void initGyroFilter(filterApplyFnPtr *applyFn, filter_t state[], uint16_t cutoff, uint32_t looptime)
{
    *applyFn = nullFilterApply;
    if (cutoff > 0) {
        *applyFn = (filterApplyFnPtr)pt1FilterApply;
        for (int axis = 0; axis < 3; axis++) {
            pt1FilterInit(&state[axis].pt1, cutoff, US2S(looptime));
        }
    }
}

static void gyroInitFilters(void)
{
    //First gyro LPF running at full gyro frequency 8kHz
    initGyroFilter(&gyroLpfApplyFn, gyroLpfState, gyroConfig()->gyro_anti_aliasing_lpf_hz, getGyroLooptime());

    if (gyroConfig()->gyroLuluEnabled && gyroConfig()->gyroLuluSampleCount > 0) {
        gyroLuluApplyFn = (filterApplyFnPtr)luluFilterApply;

        for (int axis = 0; axis < 3; axis++) {
            luluFilterInit(&gyroLuluState[axis].lulu, gyroConfig()->gyroLuluSampleCount);
        }
    } else {
        gyroLuluApplyFn = nullFilterApply;
    }

    if (gyroConfig()->gyroFilterMode != GYRO_FILTER_MODE_OFF) {
        initGyroFilter(&gyroLpf2ApplyFn, gyroLpf2State, gyroConfig()->gyro_main_lpf_hz, getLooptime());
    } else {
        gyroLpf2ApplyFn = nullFilterApply;
    }

#ifdef USE_ADAPTIVE_FILTER
    if (gyroConfig()->gyroFilterMode == GYRO_FILTER_MODE_ADAPTIVE) {
        adaptiveFilterSetDefaultFrequency(gyroConfig()->gyro_main_lpf_hz, gyroConfig()->adaptiveFilterMinHz, gyroConfig()->adaptiveFilterMaxHz);
    }
#endif

#ifdef USE_GYRO_KALMAN
    if (gyroConfig()->kalmanEnabled) {
        gyroKalmanInitialize(gyroConfig()->kalman_q);
    }
#endif
}

// Both gyros start the same way, so on a dual-IMU board their two channels in a log
// compare like for like
static void gyroDevStart(gyroDev_t *dev)
{
    dev->lpf = GYRO_LPF_256HZ;
    dev->requestedSampleIntervalUs = TASK_GYRO_LOOPTIME;
    dev->sampleRateIntervalUs = TASK_GYRO_LOOPTIME;
    dev->initFn(dev);
}

bool gyroInit(void)
{
    memset(&gyro, 0, sizeof(gyro));
    for (int i = 0; i < MAX_GYRO_COUNT; i++) {
        gyroCalibrationComplete[i] = false;
    }

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
    gyroDevStart(&gyroDev[0]);

    // initFn will initialize sampleRateIntervalUs to actual gyro sampling rate (if driver supports it). Calculate target looptime using that value
    gyro.targetLooptime = gyroDev[0].sampleRateIntervalUs;

    gyroInitFilters();

#ifdef USE_DUAL_GYRO
    // Optional secondary IMU, logged as gyroRaw2 and read by nothing else
    if (gyroConfig()->gyro_secondary_enabled) {
        // Targets disagree on how they tag the second IMU, so probe rather than assume tag 1
        for (uint8_t tag = 0; tag <= MAX_GYRO_SENSOR_TAG; tag++) {
            if (tag == gyroConfig()->gyro_to_use) {
                continue;
            }

            gyroDev[GYRO_SECONDARY].imuSensorToUse = tag;
            if (gyroDetect(&gyroDev[GYRO_SECONDARY], GYRO_AUTODETECT) != GYRO_NONE) {
                gyroDevStart(&gyroDev[GYRO_SECONDARY]);
                gyro.secondaryInitialized = true;
                break;
            }
        }
    }
#endif

#ifdef USE_DYNAMIC_FILTERS
    // Dynamic notch running at PID frequency
    dynamicGyroNotchFiltersInit(&dynamicGyroNotchState);

    secondaryDynamicGyroNotchFiltersInit(&secondaryDynamicGyroNotchState);

    gyroDataAnalyseStateInit(
        &gyroAnalyseState,
        gyroConfig()->dynamicGyroNotchMinHz,
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

#ifdef USE_DUAL_GYRO
    // The secondary measures its own zero and ignores init_gyro_cal: gyro_zero_cal is a
    // single stored value, and it belongs to the gyro that flies
    if (gyro.secondaryInitialized) {
        // The threshold is in raw counts, so convert it into the secondary's own scale
        const float secondaryThreshold = CALIBRATING_GYRO_MORON_THRESHOLD *
            gyroDev[GYRO_PRIMARY].scale / gyroDev[GYRO_SECONDARY].scale;

        // Asked to succeed like the primary: a window ending on a moving aircraft restarts
        gyroCalibrationComplete[GYRO_SECONDARY] = false;
        zeroCalibrationStartV(&gyroCalibration[GYRO_SECONDARY], CALIBRATING_GYRO_TIME_MS, secondaryThreshold, false);
    }
#endif

#ifndef USE_IMU_FAKE // fixes Test Unit compilation error
    if (!gyroConfig()->init_gyro_cal_enabled) {
        return;
    }
#endif

    gyroCalibrationComplete[GYRO_PRIMARY] = false;
    zeroCalibrationStartV(&gyroCalibration[GYRO_PRIMARY], CALIBRATING_GYRO_TIME_MS, CALIBRATING_GYRO_MORON_THRESHOLD, false);
}

#ifdef USE_DUAL_GYRO
void gyroSetSecondaryLogging(bool logging)
{
    gyroSecondaryLogging = logging;
}
#endif

bool gyroIsCalibrationComplete(void)
{
    if (!gyro.initialized) {
        return true;
    }

#ifndef USE_IMU_FAKE // fixes Test Unit compilation error
    if (!gyroConfig()->init_gyro_cal_enabled) {
        return true;
    }
#endif

    return zeroCalibrationIsCompleteV(&gyroCalibration[0]) && zeroCalibrationIsSuccessfulV(&gyroCalibration[0]);
}

STATIC_UNIT_TESTED void performGyroCalibration(uint8_t index)
{
    gyroDev_t *dev = &gyroDev[index];
    zeroCalibrationVector_t *cal = &gyroCalibration[index];
    fpVector3_t v;

    // Consume gyro reading
    v.v[X] = dev->gyroADCRaw[X];
    v.v[Y] = dev->gyroADCRaw[Y];
    v.v[Z] = dev->gyroADCRaw[Z];

    zeroCalibrationAddValueV(cal, &v);

    // Check if calibration is complete after this cycle
    if (zeroCalibrationIsCompleteV(cal)) {
        zeroCalibrationGetZeroV(cal, &v);
        dev->gyroZero[X] = v.v[X];
        dev->gyroZero[Y] = v.v[Y];
        dev->gyroZero[Z] = v.v[Z];

#ifndef USE_IMU_FAKE // fixes Test Unit compilation error
        // gyro_zero_cal is a single shared value: only the gyro that flies may write it
        if (index == GYRO_PRIMARY) {
            setGyroCalibration(dev->gyroZero);
        }
#endif

        // Cache completion status to avoid function call in hot path
        gyroCalibrationComplete[index] = true;

        LOG_DEBUG(GYRO, "Gyro calibration complete (%d, %d, %d)", (int16_t) dev->gyroZero[X], (int16_t) dev->gyroZero[Y], (int16_t) dev->gyroZero[Z]);
        schedulerResetTaskStatistics(TASK_SELF); // so calibration cycles do not pollute tasks statistics
    } else {
        dev->gyroZero[X] = 0;
        dev->gyroZero[Y] = 0;
        dev->gyroZero[Z] = 0;
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

static bool FAST_CODE NOINLINE gyroUpdateAndCalibrate(uint8_t index, float * gyroADCf)
{
    gyroDev_t *dev = &gyroDev[index];

    // range: +/- 8192; +/- 2000 deg/sec
    if (dev->readFn(dev)) {

#ifndef USE_IMU_FAKE // fixes Test Unit compilation error
    if (index == GYRO_PRIMARY && !gyroConfig()->init_gyro_cal_enabled) {
        // marks that the gyro calibration has ended
        gyroCalibration[index].params.state = ZERO_CALIBRATION_DONE;
        gyroCalibrationComplete[index] = true;
        // pass the calibration values
        dev->gyroZero[X] = gyroConfig()->gyro_zero_cal[X];
        dev->gyroZero[Y] = gyroConfig()->gyro_zero_cal[Y];
        dev->gyroZero[Z] = gyroConfig()->gyro_zero_cal[Z];
    }
#endif

        // Use cached status to avoid function call in hot path
        if (gyroCalibrationComplete[index]) {
            float gyroADCtmp[XYZ_AXIS_COUNT];

            //Apply zero calibration with CMSIS DSP
            arm_sub_f32(dev->gyroADCRaw, dev->gyroZero, gyroADCtmp, 3);

            // Apply sensor alignment
            applySensorAlignment(gyroADCtmp, gyroADCtmp, dev->gyroAlign);
            applyBoardAlignment(gyroADCtmp);

            // Convert to deg/s and store in unified data
            arm_scale_f32(gyroADCtmp, dev->scale, gyroADCf, 3);

            return true;
        } else {
            performGyroCalibration(index);

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

void FAST_CODE NOINLINE gyroFilter(void)
{
    if (!gyro.initialized) {
        return;
    }

    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        float gyroADCf = gyro.gyroADCf[axis];

#ifdef USE_RPM_FILTER
        gyroADCf = rpmFilterGyroApply(axis, gyroADCf);
#endif

        // LULU gyro filter
        DEBUG_SET(DEBUG_LULU, axis, gyroADCf); //Pre LULU debug
        float preLulu = gyroADCf;
        gyroADCf = gyroLuluApplyFn((filter_t *) &gyroLuluState[axis], gyroADCf);
        DEBUG_SET(DEBUG_LULU, axis + 3, gyroADCf); //Post LULU debug

        if (axis == ROLL) {
            DEBUG_SET(DEBUG_LULU, 6, gyroADCf - preLulu); //LULU delta debug
        }

        // Gyro Main LPF
        gyroADCf = gyroLpf2ApplyFn((filter_t *) &gyroLpf2State[axis], gyroADCf);

#ifdef USE_ADAPTIVE_FILTER
        adaptiveFilterPush(axis, gyroADCf);
#endif

#ifdef USE_DYNAMIC_FILTERS
        if (dynamicGyroNotchState.enabled) {
            gyroDataAnalysePush(&gyroAnalyseState, axis, gyroADCf);
            gyroADCf = dynamicGyroNotchFiltersApply(&dynamicGyroNotchState, axis, gyroADCf);
        }

        /**
         * Secondary dynamic notch filter.
         * In some cases, noise amplitude is high enough not to be filtered by the primary filter.
         * This happens on the first frequency with the biggest aplitude
         */
        gyroADCf = secondaryDynamicGyroNotchFiltersApply(&secondaryDynamicGyroNotchState, axis, gyroADCf);

#endif

#ifdef USE_GYRO_KALMAN
        if (gyroConfig()->kalmanEnabled) {
            gyroADCf = gyroKalmanUpdate(axis, gyroADCf);
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
                gyroAnalyseState.centerFrequency[gyroAnalyseState.filterUpdateAxis]
            );

            secondaryDynamicGyroNotchFiltersUpdate(
                &secondaryDynamicGyroNotchState,
                gyroAnalyseState.filterUpdateAxis,
                gyroAnalyseState.centerFrequency[gyroAnalyseState.filterUpdateAxis]
            );

        }
    }
#endif

}

void FAST_CODE NOINLINE gyroUpdate(void)
{
#ifdef USE_SIMULATOR
    if (ARMING_FLAG(SIMULATOR_MODE_HITL)) {
        //output: gyro.gyroADCf[axis]
        //unused: dev->gyroADCRaw[], dev->gyroZero[];
        return;
    }
#endif
    if (!gyro.initialized) {
        return;
    }

#ifdef USE_DUAL_GYRO
    // The secondary only feeds the log, so it is read while it measures its zero and while
    // a log is being written. It goes first: the primary's path returns early on a failed read
    if (gyro.secondaryInitialized && (gyroSecondaryLogging || !gyroCalibrationComplete[GYRO_SECONDARY])) {
        if (!gyroUpdateAndCalibrate(GYRO_SECONDARY, gyro.gyroRaw2)) {
            gyro.gyroRaw2[X] = 0.0f;
            gyro.gyroRaw2[Y] = 0.0f;
            gyro.gyroRaw2[Z] = 0.0f;
        }
    }
#endif

    if (!gyroUpdateAndCalibrate(GYRO_PRIMARY, gyro.gyroADCf)) {
        return;
    }

    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        // At this point gyro.gyroADCf contains unfiltered gyro value [deg/s]
        float gyroADCf = gyro.gyroADCf[axis];

        // Set raw gyro for blackbox purposes
        gyro.gyroRaw[axis] = gyroADCf;

        /*
         * First gyro LPF is the only filter applied with the full gyro sampling speed
         */
        gyroADCf = gyroLpfApplyFn((filter_t *) &gyroLpfState[axis], gyroADCf);

        gyro.gyroADCf[axis] = gyroADCf;
    }
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

void gyroUpdateDynamicLpf(float cutoffFreq) {
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        pt1FilterSetCutoff(&gyroLpf2State[axis].pt1, cutoffFreq);
    }
}

float averageAbsGyroRates(void)
{
    return (fabsf(gyro.gyroADCf[ROLL]) + fabsf(gyro.gyroADCf[PITCH]) + fabsf(gyro.gyroADCf[YAW])) / 3.0f;
}
