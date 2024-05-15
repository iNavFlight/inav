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

#if defined(USE_RANGEFINDER_USD1_V0)
#include "drivers/rangefinder/rangefinder_virtual.h"

#define USD1_HDR_V0 72 // Header Byte for beta V0 of USD1_Serial (0x48)

#define USD1_PACKET_SIZE 3
#define USD1_KEEP_DATA_TIMEOUT 2000 // 2s


static serialPort_t * serialPort = NULL;
static serialPortConfig_t * portConfig;

static bool hasNewData = false;
static bool hasEverData = false;
static uint8_t  lineBuf[USD1_PACKET_SIZE];
static int32_t sensorData = RANGEFINDER_NO_NEW_DATA;
static timeMs_t lastProtocolActivityMs;

static bool usd1RangefinderDetect(void)
{
    portConfig = findSerialPortConfig(FUNCTION_RANGEFINDER);
    if (!portConfig) {
        return false;
    }

    return true;
}

static void usd1RangefinderInit(void)
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

static void usd1RangefinderUpdate(void)
{
    float sum = 0;
    uint16_t count = 0;
    uint8_t  index = 0;

    while (serialRxBytesWaiting(serialPort) > 0) {
        uint8_t c = serialRead(serialPort);

        if (c == USD1_HDR_V0 && index == 0) {
            lineBuf[index] = c;
            index = 1;
            continue;
        }

        if (index > 0) {
            lineBuf[index] = c;
            index++;
            if (index == 3) {
                index = 0;
                sum += (float)((lineBuf[2]&0x7F) * 128 + (lineBuf[1]&0x7F));
                count++;
            }
        }
    }

    if (count == 0) {
        return;
    }

    hasNewData = true;
    hasEverData = true;
    lastProtocolActivityMs = millis();
    sensorData = (int32_t)(2.5f * sum / (float)count);
}

static int32_t usd1RangefinderGetDistance(void)
{
    int32_t altitude = (sensorData > 0) ? (sensorData) : RANGEFINDER_OUT_OF_RANGE;

    if (hasNewData) {
        hasNewData = false;
        return altitude;
    }
    else {
        //keep last value for timeout, because radar sends data only if change
        if ((millis() - lastProtocolActivityMs) < USD1_KEEP_DATA_TIMEOUT) {
            return altitude;
        }

        return hasEverData ? RANGEFINDER_OUT_OF_RANGE : RANGEFINDER_NO_NEW_DATA;
    }
}

virtualRangefinderVTable_t rangefinderUSD1Vtable = {
    .detect = usd1RangefinderDetect,
    .init = usd1RangefinderInit,
    .update = usd1RangefinderUpdate,
    .read = usd1RangefinderGetDistance
};

#endif
