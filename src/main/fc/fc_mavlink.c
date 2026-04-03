#include "mavlink/mavlink_internal.h"

#include "fc/fc_mavlink.h"

#include "mavlink/mavlink_runtime.h"
#include "mavlink/mavlink_streams.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_MAVLINK)

static uint8_t mavlinkGetVehicleType(void)
{
    switch (mixerConfig()->platformType)
    {
        case PLATFORM_MULTIROTOR:
            return MAV_TYPE_QUADROTOR;
        case PLATFORM_TRICOPTER:
            return MAV_TYPE_TRICOPTER;
        case PLATFORM_AIRPLANE:
            return MAV_TYPE_FIXED_WING;
        case PLATFORM_ROVER:
            return MAV_TYPE_GROUND_ROVER;
        case PLATFORM_BOAT:
            return MAV_TYPE_SURFACE_BOAT;
        case PLATFORM_HELICOPTER:
            return MAV_TYPE_HELICOPTER;
        default:
            return MAV_TYPE_GENERIC;
    }
}

static APM_COPTER_MODE inavToArduCopterMap(flightModeForTelemetry_e flightMode)
{
    switch (flightMode)
    {
        case FLM_ACRO:          return COPTER_MODE_ACRO;
        case FLM_ACRO_AIR:      return COPTER_MODE_ACRO;
        case FLM_ANGLE:         return COPTER_MODE_STABILIZE;
        case FLM_HORIZON:       return COPTER_MODE_STABILIZE;
        case FLM_ANGLEHOLD:     return COPTER_MODE_STABILIZE;
        case FLM_ALTITUDE_HOLD: return COPTER_MODE_ALT_HOLD;
        case FLM_POSITION_HOLD: 
            {
                if (isGCSValid()) {
                    return COPTER_MODE_GUIDED;
                } else {
                    return COPTER_MODE_POSHOLD;
                }
            }
        case FLM_RTH:           return COPTER_MODE_RTL;
        case FLM_MISSION:       return COPTER_MODE_AUTO;
        case FLM_LAUNCH:        return COPTER_MODE_THROW;
        case FLM_FAILSAFE:
            {
                if (failsafePhase() == FAILSAFE_RETURN_TO_HOME) {
                    return COPTER_MODE_RTL;
                } else if (failsafePhase() == FAILSAFE_LANDING) {
                    return COPTER_MODE_LAND;
                } else {
                    return COPTER_MODE_RTL;
                }
            }
        default:                return COPTER_MODE_STABILIZE;
    }
}

static APM_PLANE_MODE inavToArduPlaneMap(flightModeForTelemetry_e flightMode)
{
    switch (flightMode)
    {
        case FLM_MANUAL:        return PLANE_MODE_MANUAL;
        case FLM_ACRO:          return PLANE_MODE_ACRO;
        case FLM_ACRO_AIR:      return PLANE_MODE_ACRO;
        case FLM_ANGLE:         return PLANE_MODE_FLY_BY_WIRE_A;
        case FLM_HORIZON:       return PLANE_MODE_STABILIZE;
        case FLM_ANGLEHOLD:     return PLANE_MODE_STABILIZE;
        case FLM_ALTITUDE_HOLD: return PLANE_MODE_FLY_BY_WIRE_B;
        case FLM_POSITION_HOLD: 
            {
                if (isGCSValid()) {
                    return PLANE_MODE_GUIDED;
                } else {
                    return PLANE_MODE_LOITER;
                }
            }
        case FLM_RTH:           return PLANE_MODE_RTL;
        case FLM_MISSION:       return PLANE_MODE_AUTO;
        case FLM_CRUISE:        return PLANE_MODE_CRUISE;
        case FLM_LAUNCH:        return PLANE_MODE_TAKEOFF;
        case FLM_FAILSAFE: //failsafePhase_e
            {
                if (failsafePhase() == FAILSAFE_RETURN_TO_HOME) {
                    return PLANE_MODE_RTL;
                }
                else if (failsafePhase() == FAILSAFE_LANDING) {
                    return PLANE_MODE_AUTOLAND;
                }
                else {
                    return PLANE_MODE_RTL;
                }
            }
        default:                return PLANE_MODE_MANUAL;
    }
}

typedef struct mavlinkModeSelection_s {
    flightModeForTelemetry_e flightMode;
    uint8_t customMode;
} mavlinkModeSelection_t;

static mavlinkModeSelection_t selectMavlinkMode(bool isPlane)
{
    mavlinkModeSelection_t modeSelection;
    modeSelection.flightMode = getFlightModeForTelemetry();

    if (isPlane) {
        modeSelection.customMode = (uint8_t)inavToArduPlaneMap(modeSelection.flightMode);
    } else {
        modeSelection.customMode = (uint8_t)inavToArduCopterMap(modeSelection.flightMode);
    }

    return modeSelection;
}

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

static void mavlinkSendAutopilotVersion(void)
{
    if (mavlinkGetProtocolVersion() == 1) return;

    // Capabilities aligned with what we actually support.
    uint64_t capabilities = 0;
    capabilities |= MAV_PROTOCOL_CAPABILITY_MAVLINK2;
    capabilities |= MAV_PROTOCOL_CAPABILITY_MISSION_FLOAT;
    capabilities |= MAV_PROTOCOL_CAPABILITY_MISSION_INT;
    capabilities |= MAV_PROTOCOL_CAPABILITY_COMMAND_INT;
    capabilities |= MAV_PROTOCOL_CAPABILITY_SET_POSITION_TARGET_LOCAL_NED;
    capabilities |= MAV_PROTOCOL_CAPABILITY_SET_POSITION_TARGET_GLOBAL_INT;

    const uint32_t flightSwVersion =
        ((uint32_t)ARDUPILOT_VERSION_MAJOR << 24) |
        ((uint32_t)ARDUPILOT_VERSION_MINOR << 16) |
        ((uint32_t)ARDUPILOT_VERSION_PATCH << 8);

    // Bare minimum: caps + IDs. Everything else 0 is fine.
    mavlink_msg_autopilot_version_pack(
        mavSystemId, 
        mavComponentId, 
        &mavSendMsg,
        capabilities,                // capabilities
        flightSwVersion,             // flight_sw_version
        0,                           // middleware_sw_version
        0,                           // os_sw_version
        0,                           // board_version
        0ULL,                        // flight_custom_version
        0ULL,                        // middleware_custom_version
        0ULL,                        // os_custom_version
        0ULL,                        // vendor_id
        0ULL,                        // product_id
        (uint64_t)mavSystemId,       // uid (any stable nonzero is fine)
        0ULL                         // uid2
    );
    mavlinkSendMessage();
}

static void mavlinkSendProtocolVersion(void)
{
    if (mavlinkGetProtocolVersion() == 1) return;

    uint8_t specHash[8] = {0};
    uint8_t libHash[8] = {0};
    const uint16_t protocolVersion = (uint16_t)mavlinkGetProtocolVersion() * 100;

    mavlink_msg_protocol_version_pack(
        mavSystemId,
        mavComponentId,
        &mavSendMsg,
        protocolVersion,
        protocolVersion,
        protocolVersion,
        specHash,
        libHash);

    mavlinkSendMessage();
}

static bool mavlinkFrameIsSupported(uint8_t frame, mavFrameSupportMask_e allowedMask)
{
    switch (frame) {
        case MAV_FRAME_GLOBAL:
            return allowedMask & MAV_FRAME_SUPPORTED_GLOBAL;
        case MAV_FRAME_GLOBAL_INT:
            return allowedMask & MAV_FRAME_SUPPORTED_GLOBAL_INT;
        case MAV_FRAME_GLOBAL_RELATIVE_ALT:
            return allowedMask & MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT;
        case MAV_FRAME_GLOBAL_RELATIVE_ALT_INT:
            return allowedMask & MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT;
        default:
            return false;
    }
}

static bool mavlinkFrameUsesAbsoluteAltitude(uint8_t frame)
{
    return frame == MAV_FRAME_GLOBAL || frame == MAV_FRAME_GLOBAL_INT;
}

static MAV_RESULT mavlinkSetAltitudeTargetFromFrame(uint8_t frame, float altitudeMeters)
{
#ifdef USE_BARO
    geoAltitudeDatumFlag_e datum;

    switch (frame) {
    case MAV_FRAME_GLOBAL:
    case MAV_FRAME_GLOBAL_INT:
        datum = NAV_WP_MSL_DATUM;
        break;
    case MAV_FRAME_GLOBAL_RELATIVE_ALT:
    case MAV_FRAME_GLOBAL_RELATIVE_ALT_INT:
        datum = NAV_WP_TAKEOFF_DATUM;
        break;
    default:
        return MAV_RESULT_UNSUPPORTED;
    }

    const int32_t targetAltitudeCm = (int32_t)lrintf(altitudeMeters * 100.0f);
    return navigationSetAltitudeTargetWithDatum(datum, targetAltitudeCm) ? MAV_RESULT_ACCEPTED : MAV_RESULT_DENIED;
#else
    UNUSED(frame);
    UNUSED(altitudeMeters);
    return MAV_RESULT_UNSUPPORTED;
#endif
}

static MAV_MISSION_RESULT mavlinkMissionResultFromCommandResult(MAV_RESULT result)
{
    switch (result) {
    case MAV_RESULT_ACCEPTED:
        return MAV_MISSION_ACCEPTED;
    case MAV_RESULT_UNSUPPORTED:
        return MAV_MISSION_UNSUPPORTED;
    default:
        return MAV_MISSION_ERROR;
    }
}

static bool mavlinkHandleArmedGuidedMissionItem(uint8_t current, uint8_t frame, mavFrameSupportMask_e allowedFrames, int32_t latitudeE7, int32_t longitudeE7, float altitudeMeters)
{
    if (!isGCSValid()) {
        mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg,
            mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid,
            MAV_MISSION_ERROR, MAV_MISSION_TYPE_MISSION, 0);
        mavlinkSendMessage();
        return true;
    }

    if (!mavlinkFrameIsSupported(frame, allowedFrames)) {
        mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg,
            mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid,
            MAV_MISSION_UNSUPPORTED_FRAME, MAV_MISSION_TYPE_MISSION, 0);
        mavlinkSendMessage();
        return true;
    }

    if (current == 2) {
        navWaypoint_t wp = {0};
        wp.action = NAV_WP_ACTION_WAYPOINT;
        wp.lat = latitudeE7;
        wp.lon = longitudeE7;
        wp.alt = (int32_t)lrintf(altitudeMeters * 100.0f);
        wp.p3 = mavlinkFrameUsesAbsoluteAltitude(frame) ? NAV_WP_ALTMODE : 0;

        setWaypoint(255, &wp);

        mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg,
            mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid,
            MAV_MISSION_ACCEPTED, MAV_MISSION_TYPE_MISSION, 0);
        mavlinkSendMessage();
        return true;
    }

    if (current == 3) {
        const MAV_RESULT result = mavlinkSetAltitudeTargetFromFrame(frame, altitudeMeters);
        mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg,
            mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid,
            mavlinkMissionResultFromCommandResult(result), MAV_MISSION_TYPE_MISSION, 0);
        mavlinkSendMessage();
        return true;
    }

    mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg,
        mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid,
        MAV_MISSION_ERROR, MAV_MISSION_TYPE_MISSION, 0);
    mavlinkSendMessage();
    return true;
}

static uint8_t navWaypointFrame(const navWaypoint_t *wp, bool useIntMessages)
{
    switch (wp->action) {
        case NAV_WP_ACTION_RTH:
        case NAV_WP_ACTION_JUMP:
        case NAV_WP_ACTION_SET_HEAD:
            return MAV_FRAME_MISSION;
        default:
            break;
    }

    const bool absoluteAltitude = (wp->p3 & NAV_WP_ALTMODE) == NAV_WP_ALTMODE;

    if (absoluteAltitude) {
        return useIntMessages ? MAV_FRAME_GLOBAL_INT : MAV_FRAME_GLOBAL;
    }

    return useIntMessages ? MAV_FRAME_GLOBAL_RELATIVE_ALT_INT : MAV_FRAME_GLOBAL_RELATIVE_ALT;
}

const mavlinkModeDescriptor_t planeModes[] = {
    { PLANE_MODE_MANUAL,       "MANUAL" },
    { PLANE_MODE_ACRO,         "ACRO" },
    { PLANE_MODE_STABILIZE,    "STABILIZE" },
    { PLANE_MODE_FLY_BY_WIRE_A,"FBWA" },
    { PLANE_MODE_FLY_BY_WIRE_B,"FBWB" },
    { PLANE_MODE_CRUISE,       "CRUISE" },
    { PLANE_MODE_AUTO,         "AUTO" },
    { PLANE_MODE_RTL,          "RTL" },
    { PLANE_MODE_LOITER,       "LOITER" },
    { PLANE_MODE_TAKEOFF,      "TAKEOFF" },
    { PLANE_MODE_GUIDED,       "GUIDED" },
};
const uint8_t planeModesCount = (uint8_t)ARRAYLEN(planeModes);

const mavlinkModeDescriptor_t copterModes[] = {
    { COPTER_MODE_ACRO,       "ACRO" },
    { COPTER_MODE_STABILIZE,  "STABILIZE" },
    { COPTER_MODE_ALT_HOLD,   "ALT_HOLD" },
    { COPTER_MODE_POSHOLD,    "POSHOLD" },
    { COPTER_MODE_LOITER,     "LOITER" },
    { COPTER_MODE_AUTO,       "AUTO" },
    { COPTER_MODE_GUIDED,     "GUIDED" },
    { COPTER_MODE_RTL,        "RTL" },
    { COPTER_MODE_LAND,       "LAND" },
    { COPTER_MODE_BRAKE,      "BRAKE" },
    { COPTER_MODE_THROW,      "THROW" },
    { COPTER_MODE_SMART_RTL,  "SMART_RTL" },
    { COPTER_MODE_DRIFT,      "DRIFT" },
};
const uint8_t copterModesCount = (uint8_t)ARRAYLEN(copterModes);

static bool mavlinkPlaneModeIsConfigured(uint8_t customMode)
{
    switch ((APM_PLANE_MODE)customMode) {
        case PLANE_MODE_MANUAL:
            return isModeActivationConditionPresent(BOXMANUAL);
        case PLANE_MODE_ACRO:
            return true;
        case PLANE_MODE_STABILIZE:
            return isModeActivationConditionPresent(BOXHORIZON) ||
                isModeActivationConditionPresent(BOXANGLEHOLD);
        case PLANE_MODE_FLY_BY_WIRE_A:
            return isModeActivationConditionPresent(BOXANGLE);
        case PLANE_MODE_FLY_BY_WIRE_B:
            return isModeActivationConditionPresent(BOXNAVALTHOLD);
        case PLANE_MODE_CRUISE:
            return isModeActivationConditionPresent(BOXNAVCRUISE) ||
                (isModeActivationConditionPresent(BOXNAVCOURSEHOLD) &&
                isModeActivationConditionPresent(BOXNAVALTHOLD));
        case PLANE_MODE_AUTO:
            return isModeActivationConditionPresent(BOXNAVWP);
        case PLANE_MODE_RTL:
            return isModeActivationConditionPresent(BOXNAVRTH);
        case PLANE_MODE_GUIDED:
            return isModeActivationConditionPresent(BOXNAVPOSHOLD) &&
                isModeActivationConditionPresent(BOXGCSNAV);
        case PLANE_MODE_LOITER:
            return isModeActivationConditionPresent(BOXNAVPOSHOLD);
        case PLANE_MODE_TAKEOFF:
            return isModeActivationConditionPresent(BOXNAVLAUNCH);
        default:
            return false;
    }
}

static bool mavlinkCopterModeIsConfigured(uint8_t customMode)
{
    switch ((APM_COPTER_MODE)customMode) {
        case COPTER_MODE_ACRO:
            return true;
        case COPTER_MODE_STABILIZE:
            return isModeActivationConditionPresent(BOXANGLE) ||
                isModeActivationConditionPresent(BOXHORIZON) ||
                isModeActivationConditionPresent(BOXANGLEHOLD);
        case COPTER_MODE_ALT_HOLD:
            return isModeActivationConditionPresent(BOXNAVALTHOLD);
        case COPTER_MODE_GUIDED:
            return isModeActivationConditionPresent(BOXNAVPOSHOLD) &&
                isModeActivationConditionPresent(BOXGCSNAV);
        case COPTER_MODE_POSHOLD:
            return isModeActivationConditionPresent(BOXNAVPOSHOLD);
        case COPTER_MODE_RTL:
            return isModeActivationConditionPresent(BOXNAVRTH);
        case COPTER_MODE_AUTO:
            return isModeActivationConditionPresent(BOXNAVWP);
        case COPTER_MODE_THROW:
            return isModeActivationConditionPresent(BOXNAVLAUNCH);
        case COPTER_MODE_BRAKE:
            return isModeActivationConditionPresent(BOXBRAKING);
        default:
            return false;
    }
}

static void mavlinkSendAvailableModes(const mavlinkModeDescriptor_t *modes, uint8_t count, uint8_t currentCustom,
    bool (*isModeConfigured)(uint8_t customMode))
{
    uint8_t availableCount = 0;
    for (uint8_t i = 0; i < count; i++) {
        if (isModeConfigured(modes[i].customMode)) {
            availableCount++;
        }
    }

    if (availableCount == 0) {
        return;
    }

    uint8_t modeIndex = 0;
    for (uint8_t i = 0; i < count; i++) {
        if (!isModeConfigured(modes[i].customMode)) {
            continue;
        }

        modeIndex++;
        const uint8_t stdMode = MAV_STANDARD_MODE_NON_STANDARD;
        const uint32_t properties = 0;

        mavlink_msg_available_modes_pack(
            mavSystemId,
            mavComponentId,
            &mavSendMsg,
            availableCount,
            modeIndex,
            stdMode,
            modes[i].customMode,
            properties,
            modes[i].name);

        mavlinkSendMessage();

        if (modes[i].customMode == currentCustom) {
            mavlink_msg_current_mode_pack(
                mavSystemId,
                mavComponentId,
                &mavSendMsg,
                stdMode,
                currentCustom,
                currentCustom);
            mavlinkSendMessage();
        }
    }
}

void mavlinkSendExtendedSysState(void)
{
    uint8_t vtolState = MAV_VTOL_STATE_UNDEFINED;
    uint8_t landedState;

    switch (NAV_Status.state) {
        case MW_NAV_STATE_LAND_START:
        case MW_NAV_STATE_LAND_IN_PROGRESS:
        case MW_NAV_STATE_LAND_SETTLE:
        case MW_NAV_STATE_LAND_START_DESCENT:
        case MW_NAV_STATE_EMERGENCY_LANDING:
            landedState = MAV_LANDED_STATE_LANDING;
            break;
        case MW_NAV_STATE_LANDED:
            landedState = MAV_LANDED_STATE_ON_GROUND;
            break;
        default:
            if (!ARMING_FLAG(ARMED) || STATE(LANDING_DETECTED)) {
                landedState = MAV_LANDED_STATE_ON_GROUND;
            } else {
                landedState = MAV_LANDED_STATE_IN_AIR;
            }
            break;
    }

    mavlink_msg_extended_sys_state_pack(
        mavSystemId,
        mavComponentId,
        &mavSendMsg,
        vtolState,
        landedState);

    mavlinkSendMessage();
}

void mavlinkSendSystemStatus(void)
{
    // Receiver is assumed to be always present
    uint32_t onboard_control_sensors_present    = (MAV_SYS_STATUS_SENSOR_RC_RECEIVER);
    // GYRO and RC are assumed as minimum requirements
    uint32_t onboard_control_sensors_enabled    = (MAV_SYS_STATUS_SENSOR_3D_GYRO | MAV_SYS_STATUS_SENSOR_RC_RECEIVER);
    uint32_t onboard_control_sensors_health     = 0;

    if (getHwGyroStatus() == HW_SENSOR_OK) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_3D_GYRO;
        // Missing presence will report as sensor unhealthy
        onboard_control_sensors_health |= MAV_SYS_STATUS_SENSOR_3D_GYRO;
    }

    hardwareSensorStatus_e accStatus = getHwAccelerometerStatus();
    if (accStatus == HW_SENSOR_OK) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_3D_ACCEL;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_3D_ACCEL;
        onboard_control_sensors_health |= MAV_SYS_STATUS_SENSOR_3D_ACCEL;
    } else if (accStatus == HW_SENSOR_UNHEALTHY) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_3D_ACCEL;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_3D_ACCEL;
    } else if (accStatus == HW_SENSOR_UNAVAILABLE) {
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_3D_ACCEL;
    }

    hardwareSensorStatus_e compassStatus = getHwCompassStatus();
    if (compassStatus == HW_SENSOR_OK) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_3D_MAG;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_3D_MAG;
        onboard_control_sensors_health |= MAV_SYS_STATUS_SENSOR_3D_MAG;
    } else if (compassStatus == HW_SENSOR_UNHEALTHY) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_3D_MAG;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_3D_MAG;
    } else if (compassStatus == HW_SENSOR_UNAVAILABLE) {
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_3D_MAG;
    }

    hardwareSensorStatus_e baroStatus = getHwBarometerStatus();
    if (baroStatus == HW_SENSOR_OK) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE;
        onboard_control_sensors_health |= MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE;
    } else if (baroStatus == HW_SENSOR_UNHEALTHY) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE;
    } else if (baroStatus == HW_SENSOR_UNAVAILABLE) {
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE;
    }

    hardwareSensorStatus_e pitotStatus = getHwPitotmeterStatus();
    if (pitotStatus == HW_SENSOR_OK) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE;
        onboard_control_sensors_health |= MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE;
    } else if (pitotStatus == HW_SENSOR_UNHEALTHY) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE;
    } else if (pitotStatus == HW_SENSOR_UNAVAILABLE) {
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE;
    }

    hardwareSensorStatus_e gpsStatus = getHwGPSStatus();
    if (gpsStatus == HW_SENSOR_OK) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_GPS;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_GPS;
        onboard_control_sensors_health |= MAV_SYS_STATUS_SENSOR_GPS;
    } else if (gpsStatus == HW_SENSOR_UNHEALTHY) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_GPS;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_GPS;
    } else if (gpsStatus == HW_SENSOR_UNAVAILABLE) {
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_GPS;
    }

    hardwareSensorStatus_e opFlowStatus = getHwOpticalFlowStatus();
    if (opFlowStatus == HW_SENSOR_OK) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW;
        onboard_control_sensors_health |= MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW;
    } else if (opFlowStatus == HW_SENSOR_UNHEALTHY) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW;
    } else if (opFlowStatus == HW_SENSOR_UNAVAILABLE) {
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW;
    }

    hardwareSensorStatus_e rangefinderStatus = getHwRangefinderStatus();
    if (rangefinderStatus == HW_SENSOR_OK) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_LASER_POSITION;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_LASER_POSITION;
        onboard_control_sensors_health |= MAV_SYS_STATUS_SENSOR_LASER_POSITION;
    } else if (rangefinderStatus == HW_SENSOR_UNHEALTHY) {
        onboard_control_sensors_present |= MAV_SYS_STATUS_SENSOR_LASER_POSITION;
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_LASER_POSITION;
    } else if (rangefinderStatus == HW_SENSOR_UNAVAILABLE) {
        onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_LASER_POSITION;
    }

    if (rxIsReceivingSignal() && rxAreFlightChannelsValid()) {
        onboard_control_sensors_health |= MAV_SYS_STATUS_SENSOR_RC_RECEIVER;
    }

#ifdef USE_BLACKBOX
    // BLACKBOX is assumed enabled and present for boards with capability
    onboard_control_sensors_present |= MAV_SYS_STATUS_LOGGING;
    onboard_control_sensors_enabled |= MAV_SYS_STATUS_LOGGING;
    // Unhealthy only for cases with not enough space to record
    if (!isBlackboxDeviceFull()) {
        onboard_control_sensors_health |= MAV_SYS_STATUS_LOGGING;
    }
#endif

    mavlink_msg_sys_status_pack(mavSystemId, mavComponentId, &mavSendMsg,
        // onboard_control_sensors_present Bitmask showing which onboard controllers and sensors are present.
        //Value of 0: not present. Value of 1: present. Indices according MAV_SYS_STATUS_SENSOR
        onboard_control_sensors_present,
        // onboard_control_sensors_enabled Bitmask showing which onboard controllers and sensors are enabled
        onboard_control_sensors_enabled,
        // onboard_control_sensors_health Bitmask showing which onboard controllers and sensors are operational or have an error.
        onboard_control_sensors_health,
        // load Maximum usage in percent of the mainloop time, (0%: 0, 100%: 1000) should be always below 1000
        constrain(averageSystemLoadPercent*10, 0, 1000),
        // voltage_battery Battery voltage, in millivolts (1 = 1 millivolt)
        feature(FEATURE_VBAT) ? getBatteryVoltage() * 10 : 0,
        // current_battery Battery current, in 10*milliamperes (1 = 10 milliampere), -1: autopilot does not measure the current
        isAmperageConfigured() ? getAmperage() : -1,
        // battery_remaining Remaining battery energy: (0%: 0, 100%: 100), -1: autopilot estimate the remaining battery
        feature(FEATURE_VBAT) ? calculateBatteryPercentage() : 100,
        // drop_rate_comm Communication drops in percent, (0%: 0, 100%: 10'000), (UART, I2C, SPI, CAN), dropped packets on all links (packets that were corrupted on reception on the MAV)
        0,
        // errors_comm Communication errors (UART, I2C, SPI, CAN), dropped packets on all links (packets that were corrupted on reception on the MAV)
        0,
        // errors_count1 Autopilot-specific errors
        0,
        // errors_count2 Autopilot-specific errors
        0,
        // errors_count3 Autopilot-specific errors
        0,
        // errors_count4 Autopilot-specific errors
        0, 0, 0, 0);

    mavlinkSendMessage();
}

void mavlinkSendRCChannelsAndRSSI(void)
{
#define GET_CHANNEL_VALUE(x) ((rxRuntimeConfig.channelCount >= (x + 1)) ? rxGetChannelValue(x) : 0)
    if (mavlinkGetProtocolVersion() == 1) {
        mavlink_msg_rc_channels_raw_pack(mavSystemId, mavComponentId, &mavSendMsg,
            // time_boot_ms Timestamp (milliseconds since system boot)
            millis(),
            // port Servo output port (set of 8 outputs = 1 port). Most MAVs will just use one, but this allows to encode more than 8 servos.
            0,
            // chan1_raw RC channel 1 value, in microseconds
            GET_CHANNEL_VALUE(0),
            // chan2_raw RC channel 2 value, in microseconds
            GET_CHANNEL_VALUE(1),
            // chan3_raw RC channel 3 value, in microseconds
            GET_CHANNEL_VALUE(2),
            // chan4_raw RC channel 4 value, in microseconds
            GET_CHANNEL_VALUE(3),
            // chan5_raw RC channel 5 value, in microseconds
            GET_CHANNEL_VALUE(4),
            // chan6_raw RC channel 6 value, in microseconds
            GET_CHANNEL_VALUE(5),
            // chan7_raw RC channel 7 value, in microseconds
            GET_CHANNEL_VALUE(6),
            // chan8_raw RC channel 8 value, in microseconds
            GET_CHANNEL_VALUE(7),
            // rssi Receive signal strength indicator, 0: 0%, 254: 100%
    		//https://github.com/mavlink/mavlink/issues/1027
            scaleRange(getRSSI(), 0, 1023, 0, 254));
	} 
    else {
        mavlink_msg_rc_channels_pack(mavSystemId, mavComponentId, &mavSendMsg,
            // time_boot_ms Timestamp (milliseconds since system boot)
            millis(),
            // Total number of RC channels being received. 
            rxRuntimeConfig.channelCount,
            // chan1_raw RC channel 1 value, in microseconds
            GET_CHANNEL_VALUE(0),
            // chan2_raw RC channel 2 value, in microseconds
            GET_CHANNEL_VALUE(1),
            // chan3_raw RC channel 3 value, in microseconds
            GET_CHANNEL_VALUE(2),
            // chan4_raw RC channel 4 value, in microseconds
            GET_CHANNEL_VALUE(3),
            // chan5_raw RC channel 5 value, in microseconds
            GET_CHANNEL_VALUE(4),
            // chan6_raw RC channel 6 value, in microseconds
            GET_CHANNEL_VALUE(5),
            // chan7_raw RC channel 7 value, in microseconds
            GET_CHANNEL_VALUE(6),
            // chan8_raw RC channel 8 value, in microseconds
            GET_CHANNEL_VALUE(7),
            // chan9_raw RC channel 9 value, in microseconds
            GET_CHANNEL_VALUE(8),
            // chan10_raw RC channel 10 value, in microseconds
            GET_CHANNEL_VALUE(9),
            // chan11_raw RC channel 11 value, in microseconds
            GET_CHANNEL_VALUE(10),
            // chan12_raw RC channel 12 value, in microseconds
            GET_CHANNEL_VALUE(11),
            // chan13_raw RC channel 13 value, in microseconds
            GET_CHANNEL_VALUE(12),
            // chan14_raw RC channel 14 value, in microseconds
            GET_CHANNEL_VALUE(13),
            // chan15_raw RC channel 15 value, in microseconds
            GET_CHANNEL_VALUE(14),
            // chan16_raw RC channel 16 value, in microseconds
            GET_CHANNEL_VALUE(15),
            // chan17_raw RC channel 17 value, in microseconds
            GET_CHANNEL_VALUE(16),
            // chan18_raw RC channel 18 value, in microseconds
            GET_CHANNEL_VALUE(17),
            // rssi Receive signal strength indicator, 0: 0%, 254: 100%
    		//https://github.com/mavlink/mavlink/issues/1027
            scaleRange(getRSSI(), 0, 1023, 0, 254));
    }
#undef GET_CHANNEL_VALUE

    mavlinkSendMessage();
}

#if defined(USE_GPS)
static void mavlinkSendHomePosition(void)
{
    float q[4] = { 1.0f, 0.0f, 0.0f, 0.0f };

    mavlink_msg_home_position_pack(
        mavSystemId,
        mavComponentId,
        &mavSendMsg,
        GPS_home.lat,
        GPS_home.lon,
        GPS_home.alt * 10,
        0.0f,
        0.0f,
        0.0f,
        q,
        0.0f,
        0.0f,
        0.0f,
        ((uint64_t) millis()) * 1000);

    mavlinkSendMessage();
}

void mavlinkSendGpsRawInt(timeUs_t currentTimeUs)
{
    uint8_t gpsFixType = 0;

    if (!(sensors(SENSOR_GPS)
#ifdef USE_GPS_FIX_ESTIMATION
            || STATE(GPS_ESTIMATED_FIX)
#endif
        ))
        return;

    if (gpsSol.fixType == GPS_NO_FIX)
        gpsFixType = 1;
    else if (gpsSol.fixType == GPS_FIX_2D)
            gpsFixType = 2;
    else if (gpsSol.fixType == GPS_FIX_3D)
            gpsFixType = 3;

    mavlink_msg_gps_raw_int_pack(mavSystemId, mavComponentId, &mavSendMsg,
        // time_usec Timestamp (microseconds since UNIX epoch or microseconds since system boot)
        currentTimeUs,
        // fix_type 0-1: no fix, 2: 2D fix, 3: 3D fix. Some applications will not use the value of this field unless it is at least two, so always correctly fill in the fix.
        gpsFixType,
        // lat Latitude in 1E7 degrees
        gpsSol.llh.lat,
        // lon Longitude in 1E7 degrees
        gpsSol.llh.lon,
        // alt Altitude in 1E3 meters (millimeters) above MSL
        gpsSol.llh.alt * 10,
        // eph GPS HDOP horizontal dilution of position in cm (m*100). If unknown, set to: 65535
        gpsSol.eph,
        // epv GPS VDOP horizontal dilution of position in cm (m*100). If unknown, set to: 65535
        gpsSol.epv,
        // vel GPS ground speed (m/s * 100). If unknown, set to: 65535
        gpsSol.groundSpeed,
        // cog Course over ground (NOT heading, but direction of movement) in degrees * 100, 0.0..359.99 degrees. If unknown, set to: 65535
        gpsSol.groundCourse * 10,
        // satellites_visible Number of satellites visible. If unknown, set to 255
        gpsSol.numSat,
        // alt_ellipsoid Altitude (above WGS84, EGM96 ellipsoid). Positive for up
        0,
        // h_acc Position uncertainty in mm,
        gpsSol.eph * 10,
        // v_acc Altitude uncertainty in mm,
        gpsSol.epv * 10,
        // vel_acc Speed uncertainty in mm (??)
        0,
        // hdg_acc Heading uncertainty in degE5
        0,
        // yaw Yaw in earth frame from north. Use 0 if this GPS does not provide yaw. Use 65535 if this GPS is configured to provide yaw and is currently unable to provide it. Use 36000 for north.
        0);

    mavlinkSendMessage();
}

void mavlinkSendGlobalPositionInt(timeUs_t currentTimeUs)
{
    if (!(sensors(SENSOR_GPS)
#ifdef USE_GPS_FIX_ESTIMATION
            || STATE(GPS_ESTIMATED_FIX)
#endif
        )) {
        return;
    }

    mavlink_msg_global_position_int_pack(mavSystemId, mavComponentId, &mavSendMsg,
        // time_boot_ms Timestamp (milliseconds since system boot)
        currentTimeUs / 1000,
        // lat Latitude in 1E7 degrees
        gpsSol.llh.lat,
        // lon Longitude in 1E7 degrees
        gpsSol.llh.lon,
        // alt Altitude in 1E3 meters (millimeters) above MSL
        gpsSol.llh.alt * 10,
        // relative_alt Altitude above ground in meters, expressed as * 1000 (millimeters)
        getEstimatedActualPosition(Z) * 10,
        // [cm/s] Ground X Speed (Latitude, positive north)
        getEstimatedActualVelocity(X),
        // [cm/s] Ground Y Speed (Longitude, positive east)
        getEstimatedActualVelocity(Y),
        // [cm/s] Ground Z Speed (Altitude, positive down)
        -getEstimatedActualVelocity(Z),
        // [cdeg] Vehicle heading (yaw angle) (0.0..359.99 degrees, 0=north)
        DECIDEGREES_TO_CENTIDEGREES(attitude.values.yaw)
    );

    mavlinkSendMessage();
}

void mavlinkSendGpsGlobalOrigin(void)
{
    mavlink_msg_gps_global_origin_pack(mavSystemId, mavComponentId, &mavSendMsg,
        // latitude Latitude (WGS84), expressed as * 1E7
        GPS_home.lat,
        // longitude Longitude (WGS84), expressed as * 1E7
        GPS_home.lon,
        // altitude Altitude(WGS84), expressed as * 1000
        GPS_home.alt * 10, // FIXME
        // time_usec Timestamp (microseconds since system boot)
        // Use millis() * 1000 as micros() will overflow after 1.19 hours.
        ((uint64_t) millis()) * 1000);

    mavlinkSendMessage();
}

void mavlinkSendPosition(timeUs_t currentTimeUs)
{
    mavlinkSendGpsRawInt(currentTimeUs);
    mavlinkSendGlobalPositionInt(currentTimeUs);
    mavlinkSendGpsGlobalOrigin();
}
#endif

void mavlinkSendAttitude(void)
{
    mavlink_msg_attitude_pack(mavSystemId, mavComponentId, &mavSendMsg,
        // time_boot_ms Timestamp (milliseconds since system boot)
        millis(),
        // roll Roll angle (rad)
        RADIANS_TO_MAVLINK_RANGE(DECIDEGREES_TO_RADIANS(attitude.values.roll)),
        // pitch Pitch angle (rad)
        RADIANS_TO_MAVLINK_RANGE(DECIDEGREES_TO_RADIANS(-attitude.values.pitch)),
        // yaw Yaw angle (rad)
        RADIANS_TO_MAVLINK_RANGE(DECIDEGREES_TO_RADIANS(attitude.values.yaw)),
        // rollspeed Roll angular speed (rad/s)
        DEGREES_TO_RADIANS(gyro.gyroADCf[FD_ROLL]),
        // pitchspeed Pitch angular speed (rad/s)
        DEGREES_TO_RADIANS(gyro.gyroADCf[FD_PITCH]),
        // yawspeed Yaw angular speed (rad/s)
        DEGREES_TO_RADIANS(gyro.gyroADCf[FD_YAW]));

    mavlinkSendMessage();
}

void mavlinkSendVfrHud(void)
{
    float mavAltitude = 0;
    float mavGroundSpeed = 0;
    float mavAirSpeed = 0;
    float mavClimbRate = 0;

#if defined(USE_GPS)
    // use ground speed if source available
    if (sensors(SENSOR_GPS)
#ifdef USE_GPS_FIX_ESTIMATION
            || STATE(GPS_ESTIMATED_FIX)
#endif
        ) {
        mavGroundSpeed = gpsSol.groundSpeed / 100.0f;
    }
#endif

#if defined(USE_PITOT)
    if (sensors(SENSOR_PITOT) && pitotIsHealthy()) {
        mavAirSpeed = getAirspeedEstimate() / 100.0f;
    }
#endif

    // select best source for altitude
    mavAltitude = getEstimatedActualPosition(Z) / 100.0f;
    mavClimbRate = getEstimatedActualVelocity(Z) / 100.0f;

    int16_t thr = getThrottlePercent(osdUsingScaledThrottle());
    mavlink_msg_vfr_hud_pack(mavSystemId, mavComponentId, &mavSendMsg,
        // airspeed Current airspeed in m/s
        mavAirSpeed,
        // groundspeed Current ground speed in m/s
        mavGroundSpeed,
        // heading Current heading in degrees, in compass units (0..360, 0=north)
        DECIDEGREES_TO_DEGREES(attitude.values.yaw),
        // throttle Current throttle setting in integer percent, 0 to 100
        thr,
        // alt Current altitude (MSL), in meters, if we have surface or baro use them, otherwise use GPS (less accurate)
        mavAltitude,
        // climb Current climb rate in meters/second
        mavClimbRate);

    mavlinkSendMessage();
}

static uint8_t mavlinkGetAutopilotEnum(void);

void mavlinkSendHeartbeat(void)
{
    uint8_t mavModes = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;

    const bool isPlane = (STATE(FIXED_WING_LEGACY) || mixerConfig()->platformType == PLATFORM_AIRPLANE);
    const mavlinkModeSelection_t modeSelection = selectMavlinkMode(isPlane);
    flightModeForTelemetry_e flm = modeSelection.flightMode;
    uint8_t mavCustomMode = modeSelection.customMode;
    uint8_t mavSystemType;
    if (isPlane) {
        mavSystemType = MAV_TYPE_FIXED_WING;
    }
    else {
        mavSystemType = mavlinkGetVehicleType();
    }

    const bool manualInputAllowed = !(flm == FLM_MISSION || flm == FLM_RTH || flm == FLM_FAILSAFE);
    if (manualInputAllowed) {
        mavModes |= MAV_MODE_FLAG_MANUAL_INPUT_ENABLED;
    }
    if (flm == FLM_POSITION_HOLD && isGCSValid()) {
        mavModes |= MAV_MODE_FLAG_GUIDED_ENABLED;
    }
    else if (flm == FLM_MISSION || flm == FLM_RTH ) {
        mavModes |= MAV_MODE_FLAG_AUTO_ENABLED;
    }
    else if (flm != FLM_MANUAL && flm!= FLM_ACRO && flm!=FLM_ACRO_AIR) {
        mavModes |= MAV_MODE_FLAG_STABILIZE_ENABLED;
    }

    if (ARMING_FLAG(ARMED))
        mavModes |= MAV_MODE_FLAG_SAFETY_ARMED;

    uint8_t mavSystemState = 0;
    if (ARMING_FLAG(ARMED)) {
        if (failsafeIsActive()) {
            mavSystemState = MAV_STATE_CRITICAL;
        }
        else {
            mavSystemState = MAV_STATE_ACTIVE;
        }
    }
    else if (areSensorsCalibrating()) {
        mavSystemState = MAV_STATE_CALIBRATING;
    }
    else {
        mavSystemState = MAV_STATE_STANDBY;
    }

    mavlink_msg_heartbeat_pack(mavSystemId, mavComponentId, &mavSendMsg,
        // type Type of the MAV (quadrotor, helicopter, etc., up to 15 types, defined in MAV_TYPE ENUM)
        mavSystemType,
        // autopilot Autopilot type / class. defined in MAV_AUTOPILOT ENUM
        mavlinkGetAutopilotEnum(),
        // base_mode System mode bitfield, see MAV_MODE_FLAGS ENUM in mavlink/include/mavlink_types.h
        mavModes,
        // custom_mode A bitfield for use for autopilot-specific flags.
        mavCustomMode,
        // system_status System status flag, see MAV_STATE ENUM
        mavSystemState);

    mavlinkSendMessage();
}

void mavlinkSendBatteryStatus(void)
{
    uint16_t batteryVoltages[MAVLINK_MSG_BATTERY_STATUS_FIELD_VOLTAGES_LEN];
    uint16_t batteryVoltagesExt[MAVLINK_MSG_BATTERY_STATUS_FIELD_VOLTAGES_EXT_LEN];
    memset(batteryVoltages, UINT16_MAX, sizeof(batteryVoltages));
    memset(batteryVoltagesExt, 0, sizeof(batteryVoltagesExt));
    if (feature(FEATURE_VBAT)) {
        uint8_t batteryCellCount = getBatteryCellCount();
        if (batteryCellCount > 0) {
            for (int cell=0; cell < batteryCellCount && cell < MAVLINK_MSG_BATTERY_STATUS_FIELD_VOLTAGES_LEN + MAVLINK_MSG_BATTERY_STATUS_FIELD_VOLTAGES_EXT_LEN; cell++) {
                if (cell < MAVLINK_MSG_BATTERY_STATUS_FIELD_VOLTAGES_LEN) {
                    batteryVoltages[cell] = getBatteryAverageCellVoltage() * 10;
                } else {
                    batteryVoltagesExt[cell-MAVLINK_MSG_BATTERY_STATUS_FIELD_VOLTAGES_LEN] = getBatteryAverageCellVoltage() * 10;
                }
            }
        }
        else {
            batteryVoltages[0] = getBatteryVoltage() * 10;
        }
    }
    else {
        batteryVoltages[0] = 0;
    }

    mavlink_msg_battery_status_pack(mavSystemId, mavComponentId, &mavSendMsg,
        // id Battery ID
        0,
        // battery_function Function of the battery
        MAV_BATTERY_FUNCTION_UNKNOWN,
        // type Type (chemistry) of the battery
        MAV_BATTERY_TYPE_UNKNOWN,
        // temperature Temperature of the battery in centi-degrees celsius. INT16_MAX for unknown temperature
        INT16_MAX,
        // voltages Battery voltage of cells, in millivolts (1 = 1 millivolt). Cells above the valid cell count for this battery should have the UINT16_MAX value.
        batteryVoltages,
        // current_battery Battery current, in 10*milliamperes (1 = 10 milliampere), -1: autopilot does not measure the current
        isAmperageConfigured() ? getAmperage() : -1,
        // current_consumed Consumed charge, in milliampere hours (1 = 1 mAh), -1: autopilot does not provide mAh consumption estimate
        isAmperageConfigured() ? getMAhDrawn() : -1,
        // energy_consumed Consumed energy, in 100*Joules (intergrated U*I*dt)  (1 = 100 Joule), -1: autopilot does not provide energy consumption estimate
        isAmperageConfigured() ? getMWhDrawn()*36 : -1,
        // battery_remaining Remaining battery energy: (0%: 0, 100%: 100), -1: autopilot does not estimate the remaining battery);
        feature(FEATURE_VBAT) ? calculateBatteryPercentage() : -1,
        // time_remaining Remaining battery time, 0: autopilot does not provide remaining battery time estimate
        0, // TODO this could easily be implemented
        // charge_state State for extent of discharge, provided by autopilot for warning or external reactions
        0,
        // voltages_ext Battery voltages for cells 11 to 14. Cells above the valid cell count for this battery should have a value of 0, where zero indicates not supported (note, this is different than for the voltages field and allows empty byte truncation). If the measured value is 0 then 1 should be sent instead.
        batteryVoltagesExt,
        // mode Battery mode. Default (0) is that battery mode reporting is not supported or battery is in normal-use mode.
        0,
        // fault_bitmask Fault/health indications. These should be set when charge_state is MAV_BATTERY_CHARGE_STATE_FAILED or MAV_BATTERY_CHARGE_STATE_UNHEALTHY (if not, fault reporting is not supported).
        0);

    mavlinkSendMessage();
}

void mavlinkSendScaledPressure(void)
{
    int16_t temperature;
    sensors(SENSOR_BARO) ? getBaroTemperature(&temperature) : getIMUTemperature(&temperature);
    mavlink_msg_scaled_pressure_pack(mavSystemId, mavComponentId, &mavSendMsg,
        millis(),
        0,
        0,
        temperature * 10,
        0);

    mavlinkSendMessage();
}

void mavlinkSendSystemTime(void)
{
    mavlink_msg_system_time_pack(
        mavSystemId,
        mavComponentId,
        &mavSendMsg,
        0,
        millis());

    mavlinkSendMessage();
}

bool mavlinkSendStatusText(void)
{
// FIXME - Status text is limited to boards with USE_OSD
#ifdef USE_OSD
    char buff[MAVLINK_MSG_STATUSTEXT_FIELD_TEXT_LEN] = {" "};
    textAttributes_t elemAttr = osdGetSystemMessage(buff, sizeof(buff), false);
    if (buff[0] != SYM_BLANK) {
        MAV_SEVERITY severity = MAV_SEVERITY_NOTICE;
        if (TEXT_ATTRIBUTES_HAVE_BLINK(elemAttr)) {
            severity = MAV_SEVERITY_CRITICAL;
        } else if TEXT_ATTRIBUTES_HAVE_INVERTED(elemAttr) {
            severity = MAV_SEVERITY_WARNING;
        }

        mavlink_msg_statustext_pack(mavSystemId, mavComponentId, &mavSendMsg,
            (uint8_t)severity,
            buff,
            0,
            0);

        mavlinkSendMessage();
        return true;
    }
#endif
    return false;
}

void mavlinkSendBatteryTemperatureStatusText(void)
{
    mavlinkSendBatteryStatus();
    mavlinkSendScaledPressure();
    mavlinkSendStatusText();
}

static uint8_t mavlinkGetAutopilotEnum(void)
{
    if (mavAutopilotType == MAVLINK_AUTOPILOT_ARDUPILOT) {
        return MAV_AUTOPILOT_ARDUPILOTMEGA;
    }

    return MAV_AUTOPILOT_GENERIC;
}

static uint8_t mavlinkHeadingDeg2FromCd(int32_t headingCd)
{
    return (uint8_t)(wrap_36000(headingCd) / 200);
}

static uint16_t mavlinkComputeHighLatencyFailures(void)
{
    uint16_t flags = 0;

#if defined(USE_GPS)
    if (!(sensors(SENSOR_GPS)
#ifdef USE_GPS_FIX_ESTIMATION
        || STATE(GPS_ESTIMATED_FIX)
#endif
        ) || gpsSol.fixType == GPS_NO_FIX) {
        flags |= HL_FAILURE_FLAG_GPS;
    }
#endif

#ifdef USE_PITOT
    if (getHwPitotmeterStatus() != HW_SENSOR_OK) {
        flags |= HL_FAILURE_FLAG_DIFFERENTIAL_PRESSURE;
    }
#endif

    if (getHwBarometerStatus() != HW_SENSOR_OK) {
        flags |= HL_FAILURE_FLAG_ABSOLUTE_PRESSURE;
    }

    if (getHwAccelerometerStatus() != HW_SENSOR_OK) {
        flags |= HL_FAILURE_FLAG_3D_ACCEL;
    }

    if (getHwGyroStatus() != HW_SENSOR_OK) {
        flags |= HL_FAILURE_FLAG_3D_GYRO;
    }

    if (getHwCompassStatus() != HW_SENSOR_OK) {
        flags |= HL_FAILURE_FLAG_3D_MAG;
    }

    if (getHwRangefinderStatus() != HW_SENSOR_OK) {
        flags |= HL_FAILURE_FLAG_TERRAIN;
    }

    const batteryState_e batteryState = getBatteryState();
    if (batteryState == BATTERY_WARNING || batteryState == BATTERY_CRITICAL) {
        flags |= HL_FAILURE_FLAG_BATTERY;
    }

    if (!rxIsReceivingSignal() || !rxAreFlightChannelsValid()) {
        flags |= HL_FAILURE_FLAG_RC_RECEIVER;
    }

    return flags;
}

void mavlinkSendHighLatency2(timeUs_t currentTimeUs)
{
    const bool isPlane = (STATE(FIXED_WING_LEGACY) || mixerConfig()->platformType == PLATFORM_AIRPLANE);
    const mavlinkModeSelection_t modeSelection = selectMavlinkMode(isPlane);

    int32_t latitude = 0;
    int32_t longitude = 0;
    int16_t altitude = (int16_t)constrain((int32_t)(getEstimatedActualPosition(Z) / 100), INT16_MIN, INT16_MAX);
    int16_t targetAltitude = (int16_t)constrain((int32_t)(posControl.desiredState.pos.z / 100), INT16_MIN, INT16_MAX);
    uint16_t targetDistance = 0;
    uint16_t wpNum = 0;
    uint8_t heading = mavlinkHeadingDeg2FromCd(DECIDEGREES_TO_CENTIDEGREES(attitude.values.yaw));
    uint8_t targetHeading = mavlinkHeadingDeg2FromCd(posControl.desiredState.yaw);
    uint8_t groundspeed = 0;
    uint8_t airspeed = 0;
    uint8_t airspeedSp = 0;
    uint8_t windspeed = 0;
    uint8_t windHeading = 0;
    uint8_t eph = UINT8_MAX;
    uint8_t epv = UINT8_MAX;
    int8_t temperatureAir = 0;
    int8_t climbRate = (int8_t)constrain(lrintf(getEstimatedActualVelocity(Z) / 10.0f), INT8_MIN, INT8_MAX);
    int8_t battery = feature(FEATURE_VBAT) ? (int8_t)calculateBatteryPercentage() : -1;

#if defined(USE_GPS)
    if (sensors(SENSOR_GPS)
#ifdef USE_GPS_FIX_ESTIMATION
        || STATE(GPS_ESTIMATED_FIX)
#endif
        ) {
        latitude = gpsSol.llh.lat;
        longitude = gpsSol.llh.lon;
        altitude = (int16_t)constrain((int32_t)(gpsSol.llh.alt / 100), INT16_MIN, INT16_MAX);

        const int32_t desiredAltCm = lrintf(posControl.desiredState.pos.z);
        const int32_t targetAltCm = GPS_home.alt + desiredAltCm;
        targetAltitude = (int16_t)constrain(targetAltCm / 100, INT16_MIN, INT16_MAX);

        groundspeed = (uint8_t)constrain(lrintf(gpsSol.groundSpeed / 20.0f), 0, UINT8_MAX);

        if (gpsSol.flags.validEPE) {
            eph = (uint8_t)constrain((gpsSol.eph + 5) / 10, 0, UINT8_MAX);
            epv = (uint8_t)constrain((gpsSol.epv + 5) / 10, 0, UINT8_MAX);
        }

        if (posControl.activeWaypointIndex >= 0) {
            wpNum = (uint16_t)posControl.activeWaypointIndex;
            targetDistance = (uint16_t)constrain(lrintf(posControl.wpDistance / 1000.0f), 0, UINT16_MAX);
        }
    }
#endif

#if defined(USE_PITOT)
    if (sensors(SENSOR_PITOT) && pitotIsHealthy()) {
        airspeed = (uint8_t)constrain(lrintf(getAirspeedEstimate() / 20.0f), 0, UINT8_MAX);
        airspeedSp = airspeed;
    }
#endif

    if (airspeedSp == 0) {
        const float desiredVelXY = calc_length_pythagorean_2D(posControl.desiredState.vel.x, posControl.desiredState.vel.y);
        airspeedSp = (uint8_t)constrain(lrintf(desiredVelXY / 20.0f), 0, UINT8_MAX);
    }

    if (airspeed == 0) {
        airspeed = groundspeed;
    }

#ifdef USE_WIND_ESTIMATOR
    if (isEstimatedWindSpeedValid()) {
        uint16_t windAngleCd = 0;
        const float windHoriz = getEstimatedHorizontalWindSpeed(&windAngleCd);
        windspeed = (uint8_t)constrain(lrintf(windHoriz / 20.0f), 0, UINT8_MAX);
        windHeading = mavlinkHeadingDeg2FromCd(windAngleCd);
    }
#endif

    int16_t temperature;
    sensors(SENSOR_BARO) ? getBaroTemperature(&temperature) : getIMUTemperature(&temperature);
    temperatureAir = (int8_t)constrain(temperature, INT8_MIN, INT8_MAX);

    const uint16_t failureFlags = mavlinkComputeHighLatencyFailures();

    mavlink_msg_high_latency2_pack(
        mavSystemId,
        mavComponentId,
        &mavSendMsg,
        (uint32_t)(currentTimeUs / 1000),
        mavlinkGetVehicleType(),
        mavlinkGetAutopilotEnum(),
        modeSelection.customMode,
        latitude,
        longitude,
        altitude,
        targetAltitude,
        heading,
        targetHeading,
        targetDistance,
        (uint8_t)constrain(getThrottlePercent(osdUsingScaledThrottle()), 0, 100),
        airspeed,
        airspeedSp,
        groundspeed,
        windspeed,
        windHeading,
        eph,
        epv,
        temperatureAir,
        climbRate,
        battery,
        wpNum,
        failureFlags,
        0,
        0,
        0);

    mavlinkSendMessage();
}


static void mavlinkResetIncomingMissionTransaction(void);

static bool handleIncoming_MISSION_CLEAR_ALL(void)
{
    mavlink_mission_clear_all_t msg;
    mavlink_msg_mission_clear_all_decode(&mavlinkContext.recvMsg, &msg);

    // Check if this message is for us
    if (msg.target_system == mavSystemId) {
        resetWaypointList();
        mavlinkResetIncomingMissionTransaction();
        mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_ACCEPTED, MAV_MISSION_TYPE_MISSION, 0);
        mavlinkSendMessage();
        return true;
    }

    return false;
}

static void mavlinkResetIncomingMissionTransaction(void)
{
    incomingMissionWpCount = 0;
    incomingMissionWpSequence = 0;
    incomingMissionSourceSystem = 0;
    incomingMissionSourceComponent = 0;
    incomingMissionLastActivityMs = 0;
}

static void mavlinkStartIncomingMissionTransaction(int missionCount)
{
    incomingMissionWpCount = missionCount;
    incomingMissionWpSequence = 0;
    incomingMissionSourceSystem = mavlinkContext.recvMsg.sysid;
    incomingMissionSourceComponent = mavlinkContext.recvMsg.compid;
    incomingMissionLastActivityMs = millis();
}

static void mavlinkTouchIncomingMissionTransaction(void)
{
    incomingMissionLastActivityMs = millis();
}

static bool mavlinkIsIncomingMissionTransactionActive(void)
{
    if (incomingMissionWpCount <= 0) {
        return false;
    }

    if ((timeMs_t)(millis() - incomingMissionLastActivityMs) > MAVLINK_MISSION_UPLOAD_TIMEOUT_MS) {
        mavlinkResetIncomingMissionTransaction();
        return false;
    }

    return true;
}

static bool mavlinkIsIncomingMissionTransactionOwner(void)
{
    return mavlinkContext.recvMsg.sysid == incomingMissionSourceSystem &&
        mavlinkContext.recvMsg.compid == incomingMissionSourceComponent;
}

static bool mavlinkHandleMissionItemCommon(bool useIntMessages, uint8_t frame, uint16_t command, uint8_t autocontinue, uint16_t seq, float param1, float param2, float param3, float param4, int32_t lat, int32_t lon, float altMeters)
{
    if (!mavlinkIsIncomingMissionTransactionActive() || !mavlinkIsIncomingMissionTransactionOwner()) {
        mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_INVALID_SEQUENCE, MAV_MISSION_TYPE_MISSION, 0);
        mavlinkSendMessage();
        return true;
    }

    mavlinkTouchIncomingMissionTransaction();

    const bool lastMissionItem = incomingMissionWpCount > 0 && ((int)seq + 1 >= incomingMissionWpCount);

    if (autocontinue == 0 && !lastMissionItem) {
        mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_UNSUPPORTED, MAV_MISSION_TYPE_MISSION, 0);
        mavlinkSendMessage();
        return true;
    }

    UNUSED(param3);

    navWaypoint_t wp;
    memset(&wp, 0, sizeof(wp));

    switch (command) {
        case MAV_CMD_NAV_WAYPOINT:
            if (!mavlinkFrameIsSupported(frame,
                MAV_FRAME_SUPPORTED_GLOBAL |
                MAV_FRAME_SUPPORTED_GLOBAL_INT |
                MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT |
                MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT)) {
                mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_UNSUPPORTED_FRAME, MAV_MISSION_TYPE_MISSION, 0);
                mavlinkSendMessage();
                return true;
            }
            wp.action = NAV_WP_ACTION_WAYPOINT;
            wp.lat = lat;
            wp.lon = lon;
            wp.alt = (int32_t)(altMeters * 100.0f);
            wp.p1 = 0;
            wp.p2 = 0;
            wp.p3 = mavlinkFrameUsesAbsoluteAltitude(frame) ? NAV_WP_ALTMODE : 0;
            break;

        case MAV_CMD_NAV_LOITER_TIME:
            if (!mavlinkFrameIsSupported(frame,
                MAV_FRAME_SUPPORTED_GLOBAL |
                MAV_FRAME_SUPPORTED_GLOBAL_INT |
                MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT |
                MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT)) {
                mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_UNSUPPORTED_FRAME, MAV_MISSION_TYPE_MISSION, 0);
                mavlinkSendMessage();
                return true;
            }
            wp.action = NAV_WP_ACTION_HOLD_TIME;
            wp.lat = lat;
            wp.lon = lon;
            wp.alt = (int32_t)(altMeters * 100.0f);
            wp.p1 = (int16_t)lrintf(param1);
            wp.p2 = 0;
            wp.p3 = mavlinkFrameUsesAbsoluteAltitude(frame) ? NAV_WP_ALTMODE : 0;
            break;

        case MAV_CMD_NAV_RETURN_TO_LAUNCH:
            {
                const bool coordinateFrame = mavlinkFrameIsSupported(frame,
                    MAV_FRAME_SUPPORTED_GLOBAL |
                    MAV_FRAME_SUPPORTED_GLOBAL_INT |
                    MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT |
                    MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT);

                if (!coordinateFrame && frame != MAV_FRAME_MISSION) {
                    mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_UNSUPPORTED_FRAME, MAV_MISSION_TYPE_MISSION, 0);
                    mavlinkSendMessage();
                    return true;
                }
                wp.action = NAV_WP_ACTION_RTH;
                wp.lat = 0;
                wp.lon = 0;
                wp.alt = coordinateFrame ? (int32_t)(altMeters * 100.0f) : 0;
                wp.p1 = 0; // Land if non-zero
                wp.p2 = 0;
                wp.p3 = mavlinkFrameUsesAbsoluteAltitude(frame) ? NAV_WP_ALTMODE : 0;
                break;
            }

        case MAV_CMD_NAV_LAND:
            if (!mavlinkFrameIsSupported(frame,
                MAV_FRAME_SUPPORTED_GLOBAL |
                MAV_FRAME_SUPPORTED_GLOBAL_INT |
                MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT |
                MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT)) {
                mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_UNSUPPORTED_FRAME, MAV_MISSION_TYPE_MISSION, 0);
                mavlinkSendMessage();
                return true;
            }
            wp.action = NAV_WP_ACTION_LAND;
            wp.lat = lat;
            wp.lon = lon;
            wp.alt = (int32_t)(altMeters * 100.0f);
            wp.p1 = 0; // Speed (cm/s)
            wp.p2 = 0; // Elevation Adjustment (m): P2 defines the ground elevation (in metres) for the LAND WP. If the altitude mode is absolute, this is also absolute; for relative altitude, then it is the difference between the assumed home location and the LAND WP.
            wp.p3 = mavlinkFrameUsesAbsoluteAltitude(frame) ? NAV_WP_ALTMODE : 0; // Altitude Mode & Actions, P3 defines the altitude mode. 0 (default, legacy) = Relative to Home, 1 = Absolute (AMSL).
            break;

        case MAV_CMD_DO_JUMP:
            if (frame != MAV_FRAME_MISSION) {
                mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_UNSUPPORTED_FRAME, MAV_MISSION_TYPE_MISSION, 0);
                mavlinkSendMessage();
                return true;
            }
            if (param1 < 0.0f) {
                mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_UNSUPPORTED, MAV_MISSION_TYPE_MISSION, 0);
                mavlinkSendMessage();
                return true;
            }
            wp.lat = 0;
            wp.lon = 0;
            wp.alt = 0;
            wp.action = NAV_WP_ACTION_JUMP;
            wp.p1 = (int16_t)lrintf(param1 + 1.0f);
            wp.p2 = (int16_t)lrintf(param2);
            wp.p3 = 0;
            break;

        case MAV_CMD_DO_SET_ROI:
            if (param1 != MAV_ROI_LOCATION ||
                !mavlinkFrameIsSupported(frame,
                    MAV_FRAME_SUPPORTED_GLOBAL |
                    MAV_FRAME_SUPPORTED_GLOBAL_INT |
                    MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT |
                    MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT)) {
                mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_UNSUPPORTED, MAV_MISSION_TYPE_MISSION, 0);
                mavlinkSendMessage();
                return true;
            }
            wp.action = NAV_WP_ACTION_SET_POI;
            wp.lat = lat;
            wp.lon = lon;
            wp.alt = (int32_t)(altMeters * 100.0f);
            wp.p1 = 0;
            wp.p2 = 0;
            wp.p3 = mavlinkFrameUsesAbsoluteAltitude(frame) ? NAV_WP_ALTMODE : 0;
            break;

        case MAV_CMD_CONDITION_YAW:
            if (frame != MAV_FRAME_MISSION) {
                mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_UNSUPPORTED_FRAME, MAV_MISSION_TYPE_MISSION, 0);
                mavlinkSendMessage();
                return true;
            }
            if (param4 != 0.0f) {
                mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_UNSUPPORTED, MAV_MISSION_TYPE_MISSION, 0);
                mavlinkSendMessage();
                return true;
            }
            wp.lat = 0;
            wp.lon = 0;
            wp.alt = 0;
            wp.action = NAV_WP_ACTION_SET_HEAD;
            wp.p1 = (int16_t)lrintf(param1);
            wp.p2 = 0;
            wp.p3 = 0;
            break;

        default:
            mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_UNSUPPORTED, MAV_MISSION_TYPE_MISSION, 0);
            mavlinkSendMessage();
            return true;
    }

    if (seq == incomingMissionWpSequence) {
        incomingMissionWpSequence++;

        wp.flag = (incomingMissionWpSequence >= incomingMissionWpCount) ? NAV_WP_FLAG_LAST : 0;

        setWaypoint(incomingMissionWpSequence, &wp);

        if (incomingMissionWpSequence >= incomingMissionWpCount) {
            if (isWaypointListValid()) {
                mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_ACCEPTED, MAV_MISSION_TYPE_MISSION, 0);
                mavlinkSendMessage();
            }
            else {
                mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_INVALID, MAV_MISSION_TYPE_MISSION, 0);
                mavlinkSendMessage();
            }
            mavlinkResetIncomingMissionTransaction();
        }
        else {
            if (useIntMessages) {
                mavlink_msg_mission_request_int_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, incomingMissionWpSequence, MAV_MISSION_TYPE_MISSION);
            } else {
                mavlink_msg_mission_request_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, incomingMissionWpSequence, MAV_MISSION_TYPE_MISSION);
            }
            mavlinkSendMessage();
        }
    }
    else {
        // If we get a duplicate of the last accepted item, re-request the next one instead of aborting.
        if (seq + 1 == incomingMissionWpSequence) {
            mavlinkTouchIncomingMissionTransaction();
            if (useIntMessages) {
                mavlink_msg_mission_request_int_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, incomingMissionWpSequence, MAV_MISSION_TYPE_MISSION);
            } else {
                mavlink_msg_mission_request_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, incomingMissionWpSequence, MAV_MISSION_TYPE_MISSION);
            }
            mavlinkSendMessage();
        } else {
            mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_INVALID_SEQUENCE, MAV_MISSION_TYPE_MISSION, 0);
            mavlinkSendMessage();
        }
    }

    return true;
}

static bool handleIncoming_MISSION_COUNT(void)
{
    mavlink_mission_count_t msg;
    mavlink_msg_mission_count_decode(&mavlinkContext.recvMsg, &msg);

    // Check if this message is for us
    if (msg.target_system == mavSystemId) {
        mavlinkResetIncomingMissionTransaction();
        if (ARMING_FLAG(ARMED)) {
            mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_ERROR, MAV_MISSION_TYPE_MISSION, 0);
            mavlinkSendMessage();
            return true;
        }
        if (msg.count == 0) {
            resetWaypointList();
            mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_ACCEPTED, MAV_MISSION_TYPE_MISSION, 0);
            mavlinkSendMessage();
            return true;
        }
        if (msg.count <= NAV_MAX_WAYPOINTS) {
            mavlinkStartIncomingMissionTransaction(msg.count);
            mavlink_msg_mission_request_int_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, incomingMissionWpSequence, MAV_MISSION_TYPE_MISSION);
            mavlinkSendMessage();
            return true;
        } else {
            mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_NO_SPACE, MAV_MISSION_TYPE_MISSION, 0);
            mavlinkSendMessage();
            return true;
        }
    }

    return false;
}

static bool handleIncoming_MISSION_ITEM(void)
{
    mavlink_mission_item_t msg;
    mavlink_msg_mission_item_decode(&mavlinkContext.recvMsg, &msg);

    if (msg.target_system != mavSystemId) {
        return false;
    }

    if (ARMING_FLAG(ARMED)) {
        if (msg.command == MAV_CMD_NAV_WAYPOINT) {
            return mavlinkHandleArmedGuidedMissionItem(msg.current, msg.frame,
                MAV_FRAME_SUPPORTED_GLOBAL | MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT,
                (int32_t)lrintf(msg.x * 1e7f), (int32_t)lrintf(msg.y * 1e7f), msg.z);
        }

        mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_ERROR, MAV_MISSION_TYPE_MISSION, 0);
        mavlinkSendMessage();
        return true;
    }

    return mavlinkHandleMissionItemCommon(false, msg.frame, msg.command, msg.autocontinue, msg.seq, msg.param1, msg.param2, msg.param3, msg.param4, (int32_t)(msg.x * 1e7f), (int32_t)(msg.y * 1e7f), msg.z);
}

static bool handleIncoming_MISSION_REQUEST_LIST(void)
{
    mavlink_mission_request_list_t msg;
    mavlink_msg_mission_request_list_decode(&mavlinkContext.recvMsg, &msg);

    // Check if this message is for us
    if (msg.target_system == mavSystemId) {
        mavlink_msg_mission_count_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, getWaypointCount(), MAV_MISSION_TYPE_MISSION, 0);
        mavlinkSendMessage();
        return true;
    }

    return false;
}

static bool fillMavlinkMissionItemFromWaypoint(const navWaypoint_t *wp, bool useIntMessages, mavlinkMissionItemData_t *item)
{
    mavlinkMissionItemData_t data = {0};

    data.frame = navWaypointFrame(wp, useIntMessages);

    switch (wp->action) {
        case NAV_WP_ACTION_WAYPOINT:
            data.command = MAV_CMD_NAV_WAYPOINT;
            data.lat = wp->lat;
            data.lon = wp->lon;
            data.alt = wp->alt / 100.0f;
            break;

        case NAV_WP_ACTION_HOLD_TIME:
            data.command = MAV_CMD_NAV_LOITER_TIME;
            data.param1 = wp->p1;
            data.lat = wp->lat;
            data.lon = wp->lon;
            data.alt = wp->alt / 100.0f;
            break;

        case NAV_WP_ACTION_RTH:
            data.command = MAV_CMD_NAV_RETURN_TO_LAUNCH;
            break;

        case NAV_WP_ACTION_LAND:
            data.command = MAV_CMD_NAV_LAND;
            data.lat = wp->lat;
            data.lon = wp->lon;
            data.alt = wp->alt / 100.0f;
            break;

        case NAV_WP_ACTION_JUMP:
            data.command = MAV_CMD_DO_JUMP;
            data.param1 = (wp->p1 > 0) ? (float)(wp->p1 - 1) : 0.0f;
            data.param2 = wp->p2;
            break;

        case NAV_WP_ACTION_SET_POI:
            data.command = MAV_CMD_DO_SET_ROI;
            data.param1 = MAV_ROI_LOCATION;
            data.lat = wp->lat;
            data.lon = wp->lon;
            data.alt = wp->alt / 100.0f;
            break;

        case NAV_WP_ACTION_SET_HEAD:
            data.command = MAV_CMD_CONDITION_YAW;
            data.param1 = wp->p1;
            break;

        default:
            return false;
    }

    *item = data;
    return true;
}

static bool handleIncoming_MISSION_REQUEST(void)
{
    mavlink_mission_request_t msg;
    mavlink_msg_mission_request_decode(&mavlinkContext.recvMsg, &msg);

    if (msg.target_system != mavSystemId) {
        return false;
    }

    int wpCount = getWaypointCount();

    if (msg.seq < wpCount) {
        navWaypoint_t wp;
        getWaypoint(msg.seq + 1, &wp);

        mavlinkMissionItemData_t item;
        if (fillMavlinkMissionItemFromWaypoint(&wp, false, &item)) {
            mavlink_msg_mission_item_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid,
                        msg.seq,
                        item.frame,
                        item.command,
                        0,
                        1,
                        item.param1, item.param2, item.param3, item.param4,
                        item.lat / 1e7f,
                        item.lon / 1e7f,
                        item.alt,
                        MAV_MISSION_TYPE_MISSION);
            mavlinkSendMessage();
        } else {
            mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_ERROR, MAV_MISSION_TYPE_MISSION, 0);
            mavlinkSendMessage();
        }
    }
    else {
        mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_INVALID_SEQUENCE, MAV_MISSION_TYPE_MISSION, 0);
        mavlinkSendMessage();
    }

    return true;
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
                bool sent = false;
                uint16_t messageId = (uint16_t)param1;

                switch (messageId) {
                    case MAVLINK_MSG_ID_HEARTBEAT:
                        mavlinkSendHeartbeat();
                        sent = true;
                        break;
                    case MAVLINK_MSG_ID_AUTOPILOT_VERSION:
                        if (mavlinkGetProtocolVersion() != 1) {
                            mavlinkSendAutopilotVersion();
                            sent = true;
                        }
                        break;
                    case MAVLINK_MSG_ID_PROTOCOL_VERSION:
                        if (mavlinkGetProtocolVersion() != 1) {
                            mavlinkSendProtocolVersion();
                            sent = true;
                        }
                        break;
                    case MAVLINK_MSG_ID_SYS_STATUS:
                        mavlinkSendSystemStatus();
                        sent = true;
                        break;
                    case MAVLINK_MSG_ID_ATTITUDE:
                        mavlinkSendAttitude();
                        sent = true;
                        break;
                    case MAVLINK_MSG_ID_VFR_HUD:
                        mavlinkSendVfrHud();
                        sent = true;
                        break;
                    case MAVLINK_MSG_ID_AVAILABLE_MODES:
                        {
                            const bool isPlane = mixerConfig()->platformType == PLATFORM_AIRPLANE || STATE(FIXED_WING_LEGACY);
                            const mavlinkModeSelection_t modeSelection = selectMavlinkMode(isPlane);
                            if (isPlane) {
                                mavlinkSendAvailableModes(planeModes, planeModesCount, modeSelection.customMode, mavlinkPlaneModeIsConfigured);
                            } else {
                                mavlinkSendAvailableModes(copterModes, copterModesCount, modeSelection.customMode, mavlinkCopterModeIsConfigured);
                            }
                            sent = true;
                        }
                        break;
                    case MAVLINK_MSG_ID_CURRENT_MODE:
                        {
                            const bool isPlane = mixerConfig()->platformType == PLATFORM_AIRPLANE || STATE(FIXED_WING_LEGACY);
                            const mavlinkModeSelection_t modeSelection = selectMavlinkMode(isPlane);
                            mavlink_msg_current_mode_pack(
                                mavSystemId,
                                mavComponentId,
                                &mavSendMsg,
                                MAV_STANDARD_MODE_NON_STANDARD,
                                modeSelection.customMode,
                                modeSelection.customMode);
                            mavlinkSendMessage();
                            sent = true;
                        }
                        break;
                    case MAVLINK_MSG_ID_EXTENDED_SYS_STATE:
                        mavlinkSendExtendedSysState();
                        sent = true;
                        break;
                    case MAVLINK_MSG_ID_RC_CHANNELS:
                    case MAVLINK_MSG_ID_RC_CHANNELS_RAW:
                    case MAVLINK_MSG_ID_RC_CHANNELS_SCALED:
                        mavlinkSendRCChannelsAndRSSI();
                        sent = true;
                        break;
                    case MAVLINK_MSG_ID_GPS_RAW_INT:
    #ifdef USE_GPS
                        mavlinkSendGpsRawInt(micros());
                        sent = true;
    #endif
                        break;
                    case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
    #ifdef USE_GPS
                        mavlinkSendGlobalPositionInt(micros());
                        sent = true;
    #endif
                        break;
                    case MAVLINK_MSG_ID_GPS_GLOBAL_ORIGIN:
    #ifdef USE_GPS
                        mavlinkSendGpsGlobalOrigin();
                        sent = true;
    #endif
                        break;
                    case MAVLINK_MSG_ID_BATTERY_STATUS:
                        mavlinkSendBatteryStatus();
                        sent = true;
                        break;
                    case MAVLINK_MSG_ID_SCALED_PRESSURE:
                        mavlinkSendScaledPressure();
                        sent = true;
                        break;
                    case MAVLINK_MSG_ID_SYSTEM_TIME:
                        mavlinkSendSystemTime();
                        sent = true;
                        break;
                    case MAVLINK_MSG_ID_STATUSTEXT:
                        sent = mavlinkSendStatusText();
                        break;
                    case MAVLINK_MSG_ID_HOME_POSITION:
    #ifdef USE_GPS
                        if (STATE(GPS_FIX_HOME)) {
                            mavlinkSendHomePosition();
                            sent = true;
                        }
    #endif
                        break;
                    default:
                        break;
                }

                mavlinkSendCommandAck(command, sent ? MAV_RESULT_ACCEPTED : MAV_RESULT_UNSUPPORTED, ackTargetSystem, ackTargetComponent);
                return true;
            }
#ifdef USE_GPS
        case MAV_CMD_GET_HOME_POSITION:
            if (STATE(GPS_FIX_HOME)) {
                mavlinkSendHomePosition();
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

static bool handleIncoming_MISSION_ITEM_INT(void)
{
    mavlink_mission_item_int_t msg;
    mavlink_msg_mission_item_int_decode(&mavlinkContext.recvMsg, &msg);

    if (msg.target_system != mavSystemId) {
        return false;
    }

    if (ARMING_FLAG(ARMED)) {
        if (msg.command == MAV_CMD_NAV_WAYPOINT) {
            return mavlinkHandleArmedGuidedMissionItem(msg.current, msg.frame,
                MAV_FRAME_SUPPORTED_GLOBAL_INT | MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT,
                msg.x, msg.y, msg.z);
        }

        mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_ERROR, MAV_MISSION_TYPE_MISSION, 0);
        mavlinkSendMessage();
        return true;
    }

    return mavlinkHandleMissionItemCommon(true, msg.frame, msg.command, msg.autocontinue, msg.seq, msg.param1, msg.param2, msg.param3, msg.param4, msg.x, msg.y, msg.z);
}

static bool handleIncoming_MISSION_REQUEST_INT(void)
{
    mavlink_mission_request_int_t msg;
    mavlink_msg_mission_request_int_decode(&mavlinkContext.recvMsg, &msg);

    if (msg.target_system != mavSystemId) {
        return false;
    }

    int wpCount = getWaypointCount();

    if (msg.seq < wpCount) {
        navWaypoint_t wp;
        getWaypoint(msg.seq + 1, &wp);

        mavlinkMissionItemData_t item;
        if (fillMavlinkMissionItemFromWaypoint(&wp, true, &item)) {
            mavlink_msg_mission_item_int_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid,
                        msg.seq,
                        item.frame,
                        item.command,
                        0,
                        1,
                        item.param1, item.param2, item.param3, item.param4,
                        item.lat,
                        item.lon,
                        item.alt,
                        MAV_MISSION_TYPE_MISSION);
            mavlinkSendMessage();
        } else {
            mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_ERROR, MAV_MISSION_TYPE_MISSION, 0);
            mavlinkSendMessage();
        }
    }
    else {
        mavlink_msg_mission_ack_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, MAV_MISSION_INVALID_SEQUENCE, MAV_MISSION_TYPE_MISSION, 0);
        mavlinkSendMessage();
    }

    return true;
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
        return handleIncoming_MISSION_CLEAR_ALL() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MISSION_COUNT:
        return handleIncoming_MISSION_COUNT() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MISSION_ITEM:
        return handleIncoming_MISSION_ITEM() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MISSION_ITEM_INT:
        return handleIncoming_MISSION_ITEM_INT() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:
        return handleIncoming_MISSION_REQUEST_LIST() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_COMMAND_LONG:
        return handleIncoming_COMMAND_LONG() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_COMMAND_INT:
        return handleIncoming_COMMAND_INT() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MISSION_REQUEST:
        return handleIncoming_MISSION_REQUEST() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
    case MAVLINK_MSG_ID_MISSION_REQUEST_INT:
        return handleIncoming_MISSION_REQUEST_INT() ? MAVLINK_FC_DISPATCH_HANDLED_ACTIVITY : MAVLINK_FC_DISPATCH_NOT_HANDLED;
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
