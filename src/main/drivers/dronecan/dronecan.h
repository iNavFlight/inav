#pragma once

#include "common/time.h"
#include "config/parameter_group.h"

typedef enum {
    DRONECAN_BITRATE_125KBPS = 0,
    DRONECAN_BITRATE_250KBPS,
    DRONECAN_BITRATE_500KBPS,
    DRONECAN_BITRATE_1000KBPS,
    DRONECAN_BITRATE_COUNT
} dronecanBitrate_e;

typedef enum {
    STATE_DRONECAN_INIT,
    STATE_DRONECAN_NORMAL,
    STATE_DRONECAN_BUS_OFF
} dronecanState_e;

typedef struct dronecanConfig_s {
    uint8_t nodeID;
    dronecanBitrate_e bitRateKbps;
} dronecanConfig_t;


void dronecanInit(void);
void dronecanUpdate(timeUs_t currentTimeUs);
dronecanState_e dronecanGetState(void);                                               
uint8_t dronecanGetNodeCount(void);                                                                                                                                                                                                       
uint32_t dronecanGetBitrateKbps(void);


PG_DECLARE(dronecanConfig_t, dronecanConfig);
