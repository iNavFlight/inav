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

#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#if defined(USE_VTX_FFPV) && defined(USE_VTX_CONTROL)

#include "build/debug.h"

#include "drivers/time.h"
#include "drivers/vtx_common.h"

#include "common/maths.h"
#include "common/utils.h"

#include "scheduler/protothreads.h"

//#include "cms/cms_menu_vtx_ffpv24g.h"

#include "io/vtx.h"
#include "io/vtx_ffpv24g.h"
#include "io/vtx_control.h"
#include "io/vtx_string.h"
#include "io/serial.h"


#define VTX_FFPV_CMD_TIMEOUT_MS     250
#define VTX_FFPV_HEARTBEAT_MS       1000

#define VTX_FFPV_MIN_BAND           (1)
#define VTX_FFPV_MAX_BAND           (VTX_FFPV_MIN_BAND + VTX_FFPV_BAND_COUNT - 1)
#define VTX_FFPV_MIN_CHANNEL        (1)
#define VTX_FFPV_MAX_CHANNEL        (VTX_FFPV_MIN_CHANNEL + VTX_FFPV_CHANNEL_COUNT -1)

#define VTX_UPDATE_REQ_NONE         0x00
#define VTX_UPDATE_REQ_FREQUENCY    0x01
#define VTX_UPDATE_REQ_POWER        0x02

typedef struct __attribute__((__packed__)) ffpvPacket_s {
    uint8_t header;
    uint8_t cmd;
    uint8_t data[12];
    uint8_t checksum;
    uint8_t footer;
} ffpvPacket_t;

typedef struct {
    bool ready;
    int protoTimeouts;
    unsigned updateReqMask;

    // VTX capabilities
    struct {
        unsigned freqMin;
        unsigned freqMax;
        unsigned powerMin;
        unsigned powerMax;
    } capabilities;

    // Requested VTX state
    struct {
        bool     setByFrequency;
        int      band;
        int      channel;
        unsigned freq;
        unsigned power;
        unsigned powerIndex;
    } request;

    // Actual VTX state
    struct {
        unsigned freq;
        unsigned power;
    } state;

    // Comms flags and state
    ffpvPacket_t sendPkt;
    ffpvPacket_t recvPkt;
    unsigned     recvPtr;
    bool         pktReceived;
} vtxProtoState_t;

/*****************************************************************************/
const char * const ffpvBandNames[VTX_FFPV_BAND_COUNT + 1] = {
    "--------",
    "FFPV 2.4 A",
    "FFPV 2.4 B",
};

const char * ffpvBandLetters = "-AB";

const uint16_t ffpvFrequencyTable[VTX_FFPV_BAND_COUNT][VTX_FFPV_CHANNEL_COUNT] =
{
    { 2410, 2430, 2450, 2470, 2370, 2390, 2490, 2510 }, // FFPV 2.4 A
    { 2414, 2432, 2450, 2468, 2411, 2433, 2453, 2473 }, // FFPV 2.4 A
};

const char * const ffpvChannelNames[VTX_FFPV_CHANNEL_COUNT + 1] = {
    "-", "1", "2", "3", "4", "5", "6", "7", "8",
};

const char * const ffpvPowerNames[VTX_FFPV_POWER_COUNT + 1] = {
    "---", "25 ", "200", "500", "800"
};

const unsigned ffpvPowerTable[VTX_FFPV_POWER_COUNT] = {
    25, 200, 500, 800
};


/*******************************************************************************/
static serialPort_t * vtxSerialPort = NULL;
static vtxProtoState_t vtxState;

static uint8_t vtxCalcChecksum(ffpvPacket_t * pkt)
{
    uint8_t sum = pkt->cmd;
    for (int i = 0; i < 12; i++) {
        sum += pkt->data[i];
    }
    return sum;
}

static bool vtxProtoRecv(void)
{
    // Return success instantly if packet is already awaiting processing
    if (vtxState.pktReceived) {
        return true;
    }

    uint8_t * bufPtr = (uint8_t*)&vtxState.recvPkt;
    while (serialRxBytesWaiting(vtxSerialPort) && !vtxState.pktReceived) {
        const uint8_t c = serialRead(vtxSerialPort);

        if (vtxState.recvPtr == 0) {
            // Wait for sync byte
            if (c == 0x0F) {
                bufPtr[vtxState.recvPtr++] = c;
            }
        }
        else {
            // Sync byte ok - wait for full packet
            if (vtxState.recvPtr < sizeof(vtxState.recvPkt)) {
                bufPtr[vtxState.recvPtr++] = c;
            }

            // Received full packet - do some processing
            if (vtxState.recvPtr == sizeof(vtxState.recvPkt)) {
                // Full packet received - validate packet, make sure it's the one we expect
                const bool pktValid = (vtxState.recvPkt.header == 0x0F && vtxState.recvPkt.cmd == vtxState.sendPkt.cmd && vtxState.recvPkt.footer == 0x00 && vtxState.recvPkt.checksum == vtxCalcChecksum(&vtxState.recvPkt));
                if (!pktValid) {
                    // Reset the receiver state - keep waiting
                    vtxState.pktReceived = false;
                    vtxState.recvPtr = 0;
                }
                // Make sure it's not the echo one (half-duplex serial might receive it's own data)
                else if (memcmp(&vtxState.recvPkt.data, &vtxState.sendPkt.data, sizeof(vtxState.recvPkt.data)) == 0) {
                    vtxState.pktReceived = false;
                    vtxState.recvPtr = 0;
                }
                // Valid receive packet
                else {
                    vtxState.pktReceived = true;
                    return true;
                }
            }
        }
    }

    return false;
}

static void vtxProtoSend(uint8_t cmd, const uint8_t * data)
{
    // Craft and send FPV packet
    vtxState.sendPkt.header = 0x0F;
    vtxState.sendPkt.cmd = cmd;

    if (data) {
        memcpy(vtxState.sendPkt.data, data, sizeof(vtxState.sendPkt.data));
    }
    else {
        memset(vtxState.sendPkt.data, 0, sizeof(vtxState.sendPkt.data));
    }

    vtxState.sendPkt.checksum = vtxCalcChecksum(&vtxState.sendPkt);
    vtxState.sendPkt.footer = 0x00;

    // Send data 
    serialWriteBuf(vtxSerialPort, (uint8_t *)&vtxState.sendPkt, sizeof(vtxState.sendPkt));

    // Reset cmd response state
    vtxState.pktReceived = false;
    vtxState.recvPtr = 0;
}

static void vtxProtoSend_SetFreqency(unsigned freq)
{
    uint8_t data[12] = {0};
    data[0] = freq & 0xFF;
    data[1] = (freq >> 8) & 0xFF;
    vtxProtoSend(0x46, data);
}

static void vtxProtoSend_SetPower(unsigned power)
{
    uint8_t data[12] = {0};
    data[0] = power & 0xFF;
    data[1] = (power >> 8) & 0xFF;
    vtxProtoSend(0x50, data);
}

STATIC_PROTOTHREAD(impl_VtxProtocolThread)
{
    ptBegin(impl_VtxProtocolThread);

    // 0: Detect VTX. Dwell here infinitely until we get a valid response from VTX
    vtxState.ready = false;
    while(!vtxState.ready) {
        // Send capabilities request and wait
        vtxProtoSend(0x72, NULL);
        ptWaitTimeout(vtxProtoRecv(), VTX_FFPV_CMD_TIMEOUT_MS);

        // Check if we got a valid response
        if (vtxState.pktReceived) {
            vtxState.capabilities.freqMin = vtxState.recvPkt.data[0] | (vtxState.recvPkt.data[1] << 8);
            vtxState.capabilities.freqMax = vtxState.recvPkt.data[2] | (vtxState.recvPkt.data[3] << 8);
            vtxState.capabilities.powerMin = 0;
            vtxState.capabilities.powerMax = vtxState.recvPkt.data[4] | (vtxState.recvPkt.data[5] << 8);
            vtxState.ready = true;
        }
    }

    // 1 : Periodically poll VTX for current channel and power, send updates
    vtxState.protoTimeouts = 0;
    vtxState.updateReqMask = VTX_UPDATE_REQ_NONE;

    while(vtxState.ready) {
        // Wait for request for update or time to check liveness
        ptWaitTimeout(vtxState.updateReqMask != VTX_UPDATE_REQ_NONE, VTX_FFPV_HEARTBEAT_MS);

        if (vtxState.updateReqMask != VTX_UPDATE_REQ_NONE) {
            if (vtxState.updateReqMask & VTX_UPDATE_REQ_FREQUENCY) {
                vtxProtoSend_SetFreqency(vtxState.request.freq);
                vtxState.updateReqMask &= ~VTX_UPDATE_REQ_FREQUENCY;
                ptDelayMs(VTX_FFPV_CMD_TIMEOUT_MS);
            }
            else if (vtxState.updateReqMask & VTX_UPDATE_REQ_POWER) {
                vtxProtoSend_SetPower(vtxState.request.power);
                vtxState.updateReqMask &= ~VTX_UPDATE_REQ_POWER;
            }
            else {
                // Unsupported request - reset
                vtxState.updateReqMask = VTX_UPDATE_REQ_NONE;
            }
        }
        else {
            // Periodic check for VTX liveness
            vtxProtoSend(0x76, NULL);
            ptWaitTimeout(vtxProtoRecv(), VTX_FFPV_CMD_TIMEOUT_MS);

            if (vtxState.pktReceived) {
                // Got a valid state from VTX
                vtxState.state.freq = (uint16_t)vtxState.recvPkt.data[0] | ((uint16_t)vtxState.recvPkt.data[1] << 8);
                vtxState.state.power = (uint16_t)vtxState.recvPkt.data[2] | ((uint16_t)vtxState.recvPkt.data[3] << 8);
                vtxState.protoTimeouts = 0;

                // Check if VTX state matches VTX request
                if (vtxState.state.freq != vtxState.request.freq) {
                    vtxState.updateReqMask |= VTX_UPDATE_REQ_FREQUENCY;
                }

                if (vtxState.state.power != vtxState.request.power) {
                    vtxState.updateReqMask |= VTX_UPDATE_REQ_POWER;
                }
            }
            else {
                vtxState.protoTimeouts++;
            }
        }

        // Sanity check. If we got more than 3 protocol erros
        if (vtxState.protoTimeouts >= 3) {
            // Reset ready flag - thread will terminate and restart
            vtxState.ready = false;
        }
    }

    ptEnd(0);
}


static void impl_Process(vtxDevice_t *vtxDevice, timeUs_t currentTimeUs)
{
    // Glue function betwen VTX VTable and actual driver protothread
    UNUSED(vtxDevice);
    UNUSED(currentTimeUs);

    impl_VtxProtocolThread();

    // If thread stopped - vtx comms failed - restart thread and re-init VTX comms
    if (ptIsStopped(ptGetHandle(impl_VtxProtocolThread))) {
        ptRestart(ptGetHandle(impl_VtxProtocolThread));
    }
}

static vtxDevType_e impl_GetDeviceType(const vtxDevice_t *vtxDevice)
{
    UNUSED(vtxDevice);
    return VTXDEV_FFPV;
}

static bool impl_IsReady(const vtxDevice_t *vtxDevice)
{
    return vtxDevice != NULL && vtxSerialPort != NULL && vtxState.ready;
}

static bool impl_DevSetFreq(uint16_t freq)
{
    if (!vtxState.ready || freq < vtxState.capabilities.freqMin || freq > vtxState.capabilities.freqMax) {
        return false;
    }

    vtxState.request.freq = freq;
    vtxState.updateReqMask |= VTX_UPDATE_REQ_FREQUENCY;

    return true;
}

static void impl_SetFreq(vtxDevice_t * vtxDevice, uint16_t freq)
{
    UNUSED(vtxDevice);

    if (impl_DevSetFreq(freq)) {
        // Keep track that we set frequency directly
        vtxState.request.setByFrequency = true;
    }
}

void ffpvSetBandAndChannel(uint8_t band, uint8_t channel)
{
    // Validate band and channel
    if (band < VTX_FFPV_MIN_BAND || band > VTX_FFPV_MAX_BAND || channel < VTX_FFPV_MIN_CHANNEL || channel > VTX_FFPV_MAX_CHANNEL) {
        return;
    }

    if (impl_DevSetFreq(ffpvFrequencyTable[band - 1][channel - 1])) {
        // Keep track of band/channel data
        vtxState.request.setByFrequency = false;
        vtxState.request.band = band;
        vtxState.request.channel = channel;
    }
}


static void impl_SetBandAndChannel(vtxDevice_t * vtxDevice, uint8_t band, uint8_t channel)
{
    UNUSED(vtxDevice);
    ffpvSetBandAndChannel(band, channel);
}

void ffpvSetRFPowerByIndex(uint16_t index)
{
    // Validate index
    if (index < 1 || index > VTX_FFPV_POWER_COUNT) {
        return;
    }

    const unsigned power = ffpvPowerTable[index - 1];
    if (!vtxState.ready || power < vtxState.capabilities.powerMin || power > vtxState.capabilities.powerMax) {
        return;
    }

    vtxState.request.power = power;
    vtxState.request.powerIndex = index;
    vtxState.updateReqMask |= VTX_UPDATE_REQ_POWER;
}

static void impl_SetPowerByIndex(vtxDevice_t * vtxDevice, uint8_t index)
{
    UNUSED(vtxDevice);
    ffpvSetRFPowerByIndex(index);
}

static void impl_SetPitMode(vtxDevice_t *vtxDevice, uint8_t onoff)
{
    // TODO: Not implemented
    UNUSED(vtxDevice);
    UNUSED(onoff);
}

static bool impl_GetBandAndChannel(const vtxDevice_t *vtxDevice, uint8_t *pBand, uint8_t *pChannel)
{
    if (!impl_IsReady(vtxDevice)) {
        return false;
    }

    // if in user-freq mode then report band as zero
    *pBand = vtxState.request.setByFrequency ? 0 : vtxState.request.band;
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

    // TODO: Not inplemented
    *pOnOff = 0;
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

vtxRunState_t * ffpvGetRuntimeState(void)
{
    static vtxRunState_t state;

    if (vtxState.ready) {
        state.pitMode = 0;
        state.band = vtxState.request.band;
        state.channel = vtxState.request.channel;
        state.frequency = vtxState.request.freq;
        state.powerIndex = vtxState.request.powerIndex;
        state.powerMilliwatt = vtxState.request.power;
    }
    else {
        state.pitMode = 0;
        state.band = 1;
        state.channel = 1;
        state.frequency = ffpvFrequencyTable[0][0];
        state.powerIndex = 1;
        state.powerMilliwatt = 25;
    }
    return &state;
}

/*****************************************************************************/
static const vtxVTable_t impl_vtxVTable = {
    .process = impl_Process,
    .getDeviceType = impl_GetDeviceType,
    .isReady = impl_IsReady,
    .setBandAndChannel = impl_SetBandAndChannel,
    .setPowerByIndex = impl_SetPowerByIndex,
    .setPitMode = impl_SetPitMode,
    .setFrequency = impl_SetFreq,
    .getBandAndChannel = impl_GetBandAndChannel,
    .getPowerIndex = impl_GetPowerIndex,
    .getPitMode = impl_GetPitMode,
    .getFrequency = impl_GetFreq,
};

static vtxDevice_t impl_vtxDevice = {
    .vTable = &impl_vtxVTable,
    .capability.bandCount = VTX_FFPV_BAND_COUNT,
    .capability.channelCount = VTX_FFPV_CHANNEL_COUNT,
    .capability.powerCount = VTX_FFPV_POWER_COUNT,
    .bandNames = (char **)ffpvBandNames,
    .channelNames = (char **)ffpvChannelNames,
    .powerNames = (char **)ffpvPowerNames,
};

bool vtxFuriousFPVInit(void)
{
    serialPortConfig_t * portConfig = findSerialPortConfig(FUNCTION_VTX_FFPV);

    if (portConfig) {
        portOptions_t portOptions = 0;
        portOptions = portOptions | (vtxConfig()->halfDuplex ? SERIAL_BIDIR : SERIAL_UNIDIR);
        vtxSerialPort = openSerialPort(portConfig->identifier, FUNCTION_VTX_FFPV, NULL, NULL, 9600, MODE_RXTX, portOptions);
    }

    if (!vtxSerialPort) {
        return false;
    }

    vtxCommonSetDevice(&impl_vtxDevice);

    ptRestart(ptGetHandle(impl_VtxProtocolThread));

    return true;
}

#endif