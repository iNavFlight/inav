/*
    ChibiOS - Copyright (C) 2015 RedoX https://github.com/RedoXyde

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "hal.h"

static THD_WORKING_AREA(waSerEcho, 128);
static THD_FUNCTION(thSerEcho, arg)
{
  (void)arg;
  chRegSetThreadName("SerEcho");
  event_listener_t elSerData;
  eventflags_t flags;
  chEvtRegisterMask((event_source_t *)chnGetEventSource(&SD1), &elSerData, EVENT_MASK(1));

  while (!chThdShouldTerminateX())
  {
     chEvtWaitOneTimeout(EVENT_MASK(1), TIME_MS2I(10));
     flags = chEvtGetAndClearFlags(&elSerData);
     if (flags & CHN_INPUT_AVAILABLE)
     {
        msg_t charbuf;
        do
        {
           charbuf = chnGetTimeout(&SD1, TIME_IMMEDIATE);
           if ( charbuf != Q_TIMEOUT )
           {
             streamPut(&SD1, charbuf);
           }
        }
        while (charbuf != Q_TIMEOUT);
     }
  }
}

SerialConfig s0cfg =
{
  19200
};

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Activates serial 1 (UART0) using the driver default configuration.
   */
  sdStart(&SD1, &s0cfg);

  chThdCreateStatic(waSerEcho, sizeof(waSerEcho), NORMALPRIO, thSerEcho, NULL);

  while (!chThdShouldTerminateX()) {
    chThdSleepMilliseconds(1000);
    palTogglePad(TEENSY_PIN13_IOPORT, TEENSY_PIN13);
    sdPut(&SD1,'B');
  }

  return 0;
}
