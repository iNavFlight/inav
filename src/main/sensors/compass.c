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
#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/compass/compass.h"
#include "drivers/compass/compass_ak8963.h"
#include "drivers/compass/compass_ak8975.h"
#include "drivers/compass/compass_fake.h"
#include "drivers/compass/compass_hmc5883l.h"
#include "drivers/compass/compass_mag3110.h"
#include "drivers/compass/compass_ist8310.h"
#include "drivers/compass/compass_ist8308.h"
#include "drivers/compass/compass_qmc5883l.h"
#include "drivers/compass/compass_mpu9250.h"
#include "drivers/compass/compass_lis3mdl.h"
#include "drivers/compass/compass_rm3100.h"
#include "drivers/compass/compass_vcm5883.h"
#include "drivers/compass/compass_msp.h"
#include "drivers/io.h"
#include "drivers/light_led.h"
#include "drivers/time.h"

#include "fc/config.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/mixer.h"

#include "io/gps.h"
#include "io/beeper.h"

#include "rx/rx.h"

#include "sensors/battery.h"
#include "sensors/boardalignment.h"
#include "sensors/compass.h"
#include "sensors/gyro.h"
#include "sensors/sensors.h"

mag_t mag; // mag access functions

//#ifdef USE_MAG

PG_REGISTER_WITH_RESET_TEMPLATE(compassConfig_t, compassConfig, PG_COMPASS_CONFIG, 6);

PG_RESET_TEMPLATE(compassConfig_t, compassConfig,
    .mag_align = SETTING_ALIGN_MAG_DEFAULT,
    .mag_hardware = SETTING_MAG_HARDWARE_DEFAULT,
    .mag_declination = SETTING_MAG_DECLINATION_DEFAULT,
#ifdef USE_DUAL_MAG
    .mag_to_use = SETTING_MAG_TO_USE_DEFAULT,
#endif
    .magCalibrationTimeLimit = SETTING_MAG_CALIBRATION_TIME_DEFAULT,
    .rollDeciDegrees = SETTING_ALIGN_MAG_ROLL_DEFAULT,
    .pitchDeciDegrees = SETTING_ALIGN_MAG_PITCH_DEFAULT,
    .yawDeciDegrees = SETTING_ALIGN_MAG_YAW_DEFAULT,
    .magGain = {SETTING_MAGGAIN_X_DEFAULT, SETTING_MAGGAIN_Y_DEFAULT, SETTING_MAGGAIN_Z_DEFAULT},
    .comp_permotor_expo = SETTING_PERMOTOR_EXPO_DEFAULT,
    .permotor_offset = {SETTING_PERMOTOR_X_DEFAULT, SETTING_PERMOTOR_Y_DEFAULT, SETTING_PERMOTOR_Z_DEFAULT},
);

static uint8_t magUpdatedAtLeastOnce = 0;

fpVector3_t permotor_initital_field;
fpVector3_t permotor_field_sum[4];
fpVector3_t permotor_compensation[4]; 

bool permotor_cal_running;
bool permotor_cal_running_once;

uint16_t permotor_cal_count[4];

float permotor_output_sum[4];

uint32_t permotor_start_ms[4];

uint32_t permotor_timeout = 0;

bool compassDetect(magDev_t *dev, magSensor_e magHardwareToUse)
{
    magSensor_e magHardware = MAG_NONE;
    requestedSensors[SENSOR_INDEX_MAG] = magHardwareToUse;

    dev->magAlign.useExternal = false;
    dev->magAlign.onBoard = ALIGN_DEFAULT;

    switch (magHardwareToUse) {
    case MAG_AUTODETECT:
        FALLTHROUGH;

    case MAG_QMC5883:
#ifdef USE_MAG_QMC5883
        if (qmc5883Detect(dev)) {
            magHardware = MAG_QMC5883;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (magHardwareToUse != MAG_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case MAG_HMC5883:
#ifdef USE_MAG_HMC5883
        if (hmc5883lDetect(dev)) {
            magHardware = MAG_HMC5883;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (magHardwareToUse != MAG_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case MAG_AK8975:
#ifdef USE_MAG_AK8975
        if (ak8975Detect(dev)) {
            magHardware = MAG_AK8975;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (magHardwareToUse != MAG_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case MAG_AK8963:
#ifdef USE_MAG_AK8963
        if (ak8963Detect(dev)) {
            magHardware = MAG_AK8963;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (magHardwareToUse != MAG_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case MAG_GPS:
#ifdef USE_GPS
        if (gpsMagDetect(dev)) {
            magHardware = MAG_GPS;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (magHardwareToUse != MAG_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case MAG_MAG3110:
#ifdef USE_MAG_MAG3110
        if (mag3110detect(dev)) {
            magHardware = MAG_MAG3110;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (magHardwareToUse != MAG_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case MAG_IST8310:
#ifdef USE_MAG_IST8310
        if (ist8310Detect(dev)) {
            magHardware = MAG_IST8310;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (magHardwareToUse != MAG_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case MAG_IST8308:
#ifdef USE_MAG_IST8308
        if (ist8308Detect(dev)) {
            magHardware = MAG_IST8308;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (magHardwareToUse != MAG_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case MAG_MPU9250:
#ifdef USE_MAG_MPU9250
        if (mpu9250CompassDetect(dev)) {
            magHardware = MAG_MPU9250;
            break;
        }
#endif
        FALLTHROUGH;

    case MAG_LIS3MDL:
#ifdef USE_MAG_LIS3MDL
        if (lis3mdlDetect(dev)) {
            magHardware = MAG_LIS3MDL;
            break;
        }
#endif

        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (magHardwareToUse != MAG_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case MAG_MSP:
#ifdef USE_MAG_MSP
        // Skip autodetection for MSP mag
        if (magHardwareToUse != MAG_AUTODETECT && mspMagDetect(dev)) {
            magHardware = MAG_MSP;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (magHardwareToUse != MAG_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case MAG_RM3100:
#ifdef USE_MAG_RM3100
        if (rm3100MagDetect(dev)) {
            magHardware = MAG_RM3100;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (magHardwareToUse != MAG_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case MAG_VCM5883:
#ifdef USE_MAG_VCM5883
        if (vcm5883Detect(dev)) {
            magHardware = MAG_VCM5883;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (magHardwareToUse != MAG_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case MAG_FAKE:
#ifdef USE_FAKE_MAG
        if (fakeMagDetect(dev)) {
            magHardware = MAG_FAKE;
            break;
        }
#endif
        /* If we are asked for a specific sensor - break out, otherwise - fall through and continue */
        if (magHardwareToUse != MAG_AUTODETECT) {
            break;
        }
        FALLTHROUGH;

    case MAG_NONE:
        magHardware = MAG_NONE;
        break;
    }

    if (magHardware == MAG_NONE) {
        sensorsClear(SENSOR_MAG);
        return false;
    }

    detectedSensors[SENSOR_INDEX_MAG] = magHardware;
    sensorsSet(SENSOR_MAG);
    return true;
}

bool compassInit(void)
{
#ifdef USE_DUAL_MAG
    mag.dev.magSensorToUse = compassConfig()->mag_to_use;
#else
    mag.dev.magSensorToUse = 0;
#endif

    if (!compassDetect(&mag.dev, compassConfig()->mag_hardware)) {
        return false;
    }
    // initialize and calibration. turn on led during mag calibration (calibration routine blinks it)
    LED1_ON;
    const bool ret = mag.dev.init(&mag.dev);
    LED1_OFF;

    if (!ret) {
        sensorsClear(SENSOR_MAG);
    }

    if (compassConfig()->rollDeciDegrees != 0 ||
        compassConfig()->pitchDeciDegrees != 0 ||
        compassConfig()->yawDeciDegrees != 0) {

        // Externally aligned compass
        mag.dev.magAlign.useExternal = true;

        fp_angles_t compassAngles = {
             .angles.roll = DECIDEGREES_TO_RADIANS(compassConfig()->rollDeciDegrees),
             .angles.pitch = DECIDEGREES_TO_RADIANS(compassConfig()->pitchDeciDegrees),
             .angles.yaw = DECIDEGREES_TO_RADIANS(compassConfig()->yawDeciDegrees),
        };
        rotationMatrixFromAngles(&mag.dev.magAlign.externalRotation, &compassAngles);
    } else {
        mag.dev.magAlign.useExternal = false;
        if (compassConfig()->mag_align != ALIGN_DEFAULT) {
            mag.dev.magAlign.onBoard = compassConfig()->mag_align;
        } else {
            mag.dev.magAlign.onBoard = CW270_DEG_FLIP;  // The most popular default is 270FLIP for external mags
        }
    }

    return ret;
}

bool compassIsHealthy(void)
{
    return (mag.magADC[X] != 0) || (mag.magADC[Y] != 0) || (mag.magADC[Z] != 0);
}

bool compassIsReady(void)
{
    return magUpdatedAtLeastOnce;
}

bool compassIsCalibrationComplete(void)
{
    if (STATE(COMPASS_CALIBRATED)) {
        return true;
    }
    else {
        return false;
    }
}

void compassUpdate(timeUs_t currentTimeUs)
{
    static sensorCalibrationState_t calState;
    static timeUs_t calStartedAt = 0;
    static int16_t magPrev[XYZ_AXIS_COUNT];
    static int magAxisDeviation[XYZ_AXIS_COUNT];

    // Check magZero
    if (
        compassConfig()->magZero.raw[X] == 0 && compassConfig()->magZero.raw[Y] == 0 && compassConfig()->magZero.raw[Z] == 0 &&
        compassConfig()->magGain[X] == 1024 && compassConfig()->magGain[Y] == 1024 && compassConfig()->magGain[Z] == 1024  
    ) {
        DISABLE_STATE(COMPASS_CALIBRATED);
    }
    else {
        ENABLE_STATE(COMPASS_CALIBRATED);
    }

    if (!mag.dev.read(&mag.dev)) {
        mag.magADC[X] = 0;
        mag.magADC[Y] = 0;
        mag.magADC[Z] = 0;
        return;
    }

    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        mag.magADC[axis] = mag.dev.magADCRaw[axis];  // int32_t copy to work with
    }

    if (STATE(CALIBRATE_MAG)) {
        calStartedAt = currentTimeUs;

        for (int axis = 0; axis < 3; axis++) {
            compassConfigMutable()->magZero.raw[axis] = 0;
            compassConfigMutable()->magGain[axis] = 1024;
            magPrev[axis] = 0;
            magAxisDeviation[axis] = 0;  // Gain is based on the biggest absolute deviation from the mag zero point. Gain computation starts at 0
        }

        beeper(BEEPER_ACTION_SUCCESS);

        sensorCalibrationResetState(&calState);
        DISABLE_STATE(CALIBRATE_MAG);
    }

    if (calStartedAt != 0) {
        if ((currentTimeUs - calStartedAt) < (compassConfig()->magCalibrationTimeLimit * 1000000)) {
            LED0_TOGGLE;

            float diffMag = 0;
            float avgMag = 0;

            for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
                diffMag += (mag.magADC[axis] - magPrev[axis]) * (mag.magADC[axis] - magPrev[axis]);
                avgMag += (mag.magADC[axis] + magPrev[axis]) * (mag.magADC[axis] + magPrev[axis]) / 4.0f;

                // Find the biggest sample deviation together with sample' sign
                if (ABS(mag.magADC[axis]) > ABS(magAxisDeviation[axis])) {
                    magAxisDeviation[axis] = mag.magADC[axis];
                }

            }

            // sqrtf(diffMag / avgMag) is a rough approximation of tangent of angle between magADC and magPrev. tan(8 deg) = 0.14
            if ((avgMag > 0.01f) && ((diffMag / avgMag) > (0.14f * 0.14f))) {
                sensorCalibrationPushSampleForOffsetCalculation(&calState, mag.magADC);

                for (int axis = 0; axis < 3; axis++) {
                    magPrev[axis] = mag.magADC[axis];
                }
            }
        } else {
            float magZerof[3];
            sensorCalibrationSolveForOffset(&calState, magZerof);

            for (int axis = 0; axis < 3; axis++) {
                compassConfigMutable()->magZero.raw[axis] = lrintf(magZerof[axis]);
            }

            /*
             * Scale calibration
             * We use max absolute value of each axis as scale calibration with constant 1024 as base
             * It is dirty, but worth checking if this will solve the problem of changing mag vector when UAV is tilted
             */
            for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
                compassConfigMutable()->magGain[axis] = ABS(magAxisDeviation[axis] - compassConfig()->magZero.raw[axis]);
            }

            calStartedAt = 0;
            saveConfigAndNotify();
        }
    }
    else {
        if (IS_RC_MODE_ACTIVE(BOXPERMOTOR)) {
            compass_permotor_update();
        } else {
            permotor_cal_running_once = false;
        }

        for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
            mag.magADC[axis] = (mag.magADC[axis] - compassConfig()->magZero.raw[axis]) * 1024 / compassConfig()->magGain[axis];
            mag.magADC[axis] += compassConfig()->permotor_offset[axis];
        }
    }

    if (mag.dev.magAlign.useExternal) {
        const fpVector3_t v = {
            .x = mag.magADC[X],
            .y = mag.magADC[Y],
            .z = mag.magADC[Z],
         };

        fpVector3_t rotated;

        rotationMatrixRotateVector(&rotated, &v, &mag.dev.magAlign.externalRotation);

         mag.magADC[X] = rotated.x;
         mag.magADC[Y] = rotated.y;
         mag.magADC[Z] = rotated.z;

    } else {
        // On-board compass
        applySensorAlignment(mag.magADC, mag.magADC, mag.dev.magAlign.onBoard);
        applyBoardAlignment(mag.magADC);
    }

    magUpdatedAtLeastOnce = 1;
}

static float motor_scaled_output(uint8_t motor_numb)
{
    // convert to range 0.0f ~ 1.0f
    float scaled_output = ((2.0 * ((float)motor[motor_numb] - rxConfig()->mincheck) / (rxConfig()->maxcheck - rxConfig()->mincheck) - 1.0) + 1) * 0.5f;

    if (scaled_output <= 0.0f) {
        return 0.0f;
    }

    // scale for voltage
    scaled_output *= getBatteryVoltage() / 100;

    // apply expo correction
    scaled_output = powf(scaled_output, compassConfig()->comp_permotor_expo);

    return scaled_output;
}

/*
  Calculate total offset for per-motor compensation
  Works only with quadricopters
 */
void compass_permotor_update(void)
{

    if (!STATE(MULTIROTOR) && getMotorCount() > 3) { // work only in quadcopter
        return;
    }
    
    if (ARMING_FLAG(ARMED)) { // don't work in flight
        return;
    }

    if (motor_disarmed[0] < 1400) { // check if the user has put at least 40% throttle on the engines 
        return;
    }

    uint32_t time_now = millis();

    if (!permotor_cal_running) {

        if (!permotor_cal_running_once) {
            permotor_cal_running = true;
            permotor_cal_running_once = true;
        }

        for (uint8_t i = 0; i < 4; i++) {
            permotor_field_sum[i].x = 0.0f;
            permotor_field_sum[i].y = 0.0f;
            permotor_field_sum[i].z = 0.0f;
            permotor_output_sum[i] = 0.0f;
            permotor_cal_count[i] = 0;
            permotor_start_ms[i] = 0;
        }

        permotor_timeout = time_now;

        permotor_initital_field.x = mag.magADC[X];
        permotor_initital_field.y = mag.magADC[Y];
        permotor_initital_field.z = mag.magADC[Z];
    
        return;
    }

    // accumulate per-motor sums
    for (uint8_t i = 0; i < 4; i++) {
        float scaled_output = motor_scaled_output(i);

        if (scaled_output <= 0.0f) {
            // motor is off
            permotor_start_ms[i] = 0;
            continue;
        }

        if (permotor_start_ms[i] == 0) {
            permotor_start_ms[i] = time_now;
        }

        if (time_now - permotor_start_ms[i] < 500) {
            // motor must run for 0.5s to settle
            continue;
        }

        // accumulate a sample
        permotor_field_sum[i].x += mag.magADC[X];
        permotor_field_sum[i].y += mag.magADC[Y];
        permotor_field_sum[i].z += mag.magADC[Z];
        permotor_output_sum[i] += scaled_output;
        permotor_cal_count[i]++;
    }

    if (time_now - permotor_timeout >= 10000) { // waits to reach the maximum time of the function to finish

        for (uint8_t i = 0; i < 4; i++) {

            if (permotor_cal_count[i] == 0) {
                continue;
            }

            // calculate effective output
            float output = permotor_output_sum[i] / permotor_cal_count[i];

            // calculate amount that field changed from initial field
            fpVector3_t field_changed;
            field_changed.x = permotor_initital_field.x - (permotor_field_sum[i].x / permotor_cal_count[i]);
            field_changed.y = permotor_initital_field.y - (permotor_field_sum[i].y / permotor_cal_count[i]);
            field_changed.z = permotor_initital_field.z - (permotor_field_sum[i].z / permotor_cal_count[i]);

            if (output <= 0.0f) {
                continue;
            }

            permotor_compensation[i].x = field_changed.x / output;
            permotor_compensation[i].y = field_changed.y / output;
            permotor_compensation[i].z = field_changed.z / output;

            float scaled_output = motor_scaled_output(i);

            compassConfigMutable()->permotor_offset[X] += permotor_compensation[i].x * scaled_output;
            compassConfigMutable()->permotor_offset[Y] += permotor_compensation[i].y * scaled_output;
            compassConfigMutable()->permotor_offset[Z] += permotor_compensation[i].z * scaled_output;
        }

        LED0_OFF;

        saveConfigAndNotify();
        permotor_cal_running = false;
    }
    else
    {
        LED0_ON;

        if (permotor_timeout == 0) { // is this the first time the "compass_permotor_timeout" has been used? yes...
            permotor_timeout = time_now;
        }
    }
}

//#endif
