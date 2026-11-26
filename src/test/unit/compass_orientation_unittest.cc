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

#include <stdint.h>
#include <math.h>

extern "C" {
    #include "common/maths.h"
    #include "common/vector.h"
    #include "common/quaternion.h"
    #include "sensors/compass_orientation.h"
}

#include "unittest_macros.h"
#include "gtest/gtest.h"

#include <vector>

namespace {

struct Candidate {
    int16_t rollDD, pitchDD, yawDD;
};

// Independent copy of sensors/compass_orientation.c's candidate table. Kept
// separate so these tests exercise the module strictly through its public
// API (they don't need to know the module's internal candidate ordering,
// just that all 16 of these are recoverable).
const Candidate kCandidates[16] = {
    {0, 0, 0}, {0, 0, 900}, {0, 0, 1800}, {0, 0, 2700},
    {1800, 0, 0}, {1800, 0, 900}, {1800, 0, 1800}, {1800, 0, 2700},
    {0, 0, 450}, {0, 0, 1350}, {0, 0, 2250}, {0, 0, 3150},
    {1800, 0, 450}, {1800, 0, 1350}, {1800, 0, 2250}, {1800, 0, 3150},
};

const float kMagZero[3] = {0.0f, 0.0f, 0.0f};
const int16_t kMagGain[3] = {1024, 1024, 1024};

fpVector3_t earthField()
{
    fpVector3_t e;
    e.x = 400.0f;
    e.y = 50.0f;
    e.z = 800.0f;   // nonzero dip - required to disambiguate candidates
    return e;
}

fpMat3_t transpose(const fpMat3_t &m)
{
    fpMat3_t t;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            t.m[i][j] = m.m[j][i];
        }
    }
    return t;
}

fpMat3_t candidateRotation(const Candidate &c)
{
    fp_angles_t angles;
    angles.angles.roll = DECIDEGREES_TO_RADIANS(c.rollDD);
    angles.angles.pitch = DECIDEGREES_TO_RADIANS(c.pitchDD);
    angles.angles.yaw = DECIDEGREES_TO_RADIANS(c.yawDD);

    fpMat3_t rmat;
    rotationMatrixFromAngles(&rmat, &angles);
    return rmat;
}

fpQuaternion_t eulerToQuat(float rollRad, float pitchRad, float yawRad)
{
    const float cr = cosf(rollRad * 0.5f), sr = sinf(rollRad * 0.5f);
    const float cp = cosf(pitchRad * 0.5f), sp = sinf(pitchRad * 0.5f);
    const float cy = cosf(yawRad * 0.5f), sy = sinf(yawRad * 0.5f);

    fpQuaternion_t q;
    q.q0 = cr * cp * cy + sr * sp * sy;
    q.q1 = sr * cp * cy - cr * sp * sy;
    q.q2 = cr * sp * cy + sr * cp * sy;
    q.q3 = cr * cp * sy - sr * sp * cy;
    return q;
}

// Inverts exactly the forward model compassOrientationDetect() uses
// (see sensors/compass_orientation.c): boardFrame = candidateRMat * calibrated,
// earthField = quaternionRotateVectorInv(boardFrame, attitude). magZero=0 /
// magGain=1024 here, so raw == calibrated.
void synthesizeSample(const Candidate &trueCandidate, const fpQuaternion_t &attitude,
                       int16_t rawMagOut[3])
{
    const fpVector3_t e = earthField();

    fpVector3_t boardFrame;
    quaternionRotateVector(&boardFrame, &e, &attitude);   // earth -> body

    const fpMat3_t rmatInv = transpose(candidateRotation(trueCandidate));
    fpVector3_t calibrated;
    rotationMatrixRotateVector(&calibrated, &boardFrame, &rmatInv);   // undo mounting rotation

    rawMagOut[0] = (int16_t)lrintf(calibrated.x);
    rawMagOut[1] = (int16_t)lrintf(calibrated.y);
    rawMagOut[2] = (int16_t)lrintf(calibrated.z);
}

// Spans a wide range of roll/pitch/yaw so the earth field's Z/dip component
// is exercised at every sample - the ambiguous case (yaw-only spin) is
// covered separately below.
std::vector<fpQuaternion_t> diverseAttitudes(int count)
{
    std::vector<fpQuaternion_t> result;
    for (int i = 0; i < count; i++) {
        const float roll = DEGREES_TO_RADIANS((float)((i * 47) % 360 - 180));
        const float pitch = DEGREES_TO_RADIANS((float)((i * 31) % 120 - 60));
        const float yaw = DEGREES_TO_RADIANS((float)((i * 73) % 360 - 180));
        result.push_back(eulerToQuat(roll, pitch, yaw));
    }
    return result;
}

}  // namespace

class CompassOrientationTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        compassOrientationBufferReset();
    }
};

TEST_F(CompassOrientationTest, RecoversEachCandidate)
{
    const std::vector<fpQuaternion_t> attitudes = diverseAttitudes(80);

    for (const Candidate &trueCandidate : kCandidates) {
        compassOrientationBufferReset();
        for (const fpQuaternion_t &q : attitudes) {
            int16_t rawMag[3];
            synthesizeSample(trueCandidate, q, rawMag);
            compassOrientationBufferPush(rawMag, &q);
        }

        int16_t outRoll, outPitch, outYaw;
        float confidence;
        const bool ok = compassOrientationDetect(kMagZero, kMagGain, 2.0f,
                                                   &outRoll, &outPitch, &outYaw, &confidence);

        EXPECT_TRUE(ok);
        EXPECT_EQ(outRoll, trueCandidate.rollDD);
        EXPECT_EQ(outPitch, trueCandidate.pitchDD);
        EXPECT_EQ(outYaw, trueCandidate.yawDD);
        EXPECT_GT(confidence, 2.0f);
    }
}

TEST_F(CompassOrientationTest, RejectsFlatYawOnlySpin)
{
    // Attitude never varies in roll/pitch: the implied earth field's Z/dip
    // component never gets exercised, so multiple candidates look equally
    // plausible. This is the classic "spun flat on a table" calibration
    // mistake - the result must be rejected, not silently wrong.
    const Candidate trueCandidate = kCandidates[0];
    for (int i = 0; i < 60; i++) {
        const fpQuaternion_t q = eulerToQuat(0.0f, 0.0f, DEGREES_TO_RADIANS((float)((i * 6) % 360)));
        int16_t rawMag[3];
        synthesizeSample(trueCandidate, q, rawMag);
        compassOrientationBufferPush(rawMag, &q);
    }

    int16_t outRoll, outPitch, outYaw;
    float confidence;
    const bool ok = compassOrientationDetect(kMagZero, kMagGain, 2.0f,
                                               &outRoll, &outPitch, &outYaw, &confidence);
    EXPECT_FALSE(ok);
}

TEST_F(CompassOrientationTest, RejectsInsufficientSamples)
{
    const Candidate trueCandidate = kCandidates[0];
    const std::vector<fpQuaternion_t> attitudes = diverseAttitudes(10);   // below the minimum
    for (const fpQuaternion_t &q : attitudes) {
        int16_t rawMag[3];
        synthesizeSample(trueCandidate, q, rawMag);
        compassOrientationBufferPush(rawMag, &q);
    }

    int16_t outRoll, outPitch, outYaw;
    float confidence;
    const bool ok = compassOrientationDetect(kMagZero, kMagGain, 2.0f,
                                               &outRoll, &outPitch, &outYaw, &confidence);
    EXPECT_FALSE(ok);
}

TEST_F(CompassOrientationTest, ToleratesModerateNoise)
{
    const std::vector<fpQuaternion_t> attitudes = diverseAttitudes(100);
    const Candidate trueCandidate = kCandidates[9];   // {0, 0, 1350} - a diagonal case

    unsigned seed = 12345u;
    for (const fpQuaternion_t &q : attitudes) {
        int16_t rawMag[3];
        synthesizeSample(trueCandidate, q, rawMag);
        for (int axis = 0; axis < 3; axis++) {
            seed = seed * 1103515245u + 12345u;
            const int noise = (int)(seed % 21) - 10;   // +/-10 raw-count jitter
            rawMag[axis] = (int16_t)(rawMag[axis] + noise);
        }
        compassOrientationBufferPush(rawMag, &q);
    }

    int16_t outRoll, outPitch, outYaw;
    float confidence;
    const bool ok = compassOrientationDetect(kMagZero, kMagGain, 2.0f,
                                               &outRoll, &outPitch, &outYaw, &confidence);

    EXPECT_TRUE(ok);
    EXPECT_EQ(outRoll, trueCandidate.rollDD);
    EXPECT_EQ(outPitch, trueCandidate.pitchDD);
    EXPECT_EQ(outYaw, trueCandidate.yawDD);
}

TEST_F(CompassOrientationTest, BufferOverflowIsSilentlyIgnored)
{
    // Pushing more than the buffer can hold must not crash or corrupt state -
    // later samples beyond capacity are simply dropped.
    const Candidate trueCandidate = kCandidates[2];
    const std::vector<fpQuaternion_t> attitudes = diverseAttitudes(200);
    for (const fpQuaternion_t &q : attitudes) {
        int16_t rawMag[3];
        synthesizeSample(trueCandidate, q, rawMag);
        compassOrientationBufferPush(rawMag, &q);
    }

    int16_t outRoll, outPitch, outYaw;
    float confidence;
    const bool ok = compassOrientationDetect(kMagZero, kMagGain, 2.0f,
                                               &outRoll, &outPitch, &outYaw, &confidence);
    EXPECT_TRUE(ok);
    EXPECT_EQ(outRoll, trueCandidate.rollDD);
    EXPECT_EQ(outYaw, trueCandidate.yawDD);
}

TEST_F(CompassOrientationTest, RejectsZeroMagGainInsteadOfNaN)
{
    // A stuck/disconnected sensor axis can leave magGain at 0 after
    // calibration; dividing by it must not silently defeat the confidence
    // gate via NaN comparison semantics (NaN < threshold is always false).
    const Candidate trueCandidate = kCandidates[3];
    const std::vector<fpQuaternion_t> attitudes = diverseAttitudes(80);
    for (const fpQuaternion_t &q : attitudes) {
        int16_t rawMag[3];
        synthesizeSample(trueCandidate, q, rawMag);
        compassOrientationBufferPush(rawMag, &q);
    }

    const int16_t zeroGain[3] = {0, 1024, 1024};
    int16_t outRoll, outPitch, outYaw;
    float confidence = -1.0f;
    const bool ok = compassOrientationDetect(kMagZero, zeroGain, 2.0f,
                                               &outRoll, &outPitch, &outYaw, &confidence);
    EXPECT_FALSE(ok);
    EXPECT_FALSE(isnan(confidence));
}
