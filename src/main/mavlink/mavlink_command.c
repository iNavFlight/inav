#include "mavlink/mavlink_internal.h"

#include "mavlink/mavlink_command.h"
#include "mavlink/mavlink_guided.h"
#include "mavlink/mavlink_runtime.h"
#include "mavlink/mavlink_streams.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_MAVLINK)

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

static bool handleIncoming_COMMAND(
    uint8_t targetSystem, 
    uint8_t targetComponent, 
    uint8_t ackTargetSystem, 
    uint8_t ackTargetComponent, 
    uint16_t command, 
    uint8_t frame, 
    float param1, 
    float param2, 
    float param3, 
    float param4, 
    float latitudeDeg, 
    float longitudeDeg, 
    float altitudeMeters) 
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

bool mavlinkHandleIncomingCommandInt(void)
{
    mavlink_command_int_t msg;
    mavlink_msg_command_int_decode(&mavlinkContext.recvMsg, &msg);

    return handleIncoming_COMMAND(msg.target_system, msg.target_component, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, msg.command, msg.frame, msg.param1, msg.param2, msg.param3, msg.param4, (float)msg.x / 1e7f, (float)msg.y / 1e7f, msg.z);
}

bool mavlinkHandleIncomingCommandLong(void)
{
    mavlink_command_long_t msg;
    mavlink_msg_command_long_decode(&mavlinkContext.recvMsg, &msg);

    // COMMAND_LONG has no frame field; location commands are WGS84 global by definition.
    return handleIncoming_COMMAND(msg.target_system, msg.target_component, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, msg.command, MAV_FRAME_GLOBAL, msg.param1, msg.param2, msg.param3, msg.param4, msg.param5, msg.param6, msg.param7);
}

#endif
