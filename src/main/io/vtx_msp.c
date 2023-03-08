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
#include <math.h>
#include <ctype.h>
#include <string.h>

#include "platform.h"

#if defined(USE_VTX_MSP) && defined(USE_VTX_CONTROL) && defined(USE_VTX_COMMON)

#include "build/debug.h"

//#include "cms/cms_menu_vtx_msp.h"
#include "common/crc.h"
#include "config/feature.h"

#include "drivers/vtx_common.h"
//#include "drivers/vtx_table.h"

#include "fc/runtime_config.h"
#include "flight/failsafe.h"

#include "io/serial.h"
#include "io/vtx_msp.h"
#include "io/vtx_control.h"
#include "io/vtx.h"

#include "msp/msp_protocol.h"
#include "msp/msp_serial.h"

//#include "pg/vtx_table.h"

#include "rx/crsf.h"
//#include "rx/crsf_protocol.h"
#include "rx/rx.h"

#include "telemetry/msp_shared.h"

//static uint16_t mspConfFreq = 0;
static uint8_t mspConfBand = 0;
static uint8_t mspConfChannel = 0;
//static uint16_t mspConfPower = 0;
static uint16_t mspConfPowerIndex = (uint16_t)-1;
static uint8_t mspConfPitMode = 0;
static bool mspVtxConfigChanged = false;
static timeUs_t mspVtxLastTimeUs = 0;
static bool prevLowPowerDisarmedState = false;

static const vtxVTable_t mspVTable; // forward
static vtxDevice_t vtxMsp = {
    .vTable = &mspVTable,
};

// Fill table with standard values for SA 1.0 and 2.0
static mspPowerTable_t mspPowerTable[VTX_MSP_MAX_POWER_COUNT] = {
    {  25,   7 },
    { 200,  16 },
    { 500,  25 },
    { 800,  40 },
    {   0,   0 } // Placeholder
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
    for(int i  =0; i < MAX_CHANNEL_ACTIVATION_CONDITION_COUNT;++i) {

        if (cfg->vtxChannelActivationConditions[i].band || 
            (cfg->vtxChannelActivationConditions[i].range.startStep && cfg->vtxChannelActivationConditions[i].range.endStep) ||
            cfg->vtxChannelActivationConditions[i].auxChannelIndex ||
            cfg->vtxChannelActivationConditions[i].channel) {
                return true;
        }
    }

    return false;
}


void setMspVtxDeviceStatusReady(const int descriptor)
{
    UNUSED(descriptor);
    /*
    vtxDevice_t *vtxDevice = NULL;
    uint8_t pBand;
    uint8_t pChannel;
    uint8_t pIndex;

    UNUSED(pBand);
    UNUSED(pChannel);
    UNUSED(pIndex);
    if (mspVtxStatus != MSP_VTX_STATUS_READY && isVtxConfigValid(vtxConfig())) {
#if 0 && defined(USE_MSP_OVER_TELEMETRY)
        if (getMspSerialPortDescriptor(mspVtxPortIdentifier) == descriptor || getMspTelemetryDescriptor() == descriptor) {
#else
        if (getMspSerialPortDescriptor(mspVtxPortIdentifier) == descriptor) {
#endif
            mspVtxStatus = MSP_VTX_STATUS_READY;
        }
    }
    */
    mspVtxStatus = MSP_VTX_STATUS_READY;
}


void prepareMspFrame(uint8_t *mspFrame)
{
    mspFrame[0] = VTXDEV_MSP,
    mspFrame[1] = vtxSettingsConfig()->band;
    mspFrame[2] = vtxSettingsConfig()->channel;
    mspFrame[3] = isLowPowerDisarmed() ? 1 : vtxSettingsConfig()->power; // index based
    mspFrame[4] = mspConfPitMode;
    mspFrame[5] = vtxCommonDeviceIsReady(&vtxMsp) ? 1 : 0;
    mspFrame[6] = vtxSettingsConfig()->lowPowerDisarm;
    mspFrame[7] = 0;
    mspFrame[8] = 0;
    mspFrame[9] = 0;
    mspFrame[10] = 0;
    mspFrame[11] = 0;
    mspFrame[12] = 0;
    mspFrame[13] = 0;
    mspFrame[14] = 0;
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

    crsfRxSendTelemetryData(); //give the FC a chance to send outstanding telemetry
    crsfRxWriteTelemetryData(sbufPtr(dst), sbufBytesRemaining(dst));
    crsfRxSendTelemetryData();
#endif
}

static uint16_t packetCounter = 0;

static void vtxMspProcess(vtxDevice_t *vtxDevice, timeUs_t currentTimeUs)
{
    UNUSED(vtxDevice);

    const serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_MSP_OSD);
    uint8_t frame[15];

    switch (mspVtxStatus) {
    case MSP_VTX_STATUS_OFFLINE:
        // wait for MSP communication from the VTX
#ifdef USE_CMS
        //mspCmsUpdateStatusString();
#endif
        break;
    case MSP_VTX_STATUS_READY:
        if (isLowPowerDisarmed() != prevLowPowerDisarmedState) {
            mspVtxConfigChanged = true;
            prevLowPowerDisarmedState = isLowPowerDisarmed();
        }

        // send an update if stuff has changed with 200ms period
        if (mspVtxConfigChanged && cmp32(currentTimeUs, mspVtxLastTimeUs) >= MSP_VTX_REQUEST_PERIOD_US) {

            prepareMspFrame(frame);

            if (isCrsfPortConfig(portConfig)) {
                mspCrsfPush(MSP_VTX_CONFIG, frame, sizeof(frame));
            } else {
                mspSerialPush(MSP_VTX_CONFIG, frame, sizeof(frame));
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
    UNUSED(vtxDevice);
    return VTXDEV_MSP;
}

static bool vtxMspIsReady(const vtxDevice_t *vtxDevice)
{
    return vtxDevice != NULL && mspVtxStatus == MSP_VTX_STATUS_READY;
}

static void vtxMspSetBandAndChannel(vtxDevice_t *vtxDevice, uint8_t band, uint8_t channel)
{
    UNUSED(vtxDevice);
    if (band != mspConfBand || channel != mspConfChannel) {
        mspVtxConfigChanged = true;
    }
    mspConfBand = band;
    mspConfChannel = channel;
}

static void vtxMspSetPowerByIndex(vtxDevice_t *vtxDevice, uint8_t index)
{
    UNUSED(vtxDevice);

    if (index > 0 && (index < VTX_MSP_MAX_POWER_COUNT + 1) && mspPowerTable[index].mW)
    {
        if (index != mspConfPowerIndex)
        {
            mspVtxConfigChanged = true;
        }
        mspConfPowerIndex = index - 1;
        //mspConfPower = mspPowerTable[index].mW;
    }
}

static void vtxMspSetPitMode(vtxDevice_t *vtxDevice, uint8_t onoff)
{
    UNUSED(vtxDevice);
    if (onoff != mspConfPitMode) {
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

    *pBand = mspConfBand;
    *pChannel = mspConfChannel;
    return true;
}

static bool vtxMspGetPowerIndex(const vtxDevice_t *vtxDevice, uint8_t *pIndex)
{
    if (!vtxMspIsReady(vtxDevice)) {
        return false;
    }

    // Special case, power not set
    if (mspConfPowerIndex > VTX_MSP_MAX_POWER_COUNT) {
        *pIndex = 0;
        return true;
    }

    *pIndex = mspConfPowerIndex + 1;

    return true;
}

static bool vtxMspGetFreq(const vtxDevice_t *vtxDevice, uint16_t *pFreq)
{
    if (!vtxMspIsReady(vtxDevice)) {
        return false;
    }

    // TODO: 
    switch(vtxSettingsConfig()->frequencyGroup)
    {
        case FREQUENCYGROUP_1G3:
            *pFreq = 1300;
            break;
        case FREQUENCYGROUP_2G4:
            *pFreq = 2400;
            break;
        case FREQUENCYGROUP_5G8:
            *pFreq = 5800;
            break;
        default:
            return false;
    }

    return true;
}

#if 0
static bool vtxMspGetStatus(const vtxDevice_t *vtxDevice, unsigned *status)
{
    if (!vtxMspIsReady(vtxDevice)) {
        return false;
    }

    // Mirror configued pit mode state rather than use current pitmode as we
    // should, otherwise the logic in vtxProcessPitMode may not get us to the
    // correct state if pitmode is toggled quickly
    *status = (mspConfPitMode ? 1 : 0);

    return true;
}
#endif

static bool vtxMspGetPower(const vtxDevice_t *vtxDevice, uint8_t *pIndex, uint16_t *pPowerMw)
{
    if (!vtxMspIsReady(vtxDevice)) {
        return 0;
    }

    *pIndex = mspConfPowerIndex + 1;
    *pPowerMw = mspPowerTable[mspConfPowerIndex].mW;

    return true;
}

/*
bool mspOsdSerialInit(void)
{
    static volatile uint8_t txBuffer[TX_BUFFER_SIZE];
    memset(&mspPort, 0, sizeof(mspPort_t));

    serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_MSP_OSD);
    if (portConfig) {
        serialPort_t *port = openSerialPort(portConfig->identifier, FUNCTION_MSP_OSD, NULL, NULL,
                baudRates[portConfig->peripheral_baudrateIndex], MODE_RXTX, SERIAL_NOT_INVERTED);

        if (port) {
            // Use a bigger TX buffer size to accommodate the configuration menus
            port->txBuffer = txBuffer;
            port->txBufferSize = TX_BUFFER_SIZE;
            port->txBufferTail = 0;
            port->txBufferHead = 0;

            resetMspPort(&mspPort, port);

            return true;
        }
    }

    return false;
}
*/

bool vtxMspInit(void)
{
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

    vtxInit();

    return true;
}

/*
typedef struct vtxVTable_s {
    void (*process)(vtxDevice_t *vtxDevice, timeUs_t currentTimeUs);
    vtxDevType_e (*getDeviceType)(const vtxDevice_t *vtxDevice);
    bool (*isReady)(const vtxDevice_t *vtxDevice);

    void (*setBandAndChannel)(vtxDevice_t *vtxDevice, uint8_t band, uint8_t channel);
    void (*setPowerByIndex)(vtxDevice_t *vtxDevice, uint8_t level);
    void (*setPitMode)(vtxDevice_t *vtxDevice, uint8_t onoff);

    bool (*getBandAndChannel)(const vtxDevice_t *vtxDevice, uint8_t *pBand, uint8_t *pChannel);
    bool (*getPowerIndex)(const vtxDevice_t *vtxDevice, uint8_t *pIndex);
    bool (*getPitMode)(const vtxDevice_t *vtxDevice, uint8_t *pOnOff);
    bool (*getFrequency)(const vtxDevice_t *vtxDevice, uint16_t *pFreq);

    bool (*getPower)(const vtxDevice_t *vtxDevice, uint8_t *pIndex, uint16_t *pPowerMw);
    bool (*getOsdInfo)(const  vtxDevice_t *vtxDevice, vtxDeviceOsdInfo_t * pOsdInfo);
}
*/

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
};


#endif