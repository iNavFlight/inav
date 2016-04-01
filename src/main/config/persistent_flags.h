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

typedef enum {
    FLAG_MAG_CALIBRATION_DONE = 1 << 0,
} persistent_flags_e;

typedef struct persistentFlags_s {
    uint8_t persistentFlags;
} persistentFlags_t;

PG_DECLARE(persistentFlags_t, persistentFlags);

bool persistentFlag(uint8_t mask);
void persistentFlagSet(uint8_t mask);
void persistentFlagClear(uint8_t mask);
void persistentFlagClearAll(void);
