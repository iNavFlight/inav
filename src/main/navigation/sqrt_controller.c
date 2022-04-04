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

/*
    Square Root Controller calculates the correction based on a proportional controller (kP). 
    Used only in AutoPilot System for Vertical (Z) control.
*/

// Inverse of the sqrt controller. Calculates the input (aka error) to the sqrt_controller required to achieve a given output
static float sqrtControllerInverse(float kp, float derivative_max, float output)
{
    if ((derivative_max > 0.0f) && (kp == 0.0f)) {
        return (output * output) / (2.0f * derivative_max);
    }

    if (((derivative_max < 0.0f) ||(derivative_max == 0.0f)) && (kp != 0.0f)) {
        return output / kp;
    }

    if (((derivative_max < 0.0f) || (derivative_max == 0.0f)) && (kp == 0.0f)) {
        return 0.0f;
    }

    // Calculate the velocity at which we switch from calculating the stopping point using a linear function to a sqrt function
    const float linear_velocity = derivative_max / kp;

    if (fabsf(output) < linear_velocity) {
        // If our current velocity is below the cross-over point we use a linear function
        return output / kp;
    }

    const float linear_dist = derivative_max / sq(kp);
    const float stopping_dist = (linear_dist * 0.5f) + sq(output) / (2.0f * derivative_max);
    return (output > 0.0f) ? stopping_dist : -stopping_dist;
}

// Proportional controller with piecewise sqrt sections to constrain derivative
float sqrtControllerApply(sqrt_controller_t *sqrt_controller_pointer, float target, float measurement, float deltaTime)
{
    float correction_rate;

    // Calculate distance error
    sqrt_controller_pointer->error = target - measurement;

    if ((sqrt_controller_pointer->error_min < 0.0f) && (sqrt_controller_pointer->error < sqrt_controller_pointer->error_min)) {
        sqrt_controller_pointer->error = sqrt_controller_pointer->error_min;
    } else if ((sqrt_controller_pointer->error_max > 0.0f) && (sqrt_controller_pointer->error > sqrt_controller_pointer->error_max)) {
        sqrt_controller_pointer->error = sqrt_controller_pointer->error_max;
    }

    if ((sqrt_controller_pointer->derivative_max < 0.0f) || sqrt_controller_pointer->derivative_max == 0.0f) {
        // Derivative Max is zero or negative.
        correction_rate = sqrt_controller_pointer->error * sqrt_controller_pointer->kp;
    } else if (sqrt_controller_pointer->kp == 0.0f) {
        // Proportional term is zero but we have a Derivative Max.
        if (sqrt_controller_pointer->error > 0.0f) {
            correction_rate = fast_fsqrtf(2.0f * sqrt_controller_pointer->derivative_max * (sqrt_controller_pointer->error));
        } else if (sqrt_controller_pointer->error < 0.0f) {
            correction_rate = -fast_fsqrtf(2.0f * sqrt_controller_pointer->derivative_max * (-sqrt_controller_pointer->error));
        } else {
            correction_rate = 0.0f;
        }
    } else {
        // Both the Proportional and Derivative Max have been defined.
        const float linear_dist = sqrt_controller_pointer->derivative_max / sq(sqrt_controller_pointer->kp);
        if (sqrt_controller_pointer->error > linear_dist) {
            correction_rate = fast_fsqrtf(2.0f * sqrt_controller_pointer->derivative_max * (sqrt_controller_pointer->error - (linear_dist / 2.0f)));
        } else if (sqrt_controller_pointer->error < -linear_dist) {
            correction_rate = -fast_fsqrtf(2.0f * sqrt_controller_pointer->derivative_max * (-sqrt_controller_pointer->error - (linear_dist / 2.0f)));
        } else {
            correction_rate = sqrt_controller_pointer->error * sqrt_controller_pointer->kp;
        }
    }

    if (deltaTime != 0.0f) {
        // This ensures we do not get small oscillations by over shooting the error correction in the last time step.
        return constrainf(correction_rate, -fabsf(sqrt_controller_pointer->error) / deltaTime, fabsf(sqrt_controller_pointer->error) / deltaTime);
    } 

    return correction_rate; 
}

// Sets the maximum error to limit output and derivative of output
void sqrtControllerInit(sqrt_controller_t *sqrt_controller_pointer,const float kp,const float output_min,const float output_max,const float derivative_out_max)
{

    // Reset the variables
    sqrt_controller_pointer->kp = kp;
    sqrt_controller_pointer->derivative_max = 0.0f;
    sqrt_controller_pointer->error_min = 0.0f;
    sqrt_controller_pointer->error_max = 0.0f;

    if (derivative_out_max > 0.0f) {
        sqrt_controller_pointer->derivative_max = derivative_out_max;
    }

    if ((output_min < 0.0f) && (sqrt_controller_pointer->kp > 0.0f)) {
        sqrt_controller_pointer->error_min = sqrtControllerInverse(sqrt_controller_pointer->kp, sqrt_controller_pointer->derivative_max, output_min);
    }

    if ((output_max > 0.0f) && (sqrt_controller_pointer->kp > 0.0f)) {
        sqrt_controller_pointer->error_max = sqrtControllerInverse(sqrt_controller_pointer->kp, sqrt_controller_pointer->derivative_max, output_max);
    }
}