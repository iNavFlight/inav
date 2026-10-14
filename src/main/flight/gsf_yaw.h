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

#define GSF_MODEL_COUNT 5

typedef struct gsfModel_s {
    float X[3];         // velocity north and east (m/s), yaw (rad, clockwise from north)
    float P[3][3];
    float S[2][2];      // velocity innovation variance (m/s)^2
    float innov[2];     // velocity innovation (m/s)
    float weight;
} gsfModel_t;

typedef struct gsfState_s {
    gsfModel_t model[GSF_MODEL_COUNT];
    float yaw;          // composite yaw (rad)
    float yawVariance;  // rad^2
    bool fusing;        // velocity fusion has started
    uint8_t clipped;    // models whose weight underflowed at the last fusion
} gsfState_t;

void gsfReset(gsfState_t *gsf);
void gsfPredict(gsfState_t *gsf, const float dvLevel[2], float dYaw, float dt);
bool gsfFuseVelocity(gsfState_t *gsf, float velNorth, float velEast, float velAccuracy);
void gsfUpdateComposite(gsfState_t *gsf);

#ifdef USE_GSF_YAW
#include "common/time.h"

typedef enum {
    GSF_YAW_FUSING  = 1 << 0,   // in flight with GPS: the models are learning
    GSF_YAW_USABLE  = 1 << 1,   // enough manoeuvring seen and the estimate has settled
} gsfYawFlags_e;

typedef struct gsfYawStatus_s {
    float yaw;              // rad, clockwise from north, carried to the present with the gyro
    float yawSigma;         // rad
    float excitation;       // velocity change seen since fusion started (m/s)
    float nis;              // normalised innovation squared of the leading model, filtered (about 2 when consistent)
    uint8_t flags;
} gsfYawStatus_t;

void gsfYawAccumulateImu(float dT);
void gsfYawOnNewGpsData(void);
void taskGsfYaw(timeUs_t currentTimeUs);
const gsfYawStatus_t *gsfYawGetStatus(void);
#endif
