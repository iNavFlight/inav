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

#include <stdbool.h>

#include "config/config_reset.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "common/logic_condition.h"
#include "common/utils.h"
#include "rx/rx.h"
#include "maths.h"
#include "fc/fc_core.h"
#include "fc/rc_controls.h"
#include "fc/runtime_config.h"
#include "navigation/navigation.h"
#include "sensors/battery.h"
#include "sensors/pitotmeter.h"
#include "flight/imu.h"

PG_REGISTER_ARRAY(logicCondition_t, MAX_LOGIC_CONDITIONS, logicConditions, PG_LOGIC_CONDITIONS, 0);

logicConditionState_t logicConditionStates[MAX_LOGIC_CONDITIONS];

static int logicConditionCompute(
    int currentVaue,
    logicOperation_e operation,
    int operandA,
    int operandB
) {
    switch (operation) {

        case LOGIC_CONDITION_TRUE:
            return true;
            break;

        case LOGIC_CONDITION_EQUAL:
            return operandA == operandB;
            break;

        case LOGIC_CONDITION_GREATER_THAN:
            return operandA > operandB;
            break;

        case LOGIC_CONDITION_LOWER_THAN:
            return operandA < operandB;
            break;

        case LOGIC_CONDITION_LOW:
            return operandA < 1333;
            break;

        case LOGIC_CONDITION_MID:
            return operandA >= 1333 && operandA <= 1666;
            break;

        case LOGIC_CONDITION_HIGH:
            return operandA > 1666;
            break;

        case LOGIC_CONDITION_AND:
            return (operandA && operandB);
            break;

        case LOGIC_CONDITION_OR:
            return (operandA || operandB);
            break;

        case LOGIC_CONDITION_XOR:
            return (operandA != operandB);
            break;

        case LOGIC_CONDITION_NAND:
            return !(operandA && operandB);
            break;

        case LOGIC_CONDITION_NOR:
            return !(operandA || operandB);
            break; 

        case LOGIC_CONDITION_NOT:
            return !operandA;
            break;

        case LOGIC_CONDITION_STICKY:
            // Operand A is activation operator
            if (operandA) {
                return true;
            }
            //Operand B is deactivation operator
            if (operandB) {
                return false;
            }

            //When both operands are not met, keep current value 
            return currentVaue;
            break;

        default:
            return false;
            break; 
    }
}

void logicConditionProcess(uint8_t i) {

    if (logicConditions(i)->enabled) {
        
        /*
         * Process condition only when latch flag is not set
         * Latched LCs can only go from OFF to ON, not the other way
         */
        if (!(logicConditionStates[i].flags & LOGIC_CONDITION_FLAG_LATCH)) {
            const int operandAValue = logicConditionGetOperandValue(logicConditions(i)->operandA.type, logicConditions(i)->operandA.value);
            const int operandBValue = logicConditionGetOperandValue(logicConditions(i)->operandB.type, logicConditions(i)->operandB.value);
            const int newValue = logicConditionCompute(
                logicConditionStates[i].value, 
                logicConditions(i)->operation, 
                operandAValue, 
                operandBValue
            );
        
            logicConditionStates[i].value = newValue;

            /*
             * if value evaluates as true, put a latch on logic condition
             */
            if (logicConditions(i)->flags & LOGIC_CONDITION_FLAG_LATCH && newValue) {
                logicConditionStates[i].flags |= LOGIC_CONDITION_FLAG_LATCH;
            }
        }
    } else {
        logicConditionStates[i].value = false;
    }
}

static int logicConditionGetFlightOperandValue(int operand) {

    switch (operand) {

        case LOGIC_CONDITION_OPERAND_FLIGHT_ARM_TIMER: // in s
            return constrain((uint32_t)getFlightTime(), 0, 32767);
            break;
        
        case LOGIC_CONDITION_OPERAND_FLIGHT_HOME_DISTANCE: //in m
            return constrain(GPS_distanceToHome, 0, 32767);
            break;
        
        case LOGIC_CONDITION_OPERAND_FLIGHT_TRIP_DISTANCE: //in m
            return constrain(getTotalTravelDistance() / 100, 0, 32767);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_RSSI:
            return constrain(getRSSI() * 100 / RSSI_MAX_VALUE, 0, 99);
            break;
        
        case LOGIC_CONDITION_OPERAND_FLIGHT_VBAT: // V / 10
            return getBatteryVoltage();
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_CELL_VOLTAGE: // V / 10
            return getBatteryAverageCellVoltage();
            break;
        case LOGIC_CONDITION_OPERAND_FLIGHT_CURRENT: // Amp / 100
            return getAmperage();
            break;
        
        case LOGIC_CONDITION_OPERAND_FLIGHT_MAH_DRAWN: // mAh
            return getMAhDrawn();
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_GPS_SATS:
            return gpsSol.numSat;
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_GROUD_SPEED: // cm/s
            return gpsSol.groundSpeed;
            break;

        //FIXME align with osdGet3DSpeed
        case LOGIC_CONDITION_OPERAND_FLIGHT_3D_SPEED: // cm/s
            return (int) sqrtf(sq(gpsSol.groundSpeed) + sq((int)getEstimatedActualVelocity(Z)));
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_AIR_SPEED: // cm/s
        #ifdef USE_PITOT
            return constrain(pitot.airSpeed, 0, 32767);
        #else
            return false;
        #endif
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_ALTITUDE: // cm
            return constrain(getEstimatedActualPosition(Z), -32678, 32767);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_VERTICAL_SPEED: // cm/s
            return constrain(getEstimatedActualVelocity(Z), 0, 32767);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_TROTTLE_POS: // %
            return (constrain(rcCommand[THROTTLE], PWM_RANGE_MIN, PWM_RANGE_MAX) - PWM_RANGE_MIN) * 100 / (PWM_RANGE_MAX - PWM_RANGE_MIN);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_ATTITUDE_ROLL: // deg
            return constrain(attitude.values.roll / 10, -180, 180);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_ATTITUDE_PITCH: // deg
            return constrain(attitude.values.pitch / 10, -180, 180);
            break;

        default:
            return 0;
            break;
    }
}

static int logicConditionGetFlightModeOperandValue(int operand) {

    switch (operand) {

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_FAILSAFE:
            return (bool) FLIGHT_MODE(FAILSAFE_MODE);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_MANUAL:
            return (bool) FLIGHT_MODE(MANUAL_MODE);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_RTH:
            return (bool) FLIGHT_MODE(NAV_RTH_MODE);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_POSHOLD:
            return (bool) FLIGHT_MODE(NAV_POSHOLD_MODE);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_CRUISE:
            return (bool) FLIGHT_MODE(NAV_CRUISE_MODE);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_ALTHOLD:
            return (bool) FLIGHT_MODE(NAV_ALTHOLD_MODE);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_ANGLE:
            return (bool) FLIGHT_MODE(ANGLE_MODE);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_HORIZON:
            return (bool) FLIGHT_MODE(HORIZON_MODE);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_AIR:
            return (bool) FLIGHT_MODE(AIRMODE_ACTIVE);
            break;

        default:
            return 0;
            break;
    }
}

int logicConditionGetOperandValue(logicOperandType_e type, int operand) {
    int retVal = 0;

    switch (type) {

        case LOGIC_CONDITION_OPERAND_TYPE_VALUE:
            retVal = operand;
            break;

        case LOGIC_CONDITION_OPERAND_TYPE_RC_CHANNEL:
            //Extract RC channel raw value
            if (operand >= 1 && operand <= 16) {
                retVal = rxGetChannelValue(operand - 1);
            } 
            break;

        case LOGIC_CONDITION_OPERAND_TYPE_FLIGHT:
            retVal = logicConditionGetFlightOperandValue(operand);
            break;

        case LOGIC_CONDITION_OPERAND_TYPE_FLIGHT_MODE:
            retVal = logicConditionGetFlightModeOperandValue(operand);
            break;

        case LOGIC_CONDITION_OPERAND_TYPE_LC:
            if (operand >= 0 && operand < MAX_LOGIC_CONDITIONS) {
                retVal = logicConditionGetValue(operand);
            }
            break;

        default:
            break;
    }

    return retVal;
}

/*
 * conditionId == -1 is always evaluated as true
 */ 
int logicConditionGetValue(int8_t conditionId) {
    if (conditionId >= 0) {
        return logicConditionStates[conditionId].value;
    } else {
        return true;
    }
}

void logicConditionUpdateTask(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);
    for (uint8_t i = 0; i < MAX_LOGIC_CONDITIONS; i++) {
        logicConditionProcess(i);
    }
}

void logicConditionReset(void) {
    for (uint8_t i = 0; i < MAX_LOGIC_CONDITIONS; i++) {
        logicConditionStates[i].value = 0;
        logicConditionStates[i].flags = 0;
    }
}
