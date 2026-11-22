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

#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "platform.h"
#include "build/build_config.h"


#if defined(USE_DRONECAN)

#include "build/debug.h"

#include "common/axis.h"
#include "common/gps_conversion.h"
#include "common/maths.h"
#include "common/utils.h"
#include "common/log.h"

#include "drivers/serial.h"
#include "drivers/time.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#include "io/gps.h"
#include "io/gps_private.h"
#include "drivers/dronecan/dronecan.h"

#include <dronecan_msgs.h>

static bool newDataReady;
static uint16_t lastHDOP = 9999;

void gpsRestartDronecan(void)
{
    // NOP
}

void gpsHandleDronecan(void)
{
    if (newDataReady) {
        gpsProcessNewSolutionData(false);
        newDataReady = false;
    }
}

static uint8_t gpsMapFixType(uint8_t dronecanFixType)
{
    if (dronecanFixType == 2)
        return GPS_FIX_2D;
    if (dronecanFixType >= 3)
        return GPS_FIX_3D;
    return GPS_NO_FIX;
}


/*
 * Date and time from the GNSS timestamp. Only UTC timestamps (microseconds
 * since the Unix epoch, as sent by ArduPilot-based GNSS nodes) are used, and
 * only within the range that INAV's RTC (int32 seconds) can represent.
 */
#define DRONECAN_GNSS_TIME_STANDARD_UTC     2
#define DRONECAN_GNSS_MIN_VALID_UNIX_SEC    1577836800LL    // 2020-01-01
#define DRONECAN_GNSS_MAX_VALID_UNIX_SEC    2147483647LL    // 2038-01-19, RTC time is int32 seconds

static void dronecanGPSSetTime(uint64_t usec, uint8_t timeStandard)
{
    const int64_t unixSec = (int64_t)(usec / 1000000ULL);

    if (timeStandard != DRONECAN_GNSS_TIME_STANDARD_UTC ||
        unixSec < DRONECAN_GNSS_MIN_VALID_UNIX_SEC || unixSec > DRONECAN_GNSS_MAX_VALID_UNIX_SEC) {
        gpsSolDRV.flags.validTime = false;
        return;
    }

    // Days since epoch -> civil date (H. Hinnant's algorithm)
    const int64_t days = unixSec / 86400;
    const int64_t secOfDay = unixSec % 86400;

    const int64_t z = days + 719468;
    const int64_t era = z / 146097;
    const int64_t doe = z - era * 146097;
    const int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const int64_t mp = (5 * doy + 2) / 153;
    const int64_t day = doy - (153 * mp + 2) / 5 + 1;
    const int64_t month = mp < 10 ? mp + 3 : mp - 9;
    const int64_t year = yoe + era * 400 + (month <= 2 ? 1 : 0);

    gpsSolDRV.time.year    = (uint16_t)year;
    gpsSolDRV.time.month   = (uint8_t)month;
    gpsSolDRV.time.day     = (uint8_t)day;
    gpsSolDRV.time.hours   = (uint8_t)(secOfDay / 3600);
    gpsSolDRV.time.minutes = (uint8_t)((secOfDay % 3600) / 60);
    gpsSolDRV.time.seconds = (uint8_t)(secOfDay % 60);
    gpsSolDRV.time.millis  = (uint16_t)((usec / 1000ULL) % 1000ULL);

    gpsSolDRV.flags.validTime = true;
}

void dronecanGPSReceiveGNSSFix(const struct uavcan_equipment_gnss_Fix * pgnssFix)
{
    gpsSolDRV.fixType   = gpsMapFixType(pgnssFix->status);
    gpsSolDRV.numSat    = pgnssFix->sats_used;
    gpsSolDRV.llh.lon   = pgnssFix->longitude_deg_1e8 / 10; // convert to deg_1e7
    gpsSolDRV.llh.lat   = pgnssFix->latitude_deg_1e8 / 10; // convert to deg_1e7
    gpsSolDRV.llh.alt   = pgnssFix->height_msl_mm / 10; // convert to cm
    gpsSolDRV.velNED[X] = pgnssFix->ned_velocity[0] * 100; // Dronecan is North, East, Down
    gpsSolDRV.velNED[Y] = pgnssFix->ned_velocity[1] * 100;
    gpsSolDRV.velNED[Z] = pgnssFix->ned_velocity[2] * 100;
    gpsSolDRV.groundSpeed = calc_length_pythagorean_2D((float)pgnssFix->ned_velocity[0], (float)pgnssFix->ned_velocity[1]) * 100;
    float groundCourse = atan2_approx(pgnssFix->ned_velocity[1], pgnssFix->ned_velocity[0]); // atan2 returns [-M_PI, M_PI], with 0 indicating the vector points in the X direction
    if (groundCourse < 0) {
        groundCourse += 2 * M_PIf;
    }
    gpsSolDRV.groundCourse = RADIANS_TO_DECIDEGREES(groundCourse);
    // TODO where to get EPH gpsSolDRV.eph = gpsConstrainEPE(pgnssFix-> / 10);
    // TODO where to get EPV gpsSolDRV.epv = gpsConstrainEPE(pkt->verticalPosAccuracy / 10);
    if(pgnssFix->pdop > 0){
        gpsSolDRV.hdop = gpsConstrainHDOP(pgnssFix->pdop * 100);  // Only update if populated
    } else if((9999 > lastHDOP) && (lastHDOP> 0)) {
        gpsSolDRV.hdop = lastHDOP;
    }
    gpsSolDRV.flags.validVelNE = true;
    gpsSolDRV.flags.validVelD = true;
    gpsSolDRV.flags.validEPE = false;  // assume invalid unless the covariance is filled in.
    if (pgnssFix->position_covariance.len >= 6) {
        float var_x = pgnssFix->position_covariance.data[0];  // meters²
        float var_y = pgnssFix->position_covariance.data[2];  // meters²
        float var_z = pgnssFix->position_covariance.data[5];  // meters²

        gpsSolDRV.eph = gpsConstrainEPE((uint32_t)(sqrtf(var_x + var_y) * 100));  // cm
        gpsSolDRV.epv = gpsConstrainEPE((uint32_t)(sqrtf(var_z) * 100));          // cm
        gpsSolDRV.flags.validEPE = true;
    } 

    dronecanGPSSetTime(pgnssFix->gnss_timestamp.usec, pgnssFix->gnss_time_standard);

    gpsStats.packetCount++;
    gpsProcessNewDriverData();
    newDataReady = true;
}

void dronecanGPSReceiveGNSSFix2(const struct uavcan_equipment_gnss_Fix2 * pgnssFix2)
{
    gpsSolDRV.fixType   = gpsMapFixType(pgnssFix2->status);
    gpsSolDRV.numSat    = pgnssFix2->sats_used;
    gpsSolDRV.llh.lon   = pgnssFix2->longitude_deg_1e8 / 10; // convert to deg_1e7
    gpsSolDRV.llh.lat   = pgnssFix2->latitude_deg_1e8 / 10; // convert to deg_1e7
    gpsSolDRV.llh.alt   = pgnssFix2->height_msl_mm / 10; // convert to cm
    gpsSolDRV.velNED[X] = pgnssFix2->ned_velocity[0] * 100; // Dronecan is North, East, Down
    gpsSolDRV.velNED[Y] = pgnssFix2->ned_velocity[1] * 100;
    gpsSolDRV.velNED[Z] = pgnssFix2->ned_velocity[2] * 100;
    gpsSolDRV.groundSpeed = calc_length_pythagorean_2D((float)pgnssFix2->ned_velocity[0], (float)pgnssFix2->ned_velocity[1]) * 100;
    float groundCourse = atan2_approx(pgnssFix2->ned_velocity[1], pgnssFix2->ned_velocity[0]); // atan2 returns [-M_PI, M_PI], with 0 indicating the vector points in the X direction
    if (groundCourse < 0) {
        groundCourse += 2 * M_PIf;
    }
    gpsSolDRV.groundCourse = RADIANS_TO_DECIDEGREES(groundCourse);
    // TODO where to get EPH gpsSolDRV.eph = gpsConstrainEPE(pgnssFix-> / 10);
    // TODO where to get EPV gpsSolDRV.epv = gpsConstrainEPE(pkt->verticalPosAccuracy / 10);
    if (pgnssFix2->pdop > 0){
        gpsSolDRV.hdop = gpsConstrainHDOP(pgnssFix2->pdop * 100); // Only update if valid.
    } else if((9999 > lastHDOP) && (lastHDOP > 0)) {
        gpsSolDRV.hdop = lastHDOP;
    }
    gpsSolDRV.flags.validVelNE = true;
    gpsSolDRV.flags.validVelD = true;
    gpsSolDRV.flags.validEPE = false;  // assume invalid unless the covariance is filled in.
    if (pgnssFix2->covariance.len >= 6) {
        float var_x = pgnssFix2->covariance.data[0];  // meters²
        float var_y = pgnssFix2->covariance.data[2];  // meters²
        float var_z = pgnssFix2->covariance.data[5];  // meters²

        gpsSolDRV.eph = gpsConstrainEPE((uint32_t)(sqrtf(var_x + var_y) * 100));  // cm
        gpsSolDRV.epv = gpsConstrainEPE((uint32_t)(sqrtf(var_z) * 100));          // cm
        gpsSolDRV.flags.validEPE = true;
    } 
    dronecanGPSSetTime(pgnssFix2->gnss_timestamp.usec, pgnssFix2->gnss_time_standard);

    gpsStats.packetCount++;
    gpsProcessNewDriverData();
    newDataReady = true;
}

void dronecanGPSReceiveGNSSAuxiliary(const struct uavcan_equipment_gnss_Auxiliary * pgnssAux)
{
    // DroneCAN float16 optional fields encode NaN when unpopulated; guard before use.
    // gpsConstrainHDOP clamps to 9999 preventing uint16_t overflow for extreme DOP values.
    if (!isnan(pgnssAux->hdop)) {
        lastHDOP = gpsConstrainHDOP((uint32_t)(pgnssAux->hdop * 100));
    }
}
#endif