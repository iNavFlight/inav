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

#include "fc/config.h"
#include "fc/fc_core.h"
#include "fc/multifunction.h"
#include "fc/rc_modes.h"

#include "flight/imu.h"

#include "io/osd.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#include "sensors/acceleration.h"
#include "sensors/battery.h"
#include "sensors/compass.h"
#include "sensors/diagnostics.h"
#include "sensors/pitotmeter.h"
#include "sensors/sensors.h"

textAttributes_t osdGetMultiFunctionMessage(char *buff);
multiFunctionWarning_t multiFunctionWarning;

#ifdef USE_MULTI_FUNCTIONS
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
        MULTI_FUNC_FLAG(MF_SUSPEND_SAFEHOMES) ? MULTI_FUNC_FLAG_DISABLE(MF_SUSPEND_SAFEHOMES) : MULTI_FUNC_FLAG_ENABLE(MF_SUSPEND_SAFEHOMES);
#endif
        break;
    case MULTI_FUNC_3:  // toggle RTH Trackback suspend
        MULTI_FUNC_FLAG(MF_SUSPEND_TRACKBACK) ? MULTI_FUNC_FLAG_DISABLE(MF_SUSPEND_TRACKBACK) : MULTI_FUNC_FLAG_ENABLE(MF_SUSPEND_TRACKBACK);
        break;
    case MULTI_FUNC_4:  // toggle Turtle mode
#ifdef USE_DSHOT
        MULTI_FUNC_FLAG(MF_TURTLE_MODE) ? MULTI_FUNC_FLAG_DISABLE(MF_TURTLE_MODE) : MULTI_FUNC_FLAG_ENABLE(MF_TURTLE_MODE);
#endif
        break;
    case MULTI_FUNC_5:  // emergency ARM
        emergencyArmingUpdate(true, true);
        break;
    case MULTI_FUNC_6:  // Calibrate compass/Zero yaw heading
#if defined(USE_GPS) || defined(USE_MAG)
        ENABLE_STATE(CALIBRATE_MAG);
#endif
        break;
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
#endif  // multifunction

static bool osdCheckWarning(bool condition, uint8_t warningFlag)
{
    static timeMs_t newWarningEndTime = 0;
    static uint8_t newWarningFlags = 0;  // bitfield
    const timeMs_t currentTimeMs = millis();

    /* New warnings dislayed individually for 10s with blinking after which
     * all current warnings displayed without blinking on 1 second cycle */
    if (condition) {    // condition required to trigger warning
        if (!(multiFunctionWarning.osdWarningsFlags & warningFlag)) {  // check for new warnings
            multiFunctionWarning.osdWarningsFlags |= warningFlag;
            newWarningFlags |= warningFlag;
            newWarningEndTime = currentTimeMs + 10000;
            multiFunctionWarning.newWarningActive = true;
        }
#ifdef USE_DEV_TOOLS
        if (systemConfig()->groundTestMode) {
            return true;
        }
#endif
        if (currentTimeMs < newWarningEndTime) {
            return (newWarningFlags & warningFlag);  // filter out new warnings excluding older warnings
        } else {
            newWarningFlags = 0;
            multiFunctionWarning.newWarningActive = false;
        }
        return true;
    } else if (multiFunctionWarning.osdWarningsFlags & warningFlag) {
        multiFunctionWarning.osdWarningsFlags &= ~warningFlag;
    }

    return false;
}

textAttributes_t osdGetMultiFunctionMessage(char *buff)
{
    /* Message length limit 10 char max */

    textAttributes_t elemAttr = TEXT_ATTRIBUTES_NONE;
    const char *message = NULL;

#ifdef USE_MULTI_FUNCTIONS
    /* --- FUNCTIONS --- */
    multi_function_e selectedFunction = multiFunctionSelection();

    if (selectedFunction) {
        multi_function_e activeFunction = selectedFunction;

        switch (selectedFunction) {
        case MULTI_FUNC_NONE:
        case MULTI_FUNC_1:
            if (ARMING_FLAG(ARMED)) {
                message = posControl.flags.manualEmergLandActive ? "ABORT LAND" : "EMERG LAND";
                break;
            }
            activeFunction++;
            FALLTHROUGH;
        case MULTI_FUNC_2:
#if defined(USE_SAFE_HOME)
            if (navConfig()->general.flags.safehome_usage_mode != SAFEHOME_USAGE_OFF) {
                message = MULTI_FUNC_FLAG(MF_SUSPEND_SAFEHOMES) ? "USE SFHOME" : "SUS SFHOME";
                break;
            }
#endif
            activeFunction++;
            FALLTHROUGH;
        case MULTI_FUNC_3:
            if (navConfig()->general.flags.rth_trackback_mode != RTH_TRACKBACK_OFF) {
                message = MULTI_FUNC_FLAG(MF_SUSPEND_TRACKBACK) ? "USE TKBACK" : "SUS TKBACK";
                break;
            }
            activeFunction++;
            FALLTHROUGH;
        case MULTI_FUNC_4:
#ifdef USE_DSHOT
            if (!ARMING_FLAG(ARMED) && STATE(MULTIROTOR)) {
                message = MULTI_FUNC_FLAG(MF_TURTLE_MODE) ? "END TURTLE" : "USE TURTLE";
                break;
            }
#endif
            activeFunction++;
            FALLTHROUGH;
        case MULTI_FUNC_5:
            if (!ARMING_FLAG(ARMED)) {
                message = "EMERG ARM ";
                break;
            }
            activeFunction++;
            FALLTHROUGH;
        case MULTI_FUNC_6:
            if (!ARMING_FLAG(ARMED)) {
#if defined(USE_MAG)
                if (sensors(SENSOR_MAG)) {
                    message = "CAL COMPAS";
                    break;
                }
#endif
#if defined(USE_GPS)
                if (isYawZeroResetAllowed()) {
                    message = "SET HEADIN";
                    break;
                }
#endif
            }
            activeFunction++;
            break;
        case MULTI_FUNC_END:
            message = "*FUNC SET*";
            break;
        }

        if (activeFunction != selectedFunction) {
            if (selectedFunction == MULTI_FUNC_1 && activeFunction == MULTI_FUNC_END) {  // no functions available so end process
                message = "*NO FUNCS*";
                setMultifunctionSelection(MULTI_FUNC_NONE);
            } else {
                setMultifunctionSelection(activeFunction);
                if (activeFunction == MULTI_FUNC_END) {   // no messages to display so return
                    return elemAttr;
                }
            }
        }

        strcpy(buff, message);

        return elemAttr;
    }
#endif  // MULTIFUNCTION - functions only, warnings always defined

    /* --- WARNINGS --- */
    const char *messages[8];
    uint8_t messageCount = 0;
    bool warningCondition = false;
    uint8_t warningFlagID = 1;

    // Low Battery Voltage
    const batteryState_e batteryVoltageState = checkBatteryVoltageState();
    warningCondition = batteryVoltageState == BATTERY_CRITICAL || batteryVoltageState == BATTERY_WARNING;
    if (osdCheckWarning(warningCondition, warningFlagID)) {
        messages[messageCount++] = batteryVoltageState == BATTERY_CRITICAL ? "VBATT LAND" : "VBATT LOW ";
    }

    // Low Battery Capacity
    if (batteryUsesCapacityThresholds()) {
        const batteryState_e batteryState = getBatteryState();
        warningCondition = batteryState == BATTERY_CRITICAL || batteryState == BATTERY_WARNING;
        if (osdCheckWarning(warningCondition, warningFlagID <<= 1)) {
            messages[messageCount++] = batteryState == BATTERY_CRITICAL ? "BATT EMPTY" : "BATT DYING";
        }
    }
#if defined(USE_GPS)
    // GPS Fix and Failure
    if (feature(FEATURE_GPS)) {
        if (osdCheckWarning(!STATE(GPS_FIX), warningFlagID <<= 1)) {
            bool gpsFailed = getHwGPSStatus() == HW_SENSOR_UNAVAILABLE;
            messages[messageCount++] = gpsFailed ? "GPS FAILED" : "NO GPS FIX";
        }
    }

    // RTH sanity (warning if RTH heads 200m further away from home than closest point)
    warningCondition = NAV_Status.state == MW_NAV_STATE_RTH_ENROUTE && !posControl.flags.rthTrackbackActive &&
                       (posControl.homeDistance - posControl.rthSanityChecker.minimalDistanceToHome) > 20000;
    if (osdCheckWarning(warningCondition, warningFlagID <<= 1)) {
        messages[messageCount++] = "RTH SANITY";
    }

    // Altitude sanity (warning if significant mismatch between estimated and GPS altitude)
    if (osdCheckWarning(posControl.flags.gpsCfEstimatedAltitudeMismatch, warningFlagID <<= 1)) {
        messages[messageCount++] = "ALT SANITY";
    }
#endif

#if defined(USE_MAG)
    // Magnetometer failure
    if (requestedSensors[SENSOR_INDEX_MAG] != MAG_NONE) {
        hardwareSensorStatus_e magStatus = getHwCompassStatus();
        if (osdCheckWarning(magStatus == HW_SENSOR_UNAVAILABLE || magStatus == HW_SENSOR_UNHEALTHY, warningFlagID <<= 1)) {
            messages[messageCount++] = "MAG FAILED";
        }
    }
#endif

#if defined(USE_PITOT)
    // Pitot sensor validation failure (blocked/failed pitot tube)
    if (sensors(SENSOR_PITOT) && detectedSensors[SENSOR_INDEX_PITOT] != PITOT_VIRTUAL) {
        if (osdCheckWarning(pitotHasFailed(), warningFlagID <<= 1)) {
            messages[messageCount++] = "PITOT FAIL";
        }
    }
#endif

    // Vibration levels   TODO - needs better vibration measurement to be useful
    // const float vibrationLevel = accGetVibrationLevel();
    // warningCondition = vibrationLevel > 1.5f;
    // if (osdCheckWarning(warningCondition, warningFlagID <<= 1, &warningsCount)) {
        // messages[messageCount++] = vibrationLevel > 2.5f ? "BAD VIBRTN" : "VIBRATION!";
    // }

#ifdef USE_DEV_TOOLS
    if (osdCheckWarning(systemConfig()->groundTestMode, warningFlagID <<= 1)) {
        messages[messageCount++] = "GRD TEST !";
    }
#endif

    if (messageCount) {
        message = messages[(millis() / 1000) % messageCount];     // display each warning on 1s cycle
        strcpy(buff, message);
        if (multiFunctionWarning.newWarningActive) {
            TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
        }
    }

    return elemAttr;
}

