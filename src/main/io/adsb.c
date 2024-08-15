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

void gpsDistanceCmBearing(int32_t currentLat1, int32_t currentLon1, int32_t destinationLat2, int32_t destinationLon2, uint32_t *dist, int32_t *bearing) {
    float GPS_scaleLonDown = cos_approx((fabsf((float) gpsSol.llh.lat) / 10000000.0f) * 0.0174532925f);
    const float dLat = destinationLat2 - currentLat1; // difference of latitude in 1/10 000 000 degrees
    const float dLon = (float) (destinationLon2 - currentLon1) * GPS_scaleLonDown;

    *dist = sqrtf(sq(dLat) + sq(dLon)) * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR;
    *bearing = 9000.0f + RADIANS_TO_CENTIDEGREES(atan2_approx(-dLat, dLon));      // Convert the output radians to 100xdeg
    *bearing = wrap_36000(*bearing);
};

bool adsbHeartbeat(void){
    adsbVehiclesStatus.heartbeatMessagesTotal++;
    return true;
}

void adsbNewVehicle(adsbVehicleValues_t* vehicleValuesLocal) {

    // no valid lat lon or altitude
    if((vehicleValuesLocal->flags & (ADSB_FLAGS_VALID_ALTITUDE | ADSB_FLAGS_VALID_COORDS)) != (ADSB_FLAGS_VALID_ALTITUDE | ADSB_FLAGS_VALID_COORDS)){
        return;
    }

    adsbVehiclesStatus.vehiclesMessagesTotal++;

    adsbVehicle_t *vehicle = NULL;

    vehicle = findVehicleByIcao(vehicleValuesLocal->icao);
    if(vehicle != NULL && vehicleValuesLocal->tslc > ADSB_MAX_SECONDS_KEEP_INACTIVE_PLANE_IN_LIST){
        vehicle->ttl = 0;
        return;
    }

    // non GPS mode, GPS is not fix, just find free space in list or by icao and save vehicle without calculated values
    if (!enviromentOkForCalculatingDistaceBearing()) {

        if(vehicle == NULL){
            vehicle = findFreeSpaceInList();
        }

        if (vehicle != NULL) {
            memcpy(&(vehicle->vehicleValues), vehicleValuesLocal, sizeof(vehicle->vehicleValues));
            vehicle->ttl = ADSB_MAX_SECONDS_KEEP_INACTIVE_PLANE_IN_LIST;
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
        }

        if (vehicle != NULL) {
            memcpy(&(vehicle->vehicleValues), vehicleValuesLocal, sizeof(vehicle->vehicleValues));
            recalculateVehicle(vehicle);
            vehicle->ttl = ADSB_MAX_SECONDS_KEEP_INACTIVE_PLANE_IN_LIST;
            return;
        }
    }
};

void recalculateVehicle(adsbVehicle_t* vehicle){
    gpsDistanceCmBearing(gpsSol.llh.lat, gpsSol.llh.lon, vehicle->vehicleValues.lat, vehicle->vehicleValues.lon, &(vehicle->calculatedVehicleValues.dist), &(vehicle->calculatedVehicleValues.dir));

    if (vehicle != NULL && vehicle->calculatedVehicleValues.dist > ADSB_LIMIT_CM) {
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
        }

        adsbTtlLastCleanServiced = currentTimeUs;
    }
};

bool enviromentOkForCalculatingDistaceBearing(void){
    return (STATE(GPS_FIX) && gpsSol.numSat > 4);
}

#endif

