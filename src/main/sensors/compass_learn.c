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

void magLearnReset(void)
{
    BITARRAY_CLR_ALL(magLearnFilled);
    magLearnHeadingsSeen = 0;
    memset(&magLearnStatus, 0, sizeof(magLearnStatus));
}

void magLearnAddSample(const float bodyField[XYZ_AXIS_COUNT], const int16_t raw[XYZ_AXIS_COUNT])
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

void magLearnEvaluate(const int16_t zero[XYZ_AXIS_COUNT], const int16_t gain[XYZ_AXIS_COUNT])
{
    uint16_t flags = magLearnStatus.flags & ~MAG_LEARN_CHECK_FLAGS;
    if (magLearnStatus.sectors < MAG_LEARN_MIN_SECTORS) {
        flags |= MAG_LEARN_FEW_SECTORS;
    }
    if (magLearnStatus.headings < MAG_LEARN_MIN_HEADINGS) {
        flags |= MAG_LEARN_FEW_HEADINGS;
    }
    if (flags & (MAG_LEARN_FEW_SECTORS | MAG_LEARN_FEW_HEADINGS)) {
        magLearnStatus.flags = flags;
        return;
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
    float step[XYZ_AXIS_COUNT];
    sensorCalibrationSolveForOffset(&fit, step);
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        centre[axis] += step[axis] * gain[axis] / 1024;
    }
    magLearnPass(centre, gain, NULL, &radius, &variance);

    // Every check is written to fail on NaN, so a degenerate fit is never saved
    const float spread = sqrtf(fabsf(variance)) / radius * 1000;
    magLearnStatus.spread = spread < 1000.0f ? lrintf(spread) : 1000;
    if (!(spread <= MAG_LEARN_MAX_SPREAD)) {
        flags |= MAG_LEARN_NOT_SPHERE;
    }
    if (!(fabsf(radius - 1024) <= MAG_LEARN_MAX_SCALE_ERROR * 1024)) {
        flags |= MAG_LEARN_OFF_SCALE;
    }
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        if (!(fabsf(step[axis]) <= MAG_LEARN_MAX_STEP * radius)) {
            flags |= MAG_LEARN_STEP_TOO_BIG;
        }
        const float delta = centre[axis] - zero[axis];
        magLearnStatus.delta[axis] = fabsf(delta) < INT16_MAX ? lrintf(delta) : 0;
    }
    if (!(flags & MAG_LEARN_CHECK_FLAGS)) {
        flags |= MAG_LEARN_SAVE_DUE;
    }
    magLearnStatus.flags = flags;
}

#endif
