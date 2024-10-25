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

/* Created by phobos- */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#include "platform.h"

#if defined(USE_VTX_MSP) && defined(USE_VTX_CONTROL) && defined(USE_VTX_COMMON)

#include "build/debug.h"

//#include "cms/cms_menu_vtx_msp.h"
#include "common/crc.h"
#include "common/log.h"
#include "config/feature.h"

#include "drivers/vtx_common.h"
//#include "drivers/vtx_table.h"

#include "fc/runtime_config.h"
#include "flight/failsafe.h"

#include "io/serial.h"
#include "io/vtx_msp.h"
#include "io/vtx_control.h"
#include "io/vtx_string.h"
#include "io/vtx_smartaudio.h"
#include "io/vtx.h"
#include "io/displayport_msp_osd.h"

#include "msp/msp_protocol.h"
#include "msp/msp_serial.h"
#include "msp/msp.h"

//#include "pg/vtx_table.h"
#include "fc/settings.h"

#include "rx/crsf.h"
//#include "rx/crsf_protocol.h"
#include "rx/rx.h"

#include "telemetry/msp_shared.h"

//static uint16_t mspConfFreq = 0;
static uint8_t mspConfBand = SETTING_VTX_BAND_DEFAULT;
static uint8_t mspConfChannel = SETTING_VTX_CHANNEL_DEFAULT;
//static uint16_t mspConfPower = 0;
static uint16_t mspConfPowerIndex = SETTING_VTX_POWER_DEFAULT;
static uint8_t mspConfPitMode = 0;
static bool mspVtxConfigChanged = false;
static timeUs_t mspVtxLastTimeUs = 0;
static bool prevLowPowerDisarmedState = false;

static const vtxVTable_t mspVTable; // forward
static vtxDevice_t vtxMsp = {
    .vTable = &mspVTable,
    .capability.bandCount = VTX_TABLE_MAX_BANDS,
    .capability.channelCount = VTX_TABLE_MAX_CHANNELS,
    .capability.powerCount = VTX_TABLE_MAX_POWER_LEVELS,
    .capability.bandNames = (char **)vtx58BandNames,
    .capability.channelNames = (char **)vtx58ChannelNames,
    .capability.powerNames = (char**)saPowerNames
};


STATIC_UNIT_TESTED mspVtxStatus_e mspVtxStatus = MSP_VTX_STATUS_OFFLINE;
static uint8_t mspVtxPortIdentifier = 255;

#define MSP_VTX_REQUEST_PERIOD_US (200 * 1000) // 200ms

static bool isCrsfPortConfig(const serialPortConfig_t *portConfig)
{
    return portConfig->functionMask & FUNCTION_RX_SERIAL && portConfig->functionMask & FUNCTION_VTX_MSP && rxConfig()->serialrx_provider == SERIALRX_CRSF;
}

static bool isLowPowerDisarmed(void)
{
    return (!ARMING_FLAG(ARMED) && !failsafeIsActive() &&
        (vtxSettingsConfig()->lowPowerDisarm == VTX_LOW_POWER_DISARM_ALWAYS ||
        (vtxSettingsConfig()->lowPowerDisarm == VTX_LOW_POWER_DISARM_UNTIL_FIRST_ARM && !ARMING_FLAG(WAS_EVER_ARMED))));
}

bool isVtxConfigValid(const vtxConfig_t *cfg)
{
    LOG_DEBUG(VTX, "msp isVtxConfigValid\r\n");
    for (int i = 0; i < MAX_CHANNEL_ACTIVATION_CONDITION_COUNT; ++i) {

        if (cfg->vtxChannelActivationConditions[i].band || 
            (cfg->vtxChannelActivationConditions[i].range.startStep && cfg->vtxChannelActivationConditions[i].range.endStep) ||
            cfg->vtxChannelActivationConditions[i].auxChannelIndex ||
            cfg->vtxChannelActivationConditions[i].channel) {
                return true;
        }
    }

    LOG_DEBUG(VTX, "msp Invalid Config!\r\n");
    return false;
}


void setMspVtxDeviceStatusReady(const int descriptor)
{
    LOG_DEBUG(VTX, "msp setMspVtxDeviceStatusReady\r\n");
    UNUSED(descriptor);

    mspVtxStatus = MSP_VTX_STATUS_READY;
    mspVtxConfigChanged = true;
}


void prepareMspFrame(uint8_t *mspFrame)
{
    LOG_DEBUG(VTX, "msp PrepareMspFrame\r\n");
/*
HDZERO parsing
    fc_band_rx = msp_rx_buf[1];
    fc_channel_rx = msp_rx_buf[2];
    fc_pwr_rx = msp_rx_buf[3];
    fc_pit_rx = msp_rx_buf[4];
    fc_lp_rx = msp_rx_buf[8];
*/

    uint8_t pitmode = 0;
    vtxCommonGetPitMode(&vtxMsp, &pitmode);

    mspFrame[0] = VTXDEV_MSP,
    mspFrame[1] = vtxSettingsConfig()->band;
    mspFrame[2] = vtxSettingsConfig()->channel;
    mspFrame[3] = isLowPowerDisarmed() ? 1 : vtxSettingsConfig()->power; // index based
    mspFrame[4] = pitmode;
    mspFrame[5] = 0; // Freq_L 
    mspFrame[6] = 0; // Freq_H
    mspFrame[7] = (mspVtxStatus == MSP_VTX_STATUS_READY) ? 1 : 0;
    mspFrame[8] = isLowPowerDisarmed();
    mspFrame[9] = 0; // Pitmode freq Low
    mspFrame[10] = 0; // pitmod freq High
    mspFrame[11] = 0; // 1 if using vtx table
    mspFrame[12] = 0; // vtx table bands or 0
    mspFrame[13] = 0; // vtx table channels or 0
    mspFrame[14] = 0; // vtx table power levels or 0
}

static void mspCrsfPush(const uint8_t mspCommand, const uint8_t *mspFrame, const uint8_t mspFrameSize)
{

    LOG_DEBUG(VTX, "msp CrsfPush\r\n");
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

    crsfRxSendTelemetryData(); //give the FC a chance to send outstanding telemetry
    crsfRxWriteTelemetryData(sbufPtr(dst), sbufBytesRemaining(dst));
    crsfRxSendTelemetryData();
#endif
}

static uint16_t packetCounter = 0;

static bool isVtxConfigChanged(void)
{
    if(mspVtxStatus == MSP_VTX_STATUS_READY) {
        if (mspVtxConfigChanged == true)
                return true;

        if (isLowPowerDisarmed() != prevLowPowerDisarmedState) {
                LOG_DEBUG(VTX, "msp vtx config changed (lower power disarm 2)\r\n");
                mspVtxConfigChanged = true;
                prevLowPowerDisarmedState = isLowPowerDisarmed();
        }

        if (mspConfPowerIndex != vtxSettingsConfig()->power) {
                LOG_DEBUG(VTX, "msp vtx config changed (power 2)\r\n");
                mspVtxConfigChanged = true;
        }

        if (mspConfBand != vtxSettingsConfig()->band || mspConfChannel != vtxSettingsConfig()->channel) {
                LOG_DEBUG(VTX, "msp vtx config changed (band and channel 2)\r\n");
                mspVtxConfigChanged = true;
        }

        return mspVtxConfigChanged;
    }

    return false;
}

static void vtxMspProcess(vtxDevice_t *vtxDevice, timeUs_t currentTimeUs)
{
    UNUSED(vtxDevice);

    const serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_MSP_OSD);
    uint8_t frame[15];

    switch (mspVtxStatus) {
    case MSP_VTX_STATUS_OFFLINE:
        LOG_DEBUG(VTX, "msp MspProcess: OFFLINE\r\n");
        // wait for MSP communication from the VTX
#ifdef USE_CMS
        //mspCmsUpdateStatusString();
#endif
        break;
    case MSP_VTX_STATUS_READY:
        LOG_DEBUG(VTX, "msp MspProcess: READY\r\n");
        // send an update if stuff has changed with 200ms period
        if ((isVtxConfigChanged()) && cmp32(currentTimeUs, mspVtxLastTimeUs) >= MSP_VTX_REQUEST_PERIOD_US) {

            LOG_DEBUG(VTX, "msp-vtx: vtxInfo Changed\r\n");
            prepareMspFrame(frame);

            if (isCrsfPortConfig(portConfig)) {
                mspCrsfPush(MSP_VTX_CONFIG, frame, sizeof(frame));
            } else {
                mspPort_t *port = getMspOsdPort();
                if(port != NULL && port->port) {
                    LOG_DEBUG(VTX, "msp-vtx: mspSerialPushPort\r\n");
                    int sent = mspSerialPushPort(MSP_VTX_CONFIG, frame, sizeof(frame), port, MSP_V2_NATIVE);
                    if (sent <= 0) {
                        break;
                    }
                }
            }
            packetCounter++;
            mspVtxLastTimeUs = currentTimeUs;
            mspVtxConfigChanged = false;

#ifdef USE_CMS
            //mspCmsUpdateStatusString();
#endif
        }
        break;
    default:
        mspVtxStatus = MSP_VTX_STATUS_OFFLINE;
        break;
    }

#if 0
    DEBUG_SET(DEBUG_VTX_MSP, 0, packetCounter);
    DEBUG_SET(DEBUG_VTX_MSP, 1, isCrsfPortConfig(portConfig));
    DEBUG_SET(DEBUG_VTX_MSP, 2, isLowPowerDisarmed());
#if defined(USE_MSP_OVER_TELEMETRY)
    DEBUG_SET(DEBUG_VTX_MSP, 3, isCrsfPortConfig(portConfig) ? getMspTelemetryDescriptor() : getMspSerialPortDescriptor(mspVtxPortIdentifier));
#else
    DEBUG_SET(DEBUG_VTX_MSP, 3, getMspSerialPortDescriptor(mspVtxPortIdentifier));
#endif
#endif
}

static vtxDevType_e vtxMspGetDeviceType(const vtxDevice_t *vtxDevice)
{
    //LOG_DEBUG(VTX, "msp GetDeviceType\r\n");
    UNUSED(vtxDevice);
    return VTXDEV_MSP;
}

static bool vtxMspIsReady(const vtxDevice_t *vtxDevice)
{
    //LOG_DEBUG(VTX, "msp vtxIsReady: %s\r\n", (vtxDevice != NULL && mspVtxStatus == MSP_VTX_STATUS_READY) ? "Y": "N");
    return vtxDevice != NULL && mspVtxStatus == MSP_VTX_STATUS_READY;
}

static void vtxMspSetBandAndChannel(vtxDevice_t *vtxDevice, uint8_t band, uint8_t channel)
{
    LOG_DEBUG(VTX, "msp SetBandAndChannel\r\n");
    UNUSED(vtxDevice);
    if (band != mspConfBand || channel != mspConfChannel) {
        LOG_DEBUG(VTX, "msp vtx config changed (band and channel)\r\n");
        mspVtxConfigChanged = true;
    }
    mspConfBand = band;
    mspConfChannel = channel;
}

static void vtxMspSetPowerByIndex(vtxDevice_t *vtxDevice, uint8_t index)
{
    LOG_DEBUG(VTX, "msp SetPowerByIndex\r\n");
    UNUSED(vtxDevice);

    if (index > 0 && (index < VTX_TABLE_MAX_POWER_LEVELS))
    {
        if (index != mspConfPowerIndex)
        {
            LOG_DEBUG(VTX, "msp vtx config changed (power by index)\r\n");
            mspVtxConfigChanged = true;
        }
        mspConfPowerIndex = index;
    }
}

static void vtxMspSetPitMode(vtxDevice_t *vtxDevice, uint8_t onoff)
{
    LOG_DEBUG(VTX, "msp SetPitMode\r\n");
    UNUSED(vtxDevice);
    if (onoff != mspConfPitMode) {
        LOG_DEBUG(VTX, "msp vtx config changed (pitmode)\r\n");
        mspVtxConfigChanged = true;
    }
    mspConfPitMode = onoff;
}

#if 0
static void vtxMspSetFreq(vtxDevice_t *vtxDevice, uint16_t freq)
{
    UNUSED(vtxDevice);
    if (freq != mspConfFreq) {
        mspVtxConfigChanged = true;
    }
    mspConfFreq = freq;
}
#endif




static bool vtxMspGetBandAndChannel(const vtxDevice_t *vtxDevice, uint8_t *pBand, uint8_t *pChannel)
{
    if (!vtxMspIsReady(vtxDevice)) {
        return false;
    }

    *pBand = vtxSettingsConfig()->band;
    *pChannel = vtxSettingsConfig()->channel;

    //LOG_DEBUG(VTX, "msp GetBandAndChannel: %02x:%02x\r\n", vtxSettingsConfig()->band, vtxSettingsConfig()->channel);

    return true;
}

static bool vtxMspGetPowerIndex(const vtxDevice_t *vtxDevice, uint8_t *pIndex)
{
    if (!vtxMspIsReady(vtxDevice)) {
        return false;
    }

    uint8_t power = isLowPowerDisarmed() ? 1 : vtxSettingsConfig()->power;
    // Special case, power not set
    if (power > VTX_TABLE_MAX_POWER_LEVELS) {
        *pIndex = 0;
        //LOG_DEBUG(VTX, "msp GetPowerIndex: %u\r\n", *pIndex);
        return true;
    }

    *pIndex = power;

    //LOG_DEBUG(VTX, "msp GetPowerIndex: %u\r\n", *pIndex);
    return true;
}

static bool vtxMspGetFreq(const vtxDevice_t *vtxDevice, uint16_t *pFreq)
{
    LOG_DEBUG(VTX, "msp GetFreq\r\n");
    if (!vtxMspIsReady(vtxDevice)) {
        return false;
    }

    *pFreq = 5800;
    return true;
}

static bool vtxMspGetPower(const vtxDevice_t *vtxDevice, uint8_t *pIndex, uint16_t *pPowerMw)
{
    LOG_DEBUG(VTX, "msp GetPower\r\n");
    uint8_t powerIndex;

    if (!vtxMspGetPowerIndex(vtxDevice, &powerIndex)) {
        return false;
    }


    *pIndex = powerIndex;
    *pPowerMw = *pIndex;
    return true;
}

static bool vtxMspGetOsdInfo(const  vtxDevice_t *vtxDevice, vtxDeviceOsdInfo_t * pOsdInfo)
{
    LOG_DEBUG(VTX, "msp GetOsdInfo\r\n");
    uint8_t powerIndex;
    uint16_t powerMw;
    uint16_t freq;
    uint8_t band, channel;

    if (!vtxMspGetBandAndChannel(vtxDevice, &band, &channel)) {
        return false;
    }

    if (!vtxMspGetFreq(vtxDevice, &freq)) {
        return false;
    }

    if (!vtxMspGetPower(vtxDevice, &powerIndex, &powerMw)) {
        return false;
    }

    pOsdInfo->band = band;
    pOsdInfo->channel = channel;
    pOsdInfo->frequency = freq;
    pOsdInfo->powerIndex = powerIndex;
    pOsdInfo->powerMilliwatt = powerMw;
    pOsdInfo->bandLetter = vtx58BandNames[band][0];
    pOsdInfo->bandName = vtx58BandNames[band];
    pOsdInfo->channelName = vtx58ChannelNames[channel];
    pOsdInfo->powerIndexLetter = '0' + powerIndex;
    return true;
}


bool vtxMspInit(void)
{
    LOG_DEBUG(VTX, "msp %s\r\n", __FUNCTION__);
    // don't bother setting up this device if we don't have MSP vtx enabled
    // Port is shared with MSP_OSD
    const serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_MSP_OSD);
    if (!portConfig) {
        return false;
    }

    mspVtxPortIdentifier = portConfig->identifier;
  
    // XXX Effect of USE_VTX_COMMON should be reviewed, as following call to vtxInit will do nothing if vtxCommonSetDevice is not called.
#if defined(USE_VTX_COMMON)
    vtxCommonSetDevice(&vtxMsp);
#endif

    mspConfBand = vtxSettingsConfig()->band;
    mspConfChannel = vtxSettingsConfig()->channel;
    mspConfPowerIndex = isLowPowerDisarmed() ? 1 : vtxSettingsConfig()->power; // index based
    vtxCommonGetPitMode(&vtxMsp, &mspConfPitMode);

    vtxInit();

    return true;
}

static const vtxVTable_t mspVTable = {
    .process = vtxMspProcess,
    .getDeviceType = vtxMspGetDeviceType,
    .isReady = vtxMspIsReady,
    .setBandAndChannel = vtxMspSetBandAndChannel,
    .setPowerByIndex = vtxMspSetPowerByIndex,
    .setPitMode = vtxMspSetPitMode,
    //.setFrequency = vtxMspSetFreq,
    .getBandAndChannel = vtxMspGetBandAndChannel,
    .getPowerIndex = vtxMspGetPowerIndex,
    .getFrequency = vtxMspGetFreq,
    //.getStatus = vtxMspGetStatus,
    .getPower = vtxMspGetPower,
    //.serializeCustomDeviceStatus = NULL,
    .getOsdInfo = vtxMspGetOsdInfo,
};

static mspResult_e mspVtxProcessMspCommand(mspPacket_t *cmd, mspPacket_t *reply, mspPostProcessFnPtr *mspPostProcessFn)
{
    //LOG_DEBUG(VTX, "msp VTX_MSP_PROCESS\r\n");
    UNUSED(mspPostProcessFn);

    sbuf_t *dst = &reply->buf;
    sbuf_t *src = &cmd->buf;

    const unsigned int dataSize = sbufBytesRemaining(src);
    UNUSED(dst);
    UNUSED(src);

    // Start initializing the reply message
    reply->cmd = cmd->cmd;
    reply->result = MSP_RESULT_ACK;

    vtxDevice_t *vtxDevice = vtxCommonDevice();
    if (!vtxDevice || vtxCommonGetDeviceType(vtxDevice) != VTXDEV_MSP) {
        LOG_DEBUG(VTX, "msp wrong vtx\r\n");
        return MSP_RESULT_ERROR;
    }

    switch (cmd->cmd)
    {
        case MSP_VTXTABLE_BAND:
        {
            LOG_DEBUG(VTX, "msp MSP_VTXTABLE_BAND\r\n");
            uint8_t deviceType = vtxCommonGetDeviceType(vtxDevice);
            if (deviceType == VTXDEV_MSP)
            {
                /*
                char bandName[MSP_VTX_TABLE_BAND_NAME_LENGTH + 1];
                memset(bandName, 0, MSP_VTX_TABLE_BAND_NAME_LENGTH + 1);
                uint16_t frequencies[MSP_VTX_TABLE_MAX_CHANNELS];
                const uint8_t band = sbufReadU8(src);
                const uint8_t bandNameLength = sbufReadU8(src);
                for (int i = 0; i < bandNameLength; i++) {
                    const char nameChar = sbufReadU8(src);
                    if (i < MSP_VTX_TABLE_BAND_NAME_LENGTH) {
                        bandName[i] = toupper(nameChar);
                    }
                }
                const char bandLetter = toupper(sbufReadU8(src));
                const bool isFactoryBand = (bool)sbufReadU8(src);
                const uint8_t channelCount = sbufReadU8(src);
                for (int i = 0; i < channelCount; i++)
                {
                    const uint16_t frequency = sbufReadU16(src);
                    if (i < vtxTableConfig()->channels)
                    {
                        frequencies[i] = frequency;
                    }
                }
                */

                setMspVtxDeviceStatusReady(1);
            }
            break;
        }
        case MSP_VTXTABLE_POWERLEVEL:
        {
            LOG_DEBUG(VTX, "msp MSP_VTXTABLE_POWERLEVEL\r\n");
            
                /*
                char powerLevelLabel[VTX_TABLE_POWER_LABEL_LENGTH + 1];
                memset(powerLevelLabel, 0, VTX_TABLE_POWER_LABEL_LENGTH + 1);
                const uint8_t powerLevel = sbufReadU8(src);
                const uint16_t powerValue = sbufReadU16(src);
                const uint8_t powerLevelLabelLength = sbufReadU8(src);
                for (int i = 0; i < powerLevelLabelLength; i++)
                {
                    const char labelChar = sbufReadU8(src);
                    if (i < VTX_TABLE_POWER_LABEL_LENGTH)
                    {
                        powerLevelLabel[i] = toupper(labelChar);
                    }
                }
                */
                setMspVtxDeviceStatusReady(1);
        }
        break;
        case MSP_VTX_CONFIG:
        {
            LOG_DEBUG(VTX, "msp MSP_VTX_CONFIG received\r\n");
            LOG_DEBUG(VTX, "msp MSP_VTX_CONFIG VTXDEV_MSP\r\n");
            uint8_t pitmode = 0;
            vtxCommonGetPitMode(vtxDevice, &pitmode);

            // VTXDEV_MSP,
            sbufWriteU8(dst, VTXDEV_MSP);
            // band;
            sbufWriteU8(dst, vtxSettingsConfig()->band);
            // channel;
            sbufWriteU8(dst, vtxSettingsConfig()->channel);
            // power; // index based
            sbufWriteU8(dst, vtxSettingsConfig()->power);
            // pit mode;
            // Freq_L
            sbufWriteU8(dst, 0);
            // Freq_H
            sbufWriteU8(dst, 0);
            // vtx status
            sbufWriteU8(dst, 1);
            // lowPowerDisarm

            sbufWriteU8(dst, vtxSettingsConfig()->lowPowerDisarm);
            // Pitmode freq Low
            sbufWriteU8(dst, 0);
            // pitmod freq High
            sbufWriteU8(dst, 0);
            // 1 if using vtx table
            sbufWriteU8(dst, 0);
            // vtx table bands or 0
            sbufWriteU8(dst, 0);
            // vtx table channels or 0
            sbufWriteU8(dst, 0);

            setMspVtxDeviceStatusReady(1);
            break;
        }
        case MSP_SET_VTX_CONFIG:
            LOG_DEBUG(VTX, "msp MSP_SET_VTX_CONFIG\r\n");
            if (dataSize == 15)
            {
                if (vtxCommonGetDeviceType(vtxDevice) != VTXDEV_UNKNOWN)
                {
                    for (int i = 0; i < 15; ++i)
                    {
                        uint8_t data = sbufReadU8(src);
                        switch (i)
                        {
                        case 1:
                            vtxSettingsConfigMutable()->band = data;
                            break;
                        case 2:
                            vtxSettingsConfigMutable()->channel = data;
                            break;
                        case 3:
                            vtxSettingsConfigMutable()->power = data;
                            break;
                        case 4:
                            vtxCommonSetPitMode(vtxDevice, data);
                            break;
                        case 7:
                            // vtx ready
                            break;
                        case 8:
                            vtxSettingsConfigMutable()->lowPowerDisarm = data;
                            break;
                        }
                    }
                }

                setMspVtxDeviceStatusReady(1);
                break;
            }
            LOG_DEBUG(VTX, "msp MSP_RESULT_ERROR\r\n");
            return MSP_RESULT_ERROR;
        default:
            // debug[1]++;
            // debug[2] = cmd->cmd;
            if(cmd->cmd != MSP_STATUS && cmd->cmd != MSP_STATUS_EX && cmd->cmd != MSP_RC) {
                LOG_DEBUG(VTX, "msp default: %02x\r\n", cmd->cmd);
            }
            reply->result = MSP_RESULT_ERROR;
            break;
    }

    // Process DONT_REPLY flag
    if (cmd->flags & MSP_FLAG_DONT_REPLY)
    {
        reply->result = MSP_RESULT_NO_REPLY;
    }

    return reply->result;
}

void mspVtxSerialProcess(mspProcessCommandFnPtr mspProcessCommandFn)
{
    UNUSED(mspProcessCommandFn);
    // Check if VTX is ready
    /*
    if (mspVtxStatus != MSP_VTX_STATUS_READY) {
        LOG_DEBUG(VTX, "msp VTX NOT READY, skipping\r\n");
        return;
    }
    */

    mspPort_t *port = getMspOsdPort();

    if(port) {
        mspSerialProcessOnePort(port, MSP_SKIP_NON_MSP_DATA, mspVtxProcessMspCommand);
    }

}


#endif