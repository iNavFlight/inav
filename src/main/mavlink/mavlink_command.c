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
    UNUSED(param3);

    if (!mavlinkIsLocalTarget(targetSystem, targetComponent)) {
        return false;
    }

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

            if (!isfinite(latitudeDeg) || latitudeDeg < -90.0f || latitudeDeg > 90.0f ||
                !isfinite(longitudeDeg) || longitudeDeg < -180.0f || longitudeDeg > 180.0f ||
                !isfinite(altitudeMeters) ||
                altitudeMeters < (float)INT32_MIN / 100.0f ||
                altitudeMeters > (float)INT32_MAX / 100.0f) {
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
                uint32_t frameValue;
                if (!isfinite(param1) || !mavlinkCommandParamToUint32(param2, UINT8_MAX, &frameValue)) {
                    mavlinkSendCommandAck(command, MAV_RESULT_DENIED, ackTargetSystem, ackTargetComponent);
                    return true;
                }
                const MAV_RESULT result = mavlinkSetAltitudeTargetFromFrame((uint8_t)frameValue, param1);
                mavlinkSendCommandAck(command, result, ackTargetSystem, ackTargetComponent);
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
