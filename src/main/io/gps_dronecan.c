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


#if defined(USE_GPS_PROTO_DRONECAN)

#include "build/debug.h"

#include "common/axis.h"
#include "common/gps_conversion.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/serial.h"
#include "drivers/time.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#include "io/gps.h"
#include "io/gps_private.h"
#include "drivers/dronecan/dronecan.h"

#include <dronecan_msgs.h>

static bool newDataReady;

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

void dronecanGPSReceiveGNSSFix(const struct uavcan_equipment_gnss_Fix * pgnssFix)
{
    //const mspSensorGpsDataMessage_t * pkt = (const mspSensorGpsDataMessage_t *)bufferPtr;

    gpsSolDRV.fixType   = gpsMapFixType(pgnssFix->status);
    gpsSolDRV.numSat    = pgnssFix->sats_used;
    gpsSolDRV.llh.lon   = pgnssFix->longitude_deg_1e8;
    gpsSolDRV.llh.lat   = pgnssFix->latitude_deg_1e8;
    gpsSolDRV.llh.alt   = pgnssFix->height_msl_mm;
    gpsSolDRV.velNED[X] = pgnssFix->ned_velocity[0]; // Dronecan is North, East, Down
    gpsSolDRV.velNED[Y] = pgnssFix->ned_velocity[1];
    gpsSolDRV.velNED[Z] = pgnssFix->ned_velocity[2];
    gpsSolDRV.groundSpeed = calc_length_pythagorean_2D((float)pgnssFix->ned_velocity[0], (float)pgnssFix->ned_velocity[1]);
    float groundCourse = atan2_approx(pgnssFix->ned_velocity[1], pgnssFix->ned_velocity[0]); // atan2 returns [-M_PI, M_PI], with 0 indicating the vector points in the X direction
    if (groundCourse < 0) {
        groundCourse += 2 * M_PIf;
    }
    gpsSolDRV.groundCourse = RADIANS_TO_DECIDEGREES(groundCourse);
    // TODO where to get EPH gpsSolDRV.eph = gpsConstrainEPE(pgnssFix-> / 10);
    // TODO where to get EPV gpsSolDRV.epv = gpsConstrainEPE(pkt->verticalPosAccuracy / 10);
    gpsSolDRV.hdop = gpsConstrainHDOP(pgnssFix->pdop);
    gpsSolDRV.flags.validVelNE = true;
    gpsSolDRV.flags.validVelD = true;
    gpsSolDRV.flags.validEPE = false;

    // gpsSolDRV.time.year   = pkt->year;
    // gpsSolDRV.time.month  = pkt->month;
    // gpsSolDRV.time.day    = pkt->day;
    // gpsSolDRV.time.hours  = pkt->hour;
    // gpsSolDRV.time.minutes = pkt->min;
    // gpsSolDRV.time.seconds = pkt->sec;
    // gpsSolDRV.time.millis  = 0;

    gpsSolDRV.flags.validTime = 0; //(pkt->fixType >= 3);

    gpsProcessNewDriverData();
    newDataReady = true;
}

void dronecanGPSReceiveGNSSFix2(const struct uavcan_equipment_gnss_Fix2 * pgnssFix2)
{
    //const mspSensorGpsDataMessage_t * pkt = (const mspSensorGpsDataMessage_t *)bufferPtr;

    gpsSolDRV.fixType   = gpsMapFixType(pgnssFix2->status);
    gpsSolDRV.numSat    = pgnssFix2->sats_used;
    gpsSolDRV.llh.lon   = pgnssFix2->longitude_deg_1e8;
    gpsSolDRV.llh.lat   = pgnssFix2->latitude_deg_1e8;
    gpsSolDRV.llh.alt   = pgnssFix2->height_msl_mm;
    gpsSolDRV.velNED[X] = pgnssFix2->ned_velocity[0]; // Dronecan is North, East, Down
    gpsSolDRV.velNED[Y] = pgnssFix2->ned_velocity[1];
    gpsSolDRV.velNED[Z] = pgnssFix2->ned_velocity[2];
    gpsSolDRV.groundSpeed = calc_length_pythagorean_2D((float)pgnssFix2->ned_velocity[0], (float)pgnssFix2->ned_velocity[1]);
    float groundCourse = atan2_approx(pgnssFix2->ned_velocity[1], pgnssFix2->ned_velocity[0]); // atan2 returns [-M_PI, M_PI], with 0 indicating the vector points in the X direction
    if (groundCourse < 0) {
        groundCourse += 2 * M_PIf;
    }
    gpsSolDRV.groundCourse = RADIANS_TO_DECIDEGREES(groundCourse);
    // TODO where to get EPH gpsSolDRV.eph = gpsConstrainEPE(pgnssFix-> / 10);
    // TODO where to get EPV gpsSolDRV.epv = gpsConstrainEPE(pkt->verticalPosAccuracy / 10);
    gpsSolDRV.hdop = gpsConstrainHDOP(pgnssFix2->pdop);
    gpsSolDRV.flags.validVelNE = true;
    gpsSolDRV.flags.validVelD = true;
    gpsSolDRV.flags.validEPE = false;

    // gpsSolDRV.time.year   = pkt->year;
    // gpsSolDRV.time.month  = pkt->month;
    // gpsSolDRV.time.day    = pkt->day;
    // gpsSolDRV.time.hours  = pkt->hour;
    // gpsSolDRV.time.minutes = pkt->min;
    // gpsSolDRV.time.seconds = pkt->sec;
    // gpsSolDRV.time.millis  = 0;

    gpsSolDRV.flags.validTime = 0; //(pkt->fixType >= 3);

    gpsProcessNewDriverData();
    newDataReady = true;
}

void dronecanGPSReceiveGNSSAuxiliary(const struct uavcan_equipment_gnss_Auxiliary * pgnssAux)
{
    UNUSED(pgnssAux);
    // No useful information I think...  Placeholder until after testing.
}
#endif