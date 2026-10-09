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

#include "drivers/bus.h"

struct baroDev_s;
typedef bool (*baroOpFuncPtr)(struct baroDev_s * baro);
typedef bool (*baroCalculateFuncPtr)(struct baroDev_s * baro, int32_t *pressure, int32_t *temperature);

/*
 * Measurement cycle driven by baroUpdate(): start_ut -> (ut_delay) -> read_ut -> get_ut -> start_up -> (up_delay) -> read_up -> get_up -> calculate
 *
 * A driver may leave any hook NULL. The optional read_* hooks start a non-blocking bus transfer and must return false
 * when the bus is busy; the matching get_* then only parses the received buffer once the bus reports idle and returns
 * false if the sample is not usable (the phase is restarted). A start_* hook paired with a read_* hook may start a
 * non-blocking write: read_* is only called once that write is over and the phase is restarted if it failed. Drivers
 * without read_* hooks do their blocking bus access inside get_*. A delay of 0 keeps the current task period.
 */
typedef struct baroDev_s {
    busDevice_t * busDev;
    bool combined_read;         // get_up delivers temperature as well, the temperature phase is skipped
    uint16_t ut_delay;
    uint16_t up_delay;
    baroOpFuncPtr start_ut;
    baroOpFuncPtr read_ut;
    baroOpFuncPtr get_ut;
    baroOpFuncPtr start_up;
    baroOpFuncPtr read_up;
    baroOpFuncPtr get_up;
    baroCalculateFuncPtr calculate;
} baroDev_t;
