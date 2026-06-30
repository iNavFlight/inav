#include "mavlink/mavlink_internal.h"

#include "mavlink/mavlink_routing.h"
#include "mavlink/mavlink_runtime.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_MAVLINK)

bool mavlinkIsFromLocalIdentity(uint8_t sysid, uint8_t compid)
{
    return sysid == mavlinkGetCommonConfig()->sysid && compid == MAV_COMP_ID_AUTOPILOT1;
}

void mavlinkLearnRoute(uint8_t ingressPortIndex)
{
    if (mavlinkContext.recvMsg.sysid == 0 || mavlinkIsFromLocalIdentity(mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid)) {
        return;
    }

    for (uint8_t routeIndex = 0; routeIndex < mavRouteCount; routeIndex++) {
        mavlinkRouteEntry_t *route = &mavRouteTable[routeIndex];
        if (route->sysid == mavlinkContext.recvMsg.sysid && route->compid == mavlinkContext.recvMsg.compid) {
            route->ingressPortIndex = ingressPortIndex;
            return;
        }
    }

    if (mavRouteCount >= MAVLINK_MAX_ROUTES) {
        return;
    }

    mavRouteTable[mavRouteCount].sysid = mavlinkContext.recvMsg.sysid;
    mavRouteTable[mavRouteCount].compid = mavlinkContext.recvMsg.compid;
    mavRouteTable[mavRouteCount].ingressPortIndex = ingressPortIndex;
    mavRouteCount++;
}

void mavlinkExtractTargets(const mavlink_message_t *msg, int16_t *targetSystem, int16_t *targetComponent)
{
    *targetSystem = -1;
    *targetComponent = -1;

    const mavlink_msg_entry_t *msgEntry = mavlink_get_msg_entry(msg->msgid);
    if (!msgEntry) {
        return;
    }

    if (msgEntry->flags & MAV_MSG_ENTRY_FLAG_HAVE_TARGET_SYSTEM) {
        *targetSystem = _MAV_RETURN_uint8_t(msg, msgEntry->target_system_ofs);
    }

    if (msgEntry->flags & MAV_MSG_ENTRY_FLAG_HAVE_TARGET_COMPONENT) {
        *targetComponent = _MAV_RETURN_uint8_t(msg, msgEntry->target_component_ofs);
    }
}

static uint8_t mavlinkComputeForwardMask(uint8_t ingressPortIndex, int16_t targetSystem, int16_t targetComponent)
{
    uint8_t mask = 0;

    if (targetSystem <= 0) {
        for (uint8_t portIndex = 0; portIndex < mavPortCount; portIndex++) {
            if (portIndex == ingressPortIndex) {
                continue;
            }

            const mavlinkPortRuntime_t *state = &mavPortStates[portIndex];
            if (!state->telemetryEnabled || !state->port) {
                continue;
            }

            mask |= MAVLINK_PORT_MASK(portIndex);
        }
        return mask;
    }

    for (uint8_t routeIndex = 0; routeIndex < mavRouteCount; routeIndex++) {
        const mavlinkRouteEntry_t *route = &mavRouteTable[routeIndex];

        if (route->sysid != targetSystem) {
            continue;
        }
        if (targetComponent > 0 && route->compid != targetComponent) {
            continue;
        }
        if (route->ingressPortIndex == ingressPortIndex || route->ingressPortIndex >= mavPortCount) {
            continue;
        }

        const mavlinkPortRuntime_t *state = &mavPortStates[route->ingressPortIndex];
        if (!state->telemetryEnabled || !state->port) {
            continue;
        }

        mask |= MAVLINK_PORT_MASK(route->ingressPortIndex);
    }

    return mask;
}

void mavlinkForwardMessage(uint8_t ingressPortIndex, int16_t targetSystem, int16_t targetComponent)
{
    if (mavlinkContext.recvMsg.msgid == MAVLINK_MSG_ID_RADIO_STATUS ||
        mavlinkContext.recvMsg.msgid == MAVLINK_MSG_ID_MLRS_RADIO_LINK_FLOW_CONTROL) {
        return;
    }

    uint8_t mavBuffer[MAVLINK_MAX_PACKET_LEN];
    const int msgLength = mavlink_msg_to_send_buffer(mavBuffer, &mavlinkContext.recvMsg);
    if (msgLength <= 0) {
        return;
    }

    const uint8_t forwardMask = mavlinkComputeForwardMask(ingressPortIndex, targetSystem, targetComponent);
    for (uint8_t portIndex = 0; portIndex < mavPortCount; portIndex++) {
        if ((forwardMask & MAVLINK_PORT_MASK(portIndex)) == 0) {
            continue;
        }

        mavlinkPortRuntime_t *state = &mavPortStates[portIndex];
        if (serialTxBytesFree(state->port) < (uint32_t)msgLength) {
            state->txDroppedFrames++;
            continue;
        }

        serialBeginWrite(state->port);
        serialWriteBuf(state->port, mavBuffer, msgLength);
        serialEndWrite(state->port);
    }
}

int8_t mavlinkResolveLocalPortForTarget(int16_t targetSystem, int16_t targetComponent, uint8_t ingressPortIndex)
{
    if (targetSystem <= 0) {
        return ingressPortIndex;
    }

    if ((uint8_t)targetSystem != mavlinkGetCommonConfig()->sysid) {
        return -1;
    }

    if (targetComponent > 0 && (uint8_t)targetComponent != MAV_COMP_ID_AUTOPILOT1) {
        return -1;
    }

    if (ingressPortIndex < mavPortCount) {
        return ingressPortIndex;
    }

    return mavPortCount > 0 ? 0 : -1;
}

bool mavlinkShouldFanOutLocalBroadcast(const mavlink_message_t *msg)
{
    if (msg->msgid == MAVLINK_MSG_ID_REQUEST_DATA_STREAM) {
        return true;
    }

    if (msg->msgid == MAVLINK_MSG_ID_COMMAND_LONG) {
        mavlink_command_long_t cmd;
        mavlink_msg_command_long_decode(msg, &cmd);
        return cmd.command == MAV_CMD_SET_MESSAGE_INTERVAL || cmd.command == MAV_CMD_CONTROL_HIGH_LATENCY;
    }

    if (msg->msgid == MAVLINK_MSG_ID_COMMAND_INT) {
        mavlink_command_int_t cmd;
        mavlink_msg_command_int_decode(msg, &cmd);
        return cmd.command == MAV_CMD_SET_MESSAGE_INTERVAL || cmd.command == MAV_CMD_CONTROL_HIGH_LATENCY;
    }

    return false;
}

#endif
