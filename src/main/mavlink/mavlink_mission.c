#include "mavlink/mavlink_internal.h"

#include "mavlink/mavlink_guided.h"
#include "mavlink/mavlink_mission.h"
#include "mavlink/mavlink_runtime.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_MAVLINK)

/*
 * Mission transfer retry and partner tracking are adapted from Betaflight's
 * GPLv3 mavlink_mission.c, commit 87d4bd63, for INAV's routed multi-port runtime.
 */
static uint8_t mavlinkActivePortMask(void)
{
    uint8_t sendMask = 0;
    for (uint8_t portIndex = 0; portIndex < mavPortCount; portIndex++) {
        if (mavPortStates[portIndex].telemetryEnabled && mavPortStates[portIndex].port) {
            sendMask |= MAVLINK_PORT_MASK(portIndex);
        }
    }
    return sendMask;
}

void mavlinkSendPendingMissionItemReached(void)
{
    uint16_t seq;
    if (!navigationConsumeWaypointReached(&seq)) {
        return;
    }

    const uint8_t sendMask = mavlinkActivePortMask();
    if (sendMask == 0) {
        return;
    }

    mavlinkContext.missionCompleted = getWaypointCount() > 0 && seq + 1 >= getWaypointCount();
    mavSendMask = sendMask;
    mavlink_msg_mission_item_reached_pack(mavlinkGetCommonConfig()->sysid, MAV_COMP_ID_AUTOPILOT1, &mavSendMsg, seq);
    mavlinkSendMessage();
    mavSendMask = 0;
}

uint8_t mavlinkWaypointFrame(const navWaypoint_t *wp, bool useIntMessages)
{
    switch (wp->action) {
        case NAV_WP_ACTION_RTH:
        case NAV_WP_ACTION_JUMP:
        case NAV_WP_ACTION_SET_HEAD:
            return MAV_FRAME_MISSION;
        default:
            break;
    }

    if ((wp->p3 & NAV_WP_ALTMODE) == NAV_WP_ALTMODE) {
        return useIntMessages ? MAV_FRAME_GLOBAL_INT : MAV_FRAME_GLOBAL;
    }

    return useIntMessages ? MAV_FRAME_GLOBAL_RELATIVE_ALT_INT : MAV_FRAME_GLOBAL_RELATIVE_ALT;
}


static bool mavlinkMissionTargetIsLocal(uint8_t targetSystem, uint8_t targetComponent)
{
    return (targetSystem == 0 || targetSystem == mavSystemId) &&
        (targetComponent == 0 || targetComponent == mavComponentId);
}

static bool mavlinkMissionSenderOwnsTransfer(void)
{
    return mavlinkContext.recvMsg.sysid == mavMissionTransfer.partnerSystem &&
        mavlinkContext.recvMsg.compid == mavMissionTransfer.partnerComponent &&
        mavRecvPortIndex == mavMissionTransfer.ingressPortIndex;
}

static void mavlinkSendMissionAckTo(uint8_t targetSystem, uint8_t targetComponent, MAV_MISSION_RESULT result)
{
    mavlink_msg_mission_ack_pack(
        mavSystemId,
        mavComponentId,
        &mavSendMsg,
        targetSystem,
        targetComponent,
        result,
        MAV_MISSION_TYPE_MISSION,
        0
    );
    mavlinkSendMessage();
}

static void mavlinkSendMissionAck(MAV_MISSION_RESULT result)
{
    mavlinkSendMissionAckTo(mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, result);
}

static void mavlinkResetMissionTransfer(void)
{
    memset(&mavMissionTransfer, 0, sizeof(mavMissionTransfer));
    mavMissionTransfer.state = MAVLINK_MISSION_TRANSFER_IDLE;
}

static void mavlinkAbortMissionUpload(MAV_MISSION_RESULT result)
{
    mavlinkSendMissionAck(result);
    mavlinkResetMissionTransfer();
}

static void mavlinkStartMissionTransfer(mavlinkMissionTransferState_e state, uint16_t count)
{
    mavlinkResetMissionTransfer();
    mavMissionTransfer.state = state;
    mavMissionTransfer.count = count;
    mavMissionTransfer.partnerSystem = mavlinkContext.recvMsg.sysid;
    mavMissionTransfer.partnerComponent = mavlinkContext.recvMsg.compid;
    mavMissionTransfer.ingressPortIndex = mavRecvPortIndex;
    mavMissionTransfer.useIntMessages = true;
    mavMissionTransfer.lastActivityMs = millis();
}

static void mavlinkSendMissionRequest(void)
{
    if (mavMissionTransfer.useIntMessages) {
        mavlink_msg_mission_request_int_pack(
            mavSystemId,
            mavComponentId,
            &mavSendMsg,
            mavMissionTransfer.partnerSystem,
            mavMissionTransfer.partnerComponent,
            mavMissionTransfer.nextSequence,
            MAV_MISSION_TYPE_MISSION);
    } else {
        mavlink_msg_mission_request_pack(
            mavSystemId,
            mavComponentId,
            &mavSendMsg,
            mavMissionTransfer.partnerSystem,
            mavMissionTransfer.partnerComponent,
            mavMissionTransfer.nextSequence,
            MAV_MISSION_TYPE_MISSION);
    }
    mavlinkSendMessage();
}

static bool mavlinkPersistMission(void)
{
#ifdef NAV_NON_VOLATILE_WAYPOINT_STORAGE
    return saveNonVolatileWaypointList();
#else
    return true;
#endif
}

void mavlinkMissionUpdate(timeMs_t currentTimeMs)
{
    if (mavMissionTransfer.state == MAVLINK_MISSION_TRANSFER_RECEIVING &&
        currentTimeMs - mavMissionTransfer.lastActivityMs >= MAVLINK_MISSION_UPLOAD_RETRY_MS) {
        mavSendMask = MAVLINK_PORT_MASK(mavMissionTransfer.ingressPortIndex);
        if (mavMissionTransfer.retries >= MAVLINK_MISSION_UPLOAD_MAX_RETRIES) {
            mavlinkSendMissionAckTo(
                mavMissionTransfer.partnerSystem,
                mavMissionTransfer.partnerComponent,
                MAV_MISSION_OPERATION_CANCELLED);
            mavlinkResetMissionTransfer();
        } else {
            mavMissionTransfer.retries++;
            mavMissionTransfer.lastActivityMs = currentTimeMs;
            mavlinkSendMissionRequest();
        }
        mavSendMask = 0;
    }

    if (mavMissionTransfer.state == MAVLINK_MISSION_TRANSFER_SENDING &&
        currentTimeMs - mavMissionTransfer.lastActivityMs >= MAVLINK_MISSION_DOWNLOAD_TIMEOUT_MS) {
        mavlinkResetMissionTransfer();
    }

    if (currentTimeMs - mavlinkContext.lastMissionCurrentMs < MAVLINK_MISSION_CURRENT_INTERVAL_MS) {
        return;
    }

    const uint8_t sendMask = mavlinkActivePortMask();
    if (sendMask == 0) {
        return;
    }

    const uint16_t total = getWaypointCount();
    const bool active = FLIGHT_MODE(NAV_WP_MODE);
    const uint16_t seq = total > 0 && active && getActiveWpNumber() > 0 ? getActiveWpNumber() - 1 : 0;
    MISSION_STATE missionState;
    if (total == 0) {
        missionState = MISSION_STATE_NO_MISSION;
    } else if (active) {
        missionState = MISSION_STATE_ACTIVE;
    } else if (mavlinkContext.missionCompleted) {
        missionState = MISSION_STATE_COMPLETE;
    } else {
        missionState = MISSION_STATE_NOT_STARTED;
    }

    mavSendMask = sendMask;
    mavlink_msg_mission_current_pack(
        mavlinkGetCommonConfig()->sysid,
        MAV_COMP_ID_AUTOPILOT1,
        &mavSendMsg,
        seq,
        total,
        missionState,
        active ? 1 : 2,
        0,
        0,
        0);
    mavlinkSendMessage();
    mavSendMask = 0;
    mavlinkContext.lastMissionCurrentMs = currentTimeMs;
}

static bool mavlinkHandleArmedGuidedMissionItem(
    uint8_t current, 
    uint8_t frame, 
    mavFrameSupportMask_e allowedFrames, 
    int32_t latitudeE7, 
    int32_t longitudeE7, 
    float altitudeMeters)
{
    if (!isGCSValid()) {
        mavlinkSendMissionAck(MAV_MISSION_ERROR);
        return true;
    }

    if (!mavlinkFrameIsSupported(frame, allowedFrames)) {
        mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED_FRAME);
        return true;
    }
    if (!isfinite(altitudeMeters) ||
        altitudeMeters < (float)INT32_MIN / 100.0f ||
        altitudeMeters > (float)INT32_MAX / 100.0f ||
        latitudeE7 < -900000000 || latitudeE7 > 900000000 ||
        longitudeE7 < -1800000000 || longitudeE7 > 1800000000) {
        mavlinkSendMissionAck(MAV_MISSION_INVALID);
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

        mavlinkSendMissionAck(MAV_MISSION_ACCEPTED);
        return true;
    }

    if (current == 3) {
        const MAV_RESULT result = mavlinkSetAltitudeTargetFromFrame(frame, altitudeMeters);
        MAV_MISSION_RESULT response = MAV_MISSION_ERROR;
        if (result == MAV_RESULT_ACCEPTED) {response = MAV_MISSION_ACCEPTED;}
        else if (result == MAV_RESULT_UNSUPPORTED) {response = MAV_MISSION_UNSUPPORTED;}
        mavlinkSendMissionAck(response);
        return true;
    }

    mavlinkSendMissionAck(MAV_MISSION_ERROR);
    return true;
}

static bool mavlinkHandleMissionItemCommon(
    bool useIntMessages, 
    uint8_t frame, 
    uint16_t command, 
    uint8_t autocontinue, 
    uint16_t seq, 
    float param1, 
    float param2, 
    float param3, 
    float param4, 
    int32_t lat,
    int32_t lon, 
    float altMeters)
{
    if (mavMissionTransfer.state != MAVLINK_MISSION_TRANSFER_RECEIVING || !mavlinkMissionSenderOwnsTransfer()) {
        mavlinkSendMissionAck(MAV_MISSION_INVALID_SEQUENCE);
        return true;
    }
    if (!isfinite(param1) || !isfinite(param2) || !isfinite(param3) || !isfinite(param4) || !isfinite(altMeters)) {
        mavlinkAbortMissionUpload(MAV_MISSION_INVALID);
        return true;
    }
    if (altMeters < (float)INT32_MIN / 100.0f || altMeters > (float)INT32_MAX / 100.0f) {
        mavlinkAbortMissionUpload(MAV_MISSION_INVALID_PARAM7);
        return true;
    }
    if (lat < -900000000 || lat > 900000000) {
        mavlinkAbortMissionUpload(MAV_MISSION_INVALID_PARAM5_X);
        return true;
    }
    if (lon < -1800000000 || lon > 1800000000) {
        mavlinkAbortMissionUpload(MAV_MISSION_INVALID_PARAM6_Y);
        return true;
    }

    if (!mavMissionTransfer.formatSelected) {
        mavMissionTransfer.useIntMessages = useIntMessages;
        mavMissionTransfer.formatSelected = true;
    } else if (mavMissionTransfer.useIntMessages != useIntMessages) {
        mavlinkAbortMissionUpload(MAV_MISSION_UNSUPPORTED);
        return true;
    }

    mavMissionTransfer.lastActivityMs = millis();
    mavMissionTransfer.retries = 0;

    const bool lastMissionItem = seq + 1 >= mavMissionTransfer.count;

    if (autocontinue == 0 && !lastMissionItem) {
        mavlinkAbortMissionUpload(MAV_MISSION_UNSUPPORTED);
        return true;
    }

    UNUSED(param3);

    navWaypoint_t wp = {0};

    switch (command) {
        case MAV_CMD_NAV_WAYPOINT:
            if (!mavlinkFrameIsSupported(frame,
                MAV_FRAME_SUPPORTED_GLOBAL |
                MAV_FRAME_SUPPORTED_GLOBAL_INT |
                MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT |
                MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT)) {
                mavlinkAbortMissionUpload(MAV_MISSION_UNSUPPORTED_FRAME);
                return true;
            }
            wp.action = NAV_WP_ACTION_WAYPOINT;
            wp.lat = lat;
            wp.lon = lon;
            wp.alt = (int32_t)(altMeters * 100.0f);
            wp.p3 = mavlinkFrameUsesAbsoluteAltitude(frame) ? NAV_WP_ALTMODE : 0;
            break;

        case MAV_CMD_NAV_LOITER_TIME:
            if (!mavlinkFrameIsSupported(frame,
                MAV_FRAME_SUPPORTED_GLOBAL |
                MAV_FRAME_SUPPORTED_GLOBAL_INT |
                MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT |
                MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT)) {
                mavlinkAbortMissionUpload(MAV_MISSION_UNSUPPORTED_FRAME);
                return true;
            }
            if (param1 < 0.0f || param1 > INT16_MAX) {
                mavlinkAbortMissionUpload(MAV_MISSION_INVALID_PARAM1);
                return true;
            }
            wp.action = NAV_WP_ACTION_HOLD_TIME;
            wp.lat = lat;
            wp.lon = lon;
            wp.alt = (int32_t)(altMeters * 100.0f);
            wp.p1 = (int16_t)lrintf(param1);
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
                    mavlinkAbortMissionUpload(MAV_MISSION_UNSUPPORTED_FRAME);
                    return true;
                }
                wp.action = NAV_WP_ACTION_RTH;
                wp.alt = coordinateFrame ? (int32_t)(altMeters * 100.0f) : 0;
                wp.p3 = mavlinkFrameUsesAbsoluteAltitude(frame) ? NAV_WP_ALTMODE : 0;
                break;
            }

        case MAV_CMD_NAV_LAND:
            if (!mavlinkFrameIsSupported(frame,
                MAV_FRAME_SUPPORTED_GLOBAL |
                MAV_FRAME_SUPPORTED_GLOBAL_INT |
                MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT |
                MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT)) {
                mavlinkAbortMissionUpload(MAV_MISSION_UNSUPPORTED_FRAME);
                return true;
            }
            wp.action = NAV_WP_ACTION_LAND;
            wp.lat = lat;
            wp.lon = lon;
            wp.alt = (int32_t)(altMeters * 100.0f);
            wp.p3 = mavlinkFrameUsesAbsoluteAltitude(frame) ? NAV_WP_ALTMODE : 0;
            break;

        case MAV_CMD_DO_JUMP:
            if (frame != MAV_FRAME_MISSION) {
                mavlinkAbortMissionUpload(MAV_MISSION_UNSUPPORTED_FRAME);
                return true;
            }
            if (param1 < 0.0f || param1 >= INT16_MAX) {
                mavlinkAbortMissionUpload(MAV_MISSION_INVALID_PARAM1);
                return true;
            }
            if (param2 < INT16_MIN || param2 > INT16_MAX) {
                mavlinkAbortMissionUpload(MAV_MISSION_INVALID_PARAM2);
                return true;
            }
            wp.action = NAV_WP_ACTION_JUMP;
            wp.p1 = (int16_t)lrintf(param1 + 1.0f);
            wp.p2 = (int16_t)lrintf(param2);
            break;

        case MAV_CMD_DO_SET_ROI:
            if (param1 != MAV_ROI_LOCATION ||
                !mavlinkFrameIsSupported(frame,
                    MAV_FRAME_SUPPORTED_GLOBAL |
                    MAV_FRAME_SUPPORTED_GLOBAL_INT |
                    MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT |
                    MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT)) {
                mavlinkAbortMissionUpload(MAV_MISSION_UNSUPPORTED);
                return true;
            }
            wp.action = NAV_WP_ACTION_SET_POI;
            wp.lat = lat;
            wp.lon = lon;
            wp.alt = (int32_t)(altMeters * 100.0f);
            wp.p3 = mavlinkFrameUsesAbsoluteAltitude(frame) ? NAV_WP_ALTMODE : 0;
            break;

        case MAV_CMD_CONDITION_YAW:
            if (frame != MAV_FRAME_MISSION) {
                mavlinkAbortMissionUpload(MAV_MISSION_UNSUPPORTED_FRAME);
                return true;
            }
            if (param1 < 0.0f || param1 >= 360.0f) {
                mavlinkAbortMissionUpload(MAV_MISSION_INVALID_PARAM1);
                return true;
            }
            if (param4 != 0.0f) {
                mavlinkAbortMissionUpload(MAV_MISSION_UNSUPPORTED);
                return true;
            }
            wp.action = NAV_WP_ACTION_SET_HEAD;
            wp.p1 = (int16_t)lrintf(param1);
            break;

        default:
            mavlinkAbortMissionUpload(MAV_MISSION_UNSUPPORTED);
            return true;
    }

    if (seq == mavMissionTransfer.nextSequence) {
        mavMissionTransfer.nextSequence++;
        wp.flag = (mavMissionTransfer.nextSequence >= mavMissionTransfer.count) ? NAV_WP_FLAG_LAST : 0;
        setWaypoint(mavMissionTransfer.nextSequence, &wp);

        if (mavMissionTransfer.nextSequence >= mavMissionTransfer.count) {
            if (isWaypointListValid() && mavlinkPersistMission()) {
                mavlinkContext.missionCompleted = false;
                mavlinkSendMissionAck(MAV_MISSION_ACCEPTED);
            } else {
                mavlinkSendMissionAck(MAV_MISSION_INVALID);
            }
            mavlinkResetMissionTransfer();
        } else {
            mavlinkSendMissionRequest();
        }
    } else {
        if (seq + 1 == mavMissionTransfer.nextSequence) {
            mavlinkSendMissionRequest();
        } else {
            mavlinkAbortMissionUpload(MAV_MISSION_INVALID_SEQUENCE);
        }
    }

    return true;
}

bool mavlinkHandleIncomingMissionClearAll(void)
{
    mavlink_mission_clear_all_t msg;
    mavlink_msg_mission_clear_all_decode(&mavlinkContext.recvMsg, &msg);

    if (!mavlinkMissionTargetIsLocal(msg.target_system, msg.target_component)) {
        return false;
    }
    if (msg.mission_type != MAV_MISSION_TYPE_MISSION && msg.mission_type != MAV_MISSION_TYPE_ALL) {
        mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED);
        return true;
    }
    if (ARMING_FLAG(ARMED)) {
        mavlinkSendMissionAck(MAV_MISSION_DENIED);
        return true;
    }

    resetWaypointList();
    mavlinkResetMissionTransfer();
    mavlinkContext.missionCompleted = false;
    mavlinkSendMissionAck(mavlinkPersistMission() ? MAV_MISSION_ACCEPTED : MAV_MISSION_ERROR);
    return true;
}

bool mavlinkHandleIncomingMissionCount(void)
{
    mavlink_mission_count_t msg;
    mavlink_msg_mission_count_decode(&mavlinkContext.recvMsg, &msg);

    if (!mavlinkMissionTargetIsLocal(msg.target_system, msg.target_component)) {
        return false;
    }
    if (msg.mission_type != MAV_MISSION_TYPE_MISSION) {
        if (mavMissionTransfer.state == MAVLINK_MISSION_TRANSFER_RECEIVING && mavlinkMissionSenderOwnsTransfer()) {
            mavlinkAbortMissionUpload(MAV_MISSION_UNSUPPORTED);
        } else {
            mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED);
        }
        return true;
    }
    if (ARMING_FLAG(ARMED)) {
        mavlinkSendMissionAck(MAV_MISSION_DENIED);
        return true;
    }
    if (mavMissionTransfer.state != MAVLINK_MISSION_TRANSFER_IDLE && !mavlinkMissionSenderOwnsTransfer()) {
        mavlinkSendMissionAck(MAV_MISSION_DENIED);
        return true;
    }
    if (msg.count > NAV_MAX_WAYPOINTS) {
        mavlinkSendMissionAck(MAV_MISSION_NO_SPACE);
        return true;
    }
    if (msg.count == 0) {
        resetWaypointList();
        mavlinkResetMissionTransfer();
        mavlinkContext.missionCompleted = false;
        mavlinkSendMissionAck(mavlinkPersistMission() ? MAV_MISSION_ACCEPTED : MAV_MISSION_ERROR);
        return true;
    }

    mavlinkStartMissionTransfer(MAVLINK_MISSION_TRANSFER_RECEIVING, msg.count);
    mavlinkSendMissionRequest();
    return true;
}

bool mavlinkHandleIncomingMissionItem(void)
{
    mavlink_mission_item_t msg;
    mavlink_msg_mission_item_decode(&mavlinkContext.recvMsg, &msg);

    if (!mavlinkMissionTargetIsLocal(msg.target_system, msg.target_component)) {
        return false;
    }
    if (msg.mission_type != MAV_MISSION_TYPE_MISSION) {
        if (mavMissionTransfer.state == MAVLINK_MISSION_TRANSFER_RECEIVING && mavlinkMissionSenderOwnsTransfer()) {
            mavlinkAbortMissionUpload(MAV_MISSION_UNSUPPORTED);
        } else {
            mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED);
        }
        return true;
    }
    const bool coordinateCommand = msg.command == MAV_CMD_NAV_WAYPOINT ||
        msg.command == MAV_CMD_NAV_LOITER_TIME ||
        msg.command == MAV_CMD_NAV_LAND ||
        msg.command == MAV_CMD_DO_SET_ROI;
    if (coordinateCommand) {
        if (!isfinite(msg.x) || msg.x < -90.0f || msg.x > 90.0f) {
            if (mavMissionTransfer.state == MAVLINK_MISSION_TRANSFER_RECEIVING && mavlinkMissionSenderOwnsTransfer()) {
                mavlinkAbortMissionUpload(MAV_MISSION_INVALID_PARAM5_X);
            } else {
                mavlinkSendMissionAck(MAV_MISSION_INVALID_PARAM5_X);
            }
            return true;
        }
        if (!isfinite(msg.y) || msg.y < -180.0f || msg.y > 180.0f) {
            if (mavMissionTransfer.state == MAVLINK_MISSION_TRANSFER_RECEIVING && mavlinkMissionSenderOwnsTransfer()) {
                mavlinkAbortMissionUpload(MAV_MISSION_INVALID_PARAM6_Y);
            } else {
                mavlinkSendMissionAck(MAV_MISSION_INVALID_PARAM6_Y);
            }
            return true;
        }
    }

    if (ARMING_FLAG(ARMED)) {
        if (msg.command == MAV_CMD_NAV_WAYPOINT) {
            return mavlinkHandleArmedGuidedMissionItem(msg.current, msg.frame,
                MAV_FRAME_SUPPORTED_GLOBAL | MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT,
                (int32_t)lrintf(msg.x * 1e7f), (int32_t)lrintf(msg.y * 1e7f), msg.z);
        }

        mavlinkSendMissionAck(MAV_MISSION_ERROR);
        return true;
    }

    return mavlinkHandleMissionItemCommon(false, msg.frame, msg.command, msg.autocontinue, msg.seq, msg.param1, msg.param2, msg.param3, msg.param4,
        coordinateCommand ? (int32_t)lrintf(msg.x * 1e7f) : 0,
        coordinateCommand ? (int32_t)lrintf(msg.y * 1e7f) : 0,
        msg.z);
}

bool mavlinkHandleIncomingMissionRequestList(void)
{
    mavlink_mission_request_list_t msg;
    mavlink_msg_mission_request_list_decode(&mavlinkContext.recvMsg, &msg);

    if (!mavlinkMissionTargetIsLocal(msg.target_system, msg.target_component)) {
        return false;
    }
    if (msg.mission_type != MAV_MISSION_TYPE_MISSION) {
        if (mavMissionTransfer.state == MAVLINK_MISSION_TRANSFER_RECEIVING && mavlinkMissionSenderOwnsTransfer()) {
            mavlinkAbortMissionUpload(MAV_MISSION_UNSUPPORTED);
        } else {
            mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED);
        }
        return true;
    }
    if (mavMissionTransfer.state != MAVLINK_MISSION_TRANSFER_IDLE && !mavlinkMissionSenderOwnsTransfer()) {
        mavlinkSendMissionAck(MAV_MISSION_DENIED);
        return true;
    }

    const uint16_t count = getWaypointCount();
    mavlinkStartMissionTransfer(MAVLINK_MISSION_TRANSFER_SENDING, count);
    mavlink_msg_mission_count_pack(
        mavSystemId,
        mavComponentId,
        &mavSendMsg,
        mavMissionTransfer.partnerSystem,
        mavMissionTransfer.partnerComponent,
        count,
        MAV_MISSION_TYPE_MISSION,
        0);
    mavlinkSendMessage();
    if (count == 0) {
        mavlinkResetMissionTransfer();
    }
    return true;
}

bool mavlinkFillMissionItemFromWaypoint(const navWaypoint_t *wp, bool useIntMessages, mavlinkMissionItemData_t *item)
{
    mavlinkMissionItemData_t data = {0};

    data.frame = mavlinkWaypointFrame(wp, useIntMessages);

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

bool mavlinkHandleIncomingMissionRequest(void)
{
    mavlink_mission_request_t msg;
    mavlink_msg_mission_request_decode(&mavlinkContext.recvMsg, &msg);

    if (!mavlinkMissionTargetIsLocal(msg.target_system, msg.target_component)) {
        return false;
    }
    if (msg.mission_type != MAV_MISSION_TYPE_MISSION) {
        mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED);
        return true;
    }
    if (mavMissionTransfer.state != MAVLINK_MISSION_TRANSFER_SENDING || !mavlinkMissionSenderOwnsTransfer()) {
        mavlinkSendMissionAck(MAV_MISSION_INVALID_SEQUENCE);
        return true;
    }
    if (msg.seq != mavMissionTransfer.nextSequence &&
        !(mavMissionTransfer.nextSequence > 0 && msg.seq + 1 == mavMissionTransfer.nextSequence)) {
        mavlinkSendMissionAck(MAV_MISSION_INVALID_SEQUENCE);
        mavlinkResetMissionTransfer();
        return true;
    }

    if (msg.seq < mavMissionTransfer.count) {
        navWaypoint_t wp;
        getWaypoint(msg.seq + 1, &wp);

        mavlinkMissionItemData_t item;
        if (mavlinkFillMissionItemFromWaypoint(&wp, false, &item)) {
            mavlink_msg_mission_item_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid,
                msg.seq,
                item.frame,
                item.command,
                FLIGHT_MODE(NAV_WP_MODE) && getActiveWpNumber() == msg.seq + 1,
                1,
                item.param1, item.param2, item.param3, item.param4,
                item.lat / 1e7f,
                item.lon / 1e7f,
                item.alt,
                MAV_MISSION_TYPE_MISSION);
            mavlinkSendMessage();
            if (msg.seq == mavMissionTransfer.nextSequence) {
                mavMissionTransfer.nextSequence++;
            }
            mavMissionTransfer.useIntMessages = false;
            mavMissionTransfer.lastActivityMs = millis();
        } else {
            mavlinkSendMissionAck(MAV_MISSION_ERROR);
            mavlinkResetMissionTransfer();
        }
    } else {
        mavlinkSendMissionAck(MAV_MISSION_INVALID_SEQUENCE);
        mavlinkResetMissionTransfer();
    }

    return true;
}

bool mavlinkHandleIncomingMissionItemInt(void)
{
    mavlink_mission_item_int_t msg;
    mavlink_msg_mission_item_int_decode(&mavlinkContext.recvMsg, &msg);

    if (!mavlinkMissionTargetIsLocal(msg.target_system, msg.target_component)) {
        return false;
    }
    if (msg.mission_type != MAV_MISSION_TYPE_MISSION) {
        if (mavMissionTransfer.state == MAVLINK_MISSION_TRANSFER_RECEIVING && mavlinkMissionSenderOwnsTransfer()) {
            mavlinkAbortMissionUpload(MAV_MISSION_UNSUPPORTED);
        } else {
            mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED);
        }
        return true;
    }

    if (ARMING_FLAG(ARMED)) {
        if (msg.command == MAV_CMD_NAV_WAYPOINT) {
            return mavlinkHandleArmedGuidedMissionItem(msg.current, msg.frame,
                MAV_FRAME_SUPPORTED_GLOBAL_INT | MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT,
                msg.x, msg.y, msg.z);
        }

        mavlinkSendMissionAck(MAV_MISSION_ERROR);
        return true;
    }

    return mavlinkHandleMissionItemCommon(true, msg.frame, msg.command, msg.autocontinue, msg.seq, msg.param1, msg.param2, msg.param3, msg.param4, msg.x, msg.y, msg.z);
}

bool mavlinkHandleIncomingMissionRequestInt(void)
{
    mavlink_mission_request_int_t msg;
    mavlink_msg_mission_request_int_decode(&mavlinkContext.recvMsg, &msg);

    if (!mavlinkMissionTargetIsLocal(msg.target_system, msg.target_component)) {
        return false;
    }
    if (msg.mission_type != MAV_MISSION_TYPE_MISSION) {
        mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED);
        return true;
    }
    if (mavMissionTransfer.state != MAVLINK_MISSION_TRANSFER_SENDING || !mavlinkMissionSenderOwnsTransfer()) {
        mavlinkSendMissionAck(MAV_MISSION_INVALID_SEQUENCE);
        return true;
    }
    if (msg.seq != mavMissionTransfer.nextSequence &&
        !(mavMissionTransfer.nextSequence > 0 && msg.seq + 1 == mavMissionTransfer.nextSequence)) {
        mavlinkSendMissionAck(MAV_MISSION_INVALID_SEQUENCE);
        mavlinkResetMissionTransfer();
        return true;
    }

    if (msg.seq < mavMissionTransfer.count) {
        navWaypoint_t wp;
        getWaypoint(msg.seq + 1, &wp);

        mavlinkMissionItemData_t item;
        if (mavlinkFillMissionItemFromWaypoint(&wp, true, &item)) {
            mavlink_msg_mission_item_int_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid,
                msg.seq,
                item.frame,
                item.command,
                FLIGHT_MODE(NAV_WP_MODE) && getActiveWpNumber() == msg.seq + 1,
                1,
                item.param1, item.param2, item.param3, item.param4,
                item.lat,
                item.lon,
                item.alt,
                MAV_MISSION_TYPE_MISSION);
            mavlinkSendMessage();
            if (msg.seq == mavMissionTransfer.nextSequence) {
                mavMissionTransfer.nextSequence++;
            }
            mavMissionTransfer.useIntMessages = true;
            mavMissionTransfer.lastActivityMs = millis();
        } else {
            mavlinkSendMissionAck(MAV_MISSION_ERROR);
            mavlinkResetMissionTransfer();
        }
    } else {
        mavlinkSendMissionAck(MAV_MISSION_INVALID_SEQUENCE);
        mavlinkResetMissionTransfer();
    }

    return true;
}

bool mavlinkHandleIncomingMissionAck(void)
{
    mavlink_mission_ack_t msg;
    mavlink_msg_mission_ack_decode(&mavlinkContext.recvMsg, &msg);

    if (!mavlinkMissionTargetIsLocal(msg.target_system, msg.target_component)) {
        return false;
    }
    if (mavMissionTransfer.state == MAVLINK_MISSION_TRANSFER_SENDING && mavlinkMissionSenderOwnsTransfer()) {
        mavlinkResetMissionTransfer();
    }
    return true;
}

#endif
