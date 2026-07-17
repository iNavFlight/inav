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

#include <stdbool.h>

#include <platform.h>

#ifdef USE_ORIENTATION_HOLD

#include "common/axis.h"
#include "common/maths.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/imu.h"
#include "flight/rotor_guard.h"

#include "navigation/navigation.h"

PG_REGISTER_WITH_RESET_TEMPLATE(rotorGuardConfig_t, rotorGuardConfig, PG_ROTOR_GUARD_CONFIG, 0);

PG_RESET_TEMPLATE(rotorGuardConfig_t, rotorGuardConfig,
    .bankDeg = SETTING_ROTOR_GUARD_BANK_DEFAULT,
    .sinkCms = SETTING_ROTOR_GUARD_SINK_DEFAULT,
    .recoveryPitchDeg = SETTING_ROTOR_GUARD_PITCH_DEFAULT,
    .throttleAddUs = SETTING_ROTOR_GUARD_THROTTLE_ADD_DEFAULT,
);

// The excursion must persist: a gust or a crisp figure entry crosses the
// bank line for a moment, a tip-over stays there (authority is gone)
#define ROTOR_GUARD_TRIP_MS     300
// Release hysteresis: wings back under this bank AND no longer sinking
#define ROTOR_GUARD_RELEASE_BANK_DEG 20

static bool guardRecovery = false;
static timeMs_t tripStartMs = 0;
static bool sticksSeenCentered = false;

void rotorGuardUpdate(void)
{
    if (!IS_RC_MODE_ACTIVE(BOXROTORGUARD) || !ARMING_FLAG(ARMED) || !STATE(AIRPLANE)) {
        guardRecovery = false;
        tripStartMs = 0;
        return;
    }

    const float bankDeg = ABS(attitude.values.roll) / 10.0f;
    const float vz = getEstimatedActualVelocity(Z);         // cm/s

    if (!guardRecovery) {
        // Tip-over signature: rolled past anything an autogyro flies on
        // purpose AND sinking - the soft-tilt rolloff, not a figure. Must
        // persist ROTOR_GUARD_TRIP_MS to reject transients.
        const bool tripping = bankDeg > rotorGuardConfig()->bankDeg
                           && vz < -(float)rotorGuardConfig()->sinkCms;
        if (tripping) {
            if (tripStartMs == 0) {
                tripStartMs = millis();
            } else if (millis() - tripStartMs > ROTOR_GUARD_TRIP_MS) {
                guardRecovery = true;
                sticksSeenCentered = false;
            }
        } else {
            tripStartMs = 0;
        }
    } else {
        // Release when the tilt authority is visibly back: wings level-ish
        // and the sink arrested (the thrust floor restored the inflow)
        if (bankDeg < ROTOR_GUARD_RELEASE_BANK_DEG && vz >= 0.0f) {
            guardRecovery = false;
            tripStartMs = 0;
        }
        // ... or the pilot takes over: sticks must return to center ONCE
        // (a deflection held through the tip-over is not a takeover), a
        // fresh roll/pitch input then releases immediately. Yaw stays
        // steering - same contract as the altitude floor.
        const bool deflected = ABS(rcCommand[ROLL]) > rcControlsConfig()->deadband
                            || ABS(rcCommand[PITCH]) > rcControlsConfig()->deadband;
        if (!sticksSeenCentered) {
            sticksSeenCentered = !deflected;
        } else if (deflected) {
            guardRecovery = false;
            tripStartMs = 0;
        }
    }
}

bool rotorGuardRecoveryActive(void)
{
    return guardRecovery;
}

float rotorGuardRecoveryPitchDeg(void)
{
    return (float)rotorGuardConfig()->recoveryPitchDeg;
}

#endif // USE_ORIENTATION_HOLD
