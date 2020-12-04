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

#ifdef USE_PROGRAMMING_FRAMEWORK

#include "common/utils.h"
#include "config/config_reset.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"
#include "navigation/navigation_private.h"

#include "programming/pid.h"
#include "programming/logic_condition.h"

EXTENDED_FASTRAM programmingPidState_t programmingPidState[MAX_PROGRAMMING_PID_COUNT];
static bool pidsInitiated = false;

PG_REGISTER_ARRAY_WITH_RESET_FN(programmingPid_t, MAX_PROGRAMMING_PID_COUNT, programmingPids, PG_PROGRAMMING_PID, 1);

void pgResetFn_programmingPids(programmingPid_t *instance)
{
    for (int i = 0; i < MAX_PROGRAMMING_PID_COUNT; i++) {
        RESET_CONFIG(programmingPid_t, &instance[i],
            .enabled = 0,
            .setpoint = {
                .type = LOGIC_CONDITION_OPERAND_TYPE_VALUE,
                .value = 0
            },
            .measurement = {
                .type = LOGIC_CONDITION_OPERAND_TYPE_VALUE,
                .value = 0
            },
            .gains = {
                .P = 0,
                .I = 0,
                .D = 0,
                .FF = 0,
            }
        );
    }
}

void programmingPidUpdateTask(timeUs_t currentTimeUs)
{
    static timeUs_t previousUpdateTimeUs;
    const float dT = US2S(currentTimeUs - previousUpdateTimeUs);
    
    if (!pidsInitiated) {
        programmingPidInit();
        pidsInitiated = true;
    }

    for (uint8_t i = 0; i < MAX_PROGRAMMING_PID_COUNT; i++) {
        if (programmingPids(i)->enabled) {
            const int setpoint = logicConditionGetOperandValue(programmingPids(i)->setpoint.type, programmingPids(i)->setpoint.value);
            const int measurement = logicConditionGetOperandValue(programmingPids(i)->measurement.type, programmingPids(i)->measurement.value);

            programmingPidState[i].output = navPidApply2(
                &programmingPidState[i].controller,
                setpoint,
                measurement,
                dT,
                -1000,
                1000,
                PID_LIMIT_INTEGRATOR
            );

        }
    }
}

void programmingPidInit(void)
{
    for (uint8_t i = 0; i < MAX_PROGRAMMING_PID_COUNT; i++) {
        navPidInit(
            &programmingPidState[i].controller,
            programmingPids(i)->gains.P / 1000.0f,
            programmingPids(i)->gains.I / 1000.0f,
            programmingPids(i)->gains.D / 1000.0f,
            programmingPids(i)->gains.FF / 1000.0f,
            5.0f
        );
    }
}

int programmingPidGetOutput(uint8_t i) {
    return programmingPidState[constrain(i, 0, MAX_PROGRAMMING_PID_COUNT)].output;
}

void programmingPidReset(void)
{
    for (uint8_t i = 0; i < MAX_PROGRAMMING_PID_COUNT; i++) {
        navPidReset(&programmingPidState[i].controller);
    }
}

#endif