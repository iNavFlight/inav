#pragma once

#include "common/time.h"
#include "config/parameter_group.h"
#include "drivers/dronecan/libcanard/canard.h"

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
    STATE_DRONECAN_BUS_OFF,
    STATE_DRONECAN_FAILED,
    STATE_DRONECAN_COUNT
} dronecanState_e;

#define DRONECAN_MAX_NODES 32 // Reasonably expected number of devices on the bus.  If this is regularly hit, we could go higher but it consumes more ram.

typedef struct dronecanConfig_s {
    uint8_t nodeID;
    dronecanBitrate_e bitRateKbps;
    bool dronecanUseDNAServer;
    uint8_t batteryId;
    uint8_t gpsNodeId;
    // Note: any in-flight branch that adds a field below this line
    // (e.g. feature/dronecan-actuator-control's servoOutputBitmask) must
    // also bump PG_DRONECAN_CONFIG's registered version by 1, and bump
    // EEPROM_CONF_VERSION in src/main/config/config_eeprom.h by 1.
    // Currently at 1/127; do not append fields without both bumps.
} dronecanConfig_t;

typedef struct dronecanNodeInfo_s {
    uint8_t  nodeID;
    uint8_t  health;
    uint8_t  mode;
    uint32_t uptime_sec;
    uint16_t vendor_status_code;
    uint32_t last_seen_ms;
} dronecanNodeInfo_t;

typedef enum {
    DRONECAN_ASYNC_IDLE = 0,
    DRONECAN_ASYNC_PENDING,
    DRONECAN_ASYNC_READY,
    DRONECAN_ASYNC_ERROR,
} dronecanAsyncState_e;

#define DRONECAN_SERVICE_GETNODEINFO     1
#define DRONECAN_SERVICE_RESTART_NODE    5
#define DRONECAN_SERVICE_EXECUTE_OPCODE  10
#define DRONECAN_SERVICE_PARAM_GETSET    11

#define DRONECAN_ASYNC_TIMEOUT_MS      2000
#define DRONECAN_NODE_STALE_TIMEOUT_MS 10000  // Remove node from table if no NodeStatus received for this long
#define DRONECAN_STATE_NOT_READY       0xFF   // MSP sentinel: bus not in STATE_NORMAL; outside dronecanAsyncState_e range

#define DRONECAN_PARAM_TYPE_EMPTY   0
#define DRONECAN_PARAM_TYPE_INT     1
#define DRONECAN_PARAM_TYPE_FLOAT   2
#define DRONECAN_PARAM_TYPE_BOOL    3
#define DRONECAN_PARAM_TYPE_STRING  4

typedef struct dronecanParamRequest_s {
    uint16_t index;
    uint8_t  is_write;
    uint8_t  value_type;
    int64_t  value_int;
    float    value_float;
    uint8_t  value_bool;
    uint8_t  value_str_len;
    char     value_str[128];
    uint8_t  req_name_len;
    char     req_name[92];
} dronecanParamRequest_t;

typedef struct dronecanGetNodeInfoResult_s {
    uint8_t  sw_major;
    uint8_t  sw_minor;
    uint8_t  sw_optional_field_flags;
    uint32_t sw_vcs_commit;
    uint8_t  hw_major;
    uint8_t  hw_minor;
    uint8_t  hw_unique_id[16];
    uint8_t  name_len;
    char     name[81]; // 80 bytes max + null terminator
} dronecanGetNodeInfoResult_t;

typedef struct dronecanParamResult_s {
    uint8_t type;
    int64_t value_int;
    float   value_float;
    uint8_t value_bool;
    uint8_t value_str_len;
    char    value_str[128];
    uint8_t name_len;
    char    name[93]; // 92 bytes max per UAVCAN param.GetSet DSDL + null terminator
    // NumericValue range from the GetSet response; DRONECAN_PARAM_TYPE_EMPTY means not provided.
    // Only INT and FLOAT variants are valid — BOOL and STRING have no numeric range.
    uint8_t min_type;
    int64_t min_int;
    float   min_float;
    uint8_t max_type;
    int64_t max_int;
    float   max_float;
} dronecanParamResult_t;

typedef struct dronecanSimpleResult_s {
    bool ok;
} dronecanSimpleResult_t;

typedef struct dronecanAsyncSlot_s {
    dronecanAsyncState_e state;
    uint8_t  seq;
    uint8_t service_id;
    uint8_t  node_id;
    uint8_t  transfer_id;
    uint32_t requested_at_ms;
    union {
        dronecanGetNodeInfoResult_t node_info;
        dronecanParamResult_t       param;
        dronecanSimpleResult_t      simple;
    } result;
} dronecanAsyncSlot_t;

extern dronecanAsyncSlot_t dronecanAsyncSlot;
bool dronecanAsyncRequest(uint8_t service_id, uint8_t node_id, const void *payload);

void dronecanInit(void);
void dronecanUpdate(timeUs_t currentTimeUs);
dronecanState_e dronecanGetState(void);
uint8_t dronecanGetNodeCount(void);
uint32_t dronecanGetBitrateKbps(void);
const dronecanNodeInfo_t *dronecanGetNode(uint8_t index);
uint32_t dronecanGetBusOffCount(void);
CanardPoolAllocatorStatistics dronecanGetPoolStats(void);
const dronecanNodeInfo_t *dronecanGetNodeByID(uint8_t nodeID);

PG_DECLARE(dronecanConfig_t, dronecanConfig);
