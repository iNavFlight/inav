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
#include "common/utils.h"

#include "config/feature.h"

#include "io/serial.h"

#include "fc/config.h"

#include "msp/msp.h"
#include "msp/msp_serial.h"
#include "msp/msp_protocol.h"

#ifdef DEBUG_SECTION_TIMES
timeUs_t sectionTimes[2][4];
#endif

int32_t debug[DEBUG32_VALUE_COUNT];
uint8_t debugMode;

#if defined(USE_DEBUG_TRACE)

#define DEBUG_TRACE_PREFIX                  "[%6d.%03d] "
#define DEBUG_TRACE_PREFIX_FORMATTED_SIZE   13

static serialPort_t * tracePort = NULL;
static mspPort_t * mspTracePort = NULL;

void debugTraceInit(void)
{
    if (!feature(FEATURE_DEBUG_TRACE)) {
        return;
    }

    const serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_DEBUG_TRACE);
    if (!portConfig) {
        return;
    }

    bool tracePortIsSharedWithMSP = false;

    if (determinePortSharing(portConfig, FUNCTION_DEBUG_TRACE) == PORTSHARING_SHARED) {
        // We support sharing a DEBUG_TRACE port only with MSP
        if (portConfig->functionMask != (FUNCTION_DEBUG_TRACE | FUNCTION_MSP)) {
            return;
        }
        tracePortIsSharedWithMSP = true;
    }

    // If the port is shared with MSP, reuse the port
    if (tracePortIsSharedWithMSP) {
        const serialPort_t *traceAndMspPort = findSharedSerialPort(FUNCTION_DEBUG_TRACE, FUNCTION_MSP);
        if (!traceAndMspPort) {
            return;
        }

        mspTracePort = mspSerialPortFind(traceAndMspPort);
        if (!mspTracePort) {
            return;
        }

    } else {
        tracePort = openSerialPort(portConfig->identifier, FUNCTION_DEBUG_TRACE, NULL, NULL, baudRates[BAUD_921600], MODE_TX, SERIAL_NOT_INVERTED);
        if (!tracePort) {
            return;
        }
    }
    // Initialization done
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

static void debugTracePrint(bool synchronous, const char *buf, size_t size)
{
    if (tracePort) {
        // Send data via trace UART (if configured & connected - a safeguard against zombie VCP)
        if (serialIsConnected(tracePort)) {
            serialPrint(tracePort, buf);
            if (synchronous) {
                waitForSerialPortToFinishTransmitting(tracePort);
            }
        }
    } else if (mspTracePort) {
        mspSerialPushPort(MSP_DEBUGMSG, (uint8_t*)buf, size, mspTracePort, MSP_V2_NATIVE);
    }
}

static size_t debugTraceFormatPrefix(char *buf, const timeMs_t timeMs)
{
    // Write timestamp
    return tfp_sprintf(buf, DEBUG_TRACE_PREFIX, timeMs / 1000, timeMs % 1000);
}

void debugTracePrintf(bool synchronous, const char *format, ...)
{
    char buf[128];
    char *bufPtr;
    int charCount;

    STATIC_ASSERT(MSP_PORT_OUTBUF_SIZE >= sizeof(buf), MSP_PORT_OUTBUF_SIZE_not_big_enough_for_debug_trace);

    if (!feature(FEATURE_DEBUG_TRACE))
        return;

    charCount = debugTraceFormatPrefix(buf, millis());
    bufPtr = &buf[charCount];

    // Write message
    va_list va;
    va_start(va, format);
    charCount += tfp_format(&bufPtr, debugTracePutcp, format, va);
    debugTracePutcp(&bufPtr, '\n');
    debugTracePutcp(&bufPtr, 0);
    charCount += 2;
    va_end(va);

    debugTracePrint(synchronous, buf, charCount);
}

void debugTracePrintBufferHex(bool synchronous, const void *buffer, size_t size)
{
    // Print lines of up to maxBytes bytes. We need 5 characters per byte
    // 0xAB[space|\n]
    const size_t charsPerByte = 5;
    const size_t maxBytes = 8;
    char buf[DEBUG_TRACE_PREFIX_FORMATTED_SIZE + charsPerByte * maxBytes + 1]; // +1 for the null terminator
    size_t bufPos = DEBUG_TRACE_PREFIX_FORMATTED_SIZE;
    const uint8_t *inputPtr = buffer;

    debugTraceFormatPrefix(buf, millis());

    for (size_t ii = 0; ii < size; ii++) {
        tfp_sprintf(buf + bufPos, "0x%02x ", inputPtr[ii]);
        bufPos += charsPerByte;
        if (bufPos == sizeof(buf)-1) {
            buf[bufPos-1] = '\n';
            buf[bufPos] = '\0';
            debugTracePrint(synchronous, buf, bufPos + 1);
            bufPos = DEBUG_TRACE_PREFIX_FORMATTED_SIZE;
        }
    }

    if (bufPos > DEBUG_TRACE_PREFIX_FORMATTED_SIZE) {
        buf[bufPos-1] = '\n';
        buf[bufPos] = '\0';
        debugTracePrint(synchronous, buf, bufPos + 1);
    }
}


#endif
