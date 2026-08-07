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

typedef struct sqrt_controller_s { 
    float kp;             // proportional gain
    float error;          // error calced
    float error_min;      // error limit in negative direction
    float error_max;      // error limit in positive direction
    float derivative_max; // maximum derivative of output
} sqrt_controller_t;

float sqrtControllerApply(sqrt_controller_t *sqrt_controller_pointer, float target, float measurement, float deltaTime);
void sqrtControllerInit(sqrt_controller_t *sqrt_controller_pointer, const float kp, const float output_min, const float output_max, const float derivative_out_max);