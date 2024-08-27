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

#if defined(USE_RANGEFINDER_A02)
#include "drivers/rangefinder/rangefinder_virtual.h"

#define A02_HDR 0xFF // Header Byte

typedef struct __attribute__((packed)) {
    uint8_t     header; // Header Byte
    uint8_t     data_h; // Target distance high 8 bit
    uint8_t     data_l; // Target distance low 8 bit
    uint8_t     sum;    // Checksum
} a02Packet_t;

#define A02_PACKET_SIZE    sizeof(a02Packet_t)
#define A02_TIMEOUT_MS     500 // 0.5s

static bool hasNewData = false;
static bool hasRecievedData = false;
static serialPort_t * serialPort = NULL;
static serialPortConfig_t * portConfig;
static uint8_t buffer[A02_PACKET_SIZE];
static unsigned bufferPtr;

static int32_t sensorData = RANGEFINDER_NO_NEW_DATA;
static timeMs_t lastProtocolActivityMs;

static bool a02RangefinderDetect(void)
{
    portConfig = findSerialPortConfig(FUNCTION_RANGEFINDER);
    if (!portConfig) {
        return false;
    }

    return true;
}

static void a02RangefinderInit(void)
{
    if (!portConfig) {
        return;
    }

    serialPort = openSerialPort(portConfig->identifier, FUNCTION_RANGEFINDER, NULL, NULL, 9600, MODE_RX, SERIAL_NOT_INVERTED);
    if (!serialPort) {
        return;
    }

    lastProtocolActivityMs = 0;
}

static void a02RangefinderUpdate(void)
{
    a02Packet_t *a02Packet = (a02Packet_t *)buffer;

    while (serialRxBytesWaiting(serialPort) > 0) {
        uint8_t c = serialRead(serialPort);

        if (bufferPtr < A02_PACKET_SIZE) {
            buffer[bufferPtr++] = c;
        }

        // Check for valid header
        if ((bufferPtr == 1) && (a02Packet->header != A02_HDR)) {
            bufferPtr = 0;
            continue;
        }

        // Check for complete packet
        if (bufferPtr == A02_PACKET_SIZE) {
            
            // Check for valid checksum
            if(a02Packet->sum != (a02Packet->header + a02Packet->data_h + a02Packet->data_l)) {
                
                bufferPtr = 0;
            
            } else {

                // Valid packet
                hasNewData = true;
                hasRecievedData = true;
                lastProtocolActivityMs = millis();

                sensorData = ((a02Packet->data_h * 0xFF) + a02Packet->data_l);

                // Prepare for new packet
                bufferPtr = 0;
            }
        }
    }
}

static int32_t a02RangefinderGetDistance(void)
{
    int32_t altitude = (sensorData > 0) ? (sensorData) : RANGEFINDER_OUT_OF_RANGE;

    if (hasNewData) {
        hasNewData = false;
        return altitude;
    }
    else {

        if ((millis() - lastProtocolActivityMs) < A02_TIMEOUT_MS) {
            return altitude;
        }

        return hasRecievedData ? RANGEFINDER_OUT_OF_RANGE : RANGEFINDER_NO_NEW_DATA;
    }
}

virtualRangefinderVTable_t rangefinderA02Vtable = {
    .detect = a02RangefinderDetect,
    .init = a02RangefinderInit,
    .update = a02RangefinderUpdate,
    .read = a02RangefinderGetDistance
};

#endif
