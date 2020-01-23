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

#pragma once

#include "common/time.h"

#define VTX_SETTINGS_DEFAULT_LOW_POWER_DISARM   0

#define VTX_SETTINGS_MIN_FREQUENCY_MHZ 0             //min freq (in MHz) for 'vtx_freq' setting
#define VTX_SETTINGS_MAX_FREQUENCY_MHZ 5999          //max freq (in MHz) for 'vtx_freq' setting

#if defined(USE_VTX_RTC6705)

#include "drivers/vtx_rtc6705.h"

#endif

#if defined(USE_VTX_SMARTAUDIO) || defined(USE_VTX_TRAMP)


#define VTX_SETTINGS_MAX_POWER      (VTX_SETTINGS_POWER_COUNT - VTX_SETTINGS_MIN_POWER + 1)

#elif defined(USE_VTX_RTC6705)

#define VTX_SETTINGS_POWER_COUNT    VTX_RTC6705_POWER_COUNT
#define VTX_SETTINGS_DEFAULT_POWER  VTX_RTC6705_DEFAULT_POWER
#define VTX_SETTINGS_MIN_POWER      VTX_RTC6705_MIN_POWER
#define VTX_SETTINGS_MAX_POWER      (VTX_SETTINGS_POWER_COUNT - 1)

#endif

// check value for MSP_SET_VTX_CONFIG to determine if value is encoded
// band/channel or frequency in MHz (3 bits for band and 3 bits for channel)
#define VTXCOMMON_MSP_BANDCHAN_CHKVAL ((uint16_t)((7 << 3) + 7))

typedef enum {
    VTXDEV_UNSUPPORTED = 0, // reserved for MSP
    VTXDEV_RTC6705    = 1,
    // 2 reserved
    VTXDEV_SMARTAUDIO = 3,
    VTXDEV_TRAMP      = 4,
    VTXDEV_FFPV       = 5,
    VTXDEV_UNKNOWN    = 0xFF,
} vtxDevType_e;

struct vtxVTable_s;

typedef struct vtxDevice_s {
    const struct vtxVTable_s * const vTable;
} vtxDevice_t;

// {set,get}BandAndChannel: band and channel are 1 origin
// {set,get}PowerByIndex: 0 = Power OFF, 1 = device dependent
// {set,get}PitMode: 0 = OFF, 1 = ON

typedef struct vtxVTable_s {
    void (*process)(vtxDevice_t *vtxDevice, timeUs_t currentTimeUs);
    vtxDevType_e (*getDeviceType)(const vtxDevice_t *vtxDevice);
    bool (*isReady)(const vtxDevice_t *vtxDevice);

    void (*setBandAndChannel)(vtxDevice_t *vtxDevice, uint8_t band, uint8_t channel);
    void (*setPowerByIndex)(vtxDevice_t *vtxDevice, uint8_t level);
    void (*setPitMode)(vtxDevice_t *vtxDevice, uint8_t onoff);
    void (*setFrequency)(vtxDevice_t *vtxDevice, uint16_t freq);

    bool (*getBandAndChannel)(const vtxDevice_t *vtxDevice, uint8_t *pBand, uint8_t *pChannel);
    bool (*getPowerIndex)(const vtxDevice_t *vtxDevice, uint8_t *pIndex);
    bool (*getPitMode)(const vtxDevice_t *vtxDevice, uint8_t *pOnOff);
    bool (*getFrequency)(const vtxDevice_t *vtxDevice, uint16_t *pFreq);
} vtxVTable_t;

// 3.1.0
// PIT mode is defined as LOWEST POSSIBLE RF POWER.
// - It can be a dedicated mode, or lowest RF power possible.
// - It is *NOT* RF on/off control ?

void vtxCommonInit(void);
void vtxCommonSetDevice(vtxDevice_t *vtxDevice);
vtxDevice_t *vtxCommonDevice(void);

// VTable functions
void vtxCommonProcess(vtxDevice_t *vtxDevice, timeUs_t currentTimeUs);
vtxDevType_e vtxCommonGetDeviceType(const vtxDevice_t *vtxDevice);
bool vtxCommonDeviceIsReady(const vtxDevice_t *vtxDevice);
void vtxCommonSetBandAndChannel(vtxDevice_t *vtxDevice, uint8_t band, uint8_t channel);
void vtxCommonSetPowerByIndex(vtxDevice_t *vtxDevice, uint8_t level);
void vtxCommonSetPitMode(vtxDevice_t *vtxDevice, uint8_t onoff);
void vtxCommonSetFrequency(vtxDevice_t *vtxDevice, uint16_t freq);
bool vtxCommonGetBandAndChannel(const vtxDevice_t *vtxDevice, uint8_t *pBand, uint8_t *pChannel);
bool vtxCommonGetPowerIndex(const vtxDevice_t *vtxDevice, uint8_t *pIndex);
bool vtxCommonGetPitMode(const vtxDevice_t *vtxDevice, uint8_t *pOnOff);
bool vtxCommonGetFrequency(const vtxDevice_t *vtxDevice, uint16_t *pFreq);
const char *vtxCommonLookupBandName(const vtxDevice_t *vtxDevice, int band);
char vtxCommonLookupBandLetter(const vtxDevice_t *vtxDevice, int band);
char vtxCommonGetBandLetter(const vtxDevice_t *vtxDevice, int band);
const char *vtxCommonLookupChannelName(const vtxDevice_t *vtxDevice, int channel);
uint16_t vtxCommonLookupFrequency(const vtxDevice_t *vtxDevice, int band, int channel);
bool vtxCommonLookupBandChan(const vtxDevice_t *vtxDevice, uint16_t freq, uint8_t *pBand, uint8_t *pChannel);
const char *vtxCommonLookupPowerName(const vtxDevice_t *vtxDevice, int index);
bool vtxCommonLookupPowerValue(const vtxDevice_t *vtxDevice, int index, uint16_t *pPowerValue);
