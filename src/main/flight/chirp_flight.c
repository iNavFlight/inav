/*
 * This file is part of INAV.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

#include "platform.h"

#if defined(USE_CHIRP) && defined(USE_BLACKBOX)

#include "build/debug.h"
#include "common/axis.h"
#include "common/maths.h"
#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"
#include "drivers/time.h"
#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"
#include "flight/chirp.h"
#include "flight/chirp_flight.h"
#include "flight/failsafe.h"
#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/mixer_profile.h"
#include "blackbox/blackbox.h"
#include "programming/logic_condition.h"
#include "rx/rx.h"
#include "sensors/gyro.h"

PG_REGISTER_WITH_RESET_TEMPLATE(chirpConfig_t, chirpConfig, PG_CHIRP_CONFIG, 0);

PG_RESET_TEMPLATE(chirpConfig_t, chirpConfig,
    .axis = SETTING_CHIRP_AXIS_DEFAULT,
    .triggerChannel = SETTING_CHIRP_TRIGGER_CHANNEL_DEFAULT,
    .amplitude = SETTING_CHIRP_AMPLITUDE_DEFAULT,
);

static chirpState_t chirp;
static chirpConfig_t runConfig;
static uint8_t runProfile;
static uint8_t runMixerProfile;
static int16_t runThrottle;

void chirpFlightUpdate(float dt, bool controllerReady)
{
    const chirpConfig_t *config = chirpConfig();
    const bool enabled = config->axis != CHIRP_AXIS_OFF;
    const uint32_t now = (uint32_t)micros();
    uint16_t inhibit = 0;
    bool switchLow = false;
    bool switchHigh = false;

    if (!enabled) {
        chirpUpdate(&chirp, now, false, false, false, 0, 0);
    } else {
        // Channels 13-32 may carry an MSP auxiliary overlay rather than the receiver switch.
        if (config->axis > CHIRP_AXIS_YAW || config->triggerChannel < 5 ||
            config->triggerChannel > rxRuntimeConfig.channelCount || config->triggerChannel > 12) {
            inhibit |= CHIRP_INHIBIT_CONFIG;
        } else {
            const int channel = rxGetChannelValue(config->triggerChannel - 1);
            switchLow = channel >= CHANNEL_RANGE_MIN && channel <= 1300;
            switchHigh = channel >= 1700 && channel <= CHANNEL_RANGE_MAX;
        }

        // Only manual-throttle ANGLE flight on a conventional multirotor is supported.
        // Reject all other flight-mode bits, including navigation and failsafe.
        if (!controllerReady || !ARMING_FLAG(ARMED) || !STATE(MULTIROTOR) || STATE(LANDING_DETECTED) ||
            mixerConfig()->platformType != PLATFORM_MULTIROTOR || isMixerTransitionMixing ||
            feature(FEATURE_REVERSIBLE_MOTORS) || flightModeFlags != ANGLE_MODE ||
            failsafeIsActive() || !failsafeIsReceivingRxData() || !rxIsReceivingSignal() || !rxAreFlightChannelsValid() ||
            IS_RC_MODE_ACTIVE(BOXMSPRCOVERRIDE)) {
            inhibit |= CHIRP_INHIBIT_FLIGHT;
        }

#ifdef USE_PROGRAMMING_FRAMEWORK
        if (LOGIC_CONDITION_GLOBAL_FLAG(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_THROTTLE |
            LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_THROTTLE_SCALE | LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_RC_CHANNEL |
            LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_FLIGHT_AXIS)) {
            inhibit |= CHIRP_INHIBIT_PILOT;
        }
#endif
        if (rcCommand[THROTTLE] <= 1200 || rcCommand[THROTTLE] >= 1800) {
            inhibit |= CHIRP_INHIBIT_PILOT;
        }
        for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
            if (abs(rxGetChannelValue(axis) - PWM_RANGE_MIDDLE) > 50 ||
                abs(rcCommand[axis]) > 50 || isFlightAxisAngleOverrideActive(axis) || isFlightAxisRateOverrideActive(axis)) {
                inhibit |= CHIRP_INHIBIT_PILOT;
            }
            if (!isfinite(gyro.gyroADCf[axis]) || fabsf(gyro.gyroADCf[axis]) > 200) {
                inhibit |= CHIRP_INHIBIT_MOTION;
            }
        }
        if (abs(attitude.values.roll) > 200 || abs(attitude.values.pitch) > 200) {
            inhibit |= CHIRP_INHIBIT_MOTION;
        }
        if (!isfinite(getMotorMixRange()) || getMotorMixRange() >= 0.9f) {
            inhibit |= CHIRP_INHIBIT_SATURATION;
        }

        // Require ten samples per cycle at the highest excitation frequency.
        // No automatic filter or logging configuration changes are made in flight.
        const blackboxConfig_t *logConfig = blackboxConfig();
        if (!isfinite(dt) || dt <= 0 || dt > 1.0f / (10 * CHIRP_END_HZ) ||
            debugMode != DEBUG_CHIRP || getBlackboxState() != BLACKBOX_STATE_RUNNING ||
            !logConfig->rate_num || !logConfig->rate_denom ||
            (float)logConfig->rate_num / logConfig->rate_denom < dt * (10 * CHIRP_END_HZ)) {
            inhibit |= CHIRP_INHIBIT_LOGGING;
        }

        const bool active = chirp.phase == CHIRP_SETTLING || chirp.phase == CHIRP_RUNNING;
        if (active && abs(rcCommand[THROTTLE] - runThrottle) > 100) {
            inhibit |= CHIRP_INHIBIT_PILOT;
        }
        if (active && (config->axis != runConfig.axis || config->triggerChannel != runConfig.triggerChannel ||
            config->amplitude != runConfig.amplitude || getConfigProfile() != runProfile ||
            getConfigMixerProfile() != runMixerProfile)) {
            inhibit |= CHIRP_INHIBIT_CONFIG;
        }

        chirpUpdate(&chirp, now, true, switchLow, switchHigh, inhibit, config->amplitude);
        if (!active && chirp.phase == CHIRP_SETTLING) {
            runConfig = *config;
            runProfile = getConfigProfile();
            runMixerProfile = getConfigMixerProfile();
            runThrottle = rcCommand[THROTTLE];
        }
    }

    DEBUG_SET(DEBUG_CHIRP, 0, chirp.phase);
    DEBUG_SET(DEBUG_CHIRP, 1, config->axis);
    DEBUG_SET(DEBUG_CHIRP, 2, lrintf(chirp.output * 100));
    DEBUG_SET(DEBUG_CHIRP, 3, lrintf(chirp.frequency * 100));
    DEBUG_SET(DEBUG_CHIRP, 4, chirp.inhibit);
    DEBUG_SET(DEBUG_CHIRP, 5, 0);
    DEBUG_SET(DEBUG_CHIRP, 6, 0);
    DEBUG_SET(DEBUG_CHIRP, 7, 0);
}

float chirpApplyRate(int axis, float rate)
{
    return rate + ((axis + 1 == chirpConfig()->axis) ? chirp.output : 0);
}

void chirpLogResponse(int axis, float setpoint, float measurement, float output)
{
    if (axis + 1 == chirpConfig()->axis) {
        DEBUG_SET(DEBUG_CHIRP, 5, lrintf(setpoint * 100));
        DEBUG_SET(DEBUG_CHIRP, 6, lrintf(measurement * 100));
        DEBUG_SET(DEBUG_CHIRP, 7, lrintf(output * 100));
    }
}

#endif
