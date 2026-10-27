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
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include <array>
#include <string>
#include <vector>

extern "C" {
    #include "common/maths.h"
    #include "flight/gsf_yaw.h"
}

#include "unittest_macros.h"
#include "gtest/gtest.h"

// A flight with no wind and no sideslip, so the heading is the track. The IMU's level frame turns with the heading,
// as INAV's does: dvLevel is the velocity change forward and right of the heading at the start of each step.
class GsfFlight {
public:
    gsfState_t gsf;
    float heading;      // rad, clockwise from north
    float speed;        // m/s
    float velNoise;     // GPS velocity noise (m/s), uniform
    int step = 0;
    uint32_t rng = 12345;

    GsfFlight(float headingDeg, float speedMs, float noise = 0.0f) : heading(headingDeg * RAD), speed(speedMs), velNoise(noise)
    {
        gsfReset(&gsf);
    }

    float noise()
    {
        rng = rng * 1664525u + 1013904223u;
        return velNoise * (2.0f * (rng >> 8) / 16777216.0f - 1.0f);
    }

    // turnRate in deg/s, accel in m/s^2; 50 Hz steps, GPS at 10 Hz
    void fly(float seconds, float turnRateDeg, float accel)
    {
        const float dt = 0.02f;
        for (int n = 0; n < (int)(seconds / dt); n++) {
            const float vN0 = speed * cosf(heading);
            const float vE0 = speed * sinf(heading);
            const float headingStart = heading;
            heading += turnRateDeg * RAD * dt;
            speed += accel * dt;
            const float dN = speed * cosf(heading) - vN0;
            const float dE = speed * sinf(heading) - vE0;
            const float dvLevel[2] = {
                dN * cosf(headingStart) + dE * sinf(headingStart),
                -dN * sinf(headingStart) + dE * cosf(headingStart),
            };
            gsfPredict(&gsf, dvLevel, turnRateDeg * RAD * dt, dt);
            if (++step % 5 == 0) {
                gsfFuseVelocity(&gsf, speed * cosf(heading) + noise(), speed * sinf(heading) + noise(), 0.5f);
            }
            gsfUpdateComposite(&gsf);
        }
    }

    float errorDeg() const
    {
        float e = (gsf.yaw - heading) / RAD;
        while (e > 180.0f) e -= 360.0f;
        while (e < -180.0f) e += 360.0f;
        return fabsf(e);
    }

    float sigmaDeg() const
    {
        return sqrtf(gsf.yawVariance) / RAD;
    }
};

TEST(GsfYawTest, ConvergesFromAnyHeadingAfterATurn)
{
    for (int start = 0; start < 360; start += 30) {
        GsfFlight f(start + 7.0f, 18.0f);
        f.fly(5.0f, 0.0f, 0.0f);
        f.fly(3.0f, 0.0f, 1.0f);
        f.fly(18.0f, 20.0f, 0.0f);     // a full turn to the right
        EXPECT_LT(f.errorDeg(), 2.0f) << "start heading " << start;
        EXPECT_LT(f.sigmaDeg(), 5.0f) << "start heading " << start;
        f.fly(10.0f, 0.0f, 0.0f);
        f.fly(18.0f, -20.0f, 0.0f);    // and one to the left
        EXPECT_LT(f.errorDeg(), 2.0f) << "start heading " << start;
    }
}

TEST(GsfYawTest, DoesNotClaimAHeadingWithoutManoeuvres)
{
    // Straight and level at constant speed: nothing tells the heading apart
    GsfFlight f(123.0f, 18.0f);
    f.fly(30.0f, 0.0f, 0.0f);
    EXPECT_GT(f.sigmaDeg(), 30.0f);
}

TEST(GsfYawTest, ConvergesWithNoisyGpsVelocity)
{
    for (int start = 0; start < 360; start += 45) {
        GsfFlight f(start + 3.0f, 16.0f, 0.4f);
        f.fly(5.0f, 0.0f, 0.0f);
        f.fly(36.0f, 20.0f, 0.0f);     // two full turns
        EXPECT_LT(f.errorDeg(), 5.0f) << "start heading " << start;
    }
}

TEST(GsfYawTest, WrapsAcrossSouth)
{
    // Heading near +-180 deg, where the composite yaw changes sign
    GsfFlight f(178.0f, 18.0f);
    f.fly(5.0f, 0.0f, 0.0f);
    f.fly(18.0f, 20.0f, 0.0f);
    f.fly(1.0f, 4.0f, 0.0f);
    EXPECT_LT(f.errorDeg(), 2.0f);
}

// The GPS reports the velocity it measured delayUs before the message arrives. The IMU history is the velocity change
// in a frame turned by an unknown angle from north, with an accelerometer bias and noise
class DelayFlight {
public:
    gsfImuHistory_t history;
    gsfDelayEstimator_t estimator;
    uint32_t delayUs;
    uint32_t gpsPeriodUs;
    bool turning;
    float gpsNoise;     // m/s
    float frameDrift;   // rad/s, the gyro's yaw bias turning the history frame
    uint32_t rng = 777;

    DelayFlight(uint32_t delay, uint32_t gpsPeriod, bool turns, float noise = 0.05f, float drift = 0.0f)
        : delayUs(delay), gpsPeriodUs(gpsPeriod), turning(turns), gpsNoise(noise), frameDrift(drift)
    {
        memset(&history, 0, sizeof(history));
        memset(&estimator, 0, sizeof(estimator));
    }

    float uniform()
    {
        rng = rng * 1664525u + 1013904223u;
        return 2.0f * (rng >> 8) / 16777216.0f - 1.0f;
    }

    std::string dump() const
    {
        std::string s = " samples " + std::to_string(estimator.samples) + " sgg " + std::to_string(estimator.sgg) + " cost";
        for (int j = 0; j < GSF_DELAY_STEPS; j++) {
            const float c = estimator.sii[j] + estimator.sgg - 2.0f * sqrtf(estimator.sdot[j] * estimator.sdot[j] + estimator.scross[j] * estimator.scross[j]);
            s += " " + std::to_string(c);
        }
        return s;
    }

    // Turn rate in rad/s: straight 5 s, right 6 s, straight 4 s, left 6 s, with a 0.4 s roll in and out
    float turnRate(float t) const
    {
        if (!turning) {
            return 0.0f;
        }
        const float phase = fmodf(t, 21.0f);
        const float rate = 20.0f * RAD;
        auto ramp = [](float x) { return x <= 0.0f ? 0.0f : (x >= 0.4f ? 1.0f : x / 0.4f); };
        if (phase < 5.0f) {
            return 0.0f;
        } else if (phase < 11.0f) {
            return rate * ramp(phase - 5.0f) * ramp(11.0f - phase);
        } else if (phase < 15.0f) {
            return 0.0f;
        }
        return -rate * ramp(phase - 15.0f) * ramp(21.0f - phase);
    }

    void fly(float seconds)
    {
        const float speed = 18.0f;
        float frame = 77.0f * RAD;
        const float bias[2] = { 0.15f, -0.1f };     // m/s^2 in the history frame
        const uint32_t start = 1000000;
        float heading = 0.3f;
        std::vector<std::array<float, 2>> truth;    // every 20 ms
        uint32_t nextGps = start + gpsPeriodUs;
        const int steps = (int)(seconds / 0.002f);
        const float v0[2] = { speed * cosf(heading), speed * sinf(heading) };

        for (int n = 1; n <= steps; n++) {
            const float t = n * 0.002f;
            heading += turnRate(t) * 0.002f;
            const uint32_t now = start + n * 2000;

            frame += frameDrift * 0.002f;
            if (n % 10 == 0) {
                const float vN = speed * cosf(heading), vE = speed * sinf(heading);
                truth.push_back({ vN, vE });

                // 20 ms window: the true change turned into the history frame, plus bias and noise
                const float dN = vN - v0[0], dE = vE - v0[1];
                const float imuVel[2] = {
                    cosf(frame) * dN + sinf(frame) * dE + bias[0] * t + 0.01f * uniform(),
                    -sinf(frame) * dN + cosf(frame) * dE + bias[1] * t + 0.01f * uniform(),
                };
                gsfHistoryAdd(&history, now, imuVel);

                // Measured at nextGps, arriving delayUs later with up to 10 ms of jitter
                while (nextGps + delayUs <= now) {
                    const size_t idx = (nextGps - start) / 20000 - 1;
                    const float vel[2] = { truth[idx][0] + gpsNoise * uniform(), truth[idx][1] + gpsNoise * uniform() };
                    const uint32_t arrival = nextGps + delayUs + (uint32_t)(5000.0f * (1.0f + uniform()));
                    gsfDelayAddGps(&estimator, &history, MIN(arrival, now), vel);
                    nextGps += gpsPeriodUs;
                }
            }
        }
    }
};

TEST(GsfYawTest, MeasuresTheGpsDelayInTurns)
{
    const uint32_t delays[] = { 0, 60000, 120000, 200000, 260000 };
    for (uint32_t delay : delays) {
        DelayFlight f(delay, 100000, true);
        f.fly(80.0f);
        uint32_t estimate = 0;
        ASSERT_TRUE(gsfDelayEstimate(&f.estimator, &estimate)) << "delay " << delay << f.dump();
        EXPECT_NEAR((float)estimate, (float)delay, 15000.0f) << "delay " << delay;
    }
}

TEST(GsfYawTest, MeasuresTheGpsDelayAtFiveHertz)
{
    DelayFlight f(220000, 200000, true);
    f.fly(120.0f);
    uint32_t estimate = 0;
    ASSERT_TRUE(gsfDelayEstimate(&f.estimator, &estimate));
    EXPECT_NEAR((float)estimate, 220000.0f, 20000.0f);
}

TEST(GsfYawTest, MeasuresTheGpsDelayWithANoisyGpsAndGyroDrift)
{
    // Three times the velocity noise, and the history frame drifting 0.1 deg/s against north
    DelayFlight f(120000, 100000, true, 0.15f, 0.1f * RAD);
    f.fly(120.0f);
    uint32_t estimate = 0;
    ASSERT_TRUE(gsfDelayEstimate(&f.estimator, &estimate)) << f.dump();
    EXPECT_NEAR((float)estimate, 120000.0f, 25000.0f);
}

TEST(GsfYawTest, NoGpsDelayWithoutManoeuvres)
{
    DelayFlight f(120000, 100000, false);
    f.fly(80.0f);
    uint32_t estimate = 0;
    EXPECT_FALSE(gsfDelayEstimate(&f.estimator, &estimate));
}

// Earth-up in body axes for a roll and a pitch, as rMat's last row: (-sin pitch, sin roll cos pitch, cos roll cos pitch)
static void earthUp(float rollDeg, float pitchDeg, float up[3])
{
    up[0] = -sinf(pitchDeg * RAD);
    up[1] = sinf(rollDeg * RAD) * cosf(pitchDeg * RAD);
    up[2] = cosf(rollDeg * RAD) * cosf(pitchDeg * RAD);
}

TEST(GsfYawTest, YawRateIsTheNoseHeadingRate)
{
    float up[3];

    // A roll about the nose at 20 deg pitch leaves the heading alone (the rate about the vertical would say 0.34 rad/s)
    earthUp(0.0f, 20.0f, up);
    const float roll[3] = { 1.0f, 0.0f, 0.0f };
    EXPECT_NEAR(gsfYawRate(up, roll), 0.0f, 1e-4f);

    // Level, a rotation about the body's up axis (counterclockwise seen from above) lowers the heading
    earthUp(0.0f, 0.0f, up);
    const float yawLeft[3] = { 0.0f, 0.0f, 0.5f };
    EXPECT_NEAR(gsfYawRate(up, yawLeft), -0.5f, 1e-4f);

    // A clockwise turn about the vertical at 0.3 rad/s, banked 45 deg and 10 deg nose up: the body rates are the
    // vertical rate spread over the body axes, and the heading rate is the whole 0.3
    earthUp(45.0f, 10.0f, up);
    const float turn[3] = { -0.3f * up[0], -0.3f * up[1], -0.3f * up[2] };
    EXPECT_NEAR(gsfYawRate(up, turn), 0.3f, 1e-4f);
}

TEST(GsfYawTest, ClipsASpikeAlongCorrelatedVelocityErrors)
{
    gsfState_t gsf;
    gsfReset(&gsf);
    gsf.fusing = true;
    const float velN = 10.0f, velE = -4.0f;
    for (int i = 0; i < GSF_MODEL_COUNT; i++) {
        gsf.model[i].X[0] = velN;
        gsf.model[i].X[1] = velE;
    }

    // North and east errors that move together: an innovation of opposite signs is 6.7 sigma away (test ratio 45),
    // though each axis alone looks like 3.4 sigma
    gsfModel_t *m = &gsf.model[0];
    memset(m->P, 0, sizeof(m->P));
    m->P[0][0] = m->P[1][1] = 0.75f;
    m->P[0][1] = m->P[1][0] = 0.6f;
    m->P[2][2] = 0.1f;
    m->X[0] = velN + 3.0f;
    m->X[1] = velE - 3.0f;
    m->X[2] = 0.0f;

    ASSERT_TRUE(gsfFuseVelocity(&gsf, velN, velE, 0.5f));

    // Gain 0.15 / (0.15 + 0.25) along the innovation, scaled down to 5 sigma
    const float step = 0.375f * 3.0f * sqrtf(25.0f / 45.0f);
    EXPECT_NEAR(m->X[0], velN + 3.0f - step, 1e-3f);
    EXPECT_NEAR(m->X[1], velE - 3.0f + step, 1e-3f);
}
