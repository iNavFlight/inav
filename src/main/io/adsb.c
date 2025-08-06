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

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#include "common/maths.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "common/mavlink.h"
#pragma GCC diagnostic pop


#include "math.h"


#ifdef USE_ADSB

adsbVehicle_t adsbVehiclesList[MAX_ADSB_VEHICLES];
adsbVehicleStatus_t adsbVehiclesStatus;

adsbVehicleValues_t vehicleValues;


adsbVehicleValues_t* getVehicleForFill(void){
    return &vehicleValues;
}

// use bsearch function
adsbVehicle_t *findVehicleByIcao(uint32_t avicao) {
    for (uint8_t i = 0; i < MAX_ADSB_VEHICLES; i++) {
        if (avicao == adsbVehiclesList[i].vehicleValues.icao) {
            return &adsbVehiclesList[i];
        }
    }
    return NULL;
}

adsbVehicle_t *findVehicleFarthest(void) {
    adsbVehicle_t *adsbLocal = NULL;
    for (uint8_t i = 0; i < MAX_ADSB_VEHICLES; i++) {
        if (adsbVehiclesList[i].ttl > 0 && adsbVehiclesList[i].calculatedVehicleValues.valid && (adsbLocal == NULL || adsbLocal->calculatedVehicleValues.dist < adsbVehiclesList[i].calculatedVehicleValues.dist)) {
            adsbLocal = &adsbVehiclesList[i];
        }
    }
    return adsbLocal;
}

uint8_t getActiveVehiclesCount(void) {
    uint8_t total = 0;
    for (uint8_t i = 0; i < MAX_ADSB_VEHICLES; i++) {
        if (adsbVehiclesList[i].ttl > 0) {
            total++;
        }
    }
    return total;
}

adsbVehicle_t *findVehicleClosest(void) {
    adsbVehicle_t *adsbLocal = NULL;
    for (uint8_t i = 0; i < MAX_ADSB_VEHICLES; i++) {
        if (adsbVehiclesList[i].ttl > 0 && adsbVehiclesList[i].calculatedVehicleValues.valid && (adsbLocal == NULL || adsbLocal->calculatedVehicleValues.dist > adsbVehiclesList[i].calculatedVehicleValues.dist)) {
            adsbLocal = &adsbVehiclesList[i];
        }
    }
    return adsbLocal;
}

/**
 * find the closest vehicle, apply filter max verticalDistance
 * @return
 */
adsbVehicle_t *findVehicleClosestLimit(int32_t maxVerticalDistance) {
    adsbVehicle_t *adsbLocal = NULL;
    for (uint8_t i = 0; i < MAX_ADSB_VEHICLES; i++) {
        if(adsbVehiclesList[i].ttl > 0 && adsbVehiclesList[i].calculatedVehicleValues.valid){
            if(adsbVehiclesList[i].calculatedVehicleValues.verticalDistance > 0 && maxVerticalDistance > 0 && adsbVehiclesList[i].calculatedVehicleValues.verticalDistance > maxVerticalDistance){
                continue;
            }

            if (adsbLocal == NULL || adsbLocal->calculatedVehicleValues.dist > adsbVehiclesList[i].calculatedVehicleValues.dist) {
                adsbLocal = &adsbVehiclesList[i];
            }
        }
    }
    return adsbLocal;
}

adsbVehicle_t *findFreeSpaceInList(void) {
    //find expired first
    for (uint8_t i = 0; i < MAX_ADSB_VEHICLES; i++) {
        if (adsbVehiclesList[i].ttl == 0) {
            return &adsbVehiclesList[i];
        }
    }

    return NULL;
}

adsbVehicle_t *findVehicleNotCalculated(void) {
    //find expired first
    for (uint8_t i = 0; i < MAX_ADSB_VEHICLES; i++) {
        if (adsbVehiclesList[i].calculatedVehicleValues.valid == false) {
            return &adsbVehiclesList[i];
        }
    }

    return NULL;
}

adsbVehicle_t* findVehicle(uint8_t index)
{
    if (index < MAX_ADSB_VEHICLES){
        return &adsbVehiclesList[index];
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
    if (!enviromentOkForCalculatingDistaceBearing()) {
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
};

void recalculateVehicle(adsbVehicle_t* vehicle){
    if(vehicle->ttl == 0){
        return;
    }

    fpVector3_t vehicleVector;
    geoConvertGeodeticToLocal(&vehicleVector, &posControl.gpsOrigin, &vehicle->vehicleValues.gps, GEO_ALT_RELATIVE);

    vehicle->calculatedVehicleValues.dist = calculateDistanceToDestination(&vehicleVector);
    vehicle->calculatedVehicleValues.dir = calculateBearingToDestination(&vehicleVector);

    if (vehicle->calculatedVehicleValues.dist > ADSB_LIMIT_CM) {
        vehicle->ttl = 0;
        return;
    }

    vehicle->calculatedVehicleValues.verticalDistance = vehicle->vehicleValues.alt - (int32_t)getEstimatedActualPosition(Z) - GPS_home.alt;
    vehicle->calculatedVehicleValues.valid = true;
}

void adsbTtlClean(timeUs_t currentTimeUs) {

    static timeUs_t adsbTtlLastCleanServiced = 0;
    timeDelta_t adsbTtlSinceLastCleanServiced = cmpTimeUs(currentTimeUs, adsbTtlLastCleanServiced);


    if (adsbTtlSinceLastCleanServiced > 1000000) // 1s
    {
        for (uint8_t i = 0; i < MAX_ADSB_VEHICLES; i++) {
            if (adsbVehiclesList[i].ttl > 0) {
                adsbVehiclesList[i].ttl--;
            }

            if (adsbVehiclesList[i].ttl > 0) {
                recalculateVehicle(&adsbVehiclesList[i]);
            }
        }

        adsbTtlLastCleanServiced = currentTimeUs;
    }
};

bool enviromentOkForCalculatingDistaceBearing(void){
    return (STATE(GPS_FIX) && gpsSol.numSat > 4);
}

#endif

