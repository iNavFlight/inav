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

#pragma once

#include <stdint.h>
#include "common/time.h"
#include "fc/runtime_config.h"

#define ADSB_CALL_SIGN_MAX_LENGTH 9
#define ADSB_MAX_SECONDS_KEEP_INACTIVE_PLANE_IN_LIST 10

typedef struct {
    bool valid;
    int32_t dir;   // centidegrees direction to plane, pivot is inav FC
    uint32_t dist;  // CM distance to plane, pivot is inav FC
    int32_t verticalDistance; // CM, vertical distance to plane, pivot is inav FC
} adsbVehicleCalculatedValues_t;

typedef struct {
    uint32_t icao; // ICAO address
    int32_t lat; // Latitude, expressed as degrees * 1E7
    int32_t lon; // Longitude, expressed as degrees * 1E7
    int32_t alt;  // Barometric/Geometric Altitude (ASL), in cm
    uint16_t heading; // Course over ground in centidegrees
    uint16_t flags; // Flags to indicate various statuses including valid data fields
    uint8_t altitudeType; // Type from ADSB_ALTITUDE_TYPE enum
    char callsign[ADSB_CALL_SIGN_MAX_LENGTH]; // The callsign, 8 chars + NULL
    uint8_t emitterType; // Type from ADSB_EMITTER_TYPE enum
    uint8_t tslc; // Time since last communication in seconds
} adsbVehicleValues_t;

typedef struct {
    adsbVehicleValues_t vehicleValues;
    adsbVehicleCalculatedValues_t calculatedVehicleValues;
    uint8_t ttl;
} adsbVehicle_t;



typedef struct {
   uint32_t vehiclesMessagesTotal;
   uint32_t heartbeatMessagesTotal;
} adsbVehicleStatus_t;

void adsbNewVehicle(adsbVehicleValues_t* vehicleValuesLocal);
bool adsbHeartbeat(void);
adsbVehicle_t * findVehicleClosest(void);
adsbVehicle_t * findVehicle(uint8_t index);
uint8_t getActiveVehiclesCount(void);
void adsbTtlClean(timeUs_t currentTimeUs);
adsbVehicleStatus_t* getAdsbStatus(void);
adsbVehicleValues_t* getVehicleForFill(void);
bool enviromentOkForCalculatingDistaceBearing(void);
void recalculateVehicle(adsbVehicle_t* vehicle);