/*
 * This file is part of INAV Project.
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

#include "fc/config.h"
#include "config/config_reset.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "flight/ez_tune.h"

#include "fc/settings.h"
#include "flight/pid.h"
#include "sensors/gyro.h"

PG_REGISTER_PROFILE_WITH_RESET_TEMPLATE(ezTuneSettings_t, ezTune, PG_EZ_TUNE, 0);

PG_RESET_TEMPLATE(ezTuneSettings_t, ezTune,
    .enabled = SETTING_EZ_TUNE_ENABLED_DEFAULT,
    .filterHz = SETTING_EZ_TUNE_FILTER_HZ_DEFAULT,
    .axisRatio = SETTING_EZ_TUNE_AXIS_RATIO_DEFAULT,
    .response = SETTING_EZ_TUNE_RESPONSE_DEFAULT,
    .damping = SETTING_EZ_TUNE_DAMPING_DEFAULT,
    .stability = SETTING_EZ_TUNE_STABILITY_DEFAULT,
    .aggressiveness = SETTING_EZ_TUNE_AGGRESSIVENESS_DEFAULT,
);

#define EZ_TUNE_PID_RP_DEFAULT { 40, 75, 23, 100 }
#define EZ_TUNE_PID_YAW_DEFAULT { 45, 80, 0, 100 }

#define EZ_TUNE_YAW_SCALE 0.5f

static float computePt1FilterDelayMs(uint8_t filterHz) {
    return 1.0f / (2.0f * M_PIf * filterHz);
}

static float getYawPidScale(float input) {
    const float normalized = (input - 100) * 0.01f;

    return 1.0f + (normalized * 0.5f); 
}

/**
 * Update INAV settings based on current EZTune settings
 * This has to be called every time control profile is changed, or EZTune settings are changed
 * FIXME call on profile change
 * FIXME call on EZTune settings change
 */
void ezTuneUpdate(void) {
    if (ezTune()->enabled) {

        // Setup filtering

        //Enable Smith predictor
        pidProfileMutable()->smithPredictorDelay = computePt1FilterDelayMs(ezTune()->filterHz)
            + computePt1FilterDelayMs(gyroConfig()->gyro_anti_aliasing_lpf_hz);

        //Set Dterm LPF
        pidProfileMutable()->dterm_lpf_hz = MAX(ezTune()->filterHz - 5, 50);
        pidProfileMutable()->dterm_lpf_type = FILTER_PT2;

        //Set main gyro filter
        gyroConfigMutable()->gyro_main_lpf_hz = ezTune()->filterHz;
        gyroConfigMutable()->gyro_main_lpf_type = FILTER_PT1;

        //Set anti-aliasing filter
        gyroConfigMutable()->gyro_anti_aliasing_lpf_hz = MIN(ezTune()->filterHz * 2, 250);
        gyroConfigMutable()->gyro_anti_aliasing_lpf_type = FILTER_PT1;

        //Enable dynamic notch
        gyroConfigMutable()->dynamicGyroNotchEnabled = 1;
        gyroConfigMutable()->dynamicGyroNotchQ = 250;
        gyroConfigMutable()->dynamicGyroNotchMinHz = MAX(ezTune()->filterHz * 0.667f, SETTING_DYNAMIC_GYRO_NOTCH_MIN_HZ_DEFAULT);
        gyroConfigMutable()->dynamicGyroNotchMode = DYNAMIC_NOTCH_MODE_3D;

        //Make sure Kalman filter is enabled
        gyroConfigMutable()->kalmanEnabled = 1;
        if (ezTune()->filterHz < 150) {
            gyroConfigMutable()->kalman_q = 200;
        } else {
            gyroConfigMutable()->kalman_q = scaleRangef(ezTune()->filterHz, 150, 300, 200, 400);
        }

        //Disable dynamic LPF
        gyroConfigMutable()->useDynamicLpf = 0;

        //Setup PID controller

        const uint8_t pidDefaults[4] = EZ_TUNE_PID_RP_DEFAULT;
        const uint8_t pidDefaultsYaw[4] = EZ_TUNE_PID_YAW_DEFAULT;
        const float pitchRatio = ezTune()->axisRatio / 100.0f;

        //Roll
        pidProfileMutable()->bank_mc.pid[PID_ROLL].P = pidDefaults[0] * ezTune()->response / 100.0f;
        pidProfileMutable()->bank_mc.pid[PID_ROLL].I = pidDefaults[1] * ezTune()->stability / 100.0f;
        pidProfileMutable()->bank_mc.pid[PID_ROLL].D = pidDefaults[2] * ezTune()->damping / 100.0f;
        pidProfileMutable()->bank_mc.pid[PID_ROLL].FF = pidDefaults[3] * ezTune()->aggressiveness / 100.0f;

        //Pitch
        pidProfileMutable()->bank_mc.pid[PID_PITCH].P = pidDefaults[0] * ezTune()->response / 100.0f * pitchRatio;
        pidProfileMutable()->bank_mc.pid[PID_PITCH].I = pidDefaults[1] * ezTune()->stability / 100.0f * pitchRatio;
        pidProfileMutable()->bank_mc.pid[PID_PITCH].D = pidDefaults[2] * ezTune()->damping / 100.0f * pitchRatio;
        pidProfileMutable()->bank_mc.pid[PID_PITCH].FF = pidDefaults[3] * ezTune()->aggressiveness / 100.0f * pitchRatio;
    
        //Yaw
        pidProfileMutable()->bank_mc.pid[PID_YAW].P = pidDefaultsYaw[0] * getYawPidScale(ezTune()->response);
        pidProfileMutable()->bank_mc.pid[PID_YAW].I = pidDefaultsYaw[1] * getYawPidScale(ezTune()->stability);
        pidProfileMutable()->bank_mc.pid[PID_YAW].D = pidDefaultsYaw[2] * getYawPidScale(ezTune()->damping);
        pidProfileMutable()->bank_mc.pid[PID_YAW].FF = pidDefaultsYaw[3] * getYawPidScale(ezTune()->aggressiveness);
    }
}