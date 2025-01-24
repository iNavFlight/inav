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

#include "programming/logic_condition.h"
#include "programming/global_variables.h"
#include "programming/pid.h"
#include "common/utils.h"
#include "rx/rx.h"
#include "common/maths.h"
#include "fc/config.h"
#include "fc/cli.h"
#include "fc/fc_core.h"
#include "fc/rc_controls.h"
#include "fc/runtime_config.h"
#include "fc/rc_modes.h"
#include "navigation/navigation.h"
#include "sensors/battery.h"
#include "sensors/pitotmeter.h"
#include "sensors/rangefinder.h"
#include "flight/imu.h"
#include "flight/pid.h"
#include "flight/mixer_profile.h"
#include "drivers/io_port_expander.h"
#include "io/osd_common.h"
#include "sensors/diagnostics.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#include "io/vtx.h"
#include "drivers/vtx_common.h"
#include "drivers/light_ws2811strip.h"

PG_REGISTER_ARRAY_WITH_RESET_FN(logicCondition_t, MAX_LOGIC_CONDITIONS, logicConditions, PG_LOGIC_CONDITIONS, 4);

EXTENDED_FASTRAM uint64_t logicConditionsGlobalFlags;
EXTENDED_FASTRAM int logicConditionValuesByType[LOGIC_CONDITION_LAST];
EXTENDED_FASTRAM rcChannelOverride_t rcChannelOverrides[MAX_SUPPORTED_RC_CHANNEL_COUNT];
EXTENDED_FASTRAM flightAxisOverride_t flightAxisOverride[XYZ_AXIS_COUNT];

void pgResetFn_logicConditions(logicCondition_t *instance)
{
    for (int i = 0; i < MAX_LOGIC_CONDITIONS; i++) {
        RESET_CONFIG(logicCondition_t, &instance[i],
            .enabled = 0,
            .activatorId = -1,
            .operation = 0,
            .operandA = {
                .type = LOGIC_CONDITION_OPERAND_TYPE_VALUE,
                .value = 0
            },
            .operandB = {
                .type = LOGIC_CONDITION_OPERAND_TYPE_VALUE,
                .value = 0
            },
            .flags = 0
        );
    }
}

logicConditionState_t logicConditionStates[MAX_LOGIC_CONDITIONS];

static int logicConditionCompute(
    int32_t currentValue,
    logicOperation_e operation,
    int32_t operandA,
    int32_t operandB,
    uint8_t lcIndex
) {
    int temporaryValue;
#if defined(USE_VTX_CONTROL)
    vtxDeviceCapability_t vtxDeviceCapability;
#endif

    switch (operation) {

        case LOGIC_CONDITION_TRUE:
            return true;
            break;

        case LOGIC_CONDITION_EQUAL:
            return operandA == operandB;
            break;

        case LOGIC_CONDITION_APPROX_EQUAL:
            {
                uint16_t offest = operandA / 100;
                return ((operandB >= (operandA - offest)) && (operandB <= (operandA + offest)));
            }
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
            return currentValue;
            break;

        case LOGIC_CONDITION_EDGE:
            if (operandA && logicConditionStates[lcIndex].timeout == 0 && !(logicConditionStates[lcIndex].flags & LOGIC_CONDITION_FLAG_TIMEOUT_SATISFIED)) {
                if (operandB < 100) {
                    logicConditionStates[lcIndex].timeout = millis();
                } else {
                    logicConditionStates[lcIndex].timeout = millis() + operandB;
                }
                logicConditionStates[lcIndex].flags |= LOGIC_CONDITION_FLAG_TIMEOUT_SATISFIED;
                return true;
            } else if (logicConditionStates[lcIndex].timeout > 0) {
                if (logicConditionStates[lcIndex].timeout < millis()) {
                    logicConditionStates[lcIndex].timeout = 0;
                } else {
                    return true;
                }
            }

            if (!operandA) {
                logicConditionStates[lcIndex].flags &= ~LOGIC_CONDITION_FLAG_TIMEOUT_SATISFIED;
            }
            return false;
            break;

        case LOGIC_CONDITION_DELAY:
            if (operandA) {
                if (logicConditionStates[lcIndex].timeout == 0) {
                    logicConditionStates[lcIndex].timeout = millis() + operandB;
                } else if (millis() > logicConditionStates[lcIndex].timeout ) {
                    logicConditionStates[lcIndex].flags |= LOGIC_CONDITION_FLAG_TIMEOUT_SATISFIED;
                    return true;
                } else if (logicConditionStates[lcIndex].flags & LOGIC_CONDITION_FLAG_TIMEOUT_SATISFIED) {
                    return true;
                }
            } else {
                logicConditionStates[lcIndex].timeout = 0;
                logicConditionStates[lcIndex].flags &= ~LOGIC_CONDITION_FLAG_TIMEOUT_SATISFIED;
            }

            return false;
            break;

        case LOGIC_CONDITION_TIMER:
            if ((logicConditionStates[lcIndex].timeout == 0) || (millis() > logicConditionStates[lcIndex].timeout && !currentValue)) {
                logicConditionStates[lcIndex].timeout = millis() + operandA;
                return true;
            } else if (millis() > logicConditionStates[lcIndex].timeout && currentValue) {
                logicConditionStates[lcIndex].timeout = millis() + operandB;
                return false;
            }
            return currentValue;
            break;

        case LOGIC_CONDITION_DELTA:
            {
                int difference = logicConditionStates[lcIndex].lastValue - operandA;
                logicConditionStates[lcIndex].lastValue = operandA;
                return ABS(difference) >= operandB;
            }
            break;

        case LOGIC_CONDITION_GVAR_SET:
            gvSet(operandA, operandB);
            return operandB;
            break;

        case LOGIC_CONDITION_GVAR_INC:
            temporaryValue = gvGet(operandA) + operandB;
            gvSet(operandA, temporaryValue);
            return temporaryValue;
            break;

        case LOGIC_CONDITION_GVAR_DEC:
            temporaryValue = gvGet(operandA) - operandB;
            gvSet(operandA, temporaryValue);
            return temporaryValue;
            break;

        case LOGIC_CONDITION_ADD:
            return constrain(operandA + operandB, INT32_MIN, INT32_MAX);
            break;

        case LOGIC_CONDITION_SUB:
            return constrain(operandA - operandB, INT32_MIN, INT32_MAX);
            break;

        case LOGIC_CONDITION_MUL:
            return constrain(operandA * operandB, INT32_MIN, INT32_MAX);
            break;

        case LOGIC_CONDITION_DIV:
            if (operandB != 0) {
                return constrain(operandA / operandB, INT32_MIN, INT32_MAX);
            } else {
                return operandA;
            }
            break;

        case LOGIC_CONDITION_OVERRIDE_ARMING_SAFETY:
            LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_ARMING_SAFETY);
            return true;
            break;

        case LOGIC_CONDITION_OVERRIDE_THROTTLE_SCALE:
            logicConditionValuesByType[LOGIC_CONDITION_OVERRIDE_THROTTLE_SCALE] = operandA;
            LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_THROTTLE_SCALE);
            return true;
            break;

        case LOGIC_CONDITION_SWAP_ROLL_YAW:
            LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_SWAP_ROLL_YAW);
            return true;
            break;

#ifdef USE_MAG
        case LOGIC_CONDITION_RESET_MAG_CALIBRATION:

            ENABLE_STATE(CALIBRATE_MAG);
            return true;
            break;
#endif

#if defined(USE_VTX_CONTROL)
        case LOGIC_CONDITION_SET_VTX_POWER_LEVEL:
        {
            uint8_t newPower = logicConditionValuesByType[LOGIC_CONDITION_SET_VTX_POWER_LEVEL];
            if ((newPower != operandA || newPower != vtxSettingsConfig()->power) && vtxCommonGetDeviceCapability(vtxCommonDevice(), &vtxDeviceCapability)) {
                newPower = constrain(operandA, VTX_SETTINGS_MIN_POWER, vtxDeviceCapability.powerCount);
                logicConditionValuesByType[LOGIC_CONDITION_SET_VTX_POWER_LEVEL] = newPower;
                if (newPower != vtxSettingsConfig()->power) {
                    vtxCommonSetPowerByIndex(vtxCommonDevice(), newPower); // Force setting if modified elsewhere
                }
                vtxSettingsConfigMutable()->power = newPower;
                return newPower;
            }
            return false;
            break;
        }
        case LOGIC_CONDITION_SET_VTX_BAND:
        {
            uint8_t newBand = logicConditionValuesByType[LOGIC_CONDITION_SET_VTX_BAND];
            if ((newBand != operandA  || newBand != vtxSettingsConfig()->band) && vtxCommonGetDeviceCapability(vtxCommonDevice(), &vtxDeviceCapability)) {
                newBand = constrain(operandA, VTX_SETTINGS_MIN_BAND, vtxDeviceCapability.bandCount);
                logicConditionValuesByType[LOGIC_CONDITION_SET_VTX_BAND] = newBand;
                if (newBand != vtxSettingsConfig()->band) {
                    vtxCommonSetBandAndChannel(vtxCommonDevice(), newBand, vtxSettingsConfig()->channel);
                }
                vtxSettingsConfigMutable()->band = newBand;
                return newBand;
            }
            return false;
            break;
        }
        case LOGIC_CONDITION_SET_VTX_CHANNEL:
        {
            uint8_t newChannel = logicConditionValuesByType[LOGIC_CONDITION_SET_VTX_CHANNEL];
            if ((newChannel != operandA  || newChannel != vtxSettingsConfig()->channel) && vtxCommonGetDeviceCapability(vtxCommonDevice(), &vtxDeviceCapability)) {
                newChannel = constrain(operandA, VTX_SETTINGS_MIN_CHANNEL, vtxDeviceCapability.channelCount);
                logicConditionValuesByType[LOGIC_CONDITION_SET_VTX_CHANNEL] = newChannel;
                if (newChannel != vtxSettingsConfig()->channel) {
                    vtxCommonSetBandAndChannel(vtxCommonDevice(), vtxSettingsConfig()->band, newChannel);
                }
                vtxSettingsConfigMutable()->channel = newChannel;
                return newChannel;
            }
            return false;
            break;
        }
#endif
        case LOGIC_CONDITION_INVERT_ROLL:
            LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_ROLL);
            return true;
            break;

        case LOGIC_CONDITION_INVERT_PITCH:
            LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_PITCH);
            return true;
            break;

        case LOGIC_CONDITION_INVERT_YAW:
            LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_YAW);
            return true;
            break;

        case LOGIC_CONDITION_OVERRIDE_THROTTLE:
            logicConditionValuesByType[LOGIC_CONDITION_OVERRIDE_THROTTLE] = operandA;
            LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_THROTTLE);
            return operandA;
            break;

        case LOGIC_CONDITION_SET_OSD_LAYOUT:
            logicConditionValuesByType[LOGIC_CONDITION_SET_OSD_LAYOUT] = operandA;
            LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_OSD_LAYOUT);
            return operandA;
            break;

#ifdef USE_I2C_IO_EXPANDER
        case LOGIC_CONDITION_PORT_SET:
            ioPortExpanderSet((uint8_t)operandA, (uint8_t)operandB);
            return operandB;
            break;
#endif

        case LOGIC_CONDITION_SIN:
            temporaryValue = (operandB == 0) ? 500 : operandB;
            return sin_approx(DEGREES_TO_RADIANS(operandA)) * temporaryValue;
            break;

        case LOGIC_CONDITION_COS:
            temporaryValue = (operandB == 0) ? 500 : operandB;
            return cos_approx(DEGREES_TO_RADIANS(operandA)) * temporaryValue;
            break;
        break;

        case LOGIC_CONDITION_TAN:
            temporaryValue = (operandB == 0) ? 500 : operandB;
            return tan_approx(DEGREES_TO_RADIANS(operandA)) * temporaryValue;
        break;

        case LOGIC_CONDITION_MIN:
            return (operandA < operandB) ? operandA : operandB;
        break;

        case LOGIC_CONDITION_MAX:
            return (operandA > operandB) ? operandA : operandB;
        break;

        case LOGIC_CONDITION_MAP_INPUT:
            return scaleRange(constrain(operandA, 0, operandB), 0, operandB, 0, 1000);
        break;

        case LOGIC_CONDITION_MAP_OUTPUT:
            return scaleRange(constrain(operandA, 0, 1000), 0, 1000, 0, operandB);
        break;

        case LOGIC_CONDITION_RC_CHANNEL_OVERRIDE:
            temporaryValue = constrain(operandA - 1, 0, MAX_SUPPORTED_RC_CHANNEL_COUNT - 1);
            rcChannelOverrides[temporaryValue].active = true;
            rcChannelOverrides[temporaryValue].value = constrain(operandB, PWM_RANGE_MIN, PWM_RANGE_MAX);
            LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_RC_CHANNEL);
            return true;
        break;

        case LOGIC_CONDITION_SET_HEADING_TARGET:
            temporaryValue = CENTIDEGREES_TO_DEGREES(wrap_36000(DEGREES_TO_CENTIDEGREES(operandA)));
            updateHeadingHoldTarget(temporaryValue);
            return temporaryValue;
        break;

        case LOGIC_CONDITION_MODULUS:
            if (operandB != 0) {
                return constrain(operandA % operandB, INT32_MIN, INT32_MAX);
            } else {
                return operandA;
            }
            break;

        case LOGIC_CONDITION_SET_PROFILE:
            operandA--;
            if ( getConfigProfile() != operandA  && (operandA >= 0 && operandA < MAX_PROFILE_COUNT)) {
                bool profileChanged = false;
                if (setConfigProfile(operandA)) {
                    pidInit();
                    pidInitFilters();
                    schedulePidGainsUpdate();
                    navigationUsePIDs(); //set navigation pid gains
                    profileChanged = true;
                }
                return profileChanged;
            } else {
                return false;
            }
            break;

        case LOGIC_CONDITION_LOITER_OVERRIDE:
            logicConditionValuesByType[LOGIC_CONDITION_LOITER_OVERRIDE] = constrain(operandA, 0, 100000);
            LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_LOITER_RADIUS);
            return true;
            break;

        case LOGIC_CONDITION_FLIGHT_AXIS_ANGLE_OVERRIDE:
            if (operandA >= 0 && operandA <= 2) {

                flightAxisOverride[operandA].angleTargetActive = true;
                int target = DEGREES_TO_DECIDEGREES(operandB);
                if (operandA == 0) {
                    //ROLL
                    target = constrain(target, -pidProfile()->max_angle_inclination[FD_ROLL], pidProfile()->max_angle_inclination[FD_ROLL]);
                } else if (operandA == 1) {
                    //PITCH
                    target = constrain(target, -pidProfile()->max_angle_inclination[FD_PITCH], pidProfile()->max_angle_inclination[FD_PITCH]);
                } else if (operandA == 2) {
                    //YAW
                    target = (constrain(target, 0, 3600));
                }
                flightAxisOverride[operandA].angleTarget = target;
                LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_FLIGHT_AXIS);

                return true;
            } else {
                return false;
            }
            break;

        case LOGIC_CONDITION_FLIGHT_AXIS_RATE_OVERRIDE:
            if (operandA >= 0 && operandA <= 2) {
                flightAxisOverride[operandA].rateTargetActive = true;
                flightAxisOverride[operandA].rateTarget = constrain(operandB, -2000, 2000);
                LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_FLIGHT_AXIS);
                return true;
            } else {
                return false;
            }
            break;

#ifdef USE_LED_STRIP
        case LOGIC_CONDITION_LED_PIN_PWM:

            if (operandA >=0 && operandA <= 100) {
                ledPinStartPWM((uint8_t)operandA);
            } else {
                ledPinStopPWM();
            }
            return operandA;
            break;
#endif
#ifdef USE_GPS_FIX_ESTIMATION
        case LOGIC_CONDITION_DISABLE_GPS_FIX:
            if (operandA > 0) {
                LOGIC_CONDITION_GLOBAL_FLAG_ENABLE(LOGIC_CONDITION_GLOBAL_FLAG_DISABLE_GPS_FIX);
            } else {
                LOGIC_CONDITION_GLOBAL_FLAG_DISABLE(LOGIC_CONDITION_GLOBAL_FLAG_DISABLE_GPS_FIX);
            }
                return true;
            break;
#endif

        default:
            return false;
            break;
    }
}

void logicConditionProcess(uint8_t i) {

    const int32_t activatorValue = logicConditionGetValue(logicConditions(i)->activatorId);

    if (logicConditions(i)->enabled && activatorValue && !cliMode) {

        /*
         * Process condition only when latch flag is not set
         * Latched LCs can only go from OFF to ON, not the other way
         */
        if (!(logicConditionStates[i].flags & LOGIC_CONDITION_FLAG_LATCH)) {
            const int32_t operandAValue = logicConditionGetOperandValue(logicConditions(i)->operandA.type, logicConditions(i)->operandA.value);
            const int32_t operandBValue = logicConditionGetOperandValue(logicConditions(i)->operandB.type, logicConditions(i)->operandB.value);
            const int32_t newValue = logicConditionCompute(
                logicConditionStates[i].value,
                logicConditions(i)->operation,
                operandAValue,
                operandBValue,
                i
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

static int logicConditionGetWaypointOperandValue(int operand) {

    switch (operand) {
        case LOGIC_CONDITION_OPERAND_WAYPOINTS_IS_WP: // 0/1
            return (navGetCurrentStateFlags() & NAV_AUTO_WP) ? 1 : 0;
            break;

        case LOGIC_CONDITION_OPERAND_WAYPOINTS_WAYPOINT_INDEX:
            return NAV_Status.activeWpNumber;
            break;

        case LOGIC_CONDITION_OPERAND_WAYPOINTS_WAYPOINT_ACTION:
            return NAV_Status.activeWpAction;
            break;

        case LOGIC_CONDITION_OPERAND_WAYPOINTS_NEXT_WAYPOINT_ACTION:
            {
                uint8_t wpIndex = posControl.activeWaypointIndex + 1;
                if ((wpIndex > 0) && (wpIndex < NAV_MAX_WAYPOINTS)) {
                    return posControl.waypointList[wpIndex].action;
                }
                return false;
            }
            break;

        case LOGIC_CONDITION_OPERAND_WAYPOINTS_WAYPOINT_DISTANCE:
            {
                uint32_t distance = 0;
                if (navGetCurrentStateFlags() & NAV_AUTO_WP) {
                    fpVector3_t poi;
                    gpsLocation_t wp;
                    wp.lat = posControl.waypointList[NAV_Status.activeWpIndex].lat;
                    wp.lon = posControl.waypointList[NAV_Status.activeWpIndex].lon;
                    wp.alt = posControl.waypointList[NAV_Status.activeWpIndex].alt;
                    geoConvertGeodeticToLocal(&poi, &posControl.gpsOrigin, &wp, GEO_ALT_RELATIVE);

                    distance = calculateDistanceToDestination(&poi) / 100;
                }

                return distance;
            }
            break;

        case LOGIC_CONDTIION_OPERAND_WAYPOINTS_DISTANCE_FROM_WAYPOINT:
            {
                uint32_t distance = 0;
                if ((navGetCurrentStateFlags() & NAV_AUTO_WP) && NAV_Status.activeWpIndex > 0) {
                    fpVector3_t poi;
                    gpsLocation_t wp;
                    wp.lat = posControl.waypointList[NAV_Status.activeWpIndex-1].lat;
                    wp.lon = posControl.waypointList[NAV_Status.activeWpIndex-1].lon;
                    wp.alt = posControl.waypointList[NAV_Status.activeWpIndex-1].alt;
                    geoConvertGeodeticToLocal(&poi, &posControl.gpsOrigin, &wp, GEO_ALT_RELATIVE);

                    distance = calculateDistanceToDestination(&poi) / 100;
                }

                return distance;
            }
            break;

        case LOGIC_CONDITION_OPERAND_WAYPOINTS_USER1_ACTION:
            return (NAV_Status.activeWpIndex > 0) ? ((posControl.waypointList[NAV_Status.activeWpIndex-1].p3 & NAV_WP_USER1) == NAV_WP_USER1) : 0;
            break;

        case LOGIC_CONDITION_OPERAND_WAYPOINTS_USER2_ACTION:
            return (NAV_Status.activeWpIndex > 0) ? ((posControl.waypointList[NAV_Status.activeWpIndex-1].p3 & NAV_WP_USER2) == NAV_WP_USER2) : 0;
            break;

        case LOGIC_CONDITION_OPERAND_WAYPOINTS_USER3_ACTION:
            return (NAV_Status.activeWpIndex > 0) ? ((posControl.waypointList[NAV_Status.activeWpIndex-1].p3 & NAV_WP_USER3) == NAV_WP_USER3) : 0;
            break;

        case LOGIC_CONDITION_OPERAND_WAYPOINTS_USER4_ACTION:
            return (NAV_Status.activeWpIndex > 0) ? ((posControl.waypointList[NAV_Status.activeWpIndex-1].p3 & NAV_WP_USER4) == NAV_WP_USER4) : 0;
            break;

        case LOGIC_CONDITION_OPERAND_WAYPOINTS_USER1_ACTION_NEXT_WP:
            return ((posControl.waypointList[NAV_Status.activeWpIndex].p3 & NAV_WP_USER1) == NAV_WP_USER1);
            break;

        case LOGIC_CONDITION_OPERAND_WAYPOINTS_USER2_ACTION_NEXT_WP:
            return ((posControl.waypointList[NAV_Status.activeWpIndex].p3 & NAV_WP_USER2) == NAV_WP_USER2);
            break;

        case LOGIC_CONDITION_OPERAND_WAYPOINTS_USER3_ACTION_NEXT_WP:
            return ((posControl.waypointList[NAV_Status.activeWpIndex].p3 & NAV_WP_USER3) == NAV_WP_USER3);
            break;

        case LOGIC_CONDITION_OPERAND_WAYPOINTS_USER4_ACTION_NEXT_WP:
            return ((posControl.waypointList[NAV_Status.activeWpIndex].p3 & NAV_WP_USER4) == NAV_WP_USER4);
            break;

        default:
            return 0;
            break;
    }
}

static int logicConditionGetFlightOperandValue(int operand) {

    switch (operand) {

        case LOGIC_CONDITION_OPERAND_FLIGHT_ARM_TIMER: // in s
            return constrain((uint32_t)getFlightTime(), 0, INT32_MAX);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_HOME_DISTANCE: //in m
            return constrain(GPS_distanceToHome, 0, INT32_MAX);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_TRIP_DISTANCE: //in m
            return constrain(getTotalTravelDistance() / 100, 0, INT32_MAX);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_RSSI:
            return constrain(getRSSI() * 100 / RSSI_MAX_VALUE, 0, 99);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_VBAT: // V / 100
            return getBatteryVoltage();
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_CELL_VOLTAGE: // V / 10
            return getBatteryAverageCellVoltage();
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_BATT_CELLS:
            return getBatteryCellCount();
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_CURRENT: // Amp / 100
            return getAmperage();
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_MAH_DRAWN: // mAh
            return getMAhDrawn();
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_GPS_SATS:
#ifdef USE_GPS_FIX_ESTIMATION
            if ( STATE(GPS_ESTIMATED_FIX) ){
                return gpsSol.numSat; //99
            } else
#endif
            if (getHwGPSStatus() == HW_SENSOR_UNAVAILABLE || getHwGPSStatus() == HW_SENSOR_UNHEALTHY) {
                return 0;
            } else {
                return gpsSol.numSat;
            }
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_GPS_VALID: // 0/1
            return STATE(GPS_FIX) ? 1 : 0;
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_GROUD_SPEED: // cm/s
            return gpsSol.groundSpeed;
            break;

        //FIXME align with osdGet3DSpeed
        case LOGIC_CONDITION_OPERAND_FLIGHT_3D_SPEED: // cm/s
            return osdGet3DSpeed();
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_AIR_SPEED: // cm/s
        #ifdef USE_PITOT
            return constrain(getAirspeedEstimate(), 0, INT32_MAX);
        #else
            return false;
        #endif
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_ALTITUDE: // cm
            return constrain(getEstimatedActualPosition(Z), INT32_MIN, INT32_MAX);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_VERTICAL_SPEED: // cm/s
            return constrain(getEstimatedActualVelocity(Z), INT32_MIN, INT32_MAX);
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

        case LOGIC_CONDITION_OPERAND_FLIGHT_ATTITUDE_YAW: // deg
            return constrain(attitude.values.yaw / 10, 0, 360);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_IS_ARMED: // 0/1
            return ARMING_FLAG(ARMED) ? 1 : 0;
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_IS_AUTOLAUNCH: // 0/1
            return (navGetCurrentStateFlags() & NAV_CTL_LAUNCH) ? 1 : 0;
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_IS_ALTITUDE_CONTROL: // 0/1
            return (navGetCurrentStateFlags() & NAV_CTL_ALT) ? 1 : 0;
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_IS_POSITION_CONTROL: // 0/1
            return (navGetCurrentStateFlags() & NAV_CTL_POS) ? 1 : 0;
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_IS_EMERGENCY_LANDING: // 0/1
            return (navGetCurrentStateFlags() & NAV_CTL_EMERG) ? 1 : 0;
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_IS_RTH: // 0/1
            return (navGetCurrentStateFlags() & NAV_AUTO_RTH) ? 1 : 0;
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_IS_LANDING: // 0/1
#ifdef USE_FW_AUTOLAND
            return ((navGetCurrentStateFlags() & NAV_CTL_LAND) || FLIGHT_MODE(NAV_FW_AUTOLAND)) ? 1 : 0;
#else
            return ((navGetCurrentStateFlags() & NAV_CTL_LAND)) ? 1 : 0;
#endif

            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_IS_FAILSAFE: // 0/1
            return (failsafePhase() != FAILSAFE_IDLE) ? 1 : 0;
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_STABILIZED_YAW: //
            return axisPID[YAW];
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_STABILIZED_ROLL: //
            return axisPID[ROLL];
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_STABILIZED_PITCH: //
            return axisPID[PITCH];
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_3D_HOME_DISTANCE: //in m
            return constrain(calc_length_pythagorean_2D(GPS_distanceToHome, getEstimatedActualPosition(Z) / 100.0f), 0, INT32_MAX);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_LQ_UPLINK:
#if defined(USE_SERIALRX_CRSF) || defined(USE_RX_MSP)
            return rxLinkStatistics.uplinkLQ;
#else
            return 0;
#endif
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_UPLINK_RSSI_DBM:
#if defined(USE_SERIALRX_CRSF) || defined(USE_RX_MSP)
            return rxLinkStatistics.uplinkRSSI;
#else
            return 0;
#endif        
            break;

case LOGIC_CONDITION_OPERAND_FLIGHT_LQ_DOWNLINK:
#if defined(USE_SERIALRX_CRSF) || defined(USE_RX_MSP)
            return rxLinkStatistics.downlinkLQ;
#else
            return 0;
#endif
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_SNR:
#if defined(USE_SERIALRX_CRSF) || defined(USE_RX_MSP)
            return rxLinkStatistics.uplinkSNR;
#else
            return 0;
#endif
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_ACTIVE_PROFILE: // int
            return getConfigProfile() + 1;
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_BATT_PROFILE: //int
            return getConfigBatteryProfile() + 1;
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_ACTIVE_MIXER_PROFILE: // int
            return currentMixerProfileIndex + 1;
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_MIXER_TRANSITION_ACTIVE: //0,1
            return isMixerTransitionMixing ? 1 : 0;
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_LOITER_RADIUS:
            return getLoiterRadius(navConfig()->fw.loiter_radius);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_FLOWN_LOITER_RADIUS:
            return getFlownLoiterRadius();
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_AGL_STATUS:
            return isEstimatedAglTrusted();
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_AGL:
            return getEstimatedAglPosition();
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_RANGEFINDER_RAW:
            return rangefinderGetLatestRawAltitude();
            break;
#ifdef USE_FW_AUTOLAND
        case LOGIC_CONDITION_OPERAND_FLIGHT_FW_LAND_STATE:
            return posControl.fwLandState.landState;
            break;
#endif
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

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_WAYPOINT_MISSION:
            return (bool) FLIGHT_MODE(NAV_WP_MODE);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_COURSE_HOLD:
            return ((bool) FLIGHT_MODE(NAV_COURSE_HOLD_MODE) && !(bool)FLIGHT_MODE(NAV_ALTHOLD_MODE));
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_CRUISE:
            return (bool) (FLIGHT_MODE(NAV_COURSE_HOLD_MODE) && FLIGHT_MODE(NAV_ALTHOLD_MODE));
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

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_ANGLEHOLD:
            return (bool) FLIGHT_MODE(ANGLEHOLD_MODE);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_AIR:
            return (bool) FLIGHT_MODE(AIRMODE_ACTIVE);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_ACRO:
            return (((bool) FLIGHT_MODE(ANGLE_MODE) || (bool) FLIGHT_MODE(HORIZON_MODE) || (bool) FLIGHT_MODE(ANGLEHOLD_MODE) ||
                     (bool) FLIGHT_MODE(MANUAL_MODE) || (bool) FLIGHT_MODE(NAV_RTH_MODE) || (bool) FLIGHT_MODE(NAV_POSHOLD_MODE) ||
                     (bool) FLIGHT_MODE(NAV_COURSE_HOLD_MODE) || (bool) FLIGHT_MODE(NAV_WP_MODE)) == false);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_USER1:
            return IS_RC_MODE_ACTIVE(BOXUSER1);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_USER2:
            return IS_RC_MODE_ACTIVE(BOXUSER2);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_USER3:
            return IS_RC_MODE_ACTIVE(BOXUSER3);
            break;

        case LOGIC_CONDITION_OPERAND_FLIGHT_MODE_USER4:
            return IS_RC_MODE_ACTIVE(BOXUSER4);
            break;

        default:
            return 0;
            break;
    }
}

int32_t logicConditionGetOperandValue(logicOperandType_e type, int operand) {
    int32_t retVal = 0;

    switch (type) {

        case LOGIC_CONDITION_OPERAND_TYPE_VALUE:
            retVal = operand;
            break;

        case LOGIC_CONDITION_OPERAND_TYPE_RC_CHANNEL:
            //Extract RC channel raw value
            if (operand >= 1 && operand <= MAX_SUPPORTED_RC_CHANNEL_COUNT) {
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

        case LOGIC_CONDITION_OPERAND_TYPE_GVAR:
            if (operand >= 0 && operand < MAX_GLOBAL_VARIABLES) {
                retVal = gvGet(operand);
            }
            break;

        case LOGIC_CONDITION_OPERAND_TYPE_PID:
            if (operand >= 0 && operand < MAX_PROGRAMMING_PID_COUNT) {
                retVal = programmingPidGetOutput(operand);
            }
            break;

        case LOGIC_CONDITION_OPERAND_TYPE_WAYPOINTS:
            retVal = logicConditionGetWaypointOperandValue(operand);
            break;

        default:
            break;
    }

    return retVal;
}

/*
 * conditionId == -1 is always evaluated as true
 */
int32_t logicConditionGetValue(int8_t conditionId) {
    if (conditionId >= 0) {
        return logicConditionStates[conditionId].value;
    } else {
        return true;
    }
}

void logicConditionUpdateTask(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);

    //Disable all flags
    logicConditionsGlobalFlags = 0;

    for (uint8_t i = 0; i < MAX_SUPPORTED_RC_CHANNEL_COUNT; i++) {
        rcChannelOverrides[i].active = false;
    }

    for (uint8_t i = 0; i < XYZ_AXIS_COUNT; i++) {
        flightAxisOverride[i].rateTargetActive = false;
        flightAxisOverride[i].angleTargetActive = false;
    }

    for (uint8_t i = 0; i < MAX_LOGIC_CONDITIONS; i++) {
        logicConditionProcess(i);
    }

#ifdef USE_I2C_IO_EXPANDER
    ioPortExpanderSync();
#endif
}

void logicConditionReset(void) {
    for (uint8_t i = 0; i < MAX_LOGIC_CONDITIONS; i++) {
        logicConditionStates[i].value = 0;
        logicConditionStates[i].lastValue = 0;
        logicConditionStates[i].flags = 0;
        logicConditionStates[i].timeout = 0;
    }
}

float NOINLINE getThrottleScale(float globalThrottleScale) {
    if (LOGIC_CONDITION_GLOBAL_FLAG(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_THROTTLE_SCALE)) {
        return constrainf(logicConditionValuesByType[LOGIC_CONDITION_OVERRIDE_THROTTLE_SCALE] / 100.0f, 0.0f, 1.0f);
    } else {
        return globalThrottleScale;
    }
}

int16_t getRcCommandOverride(int16_t command[], uint8_t axis) {
    int16_t outputValue = command[axis];

    if (LOGIC_CONDITION_GLOBAL_FLAG(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_SWAP_ROLL_YAW) && axis == FD_ROLL) {
        outputValue = command[FD_YAW];
    } else if (LOGIC_CONDITION_GLOBAL_FLAG(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_SWAP_ROLL_YAW) && axis == FD_YAW) {
        outputValue = command[FD_ROLL];
    }

    if (LOGIC_CONDITION_GLOBAL_FLAG(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_ROLL) && axis == FD_ROLL) {
        outputValue *= -1;
    }
    if (LOGIC_CONDITION_GLOBAL_FLAG(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_PITCH) && axis == FD_PITCH) {
        outputValue *= -1;
    }
    if (LOGIC_CONDITION_GLOBAL_FLAG(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_YAW) && axis == FD_YAW) {
        outputValue *= -1;
    }

    return outputValue;
}

int16_t getRcChannelOverride(uint8_t channel, int16_t originalValue) {
    if (rcChannelOverrides[channel].active) {
        return rcChannelOverrides[channel].value;
    } else {
        return originalValue;
    }
}

uint32_t getLoiterRadius(uint32_t loiterRadius) {
#ifdef USE_PROGRAMMING_FRAMEWORK
    if (LOGIC_CONDITION_GLOBAL_FLAG(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_LOITER_RADIUS) &&
        !(FLIGHT_MODE(FAILSAFE_MODE) || FLIGHT_MODE(NAV_RTH_MODE) || FLIGHT_MODE(NAV_WP_MODE) || navigationIsExecutingAnEmergencyLanding())) {
        return constrain(logicConditionValuesByType[LOGIC_CONDITION_LOITER_OVERRIDE], loiterRadius, 100000);
    } else {
        return loiterRadius;
    }
#else
    return loiterRadius;
#endif
}

float getFlightAxisAngleOverride(uint8_t axis, float angle) {
    if (flightAxisOverride[axis].angleTargetActive) {
        return flightAxisOverride[axis].angleTarget;
    } else {
        return angle;
    }
}

float getFlightAxisRateOverride(uint8_t axis, float rate) {
    if (flightAxisOverride[axis].rateTargetActive) {
        return flightAxisOverride[axis].rateTarget;
    } else {
        return rate;
    }
}

bool isFlightAxisAngleOverrideActive(uint8_t axis) {
    if (flightAxisOverride[axis].angleTargetActive) {
        return true;
    } else {
        return false;
    }
}

bool isFlightAxisRateOverrideActive(uint8_t axis) {
    if (flightAxisOverride[axis].rateTargetActive) {
        return true;
    } else {
        return false;
    }
}
