/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */


#pragma once

#include <stdint.h>

#include "platform.h"

#ifdef USE_TELEMETRY_SBUS2

// Sensor code from https://github.com/BrushlessPower/SBUS2-Telemetry
// SBUS2 telemetry: 2ms deadtime after rc package
// One new slot every 700us

/*
 * ++++++++++++++++++++++++++++++++
 * Temperature Sensors
 * ++++++++++++++++++++++++++++++++
 */
void send_temp125(uint8_t port, int16_t temp);
void send_alarm_as_temp125(uint8_t port, int16_t alarm);
void send_SBS01TE(uint8_t port, int16_t temp);
void send_SBS01T(uint8_t port, int16_t temp);
void send_F1713(uint8_t port, int16_t temp);

/*
 * ++++++++++++++++++++++++++++++++
 * RPM Sensors
 * ++++++++++++++++++++++++++++++++
 */
void send_RPM(uint8_t port, uint32_t RPM);
void send_SBS01RB(uint8_t port, uint32_t RPM);
void send_SBS01RM(uint8_t port, uint32_t RPM);
void send_SBS01RO(uint8_t port, uint32_t RPM);
void send_SBS01R(uint8_t port, uint32_t RPM);

/*
 * ++++++++++++++++++++++++++++++++
 * Voltage/Current Sensors
 * ++++++++++++++++++++++++++++++++
 */
void send_voltage(uint8_t port,uint16_t voltage1, uint16_t voltage2);
void send_voltagef(uint8_t port,float voltage1, float voltage2);
void send_s1678_current(uint8_t port, uint16_t current, uint16_t capacity, uint16_t voltage);
void send_s1678_currentf(uint8_t port, float current, uint16_t capacity, float voltage);
void send_SBS01C(uint8_t port, uint16_t current, uint16_t capacity, uint16_t voltage);
void send_SBS01Cf(uint8_t port, float current, uint16_t capacity, float voltage);
void send_F1678(uint8_t port, uint16_t current, uint16_t capacity, uint16_t voltage);
void send_F1678f(uint8_t port, float current, uint16_t capacity, float voltage);
void send_SBS01V(uint8_t port,uint16_t voltage1, uint16_t voltage2);
void send_SBS01Vf(uint8_t port,float voltage1, float voltage2);


/*
 * ++++++++++++++++++++++++++++++++
 * Vario Sensors
 * ++++++++++++++++++++++++++++++++
 */
void send_f1712_vario(uint8_t port, int16_t altitude, int16_t vario);
void send_f1712_variof(uint8_t port, int16_t altitude, float vario);
void send_f1672_vario(uint8_t port, int16_t altitude, int16_t vario);
void send_f1672_variof(uint8_t port, int16_t altitude, float vario);
void send_F1712(uint8_t port, int16_t altitude, int16_t vario);
void send_F1712f(uint8_t port, int16_t altitude, float vario);
void send_F1672(uint8_t port, int16_t altitude, int16_t vario);
void send_F1672f(uint8_t port, int16_t altitude, float vario);

/*
 * ++++++++++++++++++++++++++++++++
 * GPS Sensors
 * Note the different Input Types!
 * Example:
 * Position Berlin Fernsehturm
 * https://www.koordinaten-umrechner.de/decimal/52.520832,13.409430?karte=OpenStreetMap&zoom=19
 * Degree Minutes 52° 31.2499 and 13° 24.5658
 * Decimal Degree 52.520832 and 13.409430
 * ++++++++++++++++++++++++++++++++
 */
// Degree Minutes as Integer -> 52312499
void send_f1675_gps(uint8_t port, uint16_t speed, int16_t hight, int16_t vario, int32_t latitude, int32_t longitude); 
// Degree Minutes as Integer -> 52 and 312499
void send_F1675min(uint8_t port, uint16_t speed, int16_t hight, int16_t vario, int8_t lat_deg, int32_t lat_min, int8_t lon_deg, int32_t lon_min); 
// Degree Minutes as Float -> 52 and 31.2499
void send_F1675minf(uint8_t port, uint16_t speed, int16_t hight, int16_t vario, int8_t lat_deg, float lat_min, int8_t lon_deg, float lon_min); 
// Decimal Degrees as Float -> 52.520832
void send_F1675f(uint8_t port, uint16_t speed, int16_t hight, int16_t vario, float latitude, float longitude);
// Decimal Degrees as Integer -> 52520832
void send_F1675(uint8_t port, uint16_t speed, int16_t hight, int16_t vario, int32_t latitude, int32_t longitude);
void send_SBS10G(uint8_t port, uint16_t hours, uint16_t minutes, uint16_t seconds, float latitude, float longitude, float altitudeMeters, uint16_t speed, float gpsVario);

/*
 * ++++++++++++++++++++++++++++++++
 * ESC Sensors
 * Note These sensors only exists on the newer Futaba Radios 18SZ, 16IZ, etc
 * ++++++++++++++++++++++++++++++++
 */

void send_kontronik(uint8_t port,  uint16_t voltage, uint16_t capacity, uint32_t rpm, uint16_t current, uint16_t temp, uint16_t becTemp, uint16_t becCurrent, uint16_t pwm);
void send_scorpion(uint8_t port,  uint16_t voltage, uint16_t capacity, uint32_t rpm, uint16_t current, uint16_t temp, uint16_t becTemp, uint16_t becCurrent, uint16_t pwm); 

void send_jetcat(uint8_t port, uint32_t rpm, uint16_t egt, uint16_t pump_volt, uint32_t setrpm, uint16_t thrust, uint16_t fuel, uint16_t fuelflow, uint16_t altitude, uint16_t quality, uint16_t volt, uint16_t current, uint16_t speed, uint16_t status, uint32_t secondrpm);

void SBUS2_transmit_telemetry_data(uint8_t slotId , const uint8_t *bytes);

#endif