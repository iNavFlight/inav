#pragma once

#include "common/time.h"
#include "config/parameter_group.h"

void dronecanInit(void);
void dronecanUpdate(timeUs_t currentTimeUs);

typedef enum {
    DRONECAN_BITRATE_125KBPS = 0,
    DRONECAN_BITRATE_250KBPS,
    DRONECAN_BITRATE_500KBPS,
    DRONECAN_BITRATE_1000KBPS,
    DRONECAN_BITRATE_COUNT
} dronecanBitrate_e;

typedef struct dronecanConfig_s {
    uint8_t nodeID;
    dronecanBitrate_e bitRateKbps;
} dronecanConfig_t;

PG_DECLARE(dronecanConfig_t, dronecanConfig);
