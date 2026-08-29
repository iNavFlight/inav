/*
 * This file is part of Cleanflight.
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
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#ifdef USE_RX_MSP

#include "build/build_config.h"
#include "common/maths.h"
#include "common/utils.h"
#include "rx/rx.h"
#include "rx/msp.h"

typedef struct mspRxState_s {
    uint16_t frame[MAX_SUPPORTED_RC_CHANNEL_COUNT];
    bool frameDone;
    bool requireFlightChannels;
    uint8_t lastChannelCount;
} mspRxState_t;

static mspRxState_t mspRxStates[RX_LINK_COUNT];
static mspRxState_t mspOverrideState;

static uint16_t rxMspReadRawRC(const rxRuntimeConfig_t *rxRuntimeConfig, uint8_t chan)
{
    const mspRxState_t *state = (const mspRxState_t *)rxRuntimeConfig->frameData;
    return state->frame[chan];
}

static uint8_t rxMspFrameStatus(rxRuntimeConfig_t *rxRuntimeConfig)
{
    mspRxState_t *state = (mspRxState_t *)rxRuntimeConfig->frameData;
    if (!state->frameDone) {
        return RX_FRAME_PENDING;
    }
    state->frameDone = false;
    if (state->requireFlightChannels && state->lastChannelCount < NON_AUX_CHANNEL_COUNT) {
        return RX_FRAME_COMPLETE | RX_FRAME_FAILSAFE;
    }
    return RX_FRAME_COMPLETE;
}

static void rxMspInitState(rxRuntimeConfig_t *rxRuntimeConfig, mspRxState_t *state, bool requireFlightChannels)
{
    memset(state, 0, sizeof(*state));
    state->requireFlightChannels = requireFlightChannels;
    rxRuntimeConfig->channelCount = MAX_SUPPORTED_RC_CHANNEL_COUNT;
    rxRuntimeConfig->rxSignalTimeout = DELAY_5_HZ;
    rxRuntimeConfig->rcReadRawFn = rxMspReadRawRC;
    rxRuntimeConfig->rcFrameStatusFn = rxMspFrameStatus;
    rxRuntimeConfig->frameData = state;
}

void rxMspFrameReceive(uint16_t *frame, int channelCount)
{
    const int8_t mspLink = rxGetMspLink();
    mspRxState_t *state = mspLink >= 0 && mspLink < RX_LINK_COUNT ? &mspRxStates[mspLink] : &mspOverrideState;

    channelCount = constrain(channelCount, 0, MAX_SUPPORTED_RC_CHANNEL_COUNT);
    for (int i = 0; i < channelCount; i++) {
        state->frame[i] = frame[i];
    }
    for (int i = channelCount; i < MAX_SUPPORTED_RC_CHANNEL_COUNT; i++) {
        state->frame[i] = 0;
    }
    state->lastChannelCount = channelCount;
    state->frameDone = true;
}

uint8_t rxMspGetLastChannelCount(rxLink_e link)
{
    return (unsigned)link < RX_LINK_COUNT ? mspRxStates[link].lastChannelCount : 0;
}

void rxMspInit(const rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig, rxLink_e link)
{
    UNUSED(rxConfig);
    if ((unsigned)link < RX_LINK_COUNT) {
        rxMspInitState(rxRuntimeConfig, &mspRxStates[link], true);
    }
}

void rxMspOverrideInit(const rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig)
{
    UNUSED(rxConfig);
    rxMspInitState(rxRuntimeConfig, &mspOverrideState, false);
}

#endif
