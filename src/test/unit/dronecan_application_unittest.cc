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
#include "uavcan.protocol.param.GetSet_res.h"
#include "uavcan.protocol.param.ExecuteOpcode_res.h"
#include "uavcan.protocol.RestartNode_res.h"

/* Canard core and STM32 driver declarations */
#include "drivers/dronecan/libcanard/canard.h"
#include "drivers/dronecan/libcanard/canard_stm32_driver.h"

/* INAV headers pulled in by dronecan.c — included here so the types are
   available when we define stub globals below. */
#include "io/gps.h"
#include "io/gps_dronecan.h"
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
void dronecanNodeStatusHandleBroadcast(CanardInstance *ins, CanardRxTransfer *transfer);
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
void dronecanGPSReceiveGNSSFix2(const struct uavcan_equipment_gnss_Fix2 *p, uint8_t sourceNodeId) { (void)p; (void)sourceNodeId; }
void dronecanGPSReceiveGNSSAuxiliary(const struct uavcan_equipment_gnss_Auxiliary *p, uint8_t sourceNodeId) { (void)p; (void)sourceNodeId; }
void dronecanGpsOnNodeEvicted(uint8_t nodeID) { (void)nodeID; }
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

void saveConfig(void) {}

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
 * Helpers: encode response structs and build CanardRxTransfer objects.
 * ========================================================================= */

static CanardRxTransfer makeParamGetSetTransfer(
        uint8_t source_node_id, uint8_t transfer_id,
        struct uavcan_protocol_param_GetSetResponse *resp,
        uint8_t *buf)
{
    uint32_t len = uavcan_protocol_param_GetSetResponse_encode(resp, buf);
    CanardRxTransfer xfer;
    memset(&xfer, 0, sizeof(xfer));
    xfer.transfer_type  = CanardTransferTypeResponse;
    xfer.data_type_id   = UAVCAN_PROTOCOL_PARAM_GETSET_RESPONSE_ID;
    xfer.source_node_id = source_node_id;
    xfer.transfer_id    = transfer_id;
    xfer.payload_head   = buf;
    xfer.payload_len    = (uint16_t)len;
    return xfer;
}

static CanardRxTransfer makeExecuteOpcodeTransfer(
        uint8_t source_node_id, uint8_t transfer_id,
        bool ok, uint8_t *buf)
{
    struct uavcan_protocol_param_ExecuteOpcodeResponse resp;
    memset(&resp, 0, sizeof(resp));
    resp.ok = ok;
    uint32_t len = uavcan_protocol_param_ExecuteOpcodeResponse_encode(&resp, buf);
    CanardRxTransfer xfer;
    memset(&xfer, 0, sizeof(xfer));
    xfer.transfer_type  = CanardTransferTypeResponse;
    xfer.data_type_id   = UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_RESPONSE_ID;
    xfer.source_node_id = source_node_id;
    xfer.transfer_id    = transfer_id;
    xfer.payload_head   = buf;
    xfer.payload_len    = (uint16_t)len;
    return xfer;
}

static CanardRxTransfer makeRestartNodeTransfer(
        uint8_t source_node_id, uint8_t transfer_id,
        bool ok, uint8_t *buf)
{
    struct uavcan_protocol_RestartNodeResponse resp;
    memset(&resp, 0, sizeof(resp));
    resp.ok = ok;
    uint32_t len = uavcan_protocol_RestartNodeResponse_encode(&resp, buf);
    CanardRxTransfer xfer;
    memset(&xfer, 0, sizeof(xfer));
    xfer.transfer_type  = CanardTransferTypeResponse;
    xfer.data_type_id   = UAVCAN_PROTOCOL_RESTARTNODE_RESPONSE_ID;
    xfer.source_node_id = source_node_id;
    xfer.transfer_id    = transfer_id;
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
    dronecanNodeStatusHandleBroadcast(&ins, &xfer);

    EXPECT_EQ(dronecanGetNodeCount(), 1u);

    const dronecanNodeInfo_t *node = dronecanGetNode(0);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->nodeID,             10u);
    EXPECT_EQ(node->health,             UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK);
    EXPECT_EQ(node->mode,               UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL);
    EXPECT_EQ(node->uptime_sec,         100u);
    EXPECT_EQ(node->vendor_status_code, 0xABCDu);
}

/* GAP-N1 (second node): Two distinct IDs → two separate entries */
TEST_F(DroneCANNodeTableTest, TwoDistinctNodesStoredSeparately)
{
    CanardRxTransfer x1 = makeNodeStatusTransfer(10, 100, 0, 0, 0, buf);
    dronecanNodeStatusHandleBroadcast(&ins, &x1);
    CanardRxTransfer x2 = makeNodeStatusTransfer(20, 200, 0, 0, 0, buf);
    dronecanNodeStatusHandleBroadcast(&ins, &x2);

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
    dronecanNodeStatusHandleBroadcast(&ins, &x1);
    ASSERT_EQ(dronecanGetNodeCount(), 1u);

    CanardRxTransfer x2 = makeNodeStatusTransfer(
            10, 500,
            UAVCAN_PROTOCOL_NODESTATUS_HEALTH_WARNING,
            UAVCAN_PROTOCOL_NODESTATUS_MODE_MAINTENANCE,
            0xBEEF, buf);
    dronecanNodeStatusHandleBroadcast(&ins, &x2);

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
    dronecanNodeStatusHandleBroadcast(&ins, &x1);

    const dronecanNodeInfo_t *node = dronecanGetNode(0);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->last_seen_ms, 1000u);

    mock_time_ms = 2500;
    CanardRxTransfer x2 = makeNodeStatusTransfer(20, 20, 0, 0, 0, buf);
    dronecanNodeStatusHandleBroadcast(&ins, &x2);

    EXPECT_EQ(node->last_seen_ms, 2500u);
}

/* GAP-N3: last_seen_ms for a new node also uses current millis() */
TEST_F(DroneCANNodeTableTest, LastSeenMsSetOnInsert)
{
    mock_time_ms = 9999;
    CanardRxTransfer xfer = makeNodeStatusTransfer(5, 0, 0, 0, 0, buf);
    dronecanNodeStatusHandleBroadcast(&ins, &xfer);

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
        dronecanNodeStatusHandleBroadcast(&ins, &xfer);
    }
    ASSERT_EQ(dronecanGetNodeCount(), (uint8_t)DRONECAN_MAX_NODES);

    /* Try to add a 33rd node (ID 100, not in 1..32) */
    CanardRxTransfer overflow = makeNodeStatusTransfer(100, 0, 0, 0, 0, buf);
    dronecanNodeStatusHandleBroadcast(&ins, &overflow);

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

TEST(DroneCANShouldAcceptTransfer, AcceptsParamGetSetResponse)
{
    uint64_t signature = 0;
    bool accept = shouldAcceptTransfer(
            nullptr, &signature,
            UAVCAN_PROTOCOL_PARAM_GETSET_RESPONSE_ID,
            CanardTransferTypeResponse,
            42);

    EXPECT_TRUE(accept);
    EXPECT_EQ(signature, UAVCAN_PROTOCOL_PARAM_GETSET_RESPONSE_SIGNATURE);
}

TEST(DroneCANShouldAcceptTransfer, AcceptsExecuteOpcodeResponse)
{
    uint64_t signature = 0;
    bool accept = shouldAcceptTransfer(
            nullptr, &signature,
            UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_RESPONSE_ID,
            CanardTransferTypeResponse,
            42);

    EXPECT_TRUE(accept);
    EXPECT_EQ(signature, UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_RESPONSE_SIGNATURE);
}

TEST(DroneCANShouldAcceptTransfer, AcceptsRestartNodeResponse)
{
    uint64_t signature = 0;
    bool accept = shouldAcceptTransfer(
            nullptr, &signature,
            UAVCAN_PROTOCOL_RESTARTNODE_RESPONSE_ID,
            CanardTransferTypeResponse,
            42);

    EXPECT_TRUE(accept);
    EXPECT_EQ(signature, UAVCAN_PROTOCOL_RESTARTNODE_RESPONSE_SIGNATURE);
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
        memset(&dronecanAsyncSlot, 0, sizeof(dronecanAsyncSlot));
        dronecanAsyncSlot.state = DRONECAN_ASYNC_IDLE;
        mock_time_ms = 0;
        canardInit(&ins, memory_pool, sizeof(memory_pool),
                   onTransferReceived, shouldAcceptTransfer, NULL);
        canardSetLocalNodeID(&ins, 1);
    }
};

/* GAP-S2: GetNodeInfo response → handler populates async slot result.
 * The node table (dronecanNodeInfo_t) holds only NodeStatus-level fields since
 * commit 96f8a4bd9 stripped the GetNodeInfo fields to save ~3.5 KB RAM and
 * replaced auto-fetch with the on-demand async slot pattern. */
TEST_F(DroneCANDispatchTest, GetNodeInfoResponsePopulatesAsyncSlot)
{
    /* Pre-insert node 42 via a NodeStatus (node table is independent of async slot) */
    uint8_t ns_buf[UAVCAN_PROTOCOL_NODESTATUS_MAX_SIZE + 4];
    CanardRxTransfer ns_xfer = makeNodeStatusTransfer(42, 10, 0, 0, 0, ns_buf);
    dronecanNodeStatusHandleBroadcast(&ins, &ns_xfer);
    ASSERT_EQ(dronecanGetNodeCount(), 1u);

    /* Prime the async slot — handle_AsyncServiceResponse guards on state, service_id,
     * node_id, and transfer_id. The guard checks transfer_id == (slot.transfer_id-1)&0x1F,
     * so set transfer_id=1 so the expected in-flight id is 0 (matching xfer.transfer_id). */
    dronecanAsyncSlot.state       = DRONECAN_ASYNC_PENDING;
    dronecanAsyncSlot.service_id  = DRONECAN_SERVICE_GETNODEINFO;
    dronecanAsyncSlot.node_id     = 42;
    dronecanAsyncSlot.transfer_id = 1;

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
    for (int i = 0; i < 16; i++)
        resp.hardware_version.unique_id[i] = (uint8_t)(0xA0 + i);

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

    /* Slot must now be READY */
    EXPECT_EQ(dronecanAsyncSlot.state, DRONECAN_ASYNC_READY);

    /* Result fields populated from the GetNodeInfo response */
    const dronecanGetNodeInfoResult_t *r = &dronecanAsyncSlot.result.node_info;
    EXPECT_EQ(r->name_len, (uint8_t)strlen(name));
    EXPECT_EQ(0, memcmp(r->name, name, r->name_len));

    EXPECT_EQ(r->sw_major,                1u);
    EXPECT_EQ(r->sw_minor,                7u);
    EXPECT_EQ(r->sw_optional_field_flags,  1u);
    EXPECT_EQ(r->sw_vcs_commit,           0xDEADBEEFu);

    EXPECT_EQ(r->hw_major, 2u);
    EXPECT_EQ(r->hw_minor, 0u);
    for (int i = 0; i < 16; i++)
        EXPECT_EQ(r->hw_unique_id[i], (uint8_t)(0xA0 + i))
            << "unique_id mismatch at byte " << i;

    /* Node table entry still exists (populated by the preceding NodeStatus) */
    const dronecanNodeInfo_t *node = dronecanGetNode(0);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->nodeID, 42u);
}

/* =========================================================================
 * Async service response guard rejection tests (GAP-S3)
 *
 * handle_AsyncServiceResponse has four guards before decoding the payload.
 * Each test confirms a mismatched guard leaves the slot state unchanged.
 * ========================================================================= */

/* GAP-S3a: Slot in IDLE state → response silently ignored */
TEST_F(DroneCANDispatchTest, AsyncSlot_IdleState_IgnoresParamGetSetResponse)
{
    /* slot stays IDLE (SetUp default); send a valid PARAM_GETSET response */
    struct uavcan_protocol_param_GetSetResponse resp;
    memset(&resp, 0, sizeof(resp));
    resp.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE;
    resp.value.integer_value = 7;

    CanardRxTransfer xfer = makeParamGetSetTransfer(42, 0, &resp, buf);
    onTransferReceived(&ins, &xfer);

    EXPECT_EQ(dronecanAsyncSlot.state, DRONECAN_ASYNC_IDLE);
}

/* GAP-S3b: Slot PENDING but response comes from the wrong node ID */
TEST_F(DroneCANDispatchTest, AsyncSlot_WrongNodeId_IgnoresResponse)
{
    dronecanAsyncSlot.state      = DRONECAN_ASYNC_PENDING;
    dronecanAsyncSlot.service_id = DRONECAN_SERVICE_PARAM_GETSET;
    dronecanAsyncSlot.node_id    = 42;
    dronecanAsyncSlot.transfer_id = 1; /* guard expects in-flight id (1-1)&0x1F = 0 */

    struct uavcan_protocol_param_GetSetResponse resp;
    memset(&resp, 0, sizeof(resp));
    resp.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE;

    /* source_node_id = 99, not 42 */
    CanardRxTransfer xfer = makeParamGetSetTransfer(99, 0, &resp, buf);
    onTransferReceived(&ins, &xfer);

    EXPECT_EQ(dronecanAsyncSlot.state, DRONECAN_ASYNC_PENDING);
}

/* GAP-S3c: Slot PENDING but transfer_id does not match the in-flight id */
TEST_F(DroneCANDispatchTest, AsyncSlot_WrongTransferId_IgnoresResponse)
{
    dronecanAsyncSlot.state      = DRONECAN_ASYNC_PENDING;
    dronecanAsyncSlot.service_id = DRONECAN_SERVICE_PARAM_GETSET;
    dronecanAsyncSlot.node_id    = 42;
    dronecanAsyncSlot.transfer_id = 1; /* guard expects xfer.transfer_id == 0 */

    struct uavcan_protocol_param_GetSetResponse resp;
    memset(&resp, 0, sizeof(resp));
    resp.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE;

    /* xfer.transfer_id = 5, which != (1-1)&0x1F = 0 */
    CanardRxTransfer xfer = makeParamGetSetTransfer(42, 5, &resp, buf);
    onTransferReceived(&ins, &xfer);

    EXPECT_EQ(dronecanAsyncSlot.state, DRONECAN_ASYNC_PENDING);
}

/* GAP-S3d: Slot PENDING for GETNODEINFO; a PARAM_GETSET response arrives →
 * data_type_id mismatch rejects it before any decode. */
TEST_F(DroneCANDispatchTest, AsyncSlot_WrongServiceId_IgnoresResponse)
{
    dronecanAsyncSlot.state      = DRONECAN_ASYNC_PENDING;
    dronecanAsyncSlot.service_id = DRONECAN_SERVICE_GETNODEINFO;
    dronecanAsyncSlot.node_id    = 42;
    dronecanAsyncSlot.transfer_id = 1;

    struct uavcan_protocol_param_GetSetResponse resp;
    memset(&resp, 0, sizeof(resp));
    resp.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE;

    /* xfer.data_type_id == PARAM_GETSET(11) != slot.service_id(GETNODEINFO=1) */
    CanardRxTransfer xfer = makeParamGetSetTransfer(42, 0, &resp, buf);
    onTransferReceived(&ins, &xfer);

    EXPECT_EQ(dronecanAsyncSlot.state, DRONECAN_ASYNC_PENDING);
}

/* =========================================================================
 * PARAM_GETSET response decode tests (GAP-S4)
 * ========================================================================= */

/* GAP-S4a: Integer value with integer min/max range */
TEST_F(DroneCANDispatchTest, ParamGetSetIntResponse_PopulatesSlot)
{
    dronecanAsyncSlot.state      = DRONECAN_ASYNC_PENDING;
    dronecanAsyncSlot.service_id = DRONECAN_SERVICE_PARAM_GETSET;
    dronecanAsyncSlot.node_id    = 42;
    dronecanAsyncSlot.transfer_id = 1;

    struct uavcan_protocol_param_GetSetResponse resp;
    memset(&resp, 0, sizeof(resp));
    resp.value.union_tag    = UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE;
    resp.value.integer_value = 42;
    resp.min_value.union_tag   = UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_INTEGER_VALUE;
    resp.min_value.integer_value = 0;
    resp.max_value.union_tag   = UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_INTEGER_VALUE;
    resp.max_value.integer_value = 100;
    const char *name = "MOT_SPIN_MIN";
    resp.name.len = (uint8_t)strlen(name);
    memcpy(resp.name.data, name, resp.name.len);

    CanardRxTransfer xfer = makeParamGetSetTransfer(42, 0, &resp, buf);
    onTransferReceived(&ins, &xfer);

    ASSERT_EQ(dronecanAsyncSlot.state, DRONECAN_ASYNC_READY);
    const dronecanParamResult_t *r = &dronecanAsyncSlot.result.param;
    EXPECT_EQ(r->type,       (uint8_t)DRONECAN_PARAM_TYPE_INT);
    EXPECT_EQ(r->value_int,  42);
    EXPECT_EQ(r->name_len,   (uint8_t)strlen(name));
    EXPECT_EQ(0, memcmp(r->name, name, r->name_len));
    EXPECT_EQ(r->min_type,   (uint8_t)DRONECAN_PARAM_TYPE_INT);
    EXPECT_EQ(r->min_int,    0);
    EXPECT_EQ(r->max_type,   (uint8_t)DRONECAN_PARAM_TYPE_INT);
    EXPECT_EQ(r->max_int,    100);
}

/* GAP-S4b: Float value with float min/max range */
TEST_F(DroneCANDispatchTest, ParamGetSetFloatResponse_PopulatesSlot)
{
    dronecanAsyncSlot.state      = DRONECAN_ASYNC_PENDING;
    dronecanAsyncSlot.service_id = DRONECAN_SERVICE_PARAM_GETSET;
    dronecanAsyncSlot.node_id    = 42;
    dronecanAsyncSlot.transfer_id = 1;

    struct uavcan_protocol_param_GetSetResponse resp;
    memset(&resp, 0, sizeof(resp));
    resp.value.union_tag   = UAVCAN_PROTOCOL_PARAM_VALUE_REAL_VALUE;
    resp.value.real_value  = 3.14f;
    resp.min_value.union_tag  = UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_REAL_VALUE;
    resp.min_value.real_value = 0.0f;
    resp.max_value.union_tag  = UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_REAL_VALUE;
    resp.max_value.real_value = 10.0f;

    CanardRxTransfer xfer = makeParamGetSetTransfer(42, 0, &resp, buf);
    onTransferReceived(&ins, &xfer);

    ASSERT_EQ(dronecanAsyncSlot.state, DRONECAN_ASYNC_READY);
    const dronecanParamResult_t *r = &dronecanAsyncSlot.result.param;
    EXPECT_EQ(r->type,       (uint8_t)DRONECAN_PARAM_TYPE_FLOAT);
    EXPECT_FLOAT_EQ(r->value_float, 3.14f);
    EXPECT_EQ(r->min_type,   (uint8_t)DRONECAN_PARAM_TYPE_FLOAT);
    EXPECT_FLOAT_EQ(r->min_float, 0.0f);
    EXPECT_EQ(r->max_type,   (uint8_t)DRONECAN_PARAM_TYPE_FLOAT);
    EXPECT_FLOAT_EQ(r->max_float, 10.0f);
}

/* GAP-S4c: Boolean value (no numeric range) */
TEST_F(DroneCANDispatchTest, ParamGetSetBoolResponse_PopulatesSlot)
{
    dronecanAsyncSlot.state      = DRONECAN_ASYNC_PENDING;
    dronecanAsyncSlot.service_id = DRONECAN_SERVICE_PARAM_GETSET;
    dronecanAsyncSlot.node_id    = 42;
    dronecanAsyncSlot.transfer_id = 1;

    struct uavcan_protocol_param_GetSetResponse resp;
    memset(&resp, 0, sizeof(resp));
    resp.value.union_tag    = UAVCAN_PROTOCOL_PARAM_VALUE_BOOLEAN_VALUE;
    resp.value.boolean_value = 1;
    /* min/max remain EMPTY (memset to 0 = UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_EMPTY) */

    CanardRxTransfer xfer = makeParamGetSetTransfer(42, 0, &resp, buf);
    onTransferReceived(&ins, &xfer);

    ASSERT_EQ(dronecanAsyncSlot.state, DRONECAN_ASYNC_READY);
    const dronecanParamResult_t *r = &dronecanAsyncSlot.result.param;
    EXPECT_EQ(r->type,       (uint8_t)DRONECAN_PARAM_TYPE_BOOL);
    EXPECT_EQ(r->value_bool, 1u);
    EXPECT_EQ(r->min_type,   (uint8_t)DRONECAN_PARAM_TYPE_EMPTY);
    EXPECT_EQ(r->max_type,   (uint8_t)DRONECAN_PARAM_TYPE_EMPTY);
}

/* GAP-S4d: String value */
TEST_F(DroneCANDispatchTest, ParamGetSetStringResponse_PopulatesSlot)
{
    dronecanAsyncSlot.state      = DRONECAN_ASYNC_PENDING;
    dronecanAsyncSlot.service_id = DRONECAN_SERVICE_PARAM_GETSET;
    dronecanAsyncSlot.node_id    = 42;
    dronecanAsyncSlot.transfer_id = 1;

    struct uavcan_protocol_param_GetSetResponse resp;
    memset(&resp, 0, sizeof(resp));
    resp.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_STRING_VALUE;
    const char *str = "hello";
    resp.value.string_value.len = (uint8_t)strlen(str);
    memcpy(resp.value.string_value.data, str, resp.value.string_value.len);

    CanardRxTransfer xfer = makeParamGetSetTransfer(42, 0, &resp, buf);
    onTransferReceived(&ins, &xfer);

    ASSERT_EQ(dronecanAsyncSlot.state, DRONECAN_ASYNC_READY);
    const dronecanParamResult_t *r = &dronecanAsyncSlot.result.param;
    EXPECT_EQ(r->type,          (uint8_t)DRONECAN_PARAM_TYPE_STRING);
    EXPECT_EQ(r->value_str_len, (uint8_t)strlen(str));
    EXPECT_EQ(0, memcmp(r->value_str, str, r->value_str_len));
}

/* GAP-S4e: Empty value (unknown union_tag) → type forced to DRONECAN_PARAM_TYPE_EMPTY */
TEST_F(DroneCANDispatchTest, ParamGetSetEmptyResponse_SetsEmptyType)
{
    dronecanAsyncSlot.state      = DRONECAN_ASYNC_PENDING;
    dronecanAsyncSlot.service_id = DRONECAN_SERVICE_PARAM_GETSET;
    dronecanAsyncSlot.node_id    = 42;
    dronecanAsyncSlot.transfer_id = 1;

    struct uavcan_protocol_param_GetSetResponse resp;
    memset(&resp, 0, sizeof(resp));
    /* union_tag == 0 == UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY */
    resp.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY;

    CanardRxTransfer xfer = makeParamGetSetTransfer(42, 0, &resp, buf);
    onTransferReceived(&ins, &xfer);

    ASSERT_EQ(dronecanAsyncSlot.state, DRONECAN_ASYNC_READY);
    EXPECT_EQ(dronecanAsyncSlot.result.param.type, (uint8_t)DRONECAN_PARAM_TYPE_EMPTY);
}

/* =========================================================================
 * EXECUTE_OPCODE response decode tests (GAP-S5)
 * ========================================================================= */

/* GAP-S5a: ok=true */
TEST_F(DroneCANDispatchTest, ExecuteOpcodeOkResponse_PopulatesSlot)
{
    dronecanAsyncSlot.state      = DRONECAN_ASYNC_PENDING;
    dronecanAsyncSlot.service_id = DRONECAN_SERVICE_EXECUTE_OPCODE;
    dronecanAsyncSlot.node_id    = 42;
    dronecanAsyncSlot.transfer_id = 1;

    CanardRxTransfer xfer = makeExecuteOpcodeTransfer(42, 0, true, buf);
    onTransferReceived(&ins, &xfer);

    ASSERT_EQ(dronecanAsyncSlot.state, DRONECAN_ASYNC_READY);
    EXPECT_TRUE(dronecanAsyncSlot.result.simple.ok);
}

/* GAP-S5b: ok=false */
TEST_F(DroneCANDispatchTest, ExecuteOpcodeFailResponse_PopulatesSlot)
{
    dronecanAsyncSlot.state      = DRONECAN_ASYNC_PENDING;
    dronecanAsyncSlot.service_id = DRONECAN_SERVICE_EXECUTE_OPCODE;
    dronecanAsyncSlot.node_id    = 42;
    dronecanAsyncSlot.transfer_id = 1;

    CanardRxTransfer xfer = makeExecuteOpcodeTransfer(42, 0, false, buf);
    onTransferReceived(&ins, &xfer);

    ASSERT_EQ(dronecanAsyncSlot.state, DRONECAN_ASYNC_READY);
    EXPECT_FALSE(dronecanAsyncSlot.result.simple.ok);
}

/* =========================================================================
 * RESTART_NODE response decode test (GAP-S6)
 * ========================================================================= */

/* GAP-S6: ok=true */
TEST_F(DroneCANDispatchTest, RestartNodeOkResponse_PopulatesSlot)
{
    dronecanAsyncSlot.state      = DRONECAN_ASYNC_PENDING;
    dronecanAsyncSlot.service_id = DRONECAN_SERVICE_RESTART_NODE;
    dronecanAsyncSlot.node_id    = 42;
    dronecanAsyncSlot.transfer_id = 1;

    CanardRxTransfer xfer = makeRestartNodeTransfer(42, 0, true, buf);
    onTransferReceived(&ins, &xfer);

    ASSERT_EQ(dronecanAsyncSlot.state, DRONECAN_ASYNC_READY);
    EXPECT_TRUE(dronecanAsyncSlot.result.simple.ok);
}

/* =========================================================================
 * dronecanAsyncRequest re-entry guard test (GAP-S7)
 * ========================================================================= */

/* GAP-S7: A second async request is rejected while one is already in flight.
 * Uses RESTART_NODE (no null-payload check) so the re-entry guard is the only
 * reason dronecanAsyncRequest returns false.  Slot PENDING with
 * requested_at_ms=0 and mock_time_ms=0 keeps the timeout condition satisfied
 * (0 < DRONECAN_ASYNC_TIMEOUT_MS), so the guard fires before touching the bus. */
TEST_F(DroneCANDispatchTest, AsyncRequest_RejectedWhilePending)
{
    dronecanAsyncSlot.state           = DRONECAN_ASYNC_PENDING;
    dronecanAsyncSlot.requested_at_ms = 0;
    mock_time_ms = 0;

    EXPECT_FALSE(dronecanAsyncRequest(DRONECAN_SERVICE_RESTART_NODE, 42, nullptr));
    EXPECT_EQ(dronecanAsyncSlot.state, DRONECAN_ASYNC_PENDING);
}
