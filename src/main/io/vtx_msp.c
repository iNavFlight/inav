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
/* Created by geoffsim */

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "platform.h"

#if defined(USE_VTX_MSP) && defined(USE_VTX_CONTROL) && defined(USE_VTX_COMMON)

#include "common/log.h"
#include "common/crc.h"
#include "drivers/vtx_common.h"
#include "msp/msp_protocol.h"
#include "rx/rx.h"
#include "rx/crsf.h"
#include "telemetry/crsf.h"
#include "vtx.h"
#include "displayport_msp_osd.h"
#include "vtx_string.h"
#include "vtx_msp.h"

#define VTX_MSP_MIN_BAND           (1)
#define VTX_MSP_MAX_BAND           (VTX_MSP_MIN_BAND + VTX_MSP_BAND_COUNT - 1)
#define VTX_MSP_MIN_CHANNEL        (1)
#define VTX_MSP_MAX_CHANNEL        (VTX_MSP_MIN_CHANNEL + VTX_MSP_CHANNEL_COUNT -1)

#define VTX_UPDATE_REQ_NONE         0x00
#define VTX_UPDATE_REQ_FREQUENCY    0x01
#define VTX_UPDATE_REQ_POWER        0x02
#define VTX_UPDATE_REQ_PIT_MODE     0x04

typedef struct {
    bool ready;
    uint8_t timeouts;
    uint8_t updateReqMask;
    bool crsfTelemetryEnabled;

    struct {
        uint8_t     band;
        uint8_t     channel;
        uint16_t    freq;
        uint8_t     power;
        uint8_t     powerIndex;
        uint8_t     pitMode;
    } request;
;
} vtxProtoState_t;

const char * const vtxMspBandNames[VTX_MSP_BAND_COUNT + 1] = {
    "-----", "A 2.4", "B 2.4", "E 2.4", "F 2.4", "R 2.4"
};

const char * vtxMspBandLetters = "-ABEFR";

const char * const vtxMspChannelNames[VTX_MSP_CHANNEL_COUNT + 1] = {
    "-", "1", "2", "3", "4", "5", "6", "7", "8"
};

const char * const vtxMspPowerNames[VTX_MSP_POWER_COUNT + 1] = {
    "0", "25", "200", "500", "MAX"
};

const unsigned vtxMspPowerTable[VTX_MSP_POWER_COUNT + 1] = {
    0, 25, 200, 500, 1000
};

static serialPortIdentifier_e mspVtxPortIdentifier;
static vtxProtoState_t vtxState;

static vtxDevType_e vtxMspGetDeviceType(const vtxDevice_t *);
static bool vtxMspIsReady(const vtxDevice_t *);
static vtxDevice_t vtxMsp;

static void prepareMspFrame(vtxDevice_t *vtxDevice, uint8_t *mspFrame)
{
    // Send an MSP_VTX_V2 frame to the VTX
    mspFrame[0]  = vtxMspGetDeviceType(vtxDevice);
    mspFrame[1]  = vtxState.request.band;
    mspFrame[2]  = vtxState.request.channel;
    mspFrame[3]  = vtxState.request.powerIndex;
    mspFrame[4]  = vtxState.request.pitMode;
    mspFrame[5]  = 0; // Freq_L 
    mspFrame[6]  = 0; // Freq_H
    mspFrame[7]  = vtxMspIsReady(vtxDevice);
    mspFrame[8]  = vtxSettingsConfig()->lowPowerDisarm;
    mspFrame[9]  = 0; // pitmode freq Low
    mspFrame[10] = 0; // pitmode freq High
    mspFrame[11] = 0; // 1 if using vtx table
    mspFrame[12] = vtxMsp.capability.bandCount; // bands or 0
    mspFrame[13] = vtxMsp.capability.channelCount; // channels or 0
    mspFrame[14] = vtxMsp.capability.powerCount; // power levels or 0
} 

static void mspCrsfPush(const uint8_t mspCommand, const uint8_t *mspFrame, const uint8_t mspFrameSize)
{
#ifndef USE_TELEMETRY_CRSF
    UNUSED(mspCommand);
    UNUSED(mspFrame);
    UNUSED(mspFrameSize);
#else
    sbuf_t crsfPayloadBuf;
    sbuf_t *dst = &crsfPayloadBuf;

    uint8_t mspHeader[6] = {0x50, 0, mspCommand & 0xFF, (mspCommand >> 8) & 0xFF, mspFrameSize & 0xFF, (mspFrameSize >> 8) & 0xFF }; // MSP V2 over CRSF header
    uint8_t mspChecksum;

    mspChecksum = crc8_dvb_s2_update(0, &mspHeader[1], 5); // first character is not checksummable
    mspChecksum = crc8_dvb_s2_update(mspChecksum, mspFrame, mspFrameSize);

    uint8_t fullMspFrameSize = mspFrameSize + sizeof(mspHeader) + 1;  // add 1 for msp checksum
    uint8_t crsfFrameSize = CRSF_FRAME_LENGTH_EXT_TYPE_CRC + CRSF_FRAME_LENGTH_TYPE_CRC + fullMspFrameSize;

    uint8_t crsfFrame[crsfFrameSize];

    dst->ptr = crsfFrame;
    dst->end = ARRAYEND(crsfFrame);

    sbufWriteU8(dst, CRSF_SYNC_BYTE);
    sbufWriteU8(dst, fullMspFrameSize + CRSF_FRAME_LENGTH_EXT_TYPE_CRC); // size of CRSF frame (everything except sync and size itself)
    sbufWriteU8(dst, CRSF_FRAMETYPE_MSP_RESP); // CRSF type
    sbufWriteU8(dst, CRSF_ADDRESS_CRSF_RECEIVER); // response destination is the receiver the vtx connection
    sbufWriteU8(dst, CRSF_ADDRESS_FLIGHT_CONTROLLER); // origin is always this device
    sbufWriteData(dst, mspHeader, sizeof(mspHeader));
    sbufWriteData(dst, mspFrame, mspFrameSize);
    sbufWriteU8(dst, mspChecksum);
    crc8_dvb_s2_sbuf_append(dst, &crsfFrame[2]); // start at byte 2, since CRC does not include device address and frame length
    sbufSwitchToReader(dst, crsfFrame);

    crsfRxSendTelemetryData(); // give the FC a chance to send outstanding telemetry
    crsfRxWriteTelemetryData(sbufPtr(dst), sbufBytesRemaining(dst));
    crsfRxSendTelemetryData();
#endif
}

static void vtxMspProcess(vtxDevice_t *vtxDevice, timeUs_t currentTimeUs)
{
    UNUSED(vtxDevice);

    uint8_t mspFrame[15];

    mspPort_t *mspPort = getMspOsdPort();
    unsigned lastActivity = (currentTimeUs/1000) - mspPort->lastActivityMs;    
    if (lastActivity > VTX_MSP_TIMEOUT) {
        if (vtxState.timeouts++ > 3) {
            if (vtxState.ready) {
                vtxState.ready = false;
            }
        }
    } else { // active
        if (!vtxState.ready) {
            vtxState.ready = true;
        }
    }

    if (vtxState.ready) {
        if (vtxState.updateReqMask != VTX_UPDATE_REQ_NONE) {
            prepareMspFrame(vtxDevice, mspFrame);
            if (vtxState.crsfTelemetryEnabled) { // keep ELRS LUA up to date ?
                mspCrsfPush(MSP_VTX_CONFIG, mspFrame, sizeof(mspFrame));
            }

            int sent = mspSerialPushPort(MSP_VTX_CONFIG, mspFrame, sizeof(mspFrame), mspPort, MSP_V2_NATIVE);
            if (sent > 0) {
                vtxState.updateReqMask = VTX_UPDATE_REQ_NONE;
            }
        }   
    }
}

static vtxDevType_e vtxMspGetDeviceType(const vtxDevice_t *vtxDevice)
{
    UNUSED(vtxDevice);
    return VTXDEV_MSP;
}

static bool vtxMspIsReady(const vtxDevice_t *vtxDevice)
{
    return vtxDevice != NULL && mspVtxPortIdentifier >=0 && vtxState.ready;
}

static void vtxMspSetBandAndChannel(vtxDevice_t *vtxDevice, uint8_t band, uint8_t channel)
{
    UNUSED(vtxDevice);

    if (band < VTX_MSP_MIN_BAND || band > VTX_MSP_MAX_BAND || channel < VTX_MSP_MIN_CHANNEL || channel > VTX_MSP_MAX_CHANNEL) {
        return;
    }

    vtxState.request.band = band;
    vtxState.request.channel = channel;
    vtxState.request.freq = vtx58_Bandchan2Freq(band, channel);
    vtxState.updateReqMask |= VTX_UPDATE_REQ_FREQUENCY;
}

static void vtxMspSetPowerByIndex(vtxDevice_t *vtxDevice, uint8_t index)
{
    UNUSED(vtxDevice);

    vtxState.request.power = vtxMspPowerTable[index];
    vtxState.request.powerIndex = index;
    vtxState.updateReqMask |= VTX_UPDATE_REQ_POWER;
}

static void vtxMspSetPitMode(vtxDevice_t *vtxDevice, uint8_t onOff)
{
    UNUSED(vtxDevice);

    vtxState.request.pitMode = onOff;
    vtxState.updateReqMask |= VTX_UPDATE_REQ_PIT_MODE;
}

static bool vtxMspGetBandAndChannel(const vtxDevice_t *vtxDevice, uint8_t *pBand, uint8_t *pChannel)
{
    UNUSED(vtxDevice);

    *pBand = vtxState.request.band; 
    *pChannel = vtxState.request.channel;
    return true;
}

static bool vtxMspGetPowerIndex(const vtxDevice_t *vtxDevice, uint8_t *pIndex)
{
    UNUSED(vtxDevice);

    *pIndex = vtxState.request.powerIndex;
    return true;
}

static bool vtxMspGetPitMode(const vtxDevice_t *vtxDevice, uint8_t *pOnOff)
{
    UNUSED(vtxDevice);

    *pOnOff = vtxState.request.pitMode;
    return true;
}

static bool vtxMspGetFreq(const vtxDevice_t *vtxDevice, uint16_t *pFreq)
{
    UNUSED(vtxDevice);

    *pFreq = vtxState.request.freq;
    return true;
}

static bool vtxMspGetPower(const vtxDevice_t *vtxDevice, uint8_t *pIndex, uint16_t *pPowerMw)
{
    UNUSED(vtxDevice);

    *pIndex = vtxState.request.powerIndex;
    *pPowerMw = vtxState.request.power;
    return true;
}

static bool vtxMspGetOsdInfo(const vtxDevice_t *vtxDevice, vtxDeviceOsdInfo_t *pOsdInfo)
{
    UNUSED(vtxDevice);

    pOsdInfo->band = vtxState.request.band;
    pOsdInfo->channel = vtxState.request.channel;
    pOsdInfo->frequency = vtxState.request.freq;
    pOsdInfo->powerIndex = vtxState.request.powerIndex;
    pOsdInfo->powerMilliwatt = vtxState.request.power;
    pOsdInfo->bandName = vtxMspBandNames[vtxState.request.band];
    pOsdInfo->bandLetter = vtxMspBandLetters[vtxState.request.band];
    pOsdInfo->channelName = vtxMspChannelNames[vtxState.request.channel];
    pOsdInfo->powerIndexLetter = '0' + vtxState.request.powerIndex;

    return true;
}

static const vtxVTable_t mspVTable = {
    .process = vtxMspProcess,
    .getDeviceType = vtxMspGetDeviceType,
    .isReady = vtxMspIsReady,
    .setBandAndChannel = vtxMspSetBandAndChannel,
    .setPowerByIndex = vtxMspSetPowerByIndex,
    .setPitMode = vtxMspSetPitMode,
    .getBandAndChannel = vtxMspGetBandAndChannel,
    .getPowerIndex = vtxMspGetPowerIndex,
    .getPitMode = vtxMspGetPitMode,
    .getFrequency = vtxMspGetFreq,
    .getPower = vtxMspGetPower,
    .getOsdInfo = vtxMspGetOsdInfo
};

static vtxDevice_t vtxMsp = {
    .vTable = &mspVTable,
    .capability.bandCount = VTX_MSP_MAX_BAND,
    .capability.channelCount = VTX_MSP_MAX_CHANNEL,
    .capability.powerCount = VTX_MSP_POWER_COUNT,
    .capability.bandNames = (char **)vtxMspBandNames,
    .capability.channelNames = (char **)vtxMspChannelNames,
    .capability.powerNames = (char **)vtxMspPowerNames
};

bool vtxMspInit(void)
{
    const serialPortConfig_t *portConfig;
    
    // Shares MSP_OSD port
    portConfig = findSerialPortConfig(FUNCTION_VTX_MSP);
    if (!portConfig) {
        return false;
    }

    portConfig = findSerialPortConfig(FUNCTION_RX_SERIAL);

    vtxState.ready = false;
    vtxState.timeouts = 0;
    vtxState.updateReqMask = VTX_UPDATE_REQ_NONE;
    vtxState.crsfTelemetryEnabled = crsfRxIsActive();
    vtxState.request.band =  vtxSettingsConfig()->band;
    vtxState.request.channel = vtxSettingsConfig()->channel;
    vtxState.request.freq = vtx58_Bandchan2Freq(vtxState.request.band, vtxState.request.channel);
    vtxState.request.power = vtxSettingsConfig()->power;
    vtxState.request.pitMode = 0;
    vtxCommonSetDevice(&vtxMsp);

    return true;
}

#endif