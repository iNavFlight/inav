/**
 * CAN Bit-Timing Solver Unit Tests
 *
 * Regression tests for canardComputeCanTimingSolution() in
 * canard_stm32_timing.c — the HAL-free timing core shared by both the
 * STM32 bxCAN (F7) and FDCAN (H7) DroneCAN drivers. Each driver applies its
 * own peripheral-specific SJW and BS1/BS2 register-offset convention on top
 * of this solution; see canardSTM32ComputeTimings() in
 * canard_stm32f7xx_driver.c and canard_stm32h7xx_driver.c.
 *
 * This test links the real production source directly (see CMakeLists.txt),
 * so there is no separate copy of the algorithm to keep in sync.
 *
 * Primary test PCLK: 54 MHz — STM32F765 APB1 at SYSCLK=216 MHz / APBprescaler=4.
 * Secondary test PCLK: 48 MHz — alternate F7 configuration.
 */

#include <cstdint>
#include <cstring>

#include "gtest/gtest.h"

extern "C" {
#include "drivers/dronecan/libcanard/canard_stm32_timing.h"
}

// ---------------------------------------------------------------------------
// Helper: back-calculate bitrate from a solved timing solution
// ---------------------------------------------------------------------------

static uint32_t bitrateFromSolution(uint32_t pclk, const CanardCanTimingSolution &s)
{
    uint32_t total_tq = 1u + s.bs1 + s.bs2;
    return pclk / (s.prescaler * total_tq);
}

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------

class BxCanTimingTest : public ::testing::Test {
protected:
    CanardCanTimingSolution sol;

    void SetUp() override { memset(&sol, 0, sizeof(sol)); }
};

static const uint32_t PCLK_54M = 54000000U;
static const uint32_t PCLK_48M = 48000000U;

// ===========================================================================
// A. Bitrate correctness at PCLK = 54 MHz
// ===========================================================================

TEST_F(BxCanTimingTest, Pclk54_1Mbps_Succeeds)
{
    ASSERT_TRUE(canardComputeCanTimingSolution(PCLK_54M, 1000000, &sol));
    EXPECT_EQ(bitrateFromSolution(PCLK_54M, sol), 1000000U);
}

TEST_F(BxCanTimingTest, Pclk54_500kbps_Succeeds)
{
    ASSERT_TRUE(canardComputeCanTimingSolution(PCLK_54M, 500000, &sol));
    EXPECT_EQ(bitrateFromSolution(PCLK_54M, sol), 500000U);
}

TEST_F(BxCanTimingTest, Pclk54_250kbps_Succeeds)
{
    ASSERT_TRUE(canardComputeCanTimingSolution(PCLK_54M, 250000, &sol));
    EXPECT_EQ(bitrateFromSolution(PCLK_54M, sol), 250000U);
}

TEST_F(BxCanTimingTest, Pclk54_125kbps_Succeeds)
{
    ASSERT_TRUE(canardComputeCanTimingSolution(PCLK_54M, 125000, &sol));
    EXPECT_EQ(bitrateFromSolution(PCLK_54M, sol), 125000U);
}

// ===========================================================================
// B. Quanta-cap regression — verifies the dual max_quanta_per_bit is in effect
//
//    Driver caps at 10 quanta/bit for >= 1 Mbps, 17 quanta/bit below that.
//    At 54 MHz / 1 Mbps the 10-cap yields 9 quanta/bit (prescaler=6) — not
//    the 18 quanta/bit (prescaler=3) an unconditional cap would give.
//    500/250 kbps land on 12 quanta/bit; 125 kbps lands on 16 quanta/bit —
//    all below the 17 cap, since 54 MHz isn't evenly divisible at 17 quanta
//    for these rates.
// ===========================================================================

TEST_F(BxCanTimingTest, Pclk54_1Mbps_Uses9Quanta)
{
    ASSERT_TRUE(canardComputeCanTimingSolution(PCLK_54M, 1000000, &sol));

    // 54 MHz / 6 / 9 quanta = 1 Mbps
    EXPECT_EQ(sol.prescaler, 6u);
    EXPECT_EQ(sol.bs1, 7u);
    EXPECT_EQ(sol.bs2, 1u);
}

TEST_F(BxCanTimingTest, Pclk54_500kbps_Uses12Quanta)
{
    ASSERT_TRUE(canardComputeCanTimingSolution(PCLK_54M, 500000, &sol));
    EXPECT_EQ(sol.prescaler, 9u);
    EXPECT_EQ(sol.bs1, 9u);
    EXPECT_EQ(sol.bs2, 2u);
}

TEST_F(BxCanTimingTest, Pclk54_250kbps_Uses12Quanta)
{
    ASSERT_TRUE(canardComputeCanTimingSolution(PCLK_54M, 250000, &sol));
    EXPECT_EQ(sol.prescaler, 18u);
    EXPECT_EQ(sol.bs1, 9u);
    EXPECT_EQ(sol.bs2, 2u);
}

TEST_F(BxCanTimingTest, Pclk54_125kbps_Uses16Quanta)
{
    ASSERT_TRUE(canardComputeCanTimingSolution(PCLK_54M, 125000, &sol));
    EXPECT_EQ(sol.prescaler, 27u);
    EXPECT_EQ(sol.bs1, 13u);
    EXPECT_EQ(sol.bs2, 2u);
}

// ===========================================================================
// C. Bitrate correctness at PCLK = 48 MHz (alternate F7 config)
//    At 1 Mbps the 10-quanta cap resolves to 8 quanta/bit (prescaler=6).
//    48 MHz / bitrate is not divisible by 17 quanta for 500/250/125 kbps,
//    so those fall back to 16 quanta/bit at prescaler=6,12,24.
// ===========================================================================

TEST_F(BxCanTimingTest, Pclk48_1Mbps_Succeeds)
{
    ASSERT_TRUE(canardComputeCanTimingSolution(PCLK_48M, 1000000, &sol));
    EXPECT_EQ(bitrateFromSolution(PCLK_48M, sol), 1000000U);
    // 48 MHz / 6 / 8 quanta = 1 Mbps
    EXPECT_EQ(sol.prescaler, 6u);
    EXPECT_EQ(sol.bs1, 6u);
    EXPECT_EQ(sol.bs2, 1u);
}

TEST_F(BxCanTimingTest, Pclk48_500kbps_Succeeds)
{
    ASSERT_TRUE(canardComputeCanTimingSolution(PCLK_48M, 500000, &sol));
    EXPECT_EQ(bitrateFromSolution(PCLK_48M, sol), 500000U);
    EXPECT_EQ(sol.prescaler, 6u);
}

TEST_F(BxCanTimingTest, Pclk48_250kbps_Succeeds)
{
    ASSERT_TRUE(canardComputeCanTimingSolution(PCLK_48M, 250000, &sol));
    EXPECT_EQ(bitrateFromSolution(PCLK_48M, sol), 250000U);
    EXPECT_EQ(sol.prescaler, 12u);
}

TEST_F(BxCanTimingTest, Pclk48_125kbps_Succeeds)
{
    ASSERT_TRUE(canardComputeCanTimingSolution(PCLK_48M, 125000, &sol));
    EXPECT_EQ(bitrateFromSolution(PCLK_48M, sol), 125000U);
    EXPECT_EQ(sol.prescaler, 24u);
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
            ASSERT_TRUE(canardComputeCanTimingSolution(pclk, br, &sol));

            // BS1 must be in [1..16]
            EXPECT_GE(sol.bs1, 1u);
            EXPECT_LE(sol.bs1, 16u);

            // BS2 must be in [1..8]
            EXPECT_GE(sol.bs2, 1u);
            EXPECT_LE(sol.bs2, 8u);

            // Prescaler in [1..1024]
            EXPECT_GE(sol.prescaler, 1u);
            EXPECT_LE(sol.prescaler, 1024u);

            // Back-calculated bitrate must match the request
            EXPECT_EQ(bitrateFromSolution(pclk, sol), br);
        }
    }
}

TEST_F(BxCanTimingTest, SamplePoint_InValidRange)
{
    const uint32_t bitrates[] = {125000, 250000, 500000, 1000000};

    for (uint32_t br : bitrates) {
        SCOPED_TRACE(br);
        ASSERT_TRUE(canardComputeCanTimingSolution(PCLK_54M, br, &sol));

        uint32_t total   = 1u + sol.bs1 + sol.bs2;
        uint32_t sp_permill = 1000u * (1u + sol.bs1) / total;

        EXPECT_GE(sp_permill, 750u);  // practical CAN minimum
        EXPECT_LE(sp_permill, 900u);  // driver MaxSamplePointLocation
    }
}

// ===========================================================================
// E. Invalid and unsolvable inputs
// ===========================================================================

TEST_F(BxCanTimingTest, Invalid_ZeroBitrate)
{
    EXPECT_FALSE(canardComputeCanTimingSolution(PCLK_54M, 0, &sol));
}

TEST_F(BxCanTimingTest, Invalid_UnsolvableBitrate)
{
    // 999999 bps: the solver finds a valid quanta/prescaler split, but the
    // resulting bitrate (54 MHz / 6 / 9 quanta = 1000000) doesn't match the
    // requested 999999 — final bitrate validation catches it.
    EXPECT_FALSE(canardComputeCanTimingSolution(PCLK_54M, 999999, &sol));
}

TEST_F(BxCanTimingTest, Invalid_ExcessivelyLowBitrate)
{
    // prescaler_bs = 54M/100 = 540000 → even at max quanta the required
    // prescaler exceeds the 1024 hardware limit.
    EXPECT_FALSE(canardComputeCanTimingSolution(PCLK_54M, 100, &sol));
}
