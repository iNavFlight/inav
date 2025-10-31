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

#include "build/debug.h"

#include "common/axis.h"
#include "common/filter.h"
#include "common/maths.h"
#include "common/utils.h"
#include "common/calibration.h"

#include "config/config_reset.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

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
#include "drivers/accgyro/accgyro_lsm6dxx.h"
#include "drivers/accgyro/accgyro_fake.h"
#include "drivers/sensor.h"

#include "fc/config.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "io/beeper.h"

#include "sensors/acceleration.h"
#include "sensors/battery.h"
#include "sensors/boardalignment.h"
#include "sensors/gyro.h"
#include "sensors/sensors.h"

#ifdef USE_HARDWARE_REVISION_DETECTION
#include "hardware_revision.h"
#endif


FASTRAM acc_t acc;                       // acc access functions

STATIC_FASTRAM zeroCalibrationVector_t zeroCalibration;

STATIC_FASTRAM float accADC[XYZ_AXIS_COUNT];

STATIC_FASTRAM filter_t accFilter[XYZ_AXIS_COUNT];
STATIC_FASTRAM filterApplyFnPtr accSoftLpfFilterApplyFn;
STATIC_FASTRAM void *accSoftLpfFilter[XYZ_AXIS_COUNT];

static EXTENDED_FASTRAM pt1Filter_t accVibeFloorFilter[XYZ_AXIS_COUNT];
static EXTENDED_FASTRAM pt1Filter_t accVibeFilter[XYZ_AXIS_COUNT];

static EXTENDED_FASTRAM filterApplyFnPtr accNotchFilterApplyFn;
static EXTENDED_FASTRAM void *accNotchFilter[XYZ_AXIS_COUNT];

static EXTENDED_FASTRAM float fAccZero[XYZ_AXIS_COUNT];
static EXTENDED_FASTRAM float fAccGain[XYZ_AXIS_COUNT];

PG_REGISTER_WITH_RESET_FN(accelerometerConfig_t, accelerometerConfig, PG_ACCELEROMETER_CONFIG, 6);

void pgResetFn_accelerometerConfig(accelerometerConfig_t *instance)
{
    RESET_CONFIG_2(accelerometerConfig_t, instance,
        .acc_hardware = SETTING_ACC_HARDWARE_DEFAULT,
        .acc_lpf_hz = SETTING_ACC_LPF_HZ_DEFAULT,
        .acc_notch_hz = SETTING_ACC_NOTCH_HZ_DEFAULT,
        .acc_notch_cutoff = SETTING_ACC_NOTCH_CUTOFF_DEFAULT,
        .acc_soft_lpf_type = SETTING_ACC_LPF_TYPE_DEFAULT,
        .acc_temp_correction = SETTING_ACC_TEMP_CORRECTION_DEFAULT
    );
    RESET_CONFIG_2(flightDynamicsTrims_t, &instance->accZero,
        .raw[X] = SETTING_ACCZERO_X_DEFAULT,
        .raw[Y] = SETTING_ACCZERO_Y_DEFAULT,
        .raw[Z] = SETTING_ACCZERO_Z_DEFAULT
    );
    RESET_CONFIG_2(flightDynamicsTrims_t, &instance->accGain,
         .raw[X] = SETTING_ACCGAIN_X_DEFAULT,
         .raw[Y] = SETTING_ACCGAIN_Y_DEFAULT,
         .raw[Z] = SETTING_ACCGAIN_Z_DEFAULT
    );
}

static void updateAccCoefficients(void) {

    for (uint8_t i = 0; i < XYZ_AXIS_COUNT; i++) {
        //Float zero
        fAccZero[i] = (float)accelerometerConfig()->accZero.raw[i];
        //Float gain
        fAccGain[i] = (float)accelerometerConfig()->accGain.raw[i] / 4096.0f;
    }

}

static bool accDetect(accDev_t *dev, accelerationSensor_e accHardwareToUse)
{
    accelerationSensor_e accHardware = ACC_NONE;

    dev->accAlign = ALIGN_DEFAULT;

    requestedSensors[SENSOR_INDEX_ACC] = accHardwareToUse;

    switch (accHardwareToUse) {
    case ACC_AUTODETECT:
        FALLTHROUGH;

#ifdef USE_IMU_MPU6000
    case ACC_MPU6000:
        if (mpu6000AccDetect(dev)) {
            accHardware = ACC_MPU6000;
            break;
        }
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (accHardwareToUse != ACC_AUTODETECT) {
            break;
        }
        FALLTHROUGH;
#endif

#if defined(USE_IMU_MPU6500)
    case ACC_MPU6500:
        if (mpu6500AccDetect(dev)) {
            accHardware = ACC_MPU6500;
            break;
        }
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (accHardwareToUse != ACC_AUTODETECT) {
            break;
        }
        FALLTHROUGH;
#endif

#if defined(USE_IMU_MPU9250)
    case ACC_MPU9250:
        if (mpu9250AccDetect(dev)) {
            accHardware = ACC_MPU9250;
            break;
        }
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (accHardwareToUse != ACC_AUTODETECT) {
            break;
        }
        FALLTHROUGH;
#endif

#if defined(USE_IMU_BMI160)
    case ACC_BMI160:
        if (bmi160AccDetect(dev)) {
            accHardware = ACC_BMI160;
            break;
        }
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (accHardwareToUse != ACC_AUTODETECT) {
            break;
        }
        FALLTHROUGH;
#endif

#if defined(USE_IMU_BMI088)
    case ACC_BMI088:
        if (bmi088AccDetect(dev)) {
            accHardware = ACC_BMI088;
            break;
        }
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (accHardwareToUse != ACC_AUTODETECT) {
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_IMU_ICM20689
    case ACC_ICM20689:
        if (icm20689AccDetect(dev)) {
            accHardware = ACC_ICM20689;
            break;
        }
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (accHardwareToUse != ACC_AUTODETECT) {
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_IMU_ICM42605
    case ACC_ICM42605:
        if (icm42605AccDetect(dev)) {
            accHardware = ACC_ICM42605;
            break;
        }
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (accHardwareToUse != ACC_AUTODETECT) {
            break;
        }
        FALLTHROUGH;
#endif

#ifdef USE_IMU_BMI270
    case ACC_BMI270:
        if (bmi270AccDetect(dev)) {
            accHardware = ACC_BMI270;
            break;
        }
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (accHardwareToUse != ACC_AUTODETECT) {
            break;
        }
        FALLTHROUGH;
#endif
#ifdef USE_IMU_LSM6DXX
    case ACC_LSM6DXX:
        if (lsm6dAccDetect(dev)) {
            accHardware = ACC_LSM6DXX;
            break;
        }
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (accHardwareToUse != ACC_AUTODETECT) {
            break;
        }
        FALLTHROUGH;
#endif
#ifdef USE_IMU_FAKE
    case ACC_FAKE:
        if (fakeAccDetect(dev)) {
            accHardware = ACC_FAKE;
            break;
        }
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (accHardwareToUse != ACC_AUTODETECT) {
            break;
        }
        FALLTHROUGH;
#endif

    default:
    case ACC_NONE: // disable ACC
        accHardware = ACC_NONE;
        break;
    }

    if (accHardware == ACC_NONE) {
        return false;
    }

    detectedSensors[SENSOR_INDEX_ACC] = accHardware;
    sensorsSet(SENSOR_ACC);
    return true;
}

bool accInit(uint32_t targetLooptime)
{
    memset(&acc, 0, sizeof(acc));

    // Set inertial sensor tag (for dual-gyro selection)
#ifdef USE_DUAL_GYRO
    acc.dev.imuSensorToUse = gyroConfig()->gyro_to_use;     // Use the same selection from gyroConfig()
#else
    acc.dev.imuSensorToUse = 0;
#endif

    if (!accDetect(&acc.dev, accelerometerConfig()->acc_hardware)) {
        return false;
    }

    acc.dev.acc_1G = 256; // set default
    acc.dev.initFn(&acc.dev);
    acc.accTargetLooptime = targetLooptime;
    acc.accClipCount = 0;
    accInitFilters();
    updateAccCoefficients();

    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        acc.extremes[axis].min = 100;
        acc.extremes[axis].max = -100;
    }

    return true;
}

static bool calibratedPosition[6];
static float accSamples[6][3];

uint8_t accGetCalibrationAxisFlags(void)
{
    if (STATE(ACCELEROMETER_CALIBRATED)) {
        return 0x3F;    // All 6 bits are set
    }

    static const uint8_t bitMap[6] = { 0, 1, 3, 5, 2, 4 };      // A mapping of bits to match position indexes in Configurator
    uint8_t flags = 0;
    for (int i = 0; i < 6; i++) {
        if (calibratedPosition[i]) {
            flags |= (1 << bitMap[i]);
        }
    }

    return flags;
}

static int getPrimaryAxisIndex(float accADCData[3])
{
    // Work on a copy so we don't mess with accADC data
    float sample[3];

    applySensorAlignment(sample, accADCData, acc.dev.accAlign);

    // Tolerate up to atan(1 / 1.5) = 33 deg tilt (in worst case 66 deg separation between points)
    if ((ABS(sample[Z]) / 1.5f) > ABS(sample[X]) && (ABS(sample[Z]) / 1.5f) > ABS(sample[Y])) {
        //Z-axis
        return (sample[Z] > 0) ? 0 : 1;
    }
    else if ((ABS(sample[X]) / 1.5f) > ABS(sample[Y]) && (ABS(sample[X]) / 1.5f) > ABS(sample[Z])) {
        //X-axis
        return (sample[X] > 0) ? 2 : 3;
    }
    else if ((ABS(sample[Y]) / 1.5f) > ABS(sample[X]) && (ABS(sample[Y]) / 1.5f) > ABS(sample[Z])) {
        //Y-axis
        return (sample[Y] > 0) ? 4 : 5;
    }
    else
        return -1;
}

void accStartCalibration(void)
{
    int positionIndex = getPrimaryAxisIndex(accADC);

    // Fail if we can't detect the side
    if (positionIndex < 0) {
        return;
    }

    // Fail if we have accelerometer fully calibrated and are NOT starting with TOP-UP position
    if (STATE(ACCELEROMETER_CALIBRATED) && positionIndex != 0) {
        return;
    }

    // Top+up and first calibration cycle, reset everything
    if (positionIndex == 0) {
        for (int axis = 0; axis < 6; axis++) {
            calibratedPosition[axis] = false;
            accSamples[axis][X] = 0;
            accSamples[axis][Y] = 0;
            accSamples[axis][Z] = 0;
        }

        DISABLE_STATE(ACCELEROMETER_CALIBRATED);
    }

    // Tolerate 5% variance in accelerometer readings
    zeroCalibrationStartV(&zeroCalibration, CALIBRATING_ACC_TIME_MS, acc.dev.acc_1G * 0.05f, true);
}

static bool allOrientationsHaveCalibrationDataCollected(void)
{
    // Return true only if we have calibration data for all 6 positions
    return calibratedPosition[0] && calibratedPosition[1] && calibratedPosition[2] &&
           calibratedPosition[3] && calibratedPosition[4] && calibratedPosition[5];
}

bool accIsCalibrationComplete(void)
{
    return zeroCalibrationIsCompleteV(&zeroCalibration);
}

static void performAcclerationCalibration(void)
{
    // Shortcut - no need to do any math if acceleromter is marked as calibrated
    if (STATE(ACCELEROMETER_CALIBRATED)) {
        return;
    }

    // If zero calibration logic is finished - no need to do anything
    if (accIsCalibrationComplete()) {
        return;
    }

    fpVector3_t v;
    int positionIndex = getPrimaryAxisIndex(accADC);

    // Check if sample is usable
    if (positionIndex < 0) {
        return;
    }

    if (!calibratedPosition[positionIndex]) {
        v.v[X] = accADC[X];
        v.v[Y] = accADC[Y];
        v.v[Z] = accADC[Z];

        zeroCalibrationAddValueV(&zeroCalibration, &v);

        if (zeroCalibrationIsCompleteV(&zeroCalibration)) {
            if (zeroCalibrationIsSuccessfulV(&zeroCalibration)) {
                zeroCalibrationGetZeroV(&zeroCalibration, &v);

                accSamples[positionIndex][X] = v.v[X];
                accSamples[positionIndex][Y] = v.v[Y];
                accSamples[positionIndex][Z] = v.v[Z];

                calibratedPosition[positionIndex] = true;
            }
            else {
                calibratedPosition[positionIndex] = false;
            }

            beeperConfirmationBeeps(2);
        }
    }

    if (allOrientationsHaveCalibrationDataCollected()) {
        sensorCalibrationState_t calState;
        float accTmp[3];
        bool calFailed = false;

        /* Calculate offset */
        sensorCalibrationResetState(&calState);

        for (int axis = 0; axis < 6; axis++) {
            sensorCalibrationPushSampleForOffsetCalculation(&calState, accSamples[axis]);
        }

        if (!sensorCalibrationSolveForOffset(&calState, accTmp)) {
            accTmp[X] = 0.0f;
            accTmp[Y] = 0.0f;
            accTmp[Z] = 0.0f;
            calFailed = true;
        }

        accelerometerConfigMutable()->accZero.raw[X] = lrintf(accTmp[X]);
        accelerometerConfigMutable()->accZero.raw[Y] = lrintf(accTmp[Y]);
        accelerometerConfigMutable()->accZero.raw[Z] = lrintf(accTmp[Z]);

        /* Not we can offset our accumulated averages samples and calculate scale factors and calculate gains */
        sensorCalibrationResetState(&calState);

        for (int axis = 0; axis < 6; axis++) {
            float accSample[3];

            accSample[X] = accSamples[axis][X] - accelerometerConfig()->accZero.raw[X];
            accSample[Y] = accSamples[axis][Y] - accelerometerConfig()->accZero.raw[Y];
            accSample[Z] = accSamples[axis][Z] - accelerometerConfig()->accZero.raw[Z];

            sensorCalibrationPushSampleForScaleCalculation(&calState, axis / 2, accSample, acc.dev.acc_1G);
        }

        if (!sensorCalibrationSolveForScale(&calState, accTmp)) {
            accTmp[X] = 1.0f;
            accTmp[Y] = 1.0f;
            accTmp[Z] = 1.0f;
            calFailed = true;
        }

        for (int axis = 0; axis < 3; axis++) {
            accelerometerConfigMutable()->accGain.raw[axis] = lrintf(accTmp[axis] * 4096);
        }

        if (calFailed) {
            // If failed - don't save and also invalidate the calibration data for all positions
            for (int axis = 0; axis < 6; axis++) {
                calibratedPosition[axis] = false;
            }
        }
        else {
            // saveConfigAndNotify will trigger eepromREAD and in turn call back the accelerometer gain validation
            // that will set ENABLE_STATE(ACCELEROMETER_CALIBRATED) if all is good
            saveConfigAndNotify();
            //Recompute all coeffs
            updateAccCoefficients();
        }
    }
}

static void applyAccelerationZero(void)
{
    float tmp[XYZ_AXIS_COUNT];

    //Apply zero
    arm_sub_f32(accADC, fAccZero, tmp, XYZ_AXIS_COUNT);
    //Apply gain
    arm_mult_f32(tmp, fAccGain, accADC, XYZ_AXIS_COUNT);
}

/*
 * Calculate measured acceleration in body frame in m/s^2
 */
void accGetMeasuredAcceleration(fpVector3_t *measuredAcc)
{
    arm_scale_f32(acc.accADCf, GRAVITY_CMSS, measuredAcc->v, XYZ_AXIS_COUNT);
}

/*
 * Return g's
 */
const acc_extremes_t* accGetMeasuredExtremes(void)
{
    return (const acc_extremes_t *)&acc.extremes;
}

float accGetMeasuredMaxG(void)
{
    return acc.maxG;
}

void resetGForceStats(void) {
    acc.maxG = 0.0f;

    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        acc.extremes[axis].min = 100;
        acc.extremes[axis].max = -100;
    }
}

void accUpdate(void)
{
#ifdef USE_SIMULATOR
    if (ARMING_FLAG(SIMULATOR_MODE_HITL)) {
        //output: acc.accADCf
        //unused: acc.dev.ADCRaw[], acc.accClipCount, acc.accVibeSq[]
        return;
    }
#endif
    if (!acc.dev.readFn(&acc.dev)) {
        return;
    }
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        accADC[axis] = acc.dev.ADCRaw[axis];
        DEBUG_SET(DEBUG_ACC, axis, accADC[axis]);
    }

    if (!ARMING_FLAG(SIMULATOR_MODE_SITL)) {
        performAcclerationCalibration();
        applyAccelerationZero();
    }

    applySensorAlignment(accADC, accADC, acc.dev.accAlign);
    applyBoardAlignment(accADC);

    // Calculate acceleration readings in G's
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        acc.accADCf[axis] = (float)accADC[axis] / acc.dev.acc_1G;
    }

    // Before filtering check for clipping and vibration levels
    if (fabsf(acc.accADCf[X]) > ACC_CLIPPING_THRESHOLD_G || fabsf(acc.accADCf[Y]) > ACC_CLIPPING_THRESHOLD_G || fabsf(acc.accADCf[Z]) > ACC_CLIPPING_THRESHOLD_G) {
        acc.isClipped = true;
        acc.accClipCount++;
    }
    else {
        acc.isClipped = false;
    }

    // Calculate vibration levels
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        // filter accel at 5hz
        const float accFloorFilt = pt1FilterApply(&accVibeFloorFilter[axis], acc.accADCf[axis]);

        // calc difference from this sample and 5hz filtered value, square and filter at 2hz
        const float accDiff = acc.accADCf[axis] - accFloorFilt;
        acc.accVibeSq[axis] = pt1FilterApply(&accVibeFilter[axis], accDiff * accDiff);
        acc.accVibe = fast_fsqrtf(acc.accVibeSq[X] + acc.accVibeSq[Y] + acc.accVibeSq[Z]);
    }

    // Filter acceleration
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        acc.accADCf[axis] = accSoftLpfFilterApplyFn(accSoftLpfFilter[axis], acc.accADCf[axis]);
    }

    if (accelerometerConfig()->acc_notch_hz) {
        for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
            acc.accADCf[axis] = accNotchFilterApplyFn(accNotchFilter[axis], acc.accADCf[axis]);
        }
    }

}

// Record extremes: min/max for each axis and acceleration vector modulus
void updateAccExtremes(void)
{
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        if (acc.accADCf[axis] < acc.extremes[axis].min) acc.extremes[axis].min = acc.accADCf[axis];
        if (acc.accADCf[axis] > acc.extremes[axis].max) acc.extremes[axis].max = acc.accADCf[axis];
    }

    float gforce = calc_length_pythagorean_3D(acc.accADCf[X], acc.accADCf[Y], acc.accADCf[Z]);
    if (gforce > acc.maxG) acc.maxG = gforce;
}

void accGetVibrationLevels(fpVector3_t *accVibeLevels)
{
    accVibeLevels->x = fast_fsqrtf(acc.accVibeSq[X]);
    accVibeLevels->y = fast_fsqrtf(acc.accVibeSq[Y]);
    accVibeLevels->z = fast_fsqrtf(acc.accVibeSq[Z]);
}

float accGetVibrationLevel(void)
{
    return acc.accVibe;
}

uint32_t accGetClipCount(void)
{
    return acc.accClipCount;
}

bool accIsClipped(void)
{
    return acc.isClipped;
}

void accSetCalibrationValues(void)
{
    if (!ARMING_FLAG(SIMULATOR_MODE_SITL) &&
        ((accelerometerConfig()->accZero.raw[X] == 0) && (accelerometerConfig()->accZero.raw[Y] == 0) && (accelerometerConfig()->accZero.raw[Z] == 0) &&
        (accelerometerConfig()->accGain.raw[X] == 4096) && (accelerometerConfig()->accGain.raw[Y] == 4096) &&(accelerometerConfig()->accGain.raw[Z] == 4096))) {
        DISABLE_STATE(ACCELEROMETER_CALIBRATED);
    }
    else {
        ENABLE_STATE(ACCELEROMETER_CALIBRATED);
    }
}

void accInitFilters(void)
{
    accSoftLpfFilterApplyFn = nullFilterApply;

    if (acc.accTargetLooptime && accelerometerConfig()->acc_lpf_hz) {

        switch (accelerometerConfig()->acc_soft_lpf_type)
        {
        case FILTER_PT1:
            accSoftLpfFilterApplyFn = (filterApplyFnPtr)pt1FilterApply;
            for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
                accSoftLpfFilter[axis] = &accFilter[axis].pt1;
                pt1FilterInit(accSoftLpfFilter[axis], accelerometerConfig()->acc_lpf_hz, US2S(acc.accTargetLooptime));
            }
            break;
        case FILTER_BIQUAD:
            accSoftLpfFilterApplyFn = (filterApplyFnPtr)biquadFilterApply;
            for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
                accSoftLpfFilter[axis] = &accFilter[axis].biquad;
                biquadFilterInitLPF(accSoftLpfFilter[axis], accelerometerConfig()->acc_lpf_hz, acc.accTargetLooptime);
            }
            break;
        }

    }

    const float accDt = US2S(acc.accTargetLooptime);
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        pt1FilterInit(&accVibeFloorFilter[axis], ACC_VIBE_FLOOR_FILT_HZ, accDt);
        pt1FilterInit(&accVibeFilter[axis], ACC_VIBE_FILT_HZ, accDt);
    }

    STATIC_FASTRAM biquadFilter_t accFilterNotch[XYZ_AXIS_COUNT];
    accNotchFilterApplyFn = nullFilterApply;

    if (acc.accTargetLooptime && accelerometerConfig()->acc_notch_hz) {
        accNotchFilterApplyFn = (filterApplyFnPtr)biquadFilterApply;
        for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
            accNotchFilter[axis] = &accFilterNotch[axis];
            biquadFilterInitNotch(accNotchFilter[axis], acc.accTargetLooptime, accelerometerConfig()->acc_notch_hz, accelerometerConfig()->acc_notch_cutoff);
        }
    }

}

bool accIsHealthy(void)
{
    return true;
}
