#include "mavlink/mavlink_internal.h"

#include "fc/fc_mavlink.h"

#include "mavlink/mavlink_command.h"
#include "mavlink/mavlink_guided.h"
#include "mavlink/mavlink_mission.h"
#include "mavlink/mavlink_ports.h"
#include "mavlink/mavlink_runtime.h"
#include "mavlink/mavlink_streams.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_MAVLINK)

#ifdef USE_MAVLINK_MSP_TUNNEL
static void mavlinkReleaseTunnelOwnerIfIdle(uint8_t ingressPortIndex)
{
    if (mavTunnelMspPorts[ingressPortIndex].c_state == MSP_IDLE) {
        mavTunnelRemoteSystemIds[ingressPortIndex] = 0;
        mavTunnelRemoteComponentIds[ingressPortIndex] = 0;
    }
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

static bool mavlinkSendTunnelMspReply(uint8_t targetSystem, uint8_t targetComponent, mspPacket_t *reply, uint8_t *replyPayloadHead, mspVersion_e mspVersion)
{
    sbufSwitchToReader(&reply->buf, replyPayloadHead);

    mspEncodedFrame_t frame;
    if (!mspSerialPrepareFrame(reply, mspVersion, &frame)) {
        return false;
    }

    uint8_t chunk[MAVLINK_MSG_TUNNEL_FIELD_PAYLOAD_LEN];
    for (int offset = 0; offset < frame.totalLength; offset += MAVLINK_MSG_TUNNEL_FIELD_PAYLOAD_LEN) {
        const int chunkLength = mspSerialFrameCopyRange(&frame, offset, chunk, sizeof(chunk));
        if (chunkLength <= 0) {
            return false;
        }
        mavlinkSendTunnelReply(targetSystem, targetComponent, chunk, chunkLength);
    }
    return true;
}

static bool mavlinkTunnelMessageTargetsLocalFc(const mavlink_tunnel_t *msg)
{
    return msg->payload_type == MAVLINK_TUNNEL_PAYLOAD_TYPE_INAV_MSP &&
        msg->target_system == mavSystemId &&
        (msg->target_component == 0 || msg->target_component == mavComponentId);
}

static void mavlinkPrepareTunnelParser(uint8_t ingressPortIndex, timeMs_t now)
{
    mspPort_t *mspPort = &mavTunnelMspPorts[ingressPortIndex];
    const bool senderChanged =
        mavTunnelRemoteSystemIds[ingressPortIndex] != mavlinkContext.recvMsg.sysid ||
        mavTunnelRemoteComponentIds[ingressPortIndex] != mavlinkContext.recvMsg.compid;
    const bool timedOut = (now - mspPort->lastActivityMs) >= MAVLINK_TUNNEL_MSP_TIMEOUT_MS;

    if (mspPort->c_state != MSP_IDLE && (timedOut || senderChanged)) {
        mavlinkResetTunnelPortState(ingressPortIndex);
    }

    mavTunnelRemoteSystemIds[ingressPortIndex] = mavlinkContext.recvMsg.sysid;
    mavTunnelRemoteComponentIds[ingressPortIndex] = mavlinkContext.recvMsg.compid;
    mspPort->lastActivityMs = now;
}

static bool mavlinkProcessCompletedTunnelCommand(uint8_t ingressPortIndex)
{
    mspPort_t *mspPort = &mavTunnelMspPorts[ingressPortIndex];
    const mspVersion_e mspVersion = mspPort->mspVersion;

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
        mspPort->c_state = MSP_IDLE;
        mavlinkSendTunnelMspReply(
            mavlinkContext.recvMsg.sysid,
            mavlinkContext.recvMsg.compid,
            &reply,
            replyPayloadHead,
            mspVersion);
        return false;
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
        mavlinkSendTunnelMspReply(
            mavlinkContext.recvMsg.sysid,
            mavlinkContext.recvMsg.compid,
            &reply,
            replyPayloadHead,
            mspVersion);
    }

    if (!mspPostProcessFn) {
        return false;
    }

    waitForSerialPortToFinishTransmitting(mavPortStates[ingressPortIndex].port);
    mspPostProcessFn(mavPortStates[ingressPortIndex].port);
    return true;
}

static bool handleIncoming_TUNNEL(uint8_t ingressPortIndex)
{
    if (mavlinkGetProtocolVersion() == 1) {
        return false;
    }

    mavlink_tunnel_t msg;
    mavlink_msg_tunnel_decode(&mavlinkContext.recvMsg, &msg);

    if (!mavlinkTunnelMessageTargetsLocalFc(&msg)) {
        return false;
    }

    if (msg.payload_length > MAVLINK_MSG_TUNNEL_FIELD_PAYLOAD_LEN) {
        mavlinkResetTunnelPortState(ingressPortIndex);
        return false;
    }

    mavlinkPrepareTunnelParser(ingressPortIndex, millis());

    mspPort_t *mspPort = &mavTunnelMspPorts[ingressPortIndex];
    bool handled = false;
    for (uint8_t payloadIndex = 0; payloadIndex < msg.payload_length; payloadIndex++) {
        if (!mspSerialProcessReceivedByte(mspPort, msg.payload[payloadIndex])) {
            continue;
        }

        handled = true;
        if (mspPort->c_state != MSP_COMMAND_RECEIVED) {
            continue;
        }

        if (mavlinkProcessCompletedTunnelCommand(ingressPortIndex)) {
            mavlinkResetTunnelPortState(ingressPortIndex);
            break;
        }
    }

    mavlinkReleaseTunnelOwnerIfIdle(ingressPortIndex);
    return handled;
}
#endif

static int8_t mavlinkIngressPortRxLink(uint8_t ingressPortIndex)
{
    if (ingressPortIndex >= mavPortCount || !mavPortStates[ingressPortIndex].portConfig) {
        return -1;
    }
    return mavlinkRxLinkForPortFunctionMask(mavPortStates[ingressPortIndex].portConfig->functionMask);
}

static bool handleIncoming_RC_CHANNELS_OVERRIDE(uint8_t ingressPortIndex)
{
    const int8_t link = mavlinkIngressPortRxLink(ingressPortIndex);
    if (link < 0) {
        return false;
    }

    mavlink_rc_channels_override_t msg;
    mavlink_msg_rc_channels_override_decode(&mavlinkContext.recvMsg, &msg);
    if ((msg.target_system != 0 && msg.target_system != mavSystemId) ||
        (msg.target_component != 0 && msg.target_component != mavComponentId)) {
        return false;
    }

    mavlinkRxHandleMessage((rxLink_e)link, &msg);
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

static uint16_t mavlinkDbmToMilliwatts(int8_t powerDbm)
{
    if (powerDbm == INT8_MAX) {
        return 0;
    }

    return powerDbm <= 0 ? 0 : lrintf(powf(10.0f, powerDbm / 10.0f));
}

static void mavlinkParseRxStats(rxLink_e link, uint8_t ingressPortIndex, const mavlink_radio_status_t *msg)
{
    rxLinkStatistics_t *statistics = rxGetLinkStatisticsMutable(link);
    const mavlinkTelemetryPortConfig_t *portConfig = mavlinkGetPortConfig(ingressPortIndex);
    uint16_t validFields = 0;

    switch(portConfig->radio_type) {
        case MAVLINK_RADIO_SIK:
            if (msg->rssi != UINT8_MAX) {
                statistics->uplinkRSSI = (msg->rssi / 1.9) - 127;
                statistics->uplinkLQ = scaleRange(msg->rssi, 0, 254, 0, 100);
                validFields |= RX_LINK_STATS_UPLINK_RSSI | RX_LINK_STATS_UPLINK_LQ;
            }
            if (msg->noise != UINT8_MAX) {
                statistics->uplinkSNR = msg->noise / 1.9;
                validFields |= RX_LINK_STATS_UPLINK_SNR;
            }
            break;

        case MAVLINK_RADIO_ELRS:
            if (msg->remrssi != UINT8_MAX) {
                statistics->uplinkRSSI = -msg->remrssi;
                validFields |= RX_LINK_STATS_UPLINK_RSSI;
            }
            if (msg->rssi != UINT8_MAX) {
                statistics->uplinkLQ = scaleRange(msg->rssi, 0, 254, 0, 100);
                validFields |= RX_LINK_STATS_UPLINK_LQ;
            }
            if (msg->noise != UINT8_MAX) {
                statistics->uplinkSNR = msg->noise;
                validFields |= RX_LINK_STATS_UPLINK_SNR;
            }
            break;

        case MAVLINK_RADIO_MLRS:
            // mLRS exposes its RC-link metrics through the dedicated mLRS
            // messages below. RADIO_STATUS is still used for transport flow
            // control, but must not make an empty RX statistics record valid.
            return;

        case MAVLINK_RADIO_GENERIC:
        default:
            if (msg->rssi != UINT8_MAX) {
                statistics->uplinkRSSI = msg->rssi;
                statistics->uplinkLQ = scaleRange(msg->rssi, 0, 254, 0, 100);
                validFields |= RX_LINK_STATS_UPLINK_RSSI | RX_LINK_STATS_UPLINK_LQ;
            }
            if (msg->noise != UINT8_MAX) {
                statistics->uplinkSNR = msg->noise;
                validFields |= RX_LINK_STATS_UPLINK_SNR;
            }
            break;
    }

    if (validFields) {
        rxLinkStatisticsUpdated(link, validFields);
    }
}

static bool handleIncoming_RADIO_STATUS(uint8_t ingressPortIndex) {
    mavlink_radio_status_t msg;
    mavlink_msg_radio_status_decode(&mavlinkContext.recvMsg, &msg);
    mavlinkPortRuntime_t *ingressPort = &mavPortStates[ingressPortIndex];
    if (msg.txbuf > 0) {
        ingressPort->txbuffValid = true;
        ingressPort->txbuffFree = msg.txbuf;
    } else {
        ingressPort->txbuffValid = false;
        ingressPort->txbuffFree = 100;
    }

    const int8_t rxLink = mavlinkIngressPortRxLink(ingressPortIndex);
    if (rxLink >= 0) {
        mavlinkParseRxStats((rxLink_e)rxLink, ingressPortIndex, &msg);
    }

    return true;
}

static bool handleIncoming_MLRS_RADIO_LINK_STATS(uint8_t ingressPortIndex)
{
    if (mavlinkContext.recvMsg.compid != MAV_COMP_ID_TELEMETRY_RADIO) {
        return false;
    }

    mavlink_mlrs_radio_link_stats_t msg;
    mavlink_msg_mlrs_radio_link_stats_decode(&mavlinkContext.recvMsg, &msg);

    if (msg.target_system != mavSystemId) {
        return false;
    }
    if (msg.target_component != 0 && msg.target_component != mavComponentId) {
        return false;
    }

    mavlinkMlrsLinkStatsRuntime_t *stats = &mavPortStates[ingressPortIndex].mlrs.stats;
    stats->valid = true;
    stats->packet = msg;
    stats->rssiIsDbm = (msg.flags & MLRS_RADIO_LINK_STATS_FLAGS_RSSI_DBM) != 0;
    stats->activeAntenna = (msg.flags & MLRS_RADIO_LINK_STATS_FLAGS_RX_RECEIVE_ANTENNA2) ? 1 : 0;
    const bool uplinkLqValid = msg.rx_LQ_rc != UINT8_MAX;
    const bool downlinkLqValid = msg.rx_LQ_ser != UINT8_MAX;
    stats->rxLinkQualityRc = uplinkLqValid ? MIN(msg.rx_LQ_rc, 100) : 0;
    stats->rxLinkQualitySerial = downlinkLqValid ? MIN(msg.rx_LQ_ser, 100) : 0;

    uint8_t rxRssi = stats->activeAntenna ? msg.rx_rssi2 : msg.rx_rssi1;
    if (stats->activeAntenna && rxRssi == UINT8_MAX) {
        rxRssi = msg.rx_rssi1;
    }
    const bool rssiValid = stats->rssiIsDbm && rxRssi != 0 && rxRssi != UINT8_MAX;
    stats->rxRssi = rssiValid ? -rxRssi : 0;

    int8_t rxSnr = stats->activeAntenna ? msg.rx_snr2 : msg.rx_snr1;
    if (stats->activeAntenna && rxSnr == INT8_MAX) {
        rxSnr = msg.rx_snr1;
    }
    const bool snrValid = rxSnr != INT8_MAX;
    stats->rxSnr = snrValid ? rxSnr : 0;

    const int8_t rxLink = mavlinkIngressPortRxLink(ingressPortIndex);
    if (rxLink < 0) {
        return true;
    }

    rxLinkStatistics_t *statistics = rxGetLinkStatisticsMutable((rxLink_e)rxLink);
    uint16_t validFields = RX_LINK_STATS_ACTIVE_ANTENNA;
    statistics->activeAntenna = stats->activeAntenna;
    if (uplinkLqValid) {
        statistics->uplinkLQ = stats->rxLinkQualityRc;
        validFields |= RX_LINK_STATS_UPLINK_LQ;
    }
    if (downlinkLqValid) {
        statistics->downlinkLQ = stats->rxLinkQualitySerial;
        validFields |= RX_LINK_STATS_DOWNLINK_LQ;
    }
    if (rssiValid) {
        statistics->uplinkRSSI = stats->rxRssi;
        validFields |= RX_LINK_STATS_UPLINK_RSSI;
    }
    if (snrValid) {
        statistics->uplinkSNR = stats->rxSnr;
        validFields |= RX_LINK_STATS_UPLINK_SNR;
    }
    rxLinkStatisticsUpdated((rxLink_e)rxLink, validFields);

    return true;
}

static bool handleIncoming_MLRS_RADIO_LINK_INFORMATION(uint8_t ingressPortIndex)
{
    if (mavlinkContext.recvMsg.compid != MAV_COMP_ID_TELEMETRY_RADIO) {
        return false;
    }

    mavlink_mlrs_radio_link_information_t msg;
    mavlink_msg_mlrs_radio_link_information_decode(&mavlinkContext.recvMsg, &msg);

    if (msg.target_system != mavSystemId) {
        return false;
    }
    if (msg.target_component != 0 && msg.target_component != mavComponentId) {
        return false;
    }

    mavlinkMlrsLinkInformationRuntime_t *info = &mavPortStates[ingressPortIndex].mlrs.info;
    info->valid = true;
    info->packet = msg;
    memset(info->modeStr, 0, sizeof(info->modeStr));
    memset(info->bandStr, 0, sizeof(info->bandStr));
    memcpy(info->modeStr, msg.mode_str, MAVLINK_MSG_MLRS_RADIO_LINK_INFORMATION_FIELD_MODE_STR_LEN);
    memcpy(info->bandStr, msg.band_str, MAVLINK_MSG_MLRS_RADIO_LINK_INFORMATION_FIELD_BAND_STR_LEN);
    info->txPowerMw = mavlinkDbmToMilliwatts(msg.tx_power);
    info->rxPowerMw = mavlinkDbmToMilliwatts(msg.rx_power);
    info->txReceiveSensitivityDbm = msg.tx_receive_sensitivity ? -(int16_t)msg.tx_receive_sensitivity : 0;
    info->rxReceiveSensitivityDbm = msg.rx_receive_sensitivity ? -(int16_t)msg.rx_receive_sensitivity : 0;

    const int8_t rxLink = mavlinkIngressPortRxLink(ingressPortIndex);
    if (rxLink < 0) {
        return true;
    }

    rxLinkStatistics_t *statistics = rxGetLinkStatisticsMutable((rxLink_e)rxLink);
    uint16_t validFields = 0;

    if (msg.tx_power != INT8_MAX) {
        statistics->uplinkTXPower = info->txPowerMw;
        validFields |= RX_LINK_STATS_UPLINK_TX_POWER;
    }
    if (msg.rx_power != INT8_MAX) {
        statistics->downlinkTXPower = info->rxPowerMw;
        validFields |= RX_LINK_STATS_DOWNLINK_TX_POWER;
    }

    memset(statistics->band, 0, sizeof(statistics->band));
    memset(statistics->mode, 0, sizeof(statistics->mode));
    memcpy(statistics->band, info->bandStr, sizeof(statistics->band) - 1);
    memcpy(statistics->mode, info->modeStr, sizeof(statistics->mode) - 1);
    sl_toupperptr(statistics->band);
    sl_toupperptr(statistics->mode);
    if (statistics->band[0]) {
        validFields |= RX_LINK_STATS_BAND;
    }
    if (statistics->mode[0]) {
        validFields |= RX_LINK_STATS_MODE;
    }

    if (validFields) {
        rxLinkStatisticsUpdated((rxLink_e)rxLink, validFields);
    }

    return true;
}

static bool handleIncoming_MLRS_RADIO_LINK_FLOW_CONTROL(uint8_t ingressPortIndex)
{
    if (mavlinkContext.recvMsg.compid != MAV_COMP_ID_TELEMETRY_RADIO) {
        return false;
    }

    mavlink_mlrs_radio_link_flow_control_t msg;
    mavlink_msg_mlrs_radio_link_flow_control_decode(&mavlinkContext.recvMsg, &msg);

    mavlinkMlrsFlowControlRuntime_t *flowControl = &mavPortStates[ingressPortIndex].mlrs.flowControl;
    flowControl->valid = true;
    flowControl->packet = msg;

    mavlinkPortRuntime_t *ingressPort = &mavPortStates[ingressPortIndex];
    if (msg.txbuf <= 100) {
        ingressPort->txbuffValid = true;
        ingressPort->txbuffFree = msg.txbuf;
    } else {
        ingressPort->txbuffValid = false;
        ingressPort->txbuffFree = 100;
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
    switch (mavlinkContext.recvMsg.msgid) {
    case MAVLINK_MSG_ID_HEARTBEAT:
        return mavlinkHandleIncomingHeartbeat() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_PING:
        return mavlinkHandleIncomingPing() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_TIMESYNC:
        return mavlinkHandleIncomingTimesync() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
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
    case MAVLINK_MSG_ID_MISSION_ACK:
        return mavlinkHandleIncomingMissionAck() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_REQUEST_DATA_STREAM:
        return mavlinkHandleIncomingRequestDataStream() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:
        return handleIncoming_RC_CHANNELS_OVERRIDE(ingressPortIndex) ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_SET_POSITION_TARGET_LOCAL_NED:
        return mavlinkHandleIncomingSetPositionTargetLocalNed() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_SET_POSITION_TARGET_GLOBAL_INT:
        return mavlinkHandleIncomingSetPositionTargetGlobalInt() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
#ifdef USE_ADSB
    case MAVLINK_MSG_ID_ADSB_VEHICLE:
        return handleIncoming_ADSB_VEHICLE() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
#endif
    case MAVLINK_MSG_ID_MLRS_RADIO_LINK_STATS:
        return handleIncoming_MLRS_RADIO_LINK_STATS(ingressPortIndex) ? MAVLINK_FC_DISPATCH_HANDLED_NO_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MLRS_RADIO_LINK_INFORMATION:
        return handleIncoming_MLRS_RADIO_LINK_INFORMATION(ingressPortIndex) ? MAVLINK_FC_DISPATCH_HANDLED_NO_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MLRS_RADIO_LINK_FLOW_CONTROL:
        return handleIncoming_MLRS_RADIO_LINK_FLOW_CONTROL(ingressPortIndex) ? MAVLINK_FC_DISPATCH_HANDLED_NO_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_RADIO_STATUS:
        return handleIncoming_RADIO_STATUS(ingressPortIndex) ? MAVLINK_FC_DISPATCH_HANDLED_NO_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
#ifdef USE_MAVLINK_MSP_TUNNEL
    case MAVLINK_MSG_ID_TUNNEL:
        return handleIncoming_TUNNEL(ingressPortIndex) ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
#endif
    default:
        return MAVLINK_FC_DISPATCH_NOT_HANDLED;
    }
}


#endif
