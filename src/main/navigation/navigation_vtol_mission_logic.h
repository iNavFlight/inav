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

#include <stdbool.h>

typedef enum {
    NAV_MISSION_VTOL_PRECONDITION_READY = 0,
    NAV_MISSION_VTOL_PRECONDITION_WAIT,
    NAV_MISSION_VTOL_PRECONDITION_REJECT,
} navMissionVtolTransitionPrecondition_e;

static inline navMissionVtolTransitionPrecondition_e navMissionVtolTransitionPreconditionDisposition(
    const bool armed,
    const bool failsafeActive,
    const bool sensorsCalibrating,
    const bool positionUsable,
    const bool headingUsable,
    const bool mixerProfileModeConfigured,
    const bool hotSwitchAvailable,
    const bool mixerAtActive)
{
    if (!armed || failsafeActive) {
        return NAV_MISSION_VTOL_PRECONDITION_REJECT;
    }

    if (sensorsCalibrating || !positionUsable || !headingUsable || mixerAtActive) {
        return NAV_MISSION_VTOL_PRECONDITION_WAIT;
    }

    if (!mixerProfileModeConfigured || !hotSwitchAvailable) {
        return NAV_MISSION_VTOL_PRECONDITION_REJECT;
    }

    return NAV_MISSION_VTOL_PRECONDITION_READY;
}
