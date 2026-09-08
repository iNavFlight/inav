/**
 * DroneCAN DNA Server Unit Tests
 *
 * Tests the three-part UID accumulation, node ID assignment, and allocation
 * table management in dronecan_dna_server.c compiled against real DSDL
 * encode/decode and INAV stubs.
 *
 * Coverage:
 *   DNA-1  Full 3-stage handshake → entry stored with correct UID and node ID
 *   DNA-2  Same UID on second handshake → same node ID returned, no new entry
 *   DNA-3  Two different UIDs → different node IDs assigned
 *   DNA-4  Table full (DRONECAN_MAX_NODES entries) → no allocation for next node
 *   DNA-5  Non-broadcast source node ID → transfer silently rejected
 *   DNA-6  Stage 2 without prior Stage 1 → stage mismatch, rejected
 *   DNA-7  Followup timeout mid-handshake → accumulator reset, Stage 2 rejected
 *   DNA-8  FC's own node ID is never assigned to a peripheral
 *   DNA-9  Peripheral requests specific node ID → honoured when available
 *   DNA-10 First sequential assignment is 125 (top-down per spec)
 *   DNA-11 Preferred ID in reserved range (126-127) → falls back to sequential
 *   DNA-12 Preferred ID already taken → falls back to sequential
 *   DNA-13 Stored ID in use on live network → reassigned, table entry updated
 *   DNA-14 Full 16-byte UID in a single stage-1 message → completes immediately
 *   DNA-15 Full-length message without the stage-1 flag → rejected
 *   DNA-16 Malformed 4-byte stage-1 message rejected, doesn't block real handshake
 *   DNA-17 Reboot within the live-node stale window re-issues the stored ID
 */

#include "gtest/gtest.h"

extern "C" {
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "platform.h"

#include "uavcan.protocol.dynamic_node_id.Allocation.h"
#include "drivers/dronecan/libcanard/canard.h"
#include "drivers/dronecan/dronecan.h"
#include "drivers/dronecan/dronecan_dna_server.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"
#include "common/log.h"

/* Global canard instance — extern-declared in dronecan_dna_server.h.
   PG_REGISTER in dronecan_dna_server.c already emits dnaServerData_System
   and dnaServerData_Copy, so we must NOT define them here. */
CanardInstance canard;

/* Controllable time source */
static uint32_t mock_time_ms = 0;
uint32_t millis(void) { return mock_time_ms; }

/* saveConfig — called when a new allocation is persisted.
   Tracked so DNA-17 can assert that a reboot within the stale window
   does NOT trigger a write (re-issuing the stored ID is not a
   configuration change). */
static int saveConfig_call_count = 0;
void saveConfig(void) { saveConfig_call_count++; }

/* dronecanNodeStatusRegisterNode — called by the DNA server after a
   successful allocation to pre-create a live-node entry so the
   peripheral's first NodeStatus refreshes it in place. The stub adds
   an entry to mock_node_table keyed by nodeID with the given dnaIdx,
   mirroring what the production code does in
   dronecanNodeStatusHandleBroadcast(). */
void dronecanNodeStatusRegisterNode(uint8_t nodeID, uint8_t dnaIdx);

/* Logging — USE_LOG is unconditionally defined by target/common.h (pulled in
   via platform.h), so LOG_ERROR/LOG_WARNING/LOG_DEBUG in dronecan_dna_server.c
   expand to real _logf() calls. Stubbed as a no-op rather than linking
   common/log.c, which would pull in unrelated production dependencies
   (serial.h, msp.h, msp_serial.h, fc/config.h, config/feature.h). */
void _logf(logTopic_e topic, unsigned level, const char *fmt, ...) { (void)topic; (void)level; (void)fmt; }

/* Controllable live node table — populated by tests that need it (DNA-13).
   All other tests leave mock_node_count = 0 so the allocator sees no live nodes. */
static dronecanNodeInfo_t mock_node_table[DRONECAN_MAX_NODES];
static uint8_t            mock_node_count = 0;

uint8_t dronecanGetNodeCount(void) { return mock_node_count; }
const dronecanNodeInfo_t *dronecanGetNode(uint8_t index) {
    if (index >= mock_node_count) return NULL;
    return &mock_node_table[index];
}

void dronecanNodeStatusRegisterNode(uint8_t nodeID, uint8_t dnaIdx) {
    /* If already present, update dnaIdx in place. */
    for (uint8_t i = 0; i < mock_node_count; i++) {
        if (mock_node_table[i].nodeID == nodeID) {
            mock_node_table[i].dnaIdx = dnaIdx;
            return;
        }
    }
    if (mock_node_count >= DRONECAN_MAX_NODES) return;
    memset(&mock_node_table[mock_node_count], 0, sizeof(dronecanNodeInfo_t));
    mock_node_table[mock_node_count].nodeID = nodeID;
    mock_node_table[mock_node_count].dnaIdx = dnaIdx;
    mock_node_table[mock_node_count].last_seen_ms = mock_time_ms;
    mock_node_count++;
}

} /* extern "C" */

/* =========================================================================
 * Constants
 * ========================================================================= */

#define FC_NODE_ID  5u
#define MAX_LEN     UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MAX_LENGTH_OF_UNIQUE_ID_IN_REQUEST
#define FOLLOWUP_MS UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_FOLLOWUP_TIMEOUT_MS

/* =========================================================================
 * Helpers
 * ========================================================================= */

/* Build a broadcast Allocation transfer from an anonymous peripheral */
static CanardRxTransfer makeAllocationTransfer(
        uint8_t        requested_node_id,
        bool           first_part,
        const uint8_t *uid,
        uint8_t        uid_len,
        uint8_t       *buf)
{
    struct uavcan_protocol_dynamic_node_id_Allocation msg;
    memset(&msg, 0, sizeof(msg));
    msg.node_id                 = requested_node_id;
    msg.first_part_of_unique_id = first_part;
    msg.unique_id.len           = uid_len;
    memcpy(msg.unique_id.data, uid, uid_len);

    uint32_t len = uavcan_protocol_dynamic_node_id_Allocation_encode(&msg, buf);

    CanardRxTransfer xfer;
    memset(&xfer, 0, sizeof(xfer));
    xfer.transfer_type  = CanardTransferTypeBroadcast;
    xfer.data_type_id   = UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_ID;
    xfer.source_node_id = CANARD_BROADCAST_NODE_ID;
    xfer.payload_head   = buf;
    xfer.payload_len    = (uint16_t)len;
    return xfer;
}

/* Run a complete 3-stage handshake for the given 16-byte UID.
   Stages are sent 100 ms apart (well within the 500 ms followup timeout).
   Returns the node_id stored in the PG allocation table for this UID,
   or 0 if no entry was created. */
static uint8_t runHandshake(const uint8_t uid[16], uint8_t requested_node_id = 0)
{
    uint8_t buf[UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MAX_SIZE + 4];
    CanardRxTransfer xfer;

    /* Stage 1: first MAX_LEN bytes, first_part=true */
    xfer = makeAllocationTransfer(requested_node_id, true, uid, MAX_LEN, buf);
    dronecanDnaHandleAllocation(NULL, &xfer);
    mock_time_ms += 100;

    /* Stage 2: next MAX_LEN bytes, first_part=false */
    xfer = makeAllocationTransfer(0, false, uid + MAX_LEN, MAX_LEN, buf);
    dronecanDnaHandleAllocation(NULL, &xfer);
    mock_time_ms += 100;

    /* Stage 3: remaining bytes, first_part=false */
    xfer = makeAllocationTransfer(0, false, uid + MAX_LEN * 2,
                                  (uint8_t)(16 - MAX_LEN * 2), buf);
    dronecanDnaHandleAllocation(NULL, &xfer);
    mock_time_ms += 100;

    /* Look up the result in the PG allocation table */
    for (int i = 0; i < DRONECAN_MAX_NODES; i++) {
        if (dnaServerData()->entries[i].nodeId != 0 &&
            memcmp(dnaServerData()->entries[i].uniqueId, uid, 16) == 0) {
            return dnaServerData()->entries[i].nodeId;
        }
    }
    return 0;
}

/* =========================================================================
 * Fixture
 * ========================================================================= */

class DroneCANDnaServerTest : public ::testing::Test {
protected:
    static uint32_t s_base_ms;
    uint8_t canard_pool[512];

    void SetUp() override {
        /* Jump 10 seconds forward each test so the DNA server's followup
           timeout fires on the very first call, resetting any accumulated
           UID state left by the previous test. */
        s_base_ms   += 10000;
        mock_time_ms = s_base_ms;

        memset(dnaServerDataMutable(), 0, sizeof(dnaServerData_t));
        memset(mock_node_table, 0, sizeof(mock_node_table));
        mock_node_count = 0;

        canardInit(&canard, canard_pool, sizeof(canard_pool),
                   NULL, NULL, NULL);
        canardSetLocalNodeID(&canard, FC_NODE_ID);
    }
};
uint32_t DroneCANDnaServerTest::s_base_ms = 0;

/* =========================================================================
 * DNA-1: Full 3-stage handshake stores entry with correct UID and node ID
 * ========================================================================= */
TEST_F(DroneCANDnaServerTest, FullHandshakeAssignsNodeId)
{
    const uint8_t uid[16] = {
        0x01,0x02,0x03,0x04,0x05,0x06,
        0x07,0x08,0x09,0x0A,0x0B,0x0C,
        0x0D,0x0E,0x0F,0x10
    };

    uint8_t assigned = runHandshake(uid);

    EXPECT_NE(assigned, 0u);
    EXPECT_NE(assigned, FC_NODE_ID);
    EXPECT_LT(assigned, (uint8_t)CANARD_MAX_NODE_ID);

    /* Verify the table entry contains the correct UID */
    bool found = false;
    for (int i = 0; i < DRONECAN_MAX_NODES; i++) {
        if (dnaServerData()->entries[i].nodeId == assigned) {
            EXPECT_EQ(0, memcmp(dnaServerData()->entries[i].uniqueId, uid, 16));
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "Assigned ID not present in allocation table";
}

/* =========================================================================
 * DNA-2: Same UID on second handshake → same node ID, no new table entry
 * ========================================================================= */
TEST_F(DroneCANDnaServerTest, SameUidReturnsSameNodeId)
{
    const uint8_t uid[16] = {
        0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,
        0x11,0x22,0x33,0x44,0x55,0x66,
        0x77,0x88,0x99,0x00
    };

    uint8_t first = runHandshake(uid);
    ASSERT_NE(first, 0u);

    s_base_ms   += 10000;
    mock_time_ms = s_base_ms;

    uint8_t second = runHandshake(uid);

    EXPECT_EQ(first, second) << "Same UID must return same node ID on re-negotiation";

    int count = 0;
    for (int i = 0; i < DRONECAN_MAX_NODES; i++) {
        if (dnaServerData()->entries[i].nodeId != 0 &&
            memcmp(dnaServerData()->entries[i].uniqueId, uid, 16) == 0) {
            count++;
        }
    }
    EXPECT_EQ(count, 1) << "Re-negotiation must not create a second table entry";
}

/* =========================================================================
 * DNA-3: Two different UIDs receive different node IDs
 * ========================================================================= */
TEST_F(DroneCANDnaServerTest, TwoDifferentUidsGetDifferentNodeIds)
{
    const uint8_t uid1[16] = {
        0x01,0x01,0x01,0x01,0x01,0x01,
        0x01,0x01,0x01,0x01,0x01,0x01,
        0x01,0x01,0x01,0x01
    };
    const uint8_t uid2[16] = {
        0x02,0x02,0x02,0x02,0x02,0x02,
        0x02,0x02,0x02,0x02,0x02,0x02,
        0x02,0x02,0x02,0x02
    };

    uint8_t id1 = runHandshake(uid1);
    ASSERT_NE(id1, 0u);

    s_base_ms   += 10000;
    mock_time_ms = s_base_ms;

    uint8_t id2 = runHandshake(uid2);
    ASSERT_NE(id2, 0u);

    EXPECT_NE(id1, id2);
}

/* =========================================================================
 * DNA-4: Table full → the next peripheral receives no allocation
 * ========================================================================= */
TEST_F(DroneCANDnaServerTest, TableFullReturnsNoAllocation)
{
    /* Fill every slot with distinct fake UIDs and valid node IDs,
       skipping FC_NODE_ID in the assigned sequence. */
    uint8_t nextId = CANARD_MIN_NODE_ID;
    for (int i = 0; i < DRONECAN_MAX_NODES; i++) {
        if (nextId == FC_NODE_ID) nextId++;
        dnaServerDataMutable()->entries[i].nodeId = nextId++;
        memset(dnaServerDataMutable()->entries[i].uniqueId, (uint8_t)(i + 1), 16);
    }

    const uint8_t uid[16] = {
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF
    };

    uint8_t assigned = runHandshake(uid);
    EXPECT_EQ(assigned, 0u) << "Full table must not produce an allocation";

    for (int i = 0; i < DRONECAN_MAX_NODES; i++) {
        EXPECT_NE(0, memcmp(dnaServerData()->entries[i].uniqueId, uid, 16))
            << "Full table must not write a new entry at slot " << i;
    }
}

/* =========================================================================
 * DNA-5: Non-broadcast source node ID → rejected before any processing
 * ========================================================================= */
TEST_F(DroneCANDnaServerTest, NonBroadcastSourceRejected)
{
    uint8_t buf[UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MAX_SIZE + 4];
    const uint8_t uid[16] = {
        0xDE,0xAD,0xBE,0xEF,0xDE,0xAD,
        0xBE,0xEF,0xDE,0xAD,0xBE,0xEF,
        0xDE,0xAD,0xBE,0xEF
    };

    CanardRxTransfer xfer = makeAllocationTransfer(0, true, uid, MAX_LEN, buf);
    xfer.source_node_id = 42;   /* non-anonymous — must be rejected */
    dronecanDnaHandleAllocation(NULL, &xfer);

    for (int i = 0; i < DRONECAN_MAX_NODES; i++) {
        EXPECT_EQ(dnaServerData()->entries[i].nodeId, 0u)
            << "Non-broadcast source must not create a table entry";
    }
}

/* =========================================================================
 * DNA-6: Stage 2 without a preceding Stage 1 → stage mismatch, rejected
 * ========================================================================= */
TEST_F(DroneCANDnaServerTest, Stage2WithoutStage1Rejected)
{
    uint8_t buf[UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MAX_SIZE + 4];
    const uint8_t uid[16] = {
        0xCA,0xFE,0xBA,0xBE,0xCA,0xFE,
        0xBA,0xBE,0xCA,0xFE,0xBA,0xBE,
        0xCA,0xFE,0xBA,0xBE
    };

    /* first_part=false and len=MAX_LEN is a Stage 2 message */
    CanardRxTransfer xfer = makeAllocationTransfer(0, false, uid + MAX_LEN, MAX_LEN, buf);
    dronecanDnaHandleAllocation(NULL, &xfer);

    for (int i = 0; i < DRONECAN_MAX_NODES; i++) {
        EXPECT_EQ(dnaServerData()->entries[i].nodeId, 0u)
            << "Out-of-sequence Stage 2 must not create a table entry";
    }
}

/* =========================================================================
 * DNA-7: Followup timeout mid-handshake resets the UID accumulator
 * ========================================================================= */
TEST_F(DroneCANDnaServerTest, FollowupTimeoutResetsAccumulator)
{
    uint8_t buf[UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MAX_SIZE + 4];
    const uint8_t uid[16] = {
        0x10,0x20,0x30,0x40,0x50,0x60,
        0x70,0x80,0x90,0xA0,0xB0,0xC0,
        0xD0,0xE0,0xF0,0x00
    };

    /* Stage 1 */
    CanardRxTransfer s1 = makeAllocationTransfer(0, true, uid, MAX_LEN, buf);
    dronecanDnaHandleAllocation(NULL, &s1);

    /* Advance past the followup timeout — server must reset accumulator */
    mock_time_ms += FOLLOWUP_MS + 100;

    /* Stage 2 — now arrives as if no Stage 1 preceded it */
    CanardRxTransfer s2 = makeAllocationTransfer(0, false, uid + MAX_LEN, MAX_LEN, buf);
    dronecanDnaHandleAllocation(NULL, &s2);

    for (int i = 0; i < DRONECAN_MAX_NODES; i++) {
        EXPECT_EQ(dnaServerData()->entries[i].nodeId, 0u)
            << "Stage 2 after timeout must not create a table entry";
    }
}

/* =========================================================================
 * DNA-8: FC's own node ID is never assigned to a peripheral
 *
 * Top-down sequential assignment starts at 125 and DRONECAN_MAX_NODES (32)
 * caps how many allocation-table + live-node-table entries can ever be
 * occupied at once (64 combined at most) — nowhere near enough to force the
 * top-down scan down to FC_NODE_ID (5) by filling slots. Requesting
 * FC_NODE_ID as the *preferred* ID instead exercises the exact
 * isNodeAvailable(canard.node_id) guard directly and deterministically:
 * the preferred-ID search starts at the requested value and scans upward,
 * so if 5 (FC's own ID) is correctly skipped, the peripheral must land on
 * 6, the next available ID above it.
 * ========================================================================= */
TEST_F(DroneCANDnaServerTest, FcNodeIdNeverAssigned)
{
    const uint8_t uid[16] = {
        0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
        0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,
        0xFC,0xFC,0xFC,0xFC
    };

    uint8_t assigned = runHandshake(uid, FC_NODE_ID);

    ASSERT_NE(assigned, 0u) << "Requesting the FC's own node ID must not block allocation";
    EXPECT_NE(assigned, FC_NODE_ID)
        << "FC node ID " << (int)FC_NODE_ID << " was incorrectly assigned to a peripheral";
    EXPECT_EQ(assigned, FC_NODE_ID + 1)
        << "Preferred-ID search starts at the requested value and scans upward; "
        << "skipping " << (int)FC_NODE_ID << " (FC's own ID) on a clean table "
        << "must land on " << (int)(FC_NODE_ID + 1);
}

/* =========================================================================
 * DNA-9: Peripheral's preferred node ID is honoured
 *
 * The UAVCAN spec allows a peripheral to include a preferred node_id in its
 * Stage-1 request. The allocator assigns that ID if it is in the valid
 * dynamic range (1-125) and not already taken, falling back to sequential
 * assignment otherwise.
 * ========================================================================= */
TEST_F(DroneCANDnaServerTest, RequestedNodeIdIsHonoured)
{
    const uint8_t uid[16] = {
        0x55,0x55,0x55,0x55,0x55,0x55,
        0x55,0x55,0x55,0x55,0x55,0x55,
        0x55,0x55,0x55,0x55
    };
    const uint8_t requested = 42u;

    uint8_t assigned = runHandshake(uid, requested);

    EXPECT_EQ(assigned, requested)
        << "Peripheral requested node ID " << (int)requested
        << " but was assigned " << (int)assigned;
}

/* =========================================================================
 * DNA-10: First sequential (no preferred ID) assignment is 125
 *
 * The UAVCAN spec requires allocators to assign from the top of the dynamic
 * range downward so that manually assigned low IDs are least likely to
 * collide. DRONECAN_DNA_MAX_NODE_ID = 125 (126-127 reserved).
 * ========================================================================= */
TEST_F(DroneCANDnaServerTest, SequentialAssignmentStartsAtTop)
{
    const uint8_t uid[16] = {
        0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,
        0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,
        0xA0,0xA0,0xA0,0xA0
    };

    uint8_t assigned = runHandshake(uid);

    EXPECT_EQ(assigned, DRONECAN_DNA_MAX_NODE_ID)
        << "First sequential assignment must be " << (int)DRONECAN_DNA_MAX_NODE_ID
        << " (top of dynamic range), got " << (int)assigned;
}

/* =========================================================================
 * DNA-11: Preferred ID in reserved range (126 or 127) → falls back to 125
 *
 * Node IDs 126 and 127 are reserved for network maintenance tools and must
 * never be assigned dynamically. A request for these must be silently
 * ignored and a valid ID assigned instead.
 * ========================================================================= */
TEST_F(DroneCANDnaServerTest, ReservedPreferredIdFallsBackToSequential)
{
    const uint8_t uid[16] = {
        0xB0,0xB0,0xB0,0xB0,0xB0,0xB0,
        0xB0,0xB0,0xB0,0xB0,0xB0,0xB0,
        0xB0,0xB0,0xB0,0xB0
    };

    uint8_t assigned = runHandshake(uid, 126u);

    EXPECT_NE(assigned, 0u)   << "Reserved preferred ID must not block allocation";
    EXPECT_NE(assigned, 126u) << "Reserved node ID 126 must not be assigned";
    EXPECT_NE(assigned, 127u) << "Reserved node ID 127 must not be assigned";
    EXPECT_LE(assigned, (uint8_t)DRONECAN_DNA_MAX_NODE_ID)
        << "Assigned ID must be within the dynamic range";
}

/* =========================================================================
 * DNA-12: Preferred ID already taken → falls back to sequential
 *
 * If the requested node ID is valid but already in the allocation table,
 * the allocator must assign the next free ID rather than failing.
 * ========================================================================= */
TEST_F(DroneCANDnaServerTest, TakenPreferredIdFallsBackToSequential)
{
    const uint8_t uid1[16] = {
        0xC1,0xC1,0xC1,0xC1,0xC1,0xC1,
        0xC1,0xC1,0xC1,0xC1,0xC1,0xC1,
        0xC1,0xC1,0xC1,0xC1
    };
    const uint8_t uid2[16] = {
        0xC2,0xC2,0xC2,0xC2,0xC2,0xC2,
        0xC2,0xC2,0xC2,0xC2,0xC2,0xC2,
        0xC2,0xC2,0xC2,0xC2
    };
    const uint8_t preferred = 60u;

    /* First peripheral claims the preferred ID */
    uint8_t first = runHandshake(uid1, preferred);
    ASSERT_EQ(first, preferred) << "Setup: first peripheral should get its preferred ID";

    s_base_ms   += 10000;
    mock_time_ms = s_base_ms;

    /* Second peripheral requests the same ID — must get the next upward free ID */
    uint8_t second = runHandshake(uid2, preferred);

    EXPECT_EQ(second, preferred + 1)
        << "Upward search from taken preferred ID must yield preferred+1";
    EXPECT_LE(second, (uint8_t)DRONECAN_DNA_MAX_NODE_ID);
}

/* =========================================================================
 * DNA-13: Stored ID in use on live network → reassigned, table entry updated
 *
 * If a peripheral re-negotiates and its previously stored node ID is now
 * claimed by a live static-ID node (visible in the NodeStatus table), the
 * allocator must assign a new ID and overwrite the old table entry rather
 * than creating a duplicate.
 * ========================================================================= */
TEST_F(DroneCANDnaServerTest, ConflictingLiveNodeCausesReassignment)
{
    const uint8_t uid[16] = {
        0xD0,0xD0,0xD0,0xD0,0xD0,0xD0,
        0xD0,0xD0,0xD0,0xD0,0xD0,0xD0,
        0xD0,0xD0,0xD0,0xD0
    };

    /* Initial allocation — gets DRONECAN_DNA_MAX_NODE_ID (125) */
    uint8_t first = runHandshake(uid);
    ASSERT_EQ(first, (uint8_t)DRONECAN_DNA_MAX_NODE_ID);

    s_base_ms   += 10000;
    mock_time_ms = s_base_ms;

    /* Simulate a static-ID node claiming that ID on the live network.
       dnaIdx=0xFF marks this as a non-DNA-managed node (a manually
       configured static-ID peripheral, not the original DNA-allocated
       peripheral coming back). Without this, the reboot-churn fix would
       mistake this entry for the same peripheral's own previous broadcast. */
    mock_node_table[0].nodeID = first;
    mock_node_table[0].dnaIdx = 0xFF;
    mock_node_count = 1;

    /* Re-negotiate — server must detect the conflict and assign a new ID */
    uint8_t second = runHandshake(uid);

    EXPECT_NE(second, 0u)   << "Conflict must not prevent allocation";
    EXPECT_NE(second, first) << "Conflicted ID must not be re-assigned";
    EXPECT_LE(second, (uint8_t)DRONECAN_DNA_MAX_NODE_ID);

    /* Table must have exactly one entry for this UID with the new ID */
    int count = 0;
    uint8_t tableId = 0;
    for (int i = 0; i < DRONECAN_MAX_NODES; i++) {
        if (memcmp(dnaServerData()->entries[i].uniqueId, uid, 16) == 0 &&
            dnaServerData()->entries[i].nodeId != 0) {
            count++;
            tableId = dnaServerData()->entries[i].nodeId;
        }
    }
    EXPECT_EQ(count, 1)      << "Must be exactly one table entry for this UID";
    EXPECT_EQ(tableId, second) << "Table entry must reflect the newly assigned ID";
}

/* =========================================================================
 * DNA-14: Full 16-byte UID delivered in a single stage-1 message
 *
 * A transport capable of a larger single frame (e.g. CAN-FD) can carry the
 * entire 16-byte unique ID in one message instead of the classic 6+6+4
 * split. detectRequestStage() must accept first_part_of_unique_id=true with
 * unique_id.len=16 as a valid Stage 1, and the handler must complete the
 * allocation immediately without waiting for Stage 2/3 messages.
 * ========================================================================= */
TEST_F(DroneCANDnaServerTest, SingleFrameFullUidCompletesImmediately)
{
    const uint8_t uid[16] = {
        0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,
        0xE0,0xE0,0xE0,0xE0,0xE0,0xE0,
        0xE0,0xE0,0xE0,0xE0
    };
    uint8_t buf[UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MAX_SIZE + 4];

    CanardRxTransfer xfer = makeAllocationTransfer(0, true, uid, 16, buf);
    dronecanDnaHandleAllocation(NULL, &xfer);

    uint8_t assigned = 0;
    for (int i = 0; i < DRONECAN_MAX_NODES; i++) {
        if (dnaServerData()->entries[i].nodeId != 0 &&
            memcmp(dnaServerData()->entries[i].uniqueId, uid, 16) == 0) {
            assigned = dnaServerData()->entries[i].nodeId;
            break;
        }
    }

    EXPECT_NE(assigned, 0u)
        << "A single-frame 16-byte stage-1 message must complete allocation "
        << "without needing follow-up Stage 2/3 messages";
}

/* =========================================================================
 * DNA-15: Full-length message not marked as stage 1 is rejected
 *
 * A 16-byte unique_id.len is only a valid length when paired with
 * first_part_of_unique_id=true (DNA-14). The same length with the flag
 * false must be rejected as DNA_INVALID_STAGE rather than silently
 * accepted as some other stage.
 * ========================================================================= */
TEST_F(DroneCANDnaServerTest, NonFirstPartFullLengthRejected)
{
    const uint8_t uid[16] = {
        0xE1,0xE1,0xE1,0xE1,0xE1,0xE1,
        0xE1,0xE1,0xE1,0xE1,0xE1,0xE1,
        0xE1,0xE1,0xE1,0xE1
    };
    uint8_t buf[UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MAX_SIZE + 4];

    CanardRxTransfer xfer = makeAllocationTransfer(0, false, uid, 16, buf);
    dronecanDnaHandleAllocation(NULL, &xfer);

    for (int i = 0; i < DRONECAN_MAX_NODES; i++) {
        EXPECT_NE(0, memcmp(dnaServerData()->entries[i].uniqueId, uid, 16))
            << "A rejected (non-first-part, full-length) message must not "
            << "create a table entry at slot " << i;
    }
}

/* =========================================================================
 * DNA-16: Malformed 4-byte stage-1 message is rejected and does not poison
 * the accumulator against a legitimate follow-up handshake
 *
 * Before this fix, first_part_of_unique_id=true with unique_id.len=4 (the
 * DNA_STAGE3_UID_LEN length) was accepted as a valid Stage 1, since the old
 * detectRequestStage() only checked the first_part flag, not the length,
 * once the length passed the top-level {4,6,16} whitelist. That set
 * currentUniqueId.len=4, and getExpectedStage(4) falls through to
 * DNA_INVALID_STAGE (4 is neither 0, nor >=12, nor >=6) — so *any*
 * legitimate follow-up message arriving within the 500ms followup window
 * was rejected as a stage mismatch, silently blocking real allocation
 * until the timeout reset the accumulator. This is the actual regression
 * this fix guards against; DNA-14/DNA-15 do not exercise it since both of
 * their (length, first_part) combinations were already handled correctly
 * before this fix.
 * ========================================================================= */
TEST_F(DroneCANDnaServerTest, MalformedFourByteStage1DoesNotBlockRealHandshake)
{
    const uint8_t bogus_uid[4] = {0xBA, 0xD0, 0xBA, 0xD0};
    uint8_t buf[UAVCAN_PROTOCOL_DYNAMIC_NODE_ID_ALLOCATION_MAX_SIZE + 4];

    /* Malformed message: first_part=true with the stage-3-only 4-byte length.
       Must be rejected outright and must not touch the accumulator at all. */
    CanardRxTransfer bogus = makeAllocationTransfer(0, true, bogus_uid, 4, buf);
    dronecanDnaHandleAllocation(NULL, &bogus);

    /* Immediately after, a legitimate single-frame 16-byte stage-1 handshake
       (well within the 500ms followup window) must still succeed. */
    const uint8_t real_uid[16] = {
        0xE2,0xE2,0xE2,0xE2,0xE2,0xE2,
        0xE2,0xE2,0xE2,0xE2,0xE2,0xE2,
        0xE2,0xE2,0xE2,0xE2
    };
    mock_time_ms += 10;
    CanardRxTransfer real = makeAllocationTransfer(0, true, real_uid, 16, buf);
    dronecanDnaHandleAllocation(NULL, &real);

    uint8_t assigned = 0;
    for (int i = 0; i < DRONECAN_MAX_NODES; i++) {
        if (dnaServerData()->entries[i].nodeId != 0 &&
            memcmp(dnaServerData()->entries[i].uniqueId, real_uid, 16) == 0) {
            assigned = dnaServerData()->entries[i].nodeId;
            break;
        }
    }

    EXPECT_NE(assigned, 0u)
        << "A malformed 4-byte stage-1 message must not poison the accumulator "
        << "and block a legitimate handshake that follows it";
}

/* =========================================================================
 * DNA-17: Reboot within the live-node stale window re-issues the stored ID
 *
 * Regression: a peripheral with a known allocation restarts and re-negotiates
 * before its previous NodeStatus entry has been pruned from the live-node
 * table (DRONECAN_NODE_STALE_TIMEOUT_MS = 10000). The live-node table still
 * has an entry at the previously-assigned node ID. Before the fix, the
 * allocator treated that stale entry as a conflict, reassigned a new ID,
 * overwrote the table entry, and called saveConfig(). Repeated quick reboots
 * churned through the entire downward-from-125 ID space and eventually
 * exhausted it (LOG_ERROR "DNA: no free node IDs available").
 *
 * The DNA server pre-creates the live-node entry at allocation time via
 * dronecanNodeStatusRegisterNode(); the peripheral's first real NodeStatus
 * refreshes the entry in place. On a subsequent handshake within the stale
 * window, the cached entry is the same peripheral and the stored ID is
 * re-issued.
 * ========================================================================= */
TEST_F(DroneCANDnaServerTest, RebootWithinStaleWindowReissuesStoredNodeId)
{
    const uint8_t uid[16] = {
        0xB1,0xB1,0xB1,0xB1,0xB1,0xB1,
        0xB1,0xB1,0xB1,0xB1,0xB1,0xB1,
        0xB1,0xB1,0xB1,0xB1
    };

    /* First allocation: gets 125, occupies DNA slot 0, calls saveConfig(). */
    uint8_t first = runHandshake(uid);
    ASSERT_EQ(first, (uint8_t)DRONECAN_DNA_MAX_NODE_ID);
    int saves_after_first = saveConfig_call_count;
    ASSERT_GE(saves_after_first, 1);

    /* Simulate the peripheral having adopted 125 and broadcast one or more
       NodeStatus frames. The pre-created entry's dnaIdx is back-linked to
       the DNA slot. The peripheral's last broadcast was 1 s ago — well
       inside the 10 s stale window. */
    s_base_ms   += 5000;  /* 5 s into the peripheral's life */
    mock_time_ms = s_base_ms;

    mock_node_table[0].nodeID       = first;
    mock_node_table[0].last_seen_ms = mock_time_ms - 1000;
    mock_node_table[0].dnaIdx       = 0;  /* back-linked to DNA slot 0 */
    mock_node_count = 1;

    /* Reboot: peripheral re-negotiates. */
    uint8_t second = runHandshake(uid);

    /* The FC must recognise the cached live-node entry as the same
       peripheral's own previous broadcast (matched via dnaIdx == 0) and
       re-issue 125. Pre-fix this assertion fails: second == 124 because
       the FC treated the live entry as a foreign occupier. */
    EXPECT_EQ(second, first)
        << "Reboot within the 10 s stale window must re-issue the stored "
        << "node ID " << (int)first << "; the FC assigned " << (int)second
        << " instead, treating the same-peripheral's cached NodeStatus as a conflict";

    /* The persisted allocation table must still map this UID to the
       original ID — no overwrite, no saveConfig(). */
    int count = 0;
    uint8_t tableId = 0;
    for (int i = 0; i < DRONECAN_MAX_NODES; i++) {
        if (memcmp(dnaServerData()->entries[i].uniqueId, uid, 16) == 0 &&
            dnaServerData()->entries[i].nodeId != 0) {
            count++;
            tableId = dnaServerData()->entries[i].nodeId;
        }
    }
    EXPECT_EQ(count, 1)        << "Reboot must not duplicate the table entry";
    EXPECT_EQ(tableId, first)  << "Reboot must not overwrite the table entry";

    EXPECT_EQ(saveConfig_call_count, saves_after_first)
        << "Re-issuing the stored ID is not a configuration change; "
        << "saveConfig() must not be called";
}
