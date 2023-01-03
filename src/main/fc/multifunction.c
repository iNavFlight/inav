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

#include "fc/fc_core.h"
#include "fc/multifunction.h"
#include "fc/rc_modes.h"

#include "io/osd.h"
#include "navigation/navigation.h"

static void multiFunctionApply(multi_function_e selectedItem)
{
    switch (selectedItem) {
    case MULTI_FUNC_NONE:
        return;
    case MULTI_FUNC_1:
        resetOsdWarningMask();
        break;
    case MULTI_FUNC_2:
        emergencyArmingUpdate(true, true);
        break;
    case MULTI_FUNC_COUNT:
        break;
    }
}

bool multiFunctionSelection(multi_function_e * returnItem)
{
    static timeMs_t startTimer;
    static timeMs_t selectTimer;
    static int8_t selectedItem = 0;
    static bool toggle = true;
    const timeMs_t currentTime = millis();

    if (IS_RC_MODE_ACTIVE(BOXMULTIFUNCTION)) {
        if (selectTimer) {
            if (currentTime - selectTimer > 3000) {
                *returnItem = selectedItem;
                multiFunctionApply(selectedItem);
                selectTimer = 0;
                selectedItem = 0;
                return true;
            }
        } else if (toggle) {
            selectedItem++;
            selectedItem = selectedItem == MULTI_FUNC_COUNT ? 1 : selectedItem;
            selectTimer = currentTime;
        }
        startTimer = currentTime;
        toggle = false;
    } else if (startTimer) {
        selectTimer = 0;
        if (currentTime - startTimer > 2000) {
            startTimer = 0;
            selectedItem = 0;
        }
        toggle = true;
    }

    *returnItem = selectedItem;
    return false;
}
