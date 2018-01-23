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

#include "common/time.h"

#ifdef LIGHTS

//#define FAILSAFE_LIGHTS

#ifndef LIGHTS_OUTPUT_MODE
    #define LIGHTS_OUTPUT_MODE IOCFG_OUT_PP
#endif

#ifndef FAILSAFE_LIGHTS_ON_TIME
    #define FAILSAFE_LIGHTS_ON_TIME 100 // ms
#endif
#ifndef FAILSAFE_LIGHTS_OFF_TIME
    #define FAILSAFE_LIGHTS_OFF_TIME 900 // ms
#endif


void lightsUpdate(timeUs_t currentTimeUs);
void lightsInit();

#endif /* LIGHTS */
