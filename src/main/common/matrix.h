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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>

#include "common/vector.h"
#include "common/axis.h"

typedef struct {
    float m[3][3];
} fpMat3_t;

// Create a rotation matrix given some euler angles
static inline void matrixFromEuler(float roll, float pitch, float yaw, fpMat3_t *rotationMatrix)
{
    const float cp = cos_approx(pitch);
    const float sp = sin_approx(pitch);
    const float sr = sin_approx(roll);
    const float cr = cos_approx(roll);
    const float sy = sin_approx(yaw);
    const float cy = cos_approx(yaw);

    rotationMatrix->m[0][0] = cp * cy;
    rotationMatrix->m[0][1] = (sr * sp * cy) - (cr * sy);
    rotationMatrix->m[0][2] = (cr * sp * cy) + (sr * sy);

    rotationMatrix->m[1][0] = cp * sy;
    rotationMatrix->m[1][1] = (sr * sp * sy) + (cr * cy);
    rotationMatrix->m[1][2] = (cr * sp * sy) - (sr * cy);

    rotationMatrix->m[2][0] = -sp;
    rotationMatrix->m[2][1] = sr * cp;
    rotationMatrix->m[2][2] = cr * cp;
}

// Apply an additional rotation from a Body-Frame gyro vector to a rotation matrix.
static inline void matrixRotate(const fpVector3_t gyro, fpMat3_t *rotationMatrix)
{
    const fpMat3_t rotationMatrix2 = { .m = { { rotationMatrix->m[0][0], rotationMatrix->m[0][1], rotationMatrix->m[0][2] },
                                              { rotationMatrix->m[1][0], rotationMatrix->m[1][1], rotationMatrix->m[1][2] },
                                              { rotationMatrix->m[2][0], rotationMatrix->m[2][1], rotationMatrix->m[2][2] } } };

    rotationMatrix->m[0][0] += rotationMatrix2.m[0][1] * gyro.z - rotationMatrix2.m[0][2] * gyro.y;
    rotationMatrix->m[0][1] += rotationMatrix2.m[0][2] * gyro.x - rotationMatrix2.m[0][0] * gyro.z;
    rotationMatrix->m[0][2] += rotationMatrix2.m[0][0] * gyro.y - rotationMatrix2.m[0][1] * gyro.x;

    rotationMatrix->m[1][0] += rotationMatrix2.m[1][1] * gyro.z - rotationMatrix2.m[1][2] * gyro.y;
    rotationMatrix->m[1][1] += rotationMatrix2.m[1][2] * gyro.x - rotationMatrix2.m[1][0] * gyro.z;
    rotationMatrix->m[1][2] += rotationMatrix2.m[1][0] * gyro.y - rotationMatrix2.m[1][1] * gyro.x;

    rotationMatrix->m[2][0] += rotationMatrix2.m[2][1] * gyro.z - rotationMatrix2.m[2][2] * gyro.y;
    rotationMatrix->m[2][1] += rotationMatrix2.m[2][2] * gyro.x - rotationMatrix2.m[2][0] * gyro.z;
    rotationMatrix->m[2][2] += rotationMatrix2.m[2][0] * gyro.y - rotationMatrix2.m[2][1] * gyro.x;
}

// Multiplication of transpose by a vector
static inline void matrixMulTranspose(fpVector3_t *v, const fpMat3_t rotationMatrix)
{
    const fpVector3_t v2 = { .v = { v->x, v->y, v->z } };

    v->x = rotationMatrix.m[0][0] * v2.x + rotationMatrix.m[1][0] * v2.y + rotationMatrix.m[2][0] * v2.z;
    v->y = rotationMatrix.m[0][1] * v2.x + rotationMatrix.m[1][1] * v2.y + rotationMatrix.m[2][1] * v2.z;
    v->z = rotationMatrix.m[0][2] * v2.x + rotationMatrix.m[1][2] * v2.y + rotationMatrix.m[2][2] * v2.z;                
}

// Multiplication by a vector, extracting only the XY components
static inline void matrixMulXY(fpVector3_t *v, const fpMat3_t rotationMatrix) 
{
    const fpVector3_t v2 = { .v = { v->x, v->y, v->z } };

    v->x = rotationMatrix.m[0][0] * v2.x + rotationMatrix.m[0][1] * v2.y + rotationMatrix.m[0][2] * v2.z;
    v->y = rotationMatrix.m[1][0] * v2.x + rotationMatrix.m[1][1] * v2.y + rotationMatrix.m[1][2] * v2.z; 
}

// Multiplication by a vector
static inline void vectorRowTimesMat(fpVector3_t *v, const fpMat3_t rotationMatrix) 
{
    const fpVector3_t v2 = { .v = { v->x, v->y, v->z } };

    v->x = rotationMatrix.m[0][0] * v2.x + rotationMatrix.m[0][1] * v2.y + rotationMatrix.m[0][2] * v2.z;
    v->y = rotationMatrix.m[1][0] * v2.x + rotationMatrix.m[1][1] * v2.y + rotationMatrix.m[1][2] * v2.z;
    v->z = rotationMatrix.m[2][0] * v2.x + rotationMatrix.m[2][1] * v2.y + rotationMatrix.m[2][2] * v2.z;
}

static inline void matrixIdentity(fpMat3_t *rotationMatrix)
{
    // A
    rotationMatrix->m[0][0] = 1.0f;
    rotationMatrix->m[0][1] = 0.0f;
    rotationMatrix->m[0][2] = 0.0f;
    // B
    rotationMatrix->m[1][0] = 0.0f;
    rotationMatrix->m[1][1] = 1.0f;
    rotationMatrix->m[1][2] = 0.0f;
    // C
    rotationMatrix->m[2][0] = 0.0f;
    rotationMatrix->m[2][1] = 0.0f;
    rotationMatrix->m[2][2] = 1.0f;
}

static inline bool rotationMatrixIsNAN(const fpMat3_t rotationMatrix) 
{

    bool isNaN = false;

    for (uint8_t i = 0; i < 3; i++) {
        for (uint8_t ii = 0; ii < 3; ii++) {
            if (isnan(rotationMatrix.m[i][ii])) {
                isNaN = true;
                break;
            }
        }
    }

    return isNaN;
}