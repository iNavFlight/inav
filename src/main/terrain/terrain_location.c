/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <string.h>
#include "platform.h"

#ifdef USE_TERRAIN

#include "terrain_location.h"

static float longitudeScale(int32_t lat)
{
    float scale = cosf(lat * (1.0e-7f * DEG2RAD));
    return MAX(scale, 0.01f);
}

static int32_t limitLat(int32_t lat)
{
    if (lat > 900000000L) {
        lat = 1800000000LL - lat;
    } else if (lat < -900000000L) {
        lat = -(1800000000LL + lat);
    }
    return lat;
}

static int32_t wrapLongitude(int64_t lon)
{
    if (lon > 1800000000L) {
        lon = (int32_t)lon-3600000000LL;
    } else if (lon < -1800000000L) {
        lon = (int32_t)lon+3600000000LL;
    }
    return (int32_t) lon;
}

static int32_t diffLongitude(int32_t lon1, int32_t lon2)
{
    if ((lon1 & 0x80000000) == (lon2 & 0x80000000)) {
        // common case of same sign
        return lon1 - lon2;
    }
    int64_t dlon = (int64_t) lon1 - (int64_t) lon2;
    if (dlon > 1800000000LL) {
        dlon -= 3600000000LL;
    } else if (dlon < -1800000000LL) {
        dlon += 3600000000LL;
    }
    return (int32_t) dlon;
}

void offsetLatlng(gpsLocation_t *loc, float north_m, float east_m)
{
    const int32_t dlat = north_m * LOCATION_SCALING_FACTOR_INV;
    const int64_t dlon = (east_m * LOCATION_SCALING_FACTOR_INV) / longitudeScale(loc->lat + (dlat / 2));

    loc->lat += dlat;
    loc->lat = limitLat(loc->lat);
    loc->lon = wrapLongitude(dlon + loc->lon);
}

neVector_t gpsGetDistanceNE(const gpsLocation_t *a, const gpsLocation_t *b)
{
    neVector_t v;
    v.north = (float)(b->lat - a->lat) * LOCATION_SCALING_FACTOR;
    v.east  = (float)diffLongitude(b->lon, a->lon) * LOCATION_SCALING_FACTOR * longitudeScale((b->lat + b->lat)/2);
    return v;
}

#endif