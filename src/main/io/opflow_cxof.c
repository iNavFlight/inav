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

#if defined(USE_OPFLOW_CXOF)
#include "drivers/opflow/opflow_virtual.h"
#include "drivers/time.h"
#include "io/opflow.h"

#define CXOF_PACKET_SIZE    9
static serialPort_t * flowPort = NULL;
static serialPortConfig_t * portConfig;
static uint8_t  buffer[10];
static int      bufferPtr;

typedef struct __attribute__((packed)) {
    uint8_t     header;     // 0xFE
    uint8_t     res0;       // Seems to be 0x04 all the time
    int16_t     motionX;
    int16_t     motionY;
    int8_t      motionT;    // ???
    uint8_t     squal;      // Not sure about this
    uint8_t     footer;     // 0xAA
} cxofPacket_t;

static bool cxofOpflowDetect(void)
{
    portConfig = findSerialPortConfig(FUNCTION_OPTICAL_FLOW);
    if (!portConfig) {
        return false;
    }

    return true;
}

static bool cxofOpflowInit(void)
{
    if (!portConfig) {
        return false;
    }

    flowPort = openSerialPort(portConfig->identifier, FUNCTION_OPTICAL_FLOW, NULL, NULL, baudRates[BAUD_19200], MODE_RX, SERIAL_NOT_INVERTED);
    if (!flowPort) {
        return false;
    }

    bufferPtr = 0;

    return true;
}

static bool cxofOpflowUpdate(opflowData_t * data)
{
    static timeUs_t previousTimeUs = 0;
    const timeUs_t currentTimeUs = micros();

    bool newPacket = false;
    opflowData_t tmpData = { 0 };

    while (serialRxBytesWaiting(flowPort) > 0) {
        uint8_t c = serialRead(flowPort);

        // Wait for header
        if (bufferPtr == 0) {
            if (c != 0xFE) {
                break;
            }
        }

        // Consume received byte
        if (bufferPtr < CXOF_PACKET_SIZE) {
            buffer[bufferPtr++] = c;

            if (bufferPtr == CXOF_PACKET_SIZE) {
                cxofPacket_t * pkt = (cxofPacket_t *)&buffer[0];

                if (pkt->header == 0xFE && pkt->footer == 0xAA) {
                    // Valid packet
                    tmpData.deltaTime += (currentTimeUs - previousTimeUs);
                    tmpData.flowRateRaw[0] += pkt->motionX;
                    tmpData.flowRateRaw[1] += pkt->motionY;
                    tmpData.flowRateRaw[2] = 0;
                    tmpData.quality = (constrain(pkt->squal, 64, 78) - 64) * 100 / 14;

                    previousTimeUs = currentTimeUs;
                    newPacket = true;
                }

                // Reset the decoder
                bufferPtr = 0;
            }
        }
        else {
            // In case of buffer overflow - reset the decoder
            bufferPtr = 0;
        }
    }

    if (newPacket) {
        *data = tmpData;
    }

    return newPacket;
}

virtualOpflowVTable_t opflowCxofVtable = {
    .detect = cxofOpflowDetect,
    .init = cxofOpflowInit,
    .update = cxofOpflowUpdate
};

#endif