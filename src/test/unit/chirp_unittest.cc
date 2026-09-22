/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <cmath>
#include <cstdint>
#include <limits>
#include "gtest/gtest.h"

extern "C" {
#include "flight/chirp.h"
}

class ChirpTest : public testing::Test {
protected:
    chirpState_t state = {};
    uint32_t now = 1000;

    void update(bool low, bool high, uint16_t inhibit = 0, float amplitude = 10)
    {
        chirpUpdate(&state, now, true, low, high, inhibit, amplitude);
        now += 1000;
    }

    void start()
    {
        update(true, false);
        update(false, true);
        for (unsigned i = 0; i < CHIRP_SETTLE_US / 1000; ++i) {
            update(false, true);
        }
        ASSERT_EQ(state.phase, CHIRP_RUNNING);
    }
};

TEST_F(ChirpTest, HighAtBootCannotStart)
{
    for (int i = 0; i < 3000; ++i) {
        update(false, true);
        EXPECT_FLOAT_EQ(state.output, 0);
    }
    EXPECT_EQ(state.phase, CHIRP_ABORTED);
    EXPECT_EQ(state.inhibit, CHIRP_INHIBIT_SWITCH);
    start();
}

TEST_F(ChirpTest, DisabledIgnoresTriggerAndClearsOutput)
{
    start();
    chirpUpdate(&state, now, false, false, true, 0, 10);
    EXPECT_EQ(state.phase, CHIRP_IDLE);
    EXPECT_FLOAT_EQ(state.output, 0);
    update(false, true);
    EXPECT_EQ(state.phase, CHIRP_ABORTED);
}

TEST_F(ChirpTest, SweepIsBoundedFiniteMonotonicAndOneShot)
{
    start();
    float previousFrequency = 0;
    int zeroCrossings = 0;
    float previousOutput = 0;
    for (unsigned i = 0; i < CHIRP_DURATION_US / 1000; ++i) {
        update(false, true);
        ASSERT_TRUE(std::isfinite(state.output));
        EXPECT_LE(std::fabs(state.output), 10);
        EXPECT_GE(state.frequency, previousFrequency);
        EXPECT_LE(state.frequency, CHIRP_END_HZ);
        if (state.output > 0 && previousOutput <= 0) {
            ++zeroCrossings;
        }
        previousFrequency = state.frequency;
        previousOutput = state.output;
    }
    EXPECT_EQ(state.phase, CHIRP_DONE);
    EXPECT_FLOAT_EQ(state.output, 0);
    // Integral of 2 * exp(log(30) * t / 20) over 20 seconds is ~341 cycles.
    EXPECT_NEAR(zeroCrossings, 341, 2);
    for (int i = 0; i < 30000; ++i) {
        update(false, true);
        ASSERT_EQ(state.phase, CHIRP_DONE);
        ASSERT_FLOAT_EQ(state.output, 0);
    }
    start();
}

TEST_F(ChirpTest, EveryInhibitStopsExcitationAndRequiresNewLow)
{
    for (unsigned mask = 1; mask <= CHIRP_INHIBIT_SWITCH; mask <<= 1) {
        start();
        for (int i = 0; i < 1234; ++i) update(false, true);
        update(false, true, mask);
        ASSERT_EQ(state.phase, CHIRP_ABORTED);
        ASSERT_EQ(state.inhibit, mask);
        ASSERT_FLOAT_EQ(state.output, 0);
        for (int i = 0; i < 3000; ++i) {
            update(false, true);
            ASSERT_EQ(state.phase, CHIRP_ABORTED);
            ASSERT_FLOAT_EQ(state.output, 0);
        }
    }
}

TEST_F(ChirpTest, InvalidConditionsWhileSwitchLowDoNotArmTheTest)
{
    update(true, false, CHIRP_INHIBIT_FLIGHT);
    update(false, true);
    EXPECT_EQ(state.phase, CHIRP_ABORTED);
}

TEST_F(ChirpTest, SwitchLowAndMiddleStopImmediately)
{
    start();
    update(true, false);
    EXPECT_EQ(state.phase, CHIRP_READY);
    EXPECT_FLOAT_EQ(state.output, 0);
    start();
    update(false, false);
    EXPECT_EQ(state.phase, CHIRP_ABORTED);
    EXPECT_FLOAT_EQ(state.output, 0);
}

TEST_F(ChirpTest, TimingGapAbortsDuringSettlingAndDuringSweep)
{
    update(true, false);
    update(false, true);
    now += CHIRP_MAX_INTERVAL_US;
    update(false, true);
    EXPECT_EQ(state.inhibit, CHIRP_INHIBIT_TIMING);
    EXPECT_EQ(state.phase, CHIRP_ABORTED);
    start();
    now += CHIRP_MAX_INTERVAL_US;
    update(false, true);
    EXPECT_EQ(state.phase, CHIRP_ABORTED);
    EXPECT_FLOAT_EQ(state.output, 0);
}

TEST_F(ChirpTest, MicrosecondWrapDoesNotInterruptSweep)
{
    now = UINT32_MAX - CHIRP_SETTLE_US - 5000;
    start();
    for (int i = 0; i < 2000; ++i) update(false, true);
    EXPECT_EQ(state.phase, CHIRP_RUNNING);
    EXPECT_TRUE(std::isfinite(state.output));
}

TEST_F(ChirpTest, InvalidAmplitudeNeverExcites)
{
    for (float amplitude : {0.0f, -1.0f, 31.0f, std::numeric_limits<float>::infinity(),
                           std::numeric_limits<float>::quiet_NaN()}) {
        update(true, false, 0, amplitude);
        update(false, true, 0, amplitude);
        EXPECT_EQ(state.phase, CHIRP_ABORTED);
        EXPECT_EQ(state.inhibit, CHIRP_INHIBIT_CONFIG);
        EXPECT_FLOAT_EQ(state.output, 0);
    }
}

TEST_F(ChirpTest, FrequencyTracksElapsedTimeWithJitter)
{
    start();
    const uint32_t begin = state.startedAt;
    for (int i = 0; i < 4000; ++i) {
        now += (i % 2) ? 250 : 0;
        const uint32_t sampleTime = now;
        update(false, true);
        const double expected = 2 * std::exp(std::log(30.0) * (sampleTime - begin) / 20000000.0);
        ASSERT_NEAR(state.frequency, expected, 0.00002);
        ASSERT_EQ(state.phase, CHIRP_RUNNING);
    }
}
