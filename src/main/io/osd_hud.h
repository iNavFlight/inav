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

#include <stdint.h>


void osdHudClear(void);
void osdHudDrawCrosshair(uint8_t px, uint8_t py);
void osdHudDrawHoming(uint8_t px, uint8_t py);
void osdHudDrawPoi(uint32_t poiDistance, int16_t poiDirection, int32_t poiAltitude, int16_t poiHeading, uint8_t poiSignal, uint16_t poiSymbol);
void osdHudDrawExtras(uint8_t poi_id);
int8_t radarGetNearestPOI(void);

