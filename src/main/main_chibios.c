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

#include "ch.h"
#include "hal.h"
#include "nvic.h"

volatile bool idleCounterClear = 0;
volatile uint32_t idleCounter = 0;

void appIdleHook(void)
{
    // Called when the scheduler has no tasks to run
    if (idleCounterClear) {
        idleCounter = 0;
        idleCounterClear = 0;
    } else {
        ++idleCounter;
    }
}


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

static THD_WORKING_AREA(waInavThread, 1 * 1024);
static THD_FUNCTION(InavThread, arg)
{
    (void)arg;
    chRegSetThreadName("INAV");
    init();
    loopbackInit();

    while (true) {
        scheduler();
        processLoopback();
    }
}

#if defined(USE_BRAINFPV_OSD)
#include "brainfpv/brainfpv_osd.h"

static THD_WORKING_AREA(waOSDThread, 1 * 1024);
static THD_FUNCTION(OSDThread, arg)
{
    (void)arg;
    chRegSetThreadName("OSD");
    brainFpvOsdInit();
    brainFpvOsdMain();
}
#endif

int main(void)
{
    //halInit();
    /* Initializes the OS Abstraction Layer.*/
    osalInit();

    /* Platform low level initializations.*/
    hal_lld_init();

    chSysInit();

    st_lld_init();

    persistentObjectInit();

    chThdCreateStatic(waInavThread, sizeof(waInavThread), HIGHPRIO, InavThread, NULL);

#if defined(USE_BRAINFPV_OSD)
    chThdCreateStatic(waOSDThread, sizeof(waOSDThread), NORMALPRIO, OSDThread, NULL);
#endif /* USE_BRAINFPV_OSD */

    // sleep forever
    chThdSleep(TIME_INFINITE);
}

