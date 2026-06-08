#include "mavlink/mavlink_internal.h"

#include "mavlink/mavlink_guided.h"
#include "mavlink/mavlink_runtime.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_MAVLINK)

bool mavlinkFrameIsSupported(uint8_t frame, mavFrameSupportMask_e allowedMask)
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

bool mavlinkFrameUsesAbsoluteAltitude(uint8_t frame)
{
    return frame == MAV_FRAME_GLOBAL || frame == MAV_FRAME_GLOBAL_INT;
}

MAV_RESULT mavlinkSetAltitudeTargetFromFrame(uint8_t frame, float altitudeMeters)
{
#if defined(USE_BARO) || defined(USE_GPS)
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

bool mavlinkHandleIncomingSetPositionTargetGlobalInt(void)
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

bool mavlinkHandleIncomingSetPositionTargetLocalNed(void)
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

#endif
