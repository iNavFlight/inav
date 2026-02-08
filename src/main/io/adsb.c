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
#include "adsb.h"

#ifdef USE_ADSB

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#include "common/maths.h"
#include "math.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "common/mavlink.h"
#pragma GCC diagnostic pop
#include "io/osd.h"

adsbVehicle_t adsbVehiclesDictionary[MAX_ADSB_VEHICLES];
adsbVehicleStatus_t adsbVehiclesStatus;

//buffer for fill data from mavlink
adsbVehicleValues_t vehicleValues;

adsbVehicleValues_t* getVehicleForFill(void){
    return &vehicleValues;
}

static void calculateCPA(fpVector3_t vehicleVector, float ground_speed, float heading_deg, float* t_cpa, float* d_min) {
    float pos_x = vehicleVector.x * 0.01f;
    float pos_y = vehicleVector.y * 0.01f;

    float heading_rad = heading_deg * (float)M_PI / 180.0f;

    float v_x = ground_speed * cos_approx(heading_rad);
    float v_y = ground_speed * sin_approx(heading_rad);

    float v_sq = v_x * v_x + v_y * v_y;
    if (v_sq < 1e-6f) {
        *t_cpa = 0.0f;
        *d_min = sqrtf(pos_x * pos_x + pos_y * pos_y);
        return;
    }

    float dot = pos_x * v_x + pos_y * v_y;

    float t = -dot / v_sq;
    if (t < 0.0f) {
        t = 0.0f;
    }

    *t_cpa = t;

    float closest_x = pos_x + v_x * t;
    float closest_y = pos_y + v_y * t;

    *d_min = sqrtf(closest_x * closest_x + closest_y * closest_y);
}


static void calculateMeetPoint(adsbVehicle_t *vehicle) {

    fpVector3_t pos;
    gpsOrigin_t uavPosition = {
            .alt = gpsSol.llh.alt,
            .lat = gpsSol.llh.lat,
            .lon = gpsSol.llh.lon,
            .scale = posControl.gpsOrigin.scale,
            .valid = posControl.gpsOrigin.valid,
    };
    geoConvertGeodeticToLocal(&pos, &uavPosition, &vehicle->vehicleValues.gps, GEO_ALT_RELATIVE);

    float t_cpa;
    float d_min;

    calculateCPA(
            pos,
            ((float)vehicle->vehicleValues.horVelocity) * 0.01f,  // cm/s -> m/s (bez integer dělení)
            CENTIDEGREES_TO_DEGREES(vehicle->vehicleValues.heading),
            &t_cpa,
            &d_min
    );

    vehicle->calculatedVehicleValues.meetPointDistance = (int)d_min;
    vehicle->calculatedVehicleValues.meetPointTime = (int)t_cpa;
}

static void recalculateVehicle(adsbVehicle_t* vehicle) {
    if(vehicle->ttl == 0){
        return;
    }

    fpVector3_t vehicleVector;
    geoConvertGeodeticToLocal(&vehicleVector, &posControl.gpsOrigin, &vehicle->vehicleValues.gps, GEO_ALT_RELATIVE);

    vehicle->calculatedVehicleValues.dist = calculateDistanceToDestination(&vehicleVector);
    vehicle->calculatedVehicleValues.dir = calculateBearingToDestination(&vehicleVector);

    if (vehicle->calculatedVehicleValues.dist > ADSB_LIMIT_CM) {
        vehicle->ttl = 0;
        vehicle->calculatedVehicleValues.valid = false;
        return;
    }

    if(osdConfig()->adsb_calculation_use_cpa){
        calculateMeetPoint(vehicle);
    }

    vehicle->calculatedVehicleValues.verticalDistance = vehicle->vehicleValues.alt - (int32_t)getEstimatedActualPosition(Z) - GPS_home.alt;
    vehicle->calculatedVehicleValues.valid = true;
}

// use bsearch function
static adsbVehicle_t *findVehicleByIcao(uint32_t avicao) {
    for (uint8_t i = 0; i < MAX_ADSB_VEHICLES; i++) {
        if (avicao == adsbVehiclesDictionary[i].vehicleValues.icao) {
            return &adsbVehiclesDictionary[i];
        }
    }
    return NULL;
}

static adsbVehicle_t *findVehicleFarthest(void) {
    adsbVehicle_t *adsbLocal = NULL;
    for (uint8_t i = 0; i < MAX_ADSB_VEHICLES; i++) {
        if (adsbVehiclesDictionary[i].ttl > 0 && adsbVehiclesDictionary[i].calculatedVehicleValues.valid && (adsbLocal == NULL || adsbLocal->calculatedVehicleValues.dist < adsbVehiclesDictionary[i].calculatedVehicleValues.dist)) {
            adsbLocal = &adsbVehiclesDictionary[i];
        }
    }
    return adsbLocal;
}

static adsbVehicle_t *findFreeSpaceInList(void) {
    //find expired first
    for (uint8_t i = 0; i < MAX_ADSB_VEHICLES; i++) {
        if (adsbVehiclesDictionary[i].ttl == 0) {
            return &adsbVehiclesDictionary[i];
        }
    }

    return NULL;
}

static adsbVehicle_t *findVehicleNotCalculated(void) {
    //find expired first
    for (uint8_t i = 0; i < MAX_ADSB_VEHICLES; i++) {
        if (adsbVehiclesDictionary[i].calculatedVehicleValues.valid == false) {
            return &adsbVehiclesDictionary[i];
        }
    }

    return NULL;
}

adsbVehicle_t *findVehicleForWarning(uint32_t warningDistanceCm, int32_t maxVerticalDistance) {
    adsbVehicle_t *adsbLocal = NULL;

    for (uint8_t i = 0; i < MAX_ADSB_VEHICLES; i++) {

        adsbVehicle_t *vehicle = &adsbVehiclesDictionary[i];

        //only active vehicles
        if (vehicle->ttl == 0 || !vehicle->calculatedVehicleValues.valid) {
            continue;
        }

        //it's too high
        if (vehicle->calculatedVehicleValues.verticalDistance > 0 && maxVerticalDistance > 0 &&
            vehicle->calculatedVehicleValues.verticalDistance > maxVerticalDistance) {
            continue;
        }

        // allow dist == 0 to be considered valid (closest possible)
        if (vehicle->calculatedVehicleValues.dist >= warningDistanceCm) {
            continue;
        }

        if (adsbLocal == NULL || adsbLocal->calculatedVehicleValues.dist > vehicle->calculatedVehicleValues.dist) {
            adsbLocal = vehicle;
        }
    }

    return adsbLocal;
}

adsbVehicle_t *findVehicleForAlert(uint32_t alertDistanceCm, uint32_t warningDistanceCm, int32_t maxVerticalDistance) {
    adsbVehicle_t *best = NULL;
    int32_t bestTimeToAlert = INT32_MAX;

    for (uint8_t i = 0; i < MAX_ADSB_VEHICLES; i++) {

        adsbVehicle_t *vehicle = &adsbVehiclesDictionary[i];
        int32_t timeToAlert = -1;

        //only active vehicles
        if (vehicle->ttl == 0 || !vehicle->calculatedVehicleValues.valid) {
            continue;
        }

        //it's too high
        if (vehicle->calculatedVehicleValues.verticalDistance > 0 && maxVerticalDistance > 0 &&
            vehicle->calculatedVehicleValues.verticalDistance > maxVerticalDistance) {
            continue;
        }

        // Case 1: already inside the alert circle (inclusive boundary)
        if (vehicle->calculatedVehicleValues.dist <= alertDistanceCm) {
            timeToAlert = 0;
        }
        // Case 2: inside the warning circle and CPA enters the alert circle
        else if (osdConfig()->adsb_calculation_use_cpa &&
                 vehicle->calculatedVehicleValues.dist <= warningDistanceCm &&
                 vehicle->calculatedVehicleValues.meetPointTime >= 0 &&
                 vehicle->calculatedVehicleValues.meetPointDistance > 0)
        {

            const uint32_t meetPointDistanceCm = ((uint32_t)vehicle->calculatedVehicleValues.meetPointDistance) * 100u; //cn

            if (meetPointDistanceCm > alertDistanceCm) {
                continue;
            }

            timeToAlert = vehicle->calculatedVehicleValues.meetPointTime;
        }
        else {
            continue;
        }

        if (best == NULL || timeToAlert < bestTimeToAlert) {
            best = vehicle;
            bestTimeToAlert = timeToAlert;
        }
    }

    return best;
}

uint8_t getActiveVehiclesCount(void) {
    uint8_t total = 0;
    for (uint8_t i = 0; i < MAX_ADSB_VEHICLES; i++) {
        if (adsbVehiclesDictionary[i].ttl > 0) {
            total++;
        }
    }
    return total;
}

adsbVehicle_t* findVehicle(uint8_t index)
{
    if (index < MAX_ADSB_VEHICLES){
        return &adsbVehiclesDictionary[index];
    }

    return NULL;
}

adsbVehicleStatus_t* getAdsbStatus(void){
    return &adsbVehiclesStatus;
}

bool adsbHeartbeat(void){
    adsbVehiclesStatus.heartbeatMessagesTotal++;
    return true;
}

void adsbNewVehicle(adsbVehicleValues_t* vehicleValuesLocal) {

    if(vehicleValuesLocal->icao == 0)
    {
        return;
    }

    // no valid lat lon or altitude
    if((vehicleValuesLocal->flags & (ADSB_FLAGS_VALID_ALTITUDE | ADSB_FLAGS_VALID_COORDS)) != (ADSB_FLAGS_VALID_ALTITUDE | ADSB_FLAGS_VALID_COORDS)){
        return;
    }

    adsbVehiclesStatus.vehiclesMessagesTotal++;

    adsbVehicle_t *vehicle = NULL;

    vehicle = findVehicleByIcao(vehicleValuesLocal->icao);
    if(vehicleValuesLocal->tslc > ADSB_MAX_SECONDS_KEEP_INACTIVE_PLANE_IN_LIST){
        if(vehicle != NULL){
            vehicle->ttl = 0;
        }
        return;
    }

    // non GPS mode, GPS is not fix, just find free space in list or by icao and save vehicle without calculated values
    if (!isEnvironmentOkForCalculatingADSBDistanceBearing()) {
        if(vehicle == NULL){
            vehicle = findFreeSpaceInList();
        }

        if (vehicle != NULL) {
            memcpy(&(vehicle->vehicleValues), vehicleValuesLocal, sizeof(vehicle->vehicleValues));
            vehicle->ttl = MAX(0, ADSB_MAX_SECONDS_KEEP_INACTIVE_PLANE_IN_LIST - vehicleValuesLocal->tslc);
            vehicle->calculatedVehicleValues.valid = false;
            return;
        }
    } else {
        // GPS mode, GPS is fixed and has enough sats
        if(vehicle == NULL){
            vehicle = findFreeSpaceInList();
        }

        if(vehicle == NULL){
            vehicle = findVehicleNotCalculated();
        }

        if(vehicle == NULL){
            vehicle = findVehicleFarthest();
            if(vehicle != NULL)
            {
                //calculate distance to new vehicle, we need to compare if new vehicle is closer than the farthest plane
                fpVector3_t vehicleVector;
                geoConvertGeodeticToLocal(&vehicleVector, &posControl.gpsOrigin, &vehicleValuesLocal->gps, GEO_ALT_RELATIVE);
                if(calculateDistanceToDestination(&vehicleVector) > vehicle->calculatedVehicleValues.dist){
                    //saved vehicle in list is closer, no need to update vehicle in list
                    vehicle = NULL;
                }
            }
        }

        if (vehicle != NULL) {
            memcpy(&(vehicle->vehicleValues), vehicleValuesLocal, sizeof(vehicle->vehicleValues));
            recalculateVehicle(vehicle);
            vehicle->ttl = MAX(0, ADSB_MAX_SECONDS_KEEP_INACTIVE_PLANE_IN_LIST - vehicleValuesLocal->tslc);
            return;
        }
    }
}

bool isEnvironmentOkForCalculatingADSBDistanceBearing(void){
    return
    (gpsSol.numSat > 4 &&
    (
        STATE(GPS_FIX)
        #ifdef USE_GPS_FIX_ESTIMATION
            || STATE(GPS_ESTIMATED_FIX)
        #endif
        )
    );
}

void taskAdsb(timeUs_t currentTimeUs){
    static timeUs_t adsbTtlLastCleanServiced = 0;
    timeDelta_t adsbTtlSinceLastCleanServiced = cmpTimeUs(currentTimeUs, adsbTtlLastCleanServiced);

    const bool shouldDecrementTtl = (adsbTtlSinceLastCleanServiced > 1000000); // 1s

    for (uint8_t i = 0; i < MAX_ADSB_VEHICLES; i++) {
        if (adsbVehiclesDictionary[i].ttl > 0) {
            if (shouldDecrementTtl) {
                adsbVehiclesDictionary[i].ttl--;
            }
            recalculateVehicle(&adsbVehiclesDictionary[i]);
        }
    }

    if (shouldDecrementTtl) {
        adsbTtlLastCleanServiced = currentTimeUs;
    }
}

#endif

