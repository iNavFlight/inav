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

#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/altitude_floor.h"

#include "navigation/navigation.h"

PG_REGISTER_WITH_RESET_TEMPLATE(altitudeFloorConfig_t, altitudeFloorConfig, PG_ALTITUDE_FLOOR_CONFIG, 0);

PG_RESET_TEMPLATE(altitudeFloorConfig_t, altitudeFloorConfig,
    .floorAltitude = SETTING_ALT_FLOOR_ALTITUDE_DEFAULT,
    .floorMargin = SETTING_ALT_FLOOR_MARGIN_DEFAULT,
    .floorClimbPitch = SETTING_ALT_FLOOR_CLIMB_PITCH_DEFAULT,
);

static bool floorArmed = false;      // climbed above floor + margin once
static bool floorRecovery = false;
static bool sticksSeenCentered = false;

void altitudeFloorUpdate(void)
{
    if (!IS_RC_MODE_ACTIVE(BOXALTFLOOR) || !ARMING_FLAG(ARMED) || !STATE(AIRPLANE)
        || !navIsAltitudeEstimateTrusted()) {
        floorArmed = false;
        floorRecovery = false;
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
            sticksSeenCentered = false;
        }
    } else {
        // Release when back above floor + margin and climbing - the climb
        // ends at the margin, it does not run away upward
        if (z > (floorCm + marginCm) && vz > 0.0f) {
            floorRecovery = false;
        }
        // ... or when the pilot takes over after the catch: the sticks must
        // return to center ONCE first (the panic-held down-elevator from
        // the dive is not a takeover), a fresh roll/pitch deflection then
        // hands control back immediately. Yaw stays steering, not release.
        const bool deflected = ABS(rcCommand[ROLL]) > rcControlsConfig()->deadband
                            || ABS(rcCommand[PITCH]) > rcControlsConfig()->deadband;
        if (!sticksSeenCentered) {
            sticksSeenCentered = !deflected;
        } else if (deflected) {
            floorRecovery = false;
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
    return (float)altitudeFloorConfig()->floorClimbPitch;
}

#endif // USE_ORIENTATION_HOLD
