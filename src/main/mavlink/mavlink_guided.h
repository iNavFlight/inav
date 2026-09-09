#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "mavlink/mavlink_mission.h"

bool mavlinkFrameIsSupported(uint8_t frame, mavFrameSupportMask_e allowedMask);
bool mavlinkFrameUsesAbsoluteAltitude(uint8_t frame);
MAV_RESULT mavlinkSetAltitudeTargetFromFrame(uint8_t frame, float altitudeMeters);

bool mavlinkHandleIncomingSetPositionTargetGlobalInt(void);
bool mavlinkHandleIncomingSetPositionTargetLocalNed(void);
