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
#include <ctype.h>
#include <math.h>

#include "platform.h"

#include "build/build_config.h"
#include "build/debug.h"

#include "common/maths.h"
#include "common/crc.h"

#include "flight/mixer.h"
#include "drivers/pwm_output.h"
#include "sensors/esc_sensor.h"
#include "io/serial.h"

#if defined(USE_ESC_SENSOR)

#define ESC_BOOTTIME_MS         5000
#define ESC_REQUEST_TIMEOUT_MS  50
#define ESC_SENSOR_BAUDRATE     115200
#define TELEMETRY_FRAME_SIZE    10

typedef enum {
    ESC_SENSOR_WAIT_STARTUP = 0,
    ESC_SENSOR_READY = 1,
    ESC_SENSOR_WAITING = 2
} escSensorState_t;

typedef enum {
    ESC_SENSOR_FRAME_PENDING,
    ESC_SENSOR_FRAME_COMPLETE,
    ESC_SENSOR_FRAME_FAILED
} escSensorFrameStatus_t;

static escSensorState_t escSensorState = ESC_SENSOR_WAIT_STARTUP;
static timeMs_t         escTriggerTimeMs;
static int              escSensorMotor;
static uint8_t          telemetryBuffer[TELEMETRY_FRAME_SIZE];
static int              bufferPosition = 0;
static serialPort_t *   escSensorPort = NULL;

static void escSensorSelectNextMotor(void)
{
    escSensorMotor = (escSensorMotor + 1) % getMotorCount();
}


static bool escSensorDecodeFrame(void)
{
    // Receive bytes
    while (serialRxBytesWaiting(escSensorPort) > 0) {
        uint8_t c = serialRead(escSensorPort);

        if (bufferPosition < TELEMETRY_FRAME_SIZE) {
            telemetryBuffer[bufferPosition++] = c;
        }
    }

    // Decode frame
    if (bufferPosition >= TELEMETRY_FRAME_SIZE) {
        uint8_t checksum = crc8_update(0, telemetryBuffer, TELEMETRY_FRAME_SIZE - 1);
        if (checksum == telemetryBuffer[TELEMETRY_FRAME_SIZE - 1]) {
            return ESC_SENSOR_FRAME_COMPLETE;
        }
        else {
            // CRC error
            return ESC_SENSOR_FRAME_FAILED;
        }
    }

    return ESC_SENSOR_FRAME_PENDING;
}

bool escSensorInitialize(void)
{
    // FUNCTION_ESCSERIAL is shared between SERIALSHOT and ESC_SENSOR telemetry
    // They are mutually exclusive
    serialPortConfig_t * portConfig = findSerialPortConfig(FUNCTION_ESCSERIAL);
    if (!portConfig) {
        return false;
    }

    escSensorPort = openSerialPort(portConfig->identifier, FUNCTION_ESCSERIAL, NULL, NULL, ESC_SENSOR_BAUDRATE, MODE_RX, SERIAL_NOT_INVERTED | SERIAL_UNIDIR);
    if (!escSensorPort) {
        return false;
    }

    return true;
}

void escSensorUpdate(timeUs_t currentTimeUs)
{
    if (!escSensorPort) {
        return;
    }

    const timeMs_t currentTimeMs = currentTimeUs / 1000;

    switch (escSensorState) {
        case ESC_SENSOR_WAIT_STARTUP:
            if (currentTimeMs > ESC_BOOTTIME_MS) {
                escSensorMotor = 0;
                escSensorState = ESC_SENSOR_READY;
            }
            break;

        case ESC_SENSOR_READY:
            pwmRequestMotorTelemetry(escSensorMotor);
            bufferPosition = 0;
            escTriggerTimeMs = currentTimeMs;
            escSensorState = ESC_SENSOR_WAITING;
            break;

        case ESC_SENSOR_WAITING:
            if ((currentTimeMs - escTriggerTimeMs) >= ESC_REQUEST_TIMEOUT_MS) {
                // Timed out. Select next motor and move on
                escSensorSelectNextMotor();
                escSensorState = ESC_SENSOR_READY;
            }
            else {
                // Receive serial data and decode frame
                escSensorFrameStatus_t status = escSensorDecodeFrame();

                switch (status) {
                    case ESC_SENSOR_FRAME_COMPLETE:
                        escSensorSelectNextMotor();
                        escSensorState = ESC_SENSOR_READY;
                        break;

                    case ESC_SENSOR_FRAME_FAILED:
                        escSensorSelectNextMotor();
                        escSensorState = ESC_SENSOR_READY;
                        break;

                    case ESC_SENSOR_FRAME_PENDING:
                    default:
                        break;
                }
            }
            break;

    }

}

#endif
