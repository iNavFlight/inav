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

// Attitude fit: with the heading known the samples are a rotated earth field plus the offset, linear in both
#define MAG_LEARN_ATTITUDE_SIGMA        0.05f   // rad of heading reference at which a sample weighs 1, less sure ones less
#define MAG_LEARN_ATTITUDE_MIN_SAMPLES  100     // weight: 10 s at the compass rate
#define MAG_LEARN_ATTITUDE_MIN_HEADINGS 6       // a half turn: more sectors than unknowns, a heading error leaves a residual
#define MAG_LEARN_ATTITUDE_HEADING_MIN  1.0f    // weight for a heading sector to count
#define MAG_LEARN_ATTITUDE_BIN_CAP      200.0f  // weight per heading sector, so a long leg does not outweigh the turns
#define MAG_LEARN_ATTITUDE_MAX_SIGMA    0.17f   // rad, of the heading reference
#define MAG_LEARN_ATTITUDE_RIDGE        (1.0f / 64)
#define MAG_LEARN_MAX_MISFIT            120     // 12%, in 0.1% of the field
#define MAG_LEARN_MAX_DECLINATION_ERROR 0.26f   // rad between the fitted field and magnetic north
#define MAG_LEARN_ATTITUDE_MAX_STEP     0.1f    // of the radius per flight, on the vertical and on any axis the sphere fit can't confirm
#define MAG_LEARN_FITS_AGREEMENT        0.1f    // of the radius, between the two fits' horizontal offsets
#define MAG_LEARN_MIN_STEP              0.01f   // of the radius: below it the calibration is confirmed as it is
#define MAG_LEARN_MAX_YAW_TRIM_STEP     50      // decidegrees per flight toward where magnetic north should be
#define MAG_LEARN_MIN_YAW_TRIM          10      // decidegrees: below it the trim stands
#define MAG_LEARN_MAX_YAW_TRIM          300     // decidegrees, mag_yaw_trim's range

// magBiasFlags in the blackbox log, flags in MSP2_INAV_MAG_LEARN
typedef enum {
    MAG_LEARN_COLLECTING        = 1 << 0,
    MAG_LEARN_PAUSED            = 1 << 1,
    MAG_LEARN_ATTITUDE_FIT      = 1 << 2,   // delta comes from the attitude fit
    MAG_LEARN_MISFIT            = 1 << 3,   // the samples do not follow the attitude (orientation, scale, soft iron): sphere fit instead
    MAG_LEARN_SAVE_DUE          = 1 << 4,
    MAG_LEARN_FEW_SECTORS       = 1 << 5,
    MAG_LEARN_FEW_HEADINGS      = 1 << 6,
    MAG_LEARN_NOT_SPHERE        = 1 << 7,
    MAG_LEARN_OFF_SCALE         = 1 << 8,
    MAG_LEARN_STEP_TOO_BIG      = 1 << 9,
    MAG_LEARN_SAVED             = 1 << 10,
    MAG_LEARN_DISARMED_FLYING   = 1 << 11,
    MAG_LEARN_DECLINATION_OFF   = 1 << 12,  // the fitted field is not where magnetic north is: the compass is turned, beyond what the trim can take
    MAG_LEARN_FITS_DISAGREE     = 1 << 13,  // the two fits differ on the horizontal offsets (a tilt the alignment does not know, iron): nothing saved
    MAG_LEARN_CONFIRMED         = 1 << 14,  // the change was within 1% of the field: the calibration stands
} magLearnFlags_e;

#define MAG_LEARN_CHECK_FLAGS   (MAG_LEARN_SAVE_DUE | MAG_LEARN_FEW_SECTORS | MAG_LEARN_FEW_HEADINGS | \
                                 MAG_LEARN_NOT_SPHERE | MAG_LEARN_OFF_SCALE | MAG_LEARN_STEP_TOO_BIG | MAG_LEARN_FITS_DISAGREE)
#define MAG_LEARN_EVAL_FLAGS    (MAG_LEARN_CHECK_FLAGS | MAG_LEARN_ATTITUDE_FIT | MAG_LEARN_MISFIT | MAG_LEARN_DECLINATION_OFF | \
                                 MAG_LEARN_CONFIRMED)

typedef struct magLearnStatus_s {
    int16_t delta[XYZ_AXIS_COUNT];  // estimated magzero minus the stored one, raw counts
    uint16_t flags;                 // magLearnFlags_e
    uint8_t sectors;
    uint8_t headings;
    uint16_t spread;                // 0.1%: of the radius for the sphere fit, of the field for the attitude fit when it decides, else 0
    int16_t field[XYZ_AXIS_COUNT];  // earth field from the attitude fit: north, west, up, in magADC units
    int16_t yawTrimDelta;           // change to mag_yaw_trim, decidegrees, from the attitude fit
} magLearnStatus_t;

extern magLearnStatus_t magLearnStatus;

void magLearnReset(void);
// rMat (body to earth) with its heading swapped for heading (rad, clockwise from north): the compass must not vouch for itself
void magLearnAttitude(float rMat[3][3], float heading, float R[3][3]);
// R: body to earth at the sample, NULL without a heading reference; headingSigma: rad, of that reference
void magLearnAddSample(const float bodyField[XYZ_AXIS_COUNT], const int16_t raw[XYZ_AXIS_COUNT], float R[3][3], float headingSigma);
// align: sensor to body as applied to the samples; declination: rad, east positive; yawTrim: mag_yaw_trim now
void magLearnEvaluate(const int16_t zero[XYZ_AXIS_COUNT], const int16_t gain[XYZ_AXIS_COUNT], float align[3][3], float declination,
                      int16_t yawTrim);

#endif
