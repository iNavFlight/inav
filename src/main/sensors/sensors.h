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

typedef enum {
    SENSOR_INDEX_GYRO = 0,
    SENSOR_INDEX_ACC,
    SENSOR_INDEX_BARO,
    SENSOR_INDEX_MAG,
    SENSOR_INDEX_RANGEFINDER,
    SENSOR_INDEX_PITOT,
    SENSOR_INDEX_OPFLOW,
    SENSOR_INDEX_COUNT
} sensorIndex_e;

typedef struct int16_flightDynamicsTrims_s {
    int16_t roll;
    int16_t pitch;
    int16_t yaw;
} flightDynamicsTrims_def_t;

typedef union flightDynamicsTrims_u {
    int16_t raw[3];
    flightDynamicsTrims_def_t values;
} flightDynamicsTrims_t;

#define CALIBRATING_BARO_TIME_MS            2000
#define CALIBRATING_PITOT_TIME_MS           4000
#define CALIBRATING_GYRO_TIME_MS            2000
#define CALIBRATING_ACC_TIME_MS             500
#define CALIBRATING_GYRO_MORON_THRESHOLD    32

// These bits have to be aligned with sensorIndex_e
typedef enum {
    SENSOR_GYRO = 1 << 0, // always present
    SENSOR_ACC = 1 << 1,
    SENSOR_BARO = 1 << 2,
    SENSOR_MAG = 1 << 3,
    SENSOR_RANGEFINDER = 1 << 4,
    SENSOR_PITOT = 1 << 5,
    SENSOR_OPFLOW = 1 << 6,
    SENSOR_GPS = 1 << 7,
    SENSOR_GPSMAG = 1 << 8,
    SENSOR_TEMP = 1 << 9
} sensors_e;

typedef enum {
    SENSOR_TEMP_CAL_INITIALISE,
    SENSOR_TEMP_CAL_IN_PROGRESS,
    SENSOR_TEMP_CAL_COMPLETE,
} sensorTempCalState_e;

typedef struct sensor_compensation_s {
    float correctionFactor;
    float referenceMeasurement;
    int16_t referenceTemp;
    int16_t lastTemp;
    sensorTempCalState_e calibrationState;
} sensor_compensation_t;

float applySensorTempCompensation(int16_t sensorTemp, float sensorMeasurement, sensorIndex_e sensorType);
extern uint8_t requestedSensors[SENSOR_INDEX_COUNT];
extern uint8_t detectedSensors[SENSOR_INDEX_COUNT];
