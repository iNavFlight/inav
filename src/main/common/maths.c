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

#include <stdint.h>
#include <string.h>
#include <math.h>

#include "axis.h"
#include "maths.h"
#include "vector.h"
#include "quaternion.h"
#include "platform.h"

#ifdef USE_ARM_MATH
#include "arm_math.h"
#endif

// http://lolengine.net/blog/2011/12/21/better-function-approximations
// Chebyshev http://stackoverflow.com/questions/345085/how-do-trigonometric-functions-work/345117#345117
// Thanks for ledvinap for making such accuracy possible! See: https://github.com/cleanflight/cleanflight/issues/940#issuecomment-110323384
// https://github.com/Crashpilot1000/HarakiriWebstore1/blob/master/src/mw.c#L1235
#if defined(FAST_MATH) || defined(VERY_FAST_MATH)
#if defined(VERY_FAST_MATH)
#define sinPolyCoef3 -1.666568107e-1f
#define sinPolyCoef5  8.312366210e-3f
#define sinPolyCoef7 -1.849218155e-4f
#define sinPolyCoef9  0
#else
#define sinPolyCoef3 -1.666665710e-1f                                          // Double: -1.666665709650470145824129400050267289858e-1
#define sinPolyCoef5  8.333017292e-3f                                          // Double:  8.333017291562218127986291618761571373087e-3
#define sinPolyCoef7 -1.980661520e-4f                                          // Double: -1.980661520135080504411629636078917643846e-4
#define sinPolyCoef9  2.600054768e-6f                                          // Double:  2.600054767890361277123254766503271638682e-6
#endif

float sin_approx(float x)
{
    int32_t xint = x;
    if (xint < -32 || xint > 32) return 0.0f;                               // Stop here on error input (5 * 360 Deg)
    while (x >  M_PIf) x -= (2.0f * M_PIf);                                 // always wrap input angle to -PI..PI
    while (x < -M_PIf) x += (2.0f * M_PIf);
    if (x >  (0.5f * M_PIf)) x =  (0.5f * M_PIf) - (x - (0.5f * M_PIf));   // We just pick -90..+90 Degree
    else if (x < -(0.5f * M_PIf)) x = -(0.5f * M_PIf) - ((0.5f * M_PIf) + x);
    float x2 = x * x;
    return x + x * x2 * (sinPolyCoef3 + x2 * (sinPolyCoef5 + x2 * (sinPolyCoef7 + x2 * sinPolyCoef9)));
}

float cos_approx(float x)
{
    return sin_approx(x + (0.5f * M_PIf));
}

// https://github.com/Crashpilot1000/HarakiriWebstore1/blob/396715f73c6fcf859e0db0f34e12fe44bace6483/src/mw.c#L1292
// http://http.developer.nvidia.com/Cg/atan2.html (not working correctly!)
// Poly coefficients by @ledvinap (https://github.com/cleanflight/cleanflight/pull/1107)
// Max absolute error 0,000027 degree
float atan2_approx(float y, float x)
{
    #define atanPolyCoef1  3.14551665884836e-07f
    #define atanPolyCoef2  0.99997356613987f
    #define atanPolyCoef3  0.14744007058297684f
    #define atanPolyCoef4  0.3099814292351353f
    #define atanPolyCoef5  0.05030176425872175f
    #define atanPolyCoef6  0.1471039133652469f
    #define atanPolyCoef7  0.6444640676891548f

    float res, absX, absY;
    absX = fabsf(x);
    absY = fabsf(y);
    res  = MAX(absX, absY);
    if (res) res = MIN(absX, absY) / res;
    else res = 0.0f;
    res = -((((atanPolyCoef5 * res - atanPolyCoef4) * res - atanPolyCoef3) * res - atanPolyCoef2) * res - atanPolyCoef1) / ((atanPolyCoef7 * res + atanPolyCoef6) * res + 1.0f);
    if (absY > absX) res = (M_PIf / 2.0f) - res;
    if (x < 0) res = M_PIf - res;
    if (y < 0) res = -res;
    return res;
}

// http://http.developer.nvidia.com/Cg/acos.html
// Handbook of Mathematical Functions
// M. Abramowitz and I.A. Stegun, Ed.
// Absolute error <= 6.7e-5
float acos_approx(float x)
{
    float xa = fabsf(x);
    float result = fast_fsqrtf(1.0f - xa) * (1.5707288f + xa * (-0.2121144f + xa * (0.0742610f + (-0.0187293f * xa))));
    if (x < 0.0f)
        return M_PIf - result;
    else
        return result;
}
#endif

int gcd(int num, int denom)
{
    if (denom == 0) {
        return num;
    }

    return gcd(denom, num % denom);
}

int32_t wrap_18000(int32_t angle)
{
    if (angle > 18000)
        angle -= 36000;
    if (angle < -18000)
        angle += 36000;
    return angle;
}

int16_t wrap_180(int16_t angle)
{
    if (angle > 180)
        angle -= 360;
    if (angle < -180)
        angle += 360;
    return angle;
}

int32_t wrap_36000(int32_t angle)
{
    if (angle >= 36000)
        angle -= 36000;
    if (angle < 0)
        angle += 36000;
    return angle;
}

int32_t applyDeadband(int32_t value, int32_t deadband)
{
    if (ABS(value) < deadband) {
        value = 0;
    } else if (value > 0) {
        value -= deadband;
    } else if (value < 0) {
        value += deadband;
    }
    return value;
}

int32_t applyDeadbandRescaled(int32_t value, int32_t deadband, int32_t min, int32_t max)
{
    if (ABS(value) < deadband) {
        value = 0;
    } else if (value > 0) {
        value = scaleRange(value - deadband, 0, max - deadband, 0, max);
    } else if (value < 0) {
        value = scaleRange(value + deadband, min + deadband, 0, min, 0);
    }
    return value;
}

int32_t constrain(int32_t amt, int32_t low, int32_t high)
{
    if (amt < low)
        return low;
    else if (amt > high)
        return high;
    else
        return amt;
}

float constrainf(float amt, float low, float high)
{
    if (amt < low)
        return low;
    else if (amt > high)
        return high;
    else
        return amt;
}

void devClear(stdev_t *dev)
{
    dev->m_n = 0;
}

void devPush(stdev_t *dev, float x)
{
    dev->m_n++;
    if (dev->m_n == 1) {
        dev->m_oldM = dev->m_newM = x;
        dev->m_oldS = 0.0f;
    } else {
        dev->m_newM = dev->m_oldM + (x - dev->m_oldM) / dev->m_n;
        dev->m_newS = dev->m_oldS + (x - dev->m_oldM) * (x - dev->m_newM);
        dev->m_oldM = dev->m_newM;
        dev->m_oldS = dev->m_newS;
    }
}

float devVariance(stdev_t *dev)
{
    return ((dev->m_n > 1) ? dev->m_newS / (dev->m_n - 1) : 0.0f);
}

float devStandardDeviation(stdev_t *dev)
{
    return fast_fsqrtf(devVariance(dev));
}

float degreesToRadians(int16_t degrees)
{
    return degrees * RAD;
}

int scaleRange(int x, int srcMin, int srcMax, int destMin, int destMax) {
    long int a = ((long int) destMax - (long int) destMin) * ((long int) x - (long int) srcMin);
    long int b = (long int) srcMax - (long int) srcMin;
    return ((a / b) + destMin);
}

float scaleRangef(float x, float srcMin, float srcMax, float destMin, float destMax) {
    float a = (destMax - destMin) * (x - srcMin);
    float b = srcMax - srcMin;
    return ((a / b) + destMin);
}

// Build rMat from Tait–Bryan angles (convention X1, Y2, Z3)
void rotationMatrixFromAngles(fpMat3_t * rmat, const fp_angles_t * angles)
{
    float cosx, sinx, cosy, siny, cosz, sinz;
    float coszcosx, sinzcosx, coszsinx, sinzsinx;

    cosx = cos_approx(angles->angles.roll);
    sinx = sin_approx(angles->angles.roll);
    cosy = cos_approx(angles->angles.pitch);
    siny = sin_approx(angles->angles.pitch);
    cosz = cos_approx(angles->angles.yaw);
    sinz = sin_approx(angles->angles.yaw);

    coszcosx = cosz * cosx;
    sinzcosx = sinz * cosx;
    coszsinx = sinx * cosz;
    sinzsinx = sinx * sinz;

    rmat->m[0][X] = cosz * cosy;
    rmat->m[0][Y] = -cosy * sinz;
    rmat->m[0][Z] = siny;
    rmat->m[1][X] = sinzcosx + (coszsinx * siny);
    rmat->m[1][Y] = coszcosx - (sinzsinx * siny);
    rmat->m[1][Z] = -sinx * cosy;
    rmat->m[2][X] = (sinzsinx) - (coszcosx * siny);
    rmat->m[2][Y] = (coszsinx) + (sinzcosx * siny);
    rmat->m[2][Z] = cosy * cosx;
}

void rotationMatrixFromAxisAngle(fpMat3_t * rmat, const fpAxisAngle_t * a)
{
    const float sang = sin_approx(a->angle);
    const float cang = cos_approx(a->angle);
    const float C = 1.0f - cang;

    const float xC  = a->axis.x * C;
    const float yC  = a->axis.y * C;
    const float zC  = a->axis.z * C;
    const float xxC = a->axis.x * xC;
    const float yyC = a->axis.y * yC;
    const float zzC = a->axis.z * zC;
    const float xyC = a->axis.x * yC;
    const float yzC = a->axis.y * zC;
    const float zxC = a->axis.z * xC;
    const float xs  = a->axis.x * sang;
    const float ys  = a->axis.y * sang;
    const float zs  = a->axis.z * sang;

    rmat->m[0][X] = xxC + cang;
    rmat->m[0][Y] = xyC - zs;
    rmat->m[0][Z] = zxC + ys;

    rmat->m[1][X] = zxC + ys;
    rmat->m[1][Y] = yyC + cang;
    rmat->m[1][Z] = yzC - xs;

    rmat->m[2][X] = zxC - ys;
    rmat->m[2][Y] = yzC + xs;
    rmat->m[2][Z] = zzC + cang;
}

void arraySubInt32(int32_t *dest, int32_t *array1, int32_t *array2, int count)
{
    for (int i = 0; i < count; i++) {
        dest[i] = array1[i] - array2[i];
    }
}

/**
 * Sensor offset calculation code based on Freescale's AN4246
 * Initial implementation by @HaukeRa
 * Modified to be re-usable by @DigitalEntity
 */
void sensorCalibrationResetState(sensorCalibrationState_t * state)
{
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            state->XtX[i][j] = 0;
        }

        state->XtY[i] = 0;
    }
}

void sensorCalibrationPushSampleForOffsetCalculation(sensorCalibrationState_t * state, float sample[3])
{
    state->XtX[0][0] += sample[0] * sample[0];
    state->XtX[0][1] += sample[0] * sample[1];
    state->XtX[0][2] += sample[0] * sample[2];
    state->XtX[0][3] += sample[0];

    state->XtX[1][0] += sample[1] * sample[0];
    state->XtX[1][1] += sample[1] * sample[1];
    state->XtX[1][2] += sample[1] * sample[2];
    state->XtX[1][3] += sample[1];

    state->XtX[2][0] += sample[2] * sample[0];
    state->XtX[2][1] += sample[2] * sample[1];
    state->XtX[2][2] += sample[2] * sample[2];
    state->XtX[2][3] += sample[2];

    state->XtX[3][0] += sample[0];
    state->XtX[3][1] += sample[1];
    state->XtX[3][2] += sample[2];
    state->XtX[3][3] += 1;

    float squareSum = (sample[0] * sample[0]) + (sample[1] * sample[1]) + (sample[2] * sample[2]);
    state->XtY[0] += sample[0] * squareSum;
    state->XtY[1] += sample[1] * squareSum;
    state->XtY[2] += sample[2] * squareSum;
    state->XtY[3] += squareSum;
}

void sensorCalibrationPushSampleForScaleCalculation(sensorCalibrationState_t * state, int axis, float sample[3], int target)
{
    for (int i = 0; i < 3; i++) {
        float scaledSample = (float)sample[i] / (float)target;
        state->XtX[axis][i] += scaledSample * scaledSample;
        state->XtX[3][i] += scaledSample * scaledSample;
    }

    state->XtX[axis][3] += 1;
    state->XtY[axis] += 1;
    state->XtY[3] += 1;
}

static void sensorCalibration_gaussLR(float mat[4][4]) {
    uint8_t n = 4;
    int i, j, k;
    for (i = 0; i < 4; i++) {
        // Determine R
        for (j = i; j < 4; j++) {
            for (k = 0; k < i; k++) {
                mat[i][j] -= mat[i][k] * mat[k][j];
            }
        }
        // Determine L
        for (j = i + 1; j < n; j++) {
            for (k = 0; k < i; k++) {
                mat[j][i] -= mat[j][k] * mat[k][i];
            }
            mat[j][i] /= mat[i][i];
        }
    }
}

void sensorCalibration_ForwardSubstitution(float LR[4][4], float y[4], float b[4]) {
    int i, k;
    for (i = 0; i < 4; ++i) {
        y[i] = b[i];
        for (k = 0; k < i; ++k) {
            y[i] -= LR[i][k] * y[k];
        }
        //y[i] /= MAT_ELEM_AT(LR,i,i); //Do not use, LR(i,i) is 1 anyways and not stored in this matrix
    }
}

void sensorCalibration_BackwardSubstitution(float LR[4][4], float x[4], float y[4]) {
    int i, k;
    for (i = 3 ; i >= 0; --i) {
        x[i] = y[i];
        for (k = i + 1; k < 4; ++k) {
            x[i] -= LR[i][k] * x[k];
        }
        x[i] /= LR[i][i];
    }
}

// solve linear equation
// https://en.wikipedia.org/wiki/Gaussian_elimination
static void sensorCalibration_SolveLGS(float A[4][4], float x[4], float b[4]) {
    int i;
    float y[4];

    sensorCalibration_gaussLR(A);

    for (i = 0; i < 4; ++i) {
        y[i] = 0;
    }

    sensorCalibration_ForwardSubstitution(A, y, b);
    sensorCalibration_BackwardSubstitution(A, x, y);
}

bool sensorCalibrationValidateResult(const float result[3])
{
    // Validate that result is not INF and not NAN
    for (int i = 0; i < 3; i++) {
        if (isnan(result[i]) && isinf(result[i])) {
            return false;
        }
    }

    return true;
}

bool sensorCalibrationSolveForOffset(sensorCalibrationState_t * state, float result[3])
{
    float beta[4];
    sensorCalibration_SolveLGS(state->XtX, beta, state->XtY);

    for (int i = 0; i < 3; i++) {
        result[i] = beta[i] / 2;
    }

    return sensorCalibrationValidateResult(result);
}

bool sensorCalibrationSolveForScale(sensorCalibrationState_t * state, float result[3])
{
    float beta[4];
    sensorCalibration_SolveLGS(state->XtX, beta, state->XtY);

    for (int i = 0; i < 3; i++) {
        result[i] = fast_fsqrtf(beta[i]);
    }

    return sensorCalibrationValidateResult(result);
}

float gaussian(const float x, const float mu, const float sigma) {
    return exp(-pow((double)(x - mu), 2) / (2 * pow((double)sigma, 2)));
}

float bellCurve(const float x, const float curveWidth)
{
    return gaussian(x, 0.0f, curveWidth);
}

/**
 * @brief Calculate the attenuation of a value using a Gaussian function.
 * Retuns 1 for input 0 and ~0 for input width.
 * @param input The input value.
 * @param width The width of the Gaussian function.
 * @return The attenuation of the input value.
*/
float attenuation(const float input, const float width) {
    const float sigma = width / 2.35482f; // Approximately width / sqrt(2 * ln(2))
    return gaussian(input, 0.0f, sigma);
}

float fast_fsqrtf(const float value) {
    float ret = 0.0f;
#ifdef USE_ARM_MATH
    arm_sqrt_f32(value, &ret);
#else
    ret = sqrtf(value);
#endif
    if (isnan(ret))
    {
        return 0.0f;
    }
    return ret;
}

// function to calculate the normalization (pythagoras) of a 2-dimensional vector
float NOINLINE calc_length_pythagorean_2D(const float firstElement, const float secondElement)
{
    return fast_fsqrtf(sq(firstElement) + sq(secondElement));
}

// function to calculate the normalization (pythagoras) of a 3-dimensional vector
float NOINLINE calc_length_pythagorean_3D(const float firstElement, const float secondElement, const float thirdElement)
{
    return fast_fsqrtf(sq(firstElement) + sq(secondElement) + sq(thirdElement));
}

#ifdef SITL_BUILD

/**
 * @brief Floating-point vector subtraction, equivalent of CMSIS arm_sub_f32.
*/
void arm_sub_f32(
  float * pSrcA,
  float * pSrcB,
  float * pDst,
  uint32_t blockSize)
{
    for (uint32_t i = 0; i < blockSize; i++) {
        pDst[i] = pSrcA[i] - pSrcB[i];
    }
}

/**
 * @brief Floating-point vector scaling, equivalent of CMSIS arm_scale_f32.
*/
void arm_scale_f32(
  float * pSrc,
  float scale,
  float * pDst,
  uint32_t blockSize)
{
    for (uint32_t i = 0; i < blockSize; i++) {
        pDst[i] = pSrc[i] * scale;
    }
}

/**
 * @brief Floating-point vector multiplication, equivalent of CMSIS arm_mult_f32.
*/
void arm_mult_f32(
  float * pSrcA,
  float * pSrcB,
  float * pDst,
  uint32_t blockSize)
{
    for (uint32_t i = 0; i < blockSize; i++) {
        pDst[i] = pSrcA[i] * pSrcB[i];
    }
}

#endif