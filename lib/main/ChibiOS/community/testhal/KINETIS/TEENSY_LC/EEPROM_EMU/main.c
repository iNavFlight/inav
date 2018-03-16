/*
    (C) 2015-2016 flabbergast <s3+flabbergast@sdfeu.org>

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

#include "eeprom.h"

#define LED_GPIO TEENSY_PIN13_IOPORT
#define LED_PIN TEENSY_PIN13

uint8_t n_of_blinks;

static THD_WORKING_AREA(waBlinkThread, 128);
static THD_FUNCTION(BlinkThread, arg) {
  (void)arg;
  uint8_t i;

  for(i=0; i<n_of_blinks; i++) {
    palSetPad(LED_GPIO, LED_PIN);
    chThdSleepMilliseconds(300);
    palClearPad(LED_GPIO, LED_PIN);
    chThdSleepMilliseconds(300);
  }    
}

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

  n_of_blinks = 0;
  
  uint8_t *ee_addr = (uint8_t *)3;

  eeprom_write_byte(ee_addr,5);
  chThdSleepMilliseconds(500);

  eeprom_write_byte(ee_addr,10);
  chThdSleepMilliseconds(500);

  n_of_blinks = eeprom_read_byte(ee_addr);

  /*
   * Create the blink thread.
   */
  chThdCreateStatic(waBlinkThread, sizeof(waBlinkThread), NORMALPRIO, BlinkThread, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing.
   */
  while(TRUE) {
    chThdSleepMilliseconds(500);
  }

  return 0;
}
