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

#include <math.h>

extern "C" {
    #include "flight/thrust_linear.h"
}

#include "unittest_macros.h"
#include "gtest/gtest.h"

// thrust_linear 0-100 in steps of 10
static float strength(int step)
{
    return step / 10.0f;
}

TEST(ThrustLinearTest, OffIsIdentity)
{
    for (int i = 0; i <= 100; i++) {
        const float x = i / 100.0f;
        EXPECT_FLOAT_EQ(thrustLinearCurve(x, 0.0f), x);
        EXPECT_FLOAT_EQ(thrustLinearCompensate(x, 0.0f), x);
    }
}

TEST(ThrustLinearTest, MatchesBetaflight)
{
    // Betaflight 2026.6 pidApplyThrustLinearization() / pidCompensateThrustLinearization(), worked by hand
    EXPECT_NEAR(thrustLinearCurve(0.3f, 0.5f), 0.426f, 1e-5f);
    EXPECT_NEAR(thrustLinearCompensate(0.3f, 0.5f), 0.195f, 1e-5f);
    EXPECT_NEAR(thrustLinearCurve(0.8f, 1.0f), 0.864f, 1e-5f);
}

TEST(ThrustLinearTest, CurveKeepsEndsAndOrder)
{
    for (int s = 0; s <= 10; s++) {
        const float e = strength(s);
        EXPECT_NEAR(thrustLinearCurve(0.0f, e), 0.0f, 1e-6f);
        EXPECT_NEAR(thrustLinearCurve(1.0f, e), 1.0f, 1e-6f);
        float previous = 0.0f;
        for (int i = 1; i <= 1000; i++) {
            const float u = i / 1000.0f;
            const float out = thrustLinearCurve(u, e);
            EXPECT_GE(out, previous) << "e " << e << " u " << u;
            EXPECT_GE(out, u - 1e-6f) << "raises, never lowers: e " << e << " u " << u;
            EXPECT_LE(out, 1.0f + 1e-6f) << "e " << e << " u " << u;
            previous = out;
        }
    }
}

TEST(ThrustLinearTest, CompensationNeverNegative)
{
    for (int s = 0; s <= 10; s++) {
        for (int i = 0; i <= 1000; i++) {
            EXPECT_GE(thrustLinearCompensate(i / 1000.0f, strength(s)), 0.0f);
        }
    }
}

TEST(ThrustLinearTest, HoverStaysWhereItWas)
{
    // Worst cases near 15 % throttle: 0.81 % of the range at 50, 2.5 % at 70, 8.7 % at 100
    for (int s = 0; s <= 10; s++) {
        const float e = strength(s);
        const float tolerance = e <= 0.5f ? 0.0085f : (e <= 0.7f ? 0.026f : 0.088f);
        for (int i = 100; i <= 1000; i++) {
            const float t = i / 1000.0f;
            EXPECT_NEAR(thrustLinearCurve(thrustLinearCompensate(t, e), e), t, tolerance) << "e " << e << " t " << t;
        }
    }
}
