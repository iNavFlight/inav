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

#define SBUS2_TELEMETRY_PAYLOAD_SIZE 3

#define SBUS2_TELEMETRY_ITEM_SIZE   3
#define SBUS2_TELEMETRY_SLOTS       8
#define SBUS2_TELEMETRY_PAGES       4
#define SBUS2_SLOT_COUNT            (SBUS2_TELEMETRY_PAGES * SBUS2_TELEMETRY_SLOTS)

#if defined(USE_TELEMETRY) && defined(USE_SBUS2_TELEMETRY)

// Information on SBUS2 sensors from: https://github.com/BrushlessPower/SBUS2-Telemetry/tree/master
// Temperature:
// Max 125C
// value | 0x4000
typedef struct sbus2_telemetry_temp_payload_s {
    uint8_t tempHigh; // temp | 0x4000; // 125c
    uint8_t tempLow;
} __attribute__((packed)) sbsu2_telemetry_temp_payload_t;

// Temperature:
// Max 200C
// temp | 0x8000
typedef struct sbus2_telemetry_temp200_payload_s {
    uint8_t tempLow; // temp | 0x8000; // 200c
    uint8_t tempHigh;
} __attribute__((packed)) sbsu2_telemetry_temp200_payload_t;

// RPM:
// (RPM / 6) max: 0xFFFF
typedef struct sbus2_telemetry_rpm_payload_s {
    uint8_t rpmHigh; // RPM / 6, capped at 0xFFFF
    uint8_t rpmLow;
} __attribute__((packed)) sbsu2_telemetry_rpm_payload_t;

// Voltage: 1 or 2 slots
// 0x8000 = rx voltage?
// max input: 0x1FFF
typedef struct sbus2_telemetry_voltage_payload_s {
    uint8_t voltageHigh; // 2 slots // Voltage 1: value | 0x8000 
    uint8_t voltageLow;  // max input value: 0x1FFF
} __attribute__((packed)) sbsu2_telemetry_voltage_payload_t;

// Current
// 3 frames
// 1: current
// Max input: 0x3FFF
// input |= 0x4000
// input &= 0x7FFF
// 2: voltage
// same as voltage frame. may not need ot be capped.
// 3: Capacity
typedef struct sbus2_telemetry_current_payload_s {
    uint8_t currentHigh;
    uint8_t currentLow;
} __attribute__((packed)) sbsu2_telemetry_current_payload_t;

typedef struct sbus2_telemetry_capacity_payload_s {
    uint8_t capacityHigh;
    uint8_t capacityLow;
} __attribute__((packed)) sbsu2_telemetry_capacity_payload_t;

// GPS
// frames:
// 1: Speed
// 2: Altitude
// 3: Vario
// 4,5: LAT
// 5,6: LON

typedef struct sbus2_telemetry_frame_s {
    uint8_t slotId;
    union
    {
        uint8_t data[2];
        sbsu2_telemetry_temp_payload_t temp125;
        sbsu2_telemetry_temp200_payload_t temp200;
        sbsu2_telemetry_rpm_payload_t rpm;
        sbsu2_telemetry_voltage_payload_t voltage;
        sbsu2_telemetry_current_payload_t current;
        sbsu2_telemetry_capacity_payload_t capacity;
    } payload;
} __attribute__((packed)) sbus2_telemetry_frame_t;

extern const uint8_t Slot_ID[SBUS2_SLOT_COUNT];
extern sbus2_telemetry_frame_t sbusTelemetryData[SBUS2_TELEMETRY_PAGES][SBUS2_TELEMETRY_SLOTS];
extern uint8_t sbusTelemetryDataStatus[SBUS2_TELEMETRY_PAGES][SBUS2_TELEMETRY_SLOTS];

// refresh telemetry buffers 
void handleSbus2Telemetry(timeUs_t currentTimeUs);

// time critical, send sbus2 data
void taskSendSbus2Telemetry(timeUs_t currentTimeUs);
#endif
