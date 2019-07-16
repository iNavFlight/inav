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

#include <platform.h>
#include <common/base.h>

#include "build/debug.h"
#include "drivers/serial.h"
#include "drivers/serial_softserial.h"

#include "fc/fc_init.h"

#include "scheduler/scheduler.h"

#ifdef SOFTSERIAL_LOOPBACK
serialPort_t *loopbackPort;
#endif

static void loopbackInit(void)
{
#ifdef SOFTSERIAL_LOOPBACK
    loopbackPort = softSerialLoopbackPort();
    serialPrint(loopbackPort, "LOOPBACK\r\n");
#endif
}

static void processLoopback(void)
{
#ifdef SOFTSERIAL_LOOPBACK
    if (loopbackPort) {
        uint8_t bytesWaiting;
        while ((bytesWaiting = serialRxBytesWaiting(loopbackPort))) {
            uint8_t b = serialRead(loopbackPort);
            serialWrite(loopbackPort, b);
        };
    }
#endif
}

/* CRT init function, called by __libc_init_array() before passing control to main */
extern "C" void __attribute__ ((weak)) _init(void)
{
}

/* Prevent demangle from being pulled in */
extern "C" void __cxa_pure_virtual(void)
{
    // FIXME: Crash safely here
    while (true);
}

extern "C" int main(void)
{
    init();
    loopbackInit();

    while (true) {
        scheduler();
        processLoopback();
    }
}
