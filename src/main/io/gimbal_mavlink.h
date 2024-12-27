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

#include <stdint.h>

#include "platform.h"

#include "common/time.h"
#include "drivers/gimbal_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_GIMBAL_MAVLINK

bool gimbalMavlinkInit(void);
bool gimbalMavlinkDetect(void);
void gimbalMavlinkProcess(const gimbalDevice_t *gimbalDevice, timeUs_t currentTime);
bool gimbalMavlinkIsReady(const gimbalDevice_t *gimbalDevice);
bool gimbalMavlinkHasHeadTracker(const gimbalDevice_t *gimbalDevice);
gimbalDevType_e gimbalMavlinkGetDeviceType(const gimbalDevice_t *gimbalDevice);

#endif

#ifdef __cplusplus
}
#endif