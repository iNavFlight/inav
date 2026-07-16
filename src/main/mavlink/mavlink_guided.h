#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "mavlink/mavlink_types.h"

typedef enum {
    MAV_FRAME_SUPPORTED_NONE = 0,
    MAV_FRAME_SUPPORTED_GLOBAL = (1 << 0),
    MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT = (1 << 1),
    MAV_FRAME_SUPPORTED_GLOBAL_INT = (1 << 2),
    MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT = (1 << 3),
} mavFrameSupportMask_e;

bool mavlinkFrameIsSupported(uint8_t frame, mavFrameSupportMask_e allowedMask);
bool mavlinkFrameUsesAbsoluteAltitude(uint8_t frame);
MAV_RESULT mavlinkSetAltitudeTargetFromFrame(uint8_t frame, float altitudeMeters);
