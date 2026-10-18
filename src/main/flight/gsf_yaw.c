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

/*
 * Yaw from GPS velocity and the IMU, without the compass: a bank of 3-state EKFs (velocity north, east and yaw), each
 * started from a different yaw, combined as a Gaussian sum filter. After Paul Riseborough's EKF-GSF yaw estimator
 * (ArduPilot AP_NavEKF/EKFGSF_yaw.cpp, PX4 ekf2 yaw_estimator); the covariance and Kalman equations are his. Unlike
 * those, each model takes roll and pitch from INAV's AHRS instead of running its own: what reaches it is the
 * acceleration in the horizontal plane and the rate of the nose's heading. The AHRS heading cancels out of both, so
 * the compass reaches the estimate only through the tilt (the GPS inertia compensation uses the full orientation).
 */

#include <string.h>
#include <math.h>

#include "platform.h"

#include "common/maths.h"

#include "flight/gsf_yaw.h"

#define GSF_ACCEL_NOISE     2.0f        // horizontal acceleration noise (m/s^2)
#define GSF_GYRO_NOISE      0.1f        // yaw rate noise (rad/s)
#define GSF_VEL_ACC_MIN     0.5f        // floor on the GPS velocity accuracy (m/s), for stability
#define GSF_WEIGHT_MIN      1e-5f
#define GSF_VAR_MIN         1e-6f

// Bounded whatever the input: a loop would never end once 2 pi no longer changes the float
static float wrapPi(float a)
{
    return a - 2.0f * M_PIf * floorf((a + M_PIf) / (2.0f * M_PIf));
}

float gsfYawRate(const float up[3], const float rate[3])
{
    // Euler yaw rate: (q sin roll + r cos roll) / cos pitch, which a roll about the nose leaves at zero
    const float cosPitchSq = 1.0f - sq(up[0]);
    if (cosPitchSq < 0.01f) {
        // Within 6 deg of vertical the heading is undefined; the rate about the vertical stands in
        return -(up[0] * rate[0] + up[1] * rate[1] + up[2] * rate[2]);
    }
    return -(up[1] * rate[1] + up[2] * rate[2]) / cosPitchSq;
}

static void forceSymmetry(gsfModel_t *m)
{
    const float p01 = 0.5f * (m->P[0][1] + m->P[1][0]);
    const float p02 = 0.5f * (m->P[0][2] + m->P[2][0]);
    const float p12 = 0.5f * (m->P[1][2] + m->P[2][1]);
    m->P[0][1] = m->P[1][0] = p01;
    m->P[0][2] = m->P[2][0] = p02;
    m->P[1][2] = m->P[2][1] = p12;
}

void gsfReset(gsfState_t *gsf)
{
    memset(gsf, 0, sizeof(*gsf));

    const float yawIncrement = 2.0f * M_PIf / GSF_MODEL_COUNT;
    // Half the spacing: the farthest the best model can be from the truth
    gsf->yawVariance = sq(0.5f * yawIncrement);

    for (int i = 0; i < GSF_MODEL_COUNT; i++) {
        gsf->model[i].X[2] = -M_PIf + 0.5f * yawIncrement + i * yawIncrement;
        gsf->model[i].P[2][2] = gsf->yawVariance;
        gsf->model[i].weight = 1.0f / GSF_MODEL_COUNT;
    }
}

// dvLevel: velocity change over dt forward and right of a reference yaw, which dYaw then moves on from
void gsfPredict(gsfState_t *gsf, const float dvLevel[2], float dYaw, float dt)
{
    const float dvxVar = sq(GSF_ACCEL_NOISE * dt);
    const float dvyVar = dvxVar;
    const float dazVar = sq(GSF_GYRO_NOISE * dt);
    const float dvx = dvLevel[0];
    const float dvy = dvLevel[1];

    for (int i = 0; i < GSF_MODEL_COUNT; i++) {
        gsfModel_t *m = &gsf->model[i];

        if (gsf->fusing) {
            const float t2 = sin_approx(m->X[2]);
            const float t3 = cos_approx(m->X[2]);

            m->X[0] += dvx * t3 - dvy * t2;
            m->X[1] += dvx * t2 + dvy * t3;

            const float P00 = m->P[0][0], P01 = m->P[0][1], P02 = m->P[0][2];
            const float P10 = m->P[1][0], P11 = m->P[1][1], P12 = m->P[1][2];
            const float P20 = m->P[2][0], P21 = m->P[2][1], P22 = m->P[2][2];

            const float t4 = dvy * t3;
            const float t5 = dvx * t2;
            const float t6 = t4 + t5;
            const float t8 = P22 * t6;
            const float t7 = P02 - t8;
            const float t9 = dvx * t3;
            const float t11 = dvy * t2;
            const float t10 = t9 - t11;
            const float t12 = dvxVar * t2 * t3;
            const float t13 = t2 * t2;
            const float t14 = t3 * t3;
            const float t15 = P22 * t10;
            const float t16 = P12 + t15;

            m->P[0][0] = MAX(P00 - P20 * t6 + dvxVar * t14 + dvyVar * t13 - t6 * t7, GSF_VAR_MIN);
            m->P[0][1] = P01 + t12 - P21 * t6 + t7 * t10 - dvyVar * t2 * t3;
            m->P[0][2] = t7;
            m->P[1][0] = P10 + t12 + P20 * t10 - t6 * t16 - dvyVar * t2 * t3;
            m->P[1][1] = MAX(P11 + P21 * t10 + dvxVar * t13 + dvyVar * t14 + t10 * t16, GSF_VAR_MIN);
            m->P[1][2] = t16;
            m->P[2][0] = P20 - t8;
            m->P[2][1] = P21 + t15;
            m->P[2][2] = MAX(P22 + dazVar, GSF_VAR_MIN);
            forceSymmetry(m);
        }

        m->X[2] = wrapPi(m->X[2] + dYaw);
    }
}

static bool correctModel(gsfModel_t *m, float velNorth, float velEast, float velObsVar)
{
    m->innov[0] = m->X[0] - velNorth;
    m->innov[1] = m->X[1] - velEast;

    const float P00 = m->P[0][0], P01 = m->P[0][1], P02 = m->P[0][2];
    const float P10 = m->P[1][0], P11 = m->P[1][1], P12 = m->P[1][2];
    const float P20 = m->P[2][0], P21 = m->P[2][1], P22 = m->P[2][2];

    m->S[0][0] = P00 + velObsVar;
    m->S[1][1] = P11 + velObsVar;
    m->S[0][1] = P01;
    m->S[1][0] = P10;

    // Innovations past 5 sigma are scaled down to it, against GPS spikes
    float detS = m->S[0][0] * m->S[1][1] - m->S[0][1] * m->S[1][0];
    if (fabsf(detS) <= 1e-6f) {
        return false;
    }
    detS = 1.0f / detS;
    const float iNN = m->S[1][1] * detS;
    const float iEE = m->S[0][0] * detS;
    const float iNE = -m->S[0][1] * detS;
    const float testRatio = m->innov[0] * (m->innov[0] * iNN + m->innov[1] * iNE) + m->innov[1] * (m->innov[0] * iNE + m->innov[1] * iEE);
    const float innovScale = testRatio > 25.0f ? fast_fsqrtf(25.0f / testRatio) : 1.0f;

    const float t2 = P00 * velObsVar;
    const float t3 = P11 * velObsVar;
    const float t4 = velObsVar * velObsVar;
    const float t5 = P00 * P11;
    const float t9 = P01 * P10;
    const float t6 = t2 + t3 + t4 + t5 - t9;
    if (fabsf(t6) <= 1e-6f) {
        return false;
    }
    const float t7 = 1.0f / t6;
    const float t8 = P11 + velObsVar;
    const float t10 = P00 + velObsVar;

    const float K[3][2] = {
        { -P01 * P10 * t7 + P00 * t7 * t8, -P00 * P01 * t7 + P01 * t7 * t10 },
        { -P10 * P11 * t7 + P10 * t7 * t8, -P01 * P10 * t7 + P11 * t7 * t10 },
        { -P10 * P21 * t7 + P20 * t7 * t8, -P01 * P20 * t7 + P21 * t7 * t10 },
    };

    const float t11 = P00 * P01 * t7;
    const float t15 = P01 * t7 * t10;
    const float t12 = t11 - t15;
    const float t13 = P01 * P10 * t7;
    const float t16 = P00 * t7 * t8;
    const float t14 = t13 - t16;
    const float t19 = t8 * t12 + P01 * t14;
    const float t22 = t10 * t14 + P10 * t12;
    const float t27 = P11 * t7 * t10;
    const float t23 = t13 - t27;
    const float t24 = P10 * P11 * t7;
    const float t26 = P10 * t7 * t8;
    const float t25 = t24 - t26;
    const float t30 = t8 * t23 + P01 * t25;
    const float t33 = t10 * t25 + P10 * t23;
    const float t34 = P01 * P20 * t7;
    const float t38 = P21 * t7 * t10;
    const float t35 = t34 - t38;
    const float t36 = P10 * P21 * t7;
    const float t39 = P20 * t7 * t8;
    const float t37 = t36 - t39;
    const float t42 = t8 * t35 + P01 * t37;
    const float t45 = t10 * t37 + P10 * t35;

    m->P[0][0] = MAX(P00 - t12 * t19 - t14 * t22, GSF_VAR_MIN);
    m->P[0][1] = P01 - t19 * t23 - t22 * t25;
    m->P[0][2] = P02 - t19 * t35 - t22 * t37;
    m->P[1][0] = P10 - t12 * t30 - t14 * t33;
    m->P[1][1] = MAX(P11 - t23 * t30 - t25 * t33, GSF_VAR_MIN);
    m->P[1][2] = P12 - t30 * t35 - t33 * t37;
    m->P[2][0] = P20 - t12 * t42 - t14 * t45;
    m->P[2][1] = P21 - t23 * t42 - t25 * t45;
    m->P[2][2] = MAX(P22 - t35 * t42 - t37 * t45, GSF_VAR_MIN);
    forceSymmetry(m);

    for (int obs = 0; obs < 2; obs++) {
        for (int row = 0; row < 3; row++) {
            m->X[row] -= K[row][obs] * m->innov[obs] * innovScale;
        }
    }
    m->X[2] = wrapPi(m->X[2]);

    return true;
}

static float gaussianDensity(const gsfModel_t *m)
{
    const float det = m->S[0][0] * m->S[1][1] - m->S[0][1] * m->S[1][0];
    const float detInv = 1.0f / MAX(det, 1e-12f);
    const float inv00 = detInv * m->S[1][1];
    const float inv11 = detInv * m->S[0][0];
    const float inv01 = -detInv * m->S[0][1];
    const float inv10 = -detInv * m->S[1][0];
    const float tv0 = inv00 * m->innov[0] + inv01 * m->innov[1];
    const float tv1 = inv10 * m->innov[0] + inv11 * m->innov[1];
    const float normDist = tv0 * m->innov[0] + tv1 * m->innov[1];

    return expf(-0.5f * normDist) * fast_fsqrtf(detInv) / (2.0f * M_PIf);
}

// Returns false when the update was skipped
bool gsfFuseVelocity(gsfState_t *gsf, float velNorth, float velEast, float velAccuracy)
{
    const float velObsVar = sq(MAX(velAccuracy, GSF_VEL_ACC_MIN));

    if (!gsf->fusing) {
        // In-flight alignment: velocities from the first sample, yaws spread over the circle
        gsfReset(gsf);
        for (int i = 0; i < GSF_MODEL_COUNT; i++) {
            gsf->model[i].X[0] = velNorth;
            gsf->model[i].X[1] = velEast;
            gsf->model[i].P[0][0] = velObsVar;
            gsf->model[i].P[1][1] = velObsVar;
        }
        gsf->fusing = true;
        return true;
    }

    for (int i = 0; i < GSF_MODEL_COUNT; i++) {
        if (!correctModel(&gsf->model[i], velNorth, velEast, velObsVar)) {
            return false;
        }
    }

    float newWeight[GSF_MODEL_COUNT];
    float totalWeight = 0.0f;
    gsf->clipped = 0;
    for (int i = 0; i < GSF_MODEL_COUNT; i++) {
        newWeight[i] = gaussianDensity(&gsf->model[i]) * gsf->model[i].weight;
        if (newWeight[i] < GSF_WEIGHT_MIN) {
            gsf->clipped++;
            newWeight[i] = GSF_WEIGHT_MIN;
        }
        totalWeight += newWeight[i];
    }

    // Every model so far off that its weight underflowed: start again
    if (gsf->clipped == GSF_MODEL_COUNT) {
        gsfReset(gsf);
        return false;
    }

    for (int i = 0; i < GSF_MODEL_COUNT; i++) {
        gsf->model[i].weight = newWeight[i] / totalWeight;
    }

    return true;
}

void gsfUpdateComposite(gsfState_t *gsf)
{
    float sinSum = 0.0f;
    float cosSum = 0.0f;
    for (int i = 0; i < GSF_MODEL_COUNT; i++) {
        sinSum += gsf->model[i].weight * sin_approx(gsf->model[i].X[2]);
        cosSum += gsf->model[i].weight * cos_approx(gsf->model[i].X[2]);
    }
    gsf->yaw = atan2_approx(sinSum, cosSum);

    float variance = 0.0f;
    for (int i = 0; i < GSF_MODEL_COUNT; i++) {
        variance += gsf->model[i].weight * (gsf->model[i].P[2][2] + sq(wrapPi(gsf->model[i].X[2] - gsf->yaw)));
    }
    gsf->yawVariance = variance;
}

#define GSF_DELAY_FORGET        0.998f      // per GPS sample: about 50 s at 10 Hz
#define GSF_DELAY_BASELINE_US   400000      // long enough for a turn to swing the velocity change visibly
#define GSF_DELAY_MIN_SAMPLES   100
#define GSF_DELAY_SIDE_STEPS    3           // the cost's curvature is taken 60 ms either side
#define GSF_DELAY_OVERLAP       4.0f        // successive samples share most of a baseline: about 1 in 4 is independent
#define GSF_DELAY_MAX_SIGMA     1.25f       // steps: 25 ms

void gsfHistoryAdd(gsfImuHistory_t *h, uint32_t time, const float vel[2])
{
    h->time[h->head] = time;
    h->vel[h->head][0] = vel[0];
    h->vel[h->head][1] = vel[1];
    h->head = (h->head + 1) % GSF_HISTORY_COUNT;
    if (h->count < GSF_HISTORY_COUNT) {
        h->count++;
    }
}

// Interpolated between the windows around the time; false outside the history
static bool historyVelAt(const gsfImuHistory_t *h, uint32_t time, float vel[2])
{
    int newer = -1;
    for (int n = 0; n < h->count; n++) {
        const int i = (h->head + GSF_HISTORY_COUNT - 1 - n) % GSF_HISTORY_COUNT;
        const int32_t after = (int32_t)(time - h->time[i]);
        if (after >= 0) {
            if (newer < 0) {
                if (after > 0) {
                    return false;
                }
                vel[0] = h->vel[i][0];
                vel[1] = h->vel[i][1];
                return true;
            }
            const float f = (float)after / (float)(int32_t)(h->time[newer] - h->time[i]);
            vel[0] = h->vel[i][0] + f * (h->vel[newer][0] - h->vel[i][0]);
            vel[1] = h->vel[i][1] + f * (h->vel[newer][1] - h->vel[i][1]);
            return true;
        }
        newer = i;
    }
    return false;
}

void gsfDelayAddGps(gsfDelayEstimator_t *e, const gsfImuHistory_t *h, uint32_t arrival, const float vel[2])
{
    // The earlier sample nearest one baseline back
    int32_t bestErr = GSF_DELAY_BASELINE_US / 2;
    float prevVel[2] = { 0.0f, 0.0f };
    uint32_t prevArrival = 0;
    bool found = false;
    for (int n = 0; n < e->gpsCount; n++) {
        const int i = (e->gpsHead + GSF_GPS_HISTORY - 1 - n) % GSF_GPS_HISTORY;
        const int32_t err = ABS((int32_t)(arrival - e->gpsTime[i]) - GSF_DELAY_BASELINE_US);
        if (err < bestErr) {
            bestErr = err;
            prevVel[0] = e->gpsVel[i][0];
            prevVel[1] = e->gpsVel[i][1];
            prevArrival = e->gpsTime[i];
            found = true;
        }
    }

    e->gpsTime[e->gpsHead] = arrival;
    e->gpsVel[e->gpsHead][0] = vel[0];
    e->gpsVel[e->gpsHead][1] = vel[1];
    e->gpsHead = (e->gpsHead + 1) % GSF_GPS_HISTORY;
    if (e->gpsCount < GSF_GPS_HISTORY) {
        e->gpsCount++;
    }

    float from[2], to[2];
    const uint32_t longest = (GSF_DELAY_STEPS - 1) * GSF_DELAY_STEP_US;
    if (!found || !historyVelAt(h, arrival, to) || !historyVelAt(h, prevArrival - longest, from)) {
        return;
    }

    const float gps[2] = { vel[0] - prevVel[0], vel[1] - prevVel[1] };
    e->sgg = e->sgg * GSF_DELAY_FORGET + sq(gps[0]) + sq(gps[1]);
    for (int j = 0; j < GSF_DELAY_STEPS; j++) {
        const uint32_t delay = j * GSF_DELAY_STEP_US;
        historyVelAt(h, arrival - delay, to);
        historyVelAt(h, prevArrival - delay, from);
        const float imu[2] = { to[0] - from[0], to[1] - from[1] };
        e->sii[j] = e->sii[j] * GSF_DELAY_FORGET + sq(imu[0]) + sq(imu[1]);
        e->sdot[j] = e->sdot[j] * GSF_DELAY_FORGET + imu[0] * gps[0] + imu[1] * gps[1];
        e->scross[j] = e->scross[j] * GSF_DELAY_FORGET + imu[0] * gps[1] - imu[1] * gps[0];
    }
    if (e->samples < UINT16_MAX) {
        e->samples++;
    }
}

bool gsfDelayEstimate(const gsfDelayEstimator_t *e, uint32_t *delayUs)
{
    if (e->samples < GSF_DELAY_MIN_SAMPLES) {
        return false;
    }

    // Residual after the best rotation, for each delay
    float cost[GSF_DELAY_STEPS];
    int best = 0;
    for (int j = 0; j < GSF_DELAY_STEPS; j++) {
        cost[j] = MAX(e->sii[j] + e->sgg - 2.0f * fast_fsqrtf(sq(e->sdot[j]) + sq(e->scross[j])), 0.0f);
        if (cost[j] < cost[best]) {
            best = j;
        }
    }
    // At the end of the range the delay may be longer still
    if (best == GSF_DELAY_STEPS - 1) {
        return false;
    }

    // As in least squares, the delay's variance is the residual per independent sample over the cost's curvature: it
    // shrinks with every turn, however noisy the GPS
    const int side = GSF_DELAY_SIDE_STEPS;
    float curvature;    // per step squared
    if (best >= side && best + side < GSF_DELAY_STEPS) {
        curvature = (cost[best - side] + cost[best + side] - 2.0f * cost[best]) / (2.0f * side * side);
    } else if (best + side < GSF_DELAY_STEPS) {
        curvature = (cost[best + side] - cost[best]) / (side * side);
    } else {
        curvature = (cost[best - side] - cost[best]) / (side * side);
    }
    const float independent = MIN(e->samples, 1.0f / (1.0f - GSF_DELAY_FORGET)) / GSF_DELAY_OVERLAP;
    if (curvature <= 0.0f || cost[best] > sq(GSF_DELAY_MAX_SIGMA) * independent * curvature) {
        return false;
    }

    float offset = 0.0f;
    if (best > 0) {
        const float curvature = cost[best - 1] - 2.0f * cost[best] + cost[best + 1];
        if (curvature > 0.0f) {
            offset = constrainf(0.5f * (cost[best - 1] - cost[best + 1]) / curvature, -0.5f, 0.5f);
        }
    }
    *delayUs = lrintf((best + offset) * GSF_DELAY_STEP_US);
    return true;
}

#if defined(USE_GSF_YAW)

#include "build/build_config.h"

#include "common/time.h"

#include "drivers/time.h"

#include "fc/runtime_config.h"

#include "flight/imu.h"

#include "io/gps.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#include "sensors/compass.h"

#define GSF_WINDOW_COUNT        16          // 320 ms of 20 ms IMU windows, room for the GPS delay
#define GSF_DELAY_MAX_US        280000u     // what the windows can hold back
#define GSF_DELAY_DEFAULT_US    100000      // before the receiver's rate is known
#define GSF_DELAY_PROCESSING_US 20000       // on top of one GPS interval: 120 ms at 10 Hz and 220 ms at 5 Hz, ArduPilot's u-blox lags
#define GSF_VEL_ACCURACY        0.5f        // m/s
#define GSF_MIN_SATS            6
#define GSF_MAX_EPH_CM          500
#define GSF_STEADY_RATE         DEGREES_TO_RADIANS(90.0f)   // faster rotation leaves the AHRS tilt less sure
#define GSF_EXCITATION_BLOCK_S  1.0f        // velocity change counted per second and only past 1 m/s, so vibration and
#define GSF_EXCITATION_MIN      1.0f        // tilt errors do not add up to manoeuvres in straight flight
#define GSF_USABLE_EXCITATION   25.0f       // m/s: a 90 deg turn at 16 m/s
#define GSF_USABLE_SIGMA        DEGREES_TO_RADIANS(15.0f)
#define GSF_USABLE_SETTLE_US    3000000
#define GSF_NIS_FILTER          0.1f
#define GSF_NIS_INCONSISTENT    6.0f        // chi-square with 2 degrees of freedom, 95 %
#define GSF_NIS_CONSISTENT      3.0f
#define GSF_INCONSISTENT_US     10000000    // then it learns again from scratch
#define GSF_WINDOW_US           20000       // the task period: a GPS sample is fused within one window of its time
#define GSF_FUSION_MIN_US       80000       // at most ~10 fusions a second, whatever the receiver's rate
#define GSF_MAG_ERROR_FILTER    0.02f       // about 1 s at 50 Hz
#define GSF_MAG_ERROR_MIN       DEGREES_TO_RADIANS(25.0f)
#define GSF_MAG_DISAGREE_US     3000000
#define GSF_MAG_AGREE_US        10000000
#define GSF_AHRS_FULL_SIGMA     DEGREES_TO_RADIANS(5.0f)
#define GSF_RESET_ERROR        DEGREES_TO_RADIANS(45.0f)   // the gentle correction is slow from here, and nil from 180
#define GSF_RESET_AFTER_US      1000000
#define GSF_RESETS_MAX          2           // per flight, as ArduPilot's EK3_GSF_RST_MAX
#define GSF_IMU_DT_MAX          0.1f        // s: a longer step is a stalled loop or a stale timestamp, not motion

typedef struct gsfWindow_s {
    float dv[2];        // m/s, forward and right of the yaw at the window's start
    float dYaw;         // rad
    float dt;           // s
    float maxRateSq;    // (rad/s)^2
    timeUs_t end;
} gsfWindow_t;

STATIC_FASTRAM gsfState_t gsfState;
STATIC_FASTRAM gsfWindow_t gsfAccumulating;
STATIC_FASTRAM gsfWindow_t gsfWindows[GSF_WINDOW_COUNT];
STATIC_FASTRAM uint8_t gsfWindowHead;      // where the next window goes
STATIC_FASTRAM uint8_t gsfWindowCount;
STATIC_FASTRAM gsfImuHistory_t gsfHistory;
STATIC_FASTRAM gsfDelayEstimator_t gsfDelay;
STATIC_FASTRAM float gsfHistoryVel[2];     // m/s, velocity change summed since boot, in a frame the gyro turns
STATIC_FASTRAM float gsfHistoryYaw;        // rad, that frame's yaw
STATIC_FASTRAM float gsfBlockDv[2];
STATIC_FASTRAM float gsfBlockDt;
STATIC_FASTRAM gsfYawStatus_t gsfStatus;
STATIC_FASTRAM float gsfGpsVel[2];         // m/s, north and east
STATIC_FASTRAM timeUs_t gsfGpsArrival;
STATIC_FASTRAM timeUs_t gsfGpsTakenAt;
STATIC_FASTRAM timeUs_t gsfGpsLastArrival;
STATIC_FASTRAM float gsfGpsIntervalUs;
STATIC_FASTRAM bool gsfGpsPending;
STATIC_FASTRAM bool gsfGpsPendingDelay;
STATIC_FASTRAM timeUs_t gsfSettledSince;
STATIC_FASTRAM timeUs_t gsfLastFusion;
STATIC_FASTRAM timeUs_t gsfInconsistentSince;
STATIC_FASTRAM timeUs_t gsfMagDisagreeSince;
STATIC_FASTRAM timeUs_t gsfMagAgreeSince;
STATIC_FASTRAM timeUs_t gsfFarSince;
STATIC_FASTRAM bool gsfMagDisagrees;
STATIC_FASTRAM bool gsfResetRequested;
STATIC_FASTRAM bool gsfAirborne;
STATIC_FASTRAM bool gsfWasArmed;
STATIC_FASTRAM bool gsfWasTailsitter;
STATIC_FASTRAM bool gsfActive;             // the task runs: without it the accumulator would grow for ever
STATIC_FASTRAM bool gsfDelayMeasured;
STATIC_FASTRAM uint32_t gsfDelayMeasuredUs;

// Every AHRS update: out of line, since the AHRS is FAST_CODE on some targets (RP2350) and this is not time critical
NOINLINE void gsfYawAccumulateImu(float dT)
{
    if (!gsfActive || dT <= 0.0f || dT > GSF_IMU_DT_MAX) {
        return;
    }
    const fpVector3_t *acc = &imuMeasuredAccelBF;       // cm/s^2
    const fpVector3_t *rate = &imuMeasuredRotationBF;   // rad/s

    // rMat's last row is earth-up in body axes: roll and pitch only, no heading in it
    const float yawRate = gsfYawRate(rMat[2], rate->v);

    // Acceleration forward and right of the nose in the horizontal plane; the AHRS heading cancels between the two
    const float noseN = rMat[0][0];
    const float noseW = rMat[1][0];
    const float noseLen = fast_fsqrtf(sq(noseN) + sq(noseW));
    if (noseLen > 0.1f) {
        const float accN = rMat[0][0] * acc->x + rMat[0][1] * acc->y + rMat[0][2] * acc->z;
        const float accW = rMat[1][0] * acc->x + rMat[1][1] * acc->y + rMat[1][2] * acc->z;
        const float forward = (accN * noseN + accW * noseW) / noseLen;
        const float right = (accN * noseW - accW * noseN) / noseLen;

        // Into the frame of the window's start, which the yaw has turned from since
        const float c = cos_approx(gsfAccumulating.dYaw);
        const float s = sin_approx(gsfAccumulating.dYaw);
        gsfAccumulating.dv[0] += (forward * c - right * s) * dT * 0.01f;
        gsfAccumulating.dv[1] += (forward * s + right * c) * dT * 0.01f;
    }

    gsfAccumulating.dYaw += yawRate * dT;
    gsfAccumulating.dt += dT;
    gsfAccumulating.maxRateSq = MAX(gsfAccumulating.maxRateSq, vectorNormSquared(rate));
}

void gsfYawOnNewGpsData(void)
{
    if (!STATE(GPS_FIX) || !gpsSol.flags.validVelNE) {
        return;
    }

    // The receiver's rate gives the delay until one is measured
    const timeUs_t now = micros();
    if (gsfGpsLastArrival) {
        const timeDelta_t interval = cmpTimeUs(now, gsfGpsLastArrival);
        if (interval > 0 && interval < 1000000) {
            gsfGpsIntervalUs += (gsfGpsIntervalUs > 0.0f ? 0.1f : 1.0f) * (interval - gsfGpsIntervalUs);
        }
    }
    gsfGpsLastArrival = now;

    if (gpsSol.numSat < GSF_MIN_SATS || gpsSol.eph > GSF_MAX_EPH_CM) {
        return;
    }

    gsfGpsVel[0] = gpsSol.velNED[X] * 0.01f;
    gsfGpsVel[1] = gpsSol.velNED[Y] * 0.01f;
    gsfGpsArrival = now;
    gsfGpsTakenAt = now - gsfStatus.gpsDelayUs;
    gsfGpsPending = true;
    gsfGpsPendingDelay = true;
}

static void updateNis(void)
{
    int lead = 0;
    for (int i = 1; i < GSF_MODEL_COUNT; i++) {
        if (gsfState.model[i].weight > gsfState.model[lead].weight) {
            lead = i;
        }
    }

    const gsfModel_t *m = &gsfState.model[lead];
    const float det = m->S[0][0] * m->S[1][1] - m->S[0][1] * m->S[1][0];
    if (det > 1e-9f) {
        const float nis = (m->innov[0] * (m->innov[0] * m->S[1][1] - m->innov[1] * m->S[0][1]) +
                           m->innov[1] * (m->innov[1] * m->S[0][0] - m->innov[0] * m->S[1][0])) / det;
        gsfStatus.nis += GSF_NIS_FILTER * (nis - gsfStatus.nis);
    }
}

// A timer that runs while cond holds: true once it has held for us. 64-bit: cmpTimeUs would wrap after 35 min
static bool heldFor(timeUs_t *since, bool cond, timeUs_t now, timeUs_t us)
{
    if (!cond) {
        *since = 0;
        return false;
    }
    if (*since == 0) {
        *since = now ? now : 1;
    }
    return now - *since >= us;
}

static void closeWindow(timeUs_t now)
{
    gsfAccumulating.end = now;
    gsfWindows[gsfWindowHead] = gsfAccumulating;
    gsfWindowHead = (gsfWindowHead + 1) % GSF_WINDOW_COUNT;
    if (gsfWindowCount < GSF_WINDOW_COUNT) {
        gsfWindowCount++;
    }

    // The same change in the gyro-turned frame, for the delay and the manoeuvres
    const float c = cos_approx(gsfHistoryYaw);
    const float s = sin_approx(gsfHistoryYaw);
    const float dvN = gsfAccumulating.dv[0] * c - gsfAccumulating.dv[1] * s;
    const float dvE = gsfAccumulating.dv[0] * s + gsfAccumulating.dv[1] * c;
    gsfHistoryVel[0] += dvN;
    gsfHistoryVel[1] += dvE;
    gsfHistoryYaw = wrapPi(gsfHistoryYaw + gsfAccumulating.dYaw);
    gsfHistoryAdd(&gsfHistory, now, gsfHistoryVel);

    gsfBlockDv[0] += dvN;
    gsfBlockDv[1] += dvE;
    gsfBlockDt += gsfAccumulating.dt;
    if (gsfBlockDt >= GSF_EXCITATION_BLOCK_S) {
        const float dv = fast_fsqrtf(sq(gsfBlockDv[0]) + sq(gsfBlockDv[1]));
        if (gsfState.fusing && dv > GSF_EXCITATION_MIN) {
            gsfStatus.excitation += dv;
        }
        gsfBlockDv[0] = gsfBlockDv[1] = gsfBlockDt = 0.0f;
    }

    memset(&gsfAccumulating, 0, sizeof(gsfAccumulating));
}

// A measured delay is kept once it has passed the confidence test, so the value does not toggle with the manoeuvres
static void updateGpsDelay(bool inFlight)
{
    if (gsfGpsPendingDelay) {
        gsfGpsPendingDelay = false;
        if (inFlight) {
            gsfDelayAddGps(&gsfDelay, &gsfHistory, gsfGpsArrival, gsfGpsVel);
            uint32_t delay;
            if (gsfDelayEstimate(&gsfDelay, &delay)) {
                gsfDelayMeasuredUs = delay;
                gsfDelayMeasured = true;
            }
        }
    }

    uint32_t delay;
    if (gsfDelayMeasured) {
        gsfStatus.flags |= GSF_YAW_DELAY_MEASURED;
        delay = gsfDelayMeasuredUs;
    } else {
        delay = gsfGpsIntervalUs > 0.0f ? lrintf(gsfGpsIntervalUs) + GSF_DELAY_PROCESSING_US : GSF_DELAY_DEFAULT_US;
    }
    gsfStatus.gpsDelayUs = MIN(delay, GSF_DELAY_MAX_US);
}

// The compass against the estimate: dropped after staying off it, back after staying close for longer. False when
// there was nothing to compare
static bool checkCompass(timeUs_t now, bool usable, bool wasCompared)
{
    float compassHeading;
    if (!usable || !imuGetCompassHeading(&compassHeading)) {
        gsfMagDisagreeSince = gsfMagAgreeSince = 0;
        return false;
    }

    const float error = wrapPi(compassHeading - gsfStatus.yaw);
    gsfStatus.magError = wasCompared ? wrapPi(gsfStatus.magError + GSF_MAG_ERROR_FILTER * wrapPi(error - gsfStatus.magError)) : error;

    const float limit = MAX(GSF_MAG_ERROR_MIN, 3.0f * gsfStatus.yawSigma);
    const float size = fabsf(gsfStatus.magError);
    if (heldFor(&gsfMagDisagreeSince, size > limit, now, GSF_MAG_DISAGREE_US)) {
        gsfMagDisagrees = true;
    }
    if (heldFor(&gsfMagAgreeSince, size < 0.5f * limit, now, GSF_MAG_AGREE_US)) {
        gsfMagDisagrees = false;
    }
    return true;
}

// An AHRS heading far from a settled estimate, with no compass to hold it, is reset to it
static void checkAhrsHeading(timeUs_t now, bool usable)
{
#ifdef USE_MAG
    const bool compassHeld = sensors(SENSOR_MAG) && compassIsHealthy() && !gsfMagDisagrees;
#else
    const bool compassHeld = false;
#endif
    bool far = false;
    if (imuConfig()->gsf_yaw && usable && !compassHeld && gsfStatus.resets < GSF_RESETS_MAX) {
        const float error = wrapPi(DECIDEGREES_TO_RADIANS(attitude.values.yaw) - gsfStatus.yaw);
        far = fabsf(error) > GSF_RESET_ERROR;
    }
    if (heldFor(&gsfFarSince, far, now, GSF_RESET_AFTER_US)) {
        gsfResetRequested = true;
        gsfStatus.resets++;
        gsfFarSince = 0;
    }
}

void taskGsfYaw(timeUs_t currentTimeUs)
{
    // Zeroed at boot, which would read as a certain heading of 0
    if (gsfState.model[0].weight == 0.0f) {
        gsfReset(&gsfState);
    }
    gsfActive = true;

    // A tailsitter's transition turns the nose by 90 deg with no gyro yaw: the window and the models start over
    const bool tailsitter = STATE(TAILSITTER);
    if (tailsitter != gsfWasTailsitter) {
        gsfWasTailsitter = tailsitter;
        memset(&gsfAccumulating, 0, sizeof(gsfAccumulating));
        gsfReset(&gsfState);
    }

    if (gsfAccumulating.dt > 0.0f) {
        closeWindow(currentTimeUs);
    }

    // Disarmed, the compass is the only heading there is; the next flight starts trusting it, as INAV always has
    const bool armed = ARMING_FLAG(ARMED);
    if (!armed) {
        gsfMagDisagrees = false;
        gsfMagDisagreeSince = gsfMagAgreeSince = 0;
    }
    if (armed && !gsfWasArmed) {
        gsfStatus.resets = 0;
    }
    gsfWasArmed = armed;

    // Airplanes only, in flight (the wheels and the takeoff acceleration mislead it), from the first flight detection
    // to the landing detection: isFixedWingFlying alone drops with the throttle under cruise
    if (!armed || STATE(LANDING_DETECTED) || !STATE(AIRPLANE)) {
        gsfAirborne = false;
    } else if (isFixedWingFlying()) {
        gsfAirborne = true;
    }
    const bool inFlight = gsfAirborne;
    if (!inFlight && gsfState.fusing) {
        gsfState.fusing = false;
    }

    const uint8_t previous = gsfStatus.flags;
    gsfStatus.flags = 0;
    updateGpsDelay(inFlight);

    // Step the filter up to the GPS delay behind the present
    const gsfWindow_t *last = NULL;
    while (gsfWindowCount > 0) {
        const gsfWindow_t *w = &gsfWindows[(gsfWindowHead + GSF_WINDOW_COUNT - gsfWindowCount) % GSF_WINDOW_COUNT];
        if (cmpTimeUs(currentTimeUs, w->end) < (timeDelta_t)gsfStatus.gpsDelayUs) {
            break;
        }
        gsfPredict(&gsfState, w->dv, w->dYaw, w->dt);
        last = w;
        gsfWindowCount--;
    }

    // The latest GPS sample, fused once the filter has reached the time it was taken
    if (gsfGpsPending && last && cmpTimeUs(last->end, gsfGpsTakenAt) >= -GSF_WINDOW_US) {
        gsfGpsPending = false;
        const bool spaced = !gsfState.fusing || gsfGpsTakenAt - gsfLastFusion >= GSF_FUSION_MIN_US;
        if (inFlight && last->maxRateSq < sq(GSF_STEADY_RATE) && spaced) {
            if (!gsfState.fusing) {
                gsfStatus.excitation = 0.0f;
                gsfStatus.nis = 2.0f;
            }
            if (gsfFuseVelocity(&gsfState, gsfGpsVel[0], gsfGpsVel[1], GSF_VEL_ACCURACY)) {
                updateNis();
            }
            gsfLastFusion = gsfGpsTakenAt;
        }
    }

    // A leading model the GPS keeps disagreeing with: past the limit for long, start over, and the compass it may
    // have excluded gets its trust back
    bool inconsistent = gsfState.fusing && gsfStatus.nis > ((previous & GSF_YAW_INCONSISTENT) ? GSF_NIS_CONSISTENT : GSF_NIS_INCONSISTENT);
    if (heldFor(&gsfInconsistentSince, inconsistent, currentTimeUs, GSF_INCONSISTENT_US)) {
        gsfReset(&gsfState);
        gsfInconsistentSince = 0;
        gsfMagDisagrees = false;
        inconsistent = false;
    }

    gsfUpdateComposite(&gsfState);

    // Carry the delayed yaw to the present with the gyro
    float yaw = gsfState.yaw + gsfAccumulating.dYaw;
    for (int n = 0; n < gsfWindowCount; n++) {
        yaw += gsfWindows[(gsfWindowHead + GSF_WINDOW_COUNT - 1 - n) % GSF_WINDOW_COUNT].dYaw;
    }
    yaw = wrapPi(yaw);
    gsfStatus.yaw = yaw < 0.0f ? yaw + 2.0f * M_PIf : yaw;
    gsfStatus.yawSigma = fast_fsqrtf(gsfState.yawVariance);

    // Its own variance is not proof on its own: it also has to have seen manoeuvres, and stay settled a while
    const bool settled = gsfState.fusing && !inconsistent && gsfStatus.excitation >= GSF_USABLE_EXCITATION &&
        gsfStatus.yawSigma < GSF_USABLE_SIGMA;
    const bool usable = heldFor(&gsfSettledSince, settled, currentTimeUs, GSF_USABLE_SETTLE_US);

    if (checkCompass(currentTimeUs, usable, previous & GSF_YAW_MAG_COMPARED)) {
        gsfStatus.flags |= GSF_YAW_MAG_COMPARED;
    }
    checkAhrsHeading(currentTimeUs, usable);

    if (gsfState.fusing) {
        gsfStatus.flags |= GSF_YAW_FUSING;
    }
    if (usable) {
        gsfStatus.flags |= GSF_YAW_USABLE;
    }
    if (inconsistent) {
        gsfStatus.flags |= GSF_YAW_INCONSISTENT;
    }
    if (gsfMagDisagrees) {
        gsfStatus.flags |= GSF_YAW_MAG_DISAGREES;
    }
    if (imuConfig()->gsf_yaw && usable) {
        gsfStatus.flags |= GSF_YAW_IN_AHRS;
    }
    if (gsfStatus.resets) {
        gsfStatus.flags |= GSF_YAW_RESET;
    }
}

const gsfYawStatus_t *gsfYawGetStatus(void)
{
    return &gsfStatus;
}

// Every AHRS update with ahrs_gsf_yaw on: out of line, as the accumulation
NOINLINE void gsfYawGetAhrsInput(gsfYawAhrsInput_t *in)
{
    const bool enabled = imuConfig()->gsf_yaw;
    in->yaw = wrapPi(gsfStatus.yaw + gsfAccumulating.dYaw);
    in->weight = 0.0f;
    if (enabled && (gsfStatus.flags & GSF_YAW_USABLE)) {
        in->weight = constrainf(scaleRangef(gsfStatus.yawSigma, GSF_AHRS_FULL_SIGMA, GSF_USABLE_SIGMA, 1.0f, 0.0f), 0.0f, 1.0f);
    }
    in->ignoreMag = enabled && gsfMagDisagrees;
    in->resetYaw = gsfResetRequested;
    gsfResetRequested = false;
}

#endif
