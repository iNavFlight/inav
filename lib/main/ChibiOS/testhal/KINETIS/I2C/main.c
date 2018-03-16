/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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

#define MMA8451_ADDR 0x1D
#define WHO_AM_I     0x0D

static bool i2cOk = false;

static THD_WORKING_AREA(waThread1, 64);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("Blinker");
  while (TRUE) {
    if (i2cOk) {
      palSetPad(IOPORT3, 3);
      palTogglePad(IOPORT4, 4);
    } else {
      palSetPad(IOPORT4, 4);
      palTogglePad(IOPORT3, 3);
    }
    chThdSleepMilliseconds(500);
  }
}

/*
 * Application entry point.
 */
int main(void) {

  uint8_t tx[1], rx[1];

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  palSetPad(IOPORT3, 3);
  palSetPad(IOPORT4, 4);
  palSetPad(IOPORT1, 2);

  i2cStart(&I2CD1, NULL);

  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  while (1) {
    tx[0] = WHO_AM_I;
    i2cMasterTransmitTimeout(&I2CD1, MMA8451_ADDR, tx, 1, rx, 1, TIME_INFINITE);
    i2cOk = (rx[0] == 0x1A) ? true : false;
    chThdSleepMilliseconds(2000);
  }
}
