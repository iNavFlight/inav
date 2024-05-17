
/*
 * This file is part of INAV Project.
 *
 * INAV is free software: you can redistribute it and/or modify
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

#include <stdint.h>

#include "platform.h"

#if defined(USE_OSD) && defined(USE_MSP_DISPLAYPORT) && !defined(DISABLE_MSP_DJI_COMPAT)
#include "osd.h"
uint8_t getDJICharacter(uint8_t ch, uint8_t page);
#define isDJICompatibleVideoSystem(osdConfigPtr) (osdConfigPtr->video_system == VIDEO_SYSTEM_DJICOMPAT || osdConfigPtr->video_system == VIDEO_SYSTEM_DJICOMPAT_HD)
#else
#define getDJICharacter(x, page) (x)
#ifdef OSD_UNIT_TEST
#define isDJICompatibleVideoSystem(osdConfigPtr) (true)
#else
#define isDJICompatibleVideoSystem(osdConfigPtr) (false)
#endif
#endif