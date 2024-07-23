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
#include <string.h>

#include "platform.h"

#ifdef USE_GIMBAL_SERIAL

#include <common/crc.h>
#include <common/utils.h>
#include <common/maths.h>
#include <common/mavlink.h>
#include <build/debug.h>

#include <drivers/gimbal_common.h>
#include <drivers/headtracker_common.h>
#include <drivers/serial.h>
#include <drivers/time.h>

#include <io/gimbal_serial.h>
#include <io/serial.h>

#include <rx/rx.h>
#include <fc/rc_modes.h>

#include <config/parameter_group_ids.h>

#include "settings_generated.h"

gimbalVTable_t gimbalMavlinkVTable = {
    .process = gimbalMavlinkProcess,
    .getDeviceType = gimbalMavlinkGetDeviceType,
    .isReady = gimbalMavlinkIsReady,
    .hasHeadTracker = gimbalMavlinkHasHeadTracker,
};

static gimbalDevice_t mavlinkGimbalDevice = {
    .vTable = &gimbalMavlinkVTable,
    .currentPanPWM = PWM_RANGE_MIDDLE
};

gimbalDevType_e gimbalMavlinkGetDeviceType(const gimbalDevice_t *gimbalDevice)
{
    UNUSED(gimbalDevice);
    return GIMBAL_DEV_MAVLINK;
}

bool gimbalMavlinkIsReady(const gimbalDevice_t *gimbalDevice)
{
    return gimbalPort != NULL && gimbalDevice->vTable != NULL;
}

bool gimbalMavlinkHasHeadTracker(const gimbalDevice_t *gimbalDevice)
{
    return false;
}

bool gimbalMavlinkInit(void)
{
    if (gimbalMavlinkDetect()) {
        SD(fprintf(stderr, "Setting gimbal device\n"));
        gimbalCommonSetDevice(&serialGimbalDevice);
        return true;
    }

    return false;
}

bool gimbalMavlinkDetect(void)
{
    
    SD(fprintf(stderr, "[GIMBAL]: serial Detect...\n"));
    serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_TELEMETRY_MAVLINK);
    SD(fprintf(stderr, "[GIMBAL]: portConfig: %p\n", portConfig));
    SD(fprintf(stderr, "[GIMBAL]: MAVLINK: %s\n", gimbalConfig()->gimbalType == GIMBAL_DEV_MAVLINK ? "yes" : "no"));
    return portConfig && gimbalConfig()->gimbalType == GIMBAL_DEV_MAVLINK;
}

void gimbalMavlinklProcess(gimbalDevice_t *gimbalDevice, timeUs_t currentTime)
{
    UNUSED(currentTime);
    UNUSED(gimbalDevice);

    if (!gimbalMavlinklIsReady(gimbalDevice)) {
        SD(fprintf(stderr, "[GIMBAL] gimbal not ready...\n"));
        return;
    }

    const gimbalConfig_t *cfg = gimbalConfig();

    int panPWM = PWM_RANGE_MIDDLE + cfg->panTrim;
    int tiltPWM = PWM_RANGE_MIDDLE + cfg->tiltTrim;
    int rollPWM = PWM_RANGE_MIDDLE + cfg->rollTrim;

    if (IS_RC_MODE_ACTIVE(BOXGIMBALTLOCK)) {
        //attitude.mode |= GIMBAL_MODE_TILT_LOCK;
    }

    if (IS_RC_MODE_ACTIVE(BOXGIMBALRLOCK)) {
        //attitude.mode |= GIMBAL_MODE_ROLL_LOCK;
    }

    // Follow center overrides all
    if (IS_RC_MODE_ACTIVE(BOXGIMBALCENTER) || IS_RC_MODE_ACTIVE(BOXGIMBALHTRK)) {
        //attitude.mode = GIMBAL_MODE_FOLLOW;
    }
    
    if (rxAreFlightChannelsValid() && !IS_RC_MODE_ACTIVE(BOXGIMBALCENTER)) {
        if (cfg->panChannel > 0) {
            panPWM = rxGetChannelValue(cfg->panChannel - 1) + cfg->panTrim;
            panPWM = constrain(panPWM, PWM_RANGE_MIN, PWM_RANGE_MAX);
        }

        if (cfg->tiltChannel > 0) {
            tiltPWM = rxGetChannelValue(cfg->tiltChannel - 1) + cfg->tiltTrim;
            tiltPWM = constrain(tiltPWM, PWM_RANGE_MIN, PWM_RANGE_MAX);
        }

        if (cfg->rollChannel > 0) {
            rollPWM = rxGetChannelValue(cfg->rollChannel - 1) + cfg->rollTrim;
            rollPWM = constrain(rollPWM, PWM_RANGE_MIN, PWM_RANGE_MAX);
        }
    }

#ifdef USE_HEADTRACKER
    if(IS_RC_MODE_ACTIVE(BOXGIMBALHTRK)) {
        headTrackerDevice_t *dev = headTrackerCommonDevice();
        if (gimbalCommonHtrkIsEnabled() && dev && headTrackerCommonIsValid(dev)) {
            //attitude.pan = headTrackerCommonGetPan(dev);
            //attitude.tilt = headTrackerCommonGetTilt(dev);
            //attitude.roll = headTrackerCommonGetRoll(dev);

            DEBUG_SET(DEBUG_HEADTRACKING, 4, 1);
        } else {
            //attitude.pan = constrain(gimbal_scale12(PWM_RANGE_MIN, PWM_RANGE_MAX, PWM_RANGE_MIDDLE + cfg->panTrim), HEADTRACKER_RANGE_MIN, HEADTRACKER_RANGE_MAX);
            //attitude.tilt = constrain(gimbal_scale12(PWM_RANGE_MIN, PWM_RANGE_MAX, PWM_RANGE_MIDDLE + cfg->tiltTrim), HEADTRACKER_RANGE_MIN, HEADTRACKER_RANGE_MAX);
            //attitude.roll = constrain(gimbal_scale12(PWM_RANGE_MIN, PWM_RANGE_MAX, PWM_RANGE_MIDDLE + cfg->rollTrim), HEADTRACKER_RANGE_MIN, HEADTRACKER_RANGE_MAX);
            DEBUG_SET(DEBUG_HEADTRACKING, 4, -1);
        }
    } else {
#else
    {
#endif
        DEBUG_SET(DEBUG_HEADTRACKING, 4, 0);
        // Radio endpoints may need to be adjusted, as it seems ot go a bit
        // bananas at the extremes
        //attitude.pan = gimbal_scale12(PWM_RANGE_MIN, PWM_RANGE_MAX, panPWM);
        //attitude.tilt = gimbal_scale12(PWM_RANGE_MIN, PWM_RANGE_MAX, tiltPWM);
        //attitude.roll = gimbal_scale12(PWM_RANGE_MIN, PWM_RANGE_MAX, rollPWM);
    }

    //DEBUG_SET(DEBUG_HEADTRACKING, 5, attitude.pan);
    //DEBUG_SET(DEBUG_HEADTRACKING, 6, attitude.tilt);
    //DEBUG_SET(DEBUG_HEADTRACKING, 7, attitude.roll);

    //attitude.sensibility = cfg->sensitivity;

    //serialGimbalDevice.currentPanPWM = gimbal2pwm(attitude.pan);

    //serialBeginWrite(gimbalPort);
    //serialWriteBuf(gimbalPort, (uint8_t *)&attitude, sizeof(gimbalHtkAttitudePkt_t));
    //serialEndWrite(gimbalPort);
}
#endif

int16_t gimbal2pwm(int16_t value)
{
    int16_t ret = 0;
    ret = scaleRange(value, HEADTRACKER_RANGE_MIN, HEADTRACKER_RANGE_MAX, PWM_RANGE_MIN, PWM_RANGE_MAX);
    return ret;
}


int16_t gimbal_scale12(int16_t inputMin, int16_t inputMax, int16_t value)
{
    int16_t ret = 0;
    ret = scaleRange(value, inputMin, inputMax, HEADTRACKER_RANGE_MIN, HEADTRACKER_RANGE_MAX);
    return ret;
}

#ifndef GIMBAL_UNIT_TEST

#if (defined(USE_HEADTRACKER) && defined(USE_HEADTRACKER_SERIAL))
static void resetState(gimbalSerialHtrkState_t *state)
{
    state->state = WAITING_HDR1;
    state->payloadSize = 0;
}

static bool checkCrc(gimbalHtkAttitudePkt_t *attitude)
{
    uint8_t *attitudePkt = (uint8_t *)attitude;
    uint16_t crc = 0;

    for(uint8_t i = 0; i < sizeof(gimbalHtkAttitudePkt_t) - 2; ++i) {
        crc = crc16_ccitt(crc, attitudePkt[i]);
    }

    return (attitude->crch == ((crc >> 8) & 0xFF)) &&
           (attitude->crcl == (crc & 0xFF));
}

void gimbalSerialHeadTrackerReceive(uint16_t c, void *data)
{
    static int charCount = 0;
    static int pktCount = 0;
    static int errorCount = 0;
    gimbalSerialHtrkState_t *state = (gimbalSerialHtrkState_t *)data;
    uint8_t *payload = (uint8_t *)&(state->attitude);
    payload += 2;

    DEBUG_SET(DEBUG_HEADTRACKING, 0, charCount++);
    DEBUG_SET(DEBUG_HEADTRACKING, 1, state->state);

    switch(state->state) {
        case WAITING_HDR1:
            if(c == HTKATTITUDE_SYNC0) {
                state->attitude.sync[0] = c;
                state->state = WAITING_HDR2;
            }
            break;
        case WAITING_HDR2:
            if(c == HTKATTITUDE_SYNC1) {
                state->attitude.sync[1] = c;
                state->state = WAITING_PAYLOAD;
            } else {
                resetState(state);
            }
            break;
        case WAITING_PAYLOAD:
            payload[state->payloadSize++] = c;
            if(state->payloadSize == HEADTRACKER_PAYLOAD_SIZE)
            {
                state->state = WAITING_CRCH;
            }
            break;
        case WAITING_CRCH:
            state->attitude.crch = c;
            state->state = WAITING_CRCL;
            break;
        case WAITING_CRCL:
            state->attitude.crcl = c;
            if(checkCrc(&(state->attitude))) {
                headTrackerDevice.expires = micros() + MAX_HEADTRACKER_DATA_AGE_US;
                headTrackerDevice.pan = constrain(state->attitude.pan, HEADTRACKER_RANGE_MIN, HEADTRACKER_RANGE_MAX);
                headTrackerDevice.tilt = constrain(state->attitude.tilt, HEADTRACKER_RANGE_MIN, HEADTRACKER_RANGE_MAX);
                headTrackerDevice.roll = constrain(state->attitude.roll, HEADTRACKER_RANGE_MIN, HEADTRACKER_RANGE_MAX);
                DEBUG_SET(DEBUG_HEADTRACKING, 2, pktCount++);
            } else {
                DEBUG_SET(DEBUG_HEADTRACKING, 3, errorCount++);
            }
            resetState(state);
            break;
    }
}


bool gimbalSerialHeadTrackerDetect(void)
{
    bool singleUart = gimbalSerialConfig()->singleUart;

    SD(fprintf(stderr, "[GIMBAL_HTRK]: headtracker Detect...\n"));
    serialPortConfig_t *portConfig = singleUart ? NULL : findSerialPortConfig(FUNCTION_GIMBAL_HEADTRACKER);

    if (portConfig) {
        SD(fprintf(stderr, "[GIMBAL_HTRK]: found port...\n"));
        headTrackerPort = openSerialPort(portConfig->identifier, FUNCTION_GIMBAL_HEADTRACKER, gimbalSerialHeadTrackerReceive, &headTrackerState,
                baudRates[portConfig->peripheral_baudrateIndex], MODE_RXTX, SERIAL_NOT_INVERTED);

        if (headTrackerPort) {
            SD(fprintf(stderr, "[GIMBAL_HTRK]: port open!\n"));
            headTrackerPort->txBuffer = txBuffer;
            headTrackerPort->txBufferSize = GIMBAL_SERIAL_BUFFER_SIZE;
            headTrackerPort->txBufferTail = 0;
            headTrackerPort->txBufferHead = 0;
        } else {
            SD(fprintf(stderr, "[GIMBAL_HTRK]: port NOT open!\n"));
            return false;
        }
    }

    SD(fprintf(stderr, "[GIMBAL]: gimbalPort: %p headTrackerPort: %p\n", gimbalPort, headTrackerPort));
    return (singleUart && gimbalPort) || headTrackerPort;
}

bool gimbalSerialHeadTrackerInit(void)
{
    if(headTrackerConfig()->devType == HEADTRACKER_SERIAL) {
        if (gimbalSerialHeadTrackerDetect()) {
            SD(fprintf(stderr, "Setting gimbal device\n"));
            headTrackerCommonSetDevice(&headTrackerDevice);

            return true;
        }
    }

    return false;
}

void headtrackerSerialProcess(headTrackerDevice_t *headTrackerDevice, timeUs_t currentTimeUs)
{
    UNUSED(headTrackerDevice);
    UNUSED(currentTimeUs);
    return;
}

headTrackerDevType_e headtrackerSerialGetDeviceType(const headTrackerDevice_t *headTrackerDevice)
{
    UNUSED(headTrackerDevice);
    return HEADTRACKER_SERIAL;
}

#else

void gimbalSerialHeadTrackerReceive(uint16_t c, void *data)
{
    UNUSED(c);
    UNUSED(data);
}

#endif

#endif

#endif