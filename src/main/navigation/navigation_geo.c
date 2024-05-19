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

/*
    Calculate magnetic field intensity and orientation
*/
bool getMagFieldEF(float latitude, float longitude, float *intensity_gauss, float *declination_deg, float *inclination_deg)
{
    bool valid_input_data = true;

    const float latitude_deg = latitude * 1.0e-7f;
    const float longitude_deg = longitude * 1.0e-7f;

    float min_lat = (floorf(latitude_deg / SAMPLING_RES)) * SAMPLING_RES;
    float min_lon = (floorf(longitude_deg / SAMPLING_RES)) * SAMPLING_RES;

    // for the rare case of hitting the bounds exactly the rounding logic wouldn't fit, so enforce it.

    /* limit to table bounds - required for maxima even when table spans full globe range */
    if (latitude_deg <= SAMPLING_MIN_LAT) {
        min_lat = SAMPLING_MIN_LAT;
        valid_input_data = false;
    }

    if (latitude_deg >= SAMPLING_MAX_LAT) {
        min_lat = (latitude_deg / SAMPLING_RES) * SAMPLING_RES - SAMPLING_RES;
        valid_input_data = false;
    }

    if (longitude_deg <= SAMPLING_MIN_LON) {
        min_lon = SAMPLING_MIN_LON;
        valid_input_data = false;
    }

    if (longitude_deg >= SAMPLING_MAX_LON) {
        min_lon = (longitude_deg / SAMPLING_RES) * SAMPLING_RES - SAMPLING_RES;
        valid_input_data = false;
    }

    #define LAT_TABLE_SIZE 19
    #define LON_TABLE_SIZE 37

    /* find index of nearest low sampling point */
    uint32_t min_lat_index = constrain((uint32_t)((-(SAMPLING_MIN_LAT) + min_lat) / SAMPLING_RES), 0, LAT_TABLE_SIZE - 2);
    uint32_t min_lon_index = constrain((uint32_t)((-(SAMPLING_MIN_LON) + min_lon) / SAMPLING_RES), 0, LON_TABLE_SIZE -2);

    /* calculate intensity */

    float data_sw = intensity_table[min_lat_index][min_lon_index];
    float data_se = intensity_table[min_lat_index][min_lon_index + 1];
    float data_ne = intensity_table[min_lat_index + 1][min_lon_index + 1];
    float data_nw = intensity_table[min_lat_index + 1][min_lon_index];

    /* perform bilinear interpolation on the four grid corners */

    float data_min = ((longitude_deg - min_lon) / SAMPLING_RES) * (data_se - data_sw) + data_sw;
    float data_max = ((longitude_deg - min_lon) / SAMPLING_RES) * (data_ne - data_nw) + data_nw;

    *intensity_gauss = ((latitude_deg - min_lat) / SAMPLING_RES) * (data_max - data_min) + data_min;

    /* calculate declination */

    data_sw = declination_table[min_lat_index][min_lon_index];
    data_se = declination_table[min_lat_index][min_lon_index + 1];
    data_ne = declination_table[min_lat_index + 1][min_lon_index + 1];
    data_nw = declination_table[min_lat_index + 1][min_lon_index];

    /* perform bilinear interpolation on the four grid corners */

    data_min = ((longitude_deg - min_lon) / SAMPLING_RES) * (data_se - data_sw) + data_sw;
    data_max = ((longitude_deg - min_lon) / SAMPLING_RES) * (data_ne - data_nw) + data_nw;

    *declination_deg = ((latitude_deg - min_lat) / SAMPLING_RES) * (data_max - data_min) + data_min;

    /* calculate inclination */

    data_sw = inclination_table[min_lat_index][min_lon_index];
    data_se = inclination_table[min_lat_index][min_lon_index + 1];
    data_ne = inclination_table[min_lat_index + 1][min_lon_index + 1];
    data_nw = inclination_table[min_lat_index + 1][min_lon_index];

    /* perform bilinear interpolation on the four grid corners */

    data_min = ((longitude_deg - min_lon) / SAMPLING_RES) * (data_se - data_sw) + data_sw;
    data_max = ((longitude_deg - min_lon) / SAMPLING_RES) * (data_ne - data_nw) + data_nw;

    *inclination_deg = ((latitude_deg - min_lat) / SAMPLING_RES) * (data_max - data_min) + data_min;

    return valid_input_data;
}

float geoCalculateMagDeclination(const gpsLocation_t * llh) // degrees units
{
    float declination_deg = 0, inclination_deg = 0, intensity_gauss = 0;

    getMagFieldEF(llh->lat, llh->lon, &intensity_gauss, &declination_deg, &inclination_deg);

    return declination_deg;
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
