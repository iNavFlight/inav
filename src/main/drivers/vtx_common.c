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

#include "vtx_common.h"

static vtxDevice_t *commonVtxDevice = NULL;

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

void vtxCommonSetFrequency(vtxDevice_t *vtxDevice, uint16_t frequency)
{
    if (vtxDevice && vtxDevice->vTable->setFrequency) {
        vtxDevice->vTable->setFrequency(vtxDevice, frequency);
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

bool vtxCommonGetDeviceCapability(vtxDevice_t *vtxDevice, vtxDeviceCapability_t *pDeviceCapability)
{
    if (vtxDevice) {
        memcpy(pDeviceCapability, &vtxDevice->capability, sizeof(vtxDeviceCapability_t));
        return true;
    }
    return false;
}
