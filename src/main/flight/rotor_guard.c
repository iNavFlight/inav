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

#ifdef USE_FW_AEROBATICS

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
#include "flight/mixer.h"
#include "flight/rotor_guard.h"

#include "navigation/navigation.h"

#include "sensors/battery.h"

PG_REGISTER_WITH_RESET_TEMPLATE(rotorGuardConfig_t, rotorGuardConfig, PG_ROTOR_GUARD_CONFIG, 2);

PG_RESET_TEMPLATE(rotorGuardConfig_t, rotorGuardConfig,
    .bankDeg = SETTING_ROTOR_GUARD_BANK_DEFAULT,
    .sinkCms = SETTING_ROTOR_GUARD_SINK_DEFAULT,
    .recoveryPitchDeg = SETTING_ROTOR_GUARD_PITCH_DEFAULT,
    .throttleBoostPct = SETTING_ROTOR_GUARD_THROTTLE_BOOST_DEFAULT,
    .minHeightM = SETTING_ROTOR_GUARD_MIN_HEIGHT_DEFAULT,
    .rollLimitDeg = SETTING_ROTOR_GUARD_ROLL_LIMIT_DEFAULT,
    .pitchLimitDeg = SETTING_ROTOR_GUARD_PITCH_LIMIT_DEFAULT,
);

// The excursion must persist: a gust or a crisp figure entry crosses the
// bank line for a moment, a tip-over stays there (authority is gone)
#define ROTOR_GUARD_TRIP_MS     300
// Release hysteresis: wings back under this bank, held for the window
#define ROTOR_GUARD_RELEASE_BANK_DEG 20
// Minimum time on the recovery before any release: the wings answering is
// the CATCH dynamics, not proof the rotor is healthy - the rpm rebuilds
// from inflow with a seconds-long time constant, and a release into a
// still-starved rotor re-tips DEEPER (measured: -48 caught, released
// after 1.5 s, re-tipped to -138 and into the ground). Time on the
// throttle floor is the honest rpm proxy when no rpm feedback exists.
#define ROTOR_GUARD_MIN_HOLD_MS 5000
// Length of the initial max-throttle burst of the recovery
#define ROTOR_GUARD_BURST_MS    2000

static bool guardRecovery = false;
static timeMs_t tripStartMs = 0;
static timeMs_t recoveryStartMs = 0;
static timeMs_t levelSinceMs = 0;
static bool sticksSeenCentered = false;
static int16_t preTripThrottleUs = 0;   // the operating throttle when the trip
                                        // fired - the RELATIVE boost baseline

void rotorGuardUpdate(void)
{
    if (!IS_RC_MODE_ACTIVE(BOXROTORGUARD) || !ARMING_FLAG(ARMED) || !STATE(AIRPLANE)) {
        guardRecovery = false;
        tripStartMs = 0;
        return;
    }

    // THE PILOT OVERRIDES THE AUTOPILOT, throttle included: an idle stick
    // is landing intent - the guard must never spin the thrust up against
    // it (a tip in the flare would otherwise force a go-around, and a
    // rollout tip would get POWER on the ground). No trip at idle, and
    // pulling the throttle to idle releases a running recovery instantly.
    // The guard's only lever is thrust anyway - without permission to use
    // it a recovery is pointless.
    if (throttleStickIsLow()) {
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
                recoveryStartMs = millis();
                levelSinceMs = 0;
                sticksSeenCentered = false;
                // the RELATIVE boost baseline: the throttle the aircraft was
                // operating on when the trip fired (a headwind day flies on
                // a higher trim throttle and the recovery scales with it),
                // never below the cruise value
                preTripThrottleUs = MAX(rcCommand[THROTTLE],
                                        currentBatteryProfile->nav.fw.cruise_throttle);
            }
        } else {
            tripStartMs = 0;
        }
    } else {
        // Release when the tilt authority is visibly back: wings held
        // level-ish for a sustained window. Height is deliberately NOT a
        // release condition - that is the altitude floor's job, and a
        // T/W<1 autogyro settles in a slow descent at any sane recovery
        // attitude (measured: an arrest-the-sink condition pinned the
        // recovery active all the way to the ground). The box stays
        // armed; a renewed excursion simply trips it again.
        if (millis() - recoveryStartMs > ROTOR_GUARD_MIN_HOLD_MS
            && bankDeg < ROTOR_GUARD_RELEASE_BANK_DEG) {
            if (levelSinceMs == 0) {
                levelSinceMs = millis();
            } else if (millis() - levelSinceMs > 1500) {
                guardRecovery = false;
                tripStartMs = 0;
                levelSinceMs = 0;
            }
        } else {
            levelSinceMs = 0;
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

int16_t rotorGuardThrottleFloorUs(void)
{
    // Thrust (with the rotor LOADED) is the lever that brings the rotor
    // rpm - and with it the roll authority - back. The floor is RELATIVE:
    // the pre-trip operating throttle raised by rotor_guard_throttle_boost
    // percent, so a headwind trim point scales the recovery with it.
    // HEIGHT-GATED: below rotor_guard_min_height (baro above the start
    // altitude) no aggressive recovery power is flown - near the ground it
    // is wings-level + cushion only. 0 = no claim on the throttle.
    if (!guardRecovery) {
        return 0;
    }
    if (getEstimatedActualPosition(Z) < rotorGuardConfig()->minHeightM * 100.0f) {
        return 0;
    }
    // the FIRST moments of the recovery fly a BRIEF MAX-THROTTLE burst: with
    // the rotor LOADED that is the fastest way back to authority (thrust ->
    // speed -> inflow -> rpm; contract). After the burst the relative boost
    // floor holds until the release conditions clear the recovery.
    if (millis() - recoveryStartMs < ROTOR_GUARD_BURST_MS) {
        return getMaxThrottle();
    }
    return preTripThrottleUs
         + (preTripThrottleUs - 1000) * rotorGuardConfig()->throttleBoostPct / 100;
}

float rotorGuardRecoveryPitchDeg(void)
{
    // REAL-GYRO DOCTRINE: the rotor must stay LOADED - a nose-down push
    // unloads the disk and decays the rpm FASTER (power push-over). The
    // recovery pitch therefore defaults to 0 (hold the load, wings level,
    // let the max-throttle boost rebuild speed -> inflow -> rpm). The
    // parameter stays for airframes that need a small bias; keep it >= 0.
    // (The old -5 nose-down default came from a plant whose rpm model
    // coupled to airspeed only, without the load term.)
    if (ABS(attitude.values.roll) / 10.0f > ROTOR_GUARD_RELEASE_BANK_DEG + 10) {
        return (float)rotorGuardConfig()->recoveryPitchDeg;
    }
    return 0.0f;
}

#endif // USE_FW_AEROBATICS
