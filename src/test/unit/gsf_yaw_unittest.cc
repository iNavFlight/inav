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
