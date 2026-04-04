#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "navigation/navigation.h"

#include "mavlink/mavlink_types.h"

typedef enum {
    MAV_FRAME_SUPPORTED_NONE = 0,
    MAV_FRAME_SUPPORTED_GLOBAL = (1 << 0),
    MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT = (1 << 1),
    MAV_FRAME_SUPPORTED_GLOBAL_INT = (1 << 2),
    MAV_FRAME_SUPPORTED_GLOBAL_RELATIVE_ALT_INT = (1 << 3),
} mavFrameSupportMask_e;

typedef struct mavlinkMissionItemData_s {
    uint8_t frame;
    uint16_t command;
    float param1;
    float param2;
    float param3;
    float param4;
    int32_t lat;
    int32_t lon;
    float alt;
} mavlinkMissionItemData_t;

uint8_t mavlinkWaypointFrame(const navWaypoint_t *wp, bool useIntMessages);
bool mavlinkFillMissionItemFromWaypoint(const navWaypoint_t *wp, bool useIntMessages, mavlinkMissionItemData_t *item);
void mavlinkSendPendingMissionItemReached(void);
bool mavlinkHandleIncomingMissionClearAll(void);
bool mavlinkHandleIncomingMissionCount(void);
bool mavlinkHandleIncomingMissionItem(void);
bool mavlinkHandleIncomingMissionRequestList(void);
bool mavlinkHandleIncomingMissionRequest(void);
bool mavlinkHandleIncomingMissionItemInt(void);
bool mavlinkHandleIncomingMissionRequestInt(void);
