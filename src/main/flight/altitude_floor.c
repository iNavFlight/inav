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
#include "common/vector.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/altitude_floor.h"
#include "flight/imu.h"

#include "navigation/navigation.h"

#include "rx/rx.h"

PG_REGISTER_WITH_RESET_TEMPLATE(altitudeFloorConfig_t, altitudeFloorConfig, PG_ALTITUDE_FLOOR_CONFIG, 0);

PG_RESET_TEMPLATE(altitudeFloorConfig_t, altitudeFloorConfig,
    .floorAltitude = SETTING_ALT_FLOOR_ALTITUDE_DEFAULT,
    .floorMargin = SETTING_ALT_FLOOR_MARGIN_DEFAULT,
    .floorClimbPitch = SETTING_ALT_FLOOR_CLIMB_PITCH_DEFAULT,
);

static bool floorArmed = false;      // climbed above floor + margin once
static bool floorRecovery = false;
static bool floorOrbit = false;      // recovery phase 2: circle at the floor
static bool orbitViaNav = false;     // orbit flown by the nav loiter
static bool sticksSeenCentered = false;
static fpVector3_t breachPos;        // orbit anchor: where the floor broke

// Orbit once the climb is done: circle the BREACH POINT and wait. Level
// flight puts the antenna back at the sky, so GPS is healthy again
// (Daniel: no-GPS is only the degradation, not the design case) - with a
// healthy position estimate the aircraft is handed to the REAL fixed-wing
// loiter (forced position hold on the breach point, nav_fw_loiter_radius,
// wind-corrected; Daniel: use the loitering machinery). Without one it
// degrades to a constant-bank circle flown by the hold. Only a stick
// takeover or switching the box away releases either.
#define FLOOR_ORBIT_BANK_DEG          22.0f
// P-gain of the orbit altitude hold: 1 deg pitch per metre of error,
// bounded well below the climb pitch - the orbit holds, it does not zoom
#define FLOOR_ORBIT_PITCH_P_DEG_PER_M 1.0f
#define FLOOR_ORBIT_PITCH_MIN_DEG     (-5.0f)
#define FLOOR_ORBIT_PITCH_MAX_DEG     10.0f

void altitudeFloorUpdate(void)
{
    if (!IS_RC_MODE_ACTIVE(BOXALTFLOOR) || !ARMING_FLAG(ARMED) || !STATE(AIRPLANE)
        || !navIsAltitudeEstimateTrusted()) {
        floorArmed = false;
        floorRecovery = false;
        floorOrbit = false;
        if (orbitViaNav) {
            navAbortFloorOrbit();
            orbitViaNav = false;
        }
        return;
    }

    const float z = getEstimatedActualPosition(Z);          // cm above home
    const float vz = getEstimatedActualVelocity(Z);         // cm/s
    const float floorCm = altitudeFloorConfig()->floorAltitude * 100.0f;
    const float marginCm = altitudeFloorConfig()->floorMargin * 100.0f;

    if (!floorArmed) {
        // Arm only after climbing above floor + margin once, so switching
        // the box on while on the ground (or arming below the floor) never
        // grabs the aircraft during takeoff
        floorArmed = z > (floorCm + marginCm);
        return;
    }

    if (!floorRecovery) {
        // Engage when the aircraft BREAKS THROUGH the floor, sinking - no
        // prediction. A piloted trajectory is not predictable (a loop
        // downline at 30 m/s "predicts" a 90 m crash and pulls out in 15;
        // measured as the floor silently co-flying every fast loop under
        // the old 3 s lookahead). The line is the contract: above it the
        // sky belongs to the pilot, crossing it downward triggers the
        // recovery, and the height below the line is the recovery budget
        // the user chooses with alt_floor_altitude.
        if (vz < 0.0f && z < floorCm) {
            floorRecovery = true;
            floorOrbit = false;
            sticksSeenCentered = false;
            // the breach point becomes the orbit anchor; the loiter
            // altitude is the floor + margin the climb ends at
            breachPos.x = getEstimatedActualPosition(X);
            breachPos.y = getEstimatedActualPosition(Y);
            breachPos.z = floorCm + marginCm;
        }
    } else {
        // Climb done (back above floor + margin, climbing): do NOT hand
        // back - transition to the ORBIT. The aircraft circles at the
        // floor around the breach point and WAITS; after the shock the
        // pilot gets as many seconds as they need to collect themselves.
        // With a healthy position estimate the REAL fixed-wing loiter
        // flies it (forced poshold on the breach point - immune to the
        // post-figure heading estimate, it flies GPS vectors); otherwise
        // the hold flies a constant-bank circle as the degraded form.
        if (!floorOrbit && z > (floorCm + marginCm) && vz > 0.0f) {
            floorOrbit = true;
            if (navigationPositionEstimateIsHealthy()) {
                navActivateFloorOrbitAt(&breachPos);
                orbitViaNav = true;
            }
        }
        if (orbitViaNav) {
            // the poshold FSM re-anchors on current position at init -
            // keep the breach point asserted every cycle
            navAssertFloorOrbitTarget(&breachPos);
        }
        // The ONLY releases: the pilot takes over (sticks must return to
        // center ONCE first - the panic-held down-elevator from the dive
        // is not a takeover - then a fresh roll/pitch deflection hands
        // control back immediately; yaw stays steering), or the box goes
        // off (guard clause above). RAW receiver sticks, not rcCommand:
        // the fixed-wing nav loiter WRITES rcCommand to fly the orbit,
        // and reading the mix made the loiter release itself (measured).
        const bool deflected =
            ABS(rxGetChannelValue(ROLL) - PWM_RANGE_MIDDLE) > rcControlsConfig()->deadband
         || ABS(rxGetChannelValue(PITCH) - PWM_RANGE_MIDDLE) > rcControlsConfig()->deadband;
        if (!sticksSeenCentered) {
            sticksSeenCentered = !deflected;
        } else if (deflected) {
            floorRecovery = false;
            floorOrbit = false;
            if (orbitViaNav) {
                navAbortFloorOrbit();
                orbitViaNav = false;
            }
        }
    }
}

bool altitudeFloorRecoveryActive(void)
{
    return floorRecovery;
}

bool altitudeFloorArmed(void)
{
    return floorArmed;
}

float altitudeFloorRecoveryPitchDeg(void)
{
    if (floorOrbit) {
        // orbit altitude hold: small proportional pitch about the orbit
        // altitude (floor + margin), never the full climb pitch
        const float targetCm = (altitudeFloorConfig()->floorAltitude
                                + altitudeFloorConfig()->floorMargin) * 100.0f;
        const float errM = (targetCm - getEstimatedActualPosition(Z)) / 100.0f;
        return constrainf(errM * FLOOR_ORBIT_PITCH_P_DEG_PER_M,
                          FLOOR_ORBIT_PITCH_MIN_DEG, FLOOR_ORBIT_PITCH_MAX_DEG);
    }
    return (float)altitudeFloorConfig()->floorClimbPitch;
}

float altitudeFloorRecoveryRollDeg(void)
{
    if (!floorOrbit || orbitViaNav) {
        return 0.0f;   // climb flies wings level; the nav loiter flies itself
    }
    // Degraded orbit (no healthy position estimate): a constant-bank
    // circle that drifts with the wind but needs NO heading estimate -
    // deliberately, because the post-figure heading can sit on the
    // antipode (measured: 184 deg yaw error after a flat spin, stable
    // for 60 s - mag and COG corrections vanish at sin(180)). Any law
    // that steers by heading dies there; the blind circle does not.
    return FLOOR_ORBIT_BANK_DEG;
}

bool altitudeFloorOrbitActive(void)
{
    return floorOrbit;
}

bool altitudeFloorOrbitViaNav(void)
{
    return orbitViaNav;
}

// metres above (positive) or below (negative) the floor line - the
// telemetry/OSD readout of how much sky is left before the net
float altitudeFloorDistanceM(void)
{
    const float floorCm = altitudeFloorConfig()->floorAltitude * 100.0f;
    return (getEstimatedActualPosition(Z) - floorCm) / 100.0f;
}

#endif // USE_ORIENTATION_HOLD
