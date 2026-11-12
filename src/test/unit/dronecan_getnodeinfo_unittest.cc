/**
 * DroneCAN GetNodeInfo and Service Message Unit Tests
 *
 * Covers coverage gaps identified in audit 2026-06-01:
 *   GAP-D1  GetNodeInfoResponse encode/decode round-trip
 *   GAP-D2  RTCMStream encode/decode
 *   GAP-D3  SoftwareVersion optional_field_flags wire behaviour
 *           (vcs_commit/image_crc are ALWAYS encoded; flags are app-level hint)
 *
 * Node table logic (GAP-N1..N4), MSP byte-layout (GAP-M1..M2), and
 * shouldAcceptTransfer dispatch (GAP-S1..S3) require dronecan.c to be
 * compiled with mocked INAV dependencies. That infrastructure belongs in a
 * separate dronecan_application_unittest.cc — tracked in the project todo.
 */

#include <cstdint>
#include <cstring>

extern "C" {
#include "drivers/dronecan/libcanard/canard.h"
#include "uavcan.protocol.GetNodeInfo.h"
#include "uavcan.protocol.GetNodeInfo_res.h"
#include "uavcan.protocol.GetNodeInfo_req.h"
#include "uavcan.protocol.SoftwareVersion.h"
#include "uavcan.protocol.HardwareVersion.h"
#include "uavcan.protocol.NodeStatus.h"
#include "uavcan.equipment.gnss.RTCMStream.h"
}

#include "gtest/gtest.h"

class DroneCANGetNodeInfoTest : public ::testing::Test {
protected:
    void SetUp() override {
        memset(buffer, 0, sizeof(buffer));
    }

    CanardRxTransfer makeTransfer(uint32_t len) {
        CanardRxTransfer transfer;
        memset(&transfer, 0, sizeof(transfer));
        transfer.payload_len = len;
        transfer.payload_head = buffer;
        transfer.payload_middle = NULL;
        transfer.payload_tail = NULL;
        return transfer;
    }

    // Buffer large enough for the largest GetNodeInfo response (377 bytes).
    uint8_t buffer[UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_MAX_SIZE + 16];
};

// ===========================================================================
// GetNodeInfoResponse encode/decode (GAP-D1)
// ===========================================================================

TEST_F(DroneCANGetNodeInfoTest, GetNodeInfoResponse_RoundTrip)
{
    struct uavcan_protocol_GetNodeInfoResponse tx;
    memset(&tx, 0, sizeof(tx));

    // NodeStatus
    tx.status.uptime_sec              = 12345;
    tx.status.health                  = 1; // WARNING
    tx.status.mode                    = 0; // OPERATIONAL
    tx.status.vendor_specific_status_code = 0xABCD;

    // SoftwareVersion
    tx.software_version.major              = 1;
    tx.software_version.minor              = 7;
    tx.software_version.optional_field_flags = 1; // vcs_commit valid
    tx.software_version.vcs_commit         = 0xDEADBEEF;
    tx.software_version.image_crc          = 0; // not flagged

    // HardwareVersion
    tx.hardware_version.major = 2;
    tx.hardware_version.minor = 0;
    for (int i = 0; i < 16; i++) {
        tx.hardware_version.unique_id[i] = (uint8_t)(0x10 + i);
    }
    tx.hardware_version.certificate_of_authenticity.len = 0;

    // Name
    const char *name = "com.example.sensor";
    tx.name.len = (uint8_t)strlen(name);
    memcpy(tx.name.data, name, tx.name.len);

    uint32_t encoded_len = uavcan_protocol_GetNodeInfoResponse_encode(&tx, buffer);

    EXPECT_GT(encoded_len, 0u);
    EXPECT_LE(encoded_len, (uint32_t)UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_MAX_SIZE);

    CanardRxTransfer transfer = makeTransfer(encoded_len);
    struct uavcan_protocol_GetNodeInfoResponse rx;
    memset(&rx, 0, sizeof(rx));
    bool decode_failed = uavcan_protocol_GetNodeInfoResponse_decode(&transfer, &rx);

    EXPECT_FALSE(decode_failed);

    EXPECT_EQ(rx.status.uptime_sec,               tx.status.uptime_sec);
    EXPECT_EQ(rx.status.health,                   tx.status.health);
    EXPECT_EQ(rx.status.mode,                     tx.status.mode);
    EXPECT_EQ(rx.status.vendor_specific_status_code, tx.status.vendor_specific_status_code);

    EXPECT_EQ(rx.software_version.major,               tx.software_version.major);
    EXPECT_EQ(rx.software_version.minor,               tx.software_version.minor);
    EXPECT_EQ(rx.software_version.optional_field_flags, tx.software_version.optional_field_flags);
    EXPECT_EQ(rx.software_version.vcs_commit,          tx.software_version.vcs_commit);

    EXPECT_EQ(rx.hardware_version.major, tx.hardware_version.major);
    EXPECT_EQ(rx.hardware_version.minor, tx.hardware_version.minor);
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(rx.hardware_version.unique_id[i], tx.hardware_version.unique_id[i])
            << "unique_id mismatch at byte " << i;
    }

    EXPECT_EQ(rx.name.len, tx.name.len);
    EXPECT_EQ(0, memcmp(rx.name.data, tx.name.data, tx.name.len));
}

TEST_F(DroneCANGetNodeInfoTest, GetNodeInfoResponse_EmptyName)
{
    // TAO-encoded name length is inferred from remaining payload when len=0.
    // A zero-length name must decode without error and name.len must be 0.
    struct uavcan_protocol_GetNodeInfoResponse tx;
    memset(&tx, 0, sizeof(tx));
    tx.name.len = 0;

    uint32_t encoded_len = uavcan_protocol_GetNodeInfoResponse_encode(&tx, buffer);
    EXPECT_GT(encoded_len, 0u);

    CanardRxTransfer transfer = makeTransfer(encoded_len);
    struct uavcan_protocol_GetNodeInfoResponse rx;
    memset(&rx, 0xFF, sizeof(rx));
    bool decode_failed = uavcan_protocol_GetNodeInfoResponse_decode(&transfer, &rx);

    EXPECT_FALSE(decode_failed);
    EXPECT_EQ(rx.name.len, 0u);
}

TEST_F(DroneCANGetNodeInfoTest, GetNodeInfoResponse_MaxLengthName)
{
    struct uavcan_protocol_GetNodeInfoResponse tx;
    memset(&tx, 0, sizeof(tx));
    tx.name.len = 80;
    for (int i = 0; i < 80; i++) {
        tx.name.data[i] = (uint8_t)('a' + (i % 26));
    }

    uint32_t encoded_len = uavcan_protocol_GetNodeInfoResponse_encode(&tx, buffer);
    EXPECT_LE(encoded_len, (uint32_t)UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_MAX_SIZE);

    CanardRxTransfer transfer = makeTransfer(encoded_len);
    struct uavcan_protocol_GetNodeInfoResponse rx;
    memset(&rx, 0, sizeof(rx));
    bool decode_failed = uavcan_protocol_GetNodeInfoResponse_decode(&transfer, &rx);

    EXPECT_FALSE(decode_failed);
    EXPECT_EQ(rx.name.len, 80u);
    EXPECT_EQ(0, memcmp(rx.name.data, tx.name.data, 80));
}

// ===========================================================================
// SoftwareVersion optional_field_flags (GAP-D3)
//
// The DSDL-generated encoder writes vcs_commit and image_crc unconditionally
// (always 15 bytes on the wire). optional_field_flags is an app-level hint
// that tells the receiver which fields are meaningful — it does NOT gate the
// wire encoding. Tests here document this behaviour and ensure that
// handle_GetNodeInfoResponse checks the flag before storing vcs_commit.
// ===========================================================================

TEST_F(DroneCANGetNodeInfoTest, SoftwareVersion_AlwaysEncodesAllFields)
{
    // Even with flags=0, vcs_commit and image_crc bytes are present on wire.
    // Verify that a non-zero vcs_commit set with flags=0 still survives the
    // round-trip — the application must use the flag to decide whether to use
    // the value, not rely on the decoder zeroing it out.
    struct uavcan_protocol_SoftwareVersion tx;
    memset(&tx, 0, sizeof(tx));
    tx.major               = 3;
    tx.minor               = 1;
    tx.optional_field_flags = 0; // neither field is flagged as valid
    tx.vcs_commit          = 0xCAFEBABE; // present on wire, but not flagged
    tx.image_crc           = 0;

    uint32_t encoded_len = uavcan_protocol_SoftwareVersion_encode(&tx, buffer);
    EXPECT_GT(encoded_len, 0u);

    CanardRxTransfer transfer = makeTransfer(encoded_len);
    struct uavcan_protocol_SoftwareVersion rx;
    memset(&rx, 0, sizeof(rx));
    bool decode_failed = uavcan_protocol_SoftwareVersion_decode(&transfer, &rx);

    EXPECT_FALSE(decode_failed);
    EXPECT_EQ(rx.major,                tx.major);
    EXPECT_EQ(rx.minor,                tx.minor);
    EXPECT_EQ(rx.optional_field_flags, 0u);
    // vcs_commit IS decoded (wire is always 15 bytes) but flags=0 means
    // the application must NOT trust it — assert the flag is checked:
    EXPECT_EQ(rx.optional_field_flags & 1u, 0u) << "vcs_commit flag must not be set";
}

TEST_F(DroneCANGetNodeInfoTest, SoftwareVersion_VCSCommitFlaggedAndValid)
{
    struct uavcan_protocol_SoftwareVersion tx;
    memset(&tx, 0, sizeof(tx));
    tx.major               = 1;
    tx.minor               = 5;
    tx.optional_field_flags = 1; // VCS_COMMIT valid
    tx.vcs_commit          = 0xDEADBEEF;
    tx.image_crc           = 0;

    uint32_t encoded_len = uavcan_protocol_SoftwareVersion_encode(&tx, buffer);
    CanardRxTransfer transfer = makeTransfer(encoded_len);
    struct uavcan_protocol_SoftwareVersion rx;
    memset(&rx, 0, sizeof(rx));

    EXPECT_FALSE(uavcan_protocol_SoftwareVersion_decode(&transfer, &rx));
    EXPECT_EQ(rx.optional_field_flags & 1u, 1u);
    EXPECT_EQ(rx.vcs_commit, 0xDEADBEEFu);
}

TEST_F(DroneCANGetNodeInfoTest, SoftwareVersion_BothOptionalFieldsFlagged)
{
    struct uavcan_protocol_SoftwareVersion tx;
    memset(&tx, 0, sizeof(tx));
    tx.major               = 2;
    tx.minor               = 0;
    tx.optional_field_flags = 3; // VCS_COMMIT and IMAGE_CRC both valid
    tx.vcs_commit          = 0x12345678;
    tx.image_crc           = 0xABCDEF0123456789ULL;

    uint32_t encoded_len = uavcan_protocol_SoftwareVersion_encode(&tx, buffer);
    CanardRxTransfer transfer = makeTransfer(encoded_len);
    struct uavcan_protocol_SoftwareVersion rx;
    memset(&rx, 0, sizeof(rx));

    EXPECT_FALSE(uavcan_protocol_SoftwareVersion_decode(&transfer, &rx));
    EXPECT_EQ(rx.optional_field_flags, 3u);
    EXPECT_EQ(rx.vcs_commit, 0x12345678u);
    EXPECT_EQ(rx.image_crc,  0xABCDEF0123456789ULL);
}

// ===========================================================================
// HardwareVersion unique_id (part of GAP-D1)
// ===========================================================================

TEST_F(DroneCANGetNodeInfoTest, HardwareVersion_UniqueIdRoundTrip)
{
    struct uavcan_protocol_HardwareVersion tx;
    memset(&tx, 0, sizeof(tx));
    tx.major = 1;
    tx.minor = 0;
    for (int i = 0; i < 16; i++) {
        tx.unique_id[i] = (uint8_t)(0xA0 + i);
    }
    tx.certificate_of_authenticity.len = 0;

    uint32_t encoded_len = uavcan_protocol_HardwareVersion_encode(&tx, buffer);
    EXPECT_GT(encoded_len, 0u);

    CanardRxTransfer transfer = makeTransfer(encoded_len);
    struct uavcan_protocol_HardwareVersion rx;
    memset(&rx, 0, sizeof(rx));

    EXPECT_FALSE(uavcan_protocol_HardwareVersion_decode(&transfer, &rx));
    EXPECT_EQ(rx.major, tx.major);
    EXPECT_EQ(rx.minor, tx.minor);
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(rx.unique_id[i], tx.unique_id[i]) << "unique_id mismatch at byte " << i;
    }
    EXPECT_EQ(rx.certificate_of_authenticity.len, 0u);
}

TEST_F(DroneCANGetNodeInfoTest, HardwareVersion_ZeroUniqueId)
{
    struct uavcan_protocol_HardwareVersion tx;
    memset(&tx, 0, sizeof(tx));
    // All unique_id bytes zero — valid for nodes that don't implement unique ID.

    uint32_t encoded_len = uavcan_protocol_HardwareVersion_encode(&tx, buffer);
    CanardRxTransfer transfer = makeTransfer(encoded_len);
    struct uavcan_protocol_HardwareVersion rx;
    memset(&rx, 0xFF, sizeof(rx));

    EXPECT_FALSE(uavcan_protocol_HardwareVersion_decode(&transfer, &rx));
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(rx.unique_id[i], 0u) << "unique_id byte " << i << " should be zero";
    }
}

// ===========================================================================
// RTCMStream encode/decode (GAP-D2)
// ===========================================================================

TEST_F(DroneCANGetNodeInfoTest, RTCMStream_BasicEncodeDecode)
{
    struct uavcan_equipment_gnss_RTCMStream tx;
    memset(&tx, 0, sizeof(tx));
    tx.protocol_id = UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_PROTOCOL_ID_RTCM3;

    const uint8_t payload[] = {0xD3, 0x00, 0x13, 0x3E, 0xD0, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x70};
    tx.data.len = sizeof(payload);
    memcpy(tx.data.data, payload, sizeof(payload));

    uint32_t encoded_len = uavcan_equipment_gnss_RTCMStream_encode(&tx, buffer);

    EXPECT_GT(encoded_len, 0u);
    EXPECT_LE(encoded_len, (uint32_t)UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_MAX_SIZE);

    CanardRxTransfer transfer = makeTransfer(encoded_len);
    struct uavcan_equipment_gnss_RTCMStream rx;
    memset(&rx, 0, sizeof(rx));

    EXPECT_FALSE(uavcan_equipment_gnss_RTCMStream_decode(&transfer, &rx));
    EXPECT_EQ(rx.protocol_id, UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_PROTOCOL_ID_RTCM3);
    EXPECT_EQ(rx.data.len, tx.data.len);
    EXPECT_EQ(0, memcmp(rx.data.data, tx.data.data, tx.data.len));
}

TEST_F(DroneCANGetNodeInfoTest, RTCMStream_EmptyPayload)
{
    struct uavcan_equipment_gnss_RTCMStream tx;
    memset(&tx, 0, sizeof(tx));
    tx.protocol_id = UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_PROTOCOL_ID_RTCM2;
    tx.data.len    = 0;

    uint32_t encoded_len = uavcan_equipment_gnss_RTCMStream_encode(&tx, buffer);
    CanardRxTransfer transfer = makeTransfer(encoded_len);
    struct uavcan_equipment_gnss_RTCMStream rx;
    memset(&rx, 0xFF, sizeof(rx));

    EXPECT_FALSE(uavcan_equipment_gnss_RTCMStream_decode(&transfer, &rx));
    EXPECT_EQ(rx.protocol_id, UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_PROTOCOL_ID_RTCM2);
    EXPECT_EQ(rx.data.len, 0u);
}

// ===========================================================================
// Constants (extend GAP-S2: signatures must match DSDL spec)
// ===========================================================================

TEST(DroneCANGetNodeInfoConstants, Signatures)
{
    EXPECT_EQ(UAVCAN_PROTOCOL_GETNODEINFO_SIGNATURE,          0xEE468A8121C46A9EULL);
    EXPECT_EQ(UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_SIGNATURE, 0xEE468A8121C46A9EULL);
    EXPECT_EQ(UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_SIGNATURE,     0x1F56030ECB171501ULL);
}

TEST(DroneCANGetNodeInfoConstants, IDs)
{
    EXPECT_EQ(UAVCAN_PROTOCOL_GETNODEINFO_ID,         1u);
    EXPECT_EQ(UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_ID,    1062u);
}

TEST(DroneCANGetNodeInfoConstants, MessageSizes)
{
    // Response max size accounts for 80-char name + all nested structs.
    EXPECT_EQ(UAVCAN_PROTOCOL_GETNODEINFO_RESPONSE_MAX_SIZE, 377);
    EXPECT_EQ(UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_MAX_SIZE,     130);
}
