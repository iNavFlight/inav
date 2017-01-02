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

typedef struct opflow_data_s {
    union {
        int16_t A[2];
        struct {
            int16_t X;
            int16_t Y;
        } V;
    } delta;

    int16_t quality;
} opflow_data_t;

typedef struct opflowDev_s {
    sensorInitFuncPtr init;     // initialize function
    sensorReadFuncPtr read;     // read DX, DY (in pixels) and surface quality (0-255)
    bool hasSoftSPI;            // Has software SPI
} opflowDev_t;
