/**
 * DroneCAN Message Encoding/Decoding Unit Tests
 *
 * Tests for UAVCAN/DroneCAN DSDL message serialization and deserialization.
 * Part of PR #11313 - DroneCAN/Libcanard implementation
 */

#include <cstdint>
#include <cstring>
#include <cmath>

extern "C" {
#include "drivers/dronecan/libcanard/canard.h"
#include "uavcan.equipment.gnss.Fix2.h"
#include "uavcan.equipment.gnss.Fix.h"
#include "uavcan.equipment.gnss.Auxiliary.h"
#include "uavcan.equipment.power.BatteryInfo.h"
#include "uavcan.protocol.NodeStatus.h"
}

#include "gtest/gtest.h"

// Test fixture for DroneCAN message tests
class DroneCANMessageTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test buffer
        memset(buffer, 0, sizeof(buffer));
    }

    void TearDown() override {
        // Cleanup if needed
    }

    // Helper to set up a CanardRxTransfer from an encoded buffer
    CanardRxTransfer makeTransfer(uint32_t len) {
        CanardRxTransfer transfer;
        memset(&transfer, 0, sizeof(transfer));
        transfer.payload_len = len;
        transfer.payload_head = buffer;
        transfer.payload_middle = NULL;
        transfer.payload_tail = NULL;
        return transfer;
    }

    uint8_t buffer[256];
};

// ===========================================================================
// GNSS Fix2 Tests
// ===========================================================================

TEST_F(DroneCANMessageTest, GNSSFix2_BasicEncoDecode)
{
    struct uavcan_equipment_gnss_Fix2 tx_msg;
    memset(&tx_msg, 0, sizeof(tx_msg));

    tx_msg.latitude_deg_1e8 = 377749000;
    tx_msg.longitude_deg_1e8 = -1224194000;
    tx_msg.height_msl_mm = 16000;
    tx_msg.height_ellipsoid_mm = 16000;
    tx_msg.ned_velocity[0] = 10.0f;
    tx_msg.ned_velocity[1] = 5.0f;
    tx_msg.ned_velocity[2] = -1.0f;
    tx_msg.sats_used = 12;
    tx_msg.status = UAVCAN_EQUIPMENT_GNSS_FIX2_STATUS_3D_FIX;
    tx_msg.mode = UAVCAN_EQUIPMENT_GNSS_FIX2_MODE_SINGLE;
    tx_msg.pdop = 1.5f;

    uint32_t encoded_len = uavcan_equipment_gnss_Fix2_encode(&tx_msg, buffer
#if CANARD_ENABLE_TAO_OPTION
        , false
#endif
    );

    EXPECT_GT(encoded_len, 0u);
    EXPECT_LE(encoded_len, UAVCAN_EQUIPMENT_GNSS_FIX2_MAX_SIZE);

    CanardRxTransfer transfer = makeTransfer(encoded_len);

    struct uavcan_equipment_gnss_Fix2 rx_msg;
    memset(&rx_msg, 0, sizeof(rx_msg));
    bool decode_result = uavcan_equipment_gnss_Fix2_decode(&transfer, &rx_msg);

    EXPECT_FALSE(decode_result);

    EXPECT_EQ(rx_msg.latitude_deg_1e8, tx_msg.latitude_deg_1e8);
    EXPECT_EQ(rx_msg.longitude_deg_1e8, tx_msg.longitude_deg_1e8);
    EXPECT_EQ(rx_msg.height_msl_mm, tx_msg.height_msl_mm);
    EXPECT_EQ(rx_msg.height_ellipsoid_mm, tx_msg.height_ellipsoid_mm);
    EXPECT_FLOAT_EQ(rx_msg.ned_velocity[0], tx_msg.ned_velocity[0]);
    EXPECT_FLOAT_EQ(rx_msg.ned_velocity[1], tx_msg.ned_velocity[1]);
    EXPECT_FLOAT_EQ(rx_msg.ned_velocity[2], tx_msg.ned_velocity[2]);
    EXPECT_EQ(rx_msg.sats_used, tx_msg.sats_used);
    EXPECT_EQ(rx_msg.status, tx_msg.status);
    EXPECT_EQ(rx_msg.mode, tx_msg.mode);
    EXPECT_NEAR(rx_msg.pdop, tx_msg.pdop, 0.01f);
}

TEST_F(DroneCANMessageTest, GNSSFix2_BoundaryValues)
{
    struct uavcan_equipment_gnss_Fix2 tx_msg;
    memset(&tx_msg, 0, sizeof(tx_msg));

    tx_msg.latitude_deg_1e8 = 9000000000LL;
    tx_msg.longitude_deg_1e8 = 0;
    tx_msg.height_msl_mm = 0;
    tx_msg.sats_used = 4;
    tx_msg.status = UAVCAN_EQUIPMENT_GNSS_FIX2_STATUS_3D_FIX;

    uint32_t encoded_len = uavcan_equipment_gnss_Fix2_encode(&tx_msg, buffer
#if CANARD_ENABLE_TAO_OPTION
        , false
#endif
    );
    EXPECT_GT(encoded_len, 0u);

    CanardRxTransfer transfer = makeTransfer(encoded_len);

    struct uavcan_equipment_gnss_Fix2 rx_msg;
    bool result = uavcan_equipment_gnss_Fix2_decode(&transfer, &rx_msg);

    EXPECT_FALSE(result);
    EXPECT_EQ(rx_msg.latitude_deg_1e8, 9000000000LL);
    EXPECT_EQ(rx_msg.longitude_deg_1e8, 0);
}

TEST_F(DroneCANMessageTest, GNSSFix2_ModeSubModeValues)
{
    // Test RTK modes: SINGLE, DGPS, RTK (float/fixed sub_modes)
    struct {
        uint8_t mode;
        uint8_t sub_mode;
    } test_cases[] = {
        {UAVCAN_EQUIPMENT_GNSS_FIX2_MODE_SINGLE, 0},
        {UAVCAN_EQUIPMENT_GNSS_FIX2_MODE_DGPS,   UAVCAN_EQUIPMENT_GNSS_FIX2_SUB_MODE_DGPS_SBAS},
        {UAVCAN_EQUIPMENT_GNSS_FIX2_MODE_RTK,     UAVCAN_EQUIPMENT_GNSS_FIX2_SUB_MODE_RTK_FLOAT},
        {UAVCAN_EQUIPMENT_GNSS_FIX2_MODE_RTK,     UAVCAN_EQUIPMENT_GNSS_FIX2_SUB_MODE_RTK_FIXED},
        {UAVCAN_EQUIPMENT_GNSS_FIX2_MODE_PPP,     0},
    };

    for (const auto& tc : test_cases) {
        struct uavcan_equipment_gnss_Fix2 tx_msg;
        memset(&tx_msg, 0, sizeof(tx_msg));
        tx_msg.mode = tc.mode;
        tx_msg.sub_mode = tc.sub_mode;
        tx_msg.status = UAVCAN_EQUIPMENT_GNSS_FIX2_STATUS_3D_FIX;

        memset(buffer, 0, sizeof(buffer));
        uint32_t encoded_len = uavcan_equipment_gnss_Fix2_encode(&tx_msg, buffer
#if CANARD_ENABLE_TAO_OPTION
            , false
#endif
        );
        EXPECT_GT(encoded_len, 0u);

        CanardRxTransfer transfer = makeTransfer(encoded_len);
        struct uavcan_equipment_gnss_Fix2 rx_msg;
        memset(&rx_msg, 0, sizeof(rx_msg));
        EXPECT_FALSE(uavcan_equipment_gnss_Fix2_decode(&transfer, &rx_msg));
        EXPECT_EQ(rx_msg.mode, tc.mode);
        EXPECT_EQ(rx_msg.sub_mode, tc.sub_mode);
    }
}

// ===========================================================================
// GNSS Fix (v1) Tests
// ===========================================================================

TEST_F(DroneCANMessageTest, GNSSFix_BasicEncodeDecode)
{
    struct uavcan_equipment_gnss_Fix tx_msg;
    memset(&tx_msg, 0, sizeof(tx_msg));

    tx_msg.latitude_deg_1e8 = 377749000;
    tx_msg.longitude_deg_1e8 = -1224194000;
    tx_msg.height_msl_mm = 16000;
    tx_msg.height_ellipsoid_mm = 16000;
    tx_msg.ned_velocity[0] = 10.0f;
    tx_msg.ned_velocity[1] = 5.0f;
    tx_msg.ned_velocity[2] = -1.0f;
    tx_msg.sats_used = 12;
    tx_msg.status = UAVCAN_EQUIPMENT_GNSS_FIX_STATUS_3D_FIX;
    tx_msg.pdop = 1.5f;

    uint32_t encoded_len = uavcan_equipment_gnss_Fix_encode(&tx_msg, buffer
#if CANARD_ENABLE_TAO_OPTION
        , false
#endif
    );

    EXPECT_GT(encoded_len, 0u);
    EXPECT_LE(encoded_len, UAVCAN_EQUIPMENT_GNSS_FIX_MAX_SIZE);

    CanardRxTransfer transfer = makeTransfer(encoded_len);

    struct uavcan_equipment_gnss_Fix rx_msg;
    memset(&rx_msg, 0, sizeof(rx_msg));
    bool result = uavcan_equipment_gnss_Fix_decode(&transfer, &rx_msg);

    EXPECT_FALSE(result);
    EXPECT_EQ(rx_msg.latitude_deg_1e8, tx_msg.latitude_deg_1e8);
    EXPECT_EQ(rx_msg.longitude_deg_1e8, tx_msg.longitude_deg_1e8);
    EXPECT_EQ(rx_msg.height_msl_mm, tx_msg.height_msl_mm);
    EXPECT_EQ(rx_msg.sats_used, tx_msg.sats_used);
    EXPECT_EQ(rx_msg.status, tx_msg.status);
    // Fix v1 uses float16 for velocity, so allow larger tolerance
    EXPECT_NEAR(rx_msg.ned_velocity[0], tx_msg.ned_velocity[0], 0.1f);
    EXPECT_NEAR(rx_msg.ned_velocity[1], tx_msg.ned_velocity[1], 0.1f);
    EXPECT_NEAR(rx_msg.ned_velocity[2], tx_msg.ned_velocity[2], 0.1f);
    EXPECT_NEAR(rx_msg.pdop, tx_msg.pdop, 0.01f);
}

TEST_F(DroneCANMessageTest, GNSSFix_CovarianceArrays)
{
    // Test with empty covariance arrays
    {
        struct uavcan_equipment_gnss_Fix tx_msg;
        memset(&tx_msg, 0, sizeof(tx_msg));
        tx_msg.status = UAVCAN_EQUIPMENT_GNSS_FIX_STATUS_3D_FIX;
        tx_msg.position_covariance.len = 0;
        tx_msg.velocity_covariance.len = 0;

        memset(buffer, 0, sizeof(buffer));
        uint32_t encoded_len = uavcan_equipment_gnss_Fix_encode(&tx_msg, buffer
#if CANARD_ENABLE_TAO_OPTION
            , false
#endif
        );

        CanardRxTransfer transfer = makeTransfer(encoded_len);
        struct uavcan_equipment_gnss_Fix rx_msg;
        memset(&rx_msg, 0, sizeof(rx_msg));
        EXPECT_FALSE(uavcan_equipment_gnss_Fix_decode(&transfer, &rx_msg));
        EXPECT_EQ(rx_msg.position_covariance.len, 0);
        EXPECT_EQ(rx_msg.velocity_covariance.len, 0);
    }

    // Test with 3-element covariance (diagonal only)
    {
        struct uavcan_equipment_gnss_Fix tx_msg;
        memset(&tx_msg, 0, sizeof(tx_msg));
        tx_msg.status = UAVCAN_EQUIPMENT_GNSS_FIX_STATUS_3D_FIX;
        tx_msg.position_covariance.len = 3;
        tx_msg.position_covariance.data[0] = 1.0f;
        tx_msg.position_covariance.data[1] = 2.0f;
        tx_msg.position_covariance.data[2] = 3.0f;
        tx_msg.velocity_covariance.len = 3;
        tx_msg.velocity_covariance.data[0] = 0.1f;
        tx_msg.velocity_covariance.data[1] = 0.2f;
        tx_msg.velocity_covariance.data[2] = 0.3f;

        memset(buffer, 0, sizeof(buffer));
        uint32_t encoded_len = uavcan_equipment_gnss_Fix_encode(&tx_msg, buffer
#if CANARD_ENABLE_TAO_OPTION
            , false
#endif
        );

        CanardRxTransfer transfer = makeTransfer(encoded_len);
        struct uavcan_equipment_gnss_Fix rx_msg;
        memset(&rx_msg, 0, sizeof(rx_msg));
        EXPECT_FALSE(uavcan_equipment_gnss_Fix_decode(&transfer, &rx_msg));
        EXPECT_EQ(rx_msg.position_covariance.len, 3);
        EXPECT_NEAR(rx_msg.position_covariance.data[0], 1.0f, 0.01f);
        EXPECT_NEAR(rx_msg.position_covariance.data[1], 2.0f, 0.01f);
        EXPECT_NEAR(rx_msg.position_covariance.data[2], 3.0f, 0.01f);
        EXPECT_EQ(rx_msg.velocity_covariance.len, 3);
        EXPECT_NEAR(rx_msg.velocity_covariance.data[0], 0.1f, 0.001f);
    }
}

// ===========================================================================
// GNSS Auxiliary Tests
// ===========================================================================

TEST_F(DroneCANMessageTest, GNSSAuxiliary_BasicEncodeDecode)
{
    struct uavcan_equipment_gnss_Auxiliary tx_msg;
    memset(&tx_msg, 0, sizeof(tx_msg));

    tx_msg.gdop = 2.5f;
    tx_msg.pdop = 1.5f;
    tx_msg.hdop = 1.2f;
    tx_msg.vdop = 0.9f;
    tx_msg.tdop = 0.0f;
    tx_msg.ndop = 0.8f;
    tx_msg.edop = 0.7f;
    tx_msg.sats_visible = 24;
    tx_msg.sats_used = 12;

    uint32_t encoded_len = uavcan_equipment_gnss_Auxiliary_encode(&tx_msg, buffer
#if CANARD_ENABLE_TAO_OPTION
        , false
#endif
    );

    EXPECT_GT(encoded_len, 0u);
    EXPECT_LE(encoded_len, UAVCAN_EQUIPMENT_GNSS_AUXILIARY_MAX_SIZE);

    CanardRxTransfer transfer = makeTransfer(encoded_len);

    struct uavcan_equipment_gnss_Auxiliary rx_msg;
    memset(&rx_msg, 0, sizeof(rx_msg));
    bool result = uavcan_equipment_gnss_Auxiliary_decode(&transfer, &rx_msg);

    EXPECT_FALSE(result);
    // DOP values use float16, allow tolerance
    EXPECT_NEAR(rx_msg.gdop, tx_msg.gdop, 0.01f);
    EXPECT_NEAR(rx_msg.pdop, tx_msg.pdop, 0.01f);
    EXPECT_NEAR(rx_msg.hdop, tx_msg.hdop, 0.01f);
    EXPECT_NEAR(rx_msg.vdop, tx_msg.vdop, 0.01f);
    EXPECT_NEAR(rx_msg.ndop, tx_msg.ndop, 0.01f);
    EXPECT_NEAR(rx_msg.edop, tx_msg.edop, 0.01f);
    EXPECT_EQ(rx_msg.sats_visible, 24);
    EXPECT_EQ(rx_msg.sats_used, 12);
}

// ===========================================================================
// NodeStatus Tests
// ===========================================================================

TEST_F(DroneCANMessageTest, NodeStatus_BasicEncodeDecode)
{
    struct uavcan_protocol_NodeStatus tx_msg;
    memset(&tx_msg, 0, sizeof(tx_msg));

    tx_msg.uptime_sec = 12345;
    tx_msg.health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
    tx_msg.mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;
    tx_msg.sub_mode = 0;
    tx_msg.vendor_specific_status_code = 0xABCD;

    uint32_t encoded_len = uavcan_protocol_NodeStatus_encode(&tx_msg, buffer
#if CANARD_ENABLE_TAO_OPTION
        , false
#endif
    );

    EXPECT_GT(encoded_len, 0u);
    EXPECT_LE(encoded_len, UAVCAN_PROTOCOL_NODESTATUS_MAX_SIZE);

    CanardRxTransfer transfer = makeTransfer(encoded_len);

    struct uavcan_protocol_NodeStatus rx_msg;
    memset(&rx_msg, 0, sizeof(rx_msg));
    bool result = uavcan_protocol_NodeStatus_decode(&transfer, &rx_msg);

    EXPECT_FALSE(result);
    EXPECT_EQ(rx_msg.uptime_sec, 12345u);
    EXPECT_EQ(rx_msg.health, UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK);
    EXPECT_EQ(rx_msg.mode, UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL);
    EXPECT_EQ(rx_msg.sub_mode, 0);
    EXPECT_EQ(rx_msg.vendor_specific_status_code, 0xABCD);
}

TEST_F(DroneCANMessageTest, NodeStatus_AllHealthModes)
{
    uint8_t health_values[] = {
        UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK,
        UAVCAN_PROTOCOL_NODESTATUS_HEALTH_WARNING,
        UAVCAN_PROTOCOL_NODESTATUS_HEALTH_ERROR,
        UAVCAN_PROTOCOL_NODESTATUS_HEALTH_CRITICAL,
    };
    uint8_t mode_values[] = {
        UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL,
        UAVCAN_PROTOCOL_NODESTATUS_MODE_INITIALIZATION,
        UAVCAN_PROTOCOL_NODESTATUS_MODE_MAINTENANCE,
        UAVCAN_PROTOCOL_NODESTATUS_MODE_SOFTWARE_UPDATE,
        UAVCAN_PROTOCOL_NODESTATUS_MODE_OFFLINE,
    };

    for (uint8_t health : health_values) {
        for (uint8_t mode : mode_values) {
            struct uavcan_protocol_NodeStatus tx_msg;
            memset(&tx_msg, 0, sizeof(tx_msg));
            tx_msg.uptime_sec = 1000;
            tx_msg.health = health;
            tx_msg.mode = mode;

            memset(buffer, 0, sizeof(buffer));
            uint32_t encoded_len = uavcan_protocol_NodeStatus_encode(&tx_msg, buffer
#if CANARD_ENABLE_TAO_OPTION
                , false
#endif
            );
            EXPECT_GT(encoded_len, 0u);

            CanardRxTransfer transfer = makeTransfer(encoded_len);
            struct uavcan_protocol_NodeStatus rx_msg;
            memset(&rx_msg, 0, sizeof(rx_msg));
            EXPECT_FALSE(uavcan_protocol_NodeStatus_decode(&transfer, &rx_msg));
            EXPECT_EQ(rx_msg.health, health);
            EXPECT_EQ(rx_msg.mode, mode);
        }
    }
}

// ===========================================================================
// BatteryInfo Tests
// ===========================================================================

TEST_F(DroneCANMessageTest, BatteryInfo_BasicEncodeDecode)
{
    struct uavcan_equipment_power_BatteryInfo tx_msg;
    memset(&tx_msg, 0, sizeof(tx_msg));

    tx_msg.voltage = 16.8f;
    tx_msg.current = 30.0f;
    tx_msg.temperature = 25.0f;
    tx_msg.state_of_charge_pct = 85;
    tx_msg.state_of_health_pct = 100;
    tx_msg.full_charge_capacity_wh = 100.0f;
    tx_msg.battery_id = 1;

    uint32_t encoded_len = uavcan_equipment_power_BatteryInfo_encode(&tx_msg, buffer
#if CANARD_ENABLE_TAO_OPTION
        , false
#endif
    );

    EXPECT_GT(encoded_len, 0u);
    EXPECT_LE(encoded_len, UAVCAN_EQUIPMENT_POWER_BATTERYINFO_MAX_SIZE);

    CanardRxTransfer transfer = makeTransfer(encoded_len);

    struct uavcan_equipment_power_BatteryInfo rx_msg;
    memset(&rx_msg, 0, sizeof(rx_msg));
    bool result = uavcan_equipment_power_BatteryInfo_decode(&transfer, &rx_msg);

    EXPECT_FALSE(result);
    EXPECT_NEAR(rx_msg.voltage, tx_msg.voltage, 0.1f);
    EXPECT_NEAR(rx_msg.current, tx_msg.current, 0.1f);
    EXPECT_NEAR(rx_msg.temperature, tx_msg.temperature, 0.1f);
    EXPECT_EQ(rx_msg.state_of_charge_pct, tx_msg.state_of_charge_pct);
    EXPECT_EQ(rx_msg.state_of_health_pct, tx_msg.state_of_health_pct);
    EXPECT_EQ(rx_msg.battery_id, tx_msg.battery_id);
}

TEST_F(DroneCANMessageTest, BatteryInfo_BoundaryValues)
{
    // Test zero voltage (empty/disconnected battery)
    {
        struct uavcan_equipment_power_BatteryInfo tx_msg;
        memset(&tx_msg, 0, sizeof(tx_msg));
        tx_msg.voltage = 0.0f;
        tx_msg.current = 0.0f;
        tx_msg.battery_id = 0;

        memset(buffer, 0, sizeof(buffer));
        uint32_t encoded_len = uavcan_equipment_power_BatteryInfo_encode(&tx_msg, buffer
#if CANARD_ENABLE_TAO_OPTION
            , false
#endif
        );

        CanardRxTransfer transfer = makeTransfer(encoded_len);
        struct uavcan_equipment_power_BatteryInfo rx_msg;
        memset(&rx_msg, 0, sizeof(rx_msg));
        EXPECT_FALSE(uavcan_equipment_power_BatteryInfo_decode(&transfer, &rx_msg));
        EXPECT_NEAR(rx_msg.voltage, 0.0f, 0.001f);
        EXPECT_NEAR(rx_msg.current, 0.0f, 0.001f);
    }

    // Test high voltage (14S LiPo, ~60V) and high current (~200A)
    {
        struct uavcan_equipment_power_BatteryInfo tx_msg;
        memset(&tx_msg, 0, sizeof(tx_msg));
        tx_msg.voltage = 58.8f;  // 14S LiPo fully charged
        tx_msg.current = 200.0f; // High current draw
        tx_msg.battery_id = 255;

        memset(buffer, 0, sizeof(buffer));
        uint32_t encoded_len = uavcan_equipment_power_BatteryInfo_encode(&tx_msg, buffer
#if CANARD_ENABLE_TAO_OPTION
            , false
#endif
        );

        CanardRxTransfer transfer = makeTransfer(encoded_len);
        struct uavcan_equipment_power_BatteryInfo rx_msg;
        memset(&rx_msg, 0, sizeof(rx_msg));
        EXPECT_FALSE(uavcan_equipment_power_BatteryInfo_decode(&transfer, &rx_msg));
        // Float16 has limited precision at higher values
        EXPECT_NEAR(rx_msg.voltage, 58.8f, 0.1f);
        EXPECT_NEAR(rx_msg.current, 200.0f, 0.5f);
        EXPECT_EQ(rx_msg.battery_id, 255);
    }
}

// ===========================================================================
// Constants and Data Type Verification Tests
// ===========================================================================

TEST(DroneCANConstants, GNSSFixStatusValues)
{
    EXPECT_EQ(UAVCAN_EQUIPMENT_GNSS_FIX2_STATUS_NO_FIX, 0);
    EXPECT_EQ(UAVCAN_EQUIPMENT_GNSS_FIX2_STATUS_TIME_ONLY, 1);
    EXPECT_EQ(UAVCAN_EQUIPMENT_GNSS_FIX2_STATUS_2D_FIX, 2);
    EXPECT_EQ(UAVCAN_EQUIPMENT_GNSS_FIX2_STATUS_3D_FIX, 3);
}

TEST(DroneCANConstants, MessageSizes)
{
    EXPECT_GT(UAVCAN_EQUIPMENT_GNSS_FIX2_MAX_SIZE, 0);
    EXPECT_LT(UAVCAN_EQUIPMENT_GNSS_FIX2_MAX_SIZE, 256);

    EXPECT_GT(UAVCAN_EQUIPMENT_POWER_BATTERYINFO_MAX_SIZE, 0);
    EXPECT_LT(UAVCAN_EQUIPMENT_POWER_BATTERYINFO_MAX_SIZE, 256);

    EXPECT_EQ(UAVCAN_PROTOCOL_NODESTATUS_MAX_SIZE, 7);
    EXPECT_EQ(UAVCAN_EQUIPMENT_GNSS_AUXILIARY_MAX_SIZE, 16);
    EXPECT_EQ(UAVCAN_EQUIPMENT_GNSS_FIX_MAX_SIZE, 79);
}

TEST(DroneCANConstants, DataTypeSignatures)
{
    // Verify 64-bit CRC signatures match UAVCAN specification
    EXPECT_EQ(UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE, 0xF0868D0C1A7C6F1ULL);
    EXPECT_EQ(UAVCAN_EQUIPMENT_GNSS_FIX2_SIGNATURE, 0xCA41E7000F37435FULL);
    EXPECT_EQ(UAVCAN_EQUIPMENT_GNSS_FIX_SIGNATURE, 0x54C1572B9E07F297ULL);
    EXPECT_EQ(UAVCAN_EQUIPMENT_GNSS_AUXILIARY_SIGNATURE, 0x9BE8BDC4C3DBBFD2ULL);
    EXPECT_EQ(UAVCAN_EQUIPMENT_POWER_BATTERYINFO_SIGNATURE, 0x249C26548A711966ULL);
}

TEST(DroneCANConstants, DataTypeIDs)
{
    EXPECT_EQ(UAVCAN_PROTOCOL_NODESTATUS_ID, 341);
    EXPECT_EQ(UAVCAN_EQUIPMENT_GNSS_FIX_ID, 1060);
    EXPECT_EQ(UAVCAN_EQUIPMENT_GNSS_AUXILIARY_ID, 1061);
    EXPECT_EQ(UAVCAN_EQUIPMENT_GNSS_FIX2_ID, 1063);
    EXPECT_EQ(UAVCAN_EQUIPMENT_POWER_BATTERYINFO_ID, 1092);
}
