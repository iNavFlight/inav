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
#ifdef USE_SERIALRX_MAVLINK

#include "common/utils.h"
#include "rx/rx.h"
#include "rx/mavlink.h"

#define MAVLINK_CHANNEL_COUNT 18

typedef struct mavlinkRxState_s {
    uint16_t channelData[MAVLINK_CHANNEL_COUNT];
    bool frameReceived;
} mavlinkRxState_t;

static mavlinkRxState_t mavlinkRxStates[RX_LINK_COUNT];

int8_t mavlinkRxLinkForPortFunctionMask(uint32_t functionMask)
{
    const bool rx1 = (functionMask & FUNCTION_RX_SERIAL) &&
        rxConfig()->receiverType == RX_TYPE_SERIAL &&
        rxConfig()->serialrx_provider == SERIALRX_MAVLINK;
    const bool rx2 = (functionMask & FUNCTION_RX_SERIAL_SECONDARY) &&
        rxIsDualRxEnabled() &&
        rxConfig()->receiverTypeSecondary == RX_TYPE_SERIAL &&
        rxConfig()->serialrx_provider_secondary == SERIALRX_MAVLINK;

    if (rx1 == rx2) {
        return -1;
    }
    return rx2 ? RX_LINK_SECONDARY : RX_LINK_PRIMARY;
}

void mavlinkRxHandleMessage(rxLink_e link, const mavlink_rc_channels_override_t *msg)
{
    if ((unsigned)link >= RX_LINK_COUNT) {
        return;
    }

    mavlinkRxState_t *state = &mavlinkRxStates[link];
    uint8_t flightChannelMask = 0;
#define SET_MAV_CH(_n, _field, _extended) do { \
    const uint16_t value = msg->_field; \
    if (value != 0 && ((_extended) ? value < UINT16_MAX - 1 : value != UINT16_MAX)) { \
        state->channelData[_n] = value; \
        if ((_n) < NON_AUX_CHANNEL_COUNT) { \
            flightChannelMask |= 1U << (_n); \
        } \
    } \
} while (0)
    SET_MAV_CH(0, chan1_raw, false);
    SET_MAV_CH(1, chan2_raw, false);
    SET_MAV_CH(2, chan3_raw, false);
    SET_MAV_CH(3, chan4_raw, false);
    SET_MAV_CH(4, chan5_raw, false);
    SET_MAV_CH(5, chan6_raw, false);
    SET_MAV_CH(6, chan7_raw, false);
    SET_MAV_CH(7, chan8_raw, false);
    SET_MAV_CH(8, chan9_raw, true);
    SET_MAV_CH(9, chan10_raw, true);
    SET_MAV_CH(10, chan11_raw, true);
    SET_MAV_CH(11, chan12_raw, true);
    SET_MAV_CH(12, chan13_raw, true);
    SET_MAV_CH(13, chan14_raw, true);
    SET_MAV_CH(14, chan15_raw, true);
    SET_MAV_CH(15, chan16_raw, true);
    SET_MAV_CH(16, chan17_raw, true);
    SET_MAV_CH(17, chan18_raw, true);
#undef SET_MAV_CH

    // MAVLink is being used here as an RC transport, not merely as a sparse
    // override command. A message can refresh RX liveness only when it carries
    // a complete set of flight-control channels. This prevents AUX-only or
    // empty override traffic from keeping stale controls alive indefinitely.
    if (flightChannelMask == ((1U << NON_AUX_CHANNEL_COUNT) - 1U)) {
        state->frameReceived = true;
    }
}

static uint8_t mavlinkFrameStatus(rxRuntimeConfig_t *rxRuntimeConfig)
{
    mavlinkRxState_t *state = (mavlinkRxState_t *)rxRuntimeConfig->frameData;
    if (!state->frameReceived) {
        return RX_FRAME_PENDING;
    }
    state->frameReceived = false;
    return RX_FRAME_COMPLETE;
}

static uint16_t mavlinkReadRawRC(const rxRuntimeConfig_t *rxRuntimeConfig, uint8_t channel)
{
    const mavlinkRxState_t *state = (const mavlinkRxState_t *)rxRuntimeConfig->frameData;
    return state->channelData[channel];
}

bool mavlinkRxInit(const rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig, serialPortFunction_e portFunction)
{
    UNUSED(rxConfig);

    // MAVLink RX is carried by the MAVLink telemetry runtime rather than by a
    // dedicated serial RX parser. The selected UART therefore has to carry both
    // its RX role and TELEMETRY_MAVLINK or there is no ingress path at all.
    const serialPortConfig_t *portConfig = findSerialPortConfig(portFunction);
    if (!portConfig || !(portConfig->functionMask & FUNCTION_TELEMETRY_MAVLINK)) {
        return false;
    }

    const rxLink_e link = portFunction == FUNCTION_RX_SERIAL_SECONDARY ? RX_LINK_SECONDARY : RX_LINK_PRIMARY;
    mavlinkRxState_t *state = &mavlinkRxStates[link];
    memset(state, 0, sizeof(*state));

    // Zero means no usable channel sample yet. RC_CHANNELS_OVERRIDE may omit
    // individual fields, and an omitted flight channel must not become valid
    // merely because the receiver state was initialized to a plausible PWM.
    rxRuntimeConfig->channelData = state->channelData;
    rxRuntimeConfig->channelCount = MAVLINK_CHANNEL_COUNT;
    rxRuntimeConfig->rcReadRawFn = mavlinkReadRawRC;
    rxRuntimeConfig->rcFrameStatusFn = mavlinkFrameStatus;
    rxRuntimeConfig->frameData = state;
    return true;
}

#endif
