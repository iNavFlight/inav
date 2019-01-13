/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "cms/cms.h"

#include "common/printf.h"
#include "common/utils.h"

#include "fc/rc_controls.h"
#include "fc/runtime_config.h"

#include "io/beeper.h"
#include "io/rcdevice_cam.h"
#include "io/rcdevice.h"

#include "rx/rx.h"

#ifdef USE_RCDEVICE

#define IS_HI(X) (rcData[X] > 1750)
#define IS_LO(X) (rcData[X] < 1250)
#define IS_MID(X) (rcData[X] > 1250 && rcData[X] < 1750)

static runcamDevice_t runcamDevice;
runcamDevice_t *camDevice = &runcamDevice;

rcdeviceSwitchState_t switchStates[BOXCAMERA3 - BOXCAMERA1 + 1];
bool rcdeviceInMenu;
bool caddxdeviceInMenu;
bool needRelease = false;

static bool isFeatureSupported(uint8_t feature)
{
    if (camDevice->info.features & feature) {
        return true;
    }

    return false;
}

static bool rcdeviceIsCameraControlEnabled(void)
{
    bool isPowerSimulationSupported = isFeatureSupported(RCDEVICE_PROTOCOL_FEATURE_SIMULATE_POWER_BUTTON);
    bool isWiFiSimulationSupported = isFeatureSupported(RCDEVICE_PROTOCOL_FEATURE_SIMULATE_WIFI_BUTTON);
    bool isChangeModeSupported = isFeatureSupported(RCDEVICE_PROTOCOL_FEATURE_CHANGE_MODE);
    if (camDevice->serialPort != NULL && (camDevice->info.protocolVersion != CADDXDEVICE_PROTOCOL_TURTLEV2) && (isPowerSimulationSupported || isWiFiSimulationSupported || isChangeModeSupported)) {
        return true;
    }

    return false;
}

static bool caddxdeviceIsCameraControlEnabled(void)
{
    if (camDevice->serialPort != NULL && (camDevice->info.protocolVersion == CADDXDEVICE_PROTOCOL_TURTLEV2)) {
        return true;
    }
    return false;
}

bool rcdeviceIsEnabled(void)
{
    bool is5KeySimulationSupported = isFeatureSupported(RCDEVICE_PROTOCOL_FEATURE_SIMULATE_5_KEY_OSD_CABLE);

    if (camDevice->serialPort != NULL && (caddxdeviceIsCameraControlEnabled() || rcdeviceIsCameraControlEnabled() || is5KeySimulationSupported)) {
        return true;
    }

    return false;
}

static bool rcdeviceIs5KeyEnabled(void)
{
    if (camDevice->serialPort != NULL && (camDevice->info.protocolVersion != CADDXDEVICE_PROTOCOL_TURTLEV2) && isFeatureSupported(RCDEVICE_PROTOCOL_FEATURE_SIMULATE_5_KEY_OSD_CABLE)) {
        return true;
    }

    return false;
}

bool caddxdeviceIsEnabled(void)
{
    if ((camDevice->serialPort != NULL) && (camDevice->info.protocolVersion == CADDXDEVICE_PROTOCOL_TURTLEV2)) {
        return true;
    }

    return false;
}

static bool caddxdeviceIs5KeyEnabled(void)
{
    if ((camDevice->serialPort != NULL) && (camDevice->info.protocolVersion == CADDXDEVICE_PROTOCOL_TURTLEV2)) {
        return true;
    }

    return false;
}


static void rcdeviceCameraUpdateTime(void)
{
    static bool hasSynchronizedTime = false;
    // don't try more than 3 times to avoid overloading the CPU if
    // the camera doesn't accept the command for some reason.
    static int retries = 0;
    runcamDeviceWriteSettingResponse_t updateSettingResponse;
    // Format is yyyyMMddThhmmss.0 plus null terminator, hence 18
    // characters.
    char buf[18];
    dateTime_t dt;

    if (isFeatureSupported(RCDEVICE_PROTOCOL_FEATURE_DEVICE_SETTINGS_ACCESS) &&
        !hasSynchronizedTime && retries < 3) {

        if (rtcGetDateTimeLocal(&dt)) {
            retries++;
            tfp_sprintf(buf, "%04d%02d%02dT%02d%02d%02d.0",
                dt.year, dt.month, dt.day,
                dt.hours, dt.minutes, dt.seconds);

            bool ok = runcamDeviceWriteSetting(camDevice, RCDEVICE_PROTOCOL_SETTINGID_CAMERA_TIME,
                buf, sizeof(buf), &updateSettingResponse);
            if (ok && updateSettingResponse.resultCode == 0) {
                hasSynchronizedTime = true;
            }
        }
    }
}

static void rcdeviceCameraControlProcess(void)
{
    for (boxId_e i = BOXCAMERA1; i <= BOXCAMERA3; i++) {
        uint8_t switchIndex = i - BOXCAMERA1;

        if (IS_RC_MODE_ACTIVE(i)) {
            // check last state of this mode, if it's true, then ignore it.
            // Here is a logic to make a toggle control for this mode
            if (switchStates[switchIndex].isActivated) {
                continue;
            }

            uint8_t behavior = RCDEVICE_PROTOCOL_CAM_CTRL_UNKNOWN_CAMERA_OPERATION;
            switch (i) {
            case BOXCAMERA1:
                if (isFeatureSupported(RCDEVICE_PROTOCOL_FEATURE_SIMULATE_WIFI_BUTTON)) {
                    behavior = RCDEVICE_PROTOCOL_CAM_CTRL_SIMULATE_WIFI_BTN;
                }
                break;
            case BOXCAMERA2:
                if (isFeatureSupported(RCDEVICE_PROTOCOL_FEATURE_SIMULATE_POWER_BUTTON)) {
                    behavior = RCDEVICE_PROTOCOL_CAM_CTRL_SIMULATE_POWER_BTN;
                }
                break;
            case BOXCAMERA3:
                if (isFeatureSupported(RCDEVICE_PROTOCOL_FEATURE_CHANGE_MODE)) {
                    behavior = RCDEVICE_PROTOCOL_CAM_CTRL_CHANGE_MODE;
                }
                break;
            default:
                break;
            }
            if (behavior != RCDEVICE_PROTOCOL_CAM_CTRL_UNKNOWN_CAMERA_OPERATION) {
                runcamDeviceSimulateCameraButton(camDevice, behavior);
                switchStates[switchIndex].isActivated = true;
            }
        } else {
            switchStates[switchIndex].isActivated = false;
        }
    }
    rcdeviceCameraUpdateTime();
}

static void caddxdeviceCameraControlProcess(void)
{
    for (boxId_e i = BOXCAMERA1; i <= BOXCAMERA3; i++) {
        uint8_t switchIndex = i - BOXCAMERA1;

        if (IS_RC_MODE_ACTIVE(i)) {
            // check last state of this mode, if it's true, then ignore it.
            // Here is a logic to make a toggle control for this mode
            if (switchStates[switchIndex].isActivated) {
                continue;
            }

            uint8_t behavior = CADDX_PROTOCOL_CAM_CTRL_UNKNOWN_CAMERA_OPERATION;
            uint8_t command = CADDX_PROTOCOL_CAM_CTRL_UNKNOWN_CAMERA_COMMAND;
            switch (i) {
            case BOXCAMERA1:
                command = CADDX_PROTOCOL_CAM_CTRL_RECORD;
                behavior = CADDX_PROTOCOL_CAM_CTRL_START_RECORDING;
                break;
            case BOXCAMERA2:
                command = CADDX_PROTOCOL_CAM_CTRL_RECORD;
                behavior = CADDX_PROTOCOL_CAM_CTRL_STOP_RECORDING;
                break;
            case BOXCAMERA3:
                command = CADDX_PROTOCOL_CAM_CTRL_PHOTO;
                behavior = CADDX_PROTOCOL_CAM_CTRL_TAKE_A_PHOTO;
                break;
            default:
                break;
            }
            if (behavior != CADDX_PROTOCOL_CAM_CTRL_UNKNOWN_CAMERA_OPERATION) {
                caddxDeviceSendPacket(camDevice, command, behavior);
                switchStates[switchIndex].isActivated = true;
            }
        } else {
            switchStates[switchIndex].isActivated = false;
        }
    }
}

static bool rcdeviceCamSimulate5KeyCablePress(rcdeviceCamSimulationKeyEvent_e key)
{
    uint8_t operation = RCDEVICE_PROTOCOL_5KEY_SIMULATION_NONE;
    switch (key) {
    case RCDEVICE_CAM_KEY_LEFT:
        operation = RCDEVICE_PROTOCOL_5KEY_SIMULATION_LEFT;
        break;
    case RCDEVICE_CAM_KEY_UP:
        operation = RCDEVICE_PROTOCOL_5KEY_SIMULATION_UP;
        break;
    case RCDEVICE_CAM_KEY_RIGHT:
        operation = RCDEVICE_PROTOCOL_5KEY_SIMULATION_RIGHT;
        break;
    case RCDEVICE_CAM_KEY_DOWN:
        operation = RCDEVICE_PROTOCOL_5KEY_SIMULATION_DOWN;
        break;
    case RCDEVICE_CAM_KEY_ENTER:
        operation = RCDEVICE_PROTOCOL_5KEY_SIMULATION_SET;
        break;
    default:
        operation = RCDEVICE_PROTOCOL_5KEY_SIMULATION_NONE;
        break;
    }

    return runcamDeviceSimulate5KeyOSDCableButtonPress(camDevice, operation);
}

static bool rcdeviceSend5KeyOSDCableSimualtionEvent(rcdeviceCamSimulationKeyEvent_e key)
{
    bool reqResult = false;
    switch (key) {
    case RCDEVICE_CAM_KEY_CONNECTION_OPEN:
        reqResult = runcamDeviceOpen5KeyOSDCableConnection(camDevice);
        if (reqResult) {
            rcdeviceInMenu = true;
            beeper(BEEPER_CAM_CONNECTION_OPEN);
        }
        break;
    case RCDEVICE_CAM_KEY_CONNECTION_CLOSE:
        reqResult = runcamDeviceClose5KeyOSDCableConnection(camDevice);
        if (reqResult) {
            rcdeviceInMenu = false;
            beeper(BEEPER_CAM_CONNECTION_CLOSE);
        }
        break;
    case RCDEVICE_CAM_KEY_ENTER:
    case RCDEVICE_CAM_KEY_LEFT:
    case RCDEVICE_CAM_KEY_UP:
    case RCDEVICE_CAM_KEY_RIGHT:
    case RCDEVICE_CAM_KEY_DOWN:
        reqResult = rcdeviceCamSimulate5KeyCablePress(key);
        break;
    case RCDEVICE_CAM_KEY_RELEASE:
        reqResult = runcamDeviceSimulate5KeyOSDCableButtonRelease(camDevice);
        break;
    default:
        reqResult = false;
        break;
    }

    return reqResult;
}

static bool caddxdeviceSend5KeyOSDCableSimualtionEvent(caddxdeviceCamSimulationKeyEvent_e key)
{
    bool reqResult = false;
    switch (key) {
    case CADDXDEVICE_CAM_KEY_CONNECTION_OPEN:
        caddxDeviceSendPacket(camDevice, CADDX_PROTOCOL_5KEY_INSTRUCTION, RCDEVICE_PROTOCOL_5KEY_SIMULATION_SET);
		    reqResult = true;
        caddxdeviceInMenu = true;
        beeper(BEEPER_CAM_CONNECTION_OPEN);
        break;
    case CADDXDEVICE_CAM_KEY_CONNECTION_CLOSE:
        caddxdeviceInMenu = false;
        beeper(BEEPER_CAM_CONNECTION_CLOSE);
        break;
    case CADDXDEVICE_CAM_KEY_ENTER:
	      caddxDeviceSendPacket(camDevice, CADDX_PROTOCOL_5KEY_INSTRUCTION, RCDEVICE_PROTOCOL_5KEY_SIMULATION_SET);
        reqResult = true;
		break;
    case CADDXDEVICE_CAM_KEY_LEFT:
	      caddxDeviceSendPacket(camDevice, CADDX_PROTOCOL_5KEY_INSTRUCTION, RCDEVICE_PROTOCOL_5KEY_SIMULATION_LEFT);
        reqResult = true;
		break;
    case CADDXDEVICE_CAM_KEY_UP:
	      caddxDeviceSendPacket(camDevice, CADDX_PROTOCOL_5KEY_INSTRUCTION, RCDEVICE_PROTOCOL_5KEY_SIMULATION_UP);
        reqResult = true;
		break;
    case CADDXDEVICE_CAM_KEY_RIGHT:
	      caddxDeviceSendPacket(camDevice, CADDX_PROTOCOL_5KEY_INSTRUCTION, RCDEVICE_PROTOCOL_5KEY_SIMULATION_RIGHT);
        reqResult = true;
		break;
    case CADDXDEVICE_CAM_KEY_DOWN:
	      caddxDeviceSendPacket(camDevice, CADDX_PROTOCOL_5KEY_INSTRUCTION, RCDEVICE_PROTOCOL_5KEY_SIMULATION_DOWN);
        reqResult = true;
		break;
    case CADDXDEVICE_CAM_KEY_RELEASE:
        reqResult = true;
        break;
    default:
        reqResult = false;
        break;
    }

    return reqResult;
}

static void rcdevice5KeySimulationProcess(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);

#ifdef USE_CMS
    if (cmsInMenu) {
        return;
    }
#endif

    if (camDevice->serialPort == 0) {
        return;
    }

    rcdeviceCamSimulationKeyEvent_e key = RCDEVICE_CAM_KEY_NONE;

    if (needRelease) {
        if (IS_MID(YAW) && IS_MID(PITCH) && IS_MID(ROLL)) {
            key = RCDEVICE_CAM_KEY_RELEASE;
            if (rcdeviceSend5KeyOSDCableSimualtionEvent(key)) {
                needRelease = false;
            } else {
                rcdeviceInMenu = false;
            }
            return;
        } else {
            return;
        }
    } else {
        if (IS_MID(THROTTLE) && IS_MID(ROLL) && IS_MID(PITCH) && IS_LO(YAW)) { // Disconnect HI YAW
            if (rcdeviceInMenu) {
                key = RCDEVICE_CAM_KEY_CONNECTION_CLOSE;
            }
        } else {
            if (rcdeviceInMenu) {
                if (IS_LO(ROLL)) { // Left LO ROLL
                    key = RCDEVICE_CAM_KEY_LEFT;
                } else if (IS_HI(PITCH)) { // Up HI PITCH
                    key = RCDEVICE_CAM_KEY_UP;
                } else if (IS_HI(ROLL)) { // Right HI ROLL
                    key = RCDEVICE_CAM_KEY_RIGHT;
                } else if (IS_LO(PITCH)) { // Down LO PITCH
                    key = RCDEVICE_CAM_KEY_DOWN;
                } else if (IS_MID(THROTTLE) && IS_MID(ROLL) && IS_MID(PITCH) && IS_HI(YAW)) { // Enter HI YAW
                    key = RCDEVICE_CAM_KEY_ENTER;
                }
            } else {
                if (IS_MID(THROTTLE) && IS_MID(ROLL) && IS_MID(PITCH) && IS_HI(YAW) && !ARMING_FLAG(ARMED)) { // Enter HI YAW
                    key = RCDEVICE_CAM_KEY_CONNECTION_OPEN;
                }
            }
        }
    }

    if (key != RCDEVICE_CAM_KEY_NONE) {
        if (rcdeviceSend5KeyOSDCableSimualtionEvent(key)) {
            needRelease = true;
        } else {
            rcdeviceInMenu = false;
        }
    }
}

static void caddxdevice5KeySimulationProcess(void)
{
#ifdef USE_CMS
    if (cmsInMenu) {
        return;
    }
#endif

    if (camDevice->serialPort == 0) {
        return;
    }

    caddxdeviceCamSimulationKeyEvent_e key = CADDXDEVICE_CAM_KEY_NONE;

    if (needRelease) {
        if (IS_MID(YAW) && IS_MID(PITCH) && IS_MID(ROLL)) {
            key = CADDXDEVICE_CAM_KEY_RELEASE;
            if (caddxdeviceSend5KeyOSDCableSimualtionEvent(key)) {
                needRelease = false;
            } else {
                caddxdeviceInMenu = false;
            }
            return;
        } else {
            return;
        }
    } else {
        if (IS_MID(THROTTLE) && IS_MID(ROLL) && IS_MID(PITCH) && IS_LO(YAW)) { // Disconnect HI YAW
            if (caddxdeviceInMenu) {
                key = CADDXDEVICE_CAM_KEY_CONNECTION_CLOSE;
            }
        } else {
            if (caddxdeviceInMenu) {
                if (IS_LO(ROLL)) { // Left LO ROLL
                    key = CADDXDEVICE_CAM_KEY_LEFT;
                } else if (IS_HI(PITCH)) { // Up HI PITCH
                    key = CADDXDEVICE_CAM_KEY_UP;
                } else if (IS_HI(ROLL)) { // Right HI ROLL
                    key = CADDXDEVICE_CAM_KEY_RIGHT;
                } else if (IS_LO(PITCH)) { // Down LO PITCH
                    key = CADDXDEVICE_CAM_KEY_DOWN;
                } else if (IS_MID(THROTTLE) && IS_MID(ROLL) && IS_MID(PITCH) && IS_HI(YAW)) { // Enter HI YAW
                    key = CADDXDEVICE_CAM_KEY_ENTER;
                }
            } else {
                if (IS_MID(THROTTLE) && IS_MID(ROLL) && IS_MID(PITCH) && IS_HI(YAW) && !ARMING_FLAG(ARMED)) { // Enter HI YAW
                    key = CADDXDEVICE_CAM_KEY_CONNECTION_OPEN;
                }
            }
        }
    }

    if (key != CADDXDEVICE_CAM_KEY_NONE) {
        if (caddxdeviceSend5KeyOSDCableSimualtionEvent(key)) {
            needRelease = true;
        } else {
            caddxdeviceInMenu = false;
        }
    }
}

void rcdeviceUpdate(timeUs_t currentTimeUs)
{
    if (rcdeviceIsCameraControlEnabled()) {
        rcdeviceCameraControlProcess();
    }

    if (rcdeviceIs5KeyEnabled()) {
        rcdevice5KeySimulationProcess(currentTimeUs);
    }
	    		
	if (caddxdeviceIsCameraControlEnabled()) {
        caddxdeviceCameraControlProcess();
    }
	
	if (caddxdeviceIs5KeyEnabled()) {
        caddxdevice5KeySimulationProcess();
    }
}

bool rcdeviceInit(void)
{
    // open serial port
    if (!runcamDeviceInit(camDevice)) {
        return false;
    }

    for (boxId_e i = BOXCAMERA1; i <= BOXCAMERA3; i++) {
        uint8_t switchIndex = i - BOXCAMERA1;
        switchStates[switchIndex].isActivated = true;
    }
    return true;
}

#endif
