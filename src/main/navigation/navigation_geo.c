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

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "platform.h"

#if defined(USE_NAV)

#include "build/build_config.h"
#include "build/debug.h"

#include "common/axis.h"
#include "common/filter.h"
#include "common/maths.h"

#include "sensors/sensors.h"
#include "sensors/acceleration.h"
#include "sensors/boardalignment.h"

#include "flight/pid.h"
#include "flight/imu.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#include "navigation/navigation_declination_gen.c"

static float get_lookup_table_val(unsigned lat_index, unsigned lon_index)
{
    return declination_table[lat_index][lon_index];
}

float geoCalculateMagDeclination(const gpsLocation_t * llh) // degrees units
{
    /*
     * If the values exceed valid ranges, return zero as default
     * as we have no way of knowing what the closest real value
     * would be.
     */
    const float lat = llh->lat / 10000000.0f;
    const float lon = llh->lon / 10000000.0f;

    if (lat < -90.0f || lat > 90.0f ||
        lon < -180.0f || lon > 180.0f) {
        return 0.0f;
    }

    /* round down to nearest sampling resolution */
    int min_lat = (int)(lat / SAMPLING_RES) * SAMPLING_RES;
    int min_lon = (int)(lon / SAMPLING_RES) * SAMPLING_RES;

    /* for the rare case of hitting the bounds exactly
     * the rounding logic wouldn't fit, so enforce it.
     */

    /* limit to table bounds - required for maxima even when table spans full globe range */
    if (lat <= SAMPLING_MIN_LAT) {
        min_lat = SAMPLING_MIN_LAT;
    }

    if (lat >= SAMPLING_MAX_LAT) {
        min_lat = (int)(lat / SAMPLING_RES) * SAMPLING_RES - SAMPLING_RES;
    }

    if (lon <= SAMPLING_MIN_LON) {
        min_lon = SAMPLING_MIN_LON;
    }

    if (lon >= SAMPLING_MAX_LON) {
        min_lon = (int)(lon / SAMPLING_RES) * SAMPLING_RES - SAMPLING_RES;
    }

    /* find index of nearest low sampling point */
    const unsigned min_lat_index = (-(SAMPLING_MIN_LAT) + min_lat)  / SAMPLING_RES;
    const unsigned min_lon_index = (-(SAMPLING_MIN_LON) + min_lon) / SAMPLING_RES;

    const float declination_sw = get_lookup_table_val(min_lat_index, min_lon_index);
    const float declination_se = get_lookup_table_val(min_lat_index, min_lon_index + 1);
    const float declination_ne = get_lookup_table_val(min_lat_index + 1, min_lon_index + 1);
    const float declination_nw = get_lookup_table_val(min_lat_index + 1, min_lon_index);

    /* perform bilinear interpolation on the four grid corners */

    const float declination_min = ((lon - min_lon) / SAMPLING_RES) * (declination_se - declination_sw) + declination_sw;
    const float declination_max = ((lon - min_lon) / SAMPLING_RES) * (declination_ne - declination_nw) + declination_nw;

    return ((lat - min_lat) / SAMPLING_RES) * (declination_max - declination_min) + declination_min;
}

void geoSetOrigin(gpsOrigin_t *origin, const gpsLocation_t *llh, geoOriginResetMode_e resetMode)
{
    if (resetMode == GEO_ORIGIN_SET) {
        origin->valid = true;
        origin->lat = llh->lat;
        origin->lon = llh->lon;
        origin->alt = llh->alt;
        origin->scale = constrainf(cos_approx((ABS(origin->lat) / 10000000.0f) * 0.0174532925f), 0.01f, 1.0f);
    }
    else if (origin->valid && (resetMode == GEO_ORIGIN_RESET_ALTITUDE)) {
        origin->alt = llh->alt;
    }
}

bool geoConvertGeodeticToLocal(fpVector3_t *pos, const gpsOrigin_t *origin, const gpsLocation_t *llh, geoAltitudeConversionMode_e altConv)
{
    if (origin->valid) {
        pos->x = (llh->lat - origin->lat) * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR;
        pos->y = (llh->lon - origin->lon) * (DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * origin->scale);

        // If flag GEO_ALT_RELATIVE, than llh altitude is already relative to origin
        if (altConv == GEO_ALT_RELATIVE) {
            pos->z = llh->alt;
        } else {
            pos->z = llh->alt - origin->alt;
        }
        return true;
    }

    pos->x = 0.0f;
    pos->y = 0.0f;
    pos->z = 0.0f;
    return false;
}

bool geoConvertGeodeticToLocalOrigin(fpVector3_t * pos, const gpsLocation_t *llh, geoAltitudeConversionMode_e altConv)
{
    return geoConvertGeodeticToLocal(pos, &posControl.gpsOrigin, llh, altConv);
}

bool geoConvertLocalToGeodetic(gpsLocation_t *llh, const gpsOrigin_t * origin, const fpVector3_t *pos)
{
    float scaleLonDown;

    if (origin->valid) {
        llh->lat = origin->lat;
        llh->lon = origin->lon;
        llh->alt = origin->alt;
        scaleLonDown = origin->scale;
    }
    else {
        llh->lat = 0;
        llh->lon = 0;
        llh->alt = 0;
        scaleLonDown = 1.0f;
    }

    llh->lat += lrintf(pos->x / DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR);
    llh->lon += lrintf(pos->y / (DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * scaleLonDown));
    llh->alt += lrintf(pos->z);
    return origin->valid;
}


#endif  // NAV
