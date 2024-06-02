/*
 * This file is part of INAV.
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
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "platform.h"

#ifdef USE_SERIAL_GIMBAL

#include <stdint.h>

#include "config/feature.h"

typedef struct gimbalConfig_s {
    uint8_t yawChannel;
    uint8_t pitchChannel;
    uint8_t rollChannel;
    uint8_t sensitivity;
} gimbalConfig_t;

PG_DECLARE(gimbalConfig_t, gimbalConfig);

typedef enum {
    GIMBAL_MODE_PITCH_ROLL_LOCK = 0,
    GIMBAL_MODE_PITCH_LOCK = 1,
    GIMBAL_MODE_FOLLOW = 2
} gimbal_htk_mode_e;

#define GIMBAL_MODE_DEFAULT = GIMBAL_MODE_FOLLOW;

#endif