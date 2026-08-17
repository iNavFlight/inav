#include "platform.h"
#if defined(USE_DRONECAN)

#include <stdint.h>
#include <string.h>

#include "build/atomic.h"

#include "common/log.h"
#include "common/time.h"
#include "common/utils.h"

#include "drivers/nvic.h"
#include "drivers/time.h"

#include "fc/runtime_config.h"

#include "libcanard/canard.h"

#include "sensors/diagnostics.h"

#include <dronecan_msgs.h>

#include "io/gps_dronecan.h"

#include "dronecan.h"
#include "dronecan_node_status.h"

extern CanardInstance canard; /* the FC's own canard instance, owned by dronecan.c */

static struct uavcan_protocol_NodeStatus node_status;

#ifdef UNIT_TEST
uint8_t activeNodeCount = 0;
dronecanNodeInfo_t nodeTable[DRONECAN_MAX_NODES];
#else
static uint8_t activeNodeCount = 0;
static dronecanNodeInfo_t nodeTable[DRONECAN_MAX_NODES];
#endif

static dronecanNodeInfo_t *findNodeByID(uint8_t nodeID)
{
    for (uint8_t i = 0; i < activeNodeCount; i++) {
        if (nodeTable[i].nodeID == nodeID) {
            return &nodeTable[i];
        }
    }
    return NULL;
}

uint8_t dronecanGetNodeCount(void)
{
    return activeNodeCount;
}

const dronecanNodeInfo_t *dronecanGetNode(uint8_t index)
{
    if (index < activeNodeCount) return &nodeTable[index];
    return NULL;
}

const dronecanNodeInfo_t *dronecanGetNodeByID(uint8_t nodeID)
{
    return findNodeByID(nodeID);
}

void dronecanNodeStatusHandleBroadcast(CanardInstance *ins, CanardRxTransfer *transfer)
{
    UNUSED(ins);

    struct uavcan_protocol_NodeStatus nodeStatus;

    if (uavcan_protocol_NodeStatus_decode(transfer, &nodeStatus)) {
        LOG_WARNING(CAN, "NodeStatus decode failed");
        return;
    }

    uint8_t nodeID = transfer->source_node_id;
    dronecanNodeInfo_t *node = findNodeByID(nodeID);
    if (node) {
        node->health = nodeStatus.health;
        node->mode = nodeStatus.mode;
        node->uptime_sec = nodeStatus.uptime_sec;
        node->vendor_status_code = nodeStatus.vendor_specific_status_code;
        node->last_seen_ms = millis();
        return;
    }
    // new node
    if (activeNodeCount < DRONECAN_MAX_NODES) {
        memset(&nodeTable[activeNodeCount], 0, sizeof(dronecanNodeInfo_t));
        nodeTable[activeNodeCount].nodeID = nodeID;
        nodeTable[activeNodeCount].health = nodeStatus.health;
        nodeTable[activeNodeCount].mode = nodeStatus.mode;
        nodeTable[activeNodeCount].uptime_sec = nodeStatus.uptime_sec;
        nodeTable[activeNodeCount].vendor_status_code = nodeStatus.vendor_specific_status_code;
        nodeTable[activeNodeCount].last_seen_ms = millis();
        activeNodeCount++;
    } else {
        LOG_WARNING(CAN, "DroneCAN: node table full (%u nodes), ignoring node %u", DRONECAN_MAX_NODES, nodeID);
    }
}

struct uavcan_protocol_NodeStatus dronecanGetOwnNodeStatus(void)
{
    node_status.uptime_sec = millis() / 1000ULL;
    return node_status;
}

/*
  send the 1Hz NodeStatus message. This is what allows a node to show
  up in the DroneCAN GUI tool and in the flight controller logs
 */
static void send_NodeStatus(void)
{
    uint8_t buffer[UAVCAN_PROTOCOL_NODESTATUS_MAX_SIZE];

    node_status.uptime_sec = millis() / 1000UL;
    if (isHardwareHealthy()) {
        node_status.health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
    } else {
        node_status.health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_CRITICAL;
    }

    node_status.mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;  // Indicates that node is able to communicate over CAN, not that it is in flight.
    node_status.sub_mode = 0; // Not currently used in dronecan

    // put whatever you like in here for display in GUI
    node_status.vendor_specific_status_code = (uint16_t)(armingFlags & 0xFFFF);  /* field is 16-bit by UAVCAN spec; bits 16-30 of armingFlags are not transmitted */

    uint32_t len = uavcan_protocol_NodeStatus_encode(&node_status, buffer);

    // we need a static variable for the transfer ID. This is
    // incremented on each transfer, allowing for detection of packet
    // loss
    static uint8_t transfer_id;

    int16_t bc_res;
    ATOMIC_BLOCK(NVIC_PRIO_CAN) {
        bc_res = canardBroadcast(&canard,
                        UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
                        UAVCAN_PROTOCOL_NODESTATUS_ID,
                        &transfer_id,
                        CANARD_TRANSFER_PRIORITY_LOW,
                        buffer,
                        len);
    }
    if (bc_res < 0) {
        LOG_WARNING(CAN, "NodeStatus broadcast failed: %d", bc_res);
    }
}

void dronecanNodeStatusUpdate(timeUs_t timestamp_usec)
{
    UNUSED(timestamp_usec);

    // Remove nodes that have stopped broadcasting NodeStatus
    for (uint8_t i = 0; i < activeNodeCount; ) {
        if (millis() - nodeTable[i].last_seen_ms > DRONECAN_NODE_STALE_TIMEOUT_MS) {
            dronecanGpsOnNodeEvicted(nodeTable[i].nodeID);
            nodeTable[i] = nodeTable[activeNodeCount - 1];
            activeNodeCount--;
        } else {
            i++;
        }
    }

    send_NodeStatus();
}

#endif // USE_DRONECAN
