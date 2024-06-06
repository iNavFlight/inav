/*
 * This file is part of INAV.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "platform.h"

#ifdef USE_SERIAL_GIMBAL

#include <common/crc.h>
#include <common/utils.h>
#include <common/maths.h>
#include <build/debug.h>

#include <drivers/gimbal_common.h>
#include <drivers/serial.h>

#include <io/gimbal_serial.h>
#include <io/serial.h>

#include <rx/rx.h>
#include <fc/rc_modes.h>

STATIC_ASSERT(sizeof(gimbalHtkAttitudePkt_t) == 10, gimbalHtkAttitudePkt_t_size_not_10);

#define GIMBAL_SERIAL_BUFFER_SIZE 512
static volatile uint8_t txBuffer[GIMBAL_SERIAL_BUFFER_SIZE];

static serialPort_t *htkPort = NULL;

gimbalVTable_t gimbalSerialVTable = {
    .process = gimbalSerialProcess,
    .getDeviceType = gimbalSerialGetDeviceType,
    .isReady = gimbalSerialIsReady

};

gimbalDevice_t serialGimbalDevice = {
    .vTable = &gimbalSerialVTable

};

gimbalDevType_e gimbalSerialGetDeviceType(const gimbalDevice_t *gimbalDevice)
{
    UNUSED(gimbalDevice);
    return GIMBAL_DEV_SERIAL;
}

bool gimbalSerialIsReady(const gimbalDevice_t *gimbalDevice)
{
    return htkPort != NULL && gimbalDevice->vTable != NULL;
}

bool gimbalSerialInit(void)
{
    if(gimbalSerialDetect()) {
        gimbalCommonSetDevice(&serialGimbalDevice);
        return true;
    }

    return false;
}

#ifdef GIMBAL_UNIT_TEST
bool gimbalSerialDetect(void)
{
    return false;
}
#else
bool gimbalSerialDetect(void)
{

    SD(fprintf(stderr, "[GIMBAL]: serial Detect...\n"));
    serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_GIMBAL);

    if (portConfig) {
        SD(fprintf(stderr, "[GIMBAL]: found port...\n"));
        htkPort = openSerialPort(portConfig->identifier, FUNCTION_GIMBAL, NULL, NULL,
                baudRates[portConfig->peripheral_baudrateIndex], MODE_RXTX, SERIAL_NOT_INVERTED);

        if (htkPort) {
            SD(fprintf(stderr, "[GIMBAL]: port open!\n"));
            htkPort->txBuffer = txBuffer;
            htkPort->txBufferSize = GIMBAL_SERIAL_BUFFER_SIZE;
            htkPort->txBufferTail = 0;
            htkPort->txBufferHead = 0;

            return true;
        }
    }

    SD(fprintf(stderr, "[GIMBAL]: port not found :(...\n"));
    return false;
}
#endif

#ifdef GIMBAL_UNIT_TEST
void gimbalSerialProcess(gimbalDevice_t *gimbalDevice, timeUs_t currentTime)
{
}
#else
void gimbalSerialProcess(gimbalDevice_t *gimbalDevice, timeUs_t currentTime)
{
    UNUSED(currentTime);

    if (!gimbalSerialIsReady(gimbalDevice)) {
        SD(fprintf(stderr, "[GIMBAL] gimbal not ready...\n"));
        return;
    }

    gimbalHtkAttitudePkt_t attittude = {
        .sync = {HTKATTITUDE_SYNC0, HTKATTITUDE_SYNC1},
        .mode = GIMBAL_MODE_DEFAULT
    };

    const gimbalConfig_t *cfg = gimbalConfig();

    int yaw = 1500;
    int pitch = 1500;
    int roll = 1500;

    if (IS_RC_MODE_ACTIVE(BOXGIMBALCENTER)) {
        attittude.mode = GIMBAL_MODE_FOLLOW;
    } else if (IS_RC_MODE_ACTIVE(BOXGIMBALPLOCK)) {
        attittude.mode = GIMBAL_MODE_PITCH_LOCK;
    } else if (IS_RC_MODE_ACTIVE(BOXGIMBALPRLOCK)) {
        attittude.mode = GIMBAL_MODE_PITCH_ROLL_LOCK;
    }

    if (rxAreFlightChannelsValid() && !IS_RC_MODE_ACTIVE(BOXGIMBALCENTER)) {
        if (cfg->panChannel > 0) {
            yaw = rxGetChannelValue(cfg->panChannel - 1);
            // const rxChannelRangeConfig_t *channelRanges =
            // rxChannelRangeConfigs(cfg->pitchChannel - 1);
            if (yaw < 1000) {
                yaw = 1000;
            } else if (yaw > 2000) {
                yaw = 2000;
            }
        }

        if (cfg->tiltChannel > 0) {
            pitch = rxGetChannelValue(cfg->tiltChannel - 1);
            // const rxChannelRangeConfig_t *channelRanges =
            // rxChannelRangeConfigs(cfg->pitchChannel - 1);
            if (pitch < 1000) {
                pitch = 1000;
            } else if (pitch > 2000) {
                pitch = 2000;
            }
        }

        if (cfg->rollChannel > 0) {
            roll = rxGetChannelValue(cfg->rollChannel - 1);
            // const rxChannelRangeConfig_t *channelRanges =
            // rxChannelRangeConfigs(cfg->pitchChannel - 1);
            if (roll < 1000) {
                roll = 1000;
            } else if (roll > 2000) {
                roll = 2000;
            }
        }
    }

    attittude.sensibility = cfg->sensitivity; //gimbal_scale5(-16, 15, -16, 15, cfg->sensitivity);

    attittude.yaw = gimbal_scale12(1000, 2000, yaw);
    attittude.pitch = gimbal_scale12(1000, 2000, pitch);
    attittude.roll = gimbal_scale12(1000, 2000, roll);

    uint16_t crc16 = 0;
    uint8_t *b = (uint8_t *)&attittude;
    for (uint8_t i = 0; i < sizeof(gimbalHtkAttitudePkt_t) - 2; i++) {
        crc16 = crc16_ccitt(crc16, *(b + i));
    }
    attittude.crch = (crc16 >> 8) & 0xFF;
    attittude.crcl = crc16 & 0xFF;

    serialBeginWrite(htkPort);
    serialWriteBuf(htkPort, (uint8_t *)&attittude, sizeof(gimbalHtkAttitudePkt_t));
    serialEndWrite(htkPort);
}
#endif

int16_t gimbal_scale12(int16_t inputMin, int16_t inputMax, int16_t value)
{
    int16_t ret = 0;
    ret = scaleRange(value, inputMin, inputMax, -2048, 2047);
    return ret;
}

#endif