/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <string.h>

#include <algorithm>
#include <vector>

extern "C" {
    #include "platform.h"

    #include "common/streambuf.h"
    #include "common/utils.h"

    #include "fc/fc_msp.h"

    #include "msp/msp.h"

    #include "telemetry/msp_shared.h"
}

#include "gtest/gtest.h"

#define SPORT_PAYLOAD_SIZE  6   // SMARTPORT_MSP_PAYLOAD_SIZE, private to smartport.c
#define CRSF_PAYLOAD_SIZE   58  // CRSF_FRAME_TX_MSP_FRAME_SIZE

enum {
    STATUS_SEQ_MASK = 0x0f,
    STATUS_START    = 0x10,
    STATUS_V1       = 1 << 5,
    STATUS_V2       = 2 << 5,
};

enum {
    CMD_LONG_REPLY  = 10,       // 20 byte reply, needs several frames
    CMD_SHORT_REPLY = 20,       // 2 byte reply, fits into one frame
    CMD_EXACT_REPLY = 30,       // 3 byte reply, fills the first SmartPort frame exactly
    CMD_V2_LONG     = 0x2010,   // 100 byte reply, needs several CRSF frames
    CMD_V2_SHORT    = 0x2020,   // 2 byte reply
};

static std::vector<std::vector<uint8_t>> sentFrames;

extern "C" {
    mspResult_e mspFcProcessCommand(mspPacket_t *cmd, mspPacket_t *reply, mspPostProcessFnPtr *mspPostProcessFn)
    {
        UNUSED(mspPostProcessFn);

        int length = 0;
        switch (cmd->cmd) {
            case CMD_LONG_REPLY: length = 20; break;
            case CMD_SHORT_REPLY: length = 2; break;
            case CMD_EXACT_REPLY: length = 3; break;
            case CMD_V2_LONG: length = 100; break;
            case CMD_V2_SHORT: length = 2; break;
            default: break;
        }

        reply->cmd = cmd->cmd;
        for (int i = 0; i < length; i++) {
            sbufWriteU8(&reply->buf, (uint8_t)(cmd->cmd + i));
        }
        return MSP_RESULT_ACK;
    }
}

static void captureFrame(uint8_t *payload, const uint8_t payloadSize)
{
    sentFrames.emplace_back(payload, payload + payloadSize);
}

static bool sendV1Request(uint8_t cmd)
{
    uint8_t frame[SPORT_PAYLOAD_SIZE] = { STATUS_START | STATUS_V1, 0, cmd, 0, 0, 0 };
    return handleMspFrame(frame, sizeof(frame));
}

static bool sendV2Request(uint16_t cmd)
{
    uint8_t frame[SPORT_PAYLOAD_SIZE] = { STATUS_START | STATUS_V2, 0, (uint8_t)(cmd & 0xff), (uint8_t)(cmd >> 8), 0, 0 };
    return handleMspFrame(frame, sizeof(frame));
}

class MspSharedTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        initSharedMsp();
        sentFrames.clear();
    }
};

TEST_F(MspSharedTest, ReplyAfterDiscardedV1ReplyStartsWithHeader)
{
    ASSERT_TRUE(sendV1Request(CMD_LONG_REPLY));
    ASSERT_TRUE(sendMspReply(SPORT_PAYLOAD_SIZE, captureFrame));
    ASSERT_TRUE(sentFrames.back()[0] & STATUS_START);

    // a new request while the long reply is still being sent discards that reply
    ASSERT_TRUE(sendV1Request(CMD_SHORT_REPLY));
    EXPECT_FALSE(sendMspReply(SPORT_PAYLOAD_SIZE, captureFrame));

    const std::vector<uint8_t> expected = { STATUS_START | STATUS_V1, 2, CMD_SHORT_REPLY, CMD_SHORT_REPLY, CMD_SHORT_REPLY + 1 };
    const std::vector<uint8_t> &frame = sentFrames.back();
    ASSERT_EQ(expected.size(), frame.size());
    EXPECT_EQ(expected[0], frame[0] & ~STATUS_SEQ_MASK);
    EXPECT_TRUE(std::equal(expected.begin() + 1, expected.end(), frame.begin() + 1));
}

TEST_F(MspSharedTest, ReplyAfterDiscardedV2ReplyStartsWithHeader)
{
    ASSERT_TRUE(sendV2Request(CMD_V2_LONG));
    ASSERT_TRUE(sendMspReply(CRSF_PAYLOAD_SIZE, captureFrame));

    ASSERT_TRUE(sendV2Request(CMD_V2_SHORT));
    EXPECT_FALSE(sendMspReply(CRSF_PAYLOAD_SIZE, captureFrame));

    const std::vector<uint8_t> expected = { STATUS_START | STATUS_V2, 0, CMD_V2_SHORT & 0xff, CMD_V2_SHORT >> 8, 2, 0,
                                            CMD_V2_SHORT & 0xff, (CMD_V2_SHORT + 1) & 0xff };
    const std::vector<uint8_t> &frame = sentFrames.back();
    ASSERT_EQ(expected.size(), frame.size());
    EXPECT_EQ(expected[0], frame[0] & ~STATUS_SEQ_MASK);
    EXPECT_TRUE(std::equal(expected.begin() + 1, expected.end(), frame.begin() + 1));
}

TEST_F(MspSharedTest, ReplyAfterReplyDiscardedByStrayFrameStartsWithHeader)
{
    ASSERT_TRUE(sendV1Request(CMD_LONG_REPLY));
    ASSERT_TRUE(sendMspReply(SPORT_PAYLOAD_SIZE, captureFrame));

    uint8_t continuation[SPORT_PAYLOAD_SIZE] = { STATUS_V1 | 5, 0, 0, 0, 0, 0 };
    EXPECT_FALSE(handleMspFrame(continuation, sizeof(continuation)));

    ASSERT_TRUE(sendV1Request(CMD_SHORT_REPLY));
    EXPECT_FALSE(sendMspReply(SPORT_PAYLOAD_SIZE, captureFrame));
    EXPECT_TRUE(sentFrames.back()[0] & STATUS_START);
    EXPECT_EQ(CMD_SHORT_REPLY, sentFrames.back()[2]);
}

TEST_F(MspSharedTest, ExactlyFilledFrameIsFollowedByStatusOnlyFrame)
{
    ASSERT_TRUE(sendV1Request(CMD_EXACT_REPLY));
    EXPECT_TRUE(sendMspReply(SPORT_PAYLOAD_SIZE, captureFrame));
    ASSERT_EQ(1u, sentFrames.size());
    EXPECT_EQ((size_t)SPORT_PAYLOAD_SIZE, sentFrames[0].size());
    EXPECT_EQ(3, sentFrames[0][1]);

    // clients derived from the Betaflight Lua MSP code complete such a reply only on this frame
    EXPECT_FALSE(sendMspReply(SPORT_PAYLOAD_SIZE, captureFrame));
    ASSERT_EQ(2u, sentFrames.size());
    ASSERT_EQ(1u, sentFrames[1].size());
    EXPECT_FALSE(sentFrames[1][0] & STATUS_START);
    EXPECT_EQ((sentFrames[0][0] + 1) & STATUS_SEQ_MASK, sentFrames[1][0] & STATUS_SEQ_MASK);
}
