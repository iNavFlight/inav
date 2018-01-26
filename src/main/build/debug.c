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
#include "common/printf.h"
#include "drivers/serial.h"
#include "drivers/time.h"
#include "io/serial.h"

#ifdef DEBUG_SECTION_TIMES
timeUs_t sectionTimes[2][4];
#endif

int16_t debug[DEBUG16_VALUE_COUNT];
uint8_t debugMode;

#if defined(USE_DEBUG_TRACE)
static serialPort_t * tracePort = NULL;

void debugTraceInit(void)
{
    const serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_DEBUG_TRACE);
    if (!portConfig) {
        return false;
    }

    tracePort = openSerialPort(portConfig->identifier, FUNCTION_DEBUG_TRACE, NULL, NULL, baudRates[BAUD_921600], MODE_TX, SERIAL_NOT_INVERTED);
    if (!tracePort)
        return;

    DEBUG_TRACE_SYNC("Debug trace facilities initialized");
}

static void debugTracePutp(void *p, char ch)
{
    (void)p;
    serialWrite(tracePort, ch);
}

void debugTracePrintf(bool synchronous, const char *format, ...)
{
    char timestamp[16];

    if (!tracePort)
        return;

    // Write timestamp
    tfp_sprintf(timestamp, "[%10d] ", micros());
    serialPrint(tracePort, timestamp);

    // Write message
    va_list va;
    va_start(va, format);
    tfp_format(NULL, debugTracePutp, format, va);
    if (synchronous) {
        waitForSerialPortToFinishTransmitting(tracePort);
    }
    va_end(va);

    // Write newline
    serialPrint(tracePort, "\r\n");
}
#endif
