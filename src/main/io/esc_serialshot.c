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
#include "io/esc_serialshot.h"

#if defined(USE_SERIALSHOT)

#define SERIALSHOT_UART_BAUD        921600
#define THROTTLE_DATA_FRAME_SIZE    9

typedef struct __attribute__((packed)) {
    uint8_t hdr;            // Header/version marker
    uint8_t motorData[6];   // 12 bit per motor
    uint8_t crc;            // CRC8/DVB-T of hdr & motorData
} serialShortPacket_t;


static serialShortPacket_t txPkt;
static uint8_t txBuffer[THROTTLE_DATA_FRAME_SIZE];
static uint16_t motorValues[4];
static serialPort_t * escPort = NULL;
static serialPortConfig_t * portConfig;

bool serialshotInitialize(void)
{
    // Avoid double initialization
    if (escPort) {
        return true;
    }

    portConfig = findSerialPortConfig(FUNCTION_SERIALSHOT);
    if (!portConfig) {
        return false;
    }

    escPort = openSerialPort(portConfig->identifier, FUNCTION_SERIALSHOT, NULL, NULL, SERIALSHOT_UART_BAUD, MODE_RXTX, SERIAL_NOT_INVERTED | SERIAL_UNIDIR);
    if (!escPort) {
        return false;
    }

    return true;
}

void serialshotUpdateMotor(int index, uint16_t value)
{
    if (index < 0 && index > 3) {
        return;
    }

    motorValues[index] = value;
}

void serialshotSendUpdate(void)
{
    // Skip update if previous one is not yet fully sent
    // This helps to avoid buffer overflow and evenyually the data corruption
    if (!isSerialTransmitBufferEmpty(escPort)) {
        return;
    }

#if 0
    // Build motor values
    txBuffer[0 * 2 + 0] = 0xFF & (motorValues[0] >> 8);
    txBuffer[0 * 2 + 1] = 0xFF & (motorValues[0] >> 0);
    txBuffer[1 * 2 + 0] = 0xFF & (motorValues[1] >> 8);
    txBuffer[1 * 2 + 1] = 0xFF & (motorValues[1] >> 0);
    txBuffer[2 * 2 + 0] = 0xFF & (motorValues[2] >> 8);
    txBuffer[2 * 2 + 1] = 0xFF & (motorValues[2] >> 0);
    txBuffer[3 * 2 + 0] = 0xFF & (motorValues[3] >> 8);
    txBuffer[3 * 2 + 1] = 0xFF & (motorValues[3] >> 0);

    // Calculate checksum
    txBuffer[8] = 
        txBuffer[0] + txBuffer[1] +
        txBuffer[2] + txBuffer[3] +
        txBuffer[4] + txBuffer[5] +
        txBuffer[6] + txBuffer[7];

    // Send data 
    serialWriteBuf(escPort, txBuffer, THROTTLE_DATA_FRAME_SIZE);
#else 
    txPkt.hdr = 0x00;

    txPkt.motorData[0] = motorValues[0] & 0x00FF;
    txPkt.motorData[1] = motorValues[1] & 0x00FF;
    txPkt.motorData[2] = motorValues[2] & 0x00FF;
    txPkt.motorData[3] = motorValues[3] & 0x00FF;
    txPkt.motorData[4] = (((motorValues[0] & 0xF00) >> 8) << 0) | (((motorValues[1] & 0xF00) >> 8) << 4);
    txPkt.motorData[5] = (((motorValues[2] & 0xF00) >> 8) << 0) | (((motorValues[3] & 0xF00) >> 8) << 4);

    txPkt.crc = crc8_dvb_s2(0x00, txPkt.hdr);
    txPkt.crc = crc8_dvb_s2_update(txPkt.crc, txPkt.motorData, sizeof(txPkt.motorData));

    serialWriteBuf(escPort, (const uint8_t *)&txPkt, sizeof(txPkt));
#endif
}

#endif
