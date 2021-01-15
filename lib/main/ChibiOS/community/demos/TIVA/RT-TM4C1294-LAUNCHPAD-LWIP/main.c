/*
    Copyright (C) 2014..2017 Marco Veeneman

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

#include "ch.h"
#include "hal.h"
#include "lwipthread.h"
#include "lwip/apps/httpd.h"

/*
 * Application entry point.
 */
int main(void)
{
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   * - lwIP subsystem initialization using the default configuration.
   */
  halInit();
  chSysInit();
  lwipInit(NULL);

  /*
   * Start the serial driver with the default configuration.
   * Used for debug output of LwIP.
   */
  sdStart(&SD1, NULL);

  /*
   * Creates the LwIP HTTP server.
   */
  httpd_init();

  while (1) {
    osalThreadSleepMilliseconds(500);
  }

  return 0;
}
