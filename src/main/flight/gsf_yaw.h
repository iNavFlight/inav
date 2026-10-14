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

// Rate of the nose's heading, clockwise, from the body rates and earth-up in body axes (rMat's last row)
float gsfYawRate(const float up[3], const float rate[3]);

// GPS delay from how well the IMU's velocity change over 400 ms matches the GPS one, turned by the best single angle
#define GSF_HISTORY_COUNT   40          // 800 ms of 20 ms IMU windows: the longest delay plus the baseline
#define GSF_GPS_HISTORY     8
#define GSF_DELAY_STEP_US   20000
#define GSF_DELAY_STEPS     16          // 0 to 300 ms

typedef struct gsfImuHistory_s {
    uint32_t time[GSF_HISTORY_COUNT];   // us, end of each window
    float vel[GSF_HISTORY_COUNT][2];    // m/s, velocity change summed since boot, in a frame the yaw rate turns with
    uint8_t head;                       // where the next one goes
    uint8_t count;
} gsfImuHistory_t;

typedef struct gsfDelayEstimator_s {
    float sii[GSF_DELAY_STEPS];         // sums over the GPS samples, forgetting the old ones: IMU change squared,
    float sdot[GSF_DELAY_STEPS];        // its dot and cross product with the GPS change,
    float scross[GSF_DELAY_STEPS];
    float sgg;                          // and the GPS change squared
    uint32_t gpsTime[GSF_GPS_HISTORY];
    float gpsVel[GSF_GPS_HISTORY][2];
    uint8_t gpsHead;
    uint8_t gpsCount;
    uint16_t samples;
} gsfDelayEstimator_t;

void gsfHistoryAdd(gsfImuHistory_t *h, uint32_t time, const float vel[2]);
void gsfDelayAddGps(gsfDelayEstimator_t *e, const gsfImuHistory_t *h, uint32_t arrival, const float vel[2]);
bool gsfDelayEstimate(const gsfDelayEstimator_t *e, uint32_t *delayUs);

#ifdef USE_GSF_YAW
#include "common/time.h"

typedef enum {
    GSF_YAW_FUSING          = 1 << 0,   // in flight with GPS: the models are learning
    GSF_YAW_USABLE          = 1 << 1,   // enough manoeuvring seen and the estimate has settled
    GSF_YAW_INCONSISTENT    = 1 << 2,   // the GPS velocity keeps disagreeing with the leading model
    GSF_YAW_DELAY_MEASURED  = 1 << 3,   // the GPS delay comes from this flight, not from the receiver's rate
    GSF_YAW_MAG_COMPARED    = 1 << 4,   // magError holds the compass against the estimate
    GSF_YAW_MAG_DISAGREES   = 1 << 5,   // the compass has stayed off the estimate: the AHRS ignores it when ahrs_gsf_yaw is on
    GSF_YAW_IN_AHRS         = 1 << 6,   // the AHRS heading is following the estimate
    GSF_YAW_RESET           = 1 << 7,   // the AHRS heading was reset to the estimate during this flight
} gsfYawFlags_e;

typedef struct gsfYawStatus_s {
    float yaw;              // rad, clockwise from north, carried to the present with the gyro
    float yawSigma;         // rad
    float excitation;       // velocity change seen since fusion started (m/s), only from sustained accelerations
    float nis;              // normalised innovation squared of the leading model, filtered (about 2 when consistent)
    float magError;         // rad, compass heading minus the estimate, filtered
    uint32_t gpsDelayUs;    // the delay in use
    uint8_t flags;
    uint8_t resets;         // AHRS heading resets this flight
} gsfYawStatus_t;

typedef struct gsfYawAhrsInput_s {
    float yaw;              // rad, clockwise from north, at the present
    float weight;           // 0: not usable as a heading reference
    bool ignoreMag;
    bool resetYaw;          // once: set the AHRS heading to yaw
} gsfYawAhrsInput_t;

void gsfYawAccumulateImu(float dT);
void gsfYawOnNewGpsData(void);
void taskGsfYaw(timeUs_t currentTimeUs);
const gsfYawStatus_t *gsfYawGetStatus(void);
void gsfYawGetAhrsInput(gsfYawAhrsInput_t *in);
#endif
