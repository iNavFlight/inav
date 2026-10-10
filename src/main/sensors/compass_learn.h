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

#include <stdint.h>

#include "common/axis.h"

#ifdef USE_MAG_LEARN

// In-flight refit of the compass offsets: samples binned by field direction, sphere fit on them, checks before a save

#define MAG_LEARN_AZIMUTH_BINS      12
#define MAG_LEARN_ELEVATION_BINS    6
#define MAG_LEARN_BINS              (MAG_LEARN_AZIMUTH_BINS * MAG_LEARN_ELEVATION_BINS)
#define MAG_LEARN_MIN_SECTORS       12
#define MAG_LEARN_MIN_HEADINGS      9
#define MAG_LEARN_MAX_SPREAD        50      // 5%, in 0.1% of the radius
#define MAG_LEARN_MAX_SCALE_ERROR   0.3f    // of the radius maggain implies
#define MAG_LEARN_MAX_STEP          0.3f    // of the radius, per axis and flight
#define MAG_LEARN_SAVE_TIMEOUT_MS   60000   // after the disarm, for the aircraft to be still

// magBiasFlags in the blackbox log, flags in MSP2_INAV_MAG_LEARN
typedef enum {
    MAG_LEARN_COLLECTING        = 1 << 0,
    MAG_LEARN_PAUSED            = 1 << 1,
    MAG_LEARN_SAVE_DUE          = 1 << 4,
    MAG_LEARN_FEW_SECTORS       = 1 << 5,
    MAG_LEARN_FEW_HEADINGS      = 1 << 6,
    MAG_LEARN_NOT_SPHERE        = 1 << 7,
    MAG_LEARN_OFF_SCALE         = 1 << 8,
    MAG_LEARN_STEP_TOO_BIG      = 1 << 9,
    MAG_LEARN_SAVED             = 1 << 10,
    MAG_LEARN_DISARMED_FLYING   = 1 << 11,
} magLearnFlags_e;

#define MAG_LEARN_CHECK_FLAGS   (MAG_LEARN_SAVE_DUE | MAG_LEARN_FEW_SECTORS | MAG_LEARN_FEW_HEADINGS | \
                                 MAG_LEARN_NOT_SPHERE | MAG_LEARN_OFF_SCALE | MAG_LEARN_STEP_TOO_BIG)

typedef struct magLearnStatus_s {
    int16_t delta[XYZ_AXIS_COUNT];  // estimated magzero minus the stored one, raw counts
    uint16_t flags;                 // magLearnFlags_e
    uint8_t sectors;
    uint8_t headings;
    uint16_t spread;                // of the radius, 0.1%
} magLearnStatus_t;

extern magLearnStatus_t magLearnStatus;

void magLearnReset(void);
void magLearnAddSample(const float bodyField[XYZ_AXIS_COUNT], const int16_t raw[XYZ_AXIS_COUNT]);
void magLearnEvaluate(const int16_t zero[XYZ_AXIS_COUNT], const int16_t gain[XYZ_AXIS_COUNT]);

#endif
