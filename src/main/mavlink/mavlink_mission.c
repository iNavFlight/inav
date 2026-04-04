#include "mavlink/mavlink_internal.h"

#include "mavlink/mavlink_guided.h"
#include "mavlink/mavlink_mission.h"
#include "mavlink/mavlink_runtime.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_MAVLINK)

void mavlinkSendPendingMissionItemReached(void)
{
    uint16_t seq;
    if (!navigationConsumeWaypointReached(&seq)) {
        return;
    }

    uint8_t sendMask = 0;
    for (uint8_t portIndex = 0; portIndex < mavPortCount; portIndex++) {
        if (mavPortStates[portIndex].telemetryEnabled && mavPortStates[portIndex].port) {
            sendMask |= MAVLINK_PORT_MASK(portIndex);
        }
    }

    if (sendMask == 0) {
        return;
    }

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


static void mavlinkSendMissionAck(MAV_MISSION_RESULT result)
{
    mavlink_msg_mission_ack_pack(
        mavSystemId,
        mavComponentId,
        &mavSendMsg,
        mavlinkContext.recvMsg.sysid,
        mavlinkContext.recvMsg.compid,
        result,
        MAV_MISSION_TYPE_MISSION,
        0
    );
    mavlinkSendMessage();
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
    if (!mavlinkIsIncomingMissionTransactionActive() || !mavlinkIsIncomingMissionTransactionOwner()) {
        mavlinkSendMissionAck(MAV_MISSION_INVALID_SEQUENCE);
        return true;
    }

    mavlinkTouchIncomingMissionTransaction();

    const bool lastMissionItem = incomingMissionWpCount > 0 && ((int)seq + 1 >= incomingMissionWpCount);

    if (autocontinue == 0 && !lastMissionItem) {
        mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED);
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
                mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED_FRAME);
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
                mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED_FRAME);
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
                    mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED_FRAME);
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
                mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED_FRAME);
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
                mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED_FRAME);
                return true;
            }
            if (param1 < 0.0f) {
                mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED);
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
                mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED);
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
                mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED_FRAME);
                return true;
            }
            if (param4 != 0.0f) {
                mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED);
                return true;
            }
            wp.action = NAV_WP_ACTION_SET_HEAD;
            wp.p1 = (int16_t)lrintf(param1);
            break;

        default:
            mavlinkSendMissionAck(MAV_MISSION_UNSUPPORTED);
            return true;
    }

    if (seq == incomingMissionWpSequence) {
        incomingMissionWpSequence++;
        wp.flag = (incomingMissionWpSequence >= incomingMissionWpCount) ? NAV_WP_FLAG_LAST : 0;
        setWaypoint(incomingMissionWpSequence, &wp);

        if (incomingMissionWpSequence >= incomingMissionWpCount) {
            if (isWaypointListValid()) {
                mavlinkSendMissionAck(MAV_MISSION_ACCEPTED);
            } else {
                mavlinkSendMissionAck(MAV_MISSION_INVALID);
            }
            mavlinkResetIncomingMissionTransaction();
        } else {
            if (useIntMessages) {
                mavlink_msg_mission_request_int_pack(
                    mavSystemId, 
                    mavComponentId, 
                    &mavSendMsg,
                    mavlinkContext.recvMsg.sysid, 
                    mavlinkContext.recvMsg.compid, 
                    incomingMissionWpSequence, 
                    MAV_MISSION_TYPE_MISSION);
            } else {
                mavlink_msg_mission_request_pack(
                    mavSystemId, 
                    mavComponentId, 
                    &mavSendMsg, 
                    mavlinkContext.recvMsg.sysid, 
                    mavlinkContext.recvMsg.compid, 
                    incomingMissionWpSequence, 
                    MAV_MISSION_TYPE_MISSION);
            }
            mavlinkSendMessage();
        }
    } else {
        if (seq + 1 == incomingMissionWpSequence) {
            mavlinkTouchIncomingMissionTransaction();
            if (useIntMessages) {
                mavlink_msg_mission_request_int_pack(
                    mavSystemId, 
                    mavComponentId, 
                    &mavSendMsg,
                    mavlinkContext.recvMsg.sysid, 
                    mavlinkContext.recvMsg.compid, 
                    incomingMissionWpSequence, 
                    MAV_MISSION_TYPE_MISSION);
            } else {
                mavlink_msg_mission_request_pack(
                    mavSystemId, 
                    mavComponentId, 
                    &mavSendMsg, 
                    mavlinkContext.recvMsg.sysid, 
                    mavlinkContext.recvMsg.compid, 
                    incomingMissionWpSequence, 
                    MAV_MISSION_TYPE_MISSION);
            }
            mavlinkSendMessage();
        } else {
            mavlinkSendMissionAck(MAV_MISSION_INVALID_SEQUENCE);
        }
    }

    return true;
}

bool mavlinkHandleIncomingMissionClearAll(void)
{
    mavlink_mission_clear_all_t msg;
    mavlink_msg_mission_clear_all_decode(&mavlinkContext.recvMsg, &msg);

    if (msg.target_system == mavSystemId) {
        resetWaypointList();
        mavlinkResetIncomingMissionTransaction();
        mavlinkSendMissionAck(MAV_MISSION_ACCEPTED);
        return true;
    }

    return false;
}

bool mavlinkHandleIncomingMissionCount(void)
{
    mavlink_mission_count_t msg;
    mavlink_msg_mission_count_decode(&mavlinkContext.recvMsg, &msg);

    if (msg.target_system == mavSystemId) {
        mavlinkResetIncomingMissionTransaction();
        if (ARMING_FLAG(ARMED)) {
            mavlinkSendMissionAck(MAV_MISSION_ERROR);
            return true;
        }
        if (msg.count == 0) {
            resetWaypointList();
            mavlinkSendMissionAck(MAV_MISSION_ACCEPTED);
            return true;
        }
        if (msg.count <= NAV_MAX_WAYPOINTS) {
            mavlinkStartIncomingMissionTransaction(msg.count);
            mavlink_msg_mission_request_int_pack(
                mavSystemId, 
                mavComponentId, 
                &mavSendMsg,
                mavlinkContext.recvMsg.sysid, 
                mavlinkContext.recvMsg.compid, 
                incomingMissionWpSequence, 
                MAV_MISSION_TYPE_MISSION);
            mavlinkSendMessage();
            return true;
        }

        mavlinkSendMissionAck(MAV_MISSION_NO_SPACE);
        return true;
    }

    return false;
}

bool mavlinkHandleIncomingMissionItem(void)
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

        mavlinkSendMissionAck(MAV_MISSION_ERROR);
        return true;
    }

    return mavlinkHandleMissionItemCommon(false, msg.frame, msg.command, msg.autocontinue, msg.seq, msg.param1, msg.param2, msg.param3, msg.param4, (int32_t)(msg.x * 1e7f), (int32_t)(msg.y * 1e7f), msg.z);
}

bool mavlinkHandleIncomingMissionRequestList(void)
{
    mavlink_mission_request_list_t msg;
    mavlink_msg_mission_request_list_decode(&mavlinkContext.recvMsg, &msg);

    if (msg.target_system == mavSystemId) {
        mavlink_msg_mission_count_pack(mavSystemId, mavComponentId, &mavSendMsg, mavlinkContext.recvMsg.sysid, mavlinkContext.recvMsg.compid, getWaypointCount(), MAV_MISSION_TYPE_MISSION, 0);
        mavlinkSendMessage();
        return true;
    }

    return false;
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

    if (msg.target_system != mavSystemId) {
        return false;
    }

    const int wpCount = getWaypointCount();

    if (msg.seq < wpCount) {
        navWaypoint_t wp;
        getWaypoint(msg.seq + 1, &wp);

        mavlinkMissionItemData_t item;
        if (mavlinkFillMissionItemFromWaypoint(&wp, false, &item)) {
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
            mavlinkSendMissionAck(MAV_MISSION_ERROR);
        }
    } else {
        mavlinkSendMissionAck(MAV_MISSION_INVALID_SEQUENCE);
    }

    return true;
}

bool mavlinkHandleIncomingMissionItemInt(void)
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

        mavlinkSendMissionAck(MAV_MISSION_ERROR);
        return true;
    }

    return mavlinkHandleMissionItemCommon(true, msg.frame, msg.command, msg.autocontinue, msg.seq, msg.param1, msg.param2, msg.param3, msg.param4, msg.x, msg.y, msg.z);
}

bool mavlinkHandleIncomingMissionRequestInt(void)
{
    mavlink_mission_request_int_t msg;
    mavlink_msg_mission_request_int_decode(&mavlinkContext.recvMsg, &msg);

    if (msg.target_system != mavSystemId) {
        return false;
    }

    const int wpCount = getWaypointCount();

    if (msg.seq < wpCount) {
        navWaypoint_t wp;
        getWaypoint(msg.seq + 1, &wp);

        mavlinkMissionItemData_t item;
        if (mavlinkFillMissionItemFromWaypoint(&wp, true, &item)) {
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
            mavlinkSendMissionAck(MAV_MISSION_ERROR);
        }
    } else {
        mavlinkSendMissionAck(MAV_MISSION_INVALID_SEQUENCE);
    }

    return true;
}

#endif
