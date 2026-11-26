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

/* Every ref_* function is a verbatim copy of the inline expression its helper replaced (the comment
 * gives the call site at d1a87ed6c); bit-exact, because the coordinator latches on centidegree steps. */

#include <stdint.h>
#include <math.h>
#include <string.h>

extern "C" {
#include "platform.h"
#include "common/maths.h"
#include "common/time.h"
#include "sensors/acceleration.h"
#include "navigation/navigation_fixedwing_turn_math.h"
}

#include "unittest_macros.h"
#include "gtest/gtest.h"

// Bit-exact float comparison: NaN-safe and sensitive to the sign of zero
static bool bitsEqual(float a, float b)
{
    return memcmp(&a, &b, sizeof(float)) == 0;
}

#define EXPECT_BITS_EQ(a, b) EXPECT_TRUE(bitsEqual((a), (b))) \
    << "got " << (a) << " want " << (b)

// ---------------------------------------------------------------- references

// P1 @610, 779, 859, 916, 1029, 1092, 1186
static float ref_bankForRadiusCd(float v, float r)
{
    return DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(atan2_approx(v * v, GRAVITY_CMSS * r)));
}

// P2 @775, 824, 848, 954
static int32_t ref_radToBearingCd(float rad)
{
    return wrap_36000(lrintf(DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(rad))));
}

// P2b @839 (dot-product form) and @1089/1168 (atan2 form)
static void ref_tangentDir(float th, int8_t dir, float *tx, float *ty)
{
    *tx = -dir * sin_approx(th);
    *ty = dir * cos_approx(th);
}

// P3 @791-792, 889-890, 921-922, 989-990, 1017-1018, 1051, 1402
static void ref_bearingUnit(int32_t bearingCd, float *ux, float *uy)
{
    const float legRad = CENTIDEGREES_TO_RADIANS((float)bearingCd);
    *ux = cos_approx(legRad);
    *uy = sin_approx(legRad);
}

// P3 @1359-1361: pre-existing site, no explicit (float) cast on the bearing
static void ref_bearingUnitUncast(int32_t bearingCd, float *ux, float *uy)
{
    *ux = cos_approx(CENTIDEGREES_TO_RADIANS(bearingCd));
    *uy = sin_approx(CENTIDEGREES_TO_RADIANS(bearingCd));
}

// P4b @798-799, 837-838, 1265-1266
static void ref_polarOffset(float px, float py, float d, float a, float *ox, float *oy)
{
    *ox = px + d * cos_approx(a);
    *oy = py + d * sin_approx(a);
}

// P4b @812-813, 942-943: the subtracting form, replaced by passing -d
static void ref_polarOffsetMinus(float px, float py, float d, float a, float *ox, float *oy)
{
    *ox = px - d * cos_approx(a);
    *oy = py - d * sin_approx(a);
}

// P4 @785-786, 892-893, 895-896, 926-927, 1019-1020
static void ref_perpOffsetPlus(float px, float py, float r, float h, int8_t dir, float *ox, float *oy)
{
    *ox = px + r * cos_approx(h + dir * (M_PIf * 0.5f));
    *oy = py + r * sin_approx(h + dir * (M_PIf * 0.5f));
}

// P4 @801-802, 928-929: the "-" sites, replaced by passing -dirF
static void ref_perpOffsetMinus(float px, float py, float r, float h, int8_t dir, float *ox, float *oy)
{
    *ox = px + r * cos_approx(h - (float)dir * (M_PIf * 0.5f));
    *oy = py + r * sin_approx(h - (float)dir * (M_PIf * 0.5f));
}

// P5 @803-807 / 890-903
static bool ref_lineIntersect(float p1x, float p1y, float d1x, float d1y,
                              float p2x, float p2y, float d2x, float d2y,
                              float minAbsCross, float *ox, float *oy)
{
    const float cross = d1x * d2y - d1y * d2x;
    if (fabsf(cross) > minAbsCross) {
        const float tt = ((p2x - p1x) * d2y - (p2y - p1y) * d2x) / cross;
        *ox = p1x + tt * d1x;
        *oy = p1y + tt * d1y;
        return true;
    }
    return false;
}

// P7 @453-454: smoothstep with the multiply written the other way round
static float ref_smoothBlendSFirst(float from, float to, float p)
{
    const float s = p * p * (3.0f - 2.0f * p);
    return from + s * (to - from);
}

// P7 @702-703, 1075-1076, 1096+1099
static float ref_smoothBlend(float from, float to, float p)
{
    const float s = p * p * (3.0f - 2.0f * p);
    return from + (to - from) * s;
}

// P8 @616-618, 1100-1101, 1118-1119, 1189-1190
static float ref_slewToward(float cur, float target, float maxStep)
{
    float out = cur;
    out += constrainf(target - out, -maxStep, maxStep);
    return out;
}

// P8 @1100, 1118, 1189
static float ref_arcMaxStepCd(float phiCd, timeDelta_t deltaMicros, float tEaseMs)
{
    return phiCd * (US2S(deltaMicros) * 1000.0f) / MAX(tEaseMs, 1.0f);
}

// P13 @1051-1053, 1402-1404
static float ref_offLegCm(float px, float py, float qx, float qy, float legRad)
{
    return (px - qx) * (-sin_approx(legRad)) + (py - qy) * cos_approx(legRad);
}

// P17 @784, 935, 1008
static float ref_rollInLeadCm(float v, float tMs)
{
    return 1.5f * v * (tMs / 1000.0f);
}

// -------------------------------------------------------------------- sweeps

// Groundspeed 5..60 m/s in cm/s, turn radius 10..300 m in cm
static const float SPEEDS_CMS[] = { 500.0f, 733.0f, 1000.0f, 1417.0f, 2000.0f,
                                    2750.0f, 3500.0f, 4333.0f, 5000.0f, 6000.0f };
static const float RADII_CM[]   = { 1000.0f, 1637.0f, 2500.0f, 4000.0f, 6500.0f,
                                    10000.0f, 15500.0f, 21000.0f, 27000.0f, 30000.0f };

TEST(NavFwTurnMathUnittest, BankForRadius)
{
    for (float v : SPEEDS_CMS) {
        for (float r : RADII_CM) {
            EXPECT_BITS_EQ(fwBankForRadiusCd(v, r), ref_bankForRadiusCd(v, r));
        }
    }
}

TEST(NavFwTurnMathUnittest, RadToBearing)
{
    for (int32_t cd = 0; cd < 36000; cd += 37) {
        const float rad = CENTIDEGREES_TO_RADIANS((float)cd);
        EXPECT_EQ(fwRadToBearingCd(rad), ref_radToBearingCd(rad));
    }
    // outside one turn and negative, where wrap_36000 does the work
    for (float rad = -12.0f; rad <= 12.0f; rad += 0.013f) {
        EXPECT_EQ(fwRadToBearingCd(rad), ref_radToBearingCd(rad));
    }
}

TEST(NavFwTurnMathUnittest, TangentDir)
{
    for (int32_t cd = 0; cd < 36000; cd += 37) {
        const float alpha = CENTIDEGREES_TO_RADIANS((float)cd - 18000.0f);
        for (int8_t dir = -1; dir <= 1; dir += 2) {
            float tx, ty, rx, ry;
            fwTangentDir(alpha, dir, &tx, &ty);
            ref_tangentDir(alpha, dir, &rx, &ry);
            EXPECT_BITS_EQ(tx, rx);
            EXPECT_BITS_EQ(ty, ry);
            // the @1089/1168 consumers feed the pair to atan2_approx(y, x)
            EXPECT_BITS_EQ(atan2_approx(ty, tx), atan2_approx(ry, rx));
        }
    }
}

TEST(NavFwTurnMathUnittest, BearingUnit)
{
    for (int32_t cd = 0; cd < 36000; cd += 37) {
        float ux, uy, rx, ry;
        fwBearingUnit(cd, &ux, &uy);
        ref_bearingUnit(cd, &rx, &ry);
        EXPECT_BITS_EQ(ux, rx);
        EXPECT_BITS_EQ(uy, ry);
        ref_bearingUnitUncast(cd, &rx, &ry);
        EXPECT_BITS_EQ(ux, rx);
        EXPECT_BITS_EQ(uy, ry);
    }
}

TEST(NavFwTurnMathUnittest, PolarOffset)
{
    const float px = -13750.0f, py = 92100.0f;
    for (int32_t cd = 0; cd < 36000; cd += 37) {
        const float a = CENTIDEGREES_TO_RADIANS((float)cd - 18000.0f);
        for (float d : RADII_CM) {
            float ox, oy, rx, ry;
            fwPolarOffset(px, py, d, a, &ox, &oy);
            ref_polarOffset(px, py, d, a, &rx, &ry);
            EXPECT_BITS_EQ(ox, rx);
            EXPECT_BITS_EQ(oy, ry);
            // the two "p - d * cos(a)" sites pass -d instead
            fwPolarOffset(px, py, -d, a, &ox, &oy);
            ref_polarOffsetMinus(px, py, d, a, &rx, &ry);
            EXPECT_BITS_EQ(ox, rx);
            EXPECT_BITS_EQ(oy, ry);
            // 2R form used at @798-799
            fwPolarOffset(px, py, 2.0f * d, a, &ox, &oy);
            ref_polarOffset(px, py, 2.0f * d, a, &rx, &ry);
            EXPECT_BITS_EQ(ox, rx);
            EXPECT_BITS_EQ(oy, ry);
        }
    }
}

TEST(NavFwTurnMathUnittest, PerpOffset)
{
    const float px = 48320.0f, py = -7710.0f;
    for (int32_t cd = 0; cd < 36000; cd += 37) {
        const float h = CENTIDEGREES_TO_RADIANS((float)cd);
        for (float r : RADII_CM) {
            for (int8_t dir = -1; dir <= 1; dir += 2) {
                float ox, oy, rx, ry;
                fwPerpOffset(px, py, r, h, dir, &ox, &oy);
                ref_perpOffsetPlus(px, py, r, h, dir, &rx, &ry);
                EXPECT_BITS_EQ(ox, rx);
                EXPECT_BITS_EQ(oy, ry);
                fwPerpOffset(px, py, r, h, -(float)dir, &ox, &oy);
                ref_perpOffsetMinus(px, py, r, h, dir, &rx, &ry);
                EXPECT_BITS_EQ(ox, rx);
                EXPECT_BITS_EQ(oy, ry);
            }
        }
    }
}

TEST(NavFwTurnMathUnittest, LineIntersect)
{
    const float p1x = 1200.0f, p1y = -3400.0f;
    const float p2x = -9100.0f, p2y = 26500.0f;
    for (int32_t a = 0; a < 36000; a += 337) {
        for (int32_t b = 0; b < 36000; b += 331) {
            float d1x, d1y, d2x, d2y;
            fwBearingUnit(a, &d1x, &d1y);
            fwBearingUnit(b, &d2x, &d2y);
            for (float minCross : { 0.087f, 0.17f }) {
                float ox = 0.0f, oy = 0.0f, rx = 0.0f, ry = 0.0f;
                const bool got = fwLineIntersect(p1x, p1y, d1x, d1y, p2x, p2y, d2x, d2y, minCross, &ox, &oy);
                const bool want = ref_lineIntersect(p1x, p1y, d1x, d1y, p2x, p2y, d2x, d2y, minCross, &rx, &ry);
                EXPECT_EQ(got, want);
                if (want) {
                    EXPECT_BITS_EQ(ox, rx);
                    EXPECT_BITS_EQ(oy, ry);
                }
            }
        }
    }
}

TEST(NavFwTurnMathUnittest, LineIntersectDegenerate)
{
    const float p1x = 0.0f, p1y = 0.0f, p2x = 500.0f, p2y = 500.0f;
    float ox = 1.0f, oy = 2.0f;

    // exactly parallel and exactly anti-parallel
    EXPECT_FALSE(fwLineIntersect(p1x, p1y, 1.0f, 0.0f, p2x, p2y, 1.0f, 0.0f, 0.087f, &ox, &oy));
    EXPECT_FALSE(fwLineIntersect(p1x, p1y, 1.0f, 0.0f, p2x, p2y, -1.0f, 0.0f, 0.087f, &ox, &oy));
    EXPECT_BITS_EQ(ox, 1.0f);           // outputs untouched on the degenerate path
    EXPECT_BITS_EQ(oy, 2.0f);
    // NaN geometry must take the fallback, like the original `> threshold` guard
    EXPECT_FALSE(fwLineIntersect(p1x, p1y, NAN, 0.0f, p2x, p2y, 0.0f, 1.0f, 0.087f, &ox, &oy));
    EXPECT_BITS_EQ(ox, 1.0f);
    EXPECT_BITS_EQ(oy, 2.0f);

    // cross exactly on / just below / just above each threshold
    for (float thr : { 0.087f, 0.17f }) {
        for (float cross : { thr, nextafterf(thr, 0.0f), nextafterf(thr, 1.0f), -thr,
                             nextafterf(-thr, 0.0f), nextafterf(-thr, -1.0f) }) {
            float gx = 0.0f, gy = 0.0f, wx = 0.0f, wy = 0.0f;
            // d1 = (1,0), d2 = (0,cross) -> d1x*d2y - d1y*d2x == cross
            const bool got = fwLineIntersect(p1x, p1y, 1.0f, 0.0f, p2x, p2y, 0.0f, cross, thr, &gx, &gy);
            const bool want = ref_lineIntersect(p1x, p1y, 1.0f, 0.0f, p2x, p2y, 0.0f, cross, thr, &wx, &wy);
            EXPECT_EQ(got, want);
            if (want) {
                EXPECT_BITS_EQ(gx, wx);
                EXPECT_BITS_EQ(gy, wy);
            }
        }
    }
}

TEST(NavFwTurnMathUnittest, SmoothBlend)
{
    for (float from : { -4500.0f, -1234.5f, 0.0f, 987.25f, 4500.0f }) {
        for (float to : { -4500.0f, -321.75f, 0.0f, 2200.0f, 4500.0f }) {
            for (float p = 0.0f; p <= 1.0f; p += 1.0f / 512.0f) {
                EXPECT_BITS_EQ(fwSmoothBlend(from, to, p), ref_smoothBlend(from, to, p));
                EXPECT_BITS_EQ(fwSmoothBlend(from, to, p), ref_smoothBlendSFirst(from, to, p));
            }
        }
    }
}

TEST(NavFwTurnMathUnittest, SlewToward)
{
    for (float cur : { -4500.0f, -700.0f, 0.0f, 55.5f, 4500.0f }) {
        for (float target : { -4500.0f, -12.5f, 0.0f, 1800.0f, 4500.0f }) {
            for (float step : { 0.0f, 0.5f, 17.25f, 300.0f, 99999.0f }) {
                EXPECT_BITS_EQ(fwSlewToward(cur, target, step), ref_slewToward(cur, target, step));
            }
        }
    }
}

TEST(NavFwTurnMathUnittest, ArcMaxStep)
{
    for (float phi : { 100.0f, 1500.0f, 3000.0f, 4500.0f }) {
        for (float tEase : { 0.0f, 0.5f, 1.0f, 250.0f, 1400.0f }) {
            for (timeDelta_t dt : { 1, 1000, 20000, 50000, 200000 }) {
                EXPECT_BITS_EQ(fwArcMaxStepCd(phi, tEase, US2S(dt) * 1000.0f),
                               ref_arcMaxStepCd(phi, dt, tEase));
            }
        }
    }
}

TEST(NavFwTurnMathUnittest, OffLeg)
{
    const float px = 12345.0f, py = -6789.0f;
    const float qx = -2200.0f, qy = 33300.0f;
    for (int32_t cd = 0; cd < 36000; cd += 37) {
        float ux, uy;
        fwBearingUnit(cd, &ux, &uy);
        const float legRad = CENTIDEGREES_TO_RADIANS((float)cd);
        EXPECT_BITS_EQ(fwOffLegCm(px, py, qx, qy, ux, uy), ref_offLegCm(px, py, qx, qy, legRad));
    }
}

TEST(NavFwTurnMathUnittest, RollInLead)
{
    for (float v : SPEEDS_CMS) {
        for (float t = 0.0f; t <= 2500.0f; t += 7.0f) {
            EXPECT_BITS_EQ(fwRollInLeadCm(v, t), ref_rollInLeadCm(v, t));
        }
    }
}
