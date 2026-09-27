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

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "platform.h"

#include "common/maths.h"

#include "sensors/acceleration.h"

#include "navigation/navigation_fixedwing_turn_math.h"

// Coordinated bank [centideg] holding radius R at groundspeed v: phi = atan(v^2 / (g*R))
float fwBankForRadiusCd(float v, float radiusCm)
{
    return DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(atan2_approx(v * v, GRAVITY_CMSS * radiusCm)));
}

// Course angle [rad] -> wrapped compass bearing [centideg]
int32_t fwRadToBearingCd(float rad)
{
    return wrap_36000(lrintf(DEGREES_TO_CENTIDEGREES(RADIANS_TO_DEGREES(rad))));
}

// Unit tangent of a circle at azimuth alpha, in the direction of travel (dirF = +1 CW / -1 CCW)
void fwTangentDir(float alpha, float dirF, float *tx, float *ty)
{
    *tx = -dirF * sin_approx(alpha);
    *ty = dirF * cos_approx(alpha);
}

// Unit vector along a compass bearing [centideg]; the left normal is (-uy, ux) at the call site
void fwBearingUnit(int32_t bearingCd, float *ux, float *uy)
{
    const float rad = CENTIDEGREES_TO_RADIANS((float)bearingCd);
    *ux = cos_approx(rad);
    *uy = sin_approx(rad);
}

// Point offset by distance d along angle angRad
void fwPolarOffset(float px, float py, float d, float angRad, float *ox, float *oy)
{
    *ox = px + d * cos_approx(angRad);
    *oy = py + d * sin_approx(angRad);
}

// Turn centre: offset r perpendicular to headingRad, toward the turn direction dirF
void fwPerpOffset(float px, float py, float r, float headingRad, float dirF, float *ox, float *oy)
{
    fwPolarOffset(px, py, r, headingRad + dirF * (M_PIf * 0.5f), ox, oy);
}

// Intersection of the lines p1 + t*d1 and p2 + s*d2; false (outputs untouched) when near-parallel.
// Written as the positive test so a NaN cross product also lands on the fallback, as at the call sites
bool fwLineIntersect(float p1x, float p1y, float d1x, float d1y,
                     float p2x, float p2y, float d2x, float d2y,
                     float minAbsCross, float *ox, float *oy)
{
    const float cross = d1x * d2y - d1y * d2x;
    if (!(fabsf(cross) > minAbsCross)) {
        return false;
    }

    const float tt = ((p2x - p1x) * d2y - (p2y - p1y) * d2x) / cross;
    *ox = p1x + tt * d1x;
    *oy = p1y + tt * d1y;
    return true;
}

// NOINLINE (F7/H7 only): measured smaller than letting LTO re-inline it at every call site
NOINLINE float fwSmoothBlend(float from, float to, float p)
{
    const float s = p * p * (3.0f - 2.0f * p);
    return from + (to - from) * s;
}

// Rate-limited approach of cur toward target (NOINLINE for the same reason as fwSmoothBlend)
NOINLINE float fwSlewToward(float cur, float target, float maxStep)
{
    return cur + constrainf(target - cur, -maxStep, maxStep);
}

// Bank slew ceiling [centideg per tick]: the full nominal bank spread over one roll ease window
float fwArcMaxStepCd(float phiCd, float tEaseMs, float dtMs)
{
    return phiCd * dtMs / MAX(tEaseMs, 1.0f);
}

// Signed cross-track offset [cm] of p from the line through q with unit direction (ux, uy)
float fwOffLegCm(float px, float py, float qx, float qy, float ux, float uy)
{
    return (px - qx) * (-uy) + (py - qy) * ux;
}

// Ground distance covered while the roll-in ramp builds the bank (k calibrated in flight)
float fwRollInLeadCm(float v, float tMs)
{
    return 1.5f * v * (tMs / 1000.0f);
}
