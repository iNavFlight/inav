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
#include <math.h>

extern "C" {
    #include "common/maths.h"
    #include "sensors/compass_learn.h"
}

#include "unittest_macros.h"
#include "gtest/gtest.h"

#include <functional>

namespace {

const float kInclination = 58.0f * M_PIf / 180.0f;
const int16_t kTrueZero[3] = {120, -310, 75};

struct Sim {
    float radius = 450.0f;                  // field magnitude in raw counts
    float axisScale[3] = {1.0f, 1.0f, 1.0f};
    int16_t storedZero[3];
    int16_t gain[3];
    std::function<void(int, float[3])> interference;
    uint32_t seed = 12345;
    int sample = 0;

    explicit Sim(int16_t ex = 60, int16_t ey = -40, int16_t ez = 80)
    {
        storedZero[0] = kTrueZero[0] + ex;
        storedZero[1] = kTrueZero[1] + ey;
        storedZero[2] = kTrueZero[2] + ez;
        setGainFromScale();
    }

    void setGainFromScale(float wrong = 1.0f)
    {
        for (int i = 0; i < 3; i++) {
            gain[i] = lrintf(radius * axisScale[i] * wrong);
        }
    }

    float noise()
    {
        seed = seed * 1103515245u + 12345u;
        return ((seed >> 16) & 0x7FFF) / 32767.0f * 6.0f - 3.0f;
    }

    // Earth field (NED) seen by the body at roll phi, pitch theta, yaw psi; the sensor frame is the body frame
    void push(float phi, float theta, float psi)
    {
        const float b[3] = {radius * cosf(kInclination), 0.0f, radius * sinf(kInclination)};
        const float cf = cosf(phi), sf = sinf(phi), ct = cosf(theta), st = sinf(theta), cp = cosf(psi), sp = sinf(psi);
        const float m[3][3] = {
            {ct * cp, ct * sp, -st},
            {sf * st * cp - cf * sp, sf * st * sp + cf * cp, sf * ct},
            {cf * st * cp + sf * sp, cf * st * sp - sf * cp, cf * ct},
        };
        float extra[3] = {0.0f, 0.0f, 0.0f};
        if (interference) {
            interference(sample, extra);
        }
        sample++;

        int16_t raw[3];
        float body[3];
        for (int i = 0; i < 3; i++) {
            const float field = m[i][0] * b[0] + m[i][1] * b[1] + m[i][2] * b[2];
            raw[i] = lrintf(axisScale[i] * field + kTrueZero[i] + extra[i] + noise());
            body[i] = (raw[i] - storedZero[i]) * 1024.0f / gain[i];
        }
        magLearnAddSample(body, raw);
    }

    // Two full turns, one each way, with straight legs between them
    void flyCircuits(float bankDeg)
    {
        const float bank = bankDeg * M_PIf / 180.0f;
        for (int i = 0; i < 40; i++) {
            push(0.0f, 0.05f, 0.0f);
        }
        for (int deg = 0; deg < 360; deg += 3) {
            push(bank, 0.05f, deg * M_PIf / 180.0f);
        }
        for (int i = 0; i < 40; i++) {
            push(0.0f, 0.05f, 0.0f);
        }
        for (int deg = 360; deg > 0; deg -= 3) {
            push(-bank, 0.05f, deg * M_PIf / 180.0f);
        }
    }

    void evaluate()
    {
        magLearnEvaluate(storedZero, gain);
    }

    // Each flight starts from what the previous one saved, as at disarm
    void flyFlightsAndSave(int flights)
    {
        for (int flight = 0; flight < flights; flight++) {
            magLearnReset();
            flyCircuits(30);
            evaluate();
            ASSERT_TRUE(magLearnStatus.flags & MAG_LEARN_SAVE_DUE);
            for (int axis = 0; axis < 3; axis++) {
                storedZero[axis] += magLearnStatus.delta[axis];
            }
        }
    }

    int error(int axis) const
    {
        return storedZero[axis] + magLearnStatus.delta[axis] - kTrueZero[axis];
    }
};

class CompassLearnTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        magLearnReset();
    }
};

TEST_F(CompassLearnTest, ResetClearsTheStatus)
{
    Sim sim;
    sim.flyCircuits(30);
    sim.evaluate();
    magLearnReset();

    EXPECT_EQ(0, magLearnStatus.flags);
    EXPECT_EQ(0, magLearnStatus.sectors);
    EXPECT_EQ(0, magLearnStatus.headings);
    EXPECT_EQ(0, magLearnStatus.spread);
    for (int axis = 0; axis < 3; axis++) {
        EXPECT_EQ(0, magLearnStatus.delta[axis]);
    }
}

TEST_F(CompassLearnTest, TurnsRecoverTheHorizontalOffset)
{
    Sim sim;
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_EQ(MAG_LEARN_SAVE_DUE, magLearnStatus.flags);
    EXPECT_EQ(12, magLearnStatus.headings);
    EXPECT_LE(abs(sim.error(X)), 6);
    EXPECT_LE(abs(sim.error(Y)), 6);
    // The vertical shows only through the bank and the ridge holds it near the stored value: it closes over flights
    EXPECT_LE(abs(sim.error(Z)), 30);
}

TEST_F(CompassLearnTest, LevelTurnsKeepTheStoredVertical)
{
    Sim sim;
    sim.flyCircuits(2);
    sim.evaluate();

    EXPECT_EQ(MAG_LEARN_SAVE_DUE, magLearnStatus.flags);
    EXPECT_LE(abs(sim.error(X)), 6);
    EXPECT_LE(abs(sim.error(Y)), 6);
    EXPECT_LE(abs(magLearnStatus.delta[Z]), 15);
}

TEST_F(CompassLearnTest, RepeatedFlightsCloseTheVertical)
{
    Sim sim;
    sim.flyFlightsAndSave(3);

    for (int axis = 0; axis < 3; axis++) {
        EXPECT_LE(abs(sim.storedZero[axis] - kTrueZero[axis]), 5);
    }
}

// Fitting raw samples instead leaves the vertical about 70 counts off however many flights follow
TEST_F(CompassLearnTest, UnequalAxisGainsAreScaledOut)
{
    Sim sim;
    sim.axisScale[1] = 0.95f;
    sim.axisScale[2] = 1.06f;
    sim.setGainFromScale();
    sim.flyFlightsAndSave(3);

    for (int axis = 0; axis < 3; axis++) {
        EXPECT_LE(abs(sim.storedZero[axis] - kTrueZero[axis]), 5);
    }
}

TEST_F(CompassLearnTest, StraightLegIsNotEnough)
{
    Sim sim;
    for (int i = 0; i < 500; i++) {
        sim.push(0.0f, 0.05f, 0.3f);
    }
    sim.evaluate();

    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_FEW_SECTORS);
    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_FEW_HEADINGS);
    EXPECT_FALSE(magLearnStatus.flags & MAG_LEARN_SAVE_DUE);
    for (int axis = 0; axis < 3; axis++) {
        EXPECT_EQ(0, magLearnStatus.delta[axis]);
    }
}

TEST_F(CompassLearnTest, HalfTurnIsNotEnough)
{
    Sim sim;
    for (int deg = 0; deg < 180; deg += 3) {
        sim.push(0.5f, 0.05f, deg * M_PIf / 180.0f);
    }
    sim.evaluate();

    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_FEW_HEADINGS);
    EXPECT_FALSE(magLearnStatus.flags & MAG_LEARN_SAVE_DUE);
}

TEST_F(CompassLearnTest, StrongMotorFieldIsNotASphere)
{
    Sim sim;
    // A battery lead next to the compass, its current swinging with the throttle
    sim.interference = [](int n, float extra[3]) {
        const float throttle = 0.5f + 0.5f * sinf(n * 0.37f);
        extra[0] = 140.0f * throttle;
        extra[2] = -140.0f * throttle;
    };
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_NOT_SPHERE);
    EXPECT_GT(magLearnStatus.spread, MAG_LEARN_MAX_SPREAD);
    EXPECT_FALSE(magLearnStatus.flags & MAG_LEARN_SAVE_DUE);
}

TEST_F(CompassLearnTest, GainThatNoLongerHoldsIsOffScale)
{
    Sim sim;
    sim.setGainFromScale(1.6f);
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_OFF_SCALE);
    EXPECT_FALSE(magLearnStatus.flags & MAG_LEARN_SAVE_DUE);
}

TEST_F(CompassLearnTest, LargeCorrectionIsRefused)
{
    Sim sim(200, 0, 0);
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_STEP_TOO_BIG);
    EXPECT_FALSE(magLearnStatus.flags & MAG_LEARN_SAVE_DUE);
}

// maggain accepts any int16 from the CLI: a zero turns the scaled samples into inf and NaN
TEST_F(CompassLearnTest, ZeroGainIsNeverSaved)
{
    Sim sim;
    sim.gain[1] = 0;
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_FALSE(magLearnStatus.flags & MAG_LEARN_SAVE_DUE);
    EXPECT_EQ(0, magLearnStatus.delta[Y]);
}

TEST_F(CompassLearnTest, NegativeGainLearnsLikeAPositiveOne)
{
    Sim sim;
    sim.gain[2] = -sim.gain[2];
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_EQ(MAG_LEARN_SAVE_DUE, magLearnStatus.flags);
    EXPECT_LE(abs(sim.error(X)), 6);
    EXPECT_LE(abs(sim.error(Y)), 6);
    EXPECT_LE(abs(sim.error(Z)), 30);
}

TEST_F(CompassLearnTest, FlightStateBitsAreKept)
{
    Sim sim;
    sim.flyCircuits(30);
    magLearnStatus.flags |= MAG_LEARN_COLLECTING;
    sim.evaluate();

    EXPECT_EQ(MAG_LEARN_COLLECTING | MAG_LEARN_SAVE_DUE, magLearnStatus.flags);
}

}
