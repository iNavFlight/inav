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

/* Created by jflyper */

#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "platform.h"
#include "build/debug.h"

#include "common/typeconversion.h"
#include "common/log.h"

#include "vtx_common.h"

static vtxDevice_t *commonVtxDevice = NULL;

char powerNames[VTX_SETTINGS_POWER_COUNT][5];

PG_REGISTER_WITH_RESET_TEMPLATE(vtxPowerLevels_t, vtxPowerLevels, PG_VTX_POWER_LEVELS, 2);

PG_RESET_TEMPLATE(vtxPowerLevels_t, vtxPowerLevels,
    .powerTableMw[0] = VTX_DEFAULT_POWER_LEVEL,
    .powerTableMw[1] = VTX_DEFAULT_POWER_LEVEL,
    .powerTableMw[2] = VTX_DEFAULT_POWER_LEVEL,
    .powerTableMw[3] = VTX_DEFAULT_POWER_LEVEL,
    .powerTableMw[4] = VTX_DEFAULT_POWER_LEVEL,
);


void vtxCommonInit(void)
{
}

void vtxCommonSetDevice(vtxDevice_t *vtxDevice)
{
    commonVtxDevice = vtxDevice;
}

vtxDevice_t *vtxCommonDevice(void)
{
    return commonVtxDevice;
}

void vtxCommonProcess(vtxDevice_t *vtxDevice, timeUs_t currentTimeUs)
{
    if (vtxDevice && vtxDevice->vTable->process) {
        vtxDevice->vTable->process(vtxDevice, currentTimeUs);
    }
}

vtxDevType_e vtxCommonGetDeviceType(vtxDevice_t *vtxDevice)
{
    if (!vtxDevice || !vtxDevice->vTable->getDeviceType) {
        return VTXDEV_UNKNOWN;
    }

    return vtxDevice->vTable->getDeviceType(vtxDevice);
}

bool vtxCommonDeviceIsReady(vtxDevice_t *vtxDevice)
{
    if (vtxDevice && vtxDevice->vTable->isReady) {
        return vtxDevice->vTable->isReady(vtxDevice);
    }
    return false;
}

// band and channel are 1 origin
void vtxCommonSetBandAndChannel(vtxDevice_t *vtxDevice, uint8_t band, uint8_t channel)
{
    if (!vtxDevice)
        return;

    if ((band > vtxDevice->capability.bandCount) || (channel > vtxDevice->capability.channelCount))
        return;

    if (vtxDevice->vTable->setBandAndChannel) {
        vtxDevice->vTable->setBandAndChannel(vtxDevice, band, channel);
    }
}

// index is zero origin, zero = power off completely
void vtxCommonSetPowerByIndex(vtxDevice_t *vtxDevice, uint8_t index)
{
    if (!vtxDevice)
        return;

    if(vtxCommonHasCustomPowerLevels() && index > vtxCommonCustomPowerLevelsCount())
        return;
    
    if (index > vtxDevice->capability.powerCount)
        return;

    if (vtxDevice->vTable->setPowerByIndex) {
        vtxDevice->vTable->setPowerByIndex(vtxDevice, index);
    }
}

// on = 1, off = 0
void vtxCommonSetPitMode(vtxDevice_t *vtxDevice, uint8_t onoff)
{
    if (vtxDevice && vtxDevice->vTable->setPitMode) {
        vtxDevice->vTable->setPitMode(vtxDevice, onoff);
    }
}

bool vtxCommonGetBandAndChannel(vtxDevice_t *vtxDevice, uint8_t *pBand, uint8_t *pChannel)
{
    if (vtxDevice && vtxDevice->vTable->getBandAndChannel) {
        return vtxDevice->vTable->getBandAndChannel(vtxDevice, pBand, pChannel);
    }
    return false;
}

bool vtxCommonGetPowerIndex(vtxDevice_t *vtxDevice, uint8_t *pIndex)
{
    if (vtxDevice && vtxDevice->vTable->getPowerIndex) {
        return vtxDevice->vTable->getPowerIndex(vtxDevice, pIndex);
    }
    return false;
}

bool vtxCommonGetPitMode(vtxDevice_t *vtxDevice, uint8_t *pOnOff)
{
    if (vtxDevice && vtxDevice->vTable->getPitMode) {
        return vtxDevice->vTable->getPitMode(vtxDevice, pOnOff);
    }
    return false;
}

bool vtxCommonGetFrequency(const vtxDevice_t *vtxDevice, uint16_t *pFrequency)
{
    if (vtxDevice && vtxDevice->vTable->getFrequency) {
        return vtxDevice->vTable->getFrequency(vtxDevice, pFrequency);
    }
    return false;
}

inline bool vtxCommonHasCustomPowerLevels(void){
    return vtxPowerLevels()->usePowerTable;
}

int vtxCommonCustomPowerLevelsCount(void){
    int ris = 0;
    for(uint8_t i = 0; i<VTX_SETTINGS_POWER_COUNT; i++)
        if(vtxPowerLevels()->powerTableMw[i] != VTX_DEFAULT_POWER_LEVEL)
            ris++;
        else break;

    return ris;
}

bool vtxCommonOverridePowerNames(vtxDeviceCapability_t * deviceCapability){
    if(deviceCapability){

        bool forceEmptyString = false;
        //Used when in the power table there is a value with VTX_DEFAULT_POWER_LEVEL, so the next values will be an empty string
        
        for(uint8_t i = 0; i < VTX_SETTINGS_POWER_COUNT; i++){
            char buff[5];
            if(forceEmptyString || vtxPowerLevels()->powerTableMw[i]==VTX_DEFAULT_POWER_LEVEL){
                strcpy(buff,"");
                forceEmptyString = true;
            }
            else{
                itoa(vtxPowerLevels()->powerTableMw[i], buff, 10);
                strcpy(powerNames[i], buff);
            }

            if(i+1 < deviceCapability->powerCount)
                deviceCapability->powerNames[i+1] = powerNames[i];          
        }
        return true;
    }
    return false;
}

bool vtxCommonGetDeviceCapability(vtxDevice_t *vtxDevice, vtxDeviceCapability_t *pDeviceCapability)
{
    if (vtxDevice) {  
        if(vtxCommonHasCustomPowerLevels())
            vtxCommonOverridePowerNames(&vtxDevice->capability);
        memcpy(pDeviceCapability, &vtxDevice->capability, sizeof(vtxDeviceCapability_t));      
        return true;
    }
    return false;
}

bool vtxCommonGetPower(const vtxDevice_t *vtxDevice, uint8_t *pIndex, uint16_t *pPowerMw)
{
    if (vtxDevice && vtxDevice->vTable->getPower) {
        bool returnValue = vtxDevice->vTable->getPower(vtxDevice, pIndex, pPowerMw);

        if(*pIndex < VTX_SETTINGS_POWER_COUNT && vtxCommonHasCustomPowerLevels())
            *pPowerMw = vtxPowerLevels()->powerTableMw[*pIndex];

        return returnValue;
    }
    return false;
}

bool vtxCommonGetOsdInfo(vtxDevice_t *vtxDevice, vtxDeviceOsdInfo_t * pOsdInfo)
{
    bool ret = false;

    if (vtxDevice && vtxDevice->vTable->getOsdInfo) {
        ret = vtxDevice->vTable->getOsdInfo(vtxDevice, pOsdInfo);
    }

    if(pOsdInfo->powerIndex < VTX_SETTINGS_POWER_COUNT && pOsdInfo->powerIndex > 0 && vtxCommonHasCustomPowerLevels()){
        pOsdInfo->powerMilliwatt = vtxPowerLevels()->powerTableMw[pOsdInfo->powerIndex - 1];
    }

    // Make sure we provide sane results even in case API fails
    if (!ret) {
        pOsdInfo->band = 0;
        pOsdInfo->channel = 0;
        pOsdInfo->frequency = 0;
        pOsdInfo->powerIndex = 0;
        pOsdInfo->powerMilliwatt = 0;
        pOsdInfo->bandLetter = '-';
        pOsdInfo->bandName = "-";
        pOsdInfo->channelName = "-";
        pOsdInfo->powerIndexLetter = '0';
    }

    return ret;
}
