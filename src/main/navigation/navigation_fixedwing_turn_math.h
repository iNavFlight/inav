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

#include <stdbool.h>
#include <stdint.h>

/* Shared turn-geometry primitives of the fixed-wing turn coordinator. The expressions are frozen -
 * the unit test checks them bit-exactly, so operand order and unit idioms are contract, not style. */

float fwBankForRadiusCd(float v, float radiusCm);
int32_t fwRadToBearingCd(float rad);
void fwTangentDir(float alpha, float dirF, float *tx, float *ty);
void fwBearingUnit(int32_t bearingCd, float *ux, float *uy);
void fwPolarOffset(float px, float py, float d, float angRad, float *ox, float *oy);
void fwPerpOffset(float px, float py, float r, float headingRad, float dirF, float *ox, float *oy);
bool fwLineIntersect(float p1x, float p1y, float d1x, float d1y,
                     float p2x, float p2y, float d2x, float d2y,
                     float minAbsCross, float *ox, float *oy);
float fwSmoothBlend(float from, float to, float p);
float fwSlewToward(float cur, float target, float maxStep);
float fwArcMaxStepCd(float phiCd, float tEaseMs, float dtMs);
float fwOffLegCm(float px, float py, float qx, float qy, float ux, float uy);
float fwRollInLeadCm(float v, float tMs);
