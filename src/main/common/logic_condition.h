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

typedef enum {
    LOGIC_CONDITION_TRUE = 0,
    LOGIC_CONDITION_EQUAL,
    LOGIC_CONDITION_GREATER_THAN,
    LOGIC_CONDITION_LOWER_THAN,
    LOGIC_CONDITION_LOW,
    LOGIC_CONDITION_MID,
    LOGIC_CONDITION_HIGH,
    LOGIC_CONDITION_LAST
} logicOperation_e;

typedef enum logicOperandType_s {
    LOGIC_CONDITION_OPERAND_TYPE_VALUE = 0,
    LOGIC_CONDITION_OPERAND_TYPE_RC_CHANNEL,
    LOGIC_CONDITION_OPERAND_TYPE_LAST
} logicOperandType_e;

typedef struct logicOperand_s {
    logicOperandType_e type;
    int value;
} logicOperand_t;

typedef struct logicCondition_s {
    logicOperation_e operation;
    logicOperand_t operandA;
    logicOperand_t operandB;
} logicCondition_t;

int logicConditionCompute(
    logicOperation_e operation,
    int operandA,
    int operandB
);

int logicConditionGetOperandValue(logicOperandType_e type, int operand);