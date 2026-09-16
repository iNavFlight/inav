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

#include <stdbool.h>
#include <stdint.h>

static inline bool navMissionRelativeWaypointIndexToAbsolute(
    const int16_t relativeIndex,
    const int16_t missionStartIndex,
    const uint16_t missionWaypointCount,
    const uint16_t maximumWaypointCount,
    int16_t *absoluteIndex)
{
    if (!absoluteIndex ||
        relativeIndex < 0 ||
        missionStartIndex < 0 ||
        missionWaypointCount == 0 ||
        missionStartIndex >= maximumWaypointCount) {
        return false;
    }

    const int32_t missionEndIndex = (int32_t)missionStartIndex + missionWaypointCount;
    const int32_t resolvedIndex = (int32_t)missionStartIndex + relativeIndex;
    if (missionEndIndex > maximumWaypointCount ||
        resolvedIndex < missionStartIndex ||
        resolvedIndex >= missionEndIndex) {
        return false;
    }

    *absoluteIndex = (int16_t)resolvedIndex;
    return true;
}
