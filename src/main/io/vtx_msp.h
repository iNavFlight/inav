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

#pragma once

#include <stdint.h>

#include "build/build_config.h"

typedef enum {
    // Offline - device hasn't responded yet
    MSP_VTX_STATUS_OFFLINE = 0,
    MSP_VTX_STATUS_READY,
} mspVtxStatus_e;

typedef struct mspPowerTable_s {
    int mW;         // rfpower
    int16_t dbi;    // valueV1
} mspPowerTable_t;

#define VTX_MSP_MAX_POWER_COUNT 5
#define VTX_MSP_DEFAULT_POWER_COUNT 4

#define VTX_MSP_TABLE_MAX_BANDS             5 // default freq table has 5 bands
#define VTX_MSP_TABLE_MAX_CHANNELS          8 // and eight channels
#define VTX_MSP_TABLE_MAX_POWER_LEVELS      5 //max of VTX_TRAMP_POWER_COUNT, VTX_SMARTAUDIO_POWER_COUNT and VTX_RTC6705_POWER_COUNT
#define VTX_MSP_TABLE_CHANNEL_NAME_LENGTH   1
#define VTX_MSP_TABLE_BAND_NAME_LENGTH      8
#define VTX_MSP_TABLE_POWER_LABEL_LENGTH    3


bool vtxMspInit(void);
void setMspVtxDeviceStatusReady(const int descriptor);
void prepareMspFrame(uint8_t *mspFrame);
