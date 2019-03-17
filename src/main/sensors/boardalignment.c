/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#include "platform.h"

#include "common/maths.h"
#include "common/vector.h"
#include "common/axis.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/sensor.h"

#if defined(UNIT_TEST)
// Unit tests can't include settings. Provide some dummy limits.
#define SETTING_ALIGN_BOARD_ROLL_MIN -1800
#define SETTING_ALIGN_BOARD_ROLL_MAX 3600
#define SETTING_ALIGN_BOARD_PITCH_MIN -1800
#define SETTING_ALIGN_BOARD_PITCH_MAX 3600
#else
#include "fc/settings.h"
#endif

#include "boardalignment.h"

static bool standardBoardAlignment = true;     // board orientation correction
static fpMat3_t boardRotMatrix;

// no template required since defaults are zero
PG_REGISTER(boardAlignment_t, boardAlignment, PG_BOARD_ALIGNMENT, 0);

static bool isBoardAlignmentStandard(const boardAlignment_t *boardAlignment)
{
    return !boardAlignment->rollDeciDegrees && !boardAlignment->pitchDeciDegrees && !boardAlignment->yawDeciDegrees;
}

void initBoardAlignment(void)
{
    if (isBoardAlignmentStandard(boardAlignment())) {
        standardBoardAlignment = true;
    } else {
        fp_angles_t rotationAngles;

        standardBoardAlignment = false;

        rotationAngles.angles.roll  = DECIDEGREES_TO_RADIANS(boardAlignment()->rollDeciDegrees );
        rotationAngles.angles.pitch = DECIDEGREES_TO_RADIANS(boardAlignment()->pitchDeciDegrees);
        rotationAngles.angles.yaw   = DECIDEGREES_TO_RADIANS(boardAlignment()->yawDeciDegrees  );

        rotationMatrixFromAngles(&boardRotMatrix, &rotationAngles);
    }
}

void updateBoardAlignment(int16_t roll, int16_t pitch)
{
    const float sinAlignYaw = sin_approx(DECIDEGREES_TO_RADIANS(boardAlignment()->yawDeciDegrees));
    const float cosAlignYaw = cos_approx(DECIDEGREES_TO_RADIANS(boardAlignment()->yawDeciDegrees));

    int16_t rollDeciDegrees = boardAlignment()->rollDeciDegrees + -sinAlignYaw * pitch + cosAlignYaw * roll;
    int16_t pitchDeciDegrees = boardAlignment()->pitchDeciDegrees + cosAlignYaw * pitch + sinAlignYaw * roll;

    boardAlignmentMutable()->rollDeciDegrees = constrain(rollDeciDegrees, SETTING_ALIGN_BOARD_ROLL_MIN, SETTING_ALIGN_BOARD_ROLL_MAX);
    boardAlignmentMutable()->pitchDeciDegrees = constrain(pitchDeciDegrees, SETTING_ALIGN_BOARD_PITCH_MIN, SETTING_ALIGN_BOARD_PITCH_MAX);

    initBoardAlignment();
}

void FAST_CODE applyBoardAlignment(int32_t *vec)
{
    if (standardBoardAlignment) {
        return;
    }

    fpVector3_t fpVec = { .v = { vec[X], vec[Y], vec[Z] } };
    rotationMatrixRotateVector(&fpVec, &fpVec, &boardRotMatrix);

    vec[X] = lrintf(fpVec.x);
    vec[Y] = lrintf(fpVec.y);
    vec[Z] = lrintf(fpVec.z);
}

void FAST_CODE applySensorAlignment(int32_t * dest, int32_t * src, uint8_t rotation)
{
    // Create a copy so we could use the same buffer for src & dest
    const int32_t x = src[X];
    const int32_t y = src[Y];
    const int32_t z = src[Z];

    switch (rotation) {
    default:
    case CW0_DEG:
        dest[X] = x;
        dest[Y] = y;
        dest[Z] = z;
        break;
    case CW90_DEG:
        dest[X] = y;
        dest[Y] = -x;
        dest[Z] = z;
        break;
    case CW180_DEG:
        dest[X] = -x;
        dest[Y] = -y;
        dest[Z] = z;
        break;
    case CW270_DEG:
        dest[X] = -y;
        dest[Y] = x;
        dest[Z] = z;
        break;
    case CW0_DEG_FLIP:
        dest[X] = -x;
        dest[Y] = y;
        dest[Z] = -z;
        break;
    case CW90_DEG_FLIP:
        dest[X] = y;
        dest[Y] = x;
        dest[Z] = -z;
        break;
    case CW180_DEG_FLIP:
        dest[X] = x;
        dest[Y] = -y;
        dest[Z] = -z;
        break;
    case CW270_DEG_FLIP:
        dest[X] = -y;
        dest[Y] = -x;
        dest[Z] = -z;
        break;
    }
}
