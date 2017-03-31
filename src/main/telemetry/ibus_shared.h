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

#include "io/serial.h"

#define IBUS_TASK_PERIOD_US (500)
#define IBUS_BAUDRATE      (115200)
#define IBUS_CYCLE_TIME_MS (8)
#define IBUS_CHECKSUM_SIZE (2)
#define IBUS_MIN_LEN       (2 + IBUS_CHECKSUM_SIZE)
#define IBUS_MAX_TX_LEN    (6)
#define IBUS_MAX_RX_LEN    (4)
#define IBUS_RX_BUF_LEN    (IBUS_MAX_RX_LEN)

#if defined(TELEMETRY) && defined(TELEMETRY_IBUS)

typedef enum {
    IBUS_MEAS_TYPE_INTERNAL_VOLTAGE = 0x00, //0 Internal Voltage
    IBUS_MEAS_TYPE_TEMPERATURE      = 0x01, //0 Temperature -##0.0 C, 0=-40.0 C, 400=0.0 C, 65535=6513.5 C
    IBUS_MEAS_TYPE_RPM              = 0x02, //0 Rotation RPM, ####0RPM, 0=0RPM, 65535=65535RPM
    IBUS_MEAS_TYPE_EXTERNAL_VOLTAGE = 0x03, //0 External Voltage, -##0.00V, 0=0.00V, 32767=327.67V, 32768=na, 32769=-327.67V, 65535=-0.01V
    IBUS_MEAS_TYPE_HEADING          = 0x04, //3 
    IBUS_MEAS_TYPE_CURRENT          = 0x05, //3 
    IBUS_MEAS_TYPE_CLIMB            = 0x06, //3
    IBUS_MEAS_TYPE_ACC_Z            = 0x07, //3 
    IBUS_MEAS_TYPE_ACC_Y            = 0x08, //3 
    IBUS_MEAS_TYPE_ACC_X            = 0x09, //3 
    IBUS_MEAS_TYPE_VSPEED           = 0x0a, //3
    IBUS_MEAS_TYPE_SPEED            = 0x0b, //3
    IBUS_MEAS_TYPE_DIST             = 0x0c, //3 
    IBUS_MEAS_TYPE_ARMED            = 0x0d,	//3 
    IBUS_MEAS_TYPE_MODE             = 0x0e, //3 
    //IBUS_MEAS_TYPE_RESERVED         = 0x0f, //3 
    IBUS_MEAS_TYPE_PRES             = 0x41, // Pressure, not work
    //IBUS_MEAS_TYPE_ODO1             = 0x7c, // Odometer1, 0.0km, 0.0 only
    //IBUS_MEAS_TYPE_ODO2             = 0x7d, // Odometer2, 0.0km, 0.0 only
    IBUS_MEAS_TYPE_SPE              = 0x7e, //1 Speed km/h, ###0km/h, 0=0Km/h, 1000=100Km/h
    IBUS_MEAS_TYPE_COG              = 0x80, //3 2byte course deg * 100, 0.0..359.99
    IBUS_MEAS_TYPE_GPS_STATUS       = 0x81, //3 2byte special parse byte by byte
    IBUS_MEAS_TYPE_GPS_LON          = 0x82, //3 4byte signed WGS84 in deg * 1E7, format into %u?%02u'%02u
    IBUS_MEAS_TYPE_GPS_LAT          = 0x83, //3 4byte signed WGS84 in deg * 1E7 
    IBUS_MEAS_TYPE_ALT              = 0x84, //3 2byte signed barometer alt
    IBUS_MEAS_TYPE_S85              = 0x85, //3 
    IBUS_MEAS_TYPE_S86              = 0x86, //3 
    IBUS_MEAS_TYPE_S87              = 0x87, //3 
    IBUS_MEAS_TYPE_S88              = 0x88, //3 
    IBUS_MEAS_TYPE_S89              = 0x89, //3 
    IBUS_MEAS_TYPE_S8A              = 0x8A, //3 
    IBUS_MEAS_TYPE_GALT             = 0xf9, //2 Altitude m, not work
    //IBUS_MEAS_TYPE_SNR              = 0xfa, //  SNR, not work
    //IBUS_MEAS_TYPE_NOISE            = 0xfb, //  Noise, not work
    //IBUS_MEAS_TYPE_RSSI             = 0xfc, //  RSSI, not work
    IBUS_MEAS_TYPE_GPS              = 0xfd //  1byte fix 1byte satellites 4byte LAT 4byte LON 4byte alt, return UNKNOWN sensor and no formating
    //IBUS_MEAS_TYPE_ERR              = 0xfe  //0 Error rate, #0%
} ibusSensorType_e;

typedef enum {
    IBUS_MEAS_VALUE_NONE             = 0x00, //2
    IBUS_MEAS_VALUE_TEMPERATURE      = 0x01, //2
    IBUS_MEAS_VALUE_RPM              = 0x02, //2
    IBUS_MEAS_VALUE_EXTERNAL_VOLTAGE = 0x03, //2
    IBUS_MEAS_VALUE_HEADING          = 0x04, //2
    IBUS_MEAS_VALUE_CURRENT          = 0x05, //2
    IBUS_MEAS_VALUE_CLIMB            = 0x06, //2
    IBUS_MEAS_VALUE_ACC_Z            = 0x07, //2
    IBUS_MEAS_VALUE_ACC_Y            = 0x08, //2
    IBUS_MEAS_VALUE_ACC_X            = 0x09, //2
    IBUS_MEAS_VALUE_VSPEED           = 0x0a, //2
    IBUS_MEAS_VALUE_SPEED            = 0x0b, //2
    IBUS_MEAS_VALUE_DIST             = 0x0c, //2
    IBUS_MEAS_VALUE_ARMED            = 0x0d, //2
    IBUS_MEAS_VALUE_MODE             = 0x0e, //2
    IBUS_MEAS_VALUE_PRES             = 0x41, //2
    IBUS_MEAS_VALUE_SPE              = 0x7e, //2
    IBUS_MEAS_VALUE_COG              = 0x80, //2
    IBUS_MEAS_VALUE_GPS_STATUS       = 0x81, //2
    IBUS_MEAS_VALUE_GPS_LON          = 0x82, //4
    IBUS_MEAS_VALUE_GPS_LAT          = 0x83, //4
    IBUS_MEAS_VALUE_ALT              = 0x84, //2
    IBUS_MEAS_VALUE_STATUS           = 0x85, //2
    IBUS_MEAS_VALUE_GPS_LAT1         = 0x86, //2
    IBUS_MEAS_VALUE_GPS_LON1         = 0x87, //2
    IBUS_MEAS_VALUE_GPS_LAT2         = 0x88, //2
    IBUS_MEAS_VALUE_GPS_LON2         = 0x89, //2
    IBUS_MEAS_VALUE_ALT4             = 0x8A, //4
    IBUS_MEAS_VALUE_GALT             = 0xf9, //2
    IBUS_MEAS_VALUE_GPS              = 0xfd //14 1byte fix 1byte satellites 4byte LAT 4byte LON 4byte alt
} ibusSensorValue_e;

uint8_t respondToIbusRequest(uint8_t ibusPacket[static IBUS_RX_BUF_LEN]);
void initSharedIbusTelemetry(serialPort_t *port);
void changeTypeIbusTelemetry(uint8_t id, uint8_t type, uint8_t value);

#endif //defined(TELEMETRY) && defined(TELEMETRY_IBUS)

bool isChecksumOk(uint8_t ibusPacket[static IBUS_CHECKSUM_SIZE], size_t packetLength);
