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

#include "flight/pid.h"

extern int16_t throttleAngleCorrection;
extern t_fp_vector EstG;
extern int16_t accSmooth[XYZ_AXIS_COUNT];
extern int16_t smallAngle;
extern t_fp_vector imuAverageVelocity;
extern t_fp_vector imuAverageAcceleration;

typedef struct rollAndPitchInclination_s {
    // absolute angle inclination in multiple of 0.1 degree    180 deg = 1800
    int16_t rollDeciDegrees;
    int16_t pitchDeciDegrees;
} rollAndPitchInclination_t_def;

typedef union {
    int16_t raw[ANGLE_INDEX_COUNT];
    rollAndPitchInclination_t_def values;
} rollAndPitchInclination_t;

extern rollAndPitchInclination_t inclination;

typedef struct imuRuntimeConfig_s {
    uint8_t acc_lpf_factor;
    uint8_t acc_unarmedcal;
    float gyro_cmpf_factor;
    float gyro_cmpfm_factor;
    uint8_t small_angle;
} imuRuntimeConfig_t;

void imuConfigure(imuRuntimeConfig_t *initialImuRuntimeConfig, pidProfile_t *initialPidProfile);

void calculateEstimatedAltitude(uint32_t currentTime);
void imuUpdate(rollAndPitchTrims_t *accelerometerTrims);
float calculateThrottleAngleScale(uint16_t throttle_correction_angle);
int16_t calculateThrottleAngleCorrection(uint8_t throttle_correction_value, int16_t throttle_correction_angle);
int16_t calculateTiltAngle(void);
float calculateAccLowPassFilterRCTimeConstant(float acc_lpf_cutoff);
float calculateCosTiltAngle(void);

int16_t imuCalculateHeading(t_fp_vector *vec);
void imuApplyFilterToActualVelocity(uint8_t axis, float cfFactor, float referenceVelocity);