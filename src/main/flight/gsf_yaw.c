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
