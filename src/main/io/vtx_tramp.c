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

/* Created by jflyper */

#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "platform.h"

#if defined(USE_VTX_TRAMP) && defined(USE_VTX_CONTROL)

#include "build/debug.h"
#include "drivers/vtx_common.h"
#include "drivers/time.h"

#include "common/maths.h"
#include "common/utils.h"
#include "common/crc.h"

#include "io/serial.h"
#include "io/vtx_tramp.h"
#include "io/vtx_control.h"
#include "io/vtx.h"
#include "io/vtx_string.h"

#define VTX_PKT_SIZE                16
#define VTX_PROTO_STATE_TIMEOUT_MS  1000
#define VTX_STATUS_INTERVAL_MS      2000

#define VTX_UPDATE_REQ_NONE         0x00
#define VTX_UPDATE_REQ_FREQUENCY    0x01
#define VTX_UPDATE_REQ_POWER        0x02
#define VTX_UPDATE_REQ_PITMODE      0x04

typedef enum {
    VTX_STATE_RESET         = 0,
    VTX_STATE_OFFILE        = 1,    // Not detected
    VTX_STATE_DETECTING     = 2,    // 
    VTX_STATE_IDLE          = 3,    // Idle, ready to sent commands
    VTX_STATE_QUERY_DELAY   = 4,
    VTX_STATE_QUERY_STATUS  = 5,
    VTX_STATE_WAIT_STATUS   = 6,    // Wait for VTX state
} vtxProtoState_e;

typedef enum {
    VTX_RESPONSE_TYPE_NONE,
    VTX_RESPONSE_TYPE_CAPABILITIES,
    VTX_RESPONSE_TYPE_STATUS,
} vtxProtoResponseType_e;

typedef struct {
    vtxProtoState_e protoState;
    timeMs_t        lastStateChangeMs;
    timeMs_t        lastStatusQueryMs;
    int             protoTimeoutCount;
    unsigned        updateReqMask;

    // VTX capabilities
    struct {
        unsigned freqMin;   // min freq
        unsigned freqMax;   // max freq
        unsigned powerMax;  //
    } capabilities;

    // Requested VTX state
    struct {
        // Only tracking
        int      band;
        int      channel;
        unsigned powerIndex;

        // Actual settings to send to the VTX
        unsigned freq;
        unsigned power;
    } request;

    // Actual VTX state: updated from actual VTX
    struct {
        unsigned freq;      // Frequency in MHz
        unsigned power;
        unsigned temp;
        bool     pitMode;
    } state;

    struct {
        int              powerTableCount;
        const uint16_t * powerTablePtr;
    } metadata;

    // Comms flags and state
    uint8_t         sendPkt[VTX_PKT_SIZE];
    uint8_t         recvPkt[VTX_PKT_SIZE];
    unsigned        recvPtr;
    serialPort_t *  port;
} vtxProtoState_t;

static vtxProtoState_t vtxState;

static void vtxProtoUpdatePowerMetadata(uint16_t maxPower);

static bool trampIsValidResponseCode(uint8_t code)
{
    return (code == 'r' || code == 'v' || code == 's');
}

static bool vtxProtoRecv(void)
{
    uint8_t * bufPtr = (uint8_t*)&vtxState.recvPkt;
    while (serialRxBytesWaiting(vtxState.port)) {
        const uint8_t c = serialRead(vtxState.port);

        if (vtxState.recvPtr == 0) {
            // Wait for sync byte
            if (c == 0x0F) {
                bufPtr[vtxState.recvPtr++] = c;
            }
        }
        else if (vtxState.recvPtr == 1) {
            // Check if we received a valid response code
            if (trampIsValidResponseCode(c)) {
                bufPtr[vtxState.recvPtr++] = c;
            }
            else {
                vtxState.recvPtr = 0;
            }
        }
        else {
            // Consume character and check if we have got a full packet
            if (vtxState.recvPtr < VTX_PKT_SIZE) {
                bufPtr[vtxState.recvPtr++] = c;
            }

            if (vtxState.recvPtr == VTX_PKT_SIZE) {
                // Full packet received - validate packet, make sure it's the one we expect
                const bool pktValid = ((bufPtr[14] == crc8_sum_update(0, &bufPtr[1], 13)) && (bufPtr[15] == 0));

                if (!pktValid) {
                    // Reset the receiver state - keep waiting
                    vtxState.recvPtr = 0;
                }
                // Make sure it's not the echo one (half-duplex serial might receive it's own data)
                else if (memcmp(&vtxState.recvPkt, &vtxState.sendPkt, VTX_PKT_SIZE) == 0) {
                    vtxState.recvPtr = 0;
                }
                // Valid receive packet
                else {
                    return true;
                }
            }
        }
    }

    return false;
}

static void vtxProtoSend(uint8_t cmd, uint16_t param)
{
    // Craft the packet
    memset(vtxState.sendPkt, 0, ARRAYLEN(vtxState.sendPkt));
    vtxState.sendPkt[0] = 15;
    vtxState.sendPkt[1] = cmd;
    vtxState.sendPkt[2] = param & 0xff;
    vtxState.sendPkt[3] = (param >> 8) & 0xff;
    vtxState.sendPkt[14] = crc8_sum_update(0, &vtxState.sendPkt[1], 13);

    // Send data 
    serialWriteBuf(vtxState.port, (uint8_t *)&vtxState.sendPkt, sizeof(vtxState.sendPkt));

    // Reset cmd response state
    vtxState.recvPtr = 0;
}

static void vtxProtoSetState(vtxProtoState_e newState)
{
    vtxState.lastStateChangeMs = millis();
    vtxState.protoState = newState;
}

static bool vtxProtoTimeout(void)
{
    return (millis() - vtxState.lastStateChangeMs) > VTX_PROTO_STATE_TIMEOUT_MS;
}

static void vtxProtoQueryCapabilities(void)
{
    vtxProtoSend(0x72, 0);
}

static void vtxProtoQueryStatus(void)
{
    vtxProtoSend(0x76, 0);
    vtxState.lastStatusQueryMs = millis();
}

/*
static void vtxProtoQueryTemperature(void)
{
    vtxProtoSend('s', 0);
}
*/

static vtxProtoResponseType_e vtxProtoProcessResponse(void)
{
    const uint8_t respCode = vtxState.recvPkt[1];

    switch (respCode) {
        case 0x72:
            vtxState.capabilities.freqMin = vtxState.recvPkt[2] | (vtxState.recvPkt[3] << 8);
            vtxState.capabilities.freqMax = vtxState.recvPkt[4] | (vtxState.recvPkt[5] << 8);
            vtxState.capabilities.powerMax = vtxState.recvPkt[6] | (vtxState.recvPkt[7] << 8);

            if (vtxState.capabilities.freqMin != 0 && vtxState.capabilities.freqMin < vtxState.capabilities.freqMax) {
                // Some TRAMP VTXes may report max power incorrectly (i.e. 200mW for a 600mW VTX)
                // Make use of vtxSettingsConfig()->maxPowerOverride to override
                if (vtxSettingsConfig()->maxPowerOverride != 0) {
                    vtxState.capabilities.powerMax = vtxSettingsConfig()->maxPowerOverride;
                }

                // Update max power metadata so OSD settings would match VTX capabilities
                vtxProtoUpdatePowerMetadata(vtxState.capabilities.powerMax);

                return VTX_RESPONSE_TYPE_CAPABILITIES;
            }
            break;

        case 0x76:
            vtxState.state.freq = vtxState.recvPkt[2] | (vtxState.recvPkt[3] << 8);
            vtxState.state.power = vtxState.recvPkt[4]|(vtxState.recvPkt[5] << 8);
            vtxState.state.pitMode = vtxState.recvPkt[7];
            //vtxState.state.power = vtxState.recvPkt[8]|(vtxState.recvPkt[9] << 8);
            return VTX_RESPONSE_TYPE_STATUS;
    }

    return VTX_RESPONSE_TYPE_NONE;
}

static void vtxProtoSetPitMode(uint16_t mode)
{
    vtxProtoSend(0x73, mode);
}

static void vtxProtoSetPower(uint16_t power)
{
    vtxProtoSend(0x50, power);
}

static void vtxProtoSetFrequency(uint16_t freq)
{
    vtxProtoSend(0x46, freq);
}

static void impl_Process(vtxDevice_t *vtxDevice, timeUs_t currentTimeUs)
{
    // Glue function betwen VTX VTable and actual driver protothread
    UNUSED(vtxDevice);
    UNUSED(currentTimeUs);

    if (!vtxState.port) {
        return;
    }

    switch((int)vtxState.protoState) {
        case VTX_STATE_RESET:
            vtxState.protoTimeoutCount = 0;
            vtxState.updateReqMask = VTX_UPDATE_REQ_NONE;
            vtxProtoSetState(VTX_STATE_OFFILE);
            break;

        // Send request for capabilities
        case VTX_STATE_OFFILE:
            vtxProtoQueryCapabilities();
            vtxProtoSetState(VTX_STATE_DETECTING);
            break;

        // Detect VTX. We only accept VTX_RESPONSE_TYPE_CAPABILITIES here
        case VTX_STATE_DETECTING:
            if (vtxProtoRecv()) {
                if (vtxProtoProcessResponse() == VTX_RESPONSE_TYPE_CAPABILITIES) {
                    // VTX sent capabilities. Query status now
                    vtxState.protoTimeoutCount = 0;
                    vtxProtoSetState(VTX_STATE_QUERY_STATUS);
                }
                else {
                    // Unexpected response. Re-initialize
                    vtxProtoSetState(VTX_STATE_RESET);
                }
            }
            else if (vtxProtoTimeout()) {
                // Time-out while waiting for capabilities. Reset the state
                vtxProtoSetState(VTX_STATE_RESET);
            }
            break;

        // Send requests to update freqnecy and power, periodically poll device for liveness
        case VTX_STATE_IDLE:
            if (vtxState.updateReqMask != VTX_UPDATE_REQ_NONE) {
                // Updates pending. Send an appropriate command
                if (vtxState.updateReqMask & VTX_UPDATE_REQ_PITMODE) {
                    // Only disabling PIT mode supported
                    vtxState.updateReqMask &= ~VTX_UPDATE_REQ_PITMODE;
                    vtxProtoSetPitMode(0);
                    vtxProtoSetState(VTX_STATE_QUERY_DELAY);
                }
                else if (vtxState.updateReqMask & VTX_UPDATE_REQ_FREQUENCY) {
                    vtxState.updateReqMask &= ~VTX_UPDATE_REQ_FREQUENCY;
                    vtxProtoSetFrequency(vtxState.request.freq);
                    vtxProtoSetState(VTX_STATE_QUERY_DELAY);
                }
                else if (vtxState.updateReqMask & VTX_UPDATE_REQ_POWER) {
                    vtxState.updateReqMask &= ~VTX_UPDATE_REQ_POWER;
                    vtxProtoSetPower(vtxState.request.power);
                    vtxProtoSetState(VTX_STATE_QUERY_DELAY);
                }
            }
            else if ((millis() - vtxState.lastStatusQueryMs) > VTX_STATUS_INTERVAL_MS) {
                // Poll VTX for status updates
                vtxProtoSetState(VTX_STATE_QUERY_STATUS);
            }
            break;

        case VTX_STATE_QUERY_DELAY:
            // We get here after sending the command. We give VTX some time to process the command
            // and switch to VTX_STATE_QUERY_STATUS
            if (vtxProtoTimeout()) {
                // We gave VTX some time to process the command. Query status to confirm success
                vtxProtoSetState(VTX_STATE_QUERY_STATUS);
            }
            break;

        case VTX_STATE_QUERY_STATUS:
            // Just query status, nothing special
            vtxProtoQueryStatus();
            vtxProtoSetState(VTX_STATE_WAIT_STATUS);
            break;

        case VTX_STATE_WAIT_STATUS:
            if (vtxProtoRecv()) {
                vtxState.protoTimeoutCount = 0;

                if (vtxProtoProcessResponse() == VTX_RESPONSE_TYPE_STATUS) {
                    // Check if VTX state matches VTX request
                    if (!(vtxState.updateReqMask & VTX_UPDATE_REQ_FREQUENCY) && (vtxState.state.freq != vtxState.request.freq)) {
                        vtxState.updateReqMask |= VTX_UPDATE_REQ_FREQUENCY;
                    }

                    if (!(vtxState.updateReqMask & VTX_UPDATE_REQ_POWER) && (vtxState.state.power != vtxState.request.power)) {
                        vtxState.updateReqMask |= VTX_UPDATE_REQ_POWER;
                    }

                    // We got the status response - proceed to IDLE
                    vtxProtoSetState(VTX_STATE_IDLE);
                }
                else {
                    // Unexpected response. Query for STATUS again
                    vtxProtoSetState(VTX_STATE_QUERY_STATUS);
                }
            }
            else if (vtxProtoTimeout()) {
                vtxState.protoTimeoutCount++;
                if (vtxState.protoTimeoutCount > 3) {
                    vtxProtoSetState(VTX_STATE_RESET);
                }
                else {
                    vtxProtoSetState(VTX_STATE_QUERY_STATUS);
                }
            }
            break;
    }
}

static vtxDevType_e impl_GetDeviceType(const vtxDevice_t *vtxDevice)
{
    UNUSED(vtxDevice);
    return VTXDEV_TRAMP;
}

static bool impl_IsReady(const vtxDevice_t *vtxDevice)
{
    return vtxDevice != NULL && vtxState.port != NULL && vtxState.protoState >= VTX_STATE_IDLE;
}

static void impl_SetBandAndChannel(vtxDevice_t * vtxDevice, uint8_t band, uint8_t channel)
{
    UNUSED(vtxDevice);

    if (!impl_IsReady(vtxDevice)) {
        return;
    }

    // TRAMP is 5.8 GHz only
    uint16_t newFreqMhz  = vtx58_Bandchan2Freq(band, channel);

    if (newFreqMhz < vtxState.capabilities.freqMin || newFreqMhz > vtxState.capabilities.freqMax) {
        return;
    }

    // Cache band and channel
    vtxState.request.band = band;
    vtxState.request.channel = channel;
    vtxState.request.freq = newFreqMhz;
    vtxState.updateReqMask |= VTX_UPDATE_REQ_FREQUENCY;
}

static void impl_SetPowerByIndex(vtxDevice_t * vtxDevice, uint8_t index)
{
    UNUSED(vtxDevice);

    if (!impl_IsReady(vtxDevice) || index < 1 || index > vtxState.metadata.powerTableCount) {
        return;
    }

    unsigned reqPower = vtxState.metadata.powerTablePtr[index - 1];

    // Cap the power to the max capability of the VTX
    vtxState.request.power = MIN(reqPower, vtxState.capabilities.powerMax);
    vtxState.request.powerIndex = index;

    vtxState.updateReqMask |= VTX_UPDATE_REQ_POWER;
}

static void impl_SetPitMode(vtxDevice_t *vtxDevice, uint8_t onoff)
{
    UNUSED(vtxDevice);

    if (onoff == 0) {
        vtxState.updateReqMask |= VTX_UPDATE_REQ_PITMODE;
    }
}

static bool impl_GetBandAndChannel(const vtxDevice_t *vtxDevice, uint8_t *pBand, uint8_t *pChannel)
{
    if (!impl_IsReady(vtxDevice)) {
        return false;
    }

    // if in user-freq mode then report band as zero
    *pBand = vtxState.request.band;
    *pChannel = vtxState.request.channel;
    return true;
}

static bool impl_GetPowerIndex(const vtxDevice_t *vtxDevice, uint8_t *pIndex)
{
    if (!impl_IsReady(vtxDevice)) {
        return false;
    }

    *pIndex = vtxState.request.powerIndex;

    return true;
}

static bool impl_GetPitMode(const vtxDevice_t *vtxDevice, uint8_t *pOnOff)
{
    if (!impl_IsReady(vtxDevice)) {
        return false;
    }

    *pOnOff = vtxState.state.pitMode ? 1 : 0;
    return true;
}

static bool impl_GetFreq(const vtxDevice_t *vtxDevice, uint16_t *pFreq)
{
    if (!impl_IsReady(vtxDevice)) {
        return false;
    }

    *pFreq = vtxState.request.freq;
    return true;
}

static bool impl_GetPower(const vtxDevice_t *vtxDevice, uint8_t *pIndex, uint16_t *pPowerMw)
{
    if (!impl_IsReady(vtxDevice)) {
        return false;
    }

    *pIndex = vtxState.request.powerIndex;
    *pPowerMw = vtxState.request.power;
    return true;
}

static bool impl_GetOsdInfo(const  vtxDevice_t *vtxDevice, vtxDeviceOsdInfo_t * pOsdInfo)
{
    if (!impl_IsReady(vtxDevice)) {
        return false;
    }

    pOsdInfo->band = vtxState.request.band;
    pOsdInfo->channel = vtxState.request.channel;
    pOsdInfo->frequency = vtxState.request.freq;
    pOsdInfo->powerIndex = vtxState.request.powerIndex;
    pOsdInfo->powerMilliwatt = vtxState.request.power;
    pOsdInfo->bandLetter = vtx58BandNames[vtxState.request.band][0];
    pOsdInfo->bandName = vtx58BandNames[vtxState.request.band];
    pOsdInfo->channelName = vtx58ChannelNames[vtxState.request.channel];
    pOsdInfo->powerIndexLetter = '0' + vtxState.request.powerIndex;
    return true;
}

/*****************************************************************************/
static const vtxVTable_t impl_vtxVTable = {
    .process = impl_Process,
    .getDeviceType = impl_GetDeviceType,
    .isReady = impl_IsReady,
    .setBandAndChannel = impl_SetBandAndChannel,
    .setPowerByIndex = impl_SetPowerByIndex,
    .setPitMode = impl_SetPitMode,
    .getBandAndChannel = impl_GetBandAndChannel,
    .getPowerIndex = impl_GetPowerIndex,
    .getPitMode = impl_GetPitMode,
    .getFrequency = impl_GetFreq,
    .getPower = impl_GetPower,
    .getOsdInfo = impl_GetOsdInfo,
};

static vtxDevice_t impl_vtxDevice = {
    .vTable = &impl_vtxVTable,
    .capability.bandCount = VTX_TRAMP_5G8_BAND_COUNT,
    .capability.channelCount = VTX_TRAMP_5G8_CHANNEL_COUNT,
    .capability.powerCount = VTX_TRAMP_MAX_POWER_COUNT,
    .capability.bandNames = (char **)vtx58BandNames,
    .capability.channelNames = (char **)vtx58ChannelNames,
    .capability.powerNames = NULL,
};

const uint16_t trampPowerTable_200[VTX_TRAMP_MAX_POWER_COUNT]         = { 25, 100, 200, 200, 200 };
const char * const trampPowerNames_200[VTX_TRAMP_MAX_POWER_COUNT + 1] = { "---", "25 ", "100", "200", "200", "200" };

const uint16_t trampPowerTable_400[VTX_TRAMP_MAX_POWER_COUNT]         = { 25, 100, 200, 400, 400 };
const char * const trampPowerNames_400[VTX_TRAMP_MAX_POWER_COUNT + 1] = { "---", "25 ", "100", "200", "400", "400" };

const uint16_t trampPowerTable_600[VTX_TRAMP_MAX_POWER_COUNT]         = { 25, 100, 200, 400, 600 };
const char * const trampPowerNames_600[VTX_TRAMP_MAX_POWER_COUNT + 1] = { "---", "25 ", "100", "200", "400", "600" };

const uint16_t trampPowerTable_800[VTX_TRAMP_MAX_POWER_COUNT]         = { 25, 100, 200, 500, 800 };
const char * const trampPowerNames_800[VTX_TRAMP_MAX_POWER_COUNT + 1] = { "---", "25 ", "100", "200", "500", "800" };

static void vtxProtoUpdatePowerMetadata(uint16_t maxPower)
{
    if (maxPower >= 800) {
        // Max power 800mW: Use 25, 100, 200, 500, 800 table
        vtxState.metadata.powerTablePtr  = trampPowerTable_800;
        vtxState.metadata.powerTableCount = VTX_TRAMP_MAX_POWER_COUNT;
        
        impl_vtxDevice.capability.powerNames = (char **)trampPowerNames_800;
        impl_vtxDevice.capability.powerCount = VTX_TRAMP_MAX_POWER_COUNT;
    }
    else if (maxPower >= 600) {
        // Max power 600mW: Use 25, 100, 200, 400, 600 table
        vtxState.metadata.powerTablePtr  = trampPowerTable_600;
        vtxState.metadata.powerTableCount = VTX_TRAMP_MAX_POWER_COUNT;

        impl_vtxDevice.capability.powerNames = (char **)trampPowerNames_600;
        impl_vtxDevice.capability.powerCount = VTX_TRAMP_MAX_POWER_COUNT;
    }
    else if (maxPower >= 400) {
        // Max power 400mW: Use 25, 100, 200, 400 table
        vtxState.metadata.powerTablePtr  = trampPowerTable_400;
        vtxState.metadata.powerTableCount = 4;

        impl_vtxDevice.capability.powerNames = (char **)trampPowerNames_400;
        impl_vtxDevice.capability.powerCount = 4;
    }
    else if (maxPower >= 200) {
        // Max power 200mW: Use 25, 100, 200 table
        vtxState.metadata.powerTablePtr  = trampPowerTable_200;
        vtxState.metadata.powerTableCount = 3;

        impl_vtxDevice.capability.powerNames = (char **)trampPowerNames_200;
        impl_vtxDevice.capability.powerCount = 3;
    }
    else {
        // Default to standard TRAMP 600mW VTX
        vtxState.metadata.powerTablePtr  = trampPowerTable_600;
        vtxState.metadata.powerTableCount = VTX_TRAMP_MAX_POWER_COUNT;

        impl_vtxDevice.capability.powerNames = (char **)trampPowerNames_600;
        impl_vtxDevice.capability.powerCount = VTX_TRAMP_MAX_POWER_COUNT;
    }

}

bool vtxTrampInit(void)
{
    serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_VTX_TRAMP);

    if (portConfig) {
        portOptions_t portOptions = 0;
        portOptions = portOptions | (vtxConfig()->halfDuplex ? SERIAL_BIDIR : SERIAL_UNIDIR);
        vtxState.port = openSerialPort(portConfig->identifier, FUNCTION_VTX_TRAMP, NULL, NULL, 9600, MODE_RXTX, portOptions);
    }

    if (!vtxState.port) {
        return false;
    }

    vtxProtoUpdatePowerMetadata(600);
    vtxCommonSetDevice(&impl_vtxDevice);

    vtxState.protoState = VTX_STATE_RESET;

    return true;
}

#endif
