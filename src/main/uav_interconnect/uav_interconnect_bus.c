/*
 * This file is part of INAV.
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
 *
 * @author Konstantin Sharlaimov <konstantin.sharlaimov@gmail.com>
 */

#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#include "platform.h"
#include "build/build_config.h"

#ifdef USE_UAV_INTERCONNECT

#include "build/debug.h"

#include "common/maths.h"
#include "common/axis.h"
#include "common/utils.h"
#include "common/crc.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/serial.h"
#include "drivers/time.h"

#include "io/serial.h"

#include "uav_interconnect/uav_interconnect.h"

typedef enum {
    UIB_COMMAND_IDENTIFY    = (0x00 << 5),  // 0x00
    UIB_COMMAND_NOTIFY      = (0x01 << 5),  // 0x20
    UIB_COMMAND_READ        = (0x02 << 5),  // 0x40
    UIB_COMMAND_WRITE       = (0x03 << 5),  // 0x60
    UIB_COMMAND_RES_1       = (0x04 << 5),  // 0x80
    UIB_COMMAND_RES_2       = (0x05 << 5),  // 0xA0
    UIB_COMMAND_RES_3       = (0x06 << 5),  // 0xC0
    UIB_COMMAND_RES_4       = (0x07 << 5),  // 0xE0
} uibCommand_e;

typedef enum {
    UIB_FLAG_HAS_READ       = (1 << 0),     // Device supports READ command (sensor)
    UIB_FLAG_HAS_WRITE      = (1 << 1),     // Device supports WRITE command (sensor configuration or executive device)
} uibDeviceFlags_e;

#define UIB_PROTOCOL_VERSION    0x00
#define UIB_MAX_SLOTS           16          // 32 is design maximum
#define UIB_PORT_OPTIONS        (SERIAL_NOT_INVERTED | SERIAL_STOPBITS_1 | SERIAL_PARITY_NO | SERIAL_UNIDIR)

#define UIB_DISCOVERY_DELAY_US  2000000 // 2 seconds from power-up to allow all devices to boot
#define UIB_GUARD_INTERVAL_US   1000
#define UIB_REFRESH_INTERVAL_US 200000

typedef struct {
    bool        allocated;
    bool        activated;
    uint8_t     deviceAddress;
    uint16_t    deviceFlags;
    uint8_t     devParams[4];
    timeUs_t    pollIntervalUs;
    timeUs_t    lastPollTimeUs;
    uint32_t    unrepliedRequests;  // 0 - all answered, 1 - request in progress, 2 or more - device failed to answer one or more requests

    uint8_t     rxDataReadySize;
    uint8_t     txDataReadySize;
    uint8_t     rxPacket[UIB_MAX_PACKET_SIZE];
    uint8_t     txPacket[UIB_MAX_PACKET_SIZE];
} uavInterconnectSlot_t;

typedef enum {
    STATE_INITIALIZE,
    STATE_DISCOVER,
    STATE_READY,
} uavInterconnectState_t;

typedef struct {
    int sentCommands;
    int discoveredDevices;
    int failedCRC;
    int commandTimeouts;
} uavInterconnectStats_t;

static serialPort_t *           busPort;
static bool                     uibInitialized = false;

static uavInterconnectSlot_t    slots[UIB_MAX_SLOTS];
static timeUs_t                 slotStartTimeUs;
static uavInterconnectState_t   busState = STATE_INITIALIZE;

static int                      discoveryAddress;
static int                      refreshSlot;
static timeUs_t                 refreshStartTimeUs;

static uint8_t                  slotDataBuffer[20]; // Max transaction length is 20 bytes
static unsigned                 slotDataBufferCount;
static timeUs_t                 slotLastActivityUs;

// Statistics
static uavInterconnectStats_t   uibStats;

static void switchState(uavInterconnectState_t newState)
{
    if (busState == newState)
        return;

    busState = newState;
}

static void sendDiscover(timeUs_t currentTimeUs, uint8_t slot, uint8_t devId)
{
    uint8_t buf[4];
    buf[0] = UIB_COMMAND_IDENTIFY | slot;
    buf[1] = devId;
    buf[2] = UIB_PROTOCOL_VERSION;
    buf[3] = crc8_dvb_s2_update(0x00, &buf[0], 3);
    slotStartTimeUs = currentTimeUs;
    serialWriteBuf(busPort, buf, 4);
    uibStats.sentCommands++;
}

static void sendNotify(timeUs_t currentTimeUs, uint8_t slot, uint8_t devId)
{
    uint8_t buf[4];
    buf[0] = UIB_COMMAND_NOTIFY | slot;
    buf[1] = devId;
    buf[2] = UIB_PROTOCOL_VERSION;
    buf[3] = crc8_dvb_s2_update(0x00, &buf[0], 3);
    slotStartTimeUs = currentTimeUs;
    serialWriteBuf(busPort, buf, 4);
    uibStats.sentCommands++;
}

static void sendRead(timeUs_t currentTimeUs, uint8_t slot)
{
    uint8_t buf[2];
    buf[0] = UIB_COMMAND_READ | slot;
    buf[1] = crc8_dvb_s2_update(0x00, &buf[0], 1);
    slotStartTimeUs = currentTimeUs;
    serialWriteBuf(busPort, buf, 2);
    uibStats.sentCommands++;
}

static void sendWrite(timeUs_t currentTimeUs, uint8_t slot, uint8_t * data, uint8_t len)
{
    uint8_t buf[UIB_MAX_PACKET_SIZE + 3];
    buf[0] = UIB_COMMAND_WRITE | slot;
    buf[1] = len;
    memcpy(&buf[2], data, len);
    buf[UIB_MAX_PACKET_SIZE + 2] = crc8_dvb_s2_update(0x00, &buf[0], UIB_MAX_PACKET_SIZE + 2);
    slotStartTimeUs = currentTimeUs;
    serialWriteBuf(busPort, buf, UIB_MAX_PACKET_SIZE + 3);
    uibStats.sentCommands++;
}

static uavInterconnectSlot_t * findDevice(uint8_t devId)
{
    for (int i = 0; i < UIB_MAX_SLOTS; i++) {
        if (slots[i].allocated && slots[i].deviceAddress == devId) {
            return &slots[i];
        }
    }

    return NULL;
}

static int findEmptySlot(void)
{
    for (int i = 0; i < UIB_MAX_SLOTS; i++) {
        if (!slots[i].allocated)
            return i;
    }

    return -1;
}

static void registerDeviceSlot(uint8_t slotId, uint8_t devId, uint16_t deviceFlags, uint32_t pollIntervalUs, const uint8_t * devParams)
{
    slots[slotId].allocated = true;
    slots[slotId].deviceAddress = devId;
    slots[slotId].deviceFlags = deviceFlags;
    slots[slotId].pollIntervalUs = pollIntervalUs;

    if (devParams) {
        slots[slotId].devParams[0] = devParams[0];
        slots[slotId].devParams[1] = devParams[1];
        slots[slotId].devParams[2] = devParams[2];
        slots[slotId].devParams[3] = devParams[3];
    }
    else {
        slots[slotId].devParams[0] = 0;
        slots[slotId].devParams[1] = 0;
        slots[slotId].devParams[2] = 0;
        slots[slotId].devParams[3] = 0;
    }

    slots[slotId].rxDataReadySize = 0;
    slots[slotId].txDataReadySize = 0;
    slots[slotId].lastPollTimeUs = 0;
    slots[slotId].unrepliedRequests = 0;
}

typedef struct __attribute__((packed)) {
    uint8_t slotAndCmd;
    uint8_t  devId;
    uint8_t  protoVersion;
    uint8_t  crc1;
    uint16_t pollIntervalMs;
    uint16_t devFlags;
    uint8_t  devParams[4];
    uint8_t  crc2;
} uibPktIdentify_t;

typedef struct __attribute__((packed)) {
    uint8_t slotAndCmd;
    uint8_t devId;
    uint8_t protoVersion;
    uint8_t crc1;
} uibPktNotify_t;

typedef struct __attribute__((packed)) {
    uint8_t slotAndCmd;
    uint8_t crc1;
    uint8_t size;
    uint8_t data[0];
} uibPktRead_t;

typedef struct __attribute__((packed)) {
    uint8_t slotAndCmd;
    uint8_t size;
    uint8_t data[0];
} uibPktWrite_t;

typedef union {
    uibPktIdentify_t    ident;
    uibPktNotify_t      notify;
    uibPktRead_t        read;
    uibPktWrite_t       write;
} uibPktData_t;

static void processSlot(void)
{
    // First byte is command / slot
    const uint8_t cmd = slotDataBuffer[0] & 0xE0;
    const uint8_t slot = slotDataBuffer[0] & 0x1F;
    const uibPktData_t * pkt = (const uibPktData_t *)&slotDataBuffer[0];
    const int lastPacketByteIndex = slotDataBufferCount - 1;

    // CRC is calculated over the whole slot, including command byte(s) sent by FC. This ensures integrity of the transaction as a whole
    switch (cmd) {
        // Identify command (13 bytes)
        //      FC:     IDENTIFY[1] + DevID[1] + ProtoVersion [1] + CRC1[1]
        //      DEV:    PollInterval[2] + Flags[2] + DevParams[4] + CRC2[1]
        case UIB_COMMAND_IDENTIFY:
            if (slotDataBufferCount == sizeof(uibPktIdentify_t)) {
                if (crc8_dvb_s2_update(0x00, pkt, slotDataBufferCount - 1) == pkt->ident.crc2) {
                    // CRC valid - process valid IDENTIFY slot
                    registerDeviceSlot(slot, pkt->ident.devId, pkt->ident.devFlags, pkt->ident.pollIntervalMs * 1000, pkt->ident.devParams);
                    uibStats.discoveredDevices++;
                }
                else {
                    uibStats.failedCRC++;
                }

                // Regardless of CRC validity - discard buffer data
                slotDataBufferCount = 0;
            }
            break;

        // NOTIFY command (4 bytes)
        //      FC:     IDENTIFY[1] + DevID[1] + ProtoVersion [1] + CRC1[1]
        case UIB_COMMAND_NOTIFY:
            if (slotDataBufferCount == sizeof(uibPktNotify_t)) {
                // Record failed CRC. Do nothing else - NOTIFY is only sent for devices that already have a slot assigned
                if (crc8_dvb_s2_update(0x00, pkt, slotDataBufferCount - 1) != pkt->notify.crc1) {
                    uibStats.failedCRC++;
                }

                slotDataBufferCount = 0;
            }
            break;

        // Read command (min 4 bytes)
        //      FC:     READ[1] + CRC1[1]
        //      DEV:    DATA_LEN[1] + DATA[variable] + CRC2[1]
        case UIB_COMMAND_READ:
            if ((slotDataBufferCount >= sizeof(uibPktRead_t)) && (slotDataBufferCount == ((unsigned)pkt->read.size + 4)) && (pkt->read.size <= UIB_MAX_PACKET_SIZE)) {
                if (crc8_dvb_s2_update(0x00, pkt, slotDataBufferCount - 1) == slotDataBuffer[lastPacketByteIndex]) {
                    // CRC valid - process valid READ slot
                    // Check if this slot has read capability and is allocated
                    if (slots[slot].allocated && (slots[slot].deviceFlags & UIB_FLAG_HAS_READ)) {
                        if ((slots[slot].rxDataReadySize == 0) && (pkt->read.size > 0)) {
                            memcpy(slots[slot].rxPacket, pkt->read.data, pkt->read.size);
                            slots[slot].rxDataReadySize = pkt->read.size;
                        }

                        slots[slot].unrepliedRequests = 0;
                    }
                }
                else {
                    uibStats.failedCRC++;
                }

                // Regardless of CRC validity - discard buffer data
                slotDataBufferCount = 0;
            }
            break;

        // Write command (min 3 bytes)
        //      FC:     WRITE[1] + DATA_LEN[1] + DATA[variable] + CRC1[1]
        case UIB_COMMAND_WRITE:
            if ((slotDataBufferCount >= sizeof(uibPktWrite_t)) && (slotDataBufferCount == ((unsigned)pkt->write.size + 3)) && (pkt->write.size <= UIB_MAX_PACKET_SIZE)) {
                // Keep track of failed CRC events
                if (crc8_dvb_s2_update(0x00, pkt, slotDataBufferCount - 1) != slotDataBuffer[lastPacketByteIndex]) {
                    uibStats.failedCRC++;
                }

                // Regardless of CRC validity - discard buffer data
                slotDataBufferCount = 0;
            }
            break;

        default:
            break;
    }
}

static void processScheduledTransactions(timeUs_t currentTimeUs)
{
    int slotPrio = 0x100;
    int slotId = -1;

    // First - find device with highest priority that has the READ capability and is scheduled for READ
    for (int i = 0; i < UIB_MAX_SLOTS; i++) {
        // Only do scheduled READs on allocated and active slots
        if (!(slots[i].allocated && slots[i].activated))
            continue;

        if ((slots[i].deviceFlags & UIB_FLAG_HAS_READ) && ((currentTimeUs - slots[i].lastPollTimeUs) >= slots[i].pollIntervalUs) && (slotPrio > slots[i].deviceAddress)) {
            slotId = i;
            slotPrio = slots[i].deviceAddress;
        }
    }

    // READ command
    if (slotId >= 0) {
        sendRead(currentTimeUs, slotId);
        slots[slotId].unrepliedRequests++;
        slots[slotId].lastPollTimeUs = currentTimeUs;
        return;
    }

    // No READ command executed - check if we have data to WRITE
    slotPrio = 0x100;
    slotId = -1;

    for (int i = 0; i < UIB_MAX_SLOTS; i++) {
        // Only do WRITEs on allocated and active slots
        if (!(slots[i].allocated && slots[i].activated))
            continue;

        if (slots[i].txDataReadySize && (slots[i].deviceFlags & UIB_FLAG_HAS_WRITE) && (slotPrio > slots[i].deviceAddress)) {
            slotId = i;
            slotPrio = slots[i].deviceAddress;
        }
    }

    // WRITE command
    if (slotId >= 0) {
        sendWrite(currentTimeUs, slotId, slots[slotId].txPacket, slots[slotId].txDataReadySize);
        slots[slotId].unrepliedRequests++;
        slots[slotId].txDataReadySize = 0;
        return;
    }

    // Neither READ nor WRITE command are queued - issue IDENTIFY once in a while
    if ((currentTimeUs - refreshStartTimeUs) > UIB_REFRESH_INTERVAL_US) {
        // Send notifications for allocated slots
        if (slots[refreshSlot].allocated) {
            sendNotify(currentTimeUs, refreshSlot, slots[refreshSlot].deviceAddress);
            refreshStartTimeUs = currentTimeUs;
        }

        if (++refreshSlot >= UIB_MAX_SLOTS) {
            refreshSlot = 0;
        }
    }
}

static bool canSendNewRequest(timeUs_t currentTimeUs)
{
    return ((currentTimeUs - slotLastActivityUs) >= UIB_GUARD_INTERVAL_US);
}

void uavInterconnectBusTask(timeUs_t currentTimeUs)
{
    if (!uibInitialized)
        return;

    // Receive bytes to the buffer
    bool hasNewBytes = false;
    while (serialRxBytesWaiting(busPort) > 0) {
        uint8_t c = serialRead(busPort);
        if (slotDataBufferCount < (int)(sizeof(slotDataBuffer) / sizeof(slotDataBuffer[0]))) {
            slotDataBuffer[slotDataBufferCount++] = c;
            hasNewBytes = true;
        }

        slotLastActivityUs = currentTimeUs;
    }

    // Flush receive buffer if guard interval elapsed
    if ((currentTimeUs - slotLastActivityUs) >= UIB_GUARD_INTERVAL_US && slotDataBufferCount > 0) {
        uibStats.commandTimeouts++;
        slotDataBufferCount = 0;
    }

    // If we have new bytes - process packet
    if (hasNewBytes && slotDataBufferCount >= 4) {  // minimum transaction length is 4 bytes - no point in processing something smaller
        processSlot();
    }

    // Process request scheduling - we can initiate another slot if guard interval has elapsed and slot interval has elapsed as well
    if (canSendNewRequest(currentTimeUs)) {
        // We get here only if we can send requests - no timeout checking should be done beyond this point
        switch (busState) {
            case STATE_INITIALIZE:
                if ((currentTimeUs - slotStartTimeUs) > UIB_DISCOVERY_DELAY_US) {
                    discoveryAddress = 0;
                    switchState(STATE_DISCOVER);
                }
                break;

            case STATE_DISCOVER:
                {
                    int discoverySlot = findEmptySlot();
                    if (discoverySlot >= 0) {
                        sendDiscover(currentTimeUs, discoverySlot, discoveryAddress);
                        if (discoveryAddress == 0xFF) {
                            // All addresses have been polled
                            switchState(STATE_READY);
                        }
                        else {
                            // Query next address and stick here
                            discoveryAddress++;
                        }
                    }
                    else {
                        // All slots are allocated - can't discover more devices
                        refreshSlot = 0;
                        refreshStartTimeUs = currentTimeUs;
                        switchState(STATE_READY);
                    }
                }
                break;

            case STATE_READY:
                // Bus ready - process scheduled transfers
                processScheduledTransactions(currentTimeUs);
                break;
        }
    }
}

void uavInterconnectBusInit(void)
{
    for (int i = 0; i < UIB_MAX_SLOTS; i++) {
        slots[i].allocated = false;
        slots[i].activated = false;
    }

    serialPortConfig_t * portConfig = findSerialPortConfig(FUNCTION_UAV_INTERCONNECT);
    if (!portConfig)
        return;

    baudRate_e baudRateIndex = portConfig->peripheral_baudrateIndex;
    busPort = openSerialPort(portConfig->identifier, FUNCTION_UAV_INTERCONNECT, NULL, NULL, baudRates[baudRateIndex], MODE_RXTX, UIB_PORT_OPTIONS);
    if (!busPort)
        return;

    slotStartTimeUs = 0;
    uibInitialized = true;
}

bool uavInterconnectBusIsInitialized(void)
{
    return uibInitialized;
}

bool uibDetectAndActivateDevice(uint8_t devId)
{
    uavInterconnectSlot_t * slot = findDevice(devId);
    if (slot == NULL)
        return false;

    /* We have discovered device in out registry and code is asking to access it - activate device */
    slot->activated = true;

    return slot->allocated;
}

bool uibRegisterDevice(uint8_t devId)
{
    int slotId = findEmptySlot();
    if (slotId >= 0) {
        registerDeviceSlot(slotId, devId, UIB_FLAG_HAS_WRITE, 0, NULL);
        return uibDetectAndActivateDevice(devId);
    }

    return false;
}

bool uibGetDeviceParams(uint8_t devId, uint8_t * params)
{
    uavInterconnectSlot_t * slot = findDevice(devId);
    if (slot == NULL || params == NULL)
        return false;

    params[0] = slot->devParams[0];
    params[1] = slot->devParams[1];
    params[2] = slot->devParams[2];
    params[3] = slot->devParams[3];
    return true;
}

timeUs_t uibGetPollRateUs(uint8_t devId)
{
    uavInterconnectSlot_t * slot = findDevice(devId);
    if (slot == NULL)
        return 0;

    return slot->pollIntervalUs;
}

uint32_t uibGetUnansweredRequests(uint8_t devId)
{
    uavInterconnectSlot_t * slot = findDevice(devId);
    if (slot == NULL)
        return 0;

    return (slot->unrepliedRequests < 2) ? 0 : slot->unrepliedRequests - 1;
}

uint8_t uibDataAvailable(uint8_t devId)
{
    uavInterconnectSlot_t * slot = findDevice(devId);
    if (slot == NULL)
        return 0;

    return slot->rxDataReadySize;
}

uint8_t uibRead(uint8_t devId, uint8_t * buffer, uint8_t bufSize)
{
    uavInterconnectSlot_t * slot = findDevice(devId);
    if (slot == NULL)
        return 0;

    // If no READ capability - fail
    if (!(slot->deviceFlags & UIB_FLAG_HAS_READ))
        return false;

    // If no data ready - fail
    if (!slot->rxDataReadySize)
        return 0;

    uint8_t bytes = slot->rxDataReadySize;
    memcpy(buffer, slot->rxPacket, MIN(bytes, bufSize));
    slot->rxDataReadySize = 0;

    return bytes;
}

static bool slotCanWrite(const uavInterconnectSlot_t * slot)
{
    // If no WRITE capability - fail
    if (!(slot->deviceFlags & UIB_FLAG_HAS_WRITE))
        return false;

    // If we have unsent data in the buffer - fail
    if (slot->txDataReadySize > 0)
        return false;

    return true;
}

bool uibCanWrite(uint8_t devId)
{
    // No device available
    uavInterconnectSlot_t * slot = findDevice(devId);
    if (slot == NULL)
        return false;

    return slotCanWrite(slot);
}

bool uibWrite(uint8_t devId, const uint8_t * buffer, uint8_t len)
{
    uavInterconnectSlot_t * slot = findDevice(devId);
    if (slot == NULL)
        return false;

    if (slotCanWrite(slot)) {
        memcpy(slot->txPacket, buffer, len);
        slot->txDataReadySize = len;
        return true;
    }

    return false;
}
#endif
