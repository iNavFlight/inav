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
#pragma once

#include "config/parameter_group.h"
#include "common/logic_condition.h"

#define MAX_GLOBAL_FUNCTIONS 8

typedef enum {
    GLOBAL_FUNCTION_ACTION_OVERRIDE_ARMING_SAFETY = 0,
    GLOBAL_FUNCTION_ACTION_OVERRIDE_THROTTLE_SCALE,
    GLOBAL_FUNCTION_ACTION_SWAP_ROLL_YAW,
    GLOBAL_FUNCTION_ACTION_SET_VTX_POWER_LEVEL,
    GLOBAL_FUNCTION_ACTION_INVERT_ROLL,
    GLOBAL_FUNCTION_ACTION_INVERT_PITCH,
    GLOBAL_FUNCTION_ACTION_INVERT_YAW,
    GLOBAL_FUNCTION_ACTION_OVERRIDE_THROTTLE,
    GLOBAL_FUNCTION_ACTION_LAST
} globalFunctionActions_e;

typedef enum {
    GLOBAL_FUNCTION_FLAG_OVERRIDE_ARMING_SAFETY = (1 << 0),
    GLOBAL_FUNCTION_FLAG_OVERRIDE_THROTTLE_SCALE = (1 << 1),
    GLOBAL_FUNCTION_FLAG_OVERRIDE_SWAP_ROLL_YAW = (1 << 2),
    GLOBAL_FUNCTION_FLAG_OVERRIDE_INVERT_ROLL = (1 << 3),
    GLOBAL_FUNCTION_FLAG_OVERRIDE_INVERT_PITCH = (1 << 4),
    GLOBAL_FUNCTION_FLAG_OVERRIDE_INVERT_YAW = (1 << 5),
    GLOBAL_FUNCTION_FLAG_OVERRIDE_THROTTLE = (1 << 6),
} globalFunctionFlags_t;

typedef struct globalFunction_s {
    uint8_t enabled;
    int8_t conditionId;
    uint8_t action;
    logicOperand_t withValue;
    uint8_t flags;
} globalFunction_t;

typedef struct globalFunctionState_s {
    uint8_t active;
    int value;
    uint8_t flags;
} globalFunctionState_t;

extern uint64_t globalFunctionsFlags;

#define GLOBAL_FUNCTION_FLAG_DISABLE(mask) (globalFunctionsFlags &= ~(mask))
#define GLOBAL_FUNCTION_FLAG_ENABLE(mask) (globalFunctionsFlags |= (mask))
#define GLOBAL_FUNCTION_FLAG(mask) (globalFunctionsFlags & (mask))

PG_DECLARE_ARRAY(globalFunction_t, MAX_GLOBAL_FUNCTIONS, globalFunctions);
extern int globalFunctionValues[GLOBAL_FUNCTION_ACTION_LAST];

void globalFunctionsUpdateTask(timeUs_t currentTimeUs);
float getThrottleScale(float globalThrottleScale);
int16_t getRcCommandOverride(int16_t command[], uint8_t axis);