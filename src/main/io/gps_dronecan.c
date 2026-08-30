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
#include "io/gps_dronecan.h"
#include "drivers/dronecan/dronecan.h"

#include <dronecan_msgs.h>

static void parseGnssTime(uint64_t usec, uint8_t time_standard, uint8_t num_leap_seconds);

static bool newDataReady;
static uint16_t lastHDOP = 9999;
#ifdef UNIT_TEST
uint8_t activeGpsNodeId = 0;
#else
static uint8_t activeGpsNodeId = 0;
#endif

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

/* Shared acceptance gate for GNSS Fix2/Auxiliary messages: rejects
 * unhealthy nodes and non-matching gpsNodeId, then applies the
 * first-over-fence lock so two GPS nodes can't race to write gpsSolDRV.
 *
 * No automatic failover to a second GPS node: if the locked-in node degrades
 * to ERROR/CRITICAL health but keeps broadcasting NodeStatus (so it's never
 * evicted by process1HzTasks()'s stale-node purge), its data - and any other
 * node's data - stays rejected until it recovers or eventually goes silent
 * long enough to be evicted. dronecanGpsIsHealthy() correctly reports
 * unhealthy in this state, so arming/OSD reflect it, but there's no
 * redundant-sensor handoff. INAV doesn't have a general redundant-sensor
 * story, and picking one node over another when both are transmitting isn't
 * something to improvise here - it needs its own design (how to detect which
 * is actually reliable, how it's configured, etc). Deliberately out of scope
 * for this health guard. */
static bool dronecanGpsAcceptSource(uint8_t sourceNodeId)
{
    const dronecanNodeInfo_t *node = dronecanGetNodeByID(sourceNodeId);
    // node == NULL means no NodeStatus has been received yet for this ID - deliberately
    // NOT rejected here, unlike dronecanGpsIsHealthy() below, which returns false in that
    // same case. This function decides whether to use incoming GPS data right now, so it
    // fails open on unknown health (don't block real position updates just because
    // NodeStatus, which may broadcast on a different cadence than Fix2, hasn't arrived
    // yet). dronecanGpsIsHealthy() feeds arming/OSD/telemetry status, so it fails closed
    // instead (don't report healthy without evidence). The asymmetry is intentional.
    if (node && node->health >= UAVCAN_PROTOCOL_NODESTATUS_HEALTH_ERROR) {
        return false;
    }
    if (dronecanConfig()->gpsNodeId != 0) {
        // A configured static filter already uniquely selects the source,
        // so skip the first-over-fence lock entirely - otherwise
        // reconfiguring gpsNodeId at runtime to a node other than the one
        // currently locked in would leave GPS stuck rejecting it forever.
        if (sourceNodeId != dronecanConfig()->gpsNodeId) {
            return false;
        }
        activeGpsNodeId = sourceNodeId;
        return true;
    }
    if (activeGpsNodeId == 0) {
        activeGpsNodeId = sourceNodeId;
    } else if (sourceNodeId != activeGpsNodeId) {
        return false;
    }
    return true;
}

void dronecanGPSReceiveGNSSFix2(const struct uavcan_equipment_gnss_Fix2 * pgnssFix2, uint8_t sourceNodeId)
{
    if (!dronecanGpsAcceptSource(sourceNodeId)) {
        return;
    }
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

    if (pgnssFix2->pdop > 0){
        gpsSolDRV.hdop = gpsConstrainHDOP(pgnssFix2->pdop * 100); // Only update if valid.
    } else if((9999 > lastHDOP) && (lastHDOP > 0)) {
        gpsSolDRV.hdop = lastHDOP;
    }
    gpsSolDRV.flags.validVelNE = true;
    gpsSolDRV.flags.validVelD = true;
    gpsSolDRV.flags.validEPE = false;  // assume invalid unless the covariance is filled in.
    // Fix2 covariance layout is not specified by the DSDL. AP_Periph (the dominant peripheral
    // firmware) packs: [0]=[1]=hacc², [2]=vacc², [3]=[4]=[5]=sacc²  (m² / (m/s)²).
    // Verified against live data: [0]==[1] and [3]==[4]==[5] as expected from AP_Periph source.
    if (pgnssFix2->covariance.len >= 3) {
        gpsSolDRV.eph = gpsConstrainEPE((uint32_t)(sqrtf(pgnssFix2->covariance.data[0]) * 100));
        gpsSolDRV.epv = gpsConstrainEPE((uint32_t)(sqrtf(pgnssFix2->covariance.data[2]) * 100));
        gpsSolDRV.flags.validEPE = true;
    }
    parseGnssTime(pgnssFix2->gnss_timestamp.usec,
                  pgnssFix2->gnss_time_standard,
                  pgnssFix2->num_leap_seconds);

    gpsProcessNewDriverData();
    newDataReady = true;
}

void dronecanGPSReceiveGNSSAuxiliary(const struct uavcan_equipment_gnss_Auxiliary * pgnssAux, uint8_t sourceNodeId)
{
    if (!dronecanGpsAcceptSource(sourceNodeId)) {
        return;
    }
    // DroneCAN float16 optional fields encode NaN when unpopulated; guard before use.
    // gpsConstrainHDOP clamps to 9999 preventing uint16_t overflow for extreme DOP values.
    if (!isnan(pgnssAux->hdop)) {
        lastHDOP = gpsConstrainHDOP((uint32_t)(pgnssAux->hdop * 100));
    }
}

void dronecanGpsOnNodeEvicted(uint8_t nodeID)
{
    if (activeGpsNodeId == nodeID) {
        activeGpsNodeId = 0;
    }
}

bool dronecanGpsIsHealthy(void)
{
    uint8_t nodeId = (dronecanConfig()->gpsNodeId != 0) ? dronecanConfig()->gpsNodeId : activeGpsNodeId;
    if (nodeId == 0) {
        return false;
    }
    const dronecanNodeInfo_t *node = dronecanGetNodeByID(nodeId);
    if (node == NULL) {
        return false;
    }
    return node->health < UAVCAN_PROTOCOL_NODESTATUS_HEALTH_ERROR;
}

static void parseGnssTime(uint64_t usec, uint8_t time_standard, uint8_t num_leap_seconds)
{
    if (usec == UAVCAN_TIMESTAMP_UNKNOWN) {
        gpsSolDRV.flags.validTime = false;
        return;
    }
    time_t unix_s;
    switch (time_standard) {
    case UAVCAN_EQUIPMENT_GNSS_FIX2_GNSS_TIME_STANDARD_UTC:
        unix_s = (time_t)(usec / 1000000ULL);
        break;
    case UAVCAN_EQUIPMENT_GNSS_FIX2_GNSS_TIME_STANDARD_GPS:
        if (num_leap_seconds == UAVCAN_EQUIPMENT_GNSS_FIX2_NUM_LEAP_SECONDS_UNKNOWN) {
            gpsSolDRV.flags.validTime = false;
            return;
        }
        // DSDL: GPS epoch is µs since GPS time at UTC 1970-01-01; UTC = GPS - leap_seconds + 9
        unix_s = (time_t)(usec / 1000000ULL) - num_leap_seconds + 9;
        break;
    case UAVCAN_EQUIPMENT_GNSS_FIX2_GNSS_TIME_STANDARD_TAI:
        if (num_leap_seconds == UAVCAN_EQUIPMENT_GNSS_FIX2_NUM_LEAP_SECONDS_UNKNOWN) {
            gpsSolDRV.flags.validTime = false;
            return;
        }
        // DSDL: TAI epoch is µs since TAI time at UTC 1970-01-01; UTC = TAI - leap_seconds - 10
        unix_s = (time_t)(usec / 1000000ULL) - num_leap_seconds - 10;
        break;
    default:
        gpsSolDRV.flags.validTime = false;
        return;
    }
    struct tm *t = gmtime(&unix_s);
    if (!t) {
        gpsSolDRV.flags.validTime = false;
        return;
    }
    gpsSolDRV.time.year    = (uint16_t)(t->tm_year + 1900);
    gpsSolDRV.time.month   = (uint8_t)(t->tm_mon + 1);
    gpsSolDRV.time.day     = (uint8_t)t->tm_mday;
    gpsSolDRV.time.hours   = (uint8_t)t->tm_hour;
    gpsSolDRV.time.minutes = (uint8_t)t->tm_min;
    gpsSolDRV.time.seconds = (uint8_t)t->tm_sec;
    gpsSolDRV.time.millis  = (uint16_t)((usec % 1000000ULL) / 1000ULL);
    gpsSolDRV.flags.validTime = true;
}

#endif