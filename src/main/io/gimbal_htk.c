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

#include <common/crc.h>

#include <drivers/gimbal_common.h>
#include <io/gimbal_htk.h>

#include <rx/rx.h>


void gimbal_htk_update(void)
{
    const gimbalConfig_t *cfg = gimbalConfig();

    GimbalHtkAttitudePkt_t attittude = {
        .sync = {HTKATTITUDE_SYNC0, HTKATTITUDE_SYNC1},
        .mode = GIMBAL_MODE_FOLLOW,
    };

    int yaw = 1500;
    int pitch = 1500;
    int roll = 1500;

    if (rxAreFlightChannelsValid()) {
        if (cfg->yawChannel > 0) {
            yaw = rxGetChannelValue(cfg->yawChannel - 1);
            // const rxChannelRangeConfig_t *channelRanges =
            // rxChannelRangeConfigs(cfg->pitchChannel - 1);
            if (yaw < 1000) {
                yaw = 1000;
            } else if (yaw > 2000) {
                yaw = 2000;
            }
        }

        if (cfg->pitchChannel > 0) {
            pitch = rxGetChannelValue(cfg->pitchChannel - 1);
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

    attittude.sensibility = gimbal_scale8(-16, 15, 0, 31, cfg->sensitivity);

    attittude.yaw = gimbal_scale16(1000, 2000, 0, 4095, yaw);
    attittude.pitch = gimbal_scale16(1000, 2000, 0, 4095, pitch);
    attittude.roll = gimbal_scale16(1000, 2000, 0, 4095, roll);

    uint16_t crc16 = 0;
    uint8_t *b = (uint8_t *)&attittude;
    for (uint8_t i = 0; i < sizeof(GimbalHtkAttitudePkt_t) - 2; i++) {
        crc16 = crc16_ccitt(crc16, *(b + i));
    }
    attittude.crch = (crc16 >> 8) & 0xFF;
    attittude.crcl = crc16 & 0xFF;

    // Send new data
}

uint8_t gimbal_scale8(int8_t inputMin, int8_t inputMax, int8_t outputMin, int8_t outputMax, int8_t value)
{
    float m = (1.0f * outputMax - outputMin) / (inputMax - inputMin);
    return (uint8_t)((outputMin + (m * (value - inputMin))) + 0.5f);
}

uint16_t gimbal_scale16(int16_t inputMin, int16_t inputMax, int16_t outputMin, int16_t outputMax, int16_t value)
{
    float m = (1.0f * outputMax - outputMin) / (inputMax - inputMin);
    return (uint16_t)((outputMin + (m * (value - inputMin))) + 0.5f);
}