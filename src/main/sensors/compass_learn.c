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
#include <stdbool.h>
#include <string.h>

#include "platform.h"

#ifdef USE_MAG_LEARN

#include "build/build_config.h"

#include "common/bitarray.h"
#include "common/maths.h"
#include "common/utils.h"

#include "sensors/compass_learn.h"

magLearnStatus_t magLearnStatus;

// Read only where magLearnFilled is set: AT32 startup leaves FASTRAM uncleared
STATIC_FASTRAM int16_t magLearnSample[MAG_LEARN_BINS][XYZ_AXIS_COUNT];
static BITARRAY_DECLARE(magLearnFilled, MAG_LEARN_BINS);
static uint16_t magLearnHeadingsSeen;

// A sample m = R^T B + o, B the earth field and o the offset: with R a rotation the fit needs only these weighted means
static struct {
    float R[3][3];
    float m[3];
    float u[3];         // of R m, the sample turned to earth
    float mm;           // of the sample squared
    float weight;
    float perHeading[MAG_LEARN_AZIMUTH_BINS];
} magLearnAttitudeFit;

void magLearnReset(void)
{
    BITARRAY_CLR_ALL(magLearnFilled);
    magLearnHeadingsSeen = 0;
    memset(&magLearnAttitudeFit, 0, sizeof(magLearnAttitudeFit));
    memset(&magLearnStatus, 0, sizeof(magLearnStatus));
}

void magLearnAttitude(float rMat[3][3], float heading, float R[3][3])
{
    // Turn about up from the AHRS heading, -atan2(rMat[1][0], rMat[0][0]), to the reference
    const float a = -atan2_approx(rMat[1][0], rMat[0][0]) - heading;
    const float c = cos_approx(a);
    const float s = sin_approx(a);
    for (int j = 0; j < 3; j++) {
        R[0][j] = c * rMat[0][j] - s * rMat[1][j];
        R[1][j] = s * rMat[0][j] + c * rMat[1][j];
        R[2][j] = rMat[2][j];
    }
}

static void magLearnAttitudeAdd(const float m[XYZ_AXIS_COUNT], float R[3][3], float w)
{
    magLearnAttitudeFit.weight += w;
    const float k = w / magLearnAttitudeFit.weight;
    for (int i = 0; i < 3; i++) {
        const float u = R[i][X] * m[X] + R[i][Y] * m[Y] + R[i][Z] * m[Z];
        magLearnAttitudeFit.m[i] += (m[i] - magLearnAttitudeFit.m[i]) * k;
        magLearnAttitudeFit.u[i] += (u - magLearnAttitudeFit.u[i]) * k;
        for (int j = 0; j < 3; j++) {
            magLearnAttitudeFit.R[i][j] += (R[i][j] - magLearnAttitudeFit.R[i][j]) * k;
        }
    }
    magLearnAttitudeFit.mm += (m[X] * m[X] + m[Y] * m[Y] + m[Z] * m[Z] - magLearnAttitudeFit.mm) * k;
}

void magLearnAddSample(const float bodyField[XYZ_AXIS_COUNT], const int16_t raw[XYZ_AXIS_COUNT], float R[3][3], float headingSigma)
{
    const float norm = calc_length_pythagorean_3D(bodyField[X], bodyField[Y], bodyField[Z]);
    if (norm < 1.0f) {
        return;
    }

    // Bands of equal z/|B| have equal area on the sphere
    const int elevation = constrain((bodyField[Z] / norm + 1.0f) * (MAG_LEARN_ELEVATION_BINS / 2.0f), 0, MAG_LEARN_ELEVATION_BINS - 1);
    const int azimuth = constrain((atan2_approx(bodyField[Y], bodyField[X]) + M_PIf) * (MAG_LEARN_AZIMUTH_BINS / (2.0f * M_PIf)), 0, MAG_LEARN_AZIMUTH_BINS - 1);
    const int bin = elevation * MAG_LEARN_AZIMUTH_BINS + azimuth;

    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        magLearnSample[bin][axis] = raw[axis];
    }
    if (!bitArrayGet(magLearnFilled, bin)) {
        bitArraySet(magLearnFilled, bin);
        magLearnStatus.sectors++;
    }
    if (!(magLearnHeadingsSeen & BIT(azimuth))) {
        magLearnHeadingsSeen |= BIT(azimuth);
        magLearnStatus.headings++;
    }
    if (!R) {
        return;
    }
    // Capped by the heading itself: in a banked turn a steep field barely moves through the azimuth sectors
    const int heading = constrain((M_PIf - atan2_approx(R[1][0], R[0][0])) * (MAG_LEARN_AZIMUTH_BINS / (2.0f * M_PIf)), 0, MAG_LEARN_AZIMUTH_BINS - 1);
    // The first samples after the heading turns usable are its least sure: weighed down, they don't fill a sector
    const float w = MIN(1.0f, sq(MAG_LEARN_ATTITUDE_SIGMA / MAX(headingSigma, 0.001f)));
    if (magLearnAttitudeFit.perHeading[heading] < MAG_LEARN_ATTITUDE_BIN_CAP) {
        magLearnAttitudeFit.perHeading[heading] += w;
        magLearnAttitudeAdd(bodyField, R, w);
    }
}

static bool solve3(const float A[3][3], const float b[3], float x[3])
{
    const float c00 = A[1][1] * A[2][2] - A[1][2] * A[2][1];
    const float c01 = A[1][2] * A[2][0] - A[1][0] * A[2][2];
    const float c02 = A[1][0] * A[2][1] - A[1][1] * A[2][0];
    const float det = A[0][0] * c00 + A[0][1] * c01 + A[0][2] * c02;
    if (!(fabsf(det) > 1e-12f)) {
        return false;
    }
    const float c10 = A[0][2] * A[2][1] - A[0][1] * A[2][2];
    const float c11 = A[0][0] * A[2][2] - A[0][2] * A[2][0];
    const float c12 = A[0][1] * A[2][0] - A[0][0] * A[2][1];
    const float c20 = A[0][1] * A[1][2] - A[0][2] * A[1][1];
    const float c21 = A[0][2] * A[1][0] - A[0][0] * A[1][2];
    const float c22 = A[0][0] * A[1][1] - A[0][1] * A[1][0];
    x[0] = (c00 * b[0] + c10 * b[1] + c20 * b[2]) / det;
    x[1] = (c01 * b[0] + c11 * b[1] + c21 * b[2]) / det;
    x[2] = (c02 * b[0] + c12 * b[1] + c22 * b[2]) / det;
    return true;
}

// ((1 + ridge) I - R^T R) o = m - R^T u: the ridge holds o where the flight never turned (R^T R reaches 1 there)
static bool magLearnAttitudeSolve(float o[3], float B[3], float *rms)
{
    float (*R)[3] = magLearnAttitudeFit.R;
    float S[3][3];
    float rhs[3];
    for (int i = 0; i < 3; i++) {
        rhs[i] = magLearnAttitudeFit.m[i];
        for (int j = 0; j < 3; j++) {
            S[i][j] = i == j ? 1.0f + MAG_LEARN_ATTITUDE_RIDGE : 0.0f;
            for (int k = 0; k < 3; k++) {
                S[i][j] -= R[k][i] * R[k][j];
            }
        }
        for (int k = 0; k < 3; k++) {
            rhs[i] -= R[k][i] * magLearnAttitudeFit.u[k];
        }
    }
    if (!solve3(S, rhs, o)) {
        return false;
    }
    float residual = magLearnAttitudeFit.mm;
    for (int i = 0; i < 3; i++) {
        B[i] = magLearnAttitudeFit.u[i] - (R[i][X] * o[X] + R[i][Y] * o[Y] + R[i][Z] * o[Z]);
    }
    for (int i = 0; i < 3; i++) {
        const float Ro = R[i][X] * o[X] + R[i][Y] * o[Y] + R[i][Z] * o[Z];
        residual += o[i] * o[i] + B[i] * B[i] - 2.0f * o[i] * magLearnAttitudeFit.m[i] - 2.0f * B[i] * magLearnAttitudeFit.u[i] + 2.0f * Ro * B[i];
    }
    *rms = fast_fsqrtf(MAX(residual, 0.0f));
    return true;
}

// Offsets in body axes at the stored scale, misfit in 0.1% of the field, yawError (rad) from the field to magnetic north
static bool magLearnAttitudeCandidate(float declination, uint16_t *flags, float o[3], uint16_t *spread, float *yawError)
{
    int headings = 0;
    for (int bin = 0; bin < MAG_LEARN_AZIMUTH_BINS; bin++) {
        headings += magLearnAttitudeFit.perHeading[bin] >= MAG_LEARN_ATTITUDE_HEADING_MIN;
    }
    float B[3], rms;
    if (magLearnAttitudeFit.weight < MAG_LEARN_ATTITUDE_MIN_SAMPLES || headings < MAG_LEARN_ATTITUDE_MIN_HEADINGS ||
        !magLearnAttitudeSolve(o, B, &rms)) {
        return false;
    }
    const float norm = calc_length_pythagorean_3D(B[X], B[Y], B[Z]);
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        magLearnStatus.field[axis] = fabsf(B[axis]) < INT16_MAX ? lrintf(B[axis]) : 0;
    }

    // Written to fail on NaN, like the sphere fit's checks
    const float misfit = rms / norm * 1000;
    *spread = misfit < 1000.0f ? lrintf(misfit) : 1000;
    if (!(misfit <= MAG_LEARN_MAX_MISFIT)) {
        *flags |= MAG_LEARN_MISFIT;
        return false;
    }
    // Earth axes are north, west, up: a field east of north has a negative Y
    float declinationError = -atan2_approx(B[Y], B[X]) - declination;
    if (declinationError > M_PIf) {
        declinationError -= 2.0f * M_PIf;
    } else if (declinationError < -M_PIf) {
        declinationError += 2.0f * M_PIf;
    }
    if (!(fabsf(declinationError) <= MAG_LEARN_MAX_DECLINATION_ERROR)) {
        *flags |= MAG_LEARN_DECLINATION_OFF;
        return false;
    }
    *yawError = declinationError;
    if (!(fabsf(norm - 1024) <= MAG_LEARN_MAX_SCALE_ERROR * 1024)) {
        *flags |= MAG_LEARN_OFF_SCALE;
    }
    return true;
}

// Scaled by maggain like magADC, so the axes weigh alike and the sphere has radius 1024 while the gains hold
static void magLearnPass(const float centre[XYZ_AXIS_COUNT], const int16_t gain[XYZ_AXIS_COUNT],
                         sensorCalibrationState_t *fit, float *radius, float *variance)
{
    float sum = 0.0f;
    float sumSq = 0.0f;
    for (int bin = 0; bin < MAG_LEARN_BINS; bin++) {
        if (bitArrayGet(magLearnFilled, bin)) {
            float v[XYZ_AXIS_COUNT];
            for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
                v[axis] = (magLearnSample[bin][axis] - centre[axis]) * 1024 / gain[axis];
            }
            if (fit) {
                sensorCalibrationPushSampleForOffsetCalculation(fit, v);
            }
            const float r = calc_length_pythagorean_3D(v[X], v[Y], v[Z]);
            sum += r;
            sumSq += r * r;
        }
    }
    *radius = sum / magLearnStatus.sectors;
    *variance = sumSq / magLearnStatus.sectors - sq(*radius);
}

// The sphere fit's step in sensor axes at the stored scale, or false without the coverage for one
static bool magLearnSphereCandidate(const int16_t zero[XYZ_AXIS_COUNT], const int16_t gain[XYZ_AXIS_COUNT], uint16_t *flags,
                                    float step[3], uint16_t *spreadOut, bool *big)
{
    if (magLearnStatus.sectors < MAG_LEARN_MIN_SECTORS) {
        *flags |= MAG_LEARN_FEW_SECTORS;
    }
    if (magLearnStatus.headings < MAG_LEARN_MIN_HEADINGS) {
        *flags |= MAG_LEARN_FEW_HEADINGS;
    }
    if (*flags & (MAG_LEARN_FEW_SECTORS | MAG_LEARN_FEW_HEADINGS)) {
        return false;
    }

    float centre[XYZ_AXIS_COUNT];
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        centre[axis] = zero[axis];
    }
    sensorCalibrationState_t fit;
    sensorCalibrationResetState(&fit);
    float radius, variance;
    magLearnPass(centre, gain, &fit, &radius, &variance);

    // Pulls the fit toward the stored offset: a direction the flight never swept (the vertical, in level turns) keeps it
    const float ridge = (fit.XtX[X][X] + fit.XtX[Y][Y] + fit.XtX[Z][Z]) / 64.0f;
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        fit.XtX[axis][axis] += ridge;
    }
    sensorCalibrationSolveForOffset(&fit, step);
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        centre[axis] += step[axis] * gain[axis] / 1024;
    }
    magLearnPass(centre, gain, NULL, &radius, &variance);

    // Every check is written to fail on NaN, so a degenerate fit is never saved
    const float spread = sqrtf(fabsf(variance)) / radius * 1000;
    *spreadOut = spread < 1000.0f ? lrintf(spread) : 1000;
    if (!(spread <= MAG_LEARN_MAX_SPREAD)) {
        *flags |= MAG_LEARN_NOT_SPHERE;
    }
    if (!(fabsf(radius - 1024) <= MAG_LEARN_MAX_SCALE_ERROR * 1024)) {
        *flags |= MAG_LEARN_OFF_SCALE;
    }
    *big = false;
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        if (!(fabsf(step[axis]) <= MAG_LEARN_MAX_STEP * radius)) {
            *big = true;
        }
    }
    return true;
}

// The sphere fit decides as before; the attitude fit confirms a step too big for it and refines the rest, or disagrees
void magLearnEvaluate(const int16_t zero[XYZ_AXIS_COUNT], const int16_t gain[XYZ_AXIS_COUNT], float align[3][3], float declination,
                      int16_t yawTrim)
{
    uint16_t flags = magLearnStatus.flags & ~MAG_LEARN_EVAL_FLAGS;
    memset(magLearnStatus.field, 0, sizeof(magLearnStatus.field));

    float attitude[3] = { 0.0f, 0.0f, 0.0f };
    float sphere[3] = { 0.0f, 0.0f, 0.0f };
    float step[3] = { 0.0f, 0.0f, 0.0f };
    float yawError = 0.0f;
    uint16_t attitudeSpread = 0;
    uint16_t sphereSpread = 0;
    bool big = false;
    bool trimOutOfRoom = false;
    magLearnStatus.yawTrimDelta = 0;
    const bool haveAttitude = magLearnAttitudeCandidate(declination, &flags, attitude, &attitudeSpread, &yawError);
    const bool haveSphere = magLearnSphereCandidate(zero, gain, &flags, sphere, &sphereSpread, &big);
    magLearnStatus.spread = haveSphere ? sphereSpread : 0;
    if (haveSphere) {
        for (int i = 0; i < 3; i++) {
            step[i] = align[i][X] * sphere[X] + align[i][Y] * sphere[Y] + align[i][Z] * sphere[Z];
        }
    }
    if (haveAttitude && haveSphere && !(fabsf(step[X] - attitude[X]) <= MAG_LEARN_FITS_AGREEMENT * 1024 &&
                                        fabsf(step[Y] - attitude[Y]) <= MAG_LEARN_FITS_AGREEMENT * 1024)) {
        flags |= MAG_LEARN_FITS_DISAGREE;
    } else if (haveAttitude) {
        flags |= MAG_LEARN_ATTITUDE_FIT;
        flags &= ~(MAG_LEARN_FEW_SECTORS | MAG_LEARN_FEW_HEADINGS);
        magLearnStatus.spread = attitudeSpread;
        memcpy(step, attitude, sizeof(step));
        // A field off magnetic north is the compass turned about the vertical: trimmed back a little a flight
        const int trim = constrain(lrintf(RADIANS_TO_DECIDEGREES(yawError)), -MAG_LEARN_MAX_YAW_TRIM_STEP, MAG_LEARN_MAX_YAW_TRIM_STEP);
        if (ABS(trim) >= MAG_LEARN_MIN_YAW_TRIM) {
            // What the setting's range still allows; a trim with no room left is reported, not confirmed
            const int room = constrain(yawTrim + trim, -MAG_LEARN_MAX_YAW_TRIM, MAG_LEARN_MAX_YAW_TRIM) - yawTrim;
            if (ABS(room) >= MAG_LEARN_MIN_YAW_TRIM) {
                magLearnStatus.yawTrimDelta = room;
            } else {
                trimOutOfRoom = true;
                flags |= MAG_LEARN_DECLINATION_OFF;
            }
        }
        // The GPS delay shows on the vertical, and a heading error on any axis the sphere fit can't confirm
        for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
            if (!haveSphere && !(fabsf(step[axis]) <= MAG_LEARN_MAX_STEP * 1024)) {
                flags |= MAG_LEARN_STEP_TOO_BIG;
            }
            if (axis == Z || !haveSphere) {
                step[axis] = constrainf(step[axis], -MAG_LEARN_ATTITUDE_MAX_STEP * 1024, MAG_LEARN_ATTITUDE_MAX_STEP * 1024);
            }
        }
    } else if (haveSphere) {
        if (big) {
            flags |= MAG_LEARN_STEP_TOO_BIG;
        }
    } else {
        // No fit, no step: an earlier fit's would be shown next to failure flags
        memset(magLearnStatus.delta, 0, sizeof(magLearnStatus.delta));
        magLearnStatus.flags = flags;
        return;
    }

    bool withinNoise = true;
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        const float delta = (align[X][axis] * step[X] + align[Y][axis] * step[Y] + align[Z][axis] * step[Z]) * gain[axis] / 1024;
        magLearnStatus.delta[axis] = fabsf(delta) < INT16_MAX ? lrintf(delta) : 0;
        if (!(fabsf(delta) < MAG_LEARN_MIN_STEP * ABS(gain[axis]))) {
            withinNoise = false;
        }
    }
    if (!(flags & MAG_LEARN_CHECK_FLAGS)) {
        if (!withinNoise || magLearnStatus.yawTrimDelta != 0) {
            flags |= MAG_LEARN_SAVE_DUE;
        } else if (!trimOutOfRoom) {
            flags |= MAG_LEARN_CONFIRMED;
        }
    }
    magLearnStatus.flags = flags;
}

#endif
