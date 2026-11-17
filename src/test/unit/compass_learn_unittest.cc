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
#include <string.h>

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
// 10% of the radius in raw counts at the simulator's gain of 450, rounded up
const int kTenPercent = 46;

// Body to earth as imu.c reads it back: roll phi = atan2(R[2][1], R[2][2]), pitch theta = asin(-R[2][0]), heading psi
// = -atan2(R[1][0], R[0][0]), clockwise from north; the earth axes are north, west, up
void bodyToEarth(float phi, float theta, float psi, float R[3][3])
{
    const float cf = cosf(phi), sf = sinf(phi), ct = cosf(theta), st = sinf(theta), cp = cosf(psi), sp = sinf(psi);
    const float m[3][3] = {
        {cp * ct, cp * st * sf + sp * cf, cp * st * cf - sp * sf},
        {-sp * ct, -sp * st * sf + cp * cf, -sp * st * cf - cp * sf},
        {-st, ct * sf, ct * cf},
    };
    memcpy(R, m, sizeof(m));
}

void identity(float R[3][3])
{
    bodyToEarth(0.0f, 0.0f, 0.0f, R);
}

// A turn about up by deg, the way mag_yaw_trim turns the reading
void turnAboutUp(float deg, float R[3][3])
{
    bodyToEarth(0.0f, 0.0f, -deg * M_PIf / 180.0f, R);
}

struct Sim {
    float radius = 450.0f;                  // field magnitude in raw counts
    float axisScale[3] = {1.0f, 1.0f, 1.0f};
    int16_t storedZero[3];
    int16_t gain[3];
    float declination = 0.0f;               // rad east, of the field
    float configuredDeclination = 0.0f;     // what the AHRS applies
    float mount[3][3];                      // sensor to body, how the compass really sits
    float align[3][3];                      // sensor to body, as configured (align_mag and mag_yaw_trim together)
    bool withHeading = false;               // the attitude goes with each sample, as with a usable GSF
    float headingSigma = 0.02f;             // rad, how sure the heading reference says it is
    float headingBias = 0.0f;               // rad, how wrong it really is
    int16_t yawTrim = 0;                    // mag_yaw_trim as stored, decidegrees
    std::function<void(int, float[3])> interference;
    uint32_t seed = 12345;
    int sample = 0;

    explicit Sim(int16_t ex = 60, int16_t ey = -40, int16_t ez = 80)
    {
        storedZero[0] = kTrueZero[0] + ex;
        storedZero[1] = kTrueZero[1] + ey;
        storedZero[2] = kTrueZero[2] + ez;
        setGainFromScale();
        identity(mount);
        identity(align);
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

    // Earth field seen by the body at roll phi, pitch theta, heading psi
    void push(float phi, float theta, float psi)
    {
        const float b[3] = {
            radius * cosf(kInclination) * cosf(declination),
            -radius * cosf(kInclination) * sinf(declination),
            -radius * sinf(kInclination),
        };
        float R[3][3], reference[3][3];
        bodyToEarth(phi, theta, psi, R);
        bodyToEarth(phi, theta, psi + headingBias, reference);
        float body[3], sensor[3];
        for (int i = 0; i < 3; i++) {
            body[i] = R[0][i] * b[0] + R[1][i] * b[1] + R[2][i] * b[2];
        }
        for (int i = 0; i < 3; i++) {
            sensor[i] = mount[0][i] * body[0] + mount[1][i] * body[1] + mount[2][i] * body[2];
        }
        float extra[3] = {0.0f, 0.0f, 0.0f};
        if (interference) {
            interference(sample, extra);
        }
        sample++;

        int16_t raw[3];
        float scaled[3], seen[3];
        for (int i = 0; i < 3; i++) {
            raw[i] = lrintf(axisScale[i] * sensor[i] + kTrueZero[i] + extra[i] + noise());
            scaled[i] = (raw[i] - storedZero[i]) * 1024.0f / gain[i];
        }
        for (int i = 0; i < 3; i++) {
            seen[i] = align[i][0] * scaled[0] + align[i][1] * scaled[1] + align[i][2] * scaled[2];
        }
        magLearnAddSample(seen, raw, withHeading ? reference : NULL, headingSigma);
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

    // A leg, a half turn, a leg: enough for the attitude fit, not for the sphere fit
    void flyHalfTurn()
    {
        for (int i = 0; i < 300; i++) {
            push(0.0f, 0.05f, 0.3f);
        }
        for (int deg = 0; deg <= 190; deg += 2) {
            push(0.5f, 0.05f, 0.3f + deg * M_PIf / 180.0f);
        }
        for (int i = 0; i < 300; i++) {
            push(0.0f, 0.05f, 0.3f + 190.0f * M_PIf / 180.0f);
        }
    }

    void evaluate()
    {
        magLearnEvaluate(storedZero, gain, align, configuredDeclination, yawTrim);
    }

    // Each flight starts from what the previous one saved, as at disarm
    void flyFlightsAndSave(int flights)
    {
        for (int flight = 0; flight < flights; flight++) {
            magLearnReset();
            flyCircuits(30);
            evaluate();
            if (magLearnStatus.flags & MAG_LEARN_CONFIRMED) {
                return;
            }
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

    float fittedDeclinationDeg() const
    {
        return -atan2f(magLearnStatus.field[Y], magLearnStatus.field[X]) * 180.0f / M_PIf;
    }

    // The compass sits turned about up by deg, which the alignment does not know
    void mountTurned(float deg)
    {
        turnAboutUp(deg, mount);
    }

    // What the firmware does with a saved mag_yaw_trim: the reading turned after the alignment
    void applyYawTrim(float deg)
    {
        turnAboutUp(deg, align);
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
    ASSERT_NE(0, magLearnStatus.flags);

    magLearnReset();

    EXPECT_EQ(0, magLearnStatus.flags);
    EXPECT_EQ(0, magLearnStatus.sectors);
    EXPECT_EQ(0, magLearnStatus.headings);
    EXPECT_EQ(0, magLearnStatus.delta[X]);
}

TEST_F(CompassLearnTest, TurnsRecoverTheHorizontalOffset)
{
    Sim sim;
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_EQ(MAG_LEARN_SAVE_DUE, magLearnStatus.flags);
    EXPECT_EQ(12, magLearnStatus.headings);
    // The samples lie on a band of the sphere, not around it: the fit amplifies the noise to some 10 counts
    EXPECT_LE(abs(sim.error(X)), 15);
    EXPECT_LE(abs(sim.error(Y)), 15);
    // The vertical shows only through the bank and the ridge holds it near the stored value: it closes over flights
    EXPECT_LE(abs(sim.error(Z)), 45);
}

TEST_F(CompassLearnTest, LevelTurnsKeepTheStoredVertical)
{
    Sim sim;
    sim.flyCircuits(2);
    sim.evaluate();

    EXPECT_EQ(MAG_LEARN_SAVE_DUE, magLearnStatus.flags);
    EXPECT_LE(abs(sim.error(X)), 15);
    EXPECT_LE(abs(sim.error(Y)), 15);
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
        sim.push(0.2f, 0.05f, deg * M_PIf / 180.0f);
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
    EXPECT_LE(abs(sim.error(X)), 15);
    EXPECT_LE(abs(sim.error(Y)), 15);
    EXPECT_LE(abs(sim.error(Z)), 45);
}

TEST_F(CompassLearnTest, FlightStateBitsAreKept)
{
    Sim sim;
    sim.flyCircuits(30);
    magLearnStatus.flags |= MAG_LEARN_COLLECTING;
    sim.evaluate();

    EXPECT_EQ(MAG_LEARN_COLLECTING | MAG_LEARN_SAVE_DUE, magLearnStatus.flags);
}

TEST_F(CompassLearnTest, HeadingReplacesTheAhrsOne)
{
    float rMat[3][3], expected[3][3], R[3][3];
    bodyToEarth(0.4f, -0.2f, 1.0f, rMat);
    bodyToEarth(0.4f, -0.2f, 2.5f, expected);

    magLearnAttitude(rMat, 2.5f, R);

    EXPECT_NEAR(2.5f, -atan2f(R[1][0], R[0][0]), 1e-3f);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            EXPECT_NEAR(expected[i][j], R[i][j], 1e-3f);
        }
    }
}

// With both fits agreeing a calibration off by 40% of the radius is fixed in one flight on X and Y, 10% a flight on Z
TEST_F(CompassLearnTest, AttitudeFitFixesALargeOffset)
{
    Sim sim(180, -200, 100);
    sim.withHeading = true;
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_EQ(MAG_LEARN_ATTITUDE_FIT | MAG_LEARN_SAVE_DUE, magLearnStatus.flags);
    EXPECT_LE(abs(sim.error(X)), 6);
    EXPECT_LE(abs(sim.error(Y)), 6);
    EXPECT_LE(abs(magLearnStatus.delta[Z]), kTenPercent);
    EXPECT_LE(abs(sim.error(Z)), 60);
    EXPECT_LE(magLearnStatus.spread, 30);
    EXPECT_NEAR(1024.0f, calc_length_pythagorean_3D(magLearnStatus.field[X], magLearnStatus.field[Y], magLearnStatus.field[Z]), 25.0f);
}

// Without the sphere fit to confirm them the offsets move at most 10% of the radius a flight
TEST_F(CompassLearnTest, AHalfTurnIsEnoughForASmallStep)
{
    Sim sim;
    sim.withHeading = true;
    sim.flyHalfTurn();
    sim.evaluate();

    EXPECT_EQ(MAG_LEARN_ATTITUDE_FIT | MAG_LEARN_SAVE_DUE, magLearnStatus.flags);
    for (int axis = 0; axis < 3; axis++) {
        EXPECT_LE(abs(magLearnStatus.delta[axis]), kTenPercent);
    }
    EXPECT_LE(abs(sim.error(X)), 20);
    EXPECT_LE(abs(sim.error(Y)), 10);

    for (int axis = 0; axis < 3; axis++) {
        sim.storedZero[axis] += magLearnStatus.delta[axis];
    }
    magLearnReset();
    sim.flyHalfTurn();
    sim.evaluate();

    EXPECT_LE(abs(sim.error(X)), 10);
    EXPECT_LE(abs(sim.error(Y)), 10);
}

// A step beyond the sphere fit's limit is saved only when the sphere fit, with the coverage it needs, agrees
TEST_F(CompassLearnTest, ABigStepNeedsBothFits)
{
    Sim sim(180, -200, 100);
    sim.withHeading = true;
    sim.flyHalfTurn();
    sim.evaluate();

    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_ATTITUDE_FIT);
    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_STEP_TOO_BIG);
    EXPECT_FALSE(magLearnStatus.flags & MAG_LEARN_SAVE_DUE);
}

// The vertical is where the heading reference's delay leaves its mark: a small step takes at most 10% of it per flight
TEST_F(CompassLearnTest, SmallStepRefinedWithTheVerticalCapped)
{
    Sim sim;
    sim.withHeading = true;
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_EQ(MAG_LEARN_ATTITUDE_FIT | MAG_LEARN_SAVE_DUE, magLearnStatus.flags);
    EXPECT_LE(abs(sim.error(X)), 6);
    EXPECT_LE(abs(sim.error(Y)), 6);
    EXPECT_LE(abs(magLearnStatus.delta[Z]), 46);
    EXPECT_LE(abs(sim.error(Z)), 40);
}

// A compass tilted in a way the alignment does not know biases the attitude fit and not the sphere fit
TEST_F(CompassLearnTest, TiltedCompassMakesTheFitsDisagree)
{
    Sim sim;
    sim.withHeading = true;
    bodyToEarth(10.0f * M_PIf / 180.0f, 0.0f, 0.0f, sim.mount);
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_FITS_DISAGREE);
    EXPECT_FALSE(magLearnStatus.flags & MAG_LEARN_ATTITUDE_FIT);
    EXPECT_FALSE(magLearnStatus.flags & MAG_LEARN_SAVE_DUE);
}

TEST_F(CompassLearnTest, GoodCalibrationIsConfirmedNotRewritten)
{
    Sim sim(0, 0, 0);
    sim.withHeading = true;
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_EQ(MAG_LEARN_ATTITUDE_FIT | MAG_LEARN_CONFIRMED, magLearnStatus.flags);
    for (int axis = 0; axis < 3; axis++) {
        EXPECT_LT(abs(magLearnStatus.delta[axis]), 5);
    }

    magLearnReset();
    sim.withHeading = false;
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_EQ(MAG_LEARN_CONFIRMED, magLearnStatus.flags);
}

TEST_F(CompassLearnTest, StraightLegHasNoAttitudeFitEither)
{
    Sim sim;
    sim.withHeading = true;
    for (int i = 0; i < 500; i++) {
        sim.push(0.0f, 0.05f, 0.3f);
    }
    sim.evaluate();

    EXPECT_FALSE(magLearnStatus.flags & MAG_LEARN_ATTITUDE_FIT);
    EXPECT_FALSE(magLearnStatus.flags & MAG_LEARN_SAVE_DUE);
}

TEST_F(CompassLearnTest, LongLegDoesNotOutweighTheTurns)
{
    Sim sim(180, -200, 100);
    sim.withHeading = true;
    for (int i = 0; i < 3000; i++) {
        sim.push(0.0f, 0.05f, 0.3f);
    }
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_ATTITUDE_FIT);
    EXPECT_LE(abs(sim.error(X)), 6);
    EXPECT_LE(abs(sim.error(Y)), 6);
    EXPECT_LE(abs(sim.error(Z)), 60);
}

TEST_F(CompassLearnTest, AttitudeFitLearnsTheDeclination)
{
    Sim sim;
    sim.withHeading = true;
    sim.declination = 3.0f * M_PIf / 180.0f;
    sim.configuredDeclination = sim.declination;
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_ATTITUDE_FIT);
    EXPECT_NEAR(3.0f, sim.fittedDeclinationDeg(), 1.0f);
}

// The offsets are learned in body axes and stored in the sensor's
TEST_F(CompassLearnTest, OffsetsComeBackInSensorAxes)
{
    Sim sim(180, -200, 100);
    sim.withHeading = true;
    bodyToEarth(0.0f, 0.0f, M_PIf / 2, sim.mount);     // the compass sits turned 90 degrees on the board
    memcpy(sim.align, sim.mount, sizeof(sim.align));    // and align_mag says so
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_EQ(MAG_LEARN_ATTITUDE_FIT | MAG_LEARN_SAVE_DUE, magLearnStatus.flags);
    EXPECT_LE(abs(sim.error(X)), 6);
    EXPECT_LE(abs(sim.error(Y)), 6);
    EXPECT_LE(abs(sim.error(Z)), 60);
}

// In level turns a compass turned about the vertical is a field turned the same way: the declination tells
TEST_F(CompassLearnTest, TurnedCompassShowsInTheDeclination)
{
    Sim sim;
    sim.withHeading = true;
    bodyToEarth(0.0f, 0.0f, 30.0f * M_PIf / 180.0f, sim.mount);
    sim.flyCircuits(5);
    sim.evaluate();

    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_DECLINATION_OFF);
    EXPECT_FALSE(magLearnStatus.flags & MAG_LEARN_ATTITUDE_FIT);
    EXPECT_NEAR(-30.0f, sim.fittedDeclinationDeg(), 2.0f);
    // The sphere fit still has its say
    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_SAVE_DUE);
}

TEST_F(CompassLearnTest, FlippedCompassIsAMisfit)
{
    Sim sim;
    sim.withHeading = true;
    bodyToEarth(M_PIf, 0.0f, 0.0f, sim.mount);
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_MISFIT);
    EXPECT_FALSE(magLearnStatus.flags & MAG_LEARN_ATTITUDE_FIT);
}

TEST_F(CompassLearnTest, WrongDeclinationSettingBlocksTheAttitudeFit)
{
    Sim sim;
    sim.withHeading = true;
    sim.declination = 3.0f * M_PIf / 180.0f;
    sim.configuredDeclination = 25.0f * M_PIf / 180.0f;
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_DECLINATION_OFF);
    EXPECT_FALSE(magLearnStatus.flags & MAG_LEARN_ATTITUDE_FIT);
}

TEST_F(CompassLearnTest, AttitudeFitOffTheScaleIsRefused)
{
    Sim sim;
    sim.withHeading = true;
    sim.setGainFromScale(1.6f);
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_ATTITUDE_FIT);
    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_OFF_SCALE);
    EXPECT_FALSE(magLearnStatus.flags & MAG_LEARN_SAVE_DUE);
}

// A compass turned a little about up: the fit sees the field off magnetic north and trims the heading back
TEST_F(CompassLearnTest, SlightlyTurnedCompassIsTrimmed)
{
    Sim sim;
    sim.withHeading = true;
    sim.mountTurned(8.0f);
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_EQ(MAG_LEARN_ATTITUDE_FIT | MAG_LEARN_SAVE_DUE, magLearnStatus.flags);
    EXPECT_EQ(MAG_LEARN_MAX_YAW_TRIM_STEP, magLearnStatus.yawTrimDelta);

    // Each flight starts with the trim the previous one saved: the rest comes the next flight, then nothing is left
    float trim = 0.0f;
    for (int flight = 0; flight < 3; flight++) {
        sim.yawTrim += magLearnStatus.yawTrimDelta;
        trim = sim.yawTrim / 10.0f;
        sim.applyYawTrim(trim);
        for (int axis = 0; axis < 3; axis++) {
            sim.storedZero[axis] += magLearnStatus.delta[axis];
        }
        magLearnReset();
        sim.flyCircuits(30);
        sim.evaluate();
    }

    EXPECT_NEAR(8.0f, trim, 0.6f);
    EXPECT_EQ(0, magLearnStatus.yawTrimDelta);
    EXPECT_NEAR(0.0f, sim.fittedDeclinationDeg(), 1.0f);
}

TEST_F(CompassLearnTest, TrimMovesAtMostFiveDegreesAFlight)
{
    Sim sim;
    sim.withHeading = true;
    sim.mountTurned(12.0f);
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_ATTITUDE_FIT);
    EXPECT_EQ(MAG_LEARN_MAX_YAW_TRIM_STEP, magLearnStatus.yawTrimDelta);
}

TEST_F(CompassLearnTest, TrimIsNotTouchedByTheSphereFitOrAFarTurnedCompass)
{
    Sim sim;
    sim.flyCircuits(30);
    sim.evaluate();
    EXPECT_EQ(0, magLearnStatus.yawTrimDelta);

    magLearnReset();
    sim.withHeading = true;
    sim.mountTurned(30.0f);
    sim.flyCircuits(5);
    sim.evaluate();
    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_DECLINATION_OFF);
    EXPECT_EQ(0, magLearnStatus.yawTrimDelta);
}

TEST_F(CompassLearnTest, ATrimAloneIsWorthASave)
{
    Sim sim(0, 0, 0);
    sim.withHeading = true;
    sim.mountTurned(3.0f);
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_EQ(MAG_LEARN_ATTITUDE_FIT | MAG_LEARN_SAVE_DUE, magLearnStatus.flags);
    EXPECT_NEAR(30, magLearnStatus.yawTrimDelta, 8);
    for (int axis = 0; axis < 3; axis++) {
        EXPECT_LT(abs(magLearnStatus.delta[axis]), 5);
    }
}

TEST_F(CompassLearnTest, AttitudeFitZeroGainIsNeverSaved)
{
    Sim sim;
    sim.withHeading = true;
    sim.gain[1] = 0;
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_FALSE(magLearnStatus.flags & MAG_LEARN_SAVE_DUE);
    EXPECT_EQ(0, magLearnStatus.delta[Y]);
}

// Two legs with the heading reference off in opposite ways: without the sphere fit the step stays within 10% of the radius
TEST_F(CompassLearnTest, AHeadingErrorWithoutTheSphereFitMovesTheOffsetsLittle)
{
    Sim sim(0, 0, 0);
    sim.withHeading = true;
    sim.headingBias = 12.0f * M_PIf / 180.0f;
    for (int i = 0; i < 300; i++) {
        sim.push(0.0f, 0.05f, 0.3f);
    }
    for (int deg = 0; deg <= 190; deg += 2) {
        sim.headingBias = (12.0f - deg * 24.0f / 190.0f) * M_PIf / 180.0f;
        sim.push(0.5f, 0.05f, 0.3f + deg * M_PIf / 180.0f);
    }
    for (int i = 0; i < 300; i++) {
        sim.push(0.0f, 0.05f, 0.3f + 190.0f * M_PIf / 180.0f);
    }
    sim.evaluate();

    ASSERT_TRUE(magLearnStatus.flags & MAG_LEARN_ATTITUDE_FIT);
    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_SAVE_DUE);
    for (int axis = 0; axis < 3; axis++) {
        EXPECT_LE(abs(magLearnStatus.delta[axis]), kTenPercent);
    }
}

// The first samples after the heading turns usable are its least sure: off by 8 degrees there, they barely count
TEST_F(CompassLearnTest, EarlyUnsureSamplesWeighLittle)
{
    Sim sim;
    sim.withHeading = true;
    sim.headingSigma = 0.17f;
    sim.headingBias = 8.0f * M_PIf / 180.0f;
    for (int i = 0; i < 200; i++) {
        sim.push(0.0f, 0.05f, 0.3f);
    }
    sim.headingSigma = 0.02f;
    sim.headingBias = 0.0f;
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_EQ(MAG_LEARN_ATTITUDE_FIT | MAG_LEARN_SAVE_DUE, magLearnStatus.flags);
    EXPECT_LE(abs(sim.error(X)), 6);
    EXPECT_LE(abs(sim.error(Y)), 6);
    EXPECT_EQ(0, magLearnStatus.yawTrimDelta);
}

// A trim already at the end of mag_yaw_trim's range has no room left: reported, not confirmed, nothing to save
TEST_F(CompassLearnTest, ATrimAtItsLimitAsksForNoSave)
{
    Sim sim(0, 0, 0);
    sim.withHeading = true;
    sim.mountTurned(38.0f);
    sim.yawTrim = MAG_LEARN_MAX_YAW_TRIM;
    sim.applyYawTrim(30.0f);
    sim.flyCircuits(30);
    sim.evaluate();

    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_ATTITUDE_FIT);
    EXPECT_EQ(0, magLearnStatus.yawTrimDelta);
    EXPECT_TRUE(magLearnStatus.flags & MAG_LEARN_DECLINATION_OFF);
    EXPECT_FALSE(magLearnStatus.flags & MAG_LEARN_CONFIRMED);
    EXPECT_FALSE(magLearnStatus.flags & MAG_LEARN_SAVE_DUE);
}

}
