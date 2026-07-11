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

#include "common/quaternion.h"

#ifdef USE_MAG

// Empirically, accuracy is ~100% for any accepted result at confidence >=
// 1.2 (Monte Carlo sweep across wide noise/attitude-error ranges); 2.0
// leaves a comfortable margin while still accepting good spins.
#define COMPASS_ORIENTATION_CONFIDENCE_MIN 2.0f

// Detects compass mounting orientation from magnetometer samples paired with
// attitude, collected during a compass calibration spin: the correct mounting
// rotation is the one that makes the implied earth field consistent across
// samples taken at different attitudes. Covers 16 discrete candidates (upright
// / inverted x 45 degree yaw steps) -- not side mounts or arbitrary tilts.

// Discards any buffered samples. Call at the start of a calibration spin.
void compassOrientationBufferReset(void);

// Buffers one raw (pre-offset/gain) magnetometer sample paired with the
// attitude quaternion at the moment it was taken. Drops samples once full.
void compassOrientationBufferPush(const int16_t rawMag[3], const fpQuaternion_t *attitude);

// Runs the candidate search over buffered samples using the given offset/gain.
// Returns true and fills the outputs (decidegrees, same convention as
// compassConfig()->rollDeciDegrees etc.) if confidence >= confidenceMin and
// enough samples were collected; otherwise returns false and leaves them untouched.
bool compassOrientationDetect(const float magZero[3], const int16_t magGain[3],
                               float confidenceMin,
                               int16_t *outRollDD, int16_t *outPitchDD, int16_t *outYawDD,
                               float *outConfidence);

#endif
