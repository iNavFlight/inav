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

static bool isYawAdjustmentValid = false;

void applyRoverBoatPositionController(timeUs_t currentTimeUs)
{
    static timeUs_t previousTimePositionUpdate;         // Occurs @ GPS update rate
    static timeUs_t previousTimeUpdate;                 // Occurs @ looptime rate

    const timeDelta_t deltaMicros = currentTimeUs - previousTimeUpdate;
    previousTimeUpdate = currentTimeUs;

    // If last position update was too long in the past - ignore it (likely restarting altitude controller)
    if (deltaMicros > HZ2US(MIN_POSITION_UPDATE_RATE_HZ)) {
        previousTimeUpdate = currentTimeUs;
        previousTimePositionUpdate = currentTimeUs;
        resetFixedWingPositionController();
        return;
    }

    // Apply controller only if position source is valid. In absence of valid pos sensor (GPS loss), we'd stick in forced ANGLE mode
    if ((posControl.flags.estPosStatus >= EST_USABLE)) {
        // If we have new position - update velocity and acceleration controllers
        if (posControl.flags.horizontalPositionDataNew) {
            const timeDelta_t deltaMicrosPositionUpdate = currentTimeUs - previousTimePositionUpdate;
            previousTimePositionUpdate = currentTimeUs;

            if (deltaMicrosPositionUpdate < HZ2US(MIN_POSITION_UPDATE_RATE_HZ)) {
                // Calculate virtual position target at a distance of forwardVelocity * HZ2S(POSITION_TARGET_UPDATE_RATE_HZ)
                // Account for pilot's roll input (move position target left/right at max of max_manual_speed)
                // POSITION_TARGET_UPDATE_RATE_HZ should be chosen keeping in mind that position target shouldn't be reached until next pos update occurs
                // FIXME: verify the above
                // calculateVirtualPositionTarget_FW(HZ2S(MIN_POSITION_UPDATE_RATE_HZ) * 2);

                // updatePositionHeadingController_FW(currentTimeUs, deltaMicrosPositionUpdate);

                //FIXME build a simple 2D position controller 
            }
            else {
                resetFixedWingPositionController();
            }

            // Indicate that information is no longer usable
            posControl.flags.horizontalPositionDataConsumed = 1;
        }

        isYawAdjustmentValid = true;
    }
    else {
        isYawAdjustmentValid = false;
    }
}

void applyRoverBoatPitchRollThrottleController(navigationFSMStateFlags_t navStateFlags, timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);
    rcCommand[ROLL] = 0;
    rcCommand[PITCH] = 0;

    if (navStateFlags & NAV_CTL_POS) {

        if (isYawAdjustmentValid) {
            rcCommand[YAW] = posControl.rcAdjustment[YAW];
        }

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