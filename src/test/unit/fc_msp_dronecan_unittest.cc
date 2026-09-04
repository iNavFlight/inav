/**
 * fc_msp_dronecan unit tests
 *
 * Covers PR #11683 Qodo review Finding 2: MSP2_INAV_DRONECAN_ASYNC_REQUEST's
 * PARAM_GETSET write payload must be rejected, not silently zero-filled and
 * dispatched, when truncated for the declared is_write/value_type/name
 * lengths. This logic used to live inline in fc_msp.c's giant command
 * switch (untestable in isolation); it was extracted into
 * mspParseDronecanParamGetSetRequest() specifically so it could be unit
 * tested here.
 */

extern "C" {
#include <stdint.h>
#include <string.h>

#include "common/streambuf.h"
#include "drivers/dronecan/dronecan.h"
#include "fc/fc_msp_dronecan.h"
}

#include "gtest/gtest.h"

/* =========================================================================
 * Stubs — mspSerializeDronecanNodes()/mspHandleDronecanAsyncRequest()/
 * mspSerializeDronecanAsyncResult() (also compiled into this TU, since
 * they live in the same file as mspParseDronecanParamGetSetRequest())
 * reference real dronecan.c/drivers/time.c symbols. None of the tests
 * below exercise those three functions, so these are link-satisfying
 * stubs only, not behavioral fakes -- see dronecan_application_unittest.cc
 * for real DroneCAN application-layer test coverage.
 * ========================================================================= */
extern "C" {
uint32_t millis(void) { return 0; }
dronecanAsyncSlot_t dronecanAsyncSlot;
dronecanState_e dronecanGetState(void) { return STATE_DRONECAN_NORMAL; }
uint8_t dronecanGetNodeCount(void) { return 0; }
const dronecanNodeInfo_t *dronecanGetNode(uint8_t index) { (void)index; return nullptr; }
bool dronecanAsyncRequest(uint8_t service_id, uint8_t node_id, const void *payload)
{
    (void)service_id; (void)node_id; (void)payload;
    return false;
}
}

class MspDronecanParamGetSetTest : public ::testing::Test {
protected:
    uint8_t buf[64];
    sbuf_t sbuf;
    dronecanParamRequest_t req;

    void SetUp() override {
        memset(buf, 0, sizeof(buf));
        memset(&req, 0xAA, sizeof(req)); // non-zero canary; parser must fully own req on success
        sbufInit(&sbuf, buf, buf + sizeof(buf));
    }

    // Switches the write-mode sbuf used to build the payload into read mode.
    sbuf_t *reader() {
        sbufSwitchToReader(&sbuf, buf);
        return &sbuf;
    }
};

/* -------------------------------------------------------------------------
 * Underflow at the very start (index/is_write themselves truncated)
 * ---------------------------------------------------------------------- */

TEST_F(MspDronecanParamGetSetTest, TooShortForIndexAndIsWrite_Rejected)
{
    sbufWriteU8(&sbuf, 0); // only 1 byte total, need at least 3 (index u16 + is_write u8)
    EXPECT_FALSE(mspParseDronecanParamGetSetRequest(reader(), &req));
}

/* -------------------------------------------------------------------------
 * Read requests (is_write == 0) — value bytes are not expected at all
 * ---------------------------------------------------------------------- */

TEST_F(MspDronecanParamGetSetTest, ReadRequest_NoValueBytesNeeded_Accepted)
{
    sbufWriteU16(&sbuf, 3);  // index
    sbufWriteU8(&sbuf, 0);  // is_write = false
    ASSERT_TRUE(mspParseDronecanParamGetSetRequest(reader(), &req));
    EXPECT_EQ(req.index, 3);
    EXPECT_EQ(req.is_write, 0);
    EXPECT_EQ(req.req_name_len, 0);
}

/* -------------------------------------------------------------------------
 * INT writes
 * ---------------------------------------------------------------------- */

TEST_F(MspDronecanParamGetSetTest, IntWrite_Complete_Accepted)
{
    sbufWriteU16(&sbuf, 1);
    sbufWriteU8(&sbuf, 1); // is_write = true
    sbufWriteU8(&sbuf, DRONECAN_PARAM_TYPE_INT);
    uint64_t value = 123456789;
    sbufWriteData(&sbuf, &value, sizeof(value));

    ASSERT_TRUE(mspParseDronecanParamGetSetRequest(reader(), &req));
    EXPECT_EQ(req.value_type, DRONECAN_PARAM_TYPE_INT);
    EXPECT_EQ(req.value_int, 123456789);
    EXPECT_EQ(req.req_name_len, 0);
}

TEST_F(MspDronecanParamGetSetTest, IntWrite_Truncated_Rejected)
{
    sbufWriteU16(&sbuf, 1);
    sbufWriteU8(&sbuf, 1);
    sbufWriteU8(&sbuf, DRONECAN_PARAM_TYPE_INT);
    uint32_t half = 0xDEADBEEF; // only 4 of the required 8 value bytes
    sbufWriteData(&sbuf, &half, sizeof(half));

    EXPECT_FALSE(mspParseDronecanParamGetSetRequest(reader(), &req));
}

/* -------------------------------------------------------------------------
 * FLOAT writes
 * ---------------------------------------------------------------------- */

TEST_F(MspDronecanParamGetSetTest, FloatWrite_Complete_Accepted)
{
    sbufWriteU16(&sbuf, 2);
    sbufWriteU8(&sbuf, 1);
    sbufWriteU8(&sbuf, DRONECAN_PARAM_TYPE_FLOAT);
    sbufWriteU32(&sbuf, 0x3F800000); // 1.0f

    ASSERT_TRUE(mspParseDronecanParamGetSetRequest(reader(), &req));
    EXPECT_EQ(req.value_type, DRONECAN_PARAM_TYPE_FLOAT);
    EXPECT_FLOAT_EQ(req.value_float, 1.0f);
}

TEST_F(MspDronecanParamGetSetTest, FloatWrite_Truncated_Rejected)
{
    sbufWriteU16(&sbuf, 2);
    sbufWriteU8(&sbuf, 1);
    sbufWriteU8(&sbuf, DRONECAN_PARAM_TYPE_FLOAT);
    sbufWriteU8(&sbuf, 0); // only 1 of the required 4 value bytes

    EXPECT_FALSE(mspParseDronecanParamGetSetRequest(reader(), &req));
}

/* -------------------------------------------------------------------------
 * BOOL writes
 * ---------------------------------------------------------------------- */

TEST_F(MspDronecanParamGetSetTest, BoolWrite_Complete_Accepted)
{
    sbufWriteU16(&sbuf, 4);
    sbufWriteU8(&sbuf, 1);
    sbufWriteU8(&sbuf, DRONECAN_PARAM_TYPE_BOOL);
    sbufWriteU8(&sbuf, 1);

    ASSERT_TRUE(mspParseDronecanParamGetSetRequest(reader(), &req));
    EXPECT_EQ(req.value_type, DRONECAN_PARAM_TYPE_BOOL);
    EXPECT_EQ(req.value_bool, 1);
}

TEST_F(MspDronecanParamGetSetTest, BoolWrite_Truncated_Rejected)
{
    sbufWriteU16(&sbuf, 4);
    sbufWriteU8(&sbuf, 1);
    sbufWriteU8(&sbuf, DRONECAN_PARAM_TYPE_BOOL); // no value byte follows at all

    EXPECT_FALSE(mspParseDronecanParamGetSetRequest(reader(), &req));
}

/* -------------------------------------------------------------------------
 * STRING writes
 * ---------------------------------------------------------------------- */

TEST_F(MspDronecanParamGetSetTest, StringWrite_Complete_Accepted)
{
    sbufWriteU16(&sbuf, 5);
    sbufWriteU8(&sbuf, 1);
    sbufWriteU8(&sbuf, DRONECAN_PARAM_TYPE_STRING);
    const char *value = "hello";
    sbufWriteU8(&sbuf, (uint8_t)strlen(value));
    sbufWriteData(&sbuf, value, strlen(value));

    ASSERT_TRUE(mspParseDronecanParamGetSetRequest(reader(), &req));
    EXPECT_EQ(req.value_type, DRONECAN_PARAM_TYPE_STRING);
    EXPECT_EQ(req.value_str_len, strlen(value));
    EXPECT_EQ(0, memcmp(req.value_str, value, strlen(value)));
}

TEST_F(MspDronecanParamGetSetTest, StringWrite_DeclaredLengthExceedsPayload_Rejected)
{
    sbufWriteU16(&sbuf, 5);
    sbufWriteU8(&sbuf, 1);
    sbufWriteU8(&sbuf, DRONECAN_PARAM_TYPE_STRING);
    sbufWriteU8(&sbuf, 10); // declares 10 bytes of string data...
    sbufWriteData(&sbuf, "abc", 3); // ...but only 3 are actually present

    EXPECT_FALSE(mspParseDronecanParamGetSetRequest(reader(), &req));
}

/* -------------------------------------------------------------------------
 * value_type == EMPTY on a write is nonsensical
 * ---------------------------------------------------------------------- */

TEST_F(MspDronecanParamGetSetTest, EmptyTypeOnWrite_Rejected)
{
    sbufWriteU16(&sbuf, 6);
    sbufWriteU8(&sbuf, 1);
    sbufWriteU8(&sbuf, DRONECAN_PARAM_TYPE_EMPTY);

    EXPECT_FALSE(mspParseDronecanParamGetSetRequest(reader(), &req));
}

/* -------------------------------------------------------------------------
 * Trailing param-name field
 * ---------------------------------------------------------------------- */

TEST_F(MspDronecanParamGetSetTest, NameTruncated_Rejected)
{
    sbufWriteU16(&sbuf, 7);
    sbufWriteU8(&sbuf, 1);
    sbufWriteU8(&sbuf, DRONECAN_PARAM_TYPE_BOOL);
    sbufWriteU8(&sbuf, 1);   // valid bool value
    sbufWriteU8(&sbuf, 20);  // declares a 20-byte name...
    sbufWriteData(&sbuf, "short", 5); // ...but only 5 bytes follow

    EXPECT_FALSE(mspParseDronecanParamGetSetRequest(reader(), &req));
}

TEST_F(MspDronecanParamGetSetTest, NameComplete_Accepted)
{
    sbufWriteU16(&sbuf, 8);
    sbufWriteU8(&sbuf, 0); // read request, no value bytes
    const char *name = "MOT_SPIN_MIN";
    sbufWriteU8(&sbuf, (uint8_t)strlen(name));
    sbufWriteData(&sbuf, name, strlen(name));

    ASSERT_TRUE(mspParseDronecanParamGetSetRequest(reader(), &req));
    EXPECT_EQ(req.req_name_len, strlen(name));
    EXPECT_EQ(0, memcmp(req.req_name, name, strlen(name)));
}
