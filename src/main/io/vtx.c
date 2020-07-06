/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "platform.h"

#include "common/maths.h"
#include "common/time.h"
#include "common/utils.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/vtx_common.h"

#include "fc/cli.h"
#include "fc/config.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"

#include "flight/failsafe.h"

#include "io/vtx.h"
#include "io/vtx_string.h"
#include "io/vtx_control.h"

#include "navigation/navigation.h"

PG_REGISTER_WITH_RESET_TEMPLATE(vtxSettingsConfig_t, vtxSettingsConfig, PG_VTX_SETTINGS_CONFIG, 2);

PG_RESET_TEMPLATE(vtxSettingsConfig_t, vtxSettingsConfig,
    .band = VTX_SETTINGS_DEFAULT_BAND,
    .channel = VTX_SETTINGS_DEFAULT_CHANNEL,
    .power = VTX_SETTINGS_DEFAULT_POWER,
    .pitModeChan = VTX_SETTINGS_DEFAULT_PITMODE_CHANNEL,
    .lowPowerDisarm = VTX_LOW_POWER_DISARM_OFF,
    .maxPowerOverride = 0,
    .useAutoPowerLevel = false,
    .maxDistanceReachableAt25 = 130,
);

typedef enum {
    VTX_PARAM_POWER = 0,
    VTX_PARAM_BANDCHAN,
    VTX_PARAM_PITMODE,
    VTX_PARAM_COUNT
} vtxScheduleParams_e;

vtxAutoPowerState_t autoPowerState = {
    .validUntill = 0,
    .validFrom = 0
};

int vtxAutoPowerValidaUntill(int maxDistanceReachableAt25, uint16_t currentPowerMw){
    const float sqrt2 = sqrt(2.0);
    const float exp = log2(((float)currentPowerMw)/25.0);
    return maxDistanceReachableAt25 * pow(sqrt2,exp);
}

int vtxAutoGetPower(vtxDevice_t *vtxDevice){

    if(feature(FEATURE_GPS) && STATE(GPS_FIX) 
    && vtxSettingsConfig()->useAutoPowerLevel) {
        
        uint8_t powerLevel;
        char * error;
        const uint8_t histeresisDeadband = 2;   //Histeresis deadband in meter, 
        uint8_t histeresisDeadbandHalf = histeresisDeadband/2;

        if(autoPowerState.validFrom < histeresisDeadbandHalf){
            //If UAV is on the first range step, disable histeresis
            histeresisDeadbandHalf = 0;
        }

        vtxCommonGetPowerIndex(vtxDevice,&powerLevel);

        if(powerLevel > vtxDevice->capability.powerCount)
            return vtxDevice->powerIndex;

        if(powerLevel < vtxDevice->capability.powerCount && GPS_distanceToHome > autoPowerState.validUntill + histeresisDeadbandHalf)
            powerLevel++;
        else if (powerLevel > 1 && GPS_distanceToHome < autoPowerState.validFrom - histeresisDeadbandHalf)
            powerLevel--;

        //Update validFrom
        if(powerLevel <= 1){
                autoPowerState.validFrom = 0;
        }
        else {
            char * prevPowerLevel = vtxDevice->capability.powerNames[powerLevel - 1];
            uint16_t num = (uint16_t)strtol(prevPowerLevel, &error, 10);
            if(*error!='\0'){
                return vtxDevice->powerIndex;
            }
            autoPowerState.validFrom = vtxAutoPowerValidaUntill(vtxSettingsConfig()->maxDistanceReachableAt25,num);
        }

        //Update validUntill
        if(powerLevel == vtxDevice->capability.powerCount)
            autoPowerState.validUntill = UINT32_MAX;
         else 
        {
            char * currentPowerLevel = vtxDevice->capability.powerNames[powerLevel];            
            uint16_t num = (uint16_t)strtol(currentPowerLevel, &error, 10);
            if(*error!='\0') {
                return vtxDevice->powerIndex;
            }
            autoPowerState.validUntill = vtxAutoPowerValidaUntill(vtxSettingsConfig()->maxDistanceReachableAt25,num);  
        } 

        return powerLevel;
    }
    return vtxDevice->powerIndex;
}

void vtxInit(void)
{
}

static vtxSettingsConfig_t * vtxGetRuntimeSettings(void)
{
    static vtxSettingsConfig_t settings;

    settings.band = vtxSettingsConfig()->band;
    settings.channel = vtxSettingsConfig()->channel;
    settings.pitModeChan = vtxSettingsConfig()->pitModeChan;
    settings.lowPowerDisarm = vtxSettingsConfig()->lowPowerDisarm;

    if(vtxSettingsConfig()->useAutoPowerLevel)
        settings.power = vtxAutoGetPower(vtxCommonDevice());
    else 
        settings.power = vtxSettingsConfig()->power;

    if (!ARMING_FLAG(ARMED) && !failsafeIsActive() &&
        ((settings.lowPowerDisarm == VTX_LOW_POWER_DISARM_ALWAYS) ||
        (settings.lowPowerDisarm == VTX_LOW_POWER_DISARM_UNTIL_FIRST_ARM && !ARMING_FLAG(WAS_EVER_ARMED)))) {

        settings.power = VTX_SETTINGS_DEFAULT_POWER;
    }

    return &settings;
}

static bool vtxProcessBandAndChannel(vtxDevice_t *vtxDevice, const vtxSettingsConfig_t * runtimeSettings)
{
    uint8_t vtxBand;
    uint8_t vtxChan;

    // Shortcut for undefined band
    if (!runtimeSettings->band) {
        return false;
    }

    if (!vtxCommonGetBandAndChannel(vtxDevice, &vtxBand, &vtxChan)) {
        return false;
    }

    if (vtxBand != runtimeSettings->band || vtxChan != runtimeSettings->channel) {
        vtxCommonSetBandAndChannel(vtxDevice, runtimeSettings->band, runtimeSettings->channel);
        return true;
    }

    return false;
}

static bool vtxProcessPower(vtxDevice_t *vtxDevice, const vtxSettingsConfig_t * runtimeSettings)
{
    uint8_t vtxPower;

    if (!vtxCommonGetPowerIndex(vtxDevice, &vtxPower)) {
        return false;
    }

    if (vtxPower != runtimeSettings->power) {
        vtxCommonSetPowerByIndex(vtxDevice, runtimeSettings->power);
        return true;
    }

    return false;
}

static bool vtxProcessPitMode(vtxDevice_t *vtxDevice, const vtxSettingsConfig_t * runtimeSettings)
{
    UNUSED(runtimeSettings);

    uint8_t pitOnOff;

    bool        currPmSwitchState = false;
    static bool prevPmSwitchState = false;

    if (!vtxCommonGetPitMode(vtxDevice, &pitOnOff)) {
        return false;
    }

    if (currPmSwitchState != prevPmSwitchState) {
        prevPmSwitchState = currPmSwitchState;

        if (currPmSwitchState) {
            if (0) {
                if (!pitOnOff) {
                    vtxCommonSetPitMode(vtxDevice, true);
                    return true;
                }
            }
        } else {
            if (pitOnOff) {
                vtxCommonSetPitMode(vtxDevice, false);
                return true;
            }
        }
    }

    return false;
}

void vtxUpdate(timeUs_t currentTimeUs)
{
    static uint8_t currentSchedule = 0;

    if (cliMode) {
        return;
    }

    vtxDevice_t *vtxDevice = vtxCommonDevice();
    if (vtxDevice) {
        // Check input sources for config updates
        vtxControlInputPoll();

        // Build runtime settings
        const vtxSettingsConfig_t * runtimeSettings = vtxGetRuntimeSettings();

        switch (currentSchedule) {
            case VTX_PARAM_POWER:
                vtxProcessPower(vtxDevice, runtimeSettings);
                break;
            case VTX_PARAM_BANDCHAN:
                vtxProcessBandAndChannel(vtxDevice, runtimeSettings);
                break;
            case VTX_PARAM_PITMODE:
                vtxProcessPitMode(vtxDevice, runtimeSettings);
                break;
            default:
                break;
        }

        vtxCommonProcess(vtxDevice, currentTimeUs);

        currentSchedule = (currentSchedule + 1) % VTX_PARAM_COUNT;
    }
}
