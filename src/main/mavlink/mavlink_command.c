#include "mavlink/mavlink_internal.h"

#include "mavlink/mavlink_command.h"
#include "mavlink/mavlink_guided.h"
#include "mavlink/mavlink_modes.h"
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

static bool mavlinkCommandParamToUint32(float value, uint32_t maximum, uint32_t *result)
{
    if (!isfinite(value) || value < 0.0f || (double)value > (double)maximum) {
        return false;
    }

    *result = (uint32_t)value;
    return true;
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

    switch (command) {
        case MAV_CMD_COMPONENT_ARM_DISARM:
            if (param1 != 0.0f && param1 != 1.0f) {
                mavlinkSendCommandAck(command, MAV_RESULT_DENIED, ackTargetSystem, ackTargetComponent);
                return true;
            }
            mavlinkSendCommandAck(command, fcSetArmState(param1 == 1.0f) ? MAV_RESULT_ACCEPTED : MAV_RESULT_DENIED, ackTargetSystem, ackTargetComponent);
            return true;
        case MAV_CMD_NAV_RETURN_TO_LAUNCH:
            if (!ARMING_FLAG(ARMED)) {
                mavlinkSendCommandAck(command, MAV_RESULT_DENIED, ackTargetSystem, ackTargetComponent);
                return true;
            }
            mavlinkSendCommandAck(command, activateRTHMode() ? MAV_RESULT_ACCEPTED : MAV_RESULT_DENIED, ackTargetSystem, ackTargetComponent);
            return true;
        case MAV_CMD_DO_SET_MODE:
            {
                uint32_t modeFlags;
                uint32_t customMode;
                if (!mavlinkCommandParamToUint32(param1, UINT8_MAX, &modeFlags) ||
                    !mavlinkCommandParamToUint32(param2, UINT8_MAX, &customMode) ||
                    (modeFlags & MAV_MODE_FLAG_CUSTOM_MODE_ENABLED) == 0) {
                    mavlinkSendCommandAck(command, MAV_RESULT_UNSUPPORTED, ackTargetSystem, ackTargetComponent);
                    return true;
                }

                const bool fixedWing = mavlinkIsFixedWingVehicle();
                const uint8_t rthMode = fixedWing ? PLANE_MODE_RTL : COPTER_MODE_RTL;
                const bool posHoldMode = fixedWing ?
                    (customMode == PLANE_MODE_LOITER) :
                    (customMode == COPTER_MODE_LOITER || customMode == COPTER_MODE_POSHOLD || customMode == COPTER_MODE_BRAKE);

                if (customMode != rthMode && !posHoldMode) {
                    mavlinkSendCommandAck(command, MAV_RESULT_UNSUPPORTED, ackTargetSystem, ackTargetComponent);
                    return true;
                }

                if (!ARMING_FLAG(ARMED)) {
                    mavlinkSendCommandAck(command, MAV_RESULT_DENIED, ackTargetSystem, ackTargetComponent);
                    return true;
                }

                if (customMode == rthMode) {
                    mavlinkSendCommandAck(command, activateRTHMode() ? MAV_RESULT_ACCEPTED : MAV_RESULT_DENIED, ackTargetSystem, ackTargetComponent);
                    return true;
                }

                mavlinkSendCommandAck(command, activatePositionHoldMode() ? MAV_RESULT_ACCEPTED : MAV_RESULT_DENIED, ackTargetSystem, ackTargetComponent);
                return true;
            }
        case MAV_CMD_NAV_LAND:
            mavlinkSendCommandAck(command, activateForcedLanding() ? MAV_RESULT_ACCEPTED : MAV_RESULT_DENIED, ackTargetSystem, ackTargetComponent);
            return true;
        case MAV_CMD_NAV_TAKEOFF:
            mavlinkSendCommandAck(command, MAV_RESULT_UNSUPPORTED, ackTargetSystem, ackTargetComponent);
            return true;
        case MAV_CMD_DO_SET_HOME:
            {
                if ((param1 != 0.0f && param1 != 1.0f) ||
                    !mavlinkFrameIsSupported(frame,
                        MAV_FRAME_SUPPORTED_GLOBAL |
                        MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT |
                        MAV_FRAME_SUPPORTED_GLOBAL_INT |
                        MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT)) {
                    mavlinkSendCommandAck(command, MAV_RESULT_UNSUPPORTED, ackTargetSystem, ackTargetComponent);
                    return true;
                }

                if (!navCanSetHome()) {
                    mavlinkSendCommandAck(command, MAV_RESULT_DENIED, ackTargetSystem, ackTargetComponent);
                    return true;
                }

                navWaypoint_t wp = {0};
                wp.action = NAV_WP_ACTION_WAYPOINT;
                if (param1 == 1.0f) {
                    wp.lat = gpsSol.llh.lat;
                    wp.lon = gpsSol.llh.lon;
                    wp.alt = gpsSol.llh.alt - posControl.gpsOrigin.alt;
                } else {
                    if (!isfinite(latitudeDeg) || latitudeDeg < -90.0f || latitudeDeg > 90.0f ||
                        !isfinite(longitudeDeg) || longitudeDeg < -180.0f || longitudeDeg > 180.0f ||
                        !isfinite(altitudeMeters) ||
                        altitudeMeters < (float)INT32_MIN / 100.0f ||
                        altitudeMeters > (float)INT32_MAX / 100.0f) {
                        mavlinkSendCommandAck(command, MAV_RESULT_FAILED, ackTargetSystem, ackTargetComponent);
                        return true;
                    }
                    wp.lat = (int32_t)lrintf(latitudeDeg * 1e7f);
                    wp.lon = (int32_t)lrintf(longitudeDeg * 1e7f);
                    wp.alt = (int32_t)lrintf(altitudeMeters * 100.0f);
                    if (mavlinkFrameUsesAbsoluteAltitude(frame)) {
                        wp.alt -= posControl.gpsOrigin.alt;
                    }
                }
                setWaypoint(0, &wp);
                mavlinkSendCommandAck(command, MAV_RESULT_ACCEPTED, ackTargetSystem, ackTargetComponent);
                return true;
            }
        case MAV_CMD_DO_REPOSITION:
            if (!mavlinkFrameIsSupported(frame,
                MAV_FRAME_SUPPORTED_GLOBAL |
                MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT |
                MAV_FRAME_SUPPORTED_GLOBAL_INT |
                MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT)) {
                mavlinkSendCommandAck(command, MAV_RESULT_UNSUPPORTED, ackTargetSystem, ackTargetComponent);
                return true;
            }

            if (!isfinite(latitudeDeg) || latitudeDeg < -90.0f || latitudeDeg > 90.0f ||
                !isfinite(longitudeDeg) || longitudeDeg < -180.0f || longitudeDeg > 180.0f ||
                !isfinite(altitudeMeters) ||
                altitudeMeters < (float)INT32_MIN / 100.0f ||
                altitudeMeters > (float)INT32_MAX / 100.0f) {
                mavlinkSendCommandAck(command, MAV_RESULT_FAILED, ackTargetSystem, ackTargetComponent);
                return true;
            }

            if (isGCSValid()) {
                if (isfinite(param3)) {
                    const float maxLoiterRadiusMeters = (float)(UINT32_MAX / 100U);

                    if (param3 < 0.0f || param3 > maxLoiterRadiusMeters) {
                        mavlinkSendCommandAck(command, MAV_RESULT_DENIED, ackTargetSystem, ackTargetComponent);
                        return true;
                    }
                    navigationSetLoiterRadiusOverride((uint32_t)lrintf(METERS_TO_CENTIMETERS(param3)));
                }

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
                uint32_t frameValue;
                if (!isfinite(param1) || !mavlinkCommandParamToUint32(param2, UINT8_MAX, &frameValue)) {
                    mavlinkSendCommandAck(command, MAV_RESULT_DENIED, ackTargetSystem, ackTargetComponent);
                    return true;
                }
                const MAV_RESULT result = mavlinkSetAltitudeTargetFromFrame((uint8_t)frameValue, param1);
                mavlinkSendCommandAck(command, result, ackTargetSystem, ackTargetComponent);
                return true;
            }
        case MAV_CMD_CONDITION_YAW:
            {
                if (!isfinite(param1) || fabsf(param1) > 360.0f || !isfinite(param3) || !isfinite(param4)) {
                    mavlinkSendCommandAck(command, MAV_RESULT_DENIED, ackTargetSystem, ackTargetComponent);
                    return true;
                }
                const navigationFSMStateFlags_t navStateFlags = navGetCurrentStateFlags();
                if (!(navStateFlags & NAV_CTL_YAW) || (navStateFlags & NAV_MIXERAT)) {
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
                uint32_t messageId;

                if (!mavlinkCommandParamToUint32(param1, UINT16_MAX, &messageId) || !isfinite(param2)) {
                    mavlinkSendCommandAck(command, MAV_RESULT_DENIED, ackTargetSystem, ackTargetComponent);
                    return true;
                }

                if (mavlinkPeriodicMessageFromMessageId((uint16_t)messageId, &periodicMessage)) {
                    if (param2 == 0.0f) {
                        mavlinkSetMessageOverrideIntervalUs(periodicMessage, 0);
                        result = MAV_RESULT_ACCEPTED;
                    } else if (param2 < 0.0f) {
                        mavlinkSetMessageOverrideIntervalUs(periodicMessage, -1);
                        result = MAV_RESULT_ACCEPTED;
                    } else if ((double)param2 <= INT32_MAX) {
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
                uint32_t messageId;
                if (!mavlinkCommandParamToUint32(param1, UINT16_MAX, &messageId) ||
                    !mavlinkPeriodicMessageFromMessageId((uint16_t)messageId, &periodicMessage)) {
                    mavlinkSendCommandAck(command, MAV_RESULT_UNSUPPORTED, ackTargetSystem, ackTargetComponent);
                    return true;
                }

                mavlink_msg_message_interval_pack(
                    mavSystemId,
                    mavComponentId,
                    &mavSendMsg,
                    (uint16_t)messageId,
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
            {
                uint32_t request;
                if (!mavlinkCommandParamToUint32(param1, 1, &request)) {
                    mavlinkSendCommandAck(command, MAV_RESULT_DENIED, ackTargetSystem, ackTargetComponent);
                } else if (mavlinkGetProtocolVersion() == 1) {
                    mavlinkSendCommandAck(command, MAV_RESULT_UNSUPPORTED, ackTargetSystem, ackTargetComponent);
                } else {
                    if (request == 1) {
                        mavlinkSendAutopilotVersion();
                    }
                    mavlinkSendCommandAck(command, MAV_RESULT_ACCEPTED, ackTargetSystem, ackTargetComponent);
                }
                return true;
            }
        case MAV_CMD_REQUEST_MESSAGE:
            {
                uint32_t messageId;
                if (!mavlinkCommandParamToUint32(param1, UINT16_MAX, &messageId)) {
                    mavlinkSendCommandAck(command, MAV_RESULT_DENIED, ackTargetSystem, ackTargetComponent);
                    return true;
                }

                bool sent;
                switch (messageId) {
#ifdef USE_MAVLINK_STANDARD_MODES
                    case MAVLINK_MSG_ID_AVAILABLE_MODES:
                        {
                            uint32_t modeIndex;
                            sent = mavlinkCommandParamToUint32(param2, UINT8_MAX, &modeIndex) &&
                                modeIndex > 0 &&
                                mavlinkSendAvailableModeForCurrentVehicle((uint8_t)modeIndex);
                            break;
                        }
                    case MAVLINK_MSG_ID_AVAILABLE_MODES_MONITOR:
                        mavlinkSendAvailableModesMonitor();
                        sent = true;
                        break;
#endif
                    case MAVLINK_MSG_ID_MESSAGE_INTERVAL:
                        {
                            uint32_t intervalMessageId;
                            mavlinkPeriodicMessage_e periodicMessage;
                            sent = mavlinkCommandParamToUint32(param2, UINT16_MAX, &intervalMessageId) &&
                                mavlinkPeriodicMessageFromMessageId((uint16_t)intervalMessageId, &periodicMessage);
                            if (sent) {
                                mavlink_msg_message_interval_pack(
                                    mavSystemId,
                                    mavComponentId,
                                    &mavSendMsg,
                                    (uint16_t)intervalMessageId,
                                    mavlinkMessageIntervalUs(periodicMessage));
                                mavlinkSendMessage();
                            }
                            break;
                        }
                    default:
                        sent = mavlinkSendRequestedMessage((uint16_t)messageId);
                        break;
                }
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
