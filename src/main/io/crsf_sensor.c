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
#include <math.h>
#include <string.h>

#include "platform.h"

#ifdef USE_CRSF_SENSOR_INPUT

#include "build/build_config.h"

#include "common/crc.h"
#include "common/utils.h"

#include "drivers/serial.h"
#include "drivers/time.h"

#include "io/serial.h"
#include "io/crsf_sensor.h"

#include "rx/crsf.h"

#ifdef USE_GPS_PROTO_CRSF
#include "io/gps.h"
#include "io/gps_private.h"
#endif

#ifdef USE_BARO_CRSF
#include "drivers/barometer/barometer_crsf.h"
#endif

#ifdef USE_BATTERY_SENSOR_CRSF
#include "sensors/battery_sensor_crsf.h"
#endif

#define CRSF_SENSOR_BAUDRATE        420000
#define CRSF_SENSOR_PORT_OPTIONS    (SERIAL_STOPBITS_1 | SERIAL_PARITY_NO)
#define CRSF_SENSOR_TIME_NEEDED_PER_FRAME_US   1750

#define CRSF_SENSOR_VARIO_TIMEOUT_MS    2000

static serialPort_t *crsfSensorPort;
static uint8_t crsfSensorFrame[CRSF_FRAME_SIZE_MAX];
static uint8_t crsfSensorFramePosition;
static timeUs_t crsfSensorFrameStartAtUs;
static volatile bool crsfSensorFrameDone;

// Vario data storage
static int16_t crsfSensorVario;
static timeMs_t crsfSensorVarioLastUpdateMs;

static uint8_t crsfSensorFrameCRC(void)
{
    // CRC includes type and payload (bytes 2..frameLength)
    uint8_t crc = crc8_dvb_s2(0, crsfSensorFrame[2]);  // type
    const uint8_t frameLength = crsfSensorFrame[1];
    for (int ii = 0; ii < frameLength - CRSF_FRAME_LENGTH_TYPE_CRC; ++ii) {
        crc = crc8_dvb_s2(crc, crsfSensorFrame[3 + ii]);
    }
    return crc;
}

// Big-endian decode helpers matching CRSF wire format
static int32_t crsfSensorRead32(const uint8_t *p)
{
    return (int32_t)((uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]);
}

static uint16_t crsfSensorReadU16(const uint8_t *p)
{
    return (uint16_t)(p[0] << 8 | p[1]);
}

static int16_t crsfSensorRead16(const uint8_t *p)
{
    return (int16_t)crsfSensorReadU16(p);
}

#ifdef USE_GPS_PROTO_CRSF
static void crsfSensorHandleGPS(const uint8_t *payload)
{
    // GPS payload: lat(4) lon(4) groundspeed(2) heading(2) altitude(2) sats(1)
    int32_t lat = crsfSensorRead32(payload);
    int32_t lon = crsfSensorRead32(payload + 4);
    uint16_t groundSpeed = crsfSensorReadU16(payload + 8);   // km/h * 10
    uint16_t heading = crsfSensorReadU16(payload + 10);      // degrees * 100
    uint16_t altitude = crsfSensorReadU16(payload + 12);     // meters + 1000 offset
    uint8_t numSat = payload[14];

    gpsSolDRV.llh.lat = lat;    // degree / 10^7, same as INAV
    gpsSolDRV.llh.lon = lon;
    gpsSolDRV.llh.alt = (int32_t)(altitude - 1000) * 100;  // meters to cm
    gpsSolDRV.groundSpeed = (groundSpeed * 100 + 18) / 36;  // km/h*10 to cm/s
    gpsSolDRV.groundCourse = heading / 10;  // centidegrees to decidegrees
    gpsSolDRV.numSat = numSat;

    if (numSat >= 4) {
        gpsSolDRV.fixType = GPS_FIX_3D;
    } else if (numSat >= 2) {
        gpsSolDRV.fixType = GPS_FIX_2D;
    } else {
        gpsSolDRV.fixType = GPS_NO_FIX;
    }

    // CRSF GPS frame doesn't include NED velocity or accuracy
    gpsSolDRV.flags.validVelNE = false;
    gpsSolDRV.flags.validVelD = false;
    gpsSolDRV.flags.validEPE = false;
    gpsSolDRV.flags.validTime = false;

    gpsSolDRV.eph = gpsConstrainEPE(200);   // default ~2m
    gpsSolDRV.epv = gpsConstrainEPE(400);   // default ~4m
    gpsSolDRV.hdop = gpsConstrainHDOP(200);

    gpsProcessNewDriverData();
    crsfGPSNewDataReady();
}
#endif

#ifdef USE_BARO_CRSF
static void crsfSensorHandleBaro(const uint8_t *payload)
{
    // Baro payload: altitude_packed (uint16_t big-endian)
    // Format matches outgoing CRSF baro encoding:
    //   bit 15 clear: altitude_dm = packed - 10000 (fine, dm resolution)
    //   bit 15 set:   altitude_dm = (packed & 0x7fff) * 10 - 5 (coarse, meter resolution)
    uint16_t packed = crsfSensorReadU16(payload);
    int32_t altitude_dm;

    if (packed & 0x8000) {
        altitude_dm = (int32_t)(packed & 0x7fff) * 10 - 5;
    } else {
        altitude_dm = (int32_t)packed - 10000;
    }

    // Convert altitude (dm) to pressure using ISA formula:
    // P = 101325 * (1 - h/44330)^5.255
    float altitude_m = altitude_dm / 10.0f;
    float pressure_ratio = 1.0f - altitude_m / 44330.0f;

    // Clamp to avoid negative values from extreme altitudes
    if (pressure_ratio < 0.01f) {
        pressure_ratio = 0.01f;
    }

    float pressure_pa = 101325.0f * powf(pressure_ratio, 5.255f);
    crsfBaroReceiveNewData((int32_t)pressure_pa, 2500);  // 25.00 degC default
}
#endif

#ifdef USE_BATTERY_SENSOR_CRSF
static void crsfSensorHandleBattery(const uint8_t *payload)
{
    // Battery payload: voltage(2) current(2) capacity(3) remaining(1)
    // Voltage: decivolts on wire, INAV uses 0.01V
    // Current: deciamps on wire, INAV uses 0.01A
    uint16_t voltage_dv = crsfSensorReadU16(payload);
    uint16_t current_da = crsfSensorReadU16(payload + 2);
    uint32_t capacity_mah = ((uint32_t)payload[4] << 16) | ((uint32_t)payload[5] << 8) | payload[6];
    uint8_t remaining = payload[7];

    crsfBatterySensorReceiveNewData(
        voltage_dv * 10,    // decivolts to 0.01V
        current_da * 10,    // deciamps to 0.01A (centamps)
        capacity_mah,
        remaining
    );
}
#endif

static void crsfSensorHandleVario(const uint8_t *payload)
{
    crsfSensorVario = crsfSensorRead16(payload);  // cm/s
    crsfSensorVarioLastUpdateMs = millis();
}

static void crsfSensorDispatchFrame(void)
{
    const uint8_t frameLength = crsfSensorFrame[1];
    const uint8_t fullFrameLength = frameLength + CRSF_FRAME_LENGTH_ADDRESS + CRSF_FRAME_LENGTH_FRAMELENGTH;
    const uint8_t type = crsfSensorFrame[2];

    // CRC check
    const uint8_t crc = crsfSensorFrameCRC();
    if (crc != crsfSensorFrame[fullFrameLength - 1]) {
        return;
    }

    const uint8_t *payload = &crsfSensorFrame[3];

    switch (type) {
#ifdef USE_GPS_PROTO_CRSF
        case CRSF_FRAMETYPE_GPS:
            if (frameLength >= CRSF_FRAME_GPS_PAYLOAD_SIZE + CRSF_FRAME_LENGTH_TYPE_CRC) {
                crsfSensorHandleGPS(payload);
            }
            break;
#endif

#ifdef USE_BARO_CRSF
        case CRSF_FRAMETYPE_BAROMETER_ALTITUDE:
            if (frameLength >= CRSF_FRAME_BAROMETER_ALTITUDE_PAYLOAD_SIZE + CRSF_FRAME_LENGTH_TYPE_CRC) {
                crsfSensorHandleBaro(payload);
            }
            break;
#endif

#ifdef USE_BATTERY_SENSOR_CRSF
        case CRSF_FRAMETYPE_BATTERY_SENSOR:
            if (frameLength >= CRSF_FRAME_BATTERY_SENSOR_PAYLOAD_SIZE + CRSF_FRAME_LENGTH_TYPE_CRC) {
                crsfSensorHandleBattery(payload);
            }
            break;
#endif

        case CRSF_FRAMETYPE_VARIO_SENSOR:
            if (frameLength >= CRSF_FRAME_VARIO_SENSOR_PAYLOAD_SIZE + CRSF_FRAME_LENGTH_TYPE_CRC) {
                crsfSensorHandleVario(payload);
            }
            break;

        default:
            break;
    }
}

static void crsfSensorDataReceive(uint16_t c, void *rxCallbackData)
{
    UNUSED(rxCallbackData);

    const timeUs_t currentTimeUs = microsISR();

    if (cmpTimeUs(currentTimeUs, crsfSensorFrameStartAtUs) > CRSF_SENSOR_TIME_NEEDED_PER_FRAME_US) {
        crsfSensorFramePosition = 0;
    }

    if (crsfSensorFramePosition == 0) {
        crsfSensorFrameStartAtUs = currentTimeUs;
    }

    const int fullFrameLength = crsfSensorFramePosition < 3 ? 5 : crsfSensorFrame[1] + CRSF_FRAME_LENGTH_ADDRESS + CRSF_FRAME_LENGTH_FRAMELENGTH;

    if (fullFrameLength > CRSF_FRAME_SIZE_MAX) {
        crsfSensorFramePosition = 0;
        return;
    }

    if (crsfSensorFramePosition < fullFrameLength) {
        crsfSensorFrame[crsfSensorFramePosition++] = (uint8_t)c;
        if (crsfSensorFramePosition >= fullFrameLength) {
            crsfSensorFrameDone = true;
            crsfSensorFramePosition = 0;
        }
    }
}

void crsfSensorInputInit(void)
{
    const serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_CRSF_SENSOR);
    if (!portConfig) {
        return;
    }

    crsfSensorPort = openSerialPort(portConfig->identifier,
        FUNCTION_CRSF_SENSOR,
        crsfSensorDataReceive,
        NULL,
        CRSF_SENSOR_BAUDRATE,
        MODE_RX,
        CRSF_SENSOR_PORT_OPTIONS
    );

    crsfSensorVario = 0;
    crsfSensorVarioLastUpdateMs = 0;
}

bool crsfSensorVarioIsValid(void)
{
    return (crsfSensorVarioLastUpdateMs > 0) &&
           ((millis() - crsfSensorVarioLastUpdateMs) < CRSF_SENSOR_VARIO_TIMEOUT_MS);
}

int16_t crsfSensorGetVario(void)
{
    return crsfSensorVario;
}

// Called from scheduler to process completed frames outside ISR context
void crsfSensorProcess(void)
{
    if (crsfSensorFrameDone) {
        crsfSensorFrameDone = false;
        crsfSensorDispatchFrame();
    }
}

#endif
