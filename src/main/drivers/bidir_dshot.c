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

#include <math.h>

#include "platform.h"
#include "drivers/bidir_dshot.h"

// Bidirectional DSHOT telemetry only: eRPM / EDT decoding, RPM state for the RPM filter and
// the ESC sensor. Frame generation, DMA and DSHOT commands for plain DSHOT live in
// pwm_output.c under USE_DSHOT. Every user of this API is guarded by USE_DSHOT_BIDIR.
//
// Values are published as decoded, unfiltered: frames arrive at the DSHOT frame rate, which
// follows the scheduler's idle time rather than any fixed period, so smoothing belongs to the
// consumer that samples them at a known rate (rpmFilterUpdateTask()).

#ifdef USE_DSHOT_BIDIR

#include "flight/mixer.h"

// Only eRPM, temperature, voltage and current are kept; the EDT debug and status frames
// have no consumer and just mark their type as seen
#define DSHOT_TELEMETRY_STORED_TYPE_COUNT (DSHOT_TELEMETRY_TYPE_CURRENT + 1)

typedef struct {
    uint16_t telemetryData[DSHOT_TELEMETRY_STORED_TYPE_COUNT];
    uint8_t telemetryTypes;     // bit per dshotTelemetryType_e seen since boot
} dshotTelemetryMotorState_t;

bool useDshotTelemetry = false;

static dshotTelemetryMotorState_t motorTelemetry[MAX_SUPPORTED_MOTORS];
static float erpmToHz;

static uint32_t dshotDecodeErpmTelemetryValue(uint16_t value)
{
    if (value == 0x0fff) {
        return 0;
    }

    value = (value & 0x01ff) << ((value & 0xfe00) >> 9);
    if (!value) {
        return DSHOT_TELEMETRY_INVALID;
    }

    return (1000000 * 60 / 100 + value / 2) / value;
}

static void dshotDecodeTelemetryValue(uint16_t value, uint8_t motorIndex, uint32_t *decoded, dshotTelemetryType_e *type)
{
    const bool edtEnabled = motorConfig()->useDshotEdt || (motorTelemetry[motorIndex].telemetryTypes & DSHOT_EXTENDED_TELEMETRY_MASK) != 0;
    const unsigned telemetryType = (value & 0x0f00) >> 8;
    const bool isErpm = !edtEnabled || (telemetryType & 0x01) || (telemetryType == 0);

    if (isErpm) {
        *decoded = dshotDecodeErpmTelemetryValue(value);
        *type = DSHOT_TELEMETRY_TYPE_ERPM;
    } else {
        // EDT frame types 2, 4, ... 14 are the enum values 1..7 in order
        *type = telemetryType >> 1;
        *decoded = value & 0x00ff;
    }
}

bool isDshotTelemetryActive(void)
{
    return useDshotTelemetry;
}

// useDshotTelemetry itself is set by the motor driver (pwmMotorPreconfigure()), which
// knows whether the outputs were configured for DSHOT at all
void initDshotTelemetry(void)
{
    if (!useDshotTelemetry) {
        return;
    }

    erpmToHz = ERPM_PER_LSB / 60.0f / (motorConfig()->motorPoleCount / 2.0f);
}

uint16_t dshotProcessPacket(uint16_t rawValue, uint8_t motorIndex)
{
    if (!useDshotTelemetry || motorIndex >= MAX_SUPPORTED_MOTORS) {
        return rawValue;
    }

    // As in Betaflight, a window without a reply is not counted: an ESC may skip replies
    // when busy, so only replies that were at least partly captured count
    if (rawValue == DSHOT_TELEMETRY_NOEDGE) {
        return rawValue;
    }

    escFrameCounter_t *frames = escSensorFrameCounter(motorIndex);
    frames->total++;

    if (rawValue == DSHOT_TELEMETRY_INVALID) {
        return rawValue;
    }

    dshotTelemetryType_e type;
    uint32_t decoded;
    dshotDecodeTelemetryValue(rawValue, motorIndex, &decoded, &type);
    if (decoded == DSHOT_TELEMETRY_INVALID) {
        return DSHOT_TELEMETRY_INVALID;
    }

    frames->valid++;

    motorTelemetry[motorIndex].telemetryTypes |= (1 << type);
    if (type < DSHOT_TELEMETRY_STORED_TYPE_COUNT) {
        motorTelemetry[motorIndex].telemetryData[type] = decoded;
    }

    return rawValue;
}

float getMotorFrequencyHz(uint8_t motorIndex)
{
    return motorTelemetry[motorIndex].telemetryData[DSHOT_TELEMETRY_TYPE_ERPM] * erpmToHz;
}

float getDshotRpm(uint8_t motorIndex)
{
    return getMotorFrequencyHz(motorIndex) * 60.0f;
}

bool getDshotEscSensorData(escSensorData_t *data, uint8_t motorIndex)
{
    if (!useDshotTelemetry || motorIndex >= MAX_SUPPORTED_MOTORS) {
        return false;
    }

    const dshotTelemetryMotorState_t *state = &motorTelemetry[motorIndex];
    if ((state->telemetryTypes & (1 << DSHOT_TELEMETRY_TYPE_ERPM)) == 0) {
        return false;
    }

    // escSensorData_t units: rpm (mechanical), degrees C, 0.01 V, 0.01 A. EDT reports
    // the temperature in degrees C, the voltage in 0.25 V and the current in 1 A steps.
    data->rpm = lrintf(getDshotRpm(motorIndex));
    data->temperature = (state->telemetryTypes & (1 << DSHOT_TELEMETRY_TYPE_TEMPERATURE)) ? state->telemetryData[DSHOT_TELEMETRY_TYPE_TEMPERATURE] : 0;
    data->voltage = (state->telemetryTypes & (1 << DSHOT_TELEMETRY_TYPE_VOLTAGE)) ? state->telemetryData[DSHOT_TELEMETRY_TYPE_VOLTAGE] * 25 : 0;
    data->current = (state->telemetryTypes & (1 << DSHOT_TELEMETRY_TYPE_CURRENT)) ? state->telemetryData[DSHOT_TELEMETRY_TYPE_CURRENT] * 100 : 0;

    return true;
}

#endif
