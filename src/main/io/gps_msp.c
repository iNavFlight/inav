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


#if defined(USE_GPS_PROTO_MSP)

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
#include "io/serial.h"

#include "msp/msp_protocol_v2_sensor_msg.h"

static bool newDataReady;

void gpsRestartMSP(void)
{
    // NOP
}

void gpsHandleMSP(void)
{
    if (newDataReady) {
        gpsProcessNewSolutionData();
        newDataReady = false;
    }
}

static uint8_t gpsMapFixType(uint8_t mspFixType)
{
    if (mspFixType == 2)
        return GPS_FIX_2D;
    if (mspFixType >= 3)
        return GPS_FIX_3D;
    return GPS_NO_FIX;
}

void mspGPSReceiveNewData(const uint8_t * bufferPtr)
{
    const mspSensorGpsDataMessage_t * pkt = (const mspSensorGpsDataMessage_t *)bufferPtr;

    gpsSol.fixType   = gpsMapFixType(pkt->fixType);
    gpsSol.numSat    = pkt->satellitesInView;
    gpsSol.llh.lon   = pkt->longitude;
    gpsSol.llh.lat   = pkt->latitude;
    gpsSol.llh.alt   = pkt->mslAltitude;
    gpsSol.velNED[X] = pkt->nedVelNorth;
    gpsSol.velNED[Y] = pkt->nedVelEast;
    gpsSol.velNED[Z] = pkt->nedVelDown;
    gpsSol.groundSpeed = sqrtf(sq((float)pkt->nedVelNorth) + sq((float)pkt->nedVelEast));
    gpsSol.groundCourse = pkt->groundCourse / 10;   // in deg * 10
    gpsSol.eph = gpsConstrainEPE(pkt->horizontalPosAccuracy / 10);
    gpsSol.epv = gpsConstrainEPE(pkt->verticalPosAccuracy / 10);
    gpsSol.hdop = gpsConstrainHDOP(pkt->hdop);
    gpsSol.flags.validVelNE = 1;
    gpsSol.flags.validVelD = 1;
    gpsSol.flags.validEPE = 1;

    gpsSol.time.year   = pkt->year;
    gpsSol.time.month  = pkt->month;
    gpsSol.time.day    = pkt->day;
    gpsSol.time.hours  = pkt->hour;
    gpsSol.time.minutes = pkt->min;
    gpsSol.time.seconds = pkt->sec;
    gpsSol.time.millis  = 0;

    gpsSol.flags.validTime = (pkt->fixType >= 3);

    newDataReady = true;
}
#endif
