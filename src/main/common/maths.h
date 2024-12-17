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

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifndef sq
#define sq(x) ((x)*(x))
#endif

// Undefine this for use libc sinf/cosf. Keep this defined to use fast sin/cos approximations
#define FAST_MATH       // order 9 approximation
//#define VERY_FAST_MATH  // order 7 approximation

// Use floating point M_PI instead explicitly.
#define M_PIf   3.14159265358979323846f
#define M_LN2f  0.69314718055994530942f
#define M_Ef    2.71828182845904523536f

#define RAD (M_PIf / 180.0f)

#define DEGREES_TO_CENTIDEGREES(angle) ((angle) * 100)
#define CENTIDEGREES_TO_DEGREES(angle) ((angle) * 0.01f)

#define CENTIDEGREES_TO_DECIDEGREES(angle) ((angle) / 10.0f)
#define DECIDEGREES_TO_CENTIDEGREES(angle) ((angle) * 10)

#define DEGREES_TO_DECIDEGREES(angle) ((angle) * 10)
#define DECIDEGREES_TO_DEGREES(angle) ((angle) / 10.0f)

#define DEGREES_PER_DEKADEGREE 10
#define DEGREES_TO_DEKADEGREES(angle) ((angle) / DEGREES_PER_DEKADEGREE)
#define DEKADEGREES_TO_DEGREES(angle) ((angle) * DEGREES_PER_DEKADEGREE)

#define DEGREES_TO_RADIANS(angle) ((angle) * RAD)
#define RADIANS_TO_DEGREES(angle) ((angle) / RAD)
#define DECIDEGREES_TO_RADIANS(angle) (((angle) / 10.0f) * RAD)
#define RADIANS_TO_DECIDEGREES(angle) (((angle) * 10.0f) / RAD)

#define RADIANS_TO_CENTIDEGREES(angle) (((angle) * 100.0f) / RAD)
#define CENTIDEGREES_TO_RADIANS(angle) (((angle) * 0.01f) * RAD)

#define MILLIMETERS_TO_CENTIMETERS(mm)  (mm / 10.0f)

#define CENTIMETERS_TO_CENTIFEET(cm)    (cm / 0.3048f)
#define CENTIMETERS_TO_FEET(cm)         (cm / 30.48f)
#define CENTIMETERS_TO_METERS(cm)       (cm * 0.01f)

#define METERS_TO_CENTIMETERS(m)        (m * 100)
#define METERS_TO_KILOMETERS(m)         (m / 1000.0f)
#define METERS_TO_MILES(m)              (m / 1609.344f)
#define METERS_TO_NAUTICALMILES(m)      (m / 1852.00f)

#define MWH_TO_WH(mWh)                  (mWh / 1000.0f)

#define CMSEC_TO_CENTIMPH(cms)          (cms * 2.2369363f)
#define CMSEC_TO_CENTIKPH(cms)          (cms * 3.6f)
#define CMSEC_TO_CENTIKNOTS(cms)        (cms * 1.943845f)

#define CMSEC_TO_MPH(cms)          (CMSEC_TO_CENTIMPH(cms) / 100.0f)
#define CMSEC_TO_KPH(cms)          (CMSEC_TO_CENTIKPH(cms) / 100.0f)
#define CMSEC_TO_KNOTS(cms)        (CMSEC_TO_CENTIKNOTS(cms) / 100.0f)

#define C_TO_KELVIN(temp) (temp + 273.15f)

// Standard Sea Level values
// Ref:https://en.wikipedia.org/wiki/Standard_sea_level
#define SSL_AIR_DENSITY		1.225f // kg/m^3
#define SSL_AIR_PRESSURE	101325.01576f // Pascal
#define SSL_AIR_TEMPERATURE	288.15f // K

// copied from https://code.google.com/p/cxutil/source/browse/include/cxutil/utility.h#70
#define _CHOOSE2(binoper, lexpr, lvar, rexpr, rvar) \
    ( __extension__ ({                              \
        __typeof__(lexpr) lvar = (lexpr);           \
        __typeof__(rexpr) rvar = (rexpr);           \
        lvar binoper rvar ? lvar : rvar;            \
    }))
#define _CHOOSE_VAR2(prefix, unique) prefix##unique
#define _CHOOSE_VAR(prefix, unique) _CHOOSE_VAR2(prefix, unique)
#define _CHOOSE(binoper, lexpr, rexpr)          \
    _CHOOSE2(                                   \
        binoper,                                \
        lexpr, _CHOOSE_VAR(_left, __COUNTER__), \
        rexpr, _CHOOSE_VAR(_right, __COUNTER__) \
        )
#define MIN(a, b) _CHOOSE(<, a, b)
#define MAX(a, b) _CHOOSE(>, a, b)
#define SIGN(a) ((a >= 0) ? 1 : -1)

#define _ABS_II(x, var)             \
    ( __extension__ ({              \
        __typeof__(x) var = (x);    \
        var < 0 ? -var : var;       \
    }))
#define _ABS_I(x, var) _ABS_II(x, var)
#define ABS(x) _ABS_I(x, _CHOOSE_VAR(_abs, __COUNTER__))

#define power3(x) ((x)*(x)*(x))

// Floating point Euler angles.
typedef struct fp_angles {
    float roll;
    float pitch;
    float yaw;
} fp_angles_def;

typedef union {
    float raw[3];
    fp_angles_def angles;
} fp_angles_t;

typedef struct stdev_s {
    float m_oldM, m_newM, m_oldS, m_newS;
    int m_n;
} stdev_t;

typedef struct filterWithBufferSample_s {
    float value;
    uint32_t timestamp;
} filterWithBufferSample_t;

typedef struct filterWithBufferState_s {
    uint16_t filter_size;
    uint16_t sample_index;
    filterWithBufferSample_t * samples;
} filterWithBufferState_t;

typedef struct {
    float XtY[4];
    float XtX[4][4];
} sensorCalibrationState_t;

void sensorCalibrationResetState(sensorCalibrationState_t * state);
void sensorCalibrationPushSampleForOffsetCalculation(sensorCalibrationState_t * state, float sample[3]);
void sensorCalibrationPushSampleForScaleCalculation(sensorCalibrationState_t * state, int axis, float sample[3], int target);
bool sensorCalibrationSolveForOffset(sensorCalibrationState_t * state, float result[3]);
bool sensorCalibrationSolveForScale(sensorCalibrationState_t * state, float result[3]);

int gcd(int num, int denom);
int32_t applyDeadband(int32_t value, int32_t deadband);
int32_t applyDeadbandRescaled(int32_t value, int32_t deadband, int32_t min, int32_t max);

int32_t constrain(int32_t amt, int32_t low, int32_t high);
float constrainf(float amt, float low, float high);

void devClear(stdev_t *dev);
void devPush(stdev_t *dev, float x);
float devVariance(stdev_t *dev);
float devStandardDeviation(stdev_t *dev);
float degreesToRadians(int16_t degrees);

int scaleRange(int x, int srcMin, int srcMax, int destMin, int destMax);
float scaleRangef(float x, float srcMin, float srcMax, float destMin, float destMax);

int32_t wrap_18000(int32_t angle);
int16_t wrap_180(int16_t angle);
int32_t wrap_36000(int32_t angle);

#if defined(FAST_MATH) || defined(VERY_FAST_MATH)
float sin_approx(float x);
float cos_approx(float x);
float atan2_approx(float y, float x);
float acos_approx(float x);
#define tan_approx(x)       (sin_approx(x) / cos_approx(x))
#define asin_approx(x)      (M_PIf / 2 - acos_approx(x))
#else
#define asin_approx(x)      asinf(x)
#define sin_approx(x)       sinf(x)
#define cos_approx(x)       cosf(x)
#define atan2_approx(y,x)   atan2f(y,x)
#define acos_approx(x)      acosf(x)
#define tan_approx(x)       tanf(x)
#endif

void arraySubInt32(int32_t *dest, int32_t *array1, int32_t *array2, int count);

float bellCurve(const float x, const float curveWidth);
float attenuation(const float input, const float width);
float gaussian(const float x, const float mu, const float sigma);
float fast_fsqrtf(const float value);
float calc_length_pythagorean_2D(const float firstElement, const float secondElement);
float calc_length_pythagorean_3D(const float firstElement, const float secondElement, const float thirdElement);

/*
 * The most significat byte is placed at the lowest address
 * in other words, the most significant byte is "first", on even indexes
 */
#define int16_val_big_endian(v, idx) ((int16_t)(((uint8_t)v[2 * idx] << 8) | v[2 * idx + 1]))
/*
 * The most significat byte is placed at the highest address
 * in other words, the most significant byte is "last", on odd indexes
 */
#define int16_val_little_endian(v, idx) ((int16_t)(((uint8_t)v[2 * idx + 1] << 8) | v[2 * idx]))

#ifdef SITL_BUILD
void arm_sub_f32(float * pSrcA, float * pSrcB, float * pDst, uint32_t blockSize);
void arm_scale_f32(float * pSrc, float scale, float * pDst, uint32_t blockSize);
void arm_mult_f32(float * pSrcA, float * pSrcB, float * pDst, uint32_t blockSize);
#endif