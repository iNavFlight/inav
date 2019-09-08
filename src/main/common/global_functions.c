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
#include "common/global_functions.h"
#include "common/logic_condition.h"

PG_REGISTER_ARRAY(globalFunction_t, MAX_GLOBAL_FUNCTIONS, globalFunctions, PG_GLOBAL_FUNCTIONS, 0);

EXTENDED_FASTRAM uint64_t globalFunctionsFlags = 0;
EXTENDED_FASTRAM globalFunctionState_t globalFunctionsStates[MAX_GLOBAL_FUNCTIONS];

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

        globalFunctionsStates[functionId].active = (bool) conditionValue;
        globalFunctionsStates[functionId].value = logicConditionGetOperandValue(
            globalFunctions(functionId)->withValue.type,
            globalFunctions(functionId)->withValue.value
        );

        switch (globalFunctions(functionId)->action) {
            case GLOBAL_FUNCTION_ACTION_OVERRIDE_ARMING_SAFETY:
                if (conditionValue) {
                    GLOBAL_FUNCTION_FLAG_ENABLE(GLOBAL_FUNCTION_FLAG_OVERRIDE_ARMING_SAFETY);
                } else {
                    GLOBAL_FUNCTION_FLAG_DISABLE(GLOBAL_FUNCTION_FLAG_OVERRIDE_ARMING_SAFETY);
                }
                break;
        }
    }
}

void globalFunctionsUpdateTask(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);
    for (uint8_t i = 0; i < MAX_GLOBAL_FUNCTIONS; i++) {
        globalFunctionsProcess(i);
    }
}