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

#include "platform.h"

#include "common/axis.h"

#include "config/config_reset.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "fc/config.h"
#include "fc/control_profile.h"
#include "fc/rc_curves.h"
#include "fc/settings.h"

const controlConfig_t *currentControlProfile;

PG_REGISTER_ARRAY_WITH_RESET_FN(controlConfig_t, MAX_CONTROL_PROFILE_COUNT, controlProfiles, PG_CONTROL_PROFILES, 0);

void pgResetFn_controlProfiles(controlConfig_t *instance)
{
    for (int i = 0; i < MAX_CONTROL_PROFILE_COUNT; i++) {
        RESET_CONFIG(controlConfig_t, &instance[i],
            .throttle = {
                .rcMid8 = SETTING_THR_MID_DEFAULT,
                .rcExpo8 = SETTING_THR_EXPO_DEFAULT,
                .dynPID = SETTING_TPA_RATE_DEFAULT,
                .dynPID_on_YAW = SETTING_TPA_ON_YAW_DEFAULT,
                .pa_breakpoint = SETTING_TPA_BREAKPOINT_DEFAULT,
                .fixedWingTauMs = SETTING_FW_TPA_TIME_CONSTANT_DEFAULT,
                .apa_pow = SETTING_APA_POW_DEFAULT,
                .tpa_pitch_compensation = SETTING_TPA_PITCH_COMPENSATION_DEFAULT
            },

            .stabilized = {
                .rcExpo8 = SETTING_RC_EXPO_DEFAULT,
                .rcYawExpo8 = SETTING_RC_YAW_EXPO_DEFAULT,
                .rates[FD_ROLL] = SETTING_ROLL_RATE_DEFAULT,
                .rates[FD_PITCH] = SETTING_PITCH_RATE_DEFAULT,
                .rates[FD_YAW] = SETTING_YAW_RATE_DEFAULT,
            },

            .manual = {
                .rcExpo8 = SETTING_MANUAL_RC_EXPO_DEFAULT,
                .rcYawExpo8 = SETTING_MANUAL_RC_YAW_EXPO_DEFAULT,
                .rates[FD_ROLL] = SETTING_MANUAL_ROLL_RATE_DEFAULT,
                .rates[FD_PITCH] = SETTING_MANUAL_PITCH_RATE_DEFAULT,
                .rates[FD_YAW] = SETTING_MANUAL_YAW_RATE_DEFAULT
            },

            .misc = {
                .fpvCamAngleDegrees = SETTING_FPV_MIX_DEGREES_DEFAULT
            },
        #ifdef USE_RATE_DYNAMICS
            .rateDynamics = {
                .sensitivityCenter = SETTING_RATE_DYNAMICS_CENTER_SENSITIVITY_DEFAULT,
                .sensitivityEnd = SETTING_RATE_DYNAMICS_END_SENSITIVITY_DEFAULT,
                .correctionCenter = SETTING_RATE_DYNAMICS_CENTER_CORRECTION_DEFAULT,
                .correctionEnd = SETTING_RATE_DYNAMICS_END_CORRECTION_DEFAULT,
                .weightCenter = SETTING_RATE_DYNAMICS_CENTER_WEIGHT_DEFAULT,
                .weightEnd = SETTING_RATE_DYNAMICS_END_WEIGHT_DEFAULT,
            }
        #endif
        );
    }
}

void setControlProfile(uint8_t profileIndex)
{
    if (profileIndex >= MAX_CONTROL_PROFILE_COUNT) {
        profileIndex = 0;
    }
    currentControlProfile = controlProfiles(profileIndex);
}

void activateControlConfig(void)
{
    generateThrottleCurve(currentControlProfile);
}

void changeControlProfile(uint8_t profileIndex)
{
    setControlProfile(profileIndex);
    activateControlConfig();
}
