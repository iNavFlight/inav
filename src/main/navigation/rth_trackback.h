/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "common/vector.h"

#define NAV_RTH_TRACKBACK_POINTS                50 // max number RTH trackback points
#define NAV_RTH_TRACKBACK_MIN_DIST_TO_START     50 // start recording when some distance from home (meters)
#define NAV_RTH_TRACKBACK_MIN_XY_DIST_TO_SAVE   20 // minimum XY distance between two points to store in the buffer (meters)
#define NAV_RTH_TRACKBACK_MIN_Z_DIST_TO_SAVE    10 // minimum Z distance between two points to store in the buffer (meters)
#define NAV_RTH_TRACKBACK_MIN_TRIP_DIST_TO_SAVE 10 // minimum trip distance between two points to store in the buffer (meters)

typedef struct
{
    fpVector3_t pointsList[NAV_RTH_TRACKBACK_POINTS]; // buffer of points stored
    int16_t lastSavedIndex;                           // last trackback point index saved
    int16_t activePointIndex;                         // trackback points counter
    int16_t WrapAroundCounter;                        // stores trackpoint array overwrite index position
} rth_trackback_t;

extern rth_trackback_t rth_trackback;

bool rthTrackBackCanBeActivated(void);
bool rthTrackBackSetNewPosition(void);
void rthTrackBackUpdate(bool forceSaveTrackPoint);
fpVector3_t *getRthTrackBackPosition(void);
void resetRthTrackBack(void);