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
    if (!isfinite(altitudeMeters) ||
        altitudeMeters < (float)INT32_MIN / 100.0f ||
        altitudeMeters > (float)INT32_MAX / 100.0f) {
        return MAV_RESULT_DENIED;
    }

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

#endif
