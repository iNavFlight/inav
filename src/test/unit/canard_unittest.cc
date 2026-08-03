/**
 * Libcanard Core Unit Tests
 *
 * Tests for the libcanard library internals: memory pool, CRC, float16,
 * transfer ID logic, TX/RX frame processing.
 *
 * Uses CANARD_INTERNAL= (empty) to expose static functions for testing.
 */

#include <cstdint>
#include <cstring>
#include <cmath>
#include <climits>

// CANARD_INTERNAL is defined as empty via CMake definitions,
// making all internal functions non-static and accessible for testing
extern "C" {
#include "drivers/dronecan/libcanard/canard.h"
#include "drivers/dronecan/libcanard/canard_internals.h"
}

#include "gtest/gtest.h"

// ===========================================================================
// Test Fixture
// ===========================================================================

// Memory pool size for tests (enough for several transfers)
static const size_t POOL_SIZE = 4096;

class CanardTest : public ::testing::Test {
protected:
    void SetUp() override {
        memset(pool_memory, 0, sizeof(pool_memory));
        canardInit(&ins, pool_memory, sizeof(pool_memory),
                   onTransferReceived, shouldAcceptTransfer, this);
        received_transfer_count = 0;
        last_received_data_type_id = 0;
        last_received_source_node_id = 0;
        last_received_transfer_type = 0;
        memset(last_received_payload, 0, sizeof(last_received_payload));
        last_received_payload_len = 0;
    }

    void TearDown() override {}

    // Callback: accept all broadcasts
    static bool shouldAcceptTransfer(const CanardInstance* ins,
                                     uint64_t* out_data_type_signature,
                                     uint16_t data_type_id,
                                     CanardTransferType transfer_type,
                                     uint8_t source_node_id) {
        (void)ins;
        (void)data_type_id;
        (void)transfer_type;
        (void)source_node_id;
        *out_data_type_signature = 0x1234567890ABCDEFULL;
        return true;
    }

    // Callback: record received transfer
    static void onTransferReceived(CanardInstance* ins,
                                   CanardRxTransfer* transfer) {
        CanardTest* self = (CanardTest*)canardGetUserReference(ins);
        self->received_transfer_count++;
        self->last_received_data_type_id = transfer->data_type_id;
        self->last_received_source_node_id = transfer->source_node_id;
        self->last_received_transfer_type = transfer->transfer_type;
        if (transfer->payload_len > 0 && transfer->payload_head != NULL) {
            uint16_t copy_len = transfer->payload_len;
            if (copy_len > sizeof(self->last_received_payload))
                copy_len = sizeof(self->last_received_payload);
            memcpy(self->last_received_payload, transfer->payload_head, copy_len);
            self->last_received_payload_len = copy_len;
        }
    }

    // Helper: drain all TX queue frames
    int drainTxQueue() {
        int count = 0;
        while (canardPeekTxQueue(&ins) != NULL) {
            canardPopTxQueue(&ins);
            count++;
        }
        return count;
    }

    CanardInstance ins;
    uint8_t pool_memory[POOL_SIZE] __attribute__((aligned(CANARD_MEM_BLOCK_SIZE)));

    int received_transfer_count;
    uint16_t last_received_data_type_id;
    uint8_t last_received_source_node_id;
    uint8_t last_received_transfer_type;
    uint8_t last_received_payload[64];
    uint16_t last_received_payload_len;
};

// Fixture that rejects all transfers
class CanardRejectTest : public ::testing::Test {
protected:
    void SetUp() override {
        memset(pool_memory, 0, sizeof(pool_memory));
        canardInit(&ins, pool_memory, sizeof(pool_memory),
                   onTransferReceived, shouldReject, NULL);
    }

    static bool shouldReject(const CanardInstance* ins,
                             uint64_t* out_data_type_signature,
                             uint16_t data_type_id,
                             CanardTransferType transfer_type,
                             uint8_t source_node_id) {
        (void)ins; (void)out_data_type_signature; (void)data_type_id;
        (void)transfer_type; (void)source_node_id;
        return false;
    }

    static void onTransferReceived(CanardInstance* ins,
                                   CanardRxTransfer* transfer) {
        (void)ins; (void)transfer;
    }

    CanardInstance ins;
    uint8_t pool_memory[POOL_SIZE] __attribute__((aligned(CANARD_MEM_BLOCK_SIZE)));
};


// ===========================================================================
// A. Init & Node ID Tests
// ===========================================================================

TEST_F(CanardTest, Init_BasicState)
{
    EXPECT_EQ(canardGetLocalNodeID(&ins), CANARD_BROADCAST_NODE_ID);
    EXPECT_EQ(ins.tx_queue, nullptr);
    EXPECT_EQ(ins.rx_states, nullptr);
}

TEST_F(CanardTest, Init_UserReference)
{
    EXPECT_EQ(canardGetUserReference(&ins), this);
}

TEST_F(CanardTest, NodeID_SetAndGet)
{
    canardSetLocalNodeID(&ins, 42);
    EXPECT_EQ(canardGetLocalNodeID(&ins), 42);
}

TEST_F(CanardTest, NodeID_BoundaryValues)
{
    // Test minimum valid node ID
    CanardInstance ins2;
    uint8_t pool2[POOL_SIZE] __attribute__((aligned(CANARD_MEM_BLOCK_SIZE)));
    canardInit(&ins2, pool2, sizeof(pool2), onTransferReceived, shouldAcceptTransfer, NULL);

    canardSetLocalNodeID(&ins2, CANARD_MIN_NODE_ID);
    EXPECT_EQ(canardGetLocalNodeID(&ins2), 1);

    // Test maximum valid node ID
    CanardInstance ins3;
    uint8_t pool3[POOL_SIZE] __attribute__((aligned(CANARD_MEM_BLOCK_SIZE)));
    canardInit(&ins3, pool3, sizeof(pool3), onTransferReceived, shouldAcceptTransfer, NULL);

    canardSetLocalNodeID(&ins3, CANARD_MAX_NODE_ID);
    EXPECT_EQ(canardGetLocalNodeID(&ins3), 127);
}

TEST_F(CanardTest, NodeID_ForgetAndReassign)
{
    canardSetLocalNodeID(&ins, 42);
    EXPECT_EQ(canardGetLocalNodeID(&ins), 42);

    canardForgetLocalNodeID(&ins);
    EXPECT_EQ(canardGetLocalNodeID(&ins), CANARD_BROADCAST_NODE_ID);

    canardSetLocalNodeID(&ins, 100);
    EXPECT_EQ(canardGetLocalNodeID(&ins), 100);
}

// ===========================================================================
// B. Memory Pool Tests
// ===========================================================================

TEST_F(CanardTest, MemPool_AllocFree)
{
    CanardPoolAllocatorStatistics stats = canardGetPoolAllocatorStatistics(&ins);
    EXPECT_EQ(stats.current_usage_blocks, 0);
    EXPECT_GT(stats.capacity_blocks, 0);

    void* block = allocateBlock(&ins.allocator);
    ASSERT_NE(block, nullptr);

    stats = canardGetPoolAllocatorStatistics(&ins);
    EXPECT_EQ(stats.current_usage_blocks, 1);
    EXPECT_EQ(stats.peak_usage_blocks, 1);

    freeBlock(&ins.allocator, block);
    stats = canardGetPoolAllocatorStatistics(&ins);
    EXPECT_EQ(stats.current_usage_blocks, 0);
    EXPECT_EQ(stats.peak_usage_blocks, 1); // Peak stays
}

TEST_F(CanardTest, MemPool_Exhaustion)
{
    CanardPoolAllocatorStatistics stats = canardGetPoolAllocatorStatistics(&ins);
    const uint16_t capacity = stats.capacity_blocks;

    // Allocate all blocks
    void* blocks[256];
    uint16_t allocated = 0;
    for (uint16_t i = 0; i < capacity && i < 256; i++) {
        blocks[i] = allocateBlock(&ins.allocator);
        if (blocks[i] == NULL) break;
        allocated++;
    }
    EXPECT_EQ(allocated, capacity);

    // Next allocation should fail
    void* overflow = allocateBlock(&ins.allocator);
    EXPECT_EQ(overflow, nullptr);

    stats = canardGetPoolAllocatorStatistics(&ins);
    EXPECT_EQ(stats.current_usage_blocks, capacity);
    EXPECT_EQ(stats.peak_usage_blocks, capacity);

    // Free all
    for (uint16_t i = 0; i < allocated; i++) {
        freeBlock(&ins.allocator, blocks[i]);
    }
    stats = canardGetPoolAllocatorStatistics(&ins);
    EXPECT_EQ(stats.current_usage_blocks, 0);
}

TEST_F(CanardTest, MemPool_PeakTracking)
{
    void* b1 = allocateBlock(&ins.allocator);
    void* b2 = allocateBlock(&ins.allocator);
    void* b3 = allocateBlock(&ins.allocator);

    CanardPoolAllocatorStatistics stats = canardGetPoolAllocatorStatistics(&ins);
    EXPECT_EQ(stats.peak_usage_blocks, 3);

    freeBlock(&ins.allocator, b1);
    freeBlock(&ins.allocator, b2);

    stats = canardGetPoolAllocatorStatistics(&ins);
    EXPECT_EQ(stats.current_usage_blocks, 1);
    EXPECT_EQ(stats.peak_usage_blocks, 3); // Peak unchanged

    void* b4 = allocateBlock(&ins.allocator);
    stats = canardGetPoolAllocatorStatistics(&ins);
    EXPECT_EQ(stats.current_usage_blocks, 2);
    EXPECT_EQ(stats.peak_usage_blocks, 3); // Still 3

    freeBlock(&ins.allocator, b3);
    freeBlock(&ins.allocator, b4);
}

// ===========================================================================
// C. CRC Tests
// ===========================================================================

TEST_F(CanardTest, CRC_KnownValues)
{
    // CRC-16/CCITT (init 0xFFFF, poly 0x1021)
    uint16_t crc = 0xFFFF;

    // Single byte: CRC of "A" (0x41)
    crc = crcAddByte(0xFFFF, 0x41);
    // Verify it produces a non-trivial value
    EXPECT_NE(crc, 0xFFFF);
    EXPECT_NE(crc, 0x0000);
}

TEST_F(CanardTest, CRC_AddString)
{
    // CRC of "123456789" is a well-known test vector for CRC-16/CCITT
    const uint8_t test_data[] = "123456789";
    uint16_t crc = crcAdd(0xFFFF, test_data, 9);
    // CRC-16/CCITT-FALSE of "123456789" = 0x29B1
    EXPECT_EQ(crc, 0x29B1);
}

TEST_F(CanardTest, CRC_SignatureProcessing)
{
    // Verify signature is processed byte-by-byte (8 bytes from a uint64)
    uint64_t sig = 0x0102030405060708ULL;
    uint16_t crc = crcAddSignature(0xFFFF, sig);
    EXPECT_NE(crc, 0xFFFF);

    // Manually compute the same thing
    uint16_t manual_crc = 0xFFFF;
    for (int i = 0; i < 8; i++) {
        manual_crc = crcAddByte(manual_crc, (uint8_t)(sig >> (i * 8)));
    }
    EXPECT_EQ(crc, manual_crc);
}

// ===========================================================================
// D. Float16 Conversion Tests
// ===========================================================================

TEST_F(CanardTest, Float16_SpecialValues)
{
    // Zero
    uint16_t f16 = canardConvertNativeFloatToFloat16(0.0f);
    float result = canardConvertFloat16ToNativeFloat(f16);
    EXPECT_FLOAT_EQ(result, 0.0f);

    // One
    f16 = canardConvertNativeFloatToFloat16(1.0f);
    result = canardConvertFloat16ToNativeFloat(f16);
    EXPECT_FLOAT_EQ(result, 1.0f);

    // Negative one
    f16 = canardConvertNativeFloatToFloat16(-1.0f);
    result = canardConvertFloat16ToNativeFloat(f16);
    EXPECT_FLOAT_EQ(result, -1.0f);

    // Infinity
    f16 = canardConvertNativeFloatToFloat16(INFINITY);
    result = canardConvertFloat16ToNativeFloat(f16);
    EXPECT_TRUE(std::isinf(result));
    EXPECT_GT(result, 0);

    // Negative infinity
    f16 = canardConvertNativeFloatToFloat16(-INFINITY);
    result = canardConvertFloat16ToNativeFloat(f16);
    EXPECT_TRUE(std::isinf(result));
    EXPECT_LT(result, 0);

    // NaN
    f16 = canardConvertNativeFloatToFloat16(NAN);
    result = canardConvertFloat16ToNativeFloat(f16);
    EXPECT_TRUE(std::isnan(result));
}

TEST_F(CanardTest, Float16_BatteryVoltageRange)
{
    // Test float16 precision for typical battery voltage range (3.0V - 60V)
    float test_voltages[] = {3.0f, 3.7f, 4.2f, 12.6f, 16.8f, 25.2f, 50.4f};

    for (float v : test_voltages) {
        uint16_t f16 = canardConvertNativeFloatToFloat16(v);
        float result = canardConvertFloat16ToNativeFloat(f16);
        // Float16 has ~3 significant digits, so relative error < 0.1%
        EXPECT_NEAR(result, v, v * 0.002f) << "Failed for voltage " << v;
    }
}

// ===========================================================================
// E. Internal Functions Tests
// ===========================================================================

TEST_F(CanardTest, TransferIDForwardDistance)
{
    // Same ID => distance 0
    EXPECT_EQ(computeTransferIDForwardDistance(5, 5), 0);

    // Forward by 1
    EXPECT_EQ(computeTransferIDForwardDistance(6, 5), 1);

    // Forward by 10
    EXPECT_EQ(computeTransferIDForwardDistance(15, 5), 10);

    // Wrap-around: from 30 to 2 (distance = 30-2 = 28)
    EXPECT_EQ(computeTransferIDForwardDistance(30, 2), 28);

    // Wrap-around: from 2 to 30 (distance = 2-30+32 = 4)
    EXPECT_EQ(computeTransferIDForwardDistance(2, 30), 4);

    // From 0 to 31 (distance = 0-31+32 = 1)
    EXPECT_EQ(computeTransferIDForwardDistance(0, 31), 1);
}

TEST_F(CanardTest, IncrementTransferID)
{
    uint8_t tid = 0;
    incrementTransferID(&tid);
    EXPECT_EQ(tid, 1);

    tid = 30;
    incrementTransferID(&tid);
    EXPECT_EQ(tid, 31);

    // Wrap at 32
    tid = 31;
    incrementTransferID(&tid);
    EXPECT_EQ(tid, 0);
}

TEST_F(CanardTest, IsPriorityHigher)
{
    // Lower CAN ID = higher priority
    uint32_t high_prio = 0x100 | CANARD_CAN_FRAME_EFF;
    uint32_t low_prio = 0x200 | CANARD_CAN_FRAME_EFF;

    // isPriorityHigher(rhs, id) returns true if rhs has higher priority than id
    // i.e., rhs < id in CAN arbitration
    EXPECT_TRUE(isPriorityHigher(low_prio, high_prio));   // high_prio wins over low_prio
    EXPECT_FALSE(isPriorityHigher(high_prio, low_prio));  // low_prio does not win over high_prio

    // Same priority
    EXPECT_FALSE(isPriorityHigher(high_prio, high_prio));
}

TEST_F(CanardTest, DlcConversion)
{
    // Standard CAN: DLC 0-8 map directly
    for (uint16_t dlc = 0; dlc <= 8; dlc++) {
        EXPECT_EQ(dlcToDataLength(dlc), dlc);
    }

    // CAN FD DLC mappings
    EXPECT_EQ(dlcToDataLength(9), 12u);
    EXPECT_EQ(dlcToDataLength(10), 16u);
    EXPECT_EQ(dlcToDataLength(11), 20u);
    EXPECT_EQ(dlcToDataLength(12), 24u);
    EXPECT_EQ(dlcToDataLength(13), 32u);
    EXPECT_EQ(dlcToDataLength(14), 48u);
    EXPECT_EQ(dlcToDataLength(15), 64u);

    // Reverse: data length to DLC
    for (uint16_t dl = 0; dl <= 8; dl++) {
        EXPECT_EQ(dataLengthToDlc(dl), dl);
    }
    EXPECT_EQ(dataLengthToDlc(12), 9u);
    EXPECT_EQ(dataLengthToDlc(16), 10u);
    EXPECT_EQ(dataLengthToDlc(20), 11u);
    EXPECT_EQ(dataLengthToDlc(24), 12u);
    EXPECT_EQ(dataLengthToDlc(32), 13u);
    EXPECT_EQ(dataLengthToDlc(48), 14u);
    EXPECT_EQ(dataLengthToDlc(64), 15u);

    // In-between values round up
    EXPECT_EQ(dataLengthToDlc(9), 9u);   // 9 bytes -> DLC 9 (12 bytes)
    EXPECT_EQ(dataLengthToDlc(13), 10u);  // 13 bytes -> DLC 10 (16 bytes)
    EXPECT_EQ(dataLengthToDlc(33), 14u);  // 33 bytes -> DLC 14 (48 bytes)
}

// ===========================================================================
// F. Single-Frame TX Tests
// ===========================================================================

TEST_F(CanardTest, TX_SingleFrameBroadcast)
{
    canardSetLocalNodeID(&ins, 42);

    uint8_t payload[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t transfer_id = 0;

    int16_t result = canardBroadcast(&ins,
                                     0x1234567890ABCDEFULL,  // signature
                                     1000,                    // data type ID
                                     &transfer_id,
                                     CANARD_TRANSFER_PRIORITY_MEDIUM,
                                     payload,
                                     sizeof(payload));

    EXPECT_EQ(result, 1); // 1 frame enqueued

    // Verify the frame
    CanardCANFrame* frame = canardPeekTxQueue(&ins);
    ASSERT_NE(frame, nullptr);

    // Check extended frame flag
    EXPECT_NE(frame->id & CANARD_CAN_FRAME_EFF, 0u);

    // Source node ID should be in bits [0:6]
    EXPECT_EQ(frame->id & 0x7F, 42u);

    // Data type ID should be in bits [8:23]
    EXPECT_EQ((frame->id >> 8) & 0xFFFF, 1000u);

    // Payload should be present
    EXPECT_GT(frame->data_len, 0u);

    // First bytes should be the payload
    EXPECT_EQ(frame->data[0], 0x01);
    EXPECT_EQ(frame->data[1], 0x02);
    EXPECT_EQ(frame->data[2], 0x03);
    EXPECT_EQ(frame->data[3], 0x04);

    // Tail byte: for single frame, bits 7=SOT(1), 6=EOT(1), 5=toggle(0), [4:0]=transfer_id
    uint8_t tail_byte = frame->data[frame->data_len - 1];
    EXPECT_EQ(tail_byte & 0xC0, 0xC0);  // SOT + EOT
    EXPECT_EQ(tail_byte & 0x1F, 0);     // transfer_id = 0

    // Transfer ID should have been incremented
    EXPECT_EQ(transfer_id, 1);

    canardPopTxQueue(&ins);
}

TEST_F(CanardTest, TX_TransferIDWrap)
{
    canardSetLocalNodeID(&ins, 42);

    uint8_t payload[] = {0xAA};
    uint8_t transfer_id = 30;

    // Send with tid=30
    int16_t result = canardBroadcast(&ins, 0x1234ULL, 100, &transfer_id,
                                     CANARD_TRANSFER_PRIORITY_MEDIUM,
                                     payload, sizeof(payload));
    EXPECT_EQ(result, 1);
    EXPECT_EQ(transfer_id, 31); // 30 -> 31
    drainTxQueue();

    // Send with tid=31
    result = canardBroadcast(&ins, 0x1234ULL, 100, &transfer_id,
                             CANARD_TRANSFER_PRIORITY_MEDIUM,
                             payload, sizeof(payload));
    EXPECT_EQ(result, 1);
    EXPECT_EQ(transfer_id, 0); // 31 -> 0 (wrap)
    drainTxQueue();
}

TEST_F(CanardTest, TX_InvalidArguments)
{
    canardSetLocalNodeID(&ins, 42);

    // NULL payload with non-zero length
    uint8_t transfer_id = 0;
    int16_t result = canardBroadcast(&ins, 0x1234ULL, 100, &transfer_id,
                                     CANARD_TRANSFER_PRIORITY_MEDIUM,
                                     NULL, 5);
    EXPECT_EQ(result, -CANARD_ERROR_INVALID_ARGUMENT);

    // Invalid priority (> 31)
    uint8_t payload[] = {0x01};
    result = canardBroadcast(&ins, 0x1234ULL, 100, &transfer_id,
                             32, // invalid priority
                             payload, sizeof(payload));
    EXPECT_EQ(result, -CANARD_ERROR_INVALID_ARGUMENT);
}

// ===========================================================================
// G. RX Frame Processing Tests
// ===========================================================================

TEST_F(CanardTest, RX_SingleFrameBroadcast)
{
    canardSetLocalNodeID(&ins, 42);

    // Construct a single-frame broadcast from node 10, data type 1000
    CanardCANFrame frame;
    memset(&frame, 0, sizeof(frame));

    // CAN ID for broadcast: priority(24-28) | data_type_id(8-23) | source_node_id(0-6)
    frame.id = ((uint32_t)CANARD_TRANSFER_PRIORITY_MEDIUM << 24) |
               ((uint32_t)1000 << 8) |
               (uint32_t)10 |
               CANARD_CAN_FRAME_EFF;

    // Payload: 3 data bytes + tail byte
    frame.data[0] = 0xDE;
    frame.data[1] = 0xAD;
    frame.data[2] = 0xBE;
    frame.data[3] = 0xC0 | 0; // SOT=1, EOT=1, toggle=0, tid=0
    frame.data_len = 4;
    frame.iface_id = 0;

    int16_t result = canardHandleRxFrame(&ins, &frame, 1000000);
    EXPECT_EQ(result, CANARD_OK);
    EXPECT_EQ(received_transfer_count, 1);
    EXPECT_EQ(last_received_data_type_id, 1000);
    EXPECT_EQ(last_received_source_node_id, 10);
    EXPECT_EQ(last_received_transfer_type, (uint8_t)CanardTransferTypeBroadcast);

    // Payload should be the 3 data bytes (not the tail byte)
    EXPECT_EQ(last_received_payload[0], 0xDE);
    EXPECT_EQ(last_received_payload[1], 0xAD);
    EXPECT_EQ(last_received_payload[2], 0xBE);
}

TEST_F(CanardRejectTest, RX_RejectUnwanted)
{
    canardSetLocalNodeID(&ins, 42);

    CanardCANFrame frame;
    memset(&frame, 0, sizeof(frame));
    frame.id = ((uint32_t)CANARD_TRANSFER_PRIORITY_MEDIUM << 24) |
               ((uint32_t)1000 << 8) |
               (uint32_t)10 |
               CANARD_CAN_FRAME_EFF;
    frame.data[0] = 0xAA;
    frame.data[1] = 0xC0; // SOT=1, EOT=1, toggle=0, tid=0
    frame.data_len = 2;
    frame.iface_id = 0;

    int16_t result = canardHandleRxFrame(&ins, &frame, 1000000);
    EXPECT_EQ(result, -CANARD_ERROR_RX_NOT_WANTED);
}

TEST_F(CanardTest, RX_RejectNonExtended)
{
    canardSetLocalNodeID(&ins, 42);

    CanardCANFrame frame;
    memset(&frame, 0, sizeof(frame));
    // No CANARD_CAN_FRAME_EFF flag = standard frame
    frame.id = 0x100;
    frame.data[0] = 0xC0;
    frame.data_len = 1;

    int16_t result = canardHandleRxFrame(&ins, &frame, 1000000);
    EXPECT_EQ(result, -CANARD_ERROR_RX_INCOMPATIBLE_PACKET);
}

TEST_F(CanardTest, RX_RejectRTR)
{
    canardSetLocalNodeID(&ins, 42);

    CanardCANFrame frame;
    memset(&frame, 0, sizeof(frame));
    frame.id = CANARD_CAN_FRAME_EFF | CANARD_CAN_FRAME_RTR | 0x100;
    frame.data[0] = 0xC0;
    frame.data_len = 1;

    int16_t result = canardHandleRxFrame(&ins, &frame, 1000000);
    EXPECT_EQ(result, -CANARD_ERROR_RX_INCOMPATIBLE_PACKET);
}

TEST_F(CanardTest, RX_RejectZeroLength)
{
    canardSetLocalNodeID(&ins, 42);

    CanardCANFrame frame;
    memset(&frame, 0, sizeof(frame));
    frame.id = CANARD_CAN_FRAME_EFF | 0x100;
    frame.data_len = 0;

    int16_t result = canardHandleRxFrame(&ins, &frame, 1000000);
    EXPECT_EQ(result, -CANARD_ERROR_RX_INCOMPATIBLE_PACKET);
}

// ===========================================================================
// H. Multi-Frame TX Tests
// ===========================================================================

TEST_F(CanardTest, TX_MultiFrameBroadcast)
{
    canardSetLocalNodeID(&ins, 42);

    // 8 bytes of payload requires multi-frame (since 1 byte is tail byte,
    // single frame max payload = 7 bytes)
    uint8_t payload[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint8_t transfer_id = 0;

    int16_t result = canardBroadcast(&ins,
                                     0x1234567890ABCDEFULL,
                                     1000,
                                     &transfer_id,
                                     CANARD_TRANSFER_PRIORITY_MEDIUM,
                                     payload,
                                     sizeof(payload));

    EXPECT_GT(result, 1); // Multiple frames

    // First frame should have SOT=1
    CanardCANFrame* frame = canardPeekTxQueue(&ins);
    ASSERT_NE(frame, nullptr);
    uint8_t tail_byte = frame->data[frame->data_len - 1];
    EXPECT_TRUE((tail_byte & 0x80) != 0); // SOT set
    EXPECT_FALSE((tail_byte & 0x40) != 0); // EOT not set on first frame
    canardPopTxQueue(&ins);

    // Drain middle frames, verify last frame has EOT
    uint8_t last_tail = 0;
    while (canardPeekTxQueue(&ins) != NULL) {
        frame = canardPeekTxQueue(&ins);
        last_tail = frame->data[frame->data_len - 1];
        canardPopTxQueue(&ins);
    }
    EXPECT_TRUE((last_tail & 0x40) != 0); // EOT set on last frame
    EXPECT_FALSE((last_tail & 0x80) != 0); // SOT not set on last frame

    EXPECT_EQ(transfer_id, 1);
}

TEST_F(CanardTest, TX_MultiFramePoolExhaustion)
{
    // Create an instance with a very small pool
    CanardInstance tiny_ins;
    // 2 blocks = not enough for multi-frame
    uint8_t tiny_pool[CANARD_MEM_BLOCK_SIZE * 2] __attribute__((aligned(CANARD_MEM_BLOCK_SIZE)));
    canardInit(&tiny_ins, tiny_pool, sizeof(tiny_pool),
               onTransferReceived, shouldAcceptTransfer, NULL);
    canardSetLocalNodeID(&tiny_ins, 42);

    // Large payload that would need many frames
    uint8_t payload[64];
    memset(payload, 0xAA, sizeof(payload));
    uint8_t transfer_id = 0;

    int16_t result = canardBroadcast(&tiny_ins,
                                     0x1234567890ABCDEFULL,
                                     1000,
                                     &transfer_id,
                                     CANARD_TRANSFER_PRIORITY_MEDIUM,
                                     payload,
                                     sizeof(payload));

    EXPECT_EQ(result, -CANARD_ERROR_OUT_OF_MEMORY);
}

// ===========================================================================
// I. Extract Data Type / Transfer Type Tests
// ===========================================================================

TEST_F(CanardTest, ExtractDataType_Broadcast)
{
    // Broadcast CAN ID: priority(24-28) | data_type_id(8-23) | source_node_id(0-6)
    uint32_t can_id = ((uint32_t)CANARD_TRANSFER_PRIORITY_MEDIUM << 24) |
                      ((uint32_t)1063 << 8) |
                      42;

    EXPECT_EQ(extractTransferType(can_id), CanardTransferTypeBroadcast);
    EXPECT_EQ(extractDataType(can_id), 1063);
}

TEST_F(CanardTest, ExtractDataType_Service)
{
    // Service CAN ID: priority(24-28) | service_type(16-23) | req_not_resp(15) |
    //                 dest_node_id(8-14) | service_not_msg(7)=1 | source_node_id(0-6)
    uint32_t can_id = ((uint32_t)CANARD_TRANSFER_PRIORITY_MEDIUM << 24) |
                      ((uint32_t)11 << 16) |   // service type = 11
                      ((uint32_t)1 << 15) |    // request
                      ((uint32_t)42 << 8) |    // dest node
                      ((uint32_t)1 << 7) |     // service flag
                      10;                       // source node

    EXPECT_EQ(extractTransferType(can_id), CanardTransferTypeRequest);
    EXPECT_EQ(extractDataType(can_id), 11);
}

// ===========================================================================
// J. Pool Allocator Statistics Tests
// ===========================================================================

TEST_F(CanardTest, PoolStats_AfterBroadcast)
{
    canardSetLocalNodeID(&ins, 42);

    CanardPoolAllocatorStatistics stats_before = canardGetPoolAllocatorStatistics(&ins);
    EXPECT_EQ(stats_before.current_usage_blocks, 0);

    uint8_t payload[] = {0x01, 0x02, 0x03};
    uint8_t transfer_id = 0;
    canardBroadcast(&ins, 0x1234ULL, 100, &transfer_id,
                    CANARD_TRANSFER_PRIORITY_MEDIUM,
                    payload, sizeof(payload));

    CanardPoolAllocatorStatistics stats_after = canardGetPoolAllocatorStatistics(&ins);
    EXPECT_GT(stats_after.current_usage_blocks, 0u); // Frame queued

    drainTxQueue();

    CanardPoolAllocatorStatistics stats_final = canardGetPoolAllocatorStatistics(&ins);
    EXPECT_EQ(stats_final.current_usage_blocks, 0); // All freed
}
