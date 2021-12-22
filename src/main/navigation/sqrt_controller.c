/*
 * This file is part of iNav.
 *
 * iNav is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * iNav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with iNav.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "platform.h"

#include "common/maths.h"

#include "navigation/sqrt_controller.h"

// proportional controller with piecewise sqrt sections to constrainf second derivative
static float sqrt_controller(sqrt_controller_t *sqrt_controller_pointer, float deltaTime)
{
    float correction_rate;

    if ((sqrt_controller_pointer->derivative_max < 0.0f) || sqrt_controller_pointer->derivative_max == 0) {
        // second order limit is zero or negative.
        correction_rate = sqrt_controller_pointer->error * sqrt_controller_pointer->kp;
    } else if (sqrt_controller_pointer->kp == 0) {
        // P term is zero but we have a second order limit.
        if (sqrt_controller_pointer->error > 0.0f) {
            correction_rate = fast_fsqrtf(2.0 * sqrt_controller_pointer->derivative_max * (sqrt_controller_pointer->error));
        } else if (sqrt_controller_pointer->error < 0.0f) {
            correction_rate = -fast_fsqrtf(2.0 * sqrt_controller_pointer->derivative_max * (-sqrt_controller_pointer->error));
        } else {
            correction_rate = 0.0;
        }
    } else {
        // Both the P and second order limit have been defined.
        const float linear_dist = sqrt_controller_pointer->derivative_max / sq(sqrt_controller_pointer->kp);
        if (sqrt_controller_pointer->error > linear_dist) {
            correction_rate = fast_fsqrtf(2.0 * sqrt_controller_pointer->derivative_max * (sqrt_controller_pointer->error - (linear_dist / 2.0)));
        } else if (sqrt_controller_pointer->error < -linear_dist) {
            correction_rate = -fast_fsqrtf(2.0 * sqrt_controller_pointer->derivative_max * (-sqrt_controller_pointer->error - (linear_dist / 2.0)));
        } else {
            correction_rate = sqrt_controller_pointer->error * sqrt_controller_pointer->kp;
        }
    }

    if (deltaTime != 0) {
        // this ensures we do not get small oscillations by over shooting the error correction in the last time step.
        return constrainf(correction_rate, -fabsf(sqrt_controller_pointer->error) / deltaTime, fabsf(sqrt_controller_pointer->error) / deltaTime);
    } 

    return correction_rate; 
}

// inverse of the sqrt controller.  calculates the input (aka error) to the sqrt_controller required to achieve a given output
static float inv_sqrt_controller(sqrt_controller_t *sqrt_controller_pointer, float output)
{
    if ((sqrt_controller_pointer->derivative_max > 0.0f) && (sqrt_controller_pointer->kp == 0)) {
        return (output * output) / (2.0 * sqrt_controller_pointer->derivative_max);
    }

    if (((sqrt_controller_pointer->derivative_max < 0.0f) ||(sqrt_controller_pointer->derivative_max == 0)) && (sqrt_controller_pointer->kp != 0)) {
        return output / sqrt_controller_pointer->kp;
    }

    if (((sqrt_controller_pointer->derivative_max < 0.0f) || (sqrt_controller_pointer->derivative_max == 0)) && (sqrt_controller_pointer->kp == 0)) {
        return 0.0;
    }

    // calculate the velocity at which we switch from calculating the stopping point using a linear function to a sqrt function
    const float linear_velocity = sqrt_controller_pointer->derivative_max / sqrt_controller_pointer->kp;

    if (fabsf(output) < linear_velocity) {
        // if our current velocity is below the cross-over point we use a linear function
        return output / sqrt_controller_pointer->kp;
    }

    const float linear_dist = sqrt_controller_pointer->derivative_max / sq(sqrt_controller_pointer->kp);
    const float stopping_dist = (linear_dist * 0.5f) + sq(output) / (2.0 * sqrt_controller_pointer->derivative_max);
    return (output > 0.0f) ? stopping_dist : -stopping_dist;
}

float get_sqrt_controller(sqrt_controller_t *sqrt_controller_pointer, float target, float measurement, float deltaTime)
{
    // calculate distance p_error
    sqrt_controller_pointer->error = target - measurement;

    if ((sqrt_controller_pointer->error_min < 0.0f) && (sqrt_controller_pointer->error < sqrt_controller_pointer->error_min)) {
        sqrt_controller_pointer->error = sqrt_controller_pointer->error_min;
        target = measurement + sqrt_controller_pointer->error;
    } else if ((sqrt_controller_pointer->error_max > 0.0f) && (sqrt_controller_pointer->error > sqrt_controller_pointer->error_max)) {
        sqrt_controller_pointer->error = sqrt_controller_pointer->error_max;
        target = measurement + sqrt_controller_pointer->error;
    }

    return sqrt_controller(&sqrt_controller_pointer, deltaTime);
}

void sqrt_controller_set_limits(sqrt_controller_t *sqrt_controller_pointer, float output_min, float output_max, float derivative_out_max)
{
    // reset the variables
    sqrt_controller_pointer->derivative_max = 0.0f;
    sqrt_controller_pointer->error_min = 0.0f;
    sqrt_controller_pointer->error_max = 0.0f;

    if (derivative_out_max > 0.0f) {
        sqrt_controller_pointer->derivative_max = derivative_out_max;
    }

    if ((output_min > 0.0f) && (sqrt_controller_pointer->kp > 0.0f)) {
        sqrt_controller_pointer->error_min = inv_sqrt_controller(&sqrt_controller_pointer, output_min);
    }

    if ((output_max > 0.0f) && (sqrt_controller_pointer->kp > 0.0f)) {
        sqrt_controller_pointer->error_max = inv_sqrt_controller(&sqrt_controller_pointer, output_max);
    }
}