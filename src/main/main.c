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

#include "platform.h"
#include "build/debug.h"
#include "drivers/serial.h"
#include "drivers/serial_softserial.h"

#include "fc/fc_init.h"

#include "scheduler/scheduler.h"

#if defined(SITL_BUILD)
#include "target/SITL/serial_proxy.h"
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include "target/SITL/wasm_msp_bridge.h"
#endif


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

#ifdef __EMSCRIPTEN__
// WASM: Main loop iteration function (called by browser event loop)
static void mainLoopIteration(void)
{
    wasmMspProcess();  // Process WASM MSP serial port
    scheduler();
    processLoopback();
}
#endif

#if defined(SITL_BUILD)
int main(int argc, char *argv[])
{
    parseArguments(argc, argv);
#else
int main(void)
{
#endif
    init();
    loopbackInit();

#ifdef __EMSCRIPTEN__
    // WASM: Use Emscripten's cooperative main loop
    // This yields control back to browser after each iteration
    // 0 = run as fast as possible (browser will use requestAnimationFrame)
    // 1 = simulate infinite loop (never return from main)
    emscripten_set_main_loop(mainLoopIteration, 0, 1);
#else
    // Native: Traditional infinite loop
    while (true) {
#if defined(SITL_BUILD)
        serialProxyProcess();
#endif
        scheduler();
        processLoopback();
    }
#endif
}
