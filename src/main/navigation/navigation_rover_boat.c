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

#include "platform.h"

FILE_COMPILE_FOR_SIZE

#ifdef USE_NAV

#include "common/utils.h"
#include "fc/rc_controls.h"
#include "flight/mixer.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

void applyRoverBoatPitchRollThrottleController(navigationFSMStateFlags_t navStateFlags, timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);
    rcCommand[ROLL] = 0;
    rcCommand[PITCH] = 0;

    if (navStateFlags & NAV_CTL_POS) {

        // if (isYawAdjustmentValid) {
        rcCommand[YAW] = posControl.rcAdjustment[YAW];
        // }

        // const uint16_t correctedThrottleValue = constrain(navConfig()->fw.cruise_throttle, navConfig()->fw.min_throttle, navConfig()->fw.max_throttle);
        rcCommand[THROTTLE] = constrain(navConfig()->fw.cruise_throttle, motorConfig()->mincommand, motorConfig()->maxthrottle);
    }
}

void applyRoverBoatNavigationController(navigationFSMStateFlags_t navStateFlags, timeUs_t currentTimeUs)
{
    if (navStateFlags & NAV_CTL_EMERG) {
        rcCommand[ROLL] = 0;
        rcCommand[PITCH] = 0;
        rcCommand[YAW] = 0;
        rcCommand[THROTTLE] = failsafeConfig()->failsafe_throttle;
    } else if (navStateFlags & NAV_CTL_POS) {
        applyFixedWingPositionController(currentTimeUs);
        applyRoverBoatPitchRollThrottleController(navStateFlags, currentTimeUs);
    }
}

#endif