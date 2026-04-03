#include "mavlink/mavlink_internal.h"

#include "fc/fc_mavlink.h"

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

static bool mavlinkIsLocalTarget(uint8_t targetSystem, uint8_t targetComponent)
{
    if (targetSystem != 0 && targetSystem != mavSystemId) {
        return false;
    }

    if (targetComponent != 0 && targetComponent != mavComponentId) {
        return false;
    }

    return true;
}

static void mavlinkSendCommandAck(uint16_t command, MAV_RESULT result, uint8_t ackTargetSystem, uint8_t ackTargetComponent)
{
    mavlink_msg_command_ack_pack(mavSystemId, mavComponentId, &mavSendMsg,
        command,
        result,
        0,
        0,
        ackTargetSystem,
        ackTargetComponent);
    mavlinkSendMessage();
}

static bool handleIncoming_COMMAND(uint8_t targetSystem, uint8_t targetComponent, uint8_t ackTargetSystem, uint8_t ackTargetComponent, uint16_t command, uint8_t frame, float param1, float param2, float param3, float param4, float latitudeDeg, float longitudeDeg, float altitudeMeters)
{
    if (!mavlinkIsLocalTarget(targetSystem, targetComponent)) {
        return false;
    }
    UNUSED(param3);

    switch (command) {
        case MAV_CMD_DO_REPOSITION:
            if (!mavlinkFrameIsSupported(frame,
                MAV_FRAME_SUPPORTED_GLOBAL |
                MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT |
                MAV_FRAME_SUPPORTED_GLOBAL_INT |
                MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT)) {
                mavlinkSendCommandAck(command, MAV_RESULT_UNSUPPORTED, ackTargetSystem, ackTargetComponent);
                return true;
            }

            if (isnan(latitudeDeg) || isnan(longitudeDeg)) {
                mavlinkSendCommandAck(command, MAV_RESULT_FAILED, ackTargetSystem, ackTargetComponent);
                return true;
            }

            if (isGCSValid()) {
                navWaypoint_t wp = {0};
                wp.action = NAV_WP_ACTION_WAYPOINT;
                wp.lat = (int32_t)(latitudeDeg * 1e7f);
                wp.lon = (int32_t)(longitudeDeg * 1e7f);
                wp.alt = (int32_t)(altitudeMeters * 100.0f);
                if (!isnan(param4) && param4 >= 0.0f && param4 < 360.0f) {
                    wp.p1 = (int16_t)param4;
                } else {
                    wp.p1 = 0;
                }
                wp.p2 = 0;
                wp.p3 = mavlinkFrameUsesAbsoluteAltitude(frame) ? NAV_WP_ALTMODE : 0;
                wp.flag = 0;

                setWaypoint(255, &wp);

                mavlinkSendCommandAck(command, MAV_RESULT_ACCEPTED, ackTargetSystem, ackTargetComponent);
            } else {
                mavlinkSendCommandAck(command, MAV_RESULT_DENIED, ackTargetSystem, ackTargetComponent);
            }
            return true;
        case MAV_CMD_DO_CHANGE_ALTITUDE:
            {
                const MAV_RESULT result = mavlinkSetAltitudeTargetFromFrame((uint8_t)lrintf(param2), param1);
                mavlinkSendCommandAck(command, result, ackTargetSystem, ackTargetComponent);
                return true;
            }
        case MAV_CMD_CONDITION_YAW:
            {
                if (!(navGetCurrentStateFlags() & NAV_CTL_YAW)) {
                    mavlinkSendCommandAck(command, MAV_RESULT_DENIED, ackTargetSystem, ackTargetComponent);
                    return true;
                }

                int32_t targetHeadingCd = wrap_36000((int32_t)lrintf(param1 * 100.0f));

                if (param4 != 0.0f) {
                    const int32_t currentHeadingCd = STATE(AIRPLANE) ? posControl.actualState.cog : posControl.actualState.yaw;
                    const int32_t headingChangeCd = (int32_t)lrintf(fabsf(param1) * 100.0f);

                    if (param3 < 0.0f) {
                        targetHeadingCd = wrap_36000(currentHeadingCd - headingChangeCd);
                    } else {
                        targetHeadingCd = wrap_36000(currentHeadingCd + headingChangeCd);
                    }
                }

                updateHeadingHoldTarget(CENTIDEGREES_TO_DEGREES(targetHeadingCd));
                posControl.desiredState.yaw = targetHeadingCd;
                posControl.cruise.course = targetHeadingCd;
                posControl.cruise.previousCourse = targetHeadingCd;

                mavlinkSendCommandAck(command, MAV_RESULT_ACCEPTED, ackTargetSystem, ackTargetComponent);
                return true;
            }
        case MAV_CMD_SET_MESSAGE_INTERVAL:
            {
                mavlinkPeriodicMessage_e periodicMessage;
                MAV_RESULT result = MAV_RESULT_UNSUPPORTED;

                if (mavlinkPeriodicMessageFromMessageId((uint16_t)param1, &periodicMessage)) {
                    if (param2 == 0.0f) {
                        mavlinkSetMessageOverrideIntervalUs(periodicMessage, 0);
                        result = MAV_RESULT_ACCEPTED;
                    } else if (param2 < 0.0f) {
                        mavlinkSetMessageOverrideIntervalUs(periodicMessage, -1);
                        result = MAV_RESULT_ACCEPTED;
                    } else {
                        uint32_t intervalUs = (uint32_t)param2;
                        if (intervalUs > 0) {
                            const uint32_t minIntervalUs = 1000000UL / TELEMETRY_MAVLINK_MAXRATE;
                            if (intervalUs < minIntervalUs) {
                                intervalUs = minIntervalUs;
                            }

                            mavlinkSetMessageOverrideIntervalUs(periodicMessage, (int32_t)intervalUs);
                            result = MAV_RESULT_ACCEPTED;
                        }
                    }
                }

                mavlinkSendCommandAck(command, result, ackTargetSystem, ackTargetComponent);
                return true;
            }
        case MAV_CMD_GET_MESSAGE_INTERVAL:
            {
                mavlinkPeriodicMessage_e periodicMessage;
                if (!mavlinkPeriodicMessageFromMessageId((uint16_t)param1, &periodicMessage)) {
                    mavlinkSendCommandAck(command, MAV_RESULT_UNSUPPORTED, ackTargetSystem, ackTargetComponent);
                    return true;
                }

                mavlink_msg_message_interval_pack(
                    mavSystemId,
                    mavComponentId,
                    &mavSendMsg,
                    (uint16_t)param1,
                    mavlinkMessageIntervalUs(periodicMessage));
                mavlinkSendMessage();

                mavlinkSendCommandAck(command, MAV_RESULT_ACCEPTED, ackTargetSystem, ackTargetComponent);
                return true;
            }
        case MAV_CMD_CONTROL_HIGH_LATENCY:
            if (param1 == 0.0f || param1 == 1.0f) {
                if (mavlinkGetProtocolVersion() == 1 && param1 > 0.5f) {
                    mavlinkSendCommandAck(command, MAV_RESULT_UNSUPPORTED, ackTargetSystem, ackTargetComponent);
                    return true;
                }

                mavActivePort->highLatencyEnabled = param1 > 0.5f;
                if (mavActivePort->highLatencyEnabled) {
                    mavActivePort->lastHighLatencyMessageUs = 0;
                }
                mavlinkSendCommandAck(command, MAV_RESULT_ACCEPTED, ackTargetSystem, ackTargetComponent);
            } else {
                mavlinkSendCommandAck(command, MAV_RESULT_FAILED, ackTargetSystem, ackTargetComponent);
            }
            return true;
        case MAV_CMD_REQUEST_PROTOCOL_VERSION:
            if (mavlinkGetProtocolVersion() == 1) {
                mavlinkSendCommandAck(command, MAV_RESULT_UNSUPPORTED, ackTargetSystem, ackTargetComponent);
            } else {
                mavlinkSendProtocolVersion();
                mavlinkSendCommandAck(command, MAV_RESULT_ACCEPTED, ackTargetSystem, ackTargetComponent);
            }
            return true;
        case MAV_CMD_REQUEST_AUTOPILOT_CAPABILITIES:
            if (mavlinkGetProtocolVersion() == 1) {
                mavlinkSendCommandAck(command, MAV_RESULT_UNSUPPORTED, ackTargetSystem, ackTargetComponent);
            } else {
                mavlinkSendAutopilotVersion();
                mavlinkSendCommandAck(command, MAV_RESULT_ACCEPTED, ackTargetSystem, ackTargetComponent);
            }
            return true;
        case MAV_CMD_REQUEST_MESSAGE:
            {
                const bool sent = mavlinkSendRequestedMessage((uint16_t)param1);
                mavlinkSendCommandAck(command, sent ? MAV_RESULT_ACCEPTED : MAV_RESULT_UNSUPPORTED, ackTargetSystem, ackTargetComponent);
                return true;
            }
#ifdef USE_GPS
        case MAV_CMD_GET_HOME_POSITION:
            if (mavlinkSendRequestedMessage(MAVLINK_MSG_ID_HOME_POSITION)) {
                mavlinkSendCommandAck(command, MAV_RESULT_ACCEPTED, ackTargetSystem, ackTargetComponent);
            } else {
                mavlinkSendCommandAck(command, MAV_RESULT_FAILED, ackTargetSystem, ackTargetComponent);
            }
            return true;
#endif
        default:
            mavlinkSendCommandAck(command, MAV_RESULT_UNSUPPORTED, ackTargetSystem, ackTargetComponent);
            return true;
    }
}

static bool handleIncoming_COMMAND_INT(void)
{
    mavlink_command_int_t msg;
    mavlink_msg_command_int_decode(&mavlinkContext.recvMsg, &msg);

    return handleIncoming_COMMAND(msg.target_system, msg.target_component, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, msg.command, msg.frame, msg.param1, msg.param2, msg.param3, msg.param4, (float)msg.x / 1e7f, (float)msg.y / 1e7f, msg.z);
}

static bool handleIncoming_COMMAND_LONG(void)
{
    mavlink_command_long_t msg;
    mavlink_msg_command_long_decode(&mavlinkContext.recvMsg, &msg);

    // COMMAND_LONG has no frame field; location commands are WGS84 global by definition.
    return handleIncoming_COMMAND(msg.target_system, msg.target_component, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, msg.command, MAV_FRAME_GLOBAL, msg.param1, msg.param2, msg.param3, msg.param4, msg.param5, msg.param6, msg.param7);
}

static bool handleIncoming_REQUEST_DATA_STREAM(void)
{
    mavlink_request_data_stream_t msg;
    mavlink_msg_request_data_stream_decode(&mavlinkContext.recvMsg, &msg);

    if (!mavlinkIsLocalTarget(msg.target_system, msg.target_component)) {
        return false;
    }

    uint8_t rate = 0;
    if (msg.start_stop != 0) {
        rate = (uint8_t)msg.req_message_rate;
        if (rate > TELEMETRY_MAVLINK_MAXRATE) {
            rate = TELEMETRY_MAVLINK_MAXRATE;
        }
    }

    if (msg.req_stream_id == MAV_DATA_STREAM_ALL) {
        mavlinkSetStreamRate(MAV_DATA_STREAM_EXTENDED_STATUS, rate);
        mavlinkSetStreamRate(MAV_DATA_STREAM_RC_CHANNELS, rate);
        mavlinkSetStreamRate(MAV_DATA_STREAM_POSITION, rate);
        mavlinkSetStreamRate(MAV_DATA_STREAM_EXTRA1, rate);
        mavlinkSetStreamRate(MAV_DATA_STREAM_EXTRA2, rate);
        mavlinkSetStreamRate(MAV_DATA_STREAM_EXTRA3, rate);
        mavlinkSetStreamRate(MAV_DATA_STREAM_EXTENDED_SYS_STATE, rate);
        return true;
    }

    mavlinkSetStreamRate(msg.req_stream_id, rate);
    return true;
}

static bool handleIncoming_SET_POSITION_TARGET_GLOBAL_INT(void)
{
    mavlink_set_position_target_global_int_t msg;
    mavlink_msg_set_position_target_global_int_decode(&mavlinkContext.recvMsg, &msg);

    if (!mavlinkIsLocalTarget(msg.target_system, msg.target_component)) {
        return false;
    }

    uint8_t frame = msg.coordinate_frame;
    if (!mavlinkFrameIsSupported(frame,
        MAV_FRAME_SUPPORTED_GLOBAL |
        MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT |
        MAV_FRAME_SUPPORTED_GLOBAL_INT |
        MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT)) {
        return true;
    }

    const uint16_t typeMask = msg.type_mask;
    const bool xIgnored = (typeMask & POSITION_TARGET_TYPEMASK_X_IGNORE) != 0;
    const bool yIgnored = (typeMask & POSITION_TARGET_TYPEMASK_Y_IGNORE) != 0;
    const bool zIgnored = (typeMask & POSITION_TARGET_TYPEMASK_Z_IGNORE) != 0;

    // Altitude-only SET_POSITION_TARGET_GLOBAL_INT mirrors MAV_CMD_DO_CHANGE_ALTITUDE semantics.
    if (xIgnored && yIgnored && !zIgnored) {
        if (isGCSValid()) {
            mavlinkSetAltitudeTargetFromFrame(frame, msg.alt);
        }
        return true;
    }

    if (xIgnored || yIgnored) {
        return true;
    }

    if (isGCSValid()) {
        navWaypoint_t wp = {0};
        wp.action = NAV_WP_ACTION_WAYPOINT;
        wp.lat = msg.lat_int;
        wp.lon = msg.lon_int;
        wp.alt = zIgnored ? 0 : (int32_t)(msg.alt * 100.0f);
        wp.p1 = 0;
        wp.p2 = 0;
        wp.p3 = mavlinkFrameUsesAbsoluteAltitude(frame) ? NAV_WP_ALTMODE : 0;
        wp.flag = 0;

        setWaypoint(255, &wp);
    }

    return true;
}

static bool handleIncoming_SET_POSITION_TARGET_LOCAL_NED(void)
{
    mavlink_set_position_target_local_ned_t msg;
    mavlink_msg_set_position_target_local_ned_decode(&mavlinkContext.recvMsg, &msg);

    if (!mavlinkIsLocalTarget(msg.target_system, msg.target_component)) {
        return false;
    }

    if (msg.coordinate_frame != MAV_FRAME_LOCAL_OFFSET_NED) {
        return true;
    }

    const uint16_t typeMask = msg.type_mask;
    const bool xIgnored = (typeMask & POSITION_TARGET_TYPEMASK_X_IGNORE) != 0;
    const bool yIgnored = (typeMask & POSITION_TARGET_TYPEMASK_Y_IGNORE) != 0;
    const bool zIgnored = (typeMask & POSITION_TARGET_TYPEMASK_Z_IGNORE) != 0;

    if (!isGCSValid() || zIgnored) {
        return true;
    }

    if ((!xIgnored && fabsf(msg.x) > 0.01f) || (!yIgnored && fabsf(msg.y) > 0.01f)) {
        return true;
    }

    const int32_t targetAltitudeCm = (int32_t)lrintf((float)getEstimatedActualPosition(Z) - (msg.z * 100.0f));
    navigationSetAltitudeTargetWithDatum(NAV_WP_TAKEOFF_DATUM, targetAltitudeCm);

    return true;
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

    // Respond that we don't have any parameters to force Mission Planner to give up quickly
    if (mavlinkIsLocalTarget(msg.target_system, msg.target_component)) {
        // mavlink_msg_param_value_pack(system_id, component_id, msg, param_value->param_id, param_value->param_value, param_value->param_type, param_value->param_count, param_value->param_index);
        mavlink_msg_param_value_pack(mavSystemId, mavComponentId, &mavSendMsg, 0, 0, 0, 0, 0);
        mavlinkSendMessage();
    }
    return true;
}

static void mavlinkParseRxStats(const mavlink_radio_status_t *msg) {
    switch(mavActiveConfig->radio_type) {
        case MAVLINK_RADIO_NONE:
            break;
        case MAVLINK_RADIO_SIK:
            // rssi scaling info from: https://ardupilot.org/rover/docs/common-3dr-radio-advanced-configuration-and-technical-information.html
            rxLinkStatistics.uplinkRSSI = (msg->rssi / 1.9) - 127;
            rxLinkStatistics.uplinkSNR = msg->noise / 1.9;
            rxLinkStatistics.uplinkLQ = msg->rssi != 255 ? scaleRange(msg->rssi, 0, 254, 0, 100) : 0;
            break;
        case MAVLINK_RADIO_ELRS:
            rxLinkStatistics.uplinkRSSI = -msg->remrssi;
            rxLinkStatistics.uplinkSNR = msg->noise;
            rxLinkStatistics.uplinkLQ = scaleRange(msg->rssi, 0, 255, 0, 100);
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

static bool handleIncoming_HEARTBEAT(void) {
    mavlink_heartbeat_t msg;
    mavlink_msg_heartbeat_decode(&mavlinkContext.recvMsg, &msg);

    switch (msg.type) {
#ifdef USE_ADSB
        case MAV_TYPE_ADSB:
            return adsbHeartbeat();
#endif
        default:
            break;
    }
    
    return false;
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
        return handleIncoming_HEARTBEAT() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
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
        return handleIncoming_COMMAND_LONG() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_COMMAND_INT:
        return handleIncoming_COMMAND_INT() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MISSION_REQUEST:
        return mavlinkHandleIncomingMissionRequest() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MISSION_REQUEST_INT:
        return mavlinkHandleIncomingMissionRequestInt() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_REQUEST_DATA_STREAM:
        return handleIncoming_REQUEST_DATA_STREAM() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:
        handleIncoming_RC_CHANNELS_OVERRIDE();
        return MAVLINK_FC_DISPATCH_HANDLED_NO_ACTIVITY;
    case MAVLINK_MSG_ID_SET_POSITION_TARGET_LOCAL_NED:
        return handleIncoming_SET_POSITION_TARGET_LOCAL_NED() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_SET_POSITION_TARGET_GLOBAL_INT:
        return handleIncoming_SET_POSITION_TARGET_GLOBAL_INT() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
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
