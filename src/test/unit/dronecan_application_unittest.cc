/**
 * DroneCAN Application-Layer Unit Tests
 *
 * Tests node table management and transfer acceptance filter using the real
 * dronecan.c compiled against INAV stubs. The UNIT_TEST build makes
 * activeNodeCount and nodeTable non-static so tests can reset state in SetUp.
 *
 * Coverage:
 *   GAP-N1  New node ID → added to table; no slot if table full
 *   GAP-N2  Subsequent NodeStatus from same node → fields updated in place
 *   GAP-N3  last_seen_ms follows controllable millis() value
 *   GAP-N4  33rd unique node → table overflow rejected, count stays at 32
 *   GAP-S1  shouldAcceptTransfer: NodeStatus ✓, GetNodeInfo request ✓,
 *           GetNodeInfo response ✓, unknown ID ✗
 */

#include "gtest/gtest.h"

extern "C" {
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "platform.h"

/* DSDL types used by dronecan.c handlers */
#include "uavcan.protocol.NodeStatus.h"
#include "uavcan.protocol.GetNodeInfo.h"

/* Canard core and STM32 driver declarations */
#include "drivers/dronecan/libcanard/canard.h"
#include "drivers/dronecan/libcanard/canard_stm32_driver.h"

/* INAV headers pulled in by dronecan.c — included here so the types are
   available when we define stub globals below. */
#include "io/gps.h"
#include "sensors/battery_sensor_dronecan.h"
#include "fc/runtime_config.h"
#include "sensors/diagnostics.h"
#include "build/version.h"
#include "common/log.h"

/* Public API we test against */
#include "drivers/dronecan/dronecan.h"

/* Private state made non-static in UNIT_TEST builds */
extern uint8_t activeNodeCount;
extern dronecanNodeInfo_t nodeTable[];

/* Private functions not exposed in dronecan.h */
void handle_NodeStatus(CanardInstance *ins, CanardRxTransfer *transfer);
bool shouldAcceptTransfer(const CanardInstance *ins,
                          uint64_t *out_data_type_signature,
                          uint16_t data_type_id,
                          CanardTransferType transfer_type,
                          uint8_t source_node_id);
void onTransferReceived(CanardInstance *ins, CanardRxTransfer *transfer);

/* =========================================================================
 * Stubs — provide every symbol dronecan.c references that isn't supplied by
 * the compiled dependencies (dronecan.c, canard.c, DSDL .c files).
 * ========================================================================= */

/* Controllable time source */
static uint32_t mock_time_ms = 0;
uint32_t millis(void) { return mock_time_ms; }

/* Arming state — dronecan.c reads this for send_NodeStatus vendor code */
uint32_t armingFlags = 0;

/* GPS config — provider != GPS_DRONECAN so all GPS handlers return early */
gpsConfig_t gpsConfig_System;
gpsConfig_t gpsConfig_Copy;

/* Hardware health — dronecan.c reads this in send_NodeStatus */
bool isHardwareHealthy(void) { return true; }

/* Logging — USE_LOG is unconditionally defined by target/common.h (pulled in
   via platform.h), so LOG_ERROR/LOG_DEBUG in dronecan.c expand to real _logf()
   calls. Stubbed as a no-op rather than linking common/log.c, which would pull
   in drivers/serial.h, msp/msp.h, msp/msp_serial.h, fc/config.h and
   config/feature.h — unrelated production dependencies this test has no need
   for. Tests don't assert on logging output. */
void _logf(logTopic_e topic, unsigned level, const char *fmt, ...) { (void)topic; (void)level; (void)fmt; }

/* GPS and battery DroneCAN receive stubs */
void dronecanGPSReceiveGNSSFix(const struct uavcan_equipment_gnss_Fix *p) { (void)p; }
void dronecanGPSReceiveGNSSFix2(const struct uavcan_equipment_gnss_Fix2 *p) { (void)p; }
void dronecanGPSReceiveGNSSAuxiliary(const struct uavcan_equipment_gnss_Auxiliary *p) { (void)p; }
void dronecanBatterySensorReceiveInfo(struct uavcan_equipment_power_BatteryInfo *p) { (void)p; }

/* STM32 CAN driver stubs */
int16_t canardSTM32CAN1_Init(uint32_t b) { (void)b; return CANARD_OK; }
int16_t canardSTM32Receive(CanardCANFrame *f) { (void)f; return 0; }
uint32_t canardSTM32GetAndClearRxDropCount(void) { return 0; }
int16_t canardSTM32Transmit(const CanardCANFrame *f) { (void)f; return 1; }
void    canardSTM32GetProtocolStatus(canardProtocolStatus_t *s) { memset(s, 0, sizeof(*s)); }
int32_t canardSTM32GetRxFifoFillLevel(void) { return 0; }
void    canardSTM32RecoverFromBusOff(void) {}
void    canardSTM32GetUniqueID(uint8_t id[16]) { memset(id, 0, 16); }

/* Version strings declared in build/version.h */
const char* const shortGitRevision = "00000000";
const char* const compilerVersion  = "test";
const char* const targetName       = "TEST";
const char* const buildDate        = "Jan 01 2026";
const char* const buildTime        = "00:00:00";

} /* extern "C" */

/* =========================================================================
 * Helper: encode a NodeStatus and build a single-frame CanardRxTransfer.
 * buf must be at least UAVCAN_PROTOCOL_NODESTATUS_MAX_SIZE bytes.
 * ========================================================================= */
static CanardRxTransfer makeNodeStatusTransfer(
        uint8_t  nodeId,
        uint32_t uptime_sec,
        uint8_t  health,
        uint8_t  mode,
        uint16_t vendor_code,
        uint8_t *buf)
{
    struct uavcan_protocol_NodeStatus ns;
    memset(&ns, 0, sizeof(ns));
    ns.uptime_sec                    = uptime_sec;
    ns.health                        = health;
    ns.mode                          = mode;
    ns.vendor_specific_status_code   = vendor_code;

    uint32_t len = uavcan_protocol_NodeStatus_encode(&ns, buf);

    CanardRxTransfer xfer;
    memset(&xfer, 0, sizeof(xfer));
    xfer.transfer_type  = CanardTransferTypeBroadcast;
    xfer.data_type_id   = UAVCAN_PROTOCOL_NODESTATUS_ID;
    xfer.source_node_id = nodeId;
    xfer.payload_head   = buf;
    xfer.payload_len    = (uint16_t)len;
    return xfer;
}

/* =========================================================================
 * Node table tests (GAP-N1 … GAP-N4)
 * ========================================================================= */

class DroneCANNodeTableTest : public ::testing::Test {
protected:
    CanardInstance ins;
    uint8_t memory_pool[4096]; /* generous pool: 32 nodes × 1 frame each */
    uint8_t buf[UAVCAN_PROTOCOL_NODESTATUS_MAX_SIZE + 4];

    void SetUp() override {
        activeNodeCount = 0;
        memset(nodeTable, 0, sizeof(dronecanNodeInfo_t) * DRONECAN_MAX_NODES);
        mock_time_ms = 0;
        canardInit(&ins, memory_pool, sizeof(memory_pool),
                   onTransferReceived, shouldAcceptTransfer, NULL);
        canardSetLocalNodeID(&ins, 1); /* FC node ID required for canardRequestOrRespond */
    }
};

/* GAP-N1: First NodeStatus from an unseen node ID → entry added to table */
TEST_F(DroneCANNodeTableTest, NewNodeAddedOnFirstStatus)
{
    ASSERT_EQ(dronecanGetNodeCount(), 0u);

    CanardRxTransfer xfer = makeNodeStatusTransfer(
            10, 100,
            UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK,
            UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL,
            0xABCD, buf);
    handle_NodeStatus(&ins, &xfer);

    EXPECT_EQ(dronecanGetNodeCount(), 1u);

    const dronecanNodeInfo_t *node = dronecanGetNode(0);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->nodeID,             10u);
    EXPECT_EQ(node->health,             UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK);
    EXPECT_EQ(node->mode,               UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL);
    EXPECT_EQ(node->uptime_sec,         100u);
    EXPECT_EQ(node->vendor_status_code, 0xABCDu);
    EXPECT_EQ(node->name_len,           0u);
    EXPECT_EQ(node->name[0],            '\0');
}

/* GAP-N1 (second node): Two distinct IDs → two separate entries */
TEST_F(DroneCANNodeTableTest, TwoDistinctNodesStoredSeparately)
{
    CanardRxTransfer x1 = makeNodeStatusTransfer(10, 100, 0, 0, 0, buf);
    handle_NodeStatus(&ins, &x1);
    CanardRxTransfer x2 = makeNodeStatusTransfer(20, 200, 0, 0, 0, buf);
    handle_NodeStatus(&ins, &x2);

    EXPECT_EQ(dronecanGetNodeCount(), 2u);
    EXPECT_EQ(dronecanGetNode(0)->nodeID, 10u);
    EXPECT_EQ(dronecanGetNode(1)->nodeID, 20u);
}

/* GAP-N2: Second NodeStatus from the same node → fields updated, no new entry */
TEST_F(DroneCANNodeTableTest, ExistingNodeUpdatedInPlace)
{
    CanardRxTransfer x1 = makeNodeStatusTransfer(
            10, 100,
            UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK,
            UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL,
            0x0000, buf);
    handle_NodeStatus(&ins, &x1);
    ASSERT_EQ(dronecanGetNodeCount(), 1u);

    CanardRxTransfer x2 = makeNodeStatusTransfer(
            10, 500,
            UAVCAN_PROTOCOL_NODESTATUS_HEALTH_WARNING,
            UAVCAN_PROTOCOL_NODESTATUS_MODE_MAINTENANCE,
            0xBEEF, buf);
    handle_NodeStatus(&ins, &x2);

    EXPECT_EQ(dronecanGetNodeCount(), 1u);   /* still one node */

    const dronecanNodeInfo_t *node = dronecanGetNode(0);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->health,             UAVCAN_PROTOCOL_NODESTATUS_HEALTH_WARNING);
    EXPECT_EQ(node->mode,               UAVCAN_PROTOCOL_NODESTATUS_MODE_MAINTENANCE);
    EXPECT_EQ(node->uptime_sec,         500u);
    EXPECT_EQ(node->vendor_status_code, 0xBEEFu);
}

/* GAP-N3: last_seen_ms is set from millis() at the time of each call */
TEST_F(DroneCANNodeTableTest, LastSeenMsFollowsMillis)
{
    mock_time_ms = 1000;
    CanardRxTransfer x1 = makeNodeStatusTransfer(20, 10, 0, 0, 0, buf);
    handle_NodeStatus(&ins, &x1);

    const dronecanNodeInfo_t *node = dronecanGetNode(0);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->last_seen_ms, 1000u);

    mock_time_ms = 2500;
    CanardRxTransfer x2 = makeNodeStatusTransfer(20, 20, 0, 0, 0, buf);
    handle_NodeStatus(&ins, &x2);

    EXPECT_EQ(node->last_seen_ms, 2500u);
}

/* GAP-N3: last_seen_ms for a new node also uses current millis() */
TEST_F(DroneCANNodeTableTest, LastSeenMsSetOnInsert)
{
    mock_time_ms = 9999;
    CanardRxTransfer xfer = makeNodeStatusTransfer(5, 0, 0, 0, 0, buf);
    handle_NodeStatus(&ins, &xfer);

    const dronecanNodeInfo_t *node = dronecanGetNode(0);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->last_seen_ms, 9999u);
}

/* GAP-N4: Fill the table to DRONECAN_MAX_NODES, then a 33rd node is silently
   dropped — count stays at 32 and the overflow ID is not present. */
TEST_F(DroneCANNodeTableTest, TableFullNodeRejected)
{
    for (uint8_t i = 1; i <= DRONECAN_MAX_NODES; i++) {
        CanardRxTransfer xfer = makeNodeStatusTransfer(i, 0, 0, 0, 0, buf);
        handle_NodeStatus(&ins, &xfer);
    }
    ASSERT_EQ(dronecanGetNodeCount(), (uint8_t)DRONECAN_MAX_NODES);

    /* Try to add a 33rd node (ID 100, not in 1..32) */
    CanardRxTransfer overflow = makeNodeStatusTransfer(100, 0, 0, 0, 0, buf);
    handle_NodeStatus(&ins, &overflow);

    EXPECT_EQ(dronecanGetNodeCount(), (uint8_t)DRONECAN_MAX_NODES);

    for (uint8_t i = 0; i < DRONECAN_MAX_NODES; i++) {
        const dronecanNodeInfo_t *n = dronecanGetNode(i);
        ASSERT_NE(n, nullptr);
        EXPECT_NE(n->nodeID, 100u) << "overflow node ID 100 should not be in slot " << (int)i;
    }
}

/* GAP-N4 boundary: dronecanGetNode at index == DRONECAN_MAX_NODES returns NULL */
TEST_F(DroneCANNodeTableTest, GetNodeOutOfBoundsReturnsNull)
{
    EXPECT_EQ(dronecanGetNode(DRONECAN_MAX_NODES), nullptr);
    EXPECT_EQ(dronecanGetNode(255), nullptr);
}

/* =========================================================================
 * shouldAcceptTransfer tests (GAP-S1)
 * ========================================================================= */

/* shouldAcceptTransfer does not use the CanardInstance — pass NULL. */

TEST(DroneCANShouldAcceptTransfer, AcceptsNodeStatusBroadcast)
{
    uint64_t signature = 0;
    bool accept = shouldAcceptTransfer(
            nullptr, &signature,
            UAVCAN_PROTOCOL_NODESTATUS_ID,
            CanardTransferTypeBroadcast,
            42);

    EXPECT_TRUE(accept);
    EXPECT_EQ(signature, UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE);
}

TEST(DroneCANShouldAcceptTransfer, AcceptsGetNodeInfoRequest)
{
    /* The FC handles incoming GetNodeInfo requests and sends a response */
    uint64_t signature = 0;
    bool accept = shouldAcceptTransfer(
            nullptr, &signature,
            UAVCAN_PROTOCOL_GETNODEINFO_ID,
            CanardTransferTypeRequest,
            42);

    EXPECT_TRUE(accept);
    EXPECT_EQ(signature, UAVCAN_PROTOCOL_GETNODEINFO_REQUEST_SIGNATURE);
}

TEST(DroneCANShouldAcceptTransfer, AcceptsGetNodeInfoResponse)
{
    /* Phase 3: FC now accepts GetNodeInfo responses so handle_GetNodeInfoResponse
       can populate the node table with name and version data. */
    uint64_t signature = 0;
    bool accept = shouldAcceptTransfer(
            nullptr, &signature,
            UAVCAN_PROTOCOL_GETNODEINFO_ID,
            CanardTransferTypeResponse,
            42);

    EXPECT_TRUE(accept);
    EXPECT_EQ(signature, UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_SIGNATURE);
}

TEST(DroneCANShouldAcceptTransfer, RejectsUnknownBroadcastId)
{
    uint64_t signature = 0;
    bool accept = shouldAcceptTransfer(
            nullptr, &signature,
            0xFFFF,                        /* not a real UAVCAN data type ID */
            CanardTransferTypeBroadcast,
            42);

    EXPECT_FALSE(accept);
}

TEST(DroneCANShouldAcceptTransfer, RejectsUnknownResponseId)
{
    uint64_t signature = 0;
    bool accept = shouldAcceptTransfer(
            nullptr, &signature,
            0xFFFF,
            CanardTransferTypeResponse,
            42);

    EXPECT_FALSE(accept);
}

/* =========================================================================
 * onTransferReceived dispatch test (GAP-S2)
 *
 * Verifies that a GetNodeInfo response transfer is dispatched to
 * handle_GetNodeInfoResponse and populates the node table entry.
 * Written before Phase 4 — fails until the handler is implemented.
 * ========================================================================= */

class DroneCANDispatchTest : public ::testing::Test {
protected:
    CanardInstance ins;
    uint8_t memory_pool[4096];
    uint8_t buf[UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_MAX_SIZE + 16];

    void SetUp() override {
        activeNodeCount = 0;
        memset(nodeTable, 0, sizeof(dronecanNodeInfo_t) * DRONECAN_MAX_NODES);
        mock_time_ms = 0;
        canardInit(&ins, memory_pool, sizeof(memory_pool),
                   onTransferReceived, shouldAcceptTransfer, NULL);
        canardSetLocalNodeID(&ins, 1);
    }
};

/* GAP-S2: GetNodeInfo response → handler populates name and version fields */
TEST_F(DroneCANDispatchTest, GetNodeInfoResponsePopulatesNodeTableEntry)
{
    /* Pre-insert node 42 via a NodeStatus so the table has a slot for it */
    uint8_t ns_buf[UAVCAN_PROTOCOL_NODESTATUS_MAX_SIZE + 4];
    CanardRxTransfer ns_xfer = makeNodeStatusTransfer(42, 10, 0, 0, 0, ns_buf);
    handle_NodeStatus(&ins, &ns_xfer);
    ASSERT_EQ(dronecanGetNodeCount(), 1u);

    /* Build a GetNodeInfo response from node 42 */
    struct uavcan_protocol_GetNodeInfoResponse resp;
    memset(&resp, 0, sizeof(resp));

    resp.status.uptime_sec = 10;
    resp.status.health     = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
    resp.status.mode       = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;

    resp.software_version.major                = 1;
    resp.software_version.minor                = 7;
    resp.software_version.optional_field_flags = 1; /* vcs_commit valid */
    resp.software_version.vcs_commit           = 0xDEADBEEF;

    resp.hardware_version.major = 2;
    resp.hardware_version.minor = 0;
    for (int i = 0; i < 16; i++) {
        resp.hardware_version.unique_id[i] = (uint8_t)(0xA0 + i);
    }

    const char *name = "com.example.gps";
    resp.name.len = (uint8_t)strlen(name);
    memcpy(resp.name.data, name, resp.name.len);

    uint32_t encoded_len = uavcan_protocol_GetNodeInfoResponse_encode(&resp, buf);

    CanardRxTransfer xfer;
    memset(&xfer, 0, sizeof(xfer));
    xfer.transfer_type  = CanardTransferTypeResponse;
    xfer.data_type_id   = UAVCAN_PROTOCOL_GETNODEINFO_ID;
    xfer.source_node_id = 42;
    xfer.payload_head   = buf;
    xfer.payload_len    = (uint16_t)encoded_len;

    onTransferReceived(&ins, &xfer);

    /* Verify the node table entry was populated */
    const dronecanNodeInfo_t *node = dronecanGetNode(0);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->nodeID, 42u);

    EXPECT_EQ(node->name_len, (uint8_t)strlen(name));
    EXPECT_EQ(0, memcmp(node->name, name, node->name_len));

    EXPECT_EQ(node->sw_major,                1u);
    EXPECT_EQ(node->sw_minor,                7u);
    EXPECT_EQ(node->sw_optional_field_flags,  1u);
    EXPECT_EQ(node->sw_vcs_commit,           0xDEADBEEFu);

    EXPECT_EQ(node->hw_major, 2u);
    EXPECT_EQ(node->hw_minor, 0u);
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(node->hw_unique_id[i], (uint8_t)(0xA0 + i))
            << "unique_id mismatch at byte " << i;
    }
}
