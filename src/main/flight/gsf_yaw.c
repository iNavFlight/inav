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
 * acceleration in the horizontal plane and the yaw rate about the vertical.
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

static float wrapPi(float a)
{
    while (a > M_PIf) {
        a -= 2.0f * M_PIf;
    }
    while (a < -M_PIf) {
        a += 2.0f * M_PIf;
    }
    return a;
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

// dvLevel: velocity change over dt in the horizontal plane, forward and right of a reference yaw that dYaw moves on
// from; each model turns it by its own yaw
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
    const float iNE = m->S[0][1] * detS;
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

#if defined(USE_GSF_YAW)

#include "build/build_config.h"

#include "common/time.h"

#include "drivers/time.h"

#include "fc/runtime_config.h"

#include "flight/imu.h"

#include "io/gps.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#define GSF_WINDOW_COUNT        16          // 320 ms of 20 ms IMU windows, room for the GPS delay
#define GSF_GPS_DELAY_US        100000      // from the GPS fix to its message here; to be measured on real receivers
#define GSF_VEL_ACCURACY        0.5f        // m/s
#define GSF_MIN_SATS            6
#define GSF_MAX_EPH_CM          500
#define GSF_STEADY_RATE         DEGREES_TO_RADIANS(90.0f)   // faster rotation leaves the AHRS tilt less sure
#define GSF_USABLE_EXCITATION   25.0f       // m/s of velocity change: a 90 deg turn at 16 m/s
#define GSF_USABLE_SIGMA        DEGREES_TO_RADIANS(15.0f)
#define GSF_USABLE_SETTLE_US    3000000
#define GSF_NIS_FILTER          0.1f
#define GSF_WINDOW_US           20000       // the task period: a GPS sample is fused within one window of its time
#define GSF_FUSION_MIN_US       80000       // at most ~10 fusions a second, whatever the receiver's rate

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
STATIC_FASTRAM gsfYawStatus_t gsfStatus;
STATIC_FASTRAM float gsfGpsVel[2];         // m/s, north and east
STATIC_FASTRAM timeUs_t gsfGpsTakenAt;
STATIC_FASTRAM bool gsfGpsPending;
STATIC_FASTRAM timeUs_t gsfSettledSince;
STATIC_FASTRAM timeUs_t gsfLastFusion;
STATIC_FASTRAM bool gsfAirborne;

// Every AHRS update: kept out of line so LTO does not pull it into the realtime loop's fast memory
NOINLINE void gsfYawAccumulateImu(float dT)
{
    const fpVector3_t *acc = &imuMeasuredAccelBF;       // cm/s^2
    const fpVector3_t *rate = &imuMeasuredRotationBF;   // rad/s

    // rMat's last row is roll and pitch only, so the compass never reaches the yaw rate; up is +z, yaw clockwise
    const float yawRate = -(rMat[2][0] * rate->x + rMat[2][1] * rate->y + rMat[2][2] * rate->z);

    // Acceleration forward and right of the nose in the horizontal plane. rMat's earth frame is north, west, up, and
    // its heading (the compass-corrected one) cancels between the acceleration and the nose
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
    if (!STATE(GPS_FIX) || !gpsSol.flags.validVelNE || gpsSol.numSat < GSF_MIN_SATS || gpsSol.eph > GSF_MAX_EPH_CM) {
        return;
    }

    gsfGpsVel[0] = gpsSol.velNED[X] * 0.01f;
    gsfGpsVel[1] = gpsSol.velNED[Y] * 0.01f;
    gsfGpsTakenAt = micros() - GSF_GPS_DELAY_US;
    gsfGpsPending = true;
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

void taskGsfYaw(timeUs_t currentTimeUs)
{
    // Zeroed at boot, which would read as a certain heading of 0
    if (gsfState.model[0].weight == 0.0f) {
        gsfReset(&gsfState);
    }

    if (gsfAccumulating.dt > 0.0f) {
        gsfAccumulating.end = currentTimeUs;
        gsfWindows[gsfWindowHead] = gsfAccumulating;
        gsfWindowHead = (gsfWindowHead + 1) % GSF_WINDOW_COUNT;
        if (gsfWindowCount < GSF_WINDOW_COUNT) {
            gsfWindowCount++;
        }
        memset(&gsfAccumulating, 0, sizeof(gsfAccumulating));
    }

    // Learn only in flight, on the ground the wheels and the takeoff acceleration mislead it. From the first flight
    // detection to the landing detection: isFlightDetected alone drops with the throttle under cruise or hover
    if (!ARMING_FLAG(ARMED) || STATE(LANDING_DETECTED)) {
        gsfAirborne = false;
    } else if (isFlightDetected()) {
        gsfAirborne = true;
    }
    const bool inFlight = gsfAirborne;
    if (!inFlight && gsfState.fusing) {
        gsfState.fusing = false;
    }

    // Step the filter up to the GPS delay behind the present
    const gsfWindow_t *last = NULL;
    while (gsfWindowCount > 0) {
        const gsfWindow_t *w = &gsfWindows[(gsfWindowHead + GSF_WINDOW_COUNT - gsfWindowCount) % GSF_WINDOW_COUNT];
        if (cmpTimeUs(currentTimeUs, w->end) < GSF_GPS_DELAY_US) {
            break;
        }

        gsfPredict(&gsfState, w->dv, w->dYaw, w->dt);
        if (gsfState.fusing) {
            gsfStatus.excitation += fast_fsqrtf(sq(w->dv[0]) + sq(w->dv[1]));
        }
        last = w;
        gsfWindowCount--;
    }

    // The latest GPS sample, fused once the filter has reached the time it was taken
    if (gsfGpsPending && last && cmpTimeUs(last->end, gsfGpsTakenAt) >= -GSF_WINDOW_US) {
        gsfGpsPending = false;
        const bool spaced = !gsfState.fusing || cmpTimeUs(gsfGpsTakenAt, gsfLastFusion) >= GSF_FUSION_MIN_US;
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

    gsfUpdateComposite(&gsfState);

    // Carry the delayed yaw to the present with the gyro
    float yaw = gsfState.yaw + gsfAccumulating.dYaw;
    for (int n = 0; n < gsfWindowCount; n++) {
        yaw += gsfWindows[(gsfWindowHead + GSF_WINDOW_COUNT - 1 - n) % GSF_WINDOW_COUNT].dYaw;
    }
    gsfStatus.yaw = yaw < 0.0f ? yaw + 2.0f * M_PIf : (yaw >= 2.0f * M_PIf ? yaw - 2.0f * M_PIf : yaw);
    gsfStatus.yawSigma = fast_fsqrtf(gsfState.yawVariance);

    // Its own variance is not proof on its own: it also has to have seen manoeuvres, and stay settled a while
    const bool settled = gsfState.fusing && gsfStatus.excitation >= GSF_USABLE_EXCITATION && gsfStatus.yawSigma < GSF_USABLE_SIGMA;
    if (!settled) {
        gsfSettledSince = 0;
    } else if (gsfSettledSince == 0) {
        gsfSettledSince = currentTimeUs ? currentTimeUs : 1;
    }

    gsfStatus.flags = 0;
    if (gsfState.fusing) {
        gsfStatus.flags |= GSF_YAW_FUSING;
    }
    if (settled && cmpTimeUs(currentTimeUs, gsfSettledSince) >= GSF_USABLE_SETTLE_US) {
        gsfStatus.flags |= GSF_YAW_USABLE;
    }
}

const gsfYawStatus_t *gsfYawGetStatus(void)
{
    return &gsfStatus;
}

#endif
