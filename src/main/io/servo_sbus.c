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
#include "common/crc.h"

#include "io/serial.h"
#include "io/servo_sbus.h"
#include "rx/sbus_channels.h"

#include "telemetry/telemetry.h"

#if defined(USE_SERVO_SBUS)

#define SERVO_SBUS_UART_BAUD            100000
#define SERVO_SBUS_OPTIONS              (SBUS_PORT_OPTIONS | SERIAL_INVERTED | SERIAL_UNIDIR)

static serialPort_t * servoSbusPort = NULL;
static sbusFrame_t sbusFrame;

bool sbusServoInitialize(void)
{
    serialPortConfig_t * portConfig;

    // Avoid double initialization
    if (servoSbusPort) {
        return true;
    }

    portConfig = findSerialPortConfig(FUNCTION_SERVO_SERIAL);
    if (!portConfig) {
        return false;
    }

    bool servoserialCheckRxPortShared(const serialPortConfig_t *portConfig)
    {
        return portConfig->functionMask & FUNCTION_RX_SERIAL;
    }
        if (servoserialCheckRxPortShared(portConfig)) {
                servoSbusPort = telemetrySharedPort;
        } else {	
        servoSbusPort = openSerialPort(portConfig->identifier, FUNCTION_SERVO_SERIAL, NULL, NULL, SERVO_SBUS_UART_BAUD, MODE_TX, SERVO_SBUS_OPTIONS);
        if (!servoSbusPort) {
                return false;
        }
    }

    // SBUS V1 magical values
    sbusFrame.syncByte = 0x0F;
    sbusFrame.channels.flags = 0;
    sbusFrame.channels.chan0  = sbusEncodeChannelValue(1500);
    sbusFrame.channels.chan1  = sbusEncodeChannelValue(1500);
    sbusFrame.channels.chan2  = sbusEncodeChannelValue(1500);
    sbusFrame.channels.chan3  = sbusEncodeChannelValue(1500);
    sbusFrame.channels.chan4  = sbusEncodeChannelValue(1500);
    sbusFrame.channels.chan5  = sbusEncodeChannelValue(1500);
    sbusFrame.channels.chan6  = sbusEncodeChannelValue(1500);
    sbusFrame.channels.chan7  = sbusEncodeChannelValue(1500);
    sbusFrame.channels.chan8  = sbusEncodeChannelValue(1500);
    sbusFrame.channels.chan9  = sbusEncodeChannelValue(1500);
    sbusFrame.channels.chan10 = sbusEncodeChannelValue(1500);
    sbusFrame.channels.chan11 = sbusEncodeChannelValue(1500);
    sbusFrame.channels.chan12 = sbusEncodeChannelValue(1500);
    sbusFrame.channels.chan13 = sbusEncodeChannelValue(1500);
    sbusFrame.channels.chan14 = sbusEncodeChannelValue(1500);
    sbusFrame.channels.chan15 = sbusEncodeChannelValue(1500);
    sbusFrame.endByte = 0x00;

    return true;
}

void sbusServoUpdate(uint8_t index, uint16_t value)
{
    switch(index) {
        case 0: sbusFrame.channels.chan0 = sbusEncodeChannelValue(value); break;
        case 1: sbusFrame.channels.chan1 = sbusEncodeChannelValue(value); break;
        case 2: sbusFrame.channels.chan2 = sbusEncodeChannelValue(value); break;
        case 3: sbusFrame.channels.chan3 = sbusEncodeChannelValue(value); break;
        case 4: sbusFrame.channels.chan4 = sbusEncodeChannelValue(value); break;
        case 5: sbusFrame.channels.chan5 = sbusEncodeChannelValue(value); break;
        case 6: sbusFrame.channels.chan6 = sbusEncodeChannelValue(value); break;
        case 7: sbusFrame.channels.chan7 = sbusEncodeChannelValue(value); break;
        case 8: sbusFrame.channels.chan8 = sbusEncodeChannelValue(value); break;
        case 9: sbusFrame.channels.chan9 = sbusEncodeChannelValue(value); break;
        case 10: sbusFrame.channels.chan10 = sbusEncodeChannelValue(value); break;
        case 11: sbusFrame.channels.chan11 = sbusEncodeChannelValue(value); break;
        case 12: sbusFrame.channels.chan12 = sbusEncodeChannelValue(value); break;
        case 13: sbusFrame.channels.chan13 = sbusEncodeChannelValue(value); break;
        case 14: sbusFrame.channels.chan14 = sbusEncodeChannelValue(value); break;
        case 15: sbusFrame.channels.chan15 = sbusEncodeChannelValue(value); break;
        default:
            break;
    }
}

void sbusServoSendUpdate(void)
{
    // Check if the port is initialized
    if (!servoSbusPort) {
        return;
    }

    // Skip update if previous one is not yet fully sent
    // This helps to avoid buffer overflow and evenyually the data corruption
    if (!isSerialTransmitBufferEmpty(servoSbusPort)) {
        return;
    }

    serialWriteBuf(servoSbusPort, (const uint8_t *)&sbusFrame, sizeof(sbusFrame));
}

#endif
