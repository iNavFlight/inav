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

#pragma once

// Betaflight 2026.6 (#15226), on 0-1 over the throttle range with e = thrust_linear / 100; sound only for e <= 1

// Raises low motor outputs, where thrust grows more slowly than the command
static inline float thrustLinearCurve(float u, float e)
{
    const float inv = 1.0f - u;
    return u * (1.0f + e * inv * (1.0f + e * (inv - u)));
}

// Inverse of the curve to second order in e, applied to the throttle so hover stays where it was
static inline float thrustLinearCompensate(float t, float e)
{
    return t * (1.0f - e * (1.0f - t));
}
