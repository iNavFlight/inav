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

#pragma once

typedef struct {
    uint8_t dataAge;
    int16_t temperature;
    int16_t voltage;
    int32_t current;
    uint32_t rpm;
} escSensorData_t;

#define ESC_FRAME_WINDOW_MS 1000

// Telemetry link quality of one ESC over fixed back-to-back windows, shared by the serial
// ESC sensor and bidirectional DSHOT (only one source is ever active). total counts every
// reply expected (serial: a missing one too; bidir DSHOT: only replies with edges, as in
// Betaflight), valid those that decoded. The last closed window is kept, so the reported
// value moves once per window; lastWindowTotal == 0 means nothing was expected (boot, a
// source without frame counting such as SRXL2) and lastWindowSuccess is meaningless then
typedef struct {
    uint16_t total;
    uint16_t valid;
    uint16_t lastWindowTotal;
    uint8_t  lastWindowSuccess;     // percent
} escFrameCounter_t;

typedef struct escSensorConfig_s {
    uint16_t currentOffset;             // offset consumed by the flight controller / VTX / cam / ... in mA
    uint8_t  listenOnly;
} escSensorConfig_t;

PG_DECLARE(escSensorConfig_t, escSensorConfig);

#define ESC_DATA_MAX_AGE    10
#define ESC_DATA_INVALID    255
#define ERPM_PER_LSB        100.0f

bool escSensorInitialize(void);
void escSensorUpdate(timeUs_t currentTimeUs);
escSensorData_t * escSensorGetData(void);
escSensorData_t * getEscTelemetry(uint8_t esc);
escFrameCounter_t * escSensorFrameCounter(uint8_t esc);
// Closes the counting window of every ESC once ESC_FRAME_WINDOW_MS have passed; the active
// source calls it from wherever it runs regularly
void escSensorFrameWindowUpdate(timeMs_t currentTimeMs);
uint32_t computeRpm(int16_t erpm);
void escSensorInitData(void);
void escSensorSetDshotData(uint8_t esc, uint32_t rpm, int16_t temperature, int16_t voltage, int32_t current);
int getTelemetryMotorCount(void);
