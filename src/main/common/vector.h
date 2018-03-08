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
#include <math.h>

#include "common/maths.h"

// Floating point 3 vector.
typedef struct fp_vector {
    float X;
    float Y;
    float Z;
} t_fp_vector_def;

typedef union {
    float A[3];
    t_fp_vector_def V;
} t_fp_vector;

static inline float vectorNormSquared(const t_fp_vector * v)
{ 
    return sq(v->V.X) + sq(v->V.Y) + sq(v->V.Z);
}

static inline t_fp_vector * vectorNormalize(t_fp_vector * result, const t_fp_vector * v)
{
    float length = sqrtf(vectorNormSquared(v));
    if (length != 0) {
        result->V.X = v->V.X / length;
        result->V.Y = v->V.Y / length;
        result->V.Z = v->V.Z / length;
    }
    else {
        result->V.X = 0;
        result->V.Y = 0;
        result->V.Z = 0;
    }
    return result;
}

static inline t_fp_vector * vectorCrossProduct(t_fp_vector * result, const t_fp_vector * a, const t_fp_vector * b)
{
    t_fp_vector ab;

    ab.V.X = a->V.Y * b->V.Z - a->V.Z * b->V.Y;
    ab.V.Y = a->V.Z * b->V.X - a->V.X * b->V.Z;
    ab.V.Z = a->V.X * b->V.Y - a->V.Y * b->V.X;

    *result = ab;
    return result;
}

static inline t_fp_vector * vectorAdd(t_fp_vector * result, const t_fp_vector * a, const t_fp_vector * b)
{
    t_fp_vector ab;

    ab.V.X = a->V.X + b->V.X;
    ab.V.Y = a->V.Y + b->V.Y;
    ab.V.Z = a->V.Z + b->V.Z;

    *result = ab;
    return result;
}

static inline t_fp_vector * vectorScale(t_fp_vector * result, const t_fp_vector * a, const float b)
{
    t_fp_vector ab;

    ab.V.X = a->V.X * b;
    ab.V.Y = a->V.Y * b;
    ab.V.Z = a->V.Z * b;

    *result = ab;
    return result;
}
