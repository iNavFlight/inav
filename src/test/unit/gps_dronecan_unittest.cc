/**
 * DroneCAN GPS Health Guard Unit Tests
 *
 * Tests the real gps_dronecan.c compiled against the real dronecan.c (for
 * the node table / health lookups) and INAV stubs. Exercises the parts of
 * the "single active GPS node" guard that dronecan_application_unittest.cc
 * cannot reach, because that file stubs out dronecanGPSReceiveGNSSFix2/
 * Auxiliary as no-ops instead of linking the real gps_dronecan.c.
 *
 * Coverage:
 *   GPS-1   First node to report Fix2 data locks in as the active node
 *   GPS-2   A second node's Fix2 data is ignored while the first is active
 *   GPS-3   dronecanGpsOnNodeEvicted() on the active node releases the lock,
 *           allowing a different node's data through afterward
 *   GPS-4   dronecanGpsOnNodeEvicted() on an unrelated node ID is a no-op —
 *           the active lock is untouched
 *   GPS-5   Data from a node whose NodeStatus health is ERROR is rejected
 *           even before any node has locked in
 *   GPS-6   dronecan_gps_node_id static filter rejects non-matching sources
 *   GPS-7   dronecan_gps_node_id static filter accepts the configured source
 *   GPS-8   Reconfiguring dronecan_gps_node_id at runtime to a different node
 *           than the one currently locked in takes effect immediately
 *   GPS-9   parseGnssTime(): UTC standard conversion
 *   GPS-10  parseGnssTime(): GPS standard leap-second offset
 *   GPS-11  parseGnssTime(): TAI standard leap-second offset
 *   GPS-12  parseGnssTime(): GPS/TAI standards reject NUM_LEAP_SECONDS_UNKNOWN
 *   GPS-13  parseGnssTime(): UAVCAN_TIMESTAMP_UNKNOWN rejects immediately
 *   GPS-14  End-to-end: a stale node purged by process1HzTasks()'s real 1Hz
 *           task releases the GPS lock, not just dronecanGpsOnNodeEvicted()
 *           called directly
 *   GPS-15  dronecanGpsIsHealthy(): no filter, no active node -> false
 *   GPS-16  dronecanGpsIsHealthy(): no filter, healthy active node -> true
 *   GPS-17  dronecanGpsIsHealthy(): no filter, active node evicted -> false
 *   GPS-18  dronecanGpsIsHealthy(): filter configured, healthy node -> true
 *   GPS-19  dronecanGpsIsHealthy(): filter configured, ERROR-health node -> false
 *   GPS-20  dronecanGpsIsHealthy(): filter configured, node never seen -> false
 */

#include "gtest/gtest.h"

extern "C" {
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "platform.h"

/* DSDL types used by dronecan.c / gps_dronecan.c handlers */
#include "uavcan.protocol.NodeStatus.h"
#include "uavcan.protocol.GetNodeInfo.h"
#include "uavcan.protocol.param.GetSet_res.h"
#include "uavcan.protocol.param.ExecuteOpcode_res.h"
#include "uavcan.protocol.RestartNode_res.h"
#include "uavcan.equipment.gnss.Fix2.h"
#include "uavcan.equipment.gnss.Auxiliary.h"

/* Canard core and STM32 driver declarations */
#include "drivers/dronecan/libcanard/canard.h"
#include "drivers/dronecan/libcanard/canard_stm32_driver.h"

/* INAV headers pulled in by dronecan.c / gps_dronecan.c — included here so
   the types are available when we define stub globals below. */
#include "io/gps.h"
#include "io/gps_private.h"
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
extern uint8_t activeGpsNodeId;
extern dronecanState_e dronecanState;
extern timeUs_t next_1hz_service_at;

/* Private functions not exposed in dronecan.h, needed to populate node health */
void dronecanNodeStatusHandleBroadcast(CanardInstance *ins, CanardRxTransfer *transfer);
bool shouldAcceptTransfer(const CanardInstance *ins,
                          uint64_t *out_data_type_signature,
                          uint16_t data_type_id,
                          CanardTransferType transfer_type,
                          uint8_t source_node_id);
void onTransferReceived(CanardInstance *ins, CanardRxTransfer *transfer);

/* =========================================================================
 * Stubs — provide every symbol dronecan.c/gps_dronecan.c reference that
 * isn't supplied by the compiled dependencies (dronecan.c, gps_dronecan.c,
 * canard.c, common/maths.c, DSDL .c files).
 * ========================================================================= */

static uint32_t mock_time_ms = 0;
uint32_t millis(void) { return mock_time_ms; }

uint32_t armingFlags = 0;

gpsConfig_t gpsConfig_System;
gpsConfig_t gpsConfig_Copy;

bool isHardwareHealthy(void) { return true; }

/* GPS core is not under test here; gps_dronecan.c only needs a place to
   write its solution and a couple of clamp helpers. Real gps.c is not
   linked in to avoid pulling in the whole GPS subsystem. */
gpsSolutionData_t gpsSolDRV;
void gpsProcessNewDriverData(void) {}
void gpsProcessNewSolutionData(bool timeout) { (void)timeout; }
uint16_t gpsConstrainHDOP(uint32_t hdop) { return (uint16_t)(hdop > 9999 ? 9999 : hdop); }
uint16_t gpsConstrainEPE(uint32_t epe)   { return (uint16_t)(epe > 9999 ? 9999 : epe); }

/* Battery receive is not under test here; dronecan.c's handle_BatteryInfo
   still needs a symbol to link against. */
void dronecanBatterySensorReceiveInfo(struct uavcan_equipment_power_BatteryInfo *p) { (void)p; }

/* Logging and config-save are no-ops; tests don't assert on either. */
void _logf(logTopic_e topic, unsigned level, const char *fmt, ...) { (void)topic; (void)level; (void)fmt; }
void saveConfig(void) {}

/* STM32 CAN driver stubs */
int16_t canardSTM32CAN1_Init(uint32_t b) { (void)b; return CANARD_OK; }
int16_t canardSTM32Receive(CanardCANFrame *f) { (void)f; return 0; }
uint32_t canardSTM32GetAndClearRxDropCount(void) { return 0; }
int16_t canardSTM32Transmit(const CanardCANFrame *f) { (void)f; return 1; }
void    canardSTM32GetProtocolStatus(canardProtocolStatus_t *s) { memset(s, 0, sizeof(*s)); }
int32_t canardSTM32GetRxFifoFillLevel(void) { return 0; }
void    canardSTM32RecoverFromBusOff(void) {}
void    canardSTM32GetUniqueID(uint8_t id[16]) { memset(id, 0, 16); }

const char* const shortGitRevision = "00000000";
const char* const compilerVersion  = "test";
const char* const targetName       = "TEST";
const char* const buildDate        = "Jan 01 2026";
const char* const buildTime        = "00:00:00";

} /* extern "C" */

/* =========================================================================
 * Helpers
 * ========================================================================= */

static CanardRxTransfer makeNodeStatusTransfer(
        uint8_t nodeId, uint8_t health, uint8_t *buf)
{
    struct uavcan_protocol_NodeStatus ns;
    memset(&ns, 0, sizeof(ns));
    ns.health = health;
    ns.mode   = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;

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

/* Builds a minimal Fix2 message whose latitude uniquely identifies which
   call produced the resulting gpsSolDRV state. */
static void makeFix2(int32_t latitude_deg_1e8, struct uavcan_equipment_gnss_Fix2 *out)
{
    memset(out, 0, sizeof(*out));
    out->latitude_deg_1e8  = latitude_deg_1e8;
    out->longitude_deg_1e8 = 0;
    out->status            = 3; /* 3D fix */
    out->sats_used          = 8;
    out->gnss_timestamp.usec = UAVCAN_TIMESTAMP_UNKNOWN;
}

class DroneCANGpsHealthGuardTest : public ::testing::Test {
protected:
    uint8_t buf[UAVCAN_PROTOCOL_NODESTATUS_MAX_SIZE + 4];

    void SetUp() override {
        activeNodeCount  = 0;
        activeGpsNodeId  = 0;
        memset(nodeTable, 0, sizeof(dronecanNodeInfo_t) * DRONECAN_MAX_NODES);
        memset(&gpsSolDRV, 0, sizeof(gpsSolDRV));
        dronecanConfigMutable()->gpsNodeId = 0;
        mock_time_ms = 0;
        dronecanState = STATE_DRONECAN_INIT;
        next_1hz_service_at = 0;
    }

    /* CanardInstance is unused by dronecanNodeStatusHandleBroadcast; pass NULL like the
       existing dronecan_application_unittest.cc does. */
    void setNodeHealth(uint8_t nodeId, uint8_t health) {
        CanardRxTransfer xfer = makeNodeStatusTransfer(nodeId, health, buf);
        dronecanNodeStatusHandleBroadcast(nullptr, &xfer);
    }

    void sendFix2(uint8_t sourceNodeId, int32_t latitude_deg_1e8) {
        struct uavcan_equipment_gnss_Fix2 fix;
        makeFix2(latitude_deg_1e8, &fix);
        dronecanGPSReceiveGNSSFix2(&fix, sourceNodeId);
    }
};

/* GPS-1: First node to report locks in as the active node */
TEST_F(DroneCANGpsHealthGuardTest, FirstNodeLocksIn)
{
    sendFix2(10, 400000000); /* 40.0 deg */

    EXPECT_EQ(gpsSolDRV.llh.lat, 40000000);
}

/* GPS-2: A second node's data is ignored while the first is locked in */
TEST_F(DroneCANGpsHealthGuardTest, SecondNodeRejectedWhileFirstActive)
{
    sendFix2(10, 400000000); /* locks node 10 in */
    sendFix2(20, 500000000); /* should be ignored */

    EXPECT_EQ(gpsSolDRV.llh.lat, 40000000) << "node 20's data must not overwrite node 10's fix";
}

/* GPS-3: Evicting the active node releases the lock for the next reporter */
TEST_F(DroneCANGpsHealthGuardTest, EvictionReleasesLockForNewNode)
{
    sendFix2(10, 400000000);
    sendFix2(20, 500000000); /* rejected, node 10 still locked */
    ASSERT_EQ(gpsSolDRV.llh.lat, 40000000);

    dronecanGpsOnNodeEvicted(10); /* node 10 dropped off the bus */
    sendFix2(20, 500000000);      /* now accepted */

    EXPECT_EQ(gpsSolDRV.llh.lat, 50000000);
}

/* GPS-4: Evicting a node that is not the active GPS node is a no-op —
   directly covers the "what if it's not a GPS node that's evicted?"
   case: dronecanGpsOnNodeEvicted must self-filter and leave the lock alone. */
TEST_F(DroneCANGpsHealthGuardTest, EvictingUnrelatedNodeDoesNotUnlock)
{
    sendFix2(10, 400000000); /* node 10 locked in */

    dronecanGpsOnNodeEvicted(99); /* some other (e.g. battery) node evicted */
    sendFix2(20, 500000000);      /* node 10 is still locked; must be rejected */

    EXPECT_EQ(gpsSolDRV.llh.lat, 40000000);
}

/* GPS-5: A node reporting HEALTH_ERROR is rejected even before any node
   has locked in as the active GPS node. */
TEST_F(DroneCANGpsHealthGuardTest, UnhealthyNodeRejectedBeforeLockingIn)
{
    setNodeHealth(30, UAVCAN_PROTOCOL_NODESTATUS_HEALTH_ERROR);

    sendFix2(30, 400000000);

    EXPECT_EQ(gpsSolDRV.llh.lat, 0) << "unhealthy node must not lock in or update gpsSolDRV";

    /* A healthy node afterward must still be able to lock in normally —
       proves the rejection above didn't leave activeGpsNodeId stuck. */
    sendFix2(10, 500000000);
    EXPECT_EQ(gpsSolDRV.llh.lat, 50000000);
}

/* GPS-6: dronecan_gps_node_id static filter rejects non-matching sources,
   even for the first message seen (checked before the first-over-fence
   lock logic). */
TEST_F(DroneCANGpsHealthGuardTest, StaticNodeIdFilterRejectsNonMatchingSource)
{
    dronecanConfigMutable()->gpsNodeId = 15;

    sendFix2(10, 400000000);

    EXPECT_EQ(gpsSolDRV.llh.lat, 0) << "node 10 does not match the configured static gpsNodeId 15";
}

/* GPS-7: dronecan_gps_node_id static filter accepts the configured source */
TEST_F(DroneCANGpsHealthGuardTest, StaticNodeIdFilterAcceptsMatchingSource)
{
    dronecanConfigMutable()->gpsNodeId = 15;

    sendFix2(15, 400000000);

    EXPECT_EQ(gpsSolDRV.llh.lat, 40000000);
}

/* GPS-8: Reconfiguring dronecan_gps_node_id at runtime to a node other than
 * the one currently locked in by first-over-fence must take effect
 * immediately, not get stuck rejecting the newly configured node forever.
 * Node 10 locks in under accept-any (gpsNodeId=0), then the user
 * reconfigures to node 20 without rebooting. */
TEST_F(DroneCANGpsHealthGuardTest, ReconfiguringNodeIdWhileLockedToOtherNode)
{
    sendFix2(10, 400000000); /* locks in under accept-any (gpsNodeId=0) */
    ASSERT_EQ(gpsSolDRV.llh.lat, 40000000);

    dronecanConfigMutable()->gpsNodeId = 20; /* reconfigure without reboot */
    sendFix2(20, 500000000);

    EXPECT_EQ(gpsSolDRV.llh.lat, 50000000)
        << "node 20 matches the newly configured gpsNodeId but is stuck rejected "
           "because activeGpsNodeId was never released";
}

/* =========================================================================
 * parseGnssTime coverage (GPS-9 … GPS-13)
 *
 * makeFix2() defaults gnss_timestamp.usec to UAVCAN_TIMESTAMP_UNKNOWN, so
 * none of the tests above ever exercise the UTC/GPS/TAI conversion branches
 * or the leap-second guards. These call dronecanGPSReceiveGNSSFix2()
 * directly with a real timestamp and independently derive the expected
 * calendar fields via gmtime() on the same epoch-conversion formula
 * documented in the Fix2 DSDL, rather than hand-transcribing dates.
 * ========================================================================= */

static void expectGpsSolTimeMatches(time_t expected_unix_s)
{
    struct tm *t = gmtime(&expected_unix_s);
    ASSERT_NE(t, nullptr);
    EXPECT_TRUE(gpsSolDRV.flags.validTime);
    EXPECT_EQ(gpsSolDRV.time.year,    (uint16_t)(t->tm_year + 1900));
    EXPECT_EQ(gpsSolDRV.time.month,   (uint8_t)(t->tm_mon + 1));
    EXPECT_EQ(gpsSolDRV.time.day,     (uint8_t)t->tm_mday);
    EXPECT_EQ(gpsSolDRV.time.hours,   (uint8_t)t->tm_hour);
    EXPECT_EQ(gpsSolDRV.time.minutes, (uint8_t)t->tm_min);
    EXPECT_EQ(gpsSolDRV.time.seconds, (uint8_t)t->tm_sec);
}

/* GPS-9: UTC standard converts usec directly to unix seconds, no leap-second offset */
TEST_F(DroneCANGpsHealthGuardTest, GnssTimeUtcStandardPopulatesDate)
{
    const uint64_t epoch_s = 1700000000ULL;
    struct uavcan_equipment_gnss_Fix2 fix;
    makeFix2(400000000, &fix);
    fix.gnss_timestamp.usec = epoch_s * 1000000ULL;
    fix.gnss_time_standard  = UAVCAN_EQUIPMENT_GNSS_FIX2_GNSS_TIME_STANDARD_UTC;
    fix.num_leap_seconds    = UAVCAN_EQUIPMENT_GNSS_FIX2_NUM_LEAP_SECONDS_UNKNOWN; /* UTC ignores this field */

    dronecanGPSReceiveGNSSFix2(&fix, 10);

    expectGpsSolTimeMatches((time_t)epoch_s);
}

/* GPS-10: GPS standard applies UTC = GPS - leap_seconds + 9 per the DSDL comment */
TEST_F(DroneCANGpsHealthGuardTest, GnssTimeGpsStandardAppliesLeapSecondOffset)
{
    const uint64_t gps_epoch_s = 1700000000ULL;
    const uint8_t  leap_seconds = 18;
    struct uavcan_equipment_gnss_Fix2 fix;
    makeFix2(400000000, &fix);
    fix.gnss_timestamp.usec = gps_epoch_s * 1000000ULL;
    fix.gnss_time_standard  = UAVCAN_EQUIPMENT_GNSS_FIX2_GNSS_TIME_STANDARD_GPS;
    fix.num_leap_seconds    = leap_seconds;

    dronecanGPSReceiveGNSSFix2(&fix, 10);

    expectGpsSolTimeMatches((time_t)(gps_epoch_s - leap_seconds + 9));
}

/* GPS-11: TAI standard applies UTC = TAI - leap_seconds - 10 per the DSDL comment */
TEST_F(DroneCANGpsHealthGuardTest, GnssTimeTaiStandardAppliesLeapSecondOffset)
{
    const uint64_t tai_epoch_s = 1700000000ULL;
    const uint8_t  leap_seconds = 18;
    struct uavcan_equipment_gnss_Fix2 fix;
    makeFix2(400000000, &fix);
    fix.gnss_timestamp.usec = tai_epoch_s * 1000000ULL;
    fix.gnss_time_standard  = UAVCAN_EQUIPMENT_GNSS_FIX2_GNSS_TIME_STANDARD_TAI;
    fix.num_leap_seconds    = leap_seconds;

    dronecanGPSReceiveGNSSFix2(&fix, 10);

    expectGpsSolTimeMatches((time_t)(tai_epoch_s - leap_seconds - 10));
}

/* GPS-12: GPS/TAI standards require num_leap_seconds; UNKNOWN must reject rather
 * than silently compute a wrong offset. */
TEST_F(DroneCANGpsHealthGuardTest, GnssTimeGpsStandardWithUnknownLeapSecondsRejected)
{
    struct uavcan_equipment_gnss_Fix2 fix;
    makeFix2(400000000, &fix);
    fix.gnss_timestamp.usec = 1700000000ULL * 1000000ULL;
    fix.gnss_time_standard  = UAVCAN_EQUIPMENT_GNSS_FIX2_GNSS_TIME_STANDARD_GPS;
    fix.num_leap_seconds    = UAVCAN_EQUIPMENT_GNSS_FIX2_NUM_LEAP_SECONDS_UNKNOWN;

    dronecanGPSReceiveGNSSFix2(&fix, 10);

    EXPECT_FALSE(gpsSolDRV.flags.validTime);
}

/* GPS-13: usec == UAVCAN_TIMESTAMP_UNKNOWN rejects immediately, regardless of
 * time_standard - this is the default makeFix2() takes in every other test,
 * so pin it explicitly here rather than leaving it only implicit. */
TEST_F(DroneCANGpsHealthGuardTest, GnssTimeUnknownTimestampRejected)
{
    struct uavcan_equipment_gnss_Fix2 fix;
    makeFix2(400000000, &fix); /* gnss_timestamp.usec defaults to UAVCAN_TIMESTAMP_UNKNOWN */

    dronecanGPSReceiveGNSSFix2(&fix, 10);

    EXPECT_FALSE(gpsSolDRV.flags.validTime);
}

/* GPS-14: End-to-end wiring check. GPS-3 above calls dronecanGpsOnNodeEvicted()
 * directly and only proves that function works in isolation - it would still
 * pass even if process1HzTasks() never called it. This test drives the real
 * path: a node goes stale in the node table, dronecanUpdate() runs its 1Hz
 * task, and *that* eviction is what must release the GPS lock.
 *
 * dronecanState and the 1Hz scheduler's next_1hz_service_at are reset in
 * SetUp() (exposed non-static under UNIT_TEST), so this test's sequencing
 * from STATE_DRONECAN_INIT / next_1hz_service_at=0 holds regardless of test
 * order, shuffling, or repetition.
 */
TEST_F(DroneCANGpsHealthGuardTest, StaleNodeEvictionThroughOneHzTaskReleasesGpsLock)
{
    dronecanInit(); /* sets up the real canard instance process1HzTasks uses */

    setNodeHealth(10, UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK); /* last_seen_ms = 0 */
    sendFix2(10, 400000000);
    ASSERT_EQ(gpsSolDRV.llh.lat, 40000000);

    dronecanUpdate(0); /* STATE_INIT -> STATE_NORMAL; schedules next 1Hz tick at 1,000,000us */

    mock_time_ms = DRONECAN_NODE_STALE_TIMEOUT_MS + 1; /* node 10 is now stale */
    dronecanUpdate(1000001); /* crosses the 1Hz boundary -> process1HzTasks() purges node 10 */

    ASSERT_EQ(dronecanGetNodeCount(), 0u) << "stale node 10 must have been purged from the table";

    sendFix2(20, 500000000); /* must be accepted now that node 10's eviction released the lock */

    EXPECT_EQ(gpsSolDRV.llh.lat, 50000000)
        << "process1HzTasks() evicted node 10 but never told the GPS layer, "
           "so the lock was never released";
}

/* =========================================================================
 * dronecanGpsIsHealthy() coverage (GPS-15 … GPS-20)
 *
 * None of the tests above ever call dronecanGpsIsHealthy() directly - the
 * function the "health guard" is named for had no direct coverage at all.
 * These pin its behavior across every filter-on/off x node
 * present/absent/unhealthy/evicted combination.
 * ========================================================================= */

/* GPS-15: no filter, no node has ever locked in -> unhealthy */
TEST_F(DroneCANGpsHealthGuardTest, IsHealthy_NoFilterNoActiveNode_ReturnsFalse)
{
    EXPECT_FALSE(dronecanGpsIsHealthy());
}

/* GPS-16: no filter, active node is healthy -> healthy */
TEST_F(DroneCANGpsHealthGuardTest, IsHealthy_NoFilterHealthyActiveNode_ReturnsTrue)
{
    setNodeHealth(10, UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK);
    sendFix2(10, 400000000); /* locks activeGpsNodeId = 10 */

    EXPECT_TRUE(dronecanGpsIsHealthy());
}

/* GPS-17: no filter, active node evicted -> unhealthy again */
TEST_F(DroneCANGpsHealthGuardTest, IsHealthy_NoFilterActiveNodeEvicted_ReturnsFalse)
{
    setNodeHealth(10, UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK);
    sendFix2(10, 400000000);
    ASSERT_TRUE(dronecanGpsIsHealthy());

    dronecanGpsOnNodeEvicted(10);

    EXPECT_FALSE(dronecanGpsIsHealthy());
}

/* GPS-18: static filter configured, configured node is healthy in the table
 * -> healthy, regardless of whether any Fix2/Auxiliary message has been
 * processed yet (dronecanGpsIsHealthy() checks the configured gpsNodeId
 * directly, not activeGpsNodeId, when a filter is set). */
TEST_F(DroneCANGpsHealthGuardTest, IsHealthy_FilterConfiguredHealthyNode_ReturnsTrue)
{
    dronecanConfigMutable()->gpsNodeId = 15;
    setNodeHealth(15, UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK);

    EXPECT_TRUE(dronecanGpsIsHealthy());
}

/* GPS-19: static filter configured, configured node reports ERROR health -> unhealthy.
 * A different, healthy node (20) is planted as activeGpsNodeId first so this
 * only passes if the configured gpsNodeId (15) is actually being consulted -
 * if the code incorrectly fell back to activeGpsNodeId, it would see node 20
 * (healthy) and wrongly return true instead of false. */
TEST_F(DroneCANGpsHealthGuardTest, IsHealthy_FilterConfiguredErrorHealthNode_ReturnsFalse)
{
    setNodeHealth(20, UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK);
    sendFix2(20, 100000000); /* locks activeGpsNodeId = 20, a healthy decoy */

    dronecanConfigMutable()->gpsNodeId = 15;
    setNodeHealth(15, UAVCAN_PROTOCOL_NODESTATUS_HEALTH_ERROR);

    EXPECT_FALSE(dronecanGpsIsHealthy());
}

/* GPS-20: static filter configured, configured node has never sent NodeStatus
 * -> unhealthy (fails conservative: no info means "can't confirm healthy",
 * not "assume healthy"). Same healthy-decoy-on-activeGpsNodeId setup as
 * GPS-19, so this can't pass merely because activeGpsNodeId defaults to 0. */
TEST_F(DroneCANGpsHealthGuardTest, IsHealthy_FilterConfiguredNodeNeverSeen_ReturnsFalse)
{
    setNodeHealth(20, UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK);
    sendFix2(20, 100000000); /* locks activeGpsNodeId = 20, a healthy decoy */

    dronecanConfigMutable()->gpsNodeId = 15; /* node 15 never added to nodeTable */

    EXPECT_FALSE(dronecanGpsIsHealthy());
}
