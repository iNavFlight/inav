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

#include "drivers/time.h"   // timeUs_t for sensors/esc_sensor.h, which has no includes of its own
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

typedef struct {
    uint16_t rawValue;
    uint16_t telemetryData[DSHOT_TELEMETRY_TYPE_COUNT];
    uint8_t telemetryTypes;
    uint8_t maxTemp;
    escFrameCounter_t frames;
} dshotTelemetryMotorState_t;

typedef struct {
    dshotTelemetryMotorState_t motorState[MAX_SUPPORTED_MOTORS];
} dshotTelemetryState_t;

#ifdef USE_DSHOT_BIDIR
extern bool useDshotTelemetry;
extern dshotTelemetryState_t dshotTelemetryState;

void initDshotTelemetry(void);
void dshotResetTelemetry(void);
bool isDshotTelemetryConfigured(void);
bool isDshotTelemetryActive(void);
uint16_t dshotProcessPacket(uint16_t rawValue, uint8_t motorIndex);
void dshotFrameWindowUpdate(timeUs_t currentTimeUs);
float getDshotRpm(uint8_t motorIndex);
uint16_t getDshotErpm(uint8_t motorIndex);
float getDshotRpmAverage(void);
// Mechanical frequency from the last decoded eRPM frame, unfiltered (see bidir_dshot.c)
float getMotorFrequencyHz(uint8_t motorIndex);
bool getDshotEscSensorData(escSensorData_t *data, uint8_t motorIndex);
#endif
