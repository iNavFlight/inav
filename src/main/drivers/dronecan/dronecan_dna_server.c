#include "platform.h"
#include "common/log.h"
#include "drivers/time.h"

#if defined(USE_DRONECAN)

#include <string.h>
#include <dronecan_msgs.h>
#include "fc/config.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"
#include "dronecan.h"
#include "dronecan_dna_server.h"

#define DNA_INVALID_STAGE          -1
#define DNA_STAGE_1                1
#define DNA_STAGE_2                2
#define DNA_STAGE_3                3

#define DNA_STAGE3_UID_LEN  (DNA_UNIQUE_ID_LENGTH - UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MAX_LENGTH_OF_UNIQUE_ID_IN_REQUEST * 2)
_Static_assert(DNA_STAGE3_UID_LEN > 0, "DNA_UNIQUE_ID_LENGTH too small for 3-stage protocol");


PG_REGISTER(dnaServerData_t, dnaServerData, PG_DRONECAN_DNA_SERVER, 0);

static int8_t detectRequestStage(struct uavcan_protocol_dynamic_node_id_Allocation *msg);
static int8_t getExpectedStage(uint8_t currentUniqueIdLength);
static uint8_t dnaLookupOrAssignNode(const uint8_t *uid, uint8_t requestedNodeId);
static void dnaSendResponse(uint8_t nodeId, const uint8_t *uid, uint8_t uidLen);

/*
    Entry point for DroneCAN Dynamic Node Allocation messages.
    Called from onTransferReceived when a UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_ID
    broadcast is received.

    Assembles the complete unique identifier from up to three request stages,
    then assigns or confirms a node ID for the requesting peripheral.
    Follows the UAVCAN specification for non-redundant (single-master) allocators.
*/
void dronecanDnaHandleAllocation(CanardInstance *ins, CanardRxTransfer *transfer)
{
    UNUSED(ins);

    struct uavcan_protocol_dynamic_node_id_Allocation dynamicAllocation;

    static struct {
        uint8_t len;
        uint8_t data[DNA_UNIQUE_ID_LENGTH];
    } currentUniqueId;
    static uint8_t requestedNodeId = CANARD_BROADCAST_NODE_ID;

    static uint32_t lastMessageTimestamp = 0;
    int8_t request_stage;

    if (transfer->source_node_id != CANARD_BROADCAST_NODE_ID)
        return;

    if ((millis() - lastMessageTimestamp) > UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_FOLLOWUP_TIMEOUT_MS) {
        memset(currentUniqueId.data, 0, DNA_UNIQUE_ID_LENGTH);
        currentUniqueId.len = 0;
        requestedNodeId = CANARD_BROADCAST_NODE_ID;
    }

    if (uavcan_protocol_dynamic_node_id_Allocation_decode(transfer, &dynamicAllocation)) {
        LOG_ERROR(CAN, "DNA decode failed");
        return;
    }

    request_stage = detectRequestStage(&dynamicAllocation);
    if (request_stage == DNA_INVALID_STAGE)
    {
        LOG_WARNING(CAN, "DNA malformed request (invalid stage)");
        return;
    }
    const int8_t expected_stage = getExpectedStage(currentUniqueId.len);
    if (request_stage != expected_stage)
    {
        LOG_WARNING(CAN, "DNA stage mismatch (got %d, expected %d)", request_stage, expected_stage);
        return;
    }
    if (request_stage == DNA_STAGE_1)
        requestedNodeId = dynamicAllocation.node_id;   // Store requested node id as it's only on stage 1

    if (dynamicAllocation.unique_id.len > DNA_UNIQUE_ID_LENGTH - currentUniqueId.len)
    {
        LOG_WARNING(CAN, "DNA malformed request - UID exceeds remaining capacity");
        return;
    }

    memcpy(currentUniqueId.data + currentUniqueId.len, dynamicAllocation.unique_id.data, dynamicAllocation.unique_id.len);
    currentUniqueId.len += dynamicAllocation.unique_id.len;

    if (currentUniqueId.len == DNA_UNIQUE_ID_LENGTH)
    {
        uint8_t assignedNodeId = dnaLookupOrAssignNode(currentUniqueId.data, requestedNodeId);
        if (assignedNodeId != 0) {
            LOG_INFO(CAN, "DNA assigned Node ID: %u to peripheral", assignedNodeId);
            dnaSendResponse(assignedNodeId, currentUniqueId.data, currentUniqueId.len);
        }
        memset(currentUniqueId.data, 0, DNA_UNIQUE_ID_LENGTH);
        currentUniqueId.len = 0;
        requestedNodeId = CANARD_BROADCAST_NODE_ID;
    }
    else
    {
        dnaSendResponse(0, currentUniqueId.data, currentUniqueId.len);
    }

    lastMessageTimestamp = millis();
}

static void dnaSendResponse(uint8_t nodeId, const uint8_t *uid, uint8_t uidLen)
{
    struct uavcan_protocol_dynamic_node_id_Allocation msg;
    uint8_t buffer[UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MAX_SIZE];
    static uint8_t transferId;
    CanardTxTransfer outboundTransfer;

    msg.node_id = nodeId;
    msg.first_part_of_unique_id = 0;
    memcpy(msg.unique_id.data, uid, uidLen);
    msg.unique_id.len = uidLen;

    uint32_t len = uavcan_protocol_dynamic_node_id_Allocation_encode(&msg, buffer);

    canardInitTxTransfer(&outboundTransfer);
    outboundTransfer.data_type_signature = UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_SIGNATURE;
    outboundTransfer.data_type_id        = UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_ID;
    outboundTransfer.inout_transfer_id   = &transferId;
    outboundTransfer.priority            = CANARD_TRANSFER_PRIORITY_LOW;
    outboundTransfer.payload             = buffer;
    outboundTransfer.payload_len         = (uint16_t)len;

    const int16_t res = canardBroadcastObj(&canard, &outboundTransfer);
    if (res < 0) {
        LOG_WARNING(CAN, "DNA: response broadcast failed: %d", res);
    }
}

static bool isNodeAvailable(uint8_t assignedNodeId)
{
    if (assignedNodeId == canard.node_id)
        return false;  // Already assigned to this node

    for (uint8_t i = 0; i < DRONECAN_MAX_NODES; i++) {
        if (assignedNodeId == dnaServerData()->entries[i].nodeId) {
            return false;  // Already assigned to another node in the allocation table
        }
    }
    // the live node table is only populated from NodeStatus broadcasts (1 Hz), so at 
    // power-up the allocator may not have heard from all static-ID nodes yet. This is 
    // a known limitation of non-redundant allocators and is accepted in the spec — 
    // the downward-from-125 assignment strategy naturally reduces collisions with 
    // manually assigned low IDs.
    for (uint8_t i = 0; i < dronecanGetNodeCount(); i++) {
          const dronecanNodeInfo_t *node = dronecanGetNode(i);
          if (node && node->nodeID == assignedNodeId)
              return false;  // Already in use on the network
      }

    return true;
}
/*
    Search the allocation table for an existing entry matching the given unique
    identifier. If found, return the previously assigned node ID. If not found,
    find the first unused node ID (skipping our own) and record a new allocation.
    Returns 0 if the table is full.
*/
static uint8_t dnaLookupOrAssignNode(const uint8_t *uid, uint8_t requestedNodeId)
{
    uint8_t  assignedNodeId = CANARD_BROADCAST_NODE_ID;
    int8_t   conflictIdx = -1;
    int8_t   writeIdx;
    uint8_t  storedId;
    bool     inUse;

    for (int i = 0; i < DRONECAN_MAX_NODES; i++) {
        if (dnaServerData()->entries[i].nodeId != 0 &&
                memcmp(dnaServerData()->entries[i].uniqueId, uid, DNA_UNIQUE_ID_LENGTH) == 0) {
            LOG_DEBUG(CAN, "DNA: found existing allocation for node %u", dnaServerData()->entries[i].nodeId);
            storedId = dnaServerData()->entries[i].nodeId;
            inUse = (storedId == canard.node_id);
            for (uint8_t j = 0; !inUse && j < dronecanGetNodeCount(); j++) {
                const dronecanNodeInfo_t *node = dronecanGetNode(j);
                if (node && node->nodeID == storedId)
                    inUse = true;
            }
            if (!inUse)
                return storedId;
            LOG_WARNING(CAN, "DNA: stored node ID %u for UID is already in use — re-assigning", storedId);
            conflictIdx = i;
            break;
        }
    }
    if (requestedNodeId >= CANARD_MIN_NODE_ID && requestedNodeId <= DRONECAN_DNA_MAX_NODE_ID) {
        /* Search upward from preferred ID first (per UAVCAN spec) */
        for (uint8_t id = requestedNodeId; id <= DRONECAN_DNA_MAX_NODE_ID; id++) {
            if (isNodeAvailable(id)) {
                assignedNodeId = id;
                break;
            }
        }
        /* If nothing found upward, search downward from preferred - 1 */
        if (assignedNodeId == CANARD_BROADCAST_NODE_ID && requestedNodeId > CANARD_MIN_NODE_ID) {
            for (uint8_t id = requestedNodeId - 1; id >= CANARD_MIN_NODE_ID; id--) {
                if (isNodeAvailable(id)) {
                    assignedNodeId = id;
                    break;
                }
            }
        }
    }
    /* No preference or preferred range exhausted → top-down from 125 */
    if (assignedNodeId == CANARD_BROADCAST_NODE_ID) {
        for (assignedNodeId = DRONECAN_DNA_MAX_NODE_ID; assignedNodeId >= CANARD_MIN_NODE_ID; assignedNodeId--) {
            if (isNodeAvailable(assignedNodeId))
                break;
        }
    }
    if (assignedNodeId < CANARD_MIN_NODE_ID) {
        LOG_ERROR(CAN, "DNA: no free node IDs available");
        return 0;
    }
    writeIdx = conflictIdx; // Overwrite if already in table
    if(writeIdx < 0) {
        for (int i = 0; i < DRONECAN_MAX_NODES; i++) {
            if (dnaServerData()->entries[i].nodeId == 0) {
                writeIdx = i;
                break;
            }
        }
    }
    if (writeIdx < 0) { 
        LOG_ERROR(CAN, "DNA: allocation table full");
        return 0;
    }
    memcpy(dnaServerDataMutable()->entries[writeIdx].uniqueId, uid, DNA_UNIQUE_ID_LENGTH);
    dnaServerDataMutable()->entries[writeIdx].nodeId = assignedNodeId;
    LOG_INFO(CAN, "DNA added node %u (UID index %u)", assignedNodeId, writeIdx);
    saveConfig();
    return assignedNodeId;    
}

/*
    Determine which stage of the three-part allocation handshake we expect
    next, based on how many bytes of the unique identifier have been accumulated.
*/
static int8_t getExpectedStage(uint8_t currentUniqueIdLength)
{
    if (currentUniqueIdLength == 0)
        return DNA_STAGE_1;
    if (currentUniqueIdLength >= (UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MAX_LENGTH_OF_UNIQUE_ID_IN_REQUEST * 2))
        return DNA_STAGE_3;
    if (currentUniqueIdLength >= UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MAX_LENGTH_OF_UNIQUE_ID_IN_REQUEST)
        return DNA_STAGE_2;
    return DNA_INVALID_STAGE;
}

/*
    Classify an incoming allocation request by its stage based on the
    first_part_of_unique_id flag and the unique_id payload length.
    Returns DNA_STAGE_1, DNA_STAGE_2, DNA_STAGE_3, or DNA_INVALID_STAGE.
*/
static int8_t detectRequestStage(struct uavcan_protocol_dynamic_node_id_Allocation *msg)
{
    if ((msg->unique_id.len != UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MAX_LENGTH_OF_UNIQUE_ID_IN_REQUEST) &&
        (msg->unique_id.len != (uint8_t)DNA_STAGE3_UID_LEN) &&
        (msg->unique_id.len != DNA_UNIQUE_ID_LENGTH))
    {
        return DNA_INVALID_STAGE;
    }
    if (msg->first_part_of_unique_id)
    {
        return DNA_STAGE_1;
    }
    if (msg->unique_id.len == UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MAX_LENGTH_OF_UNIQUE_ID_IN_REQUEST)
    {
        return DNA_STAGE_2;
    }
    if (msg->unique_id.len == (uint8_t)DNA_STAGE3_UID_LEN)
    {
        return DNA_STAGE_3;
    }
    return DNA_INVALID_STAGE;
}

#endif // USE_DRONECAN
