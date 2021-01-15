/*
    Copyright (C) 2016 Stéphane D'Alu

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

static void pwm_cb_period(PWMDriver *pwmp) {
  (void)pwmp;

  palTogglePad(IOPORT1, LED0);
  palClearPad(IOPORT1, LED1);

}

static void pwm_cb_channel0(PWMDriver *pwmp) {
  (void)pwmp;
  palSetPad(IOPORT1, LED1);
}


/*
 * Application entry point.
 */
int main(void) {
  PWMConfig pwmcfg = {
    .frequency = PWM_FREQUENCY_31250HZ, 
    .period = 31250,                 
    .callback = pwm_cb_period,
    { { .mode           = PWM_OUTPUT_DISABLED,
	.callback       = pwm_cb_channel0, },
      { .mode           = PWM_OUTPUT_ACTIVE_HIGH,
	.callback       = NULL,
	.ioline         = LINE_LED2,
	.gpiote_channel = 0,
	.ppi_channel    = { 0, 1 } },
    },
  };

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
   *
   */
  pwmStart(&PWMD1, &pwmcfg);
  pwmEnablePeriodicNotification(&PWMD1);
  pwmEnableChannel(&PWMD1, 0, PWM_FRACTION_TO_WIDTH(&PWMD1, 2, 1));
  pwmEnableChannelNotification(&PWMD1, 0);
  pwmEnableChannel(&PWMD1, 1, PWM_FRACTION_TO_WIDTH(&PWMD1, 4, 3));

  while (1) {
    chThdSleepMilliseconds(500);
  }
}
