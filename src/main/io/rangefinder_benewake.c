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
#include <stdint.h>
#include <ctype.h>
#include <math.h>

#include "platform.h"

#include "build/build_config.h"
#include "build/debug.h"

#include "common/maths.h"

#include "io/serial.h"

#if defined(USE_RANGEFINDER_BENEWAKE)
#include "drivers/rangefinder/rangefinder_virtual.h"
#include "drivers/time.h"
#include "io/rangefinder.h"

typedef struct __attribute__((packed)) {
    uint8_t     header0;
    uint8_t     header1;
    uint8_t     distL;
    uint8_t     distH;
    uint8_t     strengthL;
    uint8_t     strengthH;
    uint8_t     res;
    uint8_t     rawQual;
    uint8_t     checksum;
} tfminiPacket_t;

#define BENEWAKE_PACKET_SIZE    sizeof(tfminiPacket_t)
#define BENEWAKE_MIN_QUALITY    0

static serialPort_t * serialPort = NULL;
static serialPortConfig_t * portConfig;
static uint8_t  buffer[BENEWAKE_PACKET_SIZE];
static unsigned bufferPtr;

static bool hasNewData = false;
static int32_t sensorData = RANGEFINDER_NO_NEW_DATA;

static bool benewakeRangefinderDetect(void)
{
    portConfig = findSerialPortConfig(FUNCTION_RANGEFINDER);
    if (!portConfig) {
        return false;
    }

    return true;
}

static void benewakeRangefinderInit(void)
{
    if (!portConfig) {
        return;
    }

    serialPort = openSerialPort(portConfig->identifier, FUNCTION_RANGEFINDER, NULL, NULL, baudRates[BAUD_115200], MODE_RX, SERIAL_NOT_INVERTED);
    if (!serialPort) {
        return;
    }

    bufferPtr = 0;
}

static void benewakeRangefinderUpdate(void)
{
    tfminiPacket_t *tfminiPacket = (tfminiPacket_t *)buffer;
    while (serialRxBytesWaiting(serialPort) > 0) {
        uint8_t c = serialRead(serialPort);

        // Add byte to buffer
        if (bufferPtr < BENEWAKE_PACKET_SIZE) {
            buffer[bufferPtr++] = c;
        }

        // Check header bytes
        if ((bufferPtr == 1) && (tfminiPacket->header0 != 0x59)) {
            bufferPtr = 0;
            continue;
        }

        if ((bufferPtr == 2) && (tfminiPacket->header1 != 0x59)) {
            bufferPtr = 0;
            continue;
        }

        // Check for complete packet
        if (bufferPtr == BENEWAKE_PACKET_SIZE) {
            const uint8_t checksum = buffer[0] + buffer[1] + buffer[2] + buffer[3] + buffer[4] + buffer[5] + buffer[6] + buffer[7];
            if (tfminiPacket->checksum == checksum) {
                // Valid packet
                hasNewData = true;
                sensorData = (tfminiPacket->distL << 0) | (tfminiPacket->distH << 8);

                uint16_t qual = (tfminiPacket->strengthL << 0) | (tfminiPacket->strengthH << 8);

                if (sensorData == 0 || qual <= BENEWAKE_MIN_QUALITY) {
                    sensorData = -1;
                }
            }

            // Prepare for new packet
            bufferPtr = 0;
        }
    }
}

static int32_t benewakeRangefinderGetDistance(void)
{
    if (hasNewData) {
        hasNewData = false;
        return (sensorData > 0) ? (sensorData) : RANGEFINDER_OUT_OF_RANGE;
    }
    else {
        return RANGEFINDER_NO_NEW_DATA;
    }
}

virtualRangefinderVTable_t rangefinderBenewakeVtable = {
    .detect = benewakeRangefinderDetect,
    .init = benewakeRangefinderInit,
    .update = benewakeRangefinderUpdate,
    .read = benewakeRangefinderGetDistance
};

#endif
