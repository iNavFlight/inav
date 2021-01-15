/*
    Copyright (C) 2016 Stephane D'Alu
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
#include "shell.h"
#include "chprintf.h"
#include <stdlib.h>


/*
 * Command Random
 */
#define RANDOM_BUFFER_SIZE 1024
static uint8_t random_buffer[RANDOM_BUFFER_SIZE];

static void cmd_random(BaseSequentialStream *chp, int argc, char *argv[]) {
    uint16_t size = 16;
    uint16_t i    = 0;
    uint8_t  nl   = 0;
    
    if (argc > 0) {
	size = atoi(argv[0]);
    }

    if (size > RANDOM_BUFFER_SIZE) {
	chprintf(chp, "random: maximum size is %d.\r\n", RANDOM_BUFFER_SIZE);
	return;
    }
    
    chprintf(chp, "Fetching %d random byte(s):\r\n", size);

    rngStart(&RNGD1, NULL);
    rngWrite(&RNGD1, random_buffer, size, TIME_INFINITE);
    rngStop(&RNGD1);

    for (i = 0 ; i < size ; i++) {
	chprintf(chp, "%02x ", random_buffer[i]);
	if ((nl = (((i+1) % 20)) == 0))
	    chprintf(chp, "\r\n");
    }
    if (!nl)
	chprintf(chp, "\r\n");
    
}


/*
 * Shell
 */
static THD_WORKING_AREA(shell_wa, 1024);

static const ShellCommand commands[] = {
  {"random", cmd_random}, 
  {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SD1,
  commands
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
   * Serial device
   */
  SerialConfig serial_config = {
      .speed   = 115200,
      .tx_pad  = UART_TX,
      .rx_pad  = UART_RX,
  };
  sdStart(&SD1, &serial_config);

  
  /*
   * Shell manager initialization.
   */
  shellInit();
  chThdCreateStatic(shell_wa, sizeof(shell_wa), NORMALPRIO,
		      shellThread, (void *)&shell_cfg1);
  

  /*
   * Normal main() thread activity, in this demo it does nothing.
   */
  while (true) {
      chThdSleepMilliseconds(500);
      palTogglePad(IOPORT1, LED1);
  }
}
