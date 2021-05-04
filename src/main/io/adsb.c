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
 

#include "io/adsb.h"
#include "navigation/navigation.h"
#include "common/maths.h"
#include "math.h"
#include "common/time.h"
#include "drivers/time.h"

adsbVehicle_t adsb;
static timeUs_t lastADSB = 0;

void GPS_distance_cm_bearing(int32_t currentLat1, int32_t currentLon1, int32_t destinationLat2, int32_t destinationLon2, uint32_t *dist, int32_t *bearing)
{
  #define DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR    1.113195f  // MagicEarthNumber from APM
  float GPS_scaleLonDown = cos_approx((fabsf((float)gpsSol.llh.lat) / 10000000.0f) * 0.0174532925f);
  const float dLat = destinationLat2 - currentLat1; // difference of latitude in 1/10 000 000 degrees
  const float dLon = (float)(destinationLon2 - currentLon1) * GPS_scaleLonDown;

  *dist = sqrtf(sq(dLat) + sq(dLon)) * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR;
  *bearing = 9000.0f + RADIANS_TO_CENTIDEGREES(atan2_approx(-dLat, dLon));      // Convert the output radians to 100xdeg
  if (*bearing < 0){
    *bearing += 36000;
  }
};

void adsbNewVehicle(uint32_t avicao, int32_t avlat, int32_t avlon, int32_t avalt)
{
    uint32_t avdist; int32_t avdir; uint8_t avupdate = 1; 
    if (STATE(GPS_FIX) && gpsSol.numSat >= 5){
      GPS_distance_cm_bearing(gpsSol.llh.lat, gpsSol.llh.lon, avlat, avlon, &avdist, &avdir); 
    }
    avdist /= 100; avdir /= 100;
#if defined(USE_NAV)
    avalt -= getEstimatedActualPosition(Z)+GPS_home.alt;
#elif defined(USE_BARO)
    avalt -= baro.alt+GPS_home.alt;
#endif
    avalt /= 1000;
    if (avdist > adsb.dist){
      avupdate = 0;
    }   
    if (adsb.ttl <= 1){
      avupdate = 1;
    }    
    if (avicao == adsb.icao){
      avupdate = 1;
    }      
    if (avdist > 50000){ // limit display to aircraft < 50Km
      avupdate = 0; 
    }  
    if (avupdate == 1){
      adsb.icao = avicao;
      adsb.dist = avdist;       
      adsb.alt = avalt; 
      adsb.dir = avdir; 
      adsb.ttl = 10;    // 10 secs default timeout    
    }   
};

void adsbexpiry(){
  uint64_t currentTimeUs = micros();
  if ((currentTimeUs - lastADSB) > 1000*1000) {
    lastADSB = currentTimeUs;
    if (adsb.ttl > 0){
      adsb.ttl--;
    }
    else{
      adsb.icao = 0;
      adsb.dist = 0;       
      adsb.alt = 0; 
      adsb.dir = 0;       
    }
  }
};

