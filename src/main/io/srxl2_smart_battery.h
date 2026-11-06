/*
 * This file is part of INAV.
 *
 * INAV is free software. You can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#define SRXL2_TELEM_SENSOR_SMART_BATTERY 0x42
#define SRXL2_SMART_BATTERY_SLOTS        2
#define SRXL2_SMART_BATTERY_MAX_CELLS    18
#define SRXL2_SMART_BATTERY_PAGE_COUNT   7
// Local freshness policy, not a protocol timing requirement. Pages rotate.
#define SRXL2_SMART_BATTERY_STALE_MS     10000

typedef struct {
    uint8_t payload[16];
    uint32_t receivedMs;
    uint32_t count;
    bool valid;
} srxl2SmartBatteryPage_t;

typedef struct {
    // Identity is opaque: never merge secondary IDs or the low type nibble.
    uint8_t secondaryId;
    uint8_t batteryId;
    uint8_t observedCellCount;
    bool used;
    // Types 0,1,2,3,8,9, and the latest other type, respectively.
    srxl2SmartBatteryPage_t pages[SRXL2_SMART_BATTERY_PAGE_COUNT];
} srxl2SmartBatterySource_t;

typedef struct {
    srxl2SmartBatterySource_t source[SRXL2_SMART_BATTERY_SLOTS];
    uint32_t droppedSources;
} srxl2SmartBatteryState_t;

typedef struct {
    uint16_t cellMv[SRXL2_SMART_BATTERY_MAX_CELLS];
    uint32_t validCells;
    int16_t temperatureDeciC;
    bool temperatureValid;
    uint8_t cellCount;
    // True only when a recent identification page supplies a supported count.
    bool cellCountValid;
    // A fresh limits page has a nonzero, non-FFFF capacity word. This
    // provisional availability check does not prove a genuine pack identity.
    bool metadataAvailable;
    uint8_t secondaryId;
    uint8_t batteryId;
} srxl2SmartBatteryTelemetry_t;

// Caller validates the enclosing SRXL2 CRC, destination and payload length.
void srxl2SmartBatteryReceive(srxl2SmartBatteryState_t *state, const uint8_t payload[16], uint32_t nowMs);
void srxl2SmartBatteryInvalidate(srxl2SmartBatteryState_t *state);
bool srxl2SmartBatteryGet(const srxl2SmartBatteryState_t *state, uint8_t slot, uint32_t nowMs, srxl2SmartBatteryTelemetry_t *out);
