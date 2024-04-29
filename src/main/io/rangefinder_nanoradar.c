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

#include <stdbool.h>
#include <ctype.h>

#include "platform.h"
#include "io/serial.h"
#include "drivers/time.h"

#if defined(USE_RANGEFINDER_NANORADAR)
#include "drivers/rangefinder/rangefinder_virtual.h"

#define NANORADAR_HDR 0xAA // Header Byte
#define NANORADAR_END 0x55

#define NANORADAR_COMMAND_TARGET_INFO 0x70C

typedef struct __attribute__((packed)) {
    uint8_t     header0;
    uint8_t     header1;
    uint8_t     commandH;
    uint8_t     commandL;
    uint8_t     index; // Target ID
    uint8_t     rcs; // The section of radar reflection
    uint8_t     rangeH; // Target distance high 8 bi
    uint8_t     rangeL; // Target distance low 8 bit
    uint8_t     rsvd1;
    uint8_t     info; // VrelH Rsvd1 RollCount
    uint8_t     vrelL;
    uint8_t     SNR; // Signal-Noise Ratio
    uint8_t     end0;
    uint8_t     end1;
} nanoradarPacket_t;

#define NANORADAR_PACKET_SIZE    sizeof(nanoradarPacket_t)
#define NANORADAR_TIMEOUT_MS     2000 // 2s

static bool hasNewData = false;
static bool hasEverData = false;
static serialPort_t * serialPort = NULL;
static serialPortConfig_t * portConfig;
static uint8_t buffer[NANORADAR_PACKET_SIZE];
static unsigned bufferPtr;

static int32_t sensorData = RANGEFINDER_NO_NEW_DATA;
static timeMs_t lastProtocolActivityMs;

static bool nanoradarRangefinderDetect(void)
{
    portConfig = findSerialPortConfig(FUNCTION_RANGEFINDER);
    if (!portConfig) {
        return false;
    }

    return true;
}

static void nanoradarRangefinderInit(void)
{
    if (!portConfig) {
        return;
    }

    serialPort = openSerialPort(portConfig->identifier, FUNCTION_RANGEFINDER, NULL, NULL, 115200, MODE_RXTX, SERIAL_NOT_INVERTED);
    if (!serialPort) {
        return;
    }

    lastProtocolActivityMs = 0;
}

static void nanoradarRangefinderUpdate(void)
{

    nanoradarPacket_t *nanoradarPacket = (nanoradarPacket_t *)buffer;
    while (serialRxBytesWaiting(serialPort) > 0) {
        uint8_t c = serialRead(serialPort);

        if (bufferPtr < NANORADAR_PACKET_SIZE) {
            buffer[bufferPtr++] = c;
        }

        if ((bufferPtr == 1) && (nanoradarPacket->header0 != NANORADAR_HDR)) {
            bufferPtr = 0;
            continue;
        }

        if ((bufferPtr == 2) && (nanoradarPacket->header1 != NANORADAR_HDR)) {
            bufferPtr = 0;
            continue;
        }

        //only target info packet we are interested
        if (bufferPtr == 4) {
            uint16_t command = nanoradarPacket->commandH + (nanoradarPacket->commandL * 0x100);

            if (command != NANORADAR_COMMAND_TARGET_INFO) {
                bufferPtr = 0;
                continue;
            }
        }

        // Check for complete packet
        if (bufferPtr == NANORADAR_PACKET_SIZE) {
            if (nanoradarPacket->end0 == NANORADAR_END && nanoradarPacket->end1 == NANORADAR_END) {
                // Valid packet
                hasNewData = true;
                hasEverData = true;
                lastProtocolActivityMs = millis();

                sensorData = ((nanoradarPacket->rangeH * 0x100) + nanoradarPacket->rangeL);
            }

            // Prepare for new packet
            bufferPtr = 0;
        }
    }
}

static int32_t nanoradarRangefinderGetDistance(void)
{
    int32_t altitude = (sensorData > 0) ? (sensorData) : RANGEFINDER_OUT_OF_RANGE;

    if (hasNewData) {
        hasNewData = false;
        return altitude;
    }
    else {
        //keep last value for timeout, because radar sends data only if change
        if ((millis() - lastProtocolActivityMs) < NANORADAR_TIMEOUT_MS) {
            return altitude;
        }

        return hasEverData ? RANGEFINDER_OUT_OF_RANGE : RANGEFINDER_NO_NEW_DATA;
    }
}

virtualRangefinderVTable_t rangefinderNanoradarVtable = {
    .detect = nanoradarRangefinderDetect,
    .init = nanoradarRangefinderInit,
    .update = nanoradarRangefinderUpdate,
    .read = nanoradarRangefinderGetDistance
};

#endif
