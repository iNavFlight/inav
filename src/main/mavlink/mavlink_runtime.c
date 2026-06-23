#include "mavlink/mavlink_internal.h"

#include "fc/fc_mavlink.h"

#include "mavlink/mavlink_mission.h"
#include "mavlink/mavlink_ports.h"
#include "mavlink/mavlink_routing.h"
#include "mavlink/mavlink_runtime.h"
#include "mavlink/mavlink_streams.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_MAVLINK)

mavlinkContext_t mavlinkContext;

const mavlinkTelemetryPortConfig_t *mavlinkGetPortConfig(uint8_t portIndex)
{
    return &telemetryConfig()->mavlink[portIndex];
}

const mavlinkTelemetryCommonConfig_t *mavlinkGetCommonConfig(void)
{
    return &telemetryConfig()->mavlink_common;
}

uint8_t mavlinkGetProtocolVersion(void)
{
    return mavlinkGetCommonConfig()->version;
}

const mavlinkMlrsPortRuntime_t *mavlinkGetPortMlrsRuntime(uint8_t portIndex)
{
    if (portIndex >= mavPortCount) {
        return NULL;
    }

    return &mavPortStates[portIndex].mlrs;
}

const mavlinkMlrsPortRuntime_t *mavlinkGetActiveMlrsRuntime(void)
{
    if (!mavActivePort) {
        return NULL;
    }

    return &mavActivePort->mlrs;
}

bool mavlinkPortTxBufferIsValid(uint8_t portIndex)
{
    return portIndex < mavPortCount && mavPortStates[portIndex].txbuffValid;
}

uint8_t mavlinkPortTxBufferFree(uint8_t portIndex)
{
    return portIndex < mavPortCount ? mavPortStates[portIndex].txbuffFree : 100;
}

static void mavlinkApplyActivePortOutputVersion(void)
{
    mavlink_status_t *chanState = mavlink_get_channel_status(MAVLINK_COMM_0);
    if (mavlinkGetProtocolVersion() == 1) {
        chanState->flags |= MAVLINK_STATUS_FLAG_OUT_MAVLINK1;
    } else {
        chanState->flags &= ~MAVLINK_STATUS_FLAG_OUT_MAVLINK1;
    }
}

void mavlinkSetActivePortContext(uint8_t portIndex)
{
    mavActivePort = &mavPortStates[portIndex];
    mavActiveConfig = mavlinkGetPortConfig(portIndex);
    const mavlinkTelemetryCommonConfig_t *commonConfig = mavlinkGetCommonConfig();
    mavAutopilotType = commonConfig->autopilot_type;
    mavSystemId = commonConfig->sysid;
    mavComponentId = MAV_COMP_ID_AUTOPILOT1;
    mavlinkApplyActivePortOutputVersion();
}

void mavlinkRuntimeFreePorts(void)
{
    for (uint8_t portIndex = 0; portIndex < mavPortCount; portIndex++) {
        freeMAVLinkTelemetryPortByIndex(portIndex);
    }

    mavSendMask = 0;
    mavRouteCount = 0;
}

void mavlinkRuntimeInit(void)
{
    memset(&mavlinkContext, 0, sizeof(mavlinkContext));
    mavSystemId = 1;
    mavComponentId = MAV_COMP_ID_AUTOPILOT1;

    const serialPortConfig_t *serialPortConfig = findSerialPortConfig(FUNCTION_TELEMETRY_MAVLINK);
    while (serialPortConfig && mavPortCount < MAX_MAVLINK_PORTS) {
        mavlinkPortRuntime_t *state = &mavPortStates[mavPortCount];
        state->portConfig = serialPortConfig;
        state->portSharing = determinePortSharing(serialPortConfig, FUNCTION_TELEMETRY_MAVLINK);
        state->txbuffFree = 100;
        state->highLatencyEnabled = mavlinkGetPortConfig(mavPortCount)->high_latency;
        configureMAVLinkStreamRates(mavPortCount);

        mavPortCount++;
        serialPortConfig = findNextSerialPortConfig(FUNCTION_TELEMETRY_MAVLINK);
    }
}

void mavlinkRuntimeCheckState(void)
{
    for (uint8_t portIndex = 0; portIndex < mavPortCount; portIndex++) {
        mavlinkPortRuntime_t *state = &mavPortStates[portIndex];

        bool newTelemetryEnabledValue = telemetryDetermineEnabledState(state->portSharing);
        if ((state->portConfig->functionMask & FUNCTION_RX_SERIAL) &&
            rxConfig()->receiverType == RX_TYPE_SERIAL &&
            rxConfig()->serialrx_provider == SERIALRX_MAVLINK) {
            newTelemetryEnabledValue = true;
        }

        if (newTelemetryEnabledValue == state->telemetryEnabled) {
            continue;
        }

        if (newTelemetryEnabledValue) {
            configureMAVLinkTelemetryPort(portIndex);
            if (state->telemetryEnabled) {
                configureMAVLinkStreamRates(portIndex);
            }
        } else {
            freeMAVLinkTelemetryPortByIndex(portIndex);
        }
    }
}

void mavlinkSendMessage(void)
{
    const mavlink_msg_entry_t *msgEntry = mavlink_get_msg_entry(mavSendMsg.msgid);
    if (!msgEntry) {
        return;
    }

    uint8_t sendMask = mavSendMask;
    if (sendMask == 0) {
        if (mavActivePort) {
            for (uint8_t portIndex = 0; portIndex < mavPortCount; portIndex++) {
                if (&mavPortStates[portIndex] == mavActivePort) {
                    sendMask = MAVLINK_PORT_MASK(portIndex);
                    break;
                }
            }
        } else if (mavPortCount == 1 && mavPortStates[0].telemetryEnabled && mavPortStates[0].port) {
            sendMask = MAVLINK_PORT_MASK(0);
        }
    }

    for (uint8_t portIndex = 0; portIndex < mavPortCount; portIndex++) {
        if ((sendMask & MAVLINK_PORT_MASK(portIndex)) == 0) {
            continue;
        }

        mavlinkPortRuntime_t *state = &mavPortStates[portIndex];
        if (!state->telemetryEnabled || !state->port) {
            continue;
        }

        mavlink_status_t txStatus = { 0 };
        txStatus.current_tx_seq = state->txSeq;
        if (mavlinkGetProtocolVersion() == 1) {
            txStatus.flags |= MAVLINK_STATUS_FLAG_OUT_MAVLINK1;
        }

        mavlink_message_t txMsg = mavSendMsg;
        mavlink_finalize_message_buffer(
            &txMsg,
            txMsg.sysid,
            txMsg.compid,
            &txStatus,
            msgEntry->min_msg_len,
            txMsg.len,
            msgEntry->crc_extra
        );
        state->txSeq = txStatus.current_tx_seq;

        uint8_t mavBuffer[MAVLINK_MAX_PACKET_LEN];
        const int msgLength = mavlink_msg_to_send_buffer(mavBuffer, &txMsg);
        if (msgLength <= 0) {
            continue;
        }

        // Drop the frame on this port if there is no room; do not block telemetry task.
        if (serialTxBytesFree(state->port) < (uint32_t)msgLength) {
            state->txDroppedFrames++;
            continue;
        }

        serialBeginWrite(state->port);
        serialWriteBuf(state->port, mavBuffer, msgLength);
        serialEndWrite(state->port);
    }
}

static bool processMAVLinkIncomingTelemetry(uint8_t ingressPortIndex, timeUs_t currentTimeUs)
{
    mavlinkPortRuntime_t *state = &mavPortStates[ingressPortIndex];

    while (serialRxBytesWaiting(state->port) > 0) {
        // Limit handling to one message per cycle
        const char c = serialRead(state->port);
        const uint8_t result = mavlink_parse_char(ingressPortIndex, c, &state->mavRecvMsg, &state->mavRecvStatus);
        if (result == MAVLINK_FRAMING_OK) {
            state->lastRxFrameUs = currentTimeUs;
            mavlinkContext.recvMsg = state->mavRecvMsg;

            if (mavlinkIsFromLocalIdentity(mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid)) {
                return false;
            }

            mavlinkLearnRoute(ingressPortIndex);

            int16_t targetSystem;
            int16_t targetComponent;
            mavlinkExtractTargets(&mavlinkContext.recvMsg, &targetSystem, &targetComponent);
            mavlinkForwardMessage(ingressPortIndex, targetSystem, targetComponent);

            if (mavlinkContext.recvMsg.msgid == MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE) {
                mavlinkSetActivePortContext(ingressPortIndex);
                mavSendMask = MAVLINK_PORT_MASK(ingressPortIndex);
                const mavlinkFcDispatchResult_e dispatchResult = mavlinkFcDispatchIncomingMessage(ingressPortIndex);
                return dispatchResult == MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY;
            }

            const int8_t localPortIndex = mavlinkResolveLocalPortForTarget(targetSystem, targetComponent, ingressPortIndex);
            if (localPortIndex < 0) {
                return false;
            }

            uint8_t localPortMask = MAVLINK_PORT_MASK((uint8_t)localPortIndex);
            const bool isLocalOrSystemBroadcast = targetSystem == 0 || ((targetSystem > 0) && ((uint8_t)targetSystem == mavlinkGetCommonConfig()->sysid));
            if (targetComponent == 0 && isLocalOrSystemBroadcast && mavlinkShouldFanOutLocalBroadcast(&mavlinkContext.recvMsg)) {
                localPortMask = 0;
                for (uint8_t portIndex = 0; portIndex < mavPortCount; portIndex++) {
                    localPortMask |= MAVLINK_PORT_MASK(portIndex);
                }
            }

            bool handled = false;
            for (uint8_t localIndex = 0; localIndex < mavPortCount; localIndex++) {
                if (!(localPortMask & MAVLINK_PORT_MASK(localIndex))) {
                    continue;
                }

                mavlinkSetActivePortContext(localIndex);
                mavSendMask = MAVLINK_PORT_MASK(ingressPortIndex);

                const mavlinkFcDispatchResult_e dispatchResult = mavlinkFcDispatchIncomingMessage(ingressPortIndex);
                handled = handled || dispatchResult == MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY;
            }

            return handled;
        }
    }

    return false;
}

static bool isMAVLinkTelemetryHalfDuplex(uint8_t portIndex)
{
    const mavlinkPortRuntime_t *state = &mavPortStates[portIndex];

    return state->portConfig &&
        (state->portConfig->functionMask & FUNCTION_RX_SERIAL) &&
        rxConfig()->receiverType == RX_TYPE_SERIAL &&
        rxConfig()->serialrx_provider == SERIALRX_MAVLINK &&
        tristateWithDefaultOffIsActive(rxConfig()->halfDuplex);
}

void mavlinkRuntimeHandle(timeUs_t currentTimeUs)
{
    mavlinkSendPendingMissionItemReached();

    for (uint8_t portIndex = 0; portIndex < mavPortCount; portIndex++) {
        mavlinkPortRuntime_t *state = &mavPortStates[portIndex];
        if (!state->telemetryEnabled || !state->port) {
            continue;
        }

        mavlinkSetActivePortContext(portIndex);

        // Process incoming MAVLink on this port and forward when needed.
        processMAVLinkIncomingTelemetry(portIndex, currentTimeUs);

        // Restore context back to this port before periodic send decisions.
        mavlinkSetActivePortContext(portIndex);
        bool shouldSendTelemetry = false;
        const bool halfDuplexBackoff = isMAVLinkTelemetryHalfDuplex(portIndex) &&
            ((currentTimeUs - state->lastRxFrameUs) < TELEMETRY_MAVLINK_DELAY);

        if (halfDuplexBackoff) {
            continue;
        }

        if (state->txbuffValid) {
            // Use flow control if available.
            shouldSendTelemetry = state->txbuffFree >= mavActiveConfig->min_txbuff;
        } else {
            // If not, use blind frame pacing.
            shouldSendTelemetry = (currentTimeUs - state->lastMavlinkMessageUs) >= TELEMETRY_MAVLINK_DELAY;
        }

        if (!shouldSendTelemetry) {
            continue;
        }

        mavSendMask = MAVLINK_PORT_MASK(portIndex);
        processMAVLinkTelemetry(currentTimeUs);
        state->lastMavlinkMessageUs = currentTimeUs;
    }

    mavSendMask = 0;
}

#endif
