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

#include "config/config_reset.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "common/utils.h"
#include "common/maths.h"
#include "common/global_functions.h"
#include "common/logic_condition.h"

#include "io/vtx.h"
#include "drivers/vtx_common.h"

#ifdef USE_GLOBAL_FUNCTIONS

#include "common/axis.h"

PG_REGISTER_ARRAY_WITH_RESET_FN(globalFunction_t, MAX_GLOBAL_FUNCTIONS, globalFunctions, PG_GLOBAL_FUNCTIONS, 0);

EXTENDED_FASTRAM uint64_t globalFunctionsFlags = 0;
EXTENDED_FASTRAM globalFunctionState_t globalFunctionsStates[MAX_GLOBAL_FUNCTIONS];
EXTENDED_FASTRAM int globalFunctionValues[GLOBAL_FUNCTION_ACTION_LAST];

void pgResetFn_globalFunctions(globalFunction_t *instance)
{
    for (int i = 0; i < MAX_GLOBAL_FUNCTIONS; i++) {
        RESET_CONFIG(globalFunction_t, &instance[i],
            .enabled = 0,
            .conditionId = -1,
            .action = 0,
            .withValue = {
                .type = LOGIC_CONDITION_OPERAND_TYPE_VALUE,
                .value = 0
            },
            .flags = 0
        );
    }
}

void globalFunctionsProcess(int8_t functionId) {
    //Process only activated functions
    if (globalFunctions(functionId)->enabled) {

        const int conditionValue = logicConditionGetValue(globalFunctions(functionId)->conditionId);
        const int previousValue = globalFunctionsStates[functionId].active;

        globalFunctionsStates[functionId].active = (bool) conditionValue;
        globalFunctionsStates[functionId].value = logicConditionGetOperandValue(
            globalFunctions(functionId)->withValue.type,
            globalFunctions(functionId)->withValue.value
        );

        switch (globalFunctions(functionId)->action) {
            case GLOBAL_FUNCTION_ACTION_OVERRIDE_ARMING_SAFETY:
                if (conditionValue) {
                    GLOBAL_FUNCTION_FLAG_ENABLE(GLOBAL_FUNCTION_FLAG_OVERRIDE_ARMING_SAFETY);
                }
                break;
            case GLOBAL_FUNCTION_ACTION_OVERRIDE_THROTTLE_SCALE:
                if (conditionValue) {
                    globalFunctionValues[GLOBAL_FUNCTION_ACTION_OVERRIDE_THROTTLE_SCALE] = globalFunctionsStates[functionId].value;
                    GLOBAL_FUNCTION_FLAG_ENABLE(GLOBAL_FUNCTION_FLAG_OVERRIDE_THROTTLE_SCALE);
                }
                break;
            case GLOBAL_FUNCTION_ACTION_SWAP_ROLL_YAW:
                if (conditionValue) {
                    GLOBAL_FUNCTION_FLAG_ENABLE(GLOBAL_FUNCTION_FLAG_OVERRIDE_SWAP_ROLL_YAW);
                }
                break;
            case GLOBAL_FUNCTION_ACTION_SET_VTX_POWER_LEVEL:
                if (conditionValue && !previousValue) {
                    vtxDeviceCapability_t vtxDeviceCapability;
                    if (vtxCommonGetDeviceCapability(vtxCommonDevice(), &vtxDeviceCapability)) {
                        vtxSettingsConfigMutable()->power = constrain(globalFunctionsStates[functionId].value, VTX_SETTINGS_MIN_POWER, vtxDeviceCapability.powerCount);
                    }
                }
                break;
            case GLOBAL_FUNCTION_ACTION_INVERT_ROLL:
                if (conditionValue) {
                    GLOBAL_FUNCTION_FLAG_ENABLE(GLOBAL_FUNCTION_FLAG_OVERRIDE_INVERT_ROLL);
                }
                break;
            case GLOBAL_FUNCTION_ACTION_INVERT_PITCH:
                if (conditionValue) {
                    GLOBAL_FUNCTION_FLAG_ENABLE(GLOBAL_FUNCTION_FLAG_OVERRIDE_INVERT_PITCH);
                }
                break;
            case GLOBAL_FUNCTION_ACTION_INVERT_YAW:
                if (conditionValue) {
                    GLOBAL_FUNCTION_FLAG_ENABLE(GLOBAL_FUNCTION_FLAG_OVERRIDE_INVERT_YAW);
                }
                break;
            case GLOBAL_FUNCTION_ACTION_OVERRIDE_THROTTLE:
                if (conditionValue) {
                    globalFunctionValues[GLOBAL_FUNCTION_ACTION_OVERRIDE_THROTTLE] = globalFunctionsStates[functionId].value;
                    GLOBAL_FUNCTION_FLAG_ENABLE(GLOBAL_FUNCTION_FLAG_OVERRIDE_THROTTLE);
                }
                break;
        }
    }
}

void NOINLINE globalFunctionsUpdateTask(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);

    //Disable all flags
    globalFunctionsFlags = 0;

    for (uint8_t i = 0; i < MAX_GLOBAL_FUNCTIONS; i++) {
        globalFunctionsProcess(i);
    }
}

float NOINLINE getThrottleScale(float globalThrottleScale) {
    if (GLOBAL_FUNCTION_FLAG(GLOBAL_FUNCTION_FLAG_OVERRIDE_THROTTLE_SCALE)) {
        return constrainf(globalFunctionValues[GLOBAL_FUNCTION_ACTION_OVERRIDE_THROTTLE_SCALE] / 100.0f, 0.0f, 1.0f);
    } else {
        return globalThrottleScale;
    }
}

int16_t FAST_CODE getRcCommandOverride(int16_t command[], uint8_t axis) {
    int16_t outputValue = command[axis];

    if (GLOBAL_FUNCTION_FLAG(GLOBAL_FUNCTION_FLAG_OVERRIDE_SWAP_ROLL_YAW) && axis == FD_ROLL) {
        outputValue = command[FD_YAW];
    } else if (GLOBAL_FUNCTION_FLAG(GLOBAL_FUNCTION_FLAG_OVERRIDE_SWAP_ROLL_YAW) && axis == FD_YAW) {
        outputValue = command[FD_ROLL];
    }

    if (GLOBAL_FUNCTION_FLAG(GLOBAL_FUNCTION_FLAG_OVERRIDE_INVERT_ROLL) && axis == FD_ROLL) {
        outputValue *= -1;
    }
    if (GLOBAL_FUNCTION_FLAG(GLOBAL_FUNCTION_FLAG_OVERRIDE_INVERT_PITCH) && axis == FD_PITCH) {
        outputValue *= -1;
    }
    if (GLOBAL_FUNCTION_FLAG(GLOBAL_FUNCTION_FLAG_OVERRIDE_INVERT_YAW) && axis == FD_YAW) {
        outputValue *= -1;
    }

    return outputValue;
}

#endif
