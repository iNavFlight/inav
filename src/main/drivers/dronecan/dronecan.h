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

#define DRONECAN_MAX_NODES 32 // Reasonably expected number of devices on the bus.  If this is regularly hit, we could go higher but it consumes more ram.

typedef struct dronecanConfig_s {
    uint8_t nodeID;
    dronecanBitrate_e bitRateKbps;
} dronecanConfig_t;

typedef struct dronecanNodeInfo_s {
    uint8_t nodeID;
    uint8_t health;
    uint8_t mode;
    uint32_t uptime_sec;
    uint16_t vendor_status_code;
    uint32_t last_seen_ms;
    uint8_t name_len;
    char name[32];
} dronecanNodeInfo_t;

// Wire format for MSP2_INAV_DRONECAN_NODES records (7 bytes each, packed).
typedef struct dronecanNodeStatus_s {
    uint8_t nodeID;
    uint8_t health;
    uint8_t mode;
    uint32_t last_seen_ms;
} __attribute__((packed)) dronecanNodeStatus_t;

void dronecanInit(void);
void dronecanUpdate(timeUs_t currentTimeUs);
dronecanState_e dronecanGetState(void);                                               
uint8_t dronecanGetNodeCount(void);                                                                                                                                                                                                       
uint32_t dronecanGetBitrateKbps(void);
const dronecanNodeInfo_t *dronecanGetNode(uint8_t index);

PG_DECLARE(dronecanConfig_t, dronecanConfig);
