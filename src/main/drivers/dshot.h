/*
 * This file is part of Betaflight.
 *
 * Betaflight is free software. You can redistribute this software
 * and/or modify this software under the terms of the GNU General
 * Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later
 * version.
 *
 * Betaflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "drivers/time.h"
#include "flight/mixer.h"

#include "sensors/esc_sensor.h"

#define DSHOT_TELEMETRY_NOEDGE          (0xfffe)
#define DSHOT_TELEMETRY_INVALID         (0xffff)

#define MIN_GCR_EDGES                   (7)
#define MAX_GCR_EDGES                   (22)

typedef enum {
    DSHOT_TELEMETRY_TYPE_ERPM = 0,
    DSHOT_TELEMETRY_TYPE_TEMPERATURE,
    DSHOT_TELEMETRY_TYPE_VOLTAGE,
    DSHOT_TELEMETRY_TYPE_CURRENT,
    DSHOT_TELEMETRY_TYPE_DEBUG1,
    DSHOT_TELEMETRY_TYPE_DEBUG2,
    DSHOT_TELEMETRY_TYPE_DEBUG3,
    DSHOT_TELEMETRY_TYPE_STATE_EVENTS,
    DSHOT_TELEMETRY_TYPE_COUNT
} dshotTelemetryType_e;

#define DSHOT_NORMAL_TELEMETRY_MASK     (1 << DSHOT_TELEMETRY_TYPE_ERPM)
#define DSHOT_EXTENDED_TELEMETRY_MASK   (~DSHOT_NORMAL_TELEMETRY_MASK)

typedef enum {
    DSHOT_RAW_VALUE_STATE_INVALID = 0,
    DSHOT_RAW_VALUE_STATE_NOT_PROCESSED,
    DSHOT_RAW_VALUE_STATE_PROCESSED,
} dshotRawValueState_e;

typedef struct {
    uint16_t rawValue;
    uint16_t telemetryData[DSHOT_TELEMETRY_TYPE_COUNT];
    uint8_t telemetryTypes;
    uint8_t maxTemp;
} dshotTelemetryMotorState_t;

typedef struct {
    uint32_t invalidPacketCount;
    uint32_t readCount;
    dshotTelemetryMotorState_t motorState[MAX_SUPPORTED_MOTORS];
    dshotRawValueState_e rawValueState;
} dshotTelemetryState_t;

extern bool useDshotTelemetry;
extern dshotTelemetryState_t dshotTelemetryState;

void initDshotTelemetry(timeUs_t looptimeUs);
void dshotResetTelemetry(void);
bool isDshotTelemetryConfigured(void);
bool isDshotTelemetryActive(void);
uint16_t dshotProcessPacket(uint16_t rawValue, uint8_t motorIndex);
float getDshotRpm(uint8_t motorIndex);
uint16_t getDshotErpm(uint8_t motorIndex);
float getDshotRpmAverage(void);
float getMotorFrequencyHz(uint8_t motorIndex);
bool getDshotEscSensorData(escSensorData_t *data, uint8_t motorIndex);
