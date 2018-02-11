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

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>

#include "build/debug.h"
#include "build/version.h"

#include "drivers/serial.h"
#include "drivers/time.h"

#include "common/printf.h"

#include "config/feature.h"

#include "io/serial.h"

#include "fc/config.h"

#ifdef DEBUG_SECTION_TIMES
timeUs_t sectionTimes[2][4];
#endif

int16_t debug[DEBUG16_VALUE_COUNT];
uint8_t debugMode;

#if defined(USE_DEBUG_TRACE)
static serialPort_t * tracePort = NULL;

void debugTraceInit(void)
{
    if (!feature(FEATURE_DEBUG_TRACE)) {
        return;
    }

    const serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_DEBUG_TRACE);
    if (!portConfig) {
        return;
    }

    tracePort = openSerialPort(portConfig->identifier, FUNCTION_DEBUG_TRACE, NULL, NULL, baudRates[BAUD_921600], MODE_TX, SERIAL_NOT_INVERTED);

    DEBUG_TRACE_SYNC("%s/%s %s %s / %s (%s)",
        FC_FIRMWARE_NAME,
        targetName,
        FC_VERSION_STRING,
        buildDate,
        buildTime,
        shortGitRevision
    );
}

static void debugTracePutcp(void *p, char ch)
{
    *(*((char **) p))++ = ch;
}

void debugTracePrintf(bool synchronous, const char *format, ...)
{
    char buf[128];
    char *bufPtr;
    int charCount;

    if (!feature(FEATURE_DEBUG_TRACE))
        return;

    // Write timestamp
    const timeMs_t timeMs = millis();
    charCount = tfp_sprintf(buf, "[%6d.%03d] ", timeMs / 1000, timeMs % 1000);
    bufPtr = &buf[charCount];

    // Write message
    va_list va;
    va_start(va, format);
    charCount += tfp_format(&bufPtr, debugTracePutcp, format, va);
    debugTracePutcp(&bufPtr, '\n');
    debugTracePutcp(&bufPtr, 0);
    charCount += 2;
    va_end(va);

    if (tracePort) {
        // Send data via trace UART (if configured)
        serialPrint(tracePort, buf);
        if (synchronous) {
            waitForSerialPortToFinishTransmitting(tracePort);
        }
    }
    else {
        // Send data via MSPV2_TRACE message
        // TODO
    }
}
#endif
