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

#pragma once

typedef struct sqrt_controller_s {
float kp;             // proportional gain
float error;          // proportional error calced
float error_min;      // error limit in negative direction
float error_max;      // error limit in positive direction
float derivative_max; // maximum derivative of output
} sqrt_controller_t;


// return the sqrt_controller calced
float get_sqrt_controller(sqrt_controller_t *sqrt_controller_pointer, float target, float measurement, float deltaTime);

// sets the maximum error to limit output and first and second derivative of output
void sqrt_controller_set_limits(sqrt_controller_t *sqrt_controller_pointer, float output_min, float output_max, float derivative_out_max);