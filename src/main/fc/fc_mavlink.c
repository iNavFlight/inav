#include "mavlink/mavlink_internal.h"

#include "fc/fc_mavlink.h"

#include "mavlink/mavlink_command.h"
#include "mavlink/mavlink_guided.h"
#include "mavlink/mavlink_mission.h"
#include "mavlink/mavlink_runtime.h"
#include "mavlink/mavlink_streams.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_MAVLINK)

static void mavlinkResetTunnelState(uint8_t ingressPortIndex)
{
    resetMspPort(&mavTunnelMspPorts[ingressPortIndex], NULL);
    mavTunnelRemoteSystemIds[ingressPortIndex] = 0;
    mavTunnelRemoteComponentIds[ingressPortIndex] = 0;
}

static void mavlinkSendTunnelReply(uint8_t targetSystem, uint8_t targetComponent, const uint8_t *payload, uint8_t payloadLength)
{
    uint8_t tunnelPayload[MAVLINK_MSG_TUNNEL_FIELD_PAYLOAD_LEN] = { 0 };
    memcpy(tunnelPayload, payload, payloadLength);

    mavlink_msg_tunnel_pack(
        mavSystemId,
        mavComponentId,
        &mavSendMsg,
        targetSystem,
        targetComponent,
        MAVLINK_TUNNEL_PAYLOAD_TYPE_INAV_MSP,
        payloadLength,
        tunnelPayload);
    mavlinkSendMessage();
}

static void mavlinkSendTunnelMspReply(uint8_t targetSystem, uint8_t targetComponent, mspPacket_t *reply, uint8_t *replyPayloadHead, mspVersion_e mspVersion)
{
    sbufSwitchToReader(&reply->buf, replyPayloadHead);

    const int frameLength = mspSerialEncodePacket(reply, mspVersion, mavTunnelFrameBuf, sizeof(mavTunnelFrameBuf));
    for (int offset = 0; offset < frameLength; offset += MAVLINK_MSG_TUNNEL_FIELD_PAYLOAD_LEN) {
        const uint8_t chunkLength = MIN(MAVLINK_MSG_TUNNEL_FIELD_PAYLOAD_LEN, frameLength - offset);
        mavlinkSendTunnelReply(
            targetSystem,
            targetComponent,
            mavTunnelFrameBuf + offset,
            chunkLength);
    }
}

static bool handleIncoming_TUNNEL(uint8_t ingressPortIndex)
{
    if (mavlinkGetProtocolVersion() == 1) {
        return false;
    }

    mavlink_tunnel_t msg;
    mavlink_msg_tunnel_decode(&mavlinkContext.recvMsg, &msg);

    if (msg.payload_type != MAVLINK_TUNNEL_PAYLOAD_TYPE_INAV_MSP) {
        return false;
    }

    if (msg.target_system != mavSystemId) {
        return false;
    }

    if (msg.target_component != 0 && msg.target_component != mavComponentId) {
        return false;
    }

    mspPort_t *mspPort = &mavTunnelMspPorts[ingressPortIndex];
    const timeMs_t now = millis();
    if (msg.payload_length > MAVLINK_MSG_TUNNEL_FIELD_PAYLOAD_LEN) {
        mavlinkResetTunnelState(ingressPortIndex);
        return false;
    }

    if (mspPort->c_state != MSP_IDLE &&
        ((now - mspPort->lastActivityMs) > MAVLINK_TUNNEL_MSP_TIMEOUT_MS ||
         mavTunnelRemoteSystemIds[ingressPortIndex] != mavlinkContext.recvMsg.sysid ||
         mavTunnelRemoteComponentIds[ingressPortIndex] != mavlinkContext.recvMsg.compid)) {
        mavlinkResetTunnelState(ingressPortIndex);
    }

    mavTunnelRemoteSystemIds[ingressPortIndex] = mavlinkContext.recvMsg.sysid;
    mavTunnelRemoteComponentIds[ingressPortIndex] = mavlinkContext.recvMsg.compid;
    mspPort->lastActivityMs = now;

    bool handled = false;
    for (uint8_t payloadIndex = 0; payloadIndex < msg.payload_length; payloadIndex++) {
        if (!mspSerialProcessReceivedByte(mspPort, msg.payload[payloadIndex])) {
            continue;
        }

        handled = true;
        if (mspPort->c_state != MSP_COMMAND_RECEIVED) {
            continue;
        }

        mspPacket_t reply = {
            .buf = { .ptr = mavTunnelReplyPayloadBuf, .end = ARRAYEND(mavTunnelReplyPayloadBuf), },
            .cmd = -1,
            .flags = 0,
            .result = 0,
        };
        uint8_t *replyPayloadHead = reply.buf.ptr;

        if (mspPort->cmdMSP == MSP_SET_PASSTHROUGH) {
            reply.cmd = MSP_SET_PASSTHROUGH;
            reply.result = MSP_RESULT_ERROR;
            mavlinkSendTunnelMspReply(mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, &reply, replyPayloadHead, mspPort->mspVersion);
            mavlinkResetTunnelState(ingressPortIndex);
            break;
        }

        mspPostProcessFnPtr mspPostProcessFn = NULL;
        const uint16_t command = mspPort->cmdMSP;
        mspResult_e status = mspSerialProcessCommand(mspPort, mspFcProcessCommand, &reply, &mspPostProcessFn);

        if (mspPostProcessFn && command != MSP_REBOOT) {
            sbufInit(&reply.buf, mavTunnelReplyPayloadBuf, ARRAYEND(mavTunnelReplyPayloadBuf));
            reply.result = MSP_RESULT_ERROR;
            mspPostProcessFn = NULL;
            status = MSP_RESULT_ERROR;
        }

        if (status != MSP_RESULT_NO_REPLY) {
            mavlinkSendTunnelMspReply(mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, &reply, replyPayloadHead, mspPort->mspVersion);
        }

        mavlinkResetTunnelState(ingressPortIndex);

        if (mspPostProcessFn) {
            waitForSerialPortToFinishTransmitting(mavPortStates[ingressPortIndex].port);
            mspPostProcessFn(mavPortStates[ingressPortIndex].port);
        }

        break;
    }

    return handled;
}

static bool handleIncoming_RC_CHANNELS_OVERRIDE(void) {
    mavlink_rc_channels_override_t msg;
    mavlink_msg_rc_channels_override_decode(&mavlinkContext.recvMsg, &msg);
    mavlinkRxHandleMessage(&msg);
    return true;
}

static bool handleIncoming_PARAM_REQUEST_LIST(void) {
    mavlink_param_request_list_t msg;
    mavlink_msg_param_request_list_decode(&mavlinkContext.recvMsg, &msg);

    if (msg.target_system != 0 && msg.target_system != mavSystemId) {
        return false;
    }
    if (msg.target_component != 0 && msg.target_component != mavComponentId) {
        return false;
    }

    mavlink_msg_param_value_pack(mavSystemId, mavComponentId, &mavSendMsg, 0, 0, 0, 0, 0);
    mavlinkSendMessage();
    return true;
}

static void mavlinkParseRxStats(const mavlink_radio_status_t *msg) {
    switch(mavActiveConfig->radio_type) {
        case MAVLINK_RADIO_NONE:
            break;
        case MAVLINK_RADIO_SIK:
            rxLinkStatistics.uplinkRSSI = (msg->rssi / 1.9) - 127;
            rxLinkStatistics.uplinkSNR = msg->noise / 1.9;
            rxLinkStatistics.uplinkLQ = msg->rssi != 255 ? scaleRange(msg->rssi, 0, 254, 0, 100) : 0;
            break;
        case MAVLINK_RADIO_ELRS:
            rxLinkStatistics.uplinkRSSI = -msg->remrssi;
            rxLinkStatistics.uplinkSNR = msg->noise;
            rxLinkStatistics.uplinkLQ = scaleRange(msg->rssi, 0, 255, 0, 100);
            break;
        case MAVLINK_RADIO_MLRS:
            // rssi/remrssi are 0-254 AP scale; back-convert to dBm: range is -120 to -50
            rxLinkStatistics.uplinkRSSI = msg->rssi != 255
                ? (int8_t)(((int32_t)msg->rssi * (-50 - -120) / 254) + (-120))
                : 0;
            // noise field is actually: clamp(-SNR + 10, 0, 127), so SNR = -(noise - 10) = 10 - noise
            rxLinkStatistics.uplinkSNR = 10 - (int8_t)msg->noise;
            rxLinkStatistics.uplinkLQ = msg->rssi != 255
                ? scaleRange(msg->rssi, 0, 254, 0, 100)
                : 0;
            break;
        case MAVLINK_RADIO_GENERIC:
        default:
            rxLinkStatistics.uplinkRSSI = msg->rssi;
            rxLinkStatistics.uplinkSNR = msg->noise;
            rxLinkStatistics.uplinkLQ = msg->rssi != 255 ? scaleRange(msg->rssi, 0, 254, 0, 100) : 0;
            break;
    }
}

static bool handleIncoming_RADIO_STATUS(void) {
    mavlink_radio_status_t msg;
    mavlink_msg_radio_status_decode(&mavlinkContext.recvMsg, &msg);
    if (msg.txbuf > 0) {
        mavActivePort->txbuffValid = true;
        mavActivePort->txbuffFree = msg.txbuf;
    } else {
        mavActivePort->txbuffValid = false;
        mavActivePort->txbuffFree = 100;
    }

    if (rxConfig()->receiverType == RX_TYPE_SERIAL &&
        rxConfig()->serialrx_provider == SERIALRX_MAVLINK) {
        mavlinkParseRxStats(&msg);
    }

    return true;
}

#ifdef USE_ADSB
static bool handleIncoming_ADSB_VEHICLE(void) {
    mavlink_adsb_vehicle_t msg;
    mavlink_msg_adsb_vehicle_decode(&mavlinkContext.recvMsg, &msg);

    adsbVehicleValues_t* vehicle = getVehicleForFill();
    if(vehicle != NULL){
        vehicle->icao = msg.ICAO_address;
        vehicle->gps.lat = msg.lat;
        vehicle->gps.lon = msg.lon;
        vehicle->alt = (int32_t)(msg.altitude / 10);
        vehicle->horVelocity = msg.hor_velocity;
        vehicle->heading = msg.heading;
        vehicle->flags = msg.flags;
        vehicle->altitudeType = msg.altitude_type;
        memcpy(&(vehicle->callsign), msg.callsign, sizeof(vehicle->callsign));
        vehicle->emitterType = msg.emitter_type;
        vehicle->tslc = msg.tslc;

        adsbNewVehicle(vehicle);
    }

    return true;
}
#endif

mavlinkFcDispatchResult_e mavlinkFcDispatchIncomingMessage(uint8_t ingressPortIndex)
{
    UNUSED(ingressPortIndex);

    switch (mavlinkContext.recvMsg.msgid) {
    case MAVLINK_MSG_ID_HEARTBEAT:
        return mavlinkHandleIncomingHeartbeat() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
        return handleIncoming_PARAM_REQUEST_LIST() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MISSION_CLEAR_ALL:
        return mavlinkHandleIncomingMissionClearAll() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MISSION_COUNT:
        return mavlinkHandleIncomingMissionCount() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MISSION_ITEM:
        return mavlinkHandleIncomingMissionItem() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MISSION_ITEM_INT:
        return mavlinkHandleIncomingMissionItemInt() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:
        return mavlinkHandleIncomingMissionRequestList() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_COMMAND_LONG:
        return mavlinkHandleIncomingCommandLong() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_COMMAND_INT:
        return mavlinkHandleIncomingCommandInt() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MISSION_REQUEST:
        return mavlinkHandleIncomingMissionRequest() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MISSION_REQUEST_INT:
        return mavlinkHandleIncomingMissionRequestInt() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_REQUEST_DATA_STREAM:
        return mavlinkHandleIncomingRequestDataStream() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:
        handleIncoming_RC_CHANNELS_OVERRIDE();
        return MAVLINK_FC_DISPATCH_HANDLED_NO_ACTIVITY;
    case MAVLINK_MSG_ID_SET_POSITION_TARGET_LOCAL_NED:
        return mavlinkHandleIncomingSetPositionTargetLocalNed() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_SET_POSITION_TARGET_GLOBAL_INT:
        return mavlinkHandleIncomingSetPositionTargetGlobalInt() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
#ifdef USE_ADSB
    case MAVLINK_MSG_ID_ADSB_VEHICLE:
        return handleIncoming_ADSB_VEHICLE() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
#endif
    case MAVLINK_MSG_ID_RADIO_STATUS:
        handleIncoming_RADIO_STATUS();
        return MAVLINK_FC_DISPATCH_HANDLED_NO_ACTIVITY;
    case MAVLINK_MSG_ID_TUNNEL:
        return handleIncoming_TUNNEL(ingressPortIndex) ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    default:
        return MAVLINK_FC_DISPATCH_NOT_HANDLED;
    }
}


#endif
