/**
 * bxCAN Timing Computation Unit Tests
 *
 * Regression tests for the timing algorithm in
 * canard_stm32f7xx_driver.c:canardSTM32ComputeTimings().
 *
 * The function is static and reads PCLK via HAL_RCC_GetPCLK1Freq(), so it
 * cannot be called directly from a unit test.  canardSTM32ComputeTimingsForPCLK()
 * below is an exact copy with pclk passed as a parameter instead.
 *
 * KEEP IN SYNC with canard_stm32f7xx_driver.c.
 *
 * Primary test PCLK: 54 MHz — STM32F765 APB1 at SYSCLK=216 MHz / APBprescaler=4.
 * Secondary test PCLK: 48 MHz — alternate F7 configuration.
 */

#include <cstdint>
#include <cstring>

#include "gtest/gtest.h"

// ---------------------------------------------------------------------------
// Mirror of canard_stm32f7xx_driver.c:canardSTM32ComputeTimings
// pclk replaces HAL_RCC_GetPCLK1Freq().  All other logic is identical.
// ---------------------------------------------------------------------------

struct Timings {
    uint16_t prescaler;
    uint8_t  sjw;
    uint8_t  bs1;
    uint8_t  bs2;
};

static bool canardSTM32ComputeTimingsForPCLK(const uint32_t pclk,
                                              const uint32_t target_bitrate,
                                              struct Timings *out_timings)
{
    if (target_bitrate < 1) {
        return false;
    }

    static const int MaxBS1 = 16;
    static const int MaxBS2 = 8;

    const int max_quanta_per_bit = 18;
    static const int MaxSamplePointLocation = 900;

    const uint32_t prescaler_bs = pclk / target_bitrate;

    uint8_t bs1_bs2_sum = (uint8_t)(max_quanta_per_bit - 1);

    while ((prescaler_bs % (1 + bs1_bs2_sum)) != 0) {
        if (bs1_bs2_sum <= 2) {
            return false;
        }
        bs1_bs2_sum--;
    }

    const uint32_t prescaler = prescaler_bs / (1 + bs1_bs2_sum);
    if ((prescaler < 1U) || (prescaler > 1024U)) {
        return false;
    }

    struct BsPair {
        uint8_t  bs1;
        uint8_t  bs2;
        uint16_t sample_point_permill;
    } solution;

    solution.bs1 = (uint8_t)(((7 * bs1_bs2_sum - 1) + 4) / 8);
    solution.bs2 = (uint8_t)(bs1_bs2_sum - solution.bs1);
    solution.sample_point_permill = (uint16_t)(1000 * (1 + solution.bs1) / (1 + solution.bs1 + solution.bs2));

    if (solution.sample_point_permill > MaxSamplePointLocation) {
        solution.bs1 = (uint8_t)((7 * bs1_bs2_sum - 1) / 8);
        solution.bs2 = (uint8_t)(bs1_bs2_sum - solution.bs1);
        solution.sample_point_permill = (uint16_t)(1000 * (1 + solution.bs1) / (1 + solution.bs1 + solution.bs2));
    }

    if ((target_bitrate != (pclk / (prescaler * (1 + solution.bs1 + solution.bs2)))) ||
        !((solution.bs1 >= 1) && (solution.bs1 <= MaxBS1) &&
          (solution.bs2 >= 1) && (solution.bs2 <= MaxBS2))) {
        return false;
    }

    out_timings->prescaler = (uint16_t)(prescaler);
    out_timings->sjw = 3;
    out_timings->bs1 = (uint8_t)(solution.bs1) - 1;  // HAL adds +1 internally
    out_timings->bs2 = (uint8_t)(solution.bs2) - 1;  // HAL adds +1 internally

    return true;
}

// ---------------------------------------------------------------------------
// Helper: back-calculate bitrate from HAL register values
// ---------------------------------------------------------------------------

static uint32_t bitrateFromTimings(uint32_t pclk, const struct Timings &t)
{
    // HAL bs1/bs2 values are stored with -1 offset; hardware adds +1 back
    uint32_t total_tq = 1u + (t.bs1 + 1u) + (t.bs2 + 1u);
    return pclk / (t.prescaler * total_tq);
}

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------

class BxCanTimingTest : public ::testing::Test {
protected:
    struct Timings t;

    void SetUp() override { memset(&t, 0, sizeof(t)); }
};

static const uint32_t PCLK_54M = 54000000U;
static const uint32_t PCLK_48M = 48000000U;

// ===========================================================================
// A. Bitrate correctness at PCLK = 54 MHz
//    All four standard bitrates should resolve to 18 quanta/bit.
// ===========================================================================

TEST_F(BxCanTimingTest, Pclk54_1Mbps_Succeeds)
{
    ASSERT_TRUE(canardSTM32ComputeTimingsForPCLK(PCLK_54M, 1000000, &t));
    EXPECT_EQ(bitrateFromTimings(PCLK_54M, t), 1000000U);
}

TEST_F(BxCanTimingTest, Pclk54_500kbps_Succeeds)
{
    ASSERT_TRUE(canardSTM32ComputeTimingsForPCLK(PCLK_54M, 500000, &t));
    EXPECT_EQ(bitrateFromTimings(PCLK_54M, t), 500000U);
}

TEST_F(BxCanTimingTest, Pclk54_250kbps_Succeeds)
{
    ASSERT_TRUE(canardSTM32ComputeTimingsForPCLK(PCLK_54M, 250000, &t));
    EXPECT_EQ(bitrateFromTimings(PCLK_54M, t), 250000U);
}

TEST_F(BxCanTimingTest, Pclk54_125kbps_Succeeds)
{
    ASSERT_TRUE(canardSTM32ComputeTimingsForPCLK(PCLK_54M, 125000, &t));
    EXPECT_EQ(bitrateFromTimings(PCLK_54M, t), 125000U);
}

// ===========================================================================
// B. 18-quanta regression — verifies max_quanta_per_bit=18 is in effect
//
//    At 54 MHz / 1 Mbps the solver must find 18 quanta/bit → prescaler=3.
//    The previous limit of 10 quanta would have yielded prescaler=6 (9 quanta).
// ===========================================================================

TEST_F(BxCanTimingTest, Pclk54_1Mbps_Uses18Quanta)
{
    ASSERT_TRUE(canardSTM32ComputeTimingsForPCLK(PCLK_54M, 1000000, &t));

    // 54 MHz / 3 / 18 quanta = 1 Mbps
    EXPECT_EQ(t.prescaler, 3u);

    // bs1 raw = t.bs1+1 = 15, bs2 raw = t.bs2+1 = 2 → 1+15+2 = 18 quanta/bit
    EXPECT_EQ(t.bs1 + 1u, 15u);
    EXPECT_EQ(t.bs2 + 1u,  2u);
}

TEST_F(BxCanTimingTest, Pclk54_500kbps_Uses18Quanta)
{
    ASSERT_TRUE(canardSTM32ComputeTimingsForPCLK(PCLK_54M, 500000, &t));
    EXPECT_EQ(t.prescaler, 6u);
    EXPECT_EQ(t.bs1 + 1u, 15u);
    EXPECT_EQ(t.bs2 + 1u,  2u);
}

TEST_F(BxCanTimingTest, Pclk54_250kbps_Uses18Quanta)
{
    ASSERT_TRUE(canardSTM32ComputeTimingsForPCLK(PCLK_54M, 250000, &t));
    EXPECT_EQ(t.prescaler, 12u);
    EXPECT_EQ(t.bs1 + 1u, 15u);
    EXPECT_EQ(t.bs2 + 1u,  2u);
}

TEST_F(BxCanTimingTest, Pclk54_125kbps_Uses18Quanta)
{
    ASSERT_TRUE(canardSTM32ComputeTimingsForPCLK(PCLK_54M, 125000, &t));
    EXPECT_EQ(t.prescaler, 24u);
    EXPECT_EQ(t.bs1 + 1u, 15u);
    EXPECT_EQ(t.bs2 + 1u,  2u);
}

// ===========================================================================
// C. Bitrate correctness at PCLK = 48 MHz (alternate F7 config)
//    48 MHz / bitrate is not divisible by 18 for standard rates,
//    so the solver falls back to 16 quanta/bit at prescaler=3,6,12,24.
// ===========================================================================

TEST_F(BxCanTimingTest, Pclk48_1Mbps_Succeeds)
{
    ASSERT_TRUE(canardSTM32ComputeTimingsForPCLK(PCLK_48M, 1000000, &t));
    EXPECT_EQ(bitrateFromTimings(PCLK_48M, t), 1000000U);
    // 48 MHz / 3 / 16 quanta = 1 Mbps
    EXPECT_EQ(t.prescaler, 3u);
    EXPECT_EQ(t.bs1 + 1u, 13u);
    EXPECT_EQ(t.bs2 + 1u,  2u);
}

TEST_F(BxCanTimingTest, Pclk48_500kbps_Succeeds)
{
    ASSERT_TRUE(canardSTM32ComputeTimingsForPCLK(PCLK_48M, 500000, &t));
    EXPECT_EQ(bitrateFromTimings(PCLK_48M, t), 500000U);
    EXPECT_EQ(t.prescaler, 6u);
}

TEST_F(BxCanTimingTest, Pclk48_250kbps_Succeeds)
{
    ASSERT_TRUE(canardSTM32ComputeTimingsForPCLK(PCLK_48M, 250000, &t));
    EXPECT_EQ(bitrateFromTimings(PCLK_48M, t), 250000U);
    EXPECT_EQ(t.prescaler, 12u);
}

TEST_F(BxCanTimingTest, Pclk48_125kbps_Succeeds)
{
    ASSERT_TRUE(canardSTM32ComputeTimingsForPCLK(PCLK_48M, 125000, &t));
    EXPECT_EQ(bitrateFromTimings(PCLK_48M, t), 125000U);
    EXPECT_EQ(t.prescaler, 24u);
}

// ===========================================================================
// D. Hardware constraint validation across all standard bitrates
// ===========================================================================

TEST_F(BxCanTimingTest, HwConstraints_AllStandardBitrates)
{
    const uint32_t bitrates[] = {125000, 250000, 500000, 1000000};
    const uint32_t pclks[]    = {PCLK_54M, PCLK_48M};

    for (uint32_t pclk : pclks) {
        for (uint32_t br : bitrates) {
            SCOPED_TRACE(testing::Message() << "pclk=" << pclk << " br=" << br);
            ASSERT_TRUE(canardSTM32ComputeTimingsForPCLK(pclk, br, &t));

            // BS1 raw (t.bs1+1) must be in [1..16]
            EXPECT_GE(t.bs1 + 1u, 1u);
            EXPECT_LE(t.bs1 + 1u, 16u);

            // BS2 raw (t.bs2+1) must be in [1..8]
            EXPECT_GE(t.bs2 + 1u, 1u);
            EXPECT_LE(t.bs2 + 1u, 8u);

            // Prescaler in [1..1024]
            EXPECT_GE(t.prescaler, 1u);
            EXPECT_LE(t.prescaler, 1024u);

            // SJW fixed at 3 (hardware SJW = 4 tq)
            EXPECT_EQ(t.sjw, 3u);

            // Back-calculated bitrate must match the request
            EXPECT_EQ(bitrateFromTimings(pclk, t), br);
        }
    }
}

TEST_F(BxCanTimingTest, SamplePoint_InValidRange)
{
    const uint32_t bitrates[] = {125000, 250000, 500000, 1000000};

    for (uint32_t br : bitrates) {
        SCOPED_TRACE(br);
        ASSERT_TRUE(canardSTM32ComputeTimingsForPCLK(PCLK_54M, br, &t));

        uint32_t bs1_raw = t.bs1 + 1u;
        uint32_t bs2_raw = t.bs2 + 1u;
        uint32_t total   = 1u + bs1_raw + bs2_raw;
        uint32_t sp_permill = 1000u * (1u + bs1_raw) / total;

        EXPECT_GE(sp_permill, 750u);  // practical CAN minimum
        EXPECT_LE(sp_permill, 900u);  // driver MaxSamplePointLocation
    }
}

// ===========================================================================
// E. Invalid and unsolvable inputs
// ===========================================================================

TEST_F(BxCanTimingTest, Invalid_ZeroBitrate)
{
    EXPECT_FALSE(canardSTM32ComputeTimingsForPCLK(PCLK_54M, 0, &t));
}

TEST_F(BxCanTimingTest, Invalid_UnsolvableBitrate)
{
    // 999999 bps: prescaler_bs=54 (integer division), but 54M/54=1M ≠ 999999
    // Final bitrate validation catches it.
    EXPECT_FALSE(canardSTM32ComputeTimingsForPCLK(PCLK_54M, 999999, &t));
}

TEST_F(BxCanTimingTest, Invalid_ExcessivelyLowBitrate)
{
    // prescaler_bs = 54M/100 = 540000 → prescaler = 540000/18 = 30000 > 1024
    EXPECT_FALSE(canardSTM32ComputeTimingsForPCLK(PCLK_54M, 100, &t));
}
