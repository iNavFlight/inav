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
#include "build/debug.h"
#include "drivers/time.h"

#ifdef USE_MULTI_FUNCTIONS

#include "fc/fc_core.h"
#include "fc/multifunction.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"

#include "io/osd.h"
#include "navigation/navigation.h"

multi_function_e selectedItem = MULTI_FUNC_NONE;
uint8_t multiFunctionFlags;

static void multiFunctionApply(multi_function_e selectedItem)
{
    switch (selectedItem) {
    case MULTI_FUNC_NONE:
        break;
    case MULTI_FUNC_1:  // control manual emergency landing
        checkManualEmergencyLandingControl(ARMING_FLAG(ARMED));
        break;
    case MULTI_FUNC_2:  // toggle Safehome suspend
#if defined(USE_SAFE_HOME)
        if (navConfig()->general.flags.safehome_usage_mode != SAFEHOME_USAGE_OFF) {
            MULTI_FUNC_FLAG(MF_SUSPEND_SAFEHOMES) ? MULTI_FUNC_FLAG_DISABLE(MF_SUSPEND_SAFEHOMES) : MULTI_FUNC_FLAG_ENABLE(MF_SUSPEND_SAFEHOMES);
        }
#endif
        break;
    case MULTI_FUNC_3:  // toggle RTH Trackback suspend
        if (navConfig()->general.flags.rth_trackback_mode != RTH_TRACKBACK_OFF) {
            MULTI_FUNC_FLAG(MF_SUSPEND_TRACKBACK) ? MULTI_FUNC_FLAG_DISABLE(MF_SUSPEND_TRACKBACK) : MULTI_FUNC_FLAG_ENABLE(MF_SUSPEND_TRACKBACK);
        }
        break;
    case MULTI_FUNC_4:
#ifdef USE_DSHOT
        if (!ARMING_FLAG(ARMED) && STATE(MULTIROTOR)) {    // toggle Turtle mode
            MULTI_FUNC_FLAG(MF_TURTLE_MODE) ? MULTI_FUNC_FLAG_DISABLE(MF_TURTLE_MODE) : MULTI_FUNC_FLAG_ENABLE(MF_TURTLE_MODE);
        }
#endif
        break;
    case MULTI_FUNC_5:  // emergency ARM
        if (!ARMING_FLAG(ARMED)) {
            emergencyArmingUpdate(true, true);
        }
    case MULTI_FUNC_END:
        break;
    }
}

void setMultifunctionSelection(multi_function_e item)
{
    selectedItem = item == MULTI_FUNC_END ? MULTI_FUNC_1 : item;
}

multi_function_e multiFunctionSelection(void)
{
    static timeMs_t functionTimer;
    const timeMs_t currentTime = millis();
    static uint8_t functionTracker = 0;

    if (IS_RC_MODE_ACTIVE(BOXMULTIFUNCTION)) {
        if (!functionTimer) {    // initiate function on first BOXMULTIFUNCTION activation
            functionTimer = currentTime;
            selectedItem = MULTI_FUNC_1;
        } else if (functionTracker && selectedItem != MULTI_FUNC_END) {
            functionTracker = 2;
            if (currentTime - functionTimer > 3000) {    // 3s BOXMULTIFUNCTION activation to trigger selected function
                multiFunctionApply(selectedItem);
                selectedItem = MULTI_FUNC_END;
            }
        }
    } else if (functionTimer) {
        if (!functionTracker) {
            functionTimer = currentTime;
        } else if (functionTracker == 2 || selectedItem == MULTI_FUNC_NONE) {
            // cancel and reset function after second BOXMULTIFUNCTION deactivation or if no functions available
            functionTimer = 0;
            functionTracker = 0;
            return selectedItem = MULTI_FUNC_NONE;
        }

        if (currentTime - functionTimer > 1500) {    // display available functions on 1.5s rolling cycle
            setMultifunctionSelection(++selectedItem);
            functionTimer = currentTime;
        }

        functionTracker = 1;
    }

    return selectedItem;
}
#endif
