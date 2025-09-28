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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <math.h>

#include "platform.h"

#include "target.h"
#include "target/SITL/sim/realFlight.h"
#include "target/SITL/sim/simple_soap_client.h"
#include "target/SITL/sim/xplane.h"
#include "target/SITL/sim/simHelper.h"
#include "fc/runtime_config.h"
#include "drivers/time.h"
#include "drivers/accgyro/accgyro_fake.h"
#include "drivers/barometer/barometer_fake.h"
#include "sensors/battery_sensor_fake.h"
#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "drivers/pitotmeter/pitotmeter_fake.h"
#include "drivers/compass/compass_fake.h"
#include "drivers/rangefinder/rangefinder_virtual.h"
#include "io/rangefinder.h"
#include "common/utils.h"
#include "common/maths.h"
#include "flight/mixer.h"
#include "flight/servos.h"
#include "flight/imu.h"
#include "io/gps.h"
#include "rx/sim.h"

#define RF_PORT 18083
#define RF_MAX_CHANNEL_COUNT 12

// "RealFlight Ranch" is located in Sierra Nevada, southern Spain
// This is not the Position of the Ranch, it's the Point of 0,0 in the Map (bottom left corner)
#define FAKE_LAT  36.910610f
#define FAKE_LON  -2.876605f

static uint8_t pwmMapping[RF_MAX_PWM_OUTS];
static uint8_t mappingCount;

static pthread_cond_t sockcond1 = PTHREAD_COND_INITIALIZER;
static pthread_cond_t sockcond2 = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t sockmtx = PTHREAD_MUTEX_INITIALIZER;

static soap_client_t *client = NULL;
static soap_client_t *clientNext = NULL;

static pthread_t soapThread;
static pthread_t creationThread;

static bool isInitalised = false;
static bool useImu = false;

typedef struct 
{
    float m_channelValues[RF_MAX_PWM_OUTS];
    float m_currentPhysicsSpeedMultiplier;
    float m_currentPhysicsTime_SEC;
    float m_airspeed_MPS;
    float m_altitudeASL_MTR;
    float m_altitudeAGL_MTR;
    float m_groundspeed_MPS;
    float m_pitchRate_DEGpSEC;
    float m_rollRate_DEGpSEC;
    float m_yawRate_DEGpSEC;
    float m_azimuth_DEG;
    float m_inclination_DEG;
    float m_roll_DEG;
    float m_orientationQuaternion_X;
    float m_orientationQuaternion_Y;
    float m_orientationQuaternion_Z;
    float m_orientationQuaternion_W;
    float m_aircraftPositionX_MTR;
    float m_aircraftPositionY_MTR;
    float m_velocityWorldU_MPS;
    float m_velocityWorldV_MPS;
    float m_velocityWorldW_MPS;
    float m_velocityBodyU_MPS;
    float m_velocityBodyV_MPS;
    float m_velocityBodyW_MPS;
    float m_accelerationWorldAX_MPS2;
    float m_accelerationWorldAY_MPS2;
    float m_accelerationWorldAZ_MPS2;
    float m_accelerationBodyAX_MPS2;
    float m_accelerationBodyAY_MPS2;
    float m_accelerationBodyAZ_MPS2;
    float m_windX_MPS;
    float m_windY_MPS;
    float m_windZ_MPSPS;
    float m_propRPM;
    float m_heliMainRotorRPM;
    float m_batteryVoltage_VOLTS;
    float m_batteryCurrentDraw_AMPS;
    float m_batteryRemainingCapacity_MAH;
    float m_fuelRemaining_OZ;
    bool m_isLocked;
    bool m_hasLostComponents;
    bool m_anEngineIsRunning;
    bool m_isTouchingGround;
    bool m_flightAxisControllerIsActive;
    char* m_currentAircraftStatus;
    bool m_resetButtonHasBeenPressed;
} rfValues_t;

rfValues_t rfValues; 

static void deleteClient(soap_client_t *client)
{
    soapClientClose(client);
    free(client);
    client = NULL;
}

static void startRequest(char* action, const char* fmt, ...)
{
    pthread_mutex_lock(&sockmtx);
    while (clientNext == NULL) {
        pthread_cond_wait(&sockcond1, &sockmtx);
    }

    client = clientNext;
    clientNext = NULL;

    pthread_cond_broadcast(&sockcond2);
    pthread_mutex_unlock(&sockmtx);

    va_list va;
    va_start(va, fmt);
    soapClientSendRequestVa(client, action, fmt, va);
    va_end(va);
}

static char* endRequest(void)
{
   char* ret = soapClientReceive(client); 
   deleteClient(client);
   return ret;   
}

// Simple, but fast ;)
static double getDoubleFromResponse(const char* response, const char* elementName)
{   
    if (!response) {
        return 0;
    }
    
    char* pos = strstr(response, elementName);
    if (!pos) {
        return 0;
    }
    return atof(pos + strlen(elementName) + 1);
}


/*
Currently unused
static bool getBoolFromResponse(const char* response, const char* elementName)
{
    if (!response) {
        return false;
    }
    
    char* pos = strstr(response, elementName);
    if (!pos) {
        return false;
    }
    return (strncmp(pos + strlen(elementName) + 1, "true", 4) == 0);
}
*/

static char* getStringFromResponse(const char* response, const char* elementName)
{
    if (!response) {
        return 0;
    }
    
    char* pos = strstr(response, elementName);
    if (!pos) {
        return NULL;
    }

    pos += strlen(elementName) + 1;
    char* end = strstr(pos, "</");
    if (!end) {
        return NULL;
    }
    size_t length = end - pos;
    char* ret = calloc(length + 1, sizeof(char));
    strncpy(ret, pos, length);
    return ret;
}

static bool getChannelValues(const char* response, uint16_t* channelValues)
{
    if (!response) {
        return false;
    }
    
    const char* channelValueTag = "<m-channelValues-0to1 xsi:type=\"SOAP-ENC:Array\" SOAP-ENC:arrayType=\"xsd:double[12]\">";
    char* pos = strstr(response, channelValueTag);
    if (!pos){
        return false;
    }

    pos += strlen(channelValueTag);
    for (size_t i = 0; i < RF_MAX_CHANNEL_COUNT; i++) {
        char* end = strstr(pos, "</");
        if (!end) {
            return false;
        }

        channelValues[i] = FLOAT_0_1_TO_PWM((float)atof(pos + 6));
        pos = end + 7;   
    }

    return true;
}


static void fakeCoords(float posX, float posY, float distanceX, float distanceY, float *lat, float *lon)
{
    float m = 1 / (2 * M_PIf / 360 * EARTH_RADIUS) / 1000;
    *lat = (posX + (distanceX * m));
    *lon = (posY + (distanceY * m) / cosf(posX * (M_PIf / 180)));
} 

static float convertAzimuth(float azimuth)
{
    if (azimuth < 0) {
        azimuth += 360;
    }
    return 360 - fmodf(azimuth + 90, 360.0f);
}

static void exchangeData(void)
{
    double servoValues[RF_MAX_PWM_OUTS] = {  };    
    for (int i = 0; i < mappingCount; i++) {
        if (pwmMapping[i] & 0x80) { // Motor
            servoValues[i] = (double)PWM_TO_FLOAT_0_1(motor[pwmMapping[i] & 0x7f]);
        } else { 
            servoValues[i] = (double)PWM_TO_FLOAT_0_1(servo[pwmMapping[i]]);
        }
    }

    startRequest("ExchangeData", "<ExchangeData><pControlInputs><m-selectedChannels>%u</m-selectedChannels>"
        "<m-channelValues-0to1><item>%.4f</item><item>%.4f</item><item>%.4f</item><item>%.4f</item><item>%.4f</item><item>%.4f</item><item>%.4f</item><item>%.4f</item>"
        "<item>%.4f</item><item>%.4f</item><item>%.4f</item><item>%.4f</item></m-channelValues-0to1></pControlInputs></ExchangeData>",
        0xFFF, 
        servoValues[0], servoValues[1], servoValues[2], servoValues[3], servoValues[4], servoValues[5], servoValues[6], servoValues[7],
        servoValues[8], servoValues[9], servoValues[10], servoValues[11]);
    char* response = endRequest();

    //rfValues.m_currentPhysicsTime_SEC = getDoubleFromResponse(response, "m-currentPhysicsTime-SEC");
    //rfValues.m_currentPhysicsSpeedMultiplier = getDoubleFromResponse(response, "m-currentPhysicsSpeedMultiplier");
    rfValues.m_airspeed_MPS = getDoubleFromResponse(response, "m-airspeed-MPS");
    rfValues.m_altitudeASL_MTR = getDoubleFromResponse(response, "m-altitudeASL-MTR");
    rfValues.m_altitudeAGL_MTR = getDoubleFromResponse(response, "m-altitudeAGL-MTR");
    rfValues.m_groundspeed_MPS = getDoubleFromResponse(response, "m-groundspeed-MPS");
    rfValues.m_pitchRate_DEGpSEC = getDoubleFromResponse(response, "m-pitchRate-DEGpSEC");
    rfValues.m_rollRate_DEGpSEC = getDoubleFromResponse(response, "m-rollRate-DEGpSEC");
    rfValues.m_yawRate_DEGpSEC = getDoubleFromResponse(response, "m-yawRate-DEGpSEC");
    rfValues.m_azimuth_DEG = getDoubleFromResponse(response, "m-azimuth-DEG");
    rfValues.m_inclination_DEG = getDoubleFromResponse(response, "m-inclination-DEG");
    rfValues.m_roll_DEG = getDoubleFromResponse(response, "m-roll-DEG");
    //rfValues.m_orientationQuaternion_X = getDoubleFromResponse(response, "m-orientationQuaternion-X");
    //rfValues.m_orientationQuaternion_Y = getDoubleFromResponse(response, "m-orientationQuaternion-Y");
    //rfValues.m_orientationQuaternion_Z = getDoubleFromResponse(response, "m-orientationQuaternion-Z");
    //rfValues.m_orientationQuaternion_W = getDoubleFromResponse(response, "m-orientationQuaternion-W");
    rfValues.m_aircraftPositionX_MTR = getDoubleFromResponse(response, "m-aircraftPositionX-MTR");
    rfValues.m_aircraftPositionY_MTR = getDoubleFromResponse(response, "m-aircraftPositionY-MTR");
    rfValues.m_velocityWorldU_MPS = getDoubleFromResponse(response, "m-velocityWorldU-MPS");
    rfValues.m_velocityWorldV_MPS = getDoubleFromResponse(response, "m-velocityWorldV-MPS");
    rfValues.m_velocityWorldW_MPS = getDoubleFromResponse(response, "m-velocityWorldW-MPS");
    //rfValues.m_velocityBodyU_MPS = getDoubleFromResponse(response, "m-velocityBodyU-MPS");
    //rfValues.m_velocityBodyV_MPS = getDoubleFromResponse(response, "mm-velocityBodyV-MPS");
    //rfValues.m_velocityBodyW_MPS = getDoubleFromResponse(response, "m-velocityBodyW-MPS");
    //rfValues.m_accelerationWorldAX_MPS2 = getDoubleFromResponse(response, "m-accelerationWorldAX-MPS2");
    //rfValues.m_accelerationWorldAY_MPS2 = getDoubleFromResponse(response, "m-accelerationWorldAY-MPS2");
    //rfValues.m_accelerationWorldAZ_MPS2 = getDoubleFromResponse(response, "m-accelerationWorldAZ-MPS2");
    rfValues.m_accelerationBodyAX_MPS2 = getDoubleFromResponse(response, "m-accelerationBodyAX-MPS2");
    rfValues.m_accelerationBodyAY_MPS2 = getDoubleFromResponse(response, "m-accelerationBodyAY-MPS2");
    rfValues.m_accelerationBodyAZ_MPS2 = getDoubleFromResponse(response, "m-accelerationBodyAZ-MPS2");
    //rfValues.m_windX_MPS = getDoubleFromResponse(response, "m-windX-MPS");
    //rfValues.m_windY_MPS = getDoubleFromResponse(response, "m-windY-MPS");
    //rfValues.m_windZ_MPSPS = getDoubleFromResponse(response, "m-windZ-MPS");
    //rfValues.m_propRPM = getDoubleFromResponse(response, "m-propRPM");
    //rfValues.m_heliMainRotorRPM = getDoubleFromResponse(response, "m-heliMainRotorRPM");
    rfValues.m_batteryVoltage_VOLTS = getDoubleFromResponse(response, "m-batteryVoltage-VOLTS");
    rfValues.m_batteryCurrentDraw_AMPS = getDoubleFromResponse(response, "m-batteryCurrentDraw-AMPS");
    //rfValues.m_batteryRemainingCapacity_MAH = getDoubleFromResponse(response, "m-batteryRemainingCapacity-MAH");
    //rfValues.m_fuelRemaining_OZ = getDoubleFromResponse(response, "m-fuelRemaining-OZ");
    //rfValues.m_isLocked = getBoolFromResponse(response, "m-isLocked");
    //rfValues.m_hasLostComponents = getBoolFromResponse(response, "m-hasLostComponents");
    //rfValues.m_anEngineIsRunning = getBoolFromResponse(response, "m-anEngineIsRunning");
    //rfValues.m_isTouchingGround = getBoolFromResponse(response, "m-isTouchingGround");
    //rfValues.m_flightAxisControllerIsActive= getBoolFromResponse(response, "m-flightAxisControllerIsActive");
    rfValues.m_currentAircraftStatus = getStringFromResponse(response, "m-currentAircraftStatus");

    
    uint16_t channelValues[RF_MAX_CHANNEL_COUNT];
    getChannelValues(response, channelValues);
    rxSimSetChannelValue(channelValues, RF_MAX_CHANNEL_COUNT);
    
    float lat, lon;
    fakeCoords(FAKE_LAT, FAKE_LON, rfValues.m_aircraftPositionX_MTR, -rfValues.m_aircraftPositionY_MTR, &lat, &lon);
    
    int16_t course = (int16_t)roundf(RADIANS_TO_DECIDEGREES(atan2_approx(-rfValues.m_velocityWorldU_MPS,rfValues.m_velocityWorldV_MPS)));
    int32_t altitude = (int32_t)roundf(rfValues.m_altitudeASL_MTR * 100);
    gpsFakeSet(
        GPS_FIX_3D,
        16,
        (int32_t)roundf(lat * 10000000),
        (int32_t)roundf(lon * 10000000),
        altitude,
        (int16_t)roundf(rfValues.m_groundspeed_MPS * 100),
        course,
        (int16_t)roundf(rfValues.m_velocityWorldV_MPS * 100), //direction seems ok
        (int16_t)roundf(-rfValues.m_velocityWorldU_MPS * 100),//direction seems ok
        (int16_t)roundf(rfValues.m_velocityWorldW_MPS * 100),//direction not sure
        0
    );

    int32_t altitudeOverGround = (int32_t)roundf(rfValues.m_altitudeAGL_MTR * 100);
    if (altitudeOverGround > 0 && altitudeOverGround <= RANGEFINDER_VIRTUAL_MAX_RANGE_CM) {
        fakeRangefindersSetData(altitudeOverGround);
    } else {
        fakeRangefindersSetData(-1);
    }

    const int16_t roll_inav = (int16_t)roundf(rfValues.m_roll_DEG * 10);
    const int16_t pitch_inav = (int16_t)roundf(-rfValues.m_inclination_DEG * 10);
    const int16_t yaw_inav = (int16_t)roundf(convertAzimuth(rfValues.m_azimuth_DEG) * 10);
    if (!useImu) {
        imuSetAttitudeRPY(roll_inav, pitch_inav, yaw_inav);
        imuUpdateAttitude(micros());
    }

    // RealFlights acc data is weird if the aircraft has not yet taken off. Fake 1G in horizontale position
    int16_t accX = 0;
    int16_t accY = 0;
    int16_t accZ = 0;
    if (rfValues.m_currentAircraftStatus && strncmp(rfValues.m_currentAircraftStatus, "CAS-WAITINGTOLAUNCH", strlen(rfValues.m_currentAircraftStatus)) == 0) {
        accX = 0;
        accY = 0;
        accZ = (int16_t)(GRAVITY_MSS * 1000.0f);
    } else {
         accX = constrainToInt16(rfValues.m_accelerationBodyAX_MPS2 * 1000);
         accY = constrainToInt16(-rfValues.m_accelerationBodyAY_MPS2 * 1000);
         accZ = constrainToInt16(-rfValues.m_accelerationBodyAZ_MPS2 * 1000);
    }

    fakeAccSet(accX, accY, accZ);

    fakeGyroSet(
        constrainToInt16(rfValues.m_rollRate_DEGpSEC * 16.0f),
        constrainToInt16(-rfValues.m_pitchRate_DEGpSEC * 16.0f),
        constrainToInt16(rfValues.m_yawRate_DEGpSEC * 16.0f)
    );

    fakeBaroSet(altitudeToPressure(altitude), DEGREES_TO_CENTIDEGREES(21));
    fakePitotSetAirspeed(rfValues.m_airspeed_MPS * 100);

    fakeBattSensorSetVbat((uint16_t)roundf(rfValues.m_batteryVoltage_VOLTS * 100));
    fakeBattSensorSetAmperage((uint16_t)roundf(rfValues.m_batteryCurrentDraw_AMPS * 100)); 

    fpQuaternion_t quat;
    fpVector3_t north;
    north.x = 1.0f;
    north.y = 0;
    north.z = 0;
    computeQuaternionFromRPY(&quat, roll_inav, pitch_inav, yaw_inav);
    transformVectorEarthToBody(&north, &quat);
    fakeMagSet(
        constrainToInt16(north.x * 16000.0f),
        constrainToInt16(north.y * 16000.0f),
        constrainToInt16(north.z * 16000.0f)
    );

    free(rfValues.m_currentAircraftStatus);
    free(response);
}

static void* soapWorker(void* arg)
{
    UNUSED(arg);
    while(true)
    {     
        if (!isInitalised) {
            startRequest("RestoreOriginalControllerDevice", "<RestoreOriginalControllerDevice><a>1</a><b>2</b></RestoreOriginalControllerDevice>");
            free(endRequest());
            startRequest("InjectUAVControllerInterface", "<InjectUAVControllerInterface><a>1</a><b>2</b></InjectUAVControllerInterface>");
            free(endRequest());  
            exchangeData();
            ENABLE_ARMING_FLAG(SIMULATOR_MODE_SITL);
            
            isInitalised = true;  
        }

        exchangeData();
        unlockMainPID();
    }

    return NULL;
}


static void* creationWorker(void* arg)
{
    char* ip = (char*)arg;
    
    while (true) {
        pthread_mutex_lock(&sockmtx);
        while (clientNext != NULL) {
            pthread_cond_wait(&sockcond2, &sockmtx);
        }
        pthread_mutex_unlock(&sockmtx);
        
        soap_client_t *cli = malloc(sizeof(soap_client_t));
        if (!soapClientConnect(cli, ip, RF_PORT)) {
            continue;
        }
        
        clientNext = cli;
        pthread_mutex_lock(&sockmtx);
        pthread_cond_broadcast(&sockcond1);
        pthread_mutex_unlock(&sockmtx);
    }

    return NULL;
}

bool simRealFlightInit(char* ip, uint8_t* mapping, uint8_t mapCount, bool imu)
{
    memcpy(pwmMapping, mapping, mapCount);
    mappingCount = mapCount;
    useImu = imu;

    if (pthread_create(&soapThread, NULL, soapWorker, NULL) < 0) {
        return false;
    }

    if (pthread_create(&creationThread, NULL, creationWorker, (void*)ip) < 0) {
        return false;
    }

    // Wait until the connection is established, the interface has been initialised 
    // and the first valid packet has been received to avoid problems with the startup calibration.   
    while (!isInitalised) {
        delay(250);
    }

    return true;
}
