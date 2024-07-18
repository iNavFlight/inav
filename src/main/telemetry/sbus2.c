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

#include <stdint.h>

#include "platform.h"

#include "build/debug.h"

#include "common/utils.h"
#include "common/time.h"
#include "common/axis.h"

#include "drivers/timer.h"

#include "telemetry/telemetry.h"
#include "telemetry/sbus2.h"
#include "telemetry/sbus2_sensors.h"


#include "rx/sbus.h"

#include "sensors/battery.h"
#include "sensors/sensors.h"
#include "sensors/temperature.h"
#include "sensors/diagnostics.h"

#include "io/gps.h"

#include "navigation/navigation.h"

#ifdef USE_ESC_SENSOR
#include "sensors/esc_sensor.h"
#include "flight/mixer.h"
#endif

#ifdef USE_TELEMETRY_SBUS2

const uint8_t sbus2SlotIds[SBUS2_SLOT_COUNT] = {
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
    0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
    0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB
};

sbus2_telemetry_frame_t sbusTelemetryData[SBUS2_SLOT_COUNT] = {};
uint8_t sbusTelemetryDataUsed[SBUS2_SLOT_COUNT] = {};
timeUs_t sbusTelemetryMinDelay[SBUS2_SLOT_COUNT] = {};

void handleSbus2Telemetry(timeUs_t currentTimeUs) 
{
    UNUSED(currentTimeUs);

    float voltage = getBatteryVoltage() * 0.01f;
    float cellVoltage = getBatteryAverageCellVoltage() * 0.01f;
    float current = getAmperage() * 0.01f;
    float capacity = getMAhDrawn();
    float altitude = getEstimatedActualPosition(Z) * 0.01f;
    float vario = getEstimatedActualVelocity(Z);
    float temperature = 0;
    uint32_t rpm = 0;

#ifdef USE_ESC_SENSOR
    escSensorData_t * escSensor = escSensorGetData();
    if (escSensor && escSensor->dataAge <= ESC_DATA_MAX_AGE) {
        rpm = escSensor->rpm;
        temperature = escSensor->temperature;
    } else {
        rpm = 0;
        temperature = 0;
    }
#endif

    //temperature = 42.16f;

    DEBUG_SET(DEBUG_SBUS2, 0, voltage);
    DEBUG_SET(DEBUG_SBUS2, 1, cellVoltage);
    DEBUG_SET(DEBUG_SBUS2, 2, current);
    DEBUG_SET(DEBUG_SBUS2, 3, capacity);
    DEBUG_SET(DEBUG_SBUS2, 4, altitude);
    DEBUG_SET(DEBUG_SBUS2, 5, vario);
    DEBUG_SET(DEBUG_SBUS2, 6, rpm);
    DEBUG_SET(DEBUG_SBUS2, 7, temperature);

    // 2 slots
    send_voltagef(1, voltage, cellVoltage);
    // 3 slots
    send_s1678_currentf(3, current, capacity, voltage);
    // 1 slot
    send_RPM(6, rpm);
    // 1 slot - esc temp
    static int change = 0;
    change++;
    int delta = change / 10;
    delta = delta % 20;
    send_SBS01T(7, temperature);

    // 8 slots, gps
    uint16_t speed = 0;
    float latitude = 0;
    float longitude = 0;

#ifdef USE_GPS
    if (gpsSol.fixType >= GPS_FIX_2D) {
        speed = (CMSEC_TO_KPH(gpsSol.groundSpeed) + 0.5f);
        latitude = gpsSol.llh.lat * 1e-7;
        longitude = gpsSol.llh.lon * 1e-7;
    }
#endif

    send_F1675f(8, speed, altitude, vario, latitude, longitude);
    // imu 1 slot
    int16_t temp16;
    bool valid = getIMUTemperature(&temp16);
    send_SBS01T(16, valid ? temp16 / 10 : 0);
    // baro
    valid = 0;
    valid = getBaroTemperature(&temp16);
    send_SBS01T(17, valid ? temp16 / 10 : 0);
    // temp sensors 18-25
#ifdef USE_TEMPERATURE_SENSOR
    for(int i = 0; i < 8; i++) {
        temp16 = 0;
        valid = getSensorTemperature(0, &temp16);
        send_SBS01T(18 + i, valid ? temp16 / 10 : 0);
    }
#else
    for(int i = 0; i < 8; i++) {
        send_SBS01T(18 + i, 0);
    }
#endif

    // 8 slots - gps
    // 
}

uint8_t sbus2GetTelemetrySlot(timeUs_t elapsed)
{
    if (elapsed < SBUS2_DEADTIME) {
        return 0xFF; // skip it
    }

    elapsed = elapsed - SBUS2_DEADTIME;

    uint8_t slot = (elapsed % SBUS2_SLOT_TIME) - 1;

    if (elapsed - (slot * SBUS2_SLOT_TIME) > SBUS2_SLOT_DELAY_MAX) {
        slot = 0xFF;
    }

    return slot;
}

FAST_CODE void taskSendSbus2Telemetry(timeUs_t currentTimeUs)
{
    if (!telemetrySharedPort || rxConfig()->receiverType != RX_TYPE_SERIAL ||
        rxConfig()->serialrx_provider != SERIALRX_SBUS2) {
        return;
    }

    timeUs_t elapsedTime = currentTimeUs - sbusGetLastFrameTime();

    if(elapsedTime > MS2US(8)) {
        return;
    }


    uint8_t telemetryPage = sbusGetCurrentTelemetryPage();

    uint8_t slot = sbus2GetTelemetrySlot(elapsedTime);

    if(slot < SBUS2_TELEMETRY_SLOTS) {
        int slotIndex = (telemetryPage * SBUS2_TELEMETRY_SLOTS) + slot;
        if (slotIndex < SBUS2_SLOT_COUNT) {
            if (sbusTelemetryDataUsed[slotIndex] != 0 && sbusTelemetryMinDelay[slotIndex] < currentTimeUs) {
                sbusTelemetryData[slotIndex].slotId = sbus2SlotIds[slotIndex];
                // send
                serialWriteBuf(telemetrySharedPort,
                               (const uint8_t *)&sbusTelemetryData[slotIndex],
                               sizeof(sbus2_telemetry_frame_t));
                sbusTelemetryMinDelay[slotIndex] = currentTimeUs + MS2US(1);
                //sbusTelemetryDataUsed[slotIndex] = 0;
            }
        }
    }
}



void sbus2startDeadTime(timeUs_t currentTime)
{
    UNUSED(currentTime);
}

void initSbus2Telemetry(void)
{
    /*
    const timerHardware_t *timerRx = timerGetByUsageFlag(TIM_USE_ANY);
    TCH_t *tch = timerGetTCH(timerRx);

    uint32_t baseClock = timerClock(tch->timHw->tim);
    uint32_t clock = baseClock;
    uint32_t timerPeriod;
    uint32_t baud = 1000000;
    

    do {
        timerPeriod = clock / baud;
        if (timerPeriod > 0xFFFF) {
            if (clock > 1) {
                clock = clock / 2;   // this is wrong - mhz stays the same ... This will double baudrate until ok (but minimum baudrate is < 1200)
            } else {
                // TODO unable to continue, unable to determine clock and timerPeriods for the given baud
            }
        }
    } while (timerPeriod > 0xFFFF);

    timerConfigure(tch, timerPeriod, baseClock);
    */
}

#endif
