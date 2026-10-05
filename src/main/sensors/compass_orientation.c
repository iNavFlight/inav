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

#include <math.h>
#include <string.h>

#include "platform.h"

#include "sensors/compass_orientation.h"

#include "common/maths.h"
#include "common/vector.h"
#include "common/quaternion.h"

#if defined(USE_MAG) && defined(USE_MAG_CALIBRATION_ORIENTATION)

#define COMPASS_ORIENTATION_MAX_SAMPLES 128
#define COMPASS_ORIENTATION_MIN_SAMPLES 30
#define COMPASS_ORIENTATION_CANDIDATE_COUNT 16

typedef struct {
    int16_t rawMag[3];
    int16_t q[4];   // attitude quaternion, Q15 fixed point (component * 32767)
} compassOrientationSample_t;

static compassOrientationSample_t sampleBuffer[COMPASS_ORIENTATION_MAX_SAMPLES];
static uint16_t sampleCount;

typedef struct {
    int16_t rollDD, pitchDD, yawDD;
} compassOrientationCandidate_t;

static const compassOrientationCandidate_t compassOrientationCandidates[COMPASS_ORIENTATION_CANDIDATE_COUNT] = {
    {   0, 0,    0}, {   0, 0,  900}, {   0, 0, 1800}, {   0, 0, 2700},
    {1800, 0,    0}, {1800, 0,  900}, {1800, 0, 1800}, {1800, 0, 2700},
    {   0, 0,  450}, {   0, 0, 1350}, {   0, 0, 2250}, {   0, 0, 3150},
    {1800, 0,  450}, {1800, 0, 1350}, {1800, 0, 2250}, {1800, 0, 3150},
};

// Running mean/variance of a 3-vector (Welford's algorithm, multivariate: m2
// accumulates squared distance to the running mean, summed over all 3 axes).
typedef struct {
    float mean[3];
    float m2;
    uint16_t n;
} compassOrientationStats_t;

static void statsReset(compassOrientationStats_t *s)
{
    memset(s, 0, sizeof(*s));
}

static void statsPush(compassOrientationStats_t *s, const fpVector3_t *sample)
{
    s->n++;
    float delta[3], delta2[3];
    for (int i = 0; i < 3; i++) {
        delta[i] = sample->v[i] - s->mean[i];
        s->mean[i] += delta[i] / s->n;
        delta2[i] = sample->v[i] - s->mean[i];
    }
    s->m2 += delta[0] * delta2[0] + delta[1] * delta2[1] + delta[2] * delta2[2];
}

static float statsVariance(const compassOrientationStats_t *s)
{
    return (s->n > 1) ? (s->m2 / (float)(s->n - 1)) : 0.0f;
}

static int16_t q15Encode(float x)
{
    return (int16_t)lrintf(constrainf(x, -1.0f, 1.0f) * 32767.0f);
}

static float q15Decode(int16_t x)
{
    return (float)x / 32767.0f;
}

void compassOrientationBufferReset(void)
{
    sampleCount = 0;
}

void compassOrientationBufferPush(const int16_t rawMag[3], const fpQuaternion_t *attitude)
{
    if (sampleCount >= COMPASS_ORIENTATION_MAX_SAMPLES) {
        return;
    }

    compassOrientationSample_t *s = &sampleBuffer[sampleCount++];
    s->rawMag[0] = rawMag[0];
    s->rawMag[1] = rawMag[1];
    s->rawMag[2] = rawMag[2];
    s->q[0] = q15Encode(attitude->q0);
    s->q[1] = q15Encode(attitude->q1);
    s->q[2] = q15Encode(attitude->q2);
    s->q[3] = q15Encode(attitude->q3);
}

bool compassOrientationDetect(const float magZero[3], const int16_t magGain[3],
                               float confidenceMin,
                               int16_t *outRollDD, int16_t *outPitchDD, int16_t *outYawDD,
                               float *outConfidence)
{
    if (sampleCount < COMPASS_ORIENTATION_MIN_SAMPLES) {
        return false;
    }
    if (magGain[0] == 0 || magGain[1] == 0 || magGain[2] == 0) {
        return false;
    }

    int best = -1;
    int second = -1;
    float bestVar = 0.0f;
    float secondVar = 0.0f;

    for (int c = 0; c < COMPASS_ORIENTATION_CANDIDATE_COUNT; c++) {
        fp_angles_t angles = {
            .angles.roll  = DECIDEGREES_TO_RADIANS(compassOrientationCandidates[c].rollDD),
            .angles.pitch = DECIDEGREES_TO_RADIANS(compassOrientationCandidates[c].pitchDD),
            .angles.yaw   = DECIDEGREES_TO_RADIANS(compassOrientationCandidates[c].yawDD),
        };
        fpMat3_t rmat;
        rotationMatrixFromAngles(&rmat, &angles);

        compassOrientationStats_t stats;
        statsReset(&stats);

        for (uint16_t i = 0; i < sampleCount; i++) {
            const compassOrientationSample_t *s = &sampleBuffer[i];

            const fpVector3_t calibrated = {
                .x = (s->rawMag[0] - magZero[0]) * 1024.0f / magGain[0],
                .y = (s->rawMag[1] - magZero[1]) * 1024.0f / magGain[1],
                .z = (s->rawMag[2] - magZero[2]) * 1024.0f / magGain[2],
            };
            const fpQuaternion_t q = {
                .q0 = q15Decode(s->q[0]), .q1 = q15Decode(s->q[1]),
                .q2 = q15Decode(s->q[2]), .q3 = q15Decode(s->q[3]),
            };

            fpVector3_t boardFrame, earthField;
            rotationMatrixRotateVector(&boardFrame, &calibrated, &rmat);
            quaternionRotateVectorInv(&earthField, &boardFrame, &q);
            statsPush(&stats, &earthField);
        }

        const float variance = statsVariance(&stats);
        if (best == -1 || variance < bestVar) {
            second = best;
            secondVar = bestVar;
            best = c;
            bestVar = variance;
        } else if (second == -1 || variance < secondVar) {
            second = c;
            secondVar = variance;
        }
    }

    (void)second;
    // bestVar can be ~0 for a mathematically-perfect fit; 1e-6f avoids a
    // near-zero/near-zero division producing a meaningless huge or NaN ratio.
    const float confidence = (bestVar < 1e-6f) ? confidenceMin : (secondVar / bestVar);
    *outConfidence = confidence;
    if (isnan(confidence) || isinf(confidence) || confidence < confidenceMin) {
        return false;
    }

    *outRollDD = compassOrientationCandidates[best].rollDD;
    *outPitchDD = compassOrientationCandidates[best].pitchDD;
    *outYawDD = compassOrientationCandidates[best].yawDD;
    return true;
}

#endif
