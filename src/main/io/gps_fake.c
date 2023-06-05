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
#include <time.h>

#include "platform.h"
#include "build/build_config.h"

#if defined(USE_GPS_FAKE)

#include "common/axis.h"

#include "io/gps.h"
#include "io/gps_private.h"

void gpsFakeRestart(void)
{
    // NOP
}

void gpsFakeHandle(void)
{
    gpsProcessNewSolutionData();
}

void gpsFakeSet(
    gpsFixType_e fixType,
    uint8_t numSat,
    int32_t lat, 
    int32_t lon, 
    int32_t alt, 
    int16_t groundSpeed, 
    int16_t groundCourse, 
    int16_t velNED_X,  
    int16_t velNED_Y,  
    int16_t velNED_Z,
    time_t time)
{
    gpsSol.fixType = fixType;
    gpsSol.hdop = gpsSol.fixType == GPS_NO_FIX ? 9999 : 100;
    gpsSol.numSat = numSat;
    
    gpsSol.llh.lat = lat;
    gpsSol.llh.lon = lon;
    gpsSol.llh.alt = alt;
    gpsSol.groundSpeed = groundSpeed;
    gpsSol.groundCourse = groundCourse;
    gpsSol.velNED[X] = velNED_X;
    gpsSol.velNED[Y] = velNED_Y;
    gpsSol.velNED[Z] = velNED_Z;
    gpsSol.eph = 100;
    gpsSol.epv = 100;
    gpsSol.flags.validVelNE = true;
    gpsSol.flags.validVelD = true;
    gpsSol.flags.validEPE = true;
    gpsSol.flags.hasNewData = true;
    
    if (time) {
        struct tm* gTime = gmtime(&time);

        gpsSol.time.year   = (uint16_t)(gTime->tm_year + 1900);
        gpsSol.time.month  = (uint16_t)(gTime->tm_mon + 1);
        gpsSol.time.day    = (uint8_t)gTime->tm_mday;
        gpsSol.time.hours  = (uint8_t)gTime->tm_hour;
        gpsSol.time.minutes = (uint8_t)gTime->tm_min;
        gpsSol.time.seconds = (uint8_t)gTime->tm_sec;
        gpsSol.time.millis  = 0;
        gpsSol.flags.validTime = gpsSol.fixType >= 3;
    }
}
   
#endif
