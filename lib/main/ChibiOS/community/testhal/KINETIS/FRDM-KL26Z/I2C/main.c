/*
    (c) 2015-2016 flabbergast <s3+flabbergast@sdfeu.org>
    Based on K20 I2C demo (c) 2015 Fabio Utzig, http://fabioutzig.com

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

#define FXOS8700CQ_ADDR 0x1D

// FXOS8700CQ internal register addresses
#define FXOS8700CQ_STATUS 0x00
#define FXOS8700CQ_WHOAMI 0x0D
#define FXOS8700CQ_XYZ_DATA_CFG 0x0E
#define FXOS8700CQ_CTRL_REG1 0x2A
#define FXOS8700CQ_M_CTRL_REG1 0x5B
#define FXOS8700CQ_M_CTRL_REG2 0x5C
#define FXOS8700CQ_WHOAMI_VAL 0xC7

static bool i2cOk = false;

static const I2CConfig i2ccfg = {
  400000 // clock
};

static THD_WORKING_AREA(waThread1, 64);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("Blinker");
  while(true) {
    if(i2cOk) {
      palSetPad(GPIO_LED_RED, PIN_LED_RED); /* Off red */
      palTogglePad(GPIO_LED_GREEN, PIN_LED_GREEN); /* Blink green */
    } else {
      palSetPad(GPIO_LED_GREEN, PIN_LED_GREEN); /* Off green */
      palTogglePad(GPIO_LED_RED, PIN_LED_RED); /* Blink red */
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

  /*
   * Turn off the RGB LED.
   */
  palSetPad(GPIO_LED_RED, PIN_LED_RED); /* red */
  palSetPad(GPIO_LED_GREEN, PIN_LED_GREEN); /* green */
  palSetPad(GPIO_LED_BLUE, PIN_LED_BLUE); /* blue */

  i2cStart(&I2CD1, &i2ccfg);

  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  while (1) {
    tx[0] = FXOS8700CQ_WHOAMI;
    i2cMasterTransmitTimeout(&I2CD1, FXOS8700CQ_ADDR, tx, 1, rx, 1, TIME_INFINITE);
    i2cOk = (rx[0] == FXOS8700CQ_WHOAMI_VAL) ? true : false;
    chThdSleepMilliseconds(2000);
  }
}
