
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

#if defined(USE_MSP_DISPLAYPORT) && !defined(DISABLE_MSP_BF_COMPAT)
#include "osd.h"
uint8_t getBfCharacter(uint8_t ch, uint8_t page);
#define isBfCompatibleVideoSystem(osdConfigPtr) (osdConfigPtr->video_system == VIDEO_SYSTEM_BFCOMPAT)
#else
#define getBfCharacter(x, page) (x)
#define isBfCompatibleVideoSystem(osdConfigPtr) (false)
#endif