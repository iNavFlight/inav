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
#include "common/time.h"

#define MAX_LOGIC_CONDITIONS 16

typedef enum {
    LOGIC_CONDITION_TRUE = 0,       // 0
    LOGIC_CONDITION_EQUAL,          // 1
    LOGIC_CONDITION_GREATER_THAN,   // 2
    LOGIC_CONDITION_LOWER_THAN,     // 3
    LOGIC_CONDITION_LOW,            // 4
    LOGIC_CONDITION_MID,            // 5
    LOGIC_CONDITION_HIGH,           // 6
    LOGIC_CONDITION_AND,            // 7
    LOGIC_CONDITION_OR,             // 8
    LOGIC_CONDITION_XOR,            // 9
    LOGIC_CONDITION_NAND,           // 10
    LOGIC_CONDITION_NOR,            // 11
    LOGIC_CONDITION_NOT,            // 12
    LOGIC_CONDITION_STICKY,         // 13
    LOGIC_CONDITION_ADD,            // 14
    LOGIC_CONDITION_SUB,            // 15
    LOGIC_CONDITION_MUL,            // 16
    LOGIC_CONDITION_DIV,            // 17
    LOGIC_CONDITION_GVAR_SET,       // 18
    LOGIC_CONDITION_GVAR_INC,       // 19
    LOGIC_CONDITION_GVAR_DEC,       // 20
    LOGIC_CONDITION_LAST
} logicOperation_e;

typedef enum logicOperandType_s {
    LOGIC_CONDITION_OPERAND_TYPE_VALUE = 0,
    LOGIC_CONDITION_OPERAND_TYPE_RC_CHANNEL,
    LOGIC_CONDITION_OPERAND_TYPE_FLIGHT,
    LOGIC_CONDITION_OPERAND_TYPE_FLIGHT_MODE,
    LOGIC_CONDITION_OPERAND_TYPE_LC,    // Result of different LC and LC operand
    LOGIC_CONDITION_OPERAND_TYPE_GVAR,  // Value from a global variable
    LOGIC_CONDITION_OPERAND_TYPE_LAST
} logicOperandType_e;

typedef enum {
    LOGIC_CONDITION_OPERAND_FLIGHT_ARM_TIMER = 0, // in s                   // 0
    LOGIC_CONDITION_OPERAND_FLIGHT_HOME_DISTANCE, //in m                    // 1
    LOGIC_CONDITION_OPERAND_FLIGHT_TRIP_DISTANCE, //in m                    // 2
    LOGIC_CONDITION_OPERAND_FLIGHT_RSSI,                                    // 3
    LOGIC_CONDITION_OPERAND_FLIGHT_VBAT, // Volt / 10                       // 4
    LOGIC_CONDITION_OPERAND_FLIGHT_CELL_VOLTAGE, // Volt / 10               // 5
    LOGIC_CONDITION_OPERAND_FLIGHT_CURRENT, // Amp / 100                    // 6
    LOGIC_CONDITION_OPERAND_FLIGHT_MAH_DRAWN, // mAh                        // 7
    LOGIC_CONDITION_OPERAND_FLIGHT_GPS_SATS,                                // 8
    LOGIC_CONDITION_OPERAND_FLIGHT_GROUD_SPEED, // cm/s                     // 9
    LOGIC_CONDITION_OPERAND_FLIGHT_3D_SPEED, // cm/s                        // 10
    LOGIC_CONDITION_OPERAND_FLIGHT_AIR_SPEED, // cm/s                       // 11
    LOGIC_CONDITION_OPERAND_FLIGHT_ALTITUDE, // cm                          // 12
    LOGIC_CONDITION_OPERAND_FLIGHT_VERTICAL_SPEED, // cm/s                  // 13
    LOGIC_CONDITION_OPERAND_FLIGHT_TROTTLE_POS, // %                        // 14
    LOGIC_CONDITION_OPERAND_FLIGHT_ATTITUDE_ROLL, // deg                    // 15
    LOGIC_CONDITION_OPERAND_FLIGHT_ATTITUDE_PITCH, // deg                   // 16
    LOGIC_CONDITION_OPERAND_FLIGHT_IS_ARMED, // 0/1                         // 17
    LOGIC_CONDITION_OPERAND_FLIGHT_IS_AUTOLAUNCH, // 0/1                    // 18
    LOGIC_CONDITION_OPERAND_FLIGHT_IS_ALTITUDE_CONTROL, // 0/1              // 19
    LOGIC_CONDITION_OPERAND_FLIGHT_IS_POSITION_CONTROL, // 0/1              // 20
    LOGIC_CONDITION_OPERAND_FLIGHT_IS_EMERGENCY_LANDING, // 0/1             // 21
    LOGIC_CONDITION_OPERAND_FLIGHT_IS_RTH, // 0/1                           // 22
    LOGIC_CONDITION_OPERAND_FLIGHT_IS_WP, // 0/1                            // 23
    LOGIC_CONDITION_OPERAND_FLIGHT_IS_LANDING, // 0/1                       // 24
    LOGIC_CONDITION_OPERAND_FLIGHT_IS_FAILSAFE, // 0/1                      // 25
    LOGIC_CONDITION_OPERAND_FLIGHT_STABILIZED_ROLL,                         // 26
    LOGIC_CONDITION_OPERAND_FLIGHT_STABILIZED_PITCH,                        // 27
    LOGIC_CONDITION_OPERAND_FLIGHT_STABILIZED_YAW,                          // 28
} logicFlightOperands_e;

typedef enum {
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_FAILSAFE,
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_MANUAL,
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_RTH,
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_POSHOLD,
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_CRUISE,
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_ALTHOLD,
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_ANGLE,
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_HORIZON,
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_AIR,
} logicFlightModeOperands_e;

typedef enum {
    LOGIC_CONDITION_FLAG_LATCH      = 1 << 0,
} logicConditionFlags_e;

typedef struct logicOperand_s {
    logicOperandType_e type;
    int32_t value;
} logicOperand_t;

typedef struct logicCondition_s {
    uint8_t enabled;
    int8_t activatorId;
    logicOperation_e operation;
    logicOperand_t operandA;
    logicOperand_t operandB;
    uint8_t flags;
} logicCondition_t;

PG_DECLARE_ARRAY(logicCondition_t, MAX_LOGIC_CONDITIONS, logicConditions);

typedef struct logicConditionState_s {
    int value;
    uint8_t flags;
} logicConditionState_t;

void logicConditionProcess(uint8_t i);

int logicConditionGetOperandValue(logicOperandType_e type, int operand);

int logicConditionGetValue(int8_t conditionId);
void logicConditionUpdateTask(timeUs_t currentTimeUs);
void logicConditionReset(void);
