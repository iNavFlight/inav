/*
    Copyright (C) 2018 andru

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
#include "chprintf.h"

static icucnt_t last_width, last_period;

static SerialConfig serial_config = {
    .speed   = 38400,
    .tx_pad  = UART_TX,
    .rx_pad  = UART_RX,
#if NRF5_SERIAL_USE_HWFLOWCTRL  == TRUE
    .rts_pad = UART_RTS,
    .cts_pad = UART_CTS,
#endif
};

static void pwm_cb_period(PWMDriver *pwmp) {
  (void)pwmp;
  palTogglePad(IOPORT1, LED1);
}

void icu_width_cb(ICUDriver *icup) {
  last_width = icuGetWidthX(icup);
}

void icu_period_cb(ICUDriver *icup) {
  last_period = icuGetPeriodX(icup);
}

ICUConfig icucfg = {
  .frequency = ICU_FREQUENCY_250KHZ,
  .width_cb = icu_width_cb,
  .period_cb = icu_period_cb,
  NULL,
  .iccfgp = {
	{
      .ioline = { BTN1, BTN2 },
	  .mode = ICU_INPUT_ACTIVE_HIGH,
	  .gpiote_channel = { 0, 1 },
	  .ppi_channel = { 0, 1 },
    },
  },
};

PWMConfig pwmcfg = {
  .frequency = PWM_FREQUENCY_125KHZ,
  .period = 12500,
  .callback = pwm_cb_period,
  {
	{ .mode = PWM_OUTPUT_DISABLED,
    },
    { .mode = PWM_OUTPUT_ACTIVE_HIGH,
  	.ioline = LINE_LED2,
    },
  },
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

  sdStart(&SD1, &serial_config);

  /*
   *
   */
  pwmStart(&PWMD1, &pwmcfg);
  pwmEnablePeriodicNotification(&PWMD1);

  icuStart(&ICUD1, &icucfg);
  icuStartCapture(&ICUD1);

  pwmEnableChannel(&PWMD1, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 2500));	// 25%
  chThdSleepMilliseconds(5000);
  chprintf((BaseSequentialStream *) &SD1, "period=%d, width=%d\r\n", last_period, last_width);

  pwmEnableChannel(&PWMD1, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 5000));	// 50%
  chThdSleepMilliseconds(5000);
  chprintf((BaseSequentialStream *) &SD1, "period=%d, width=%d\r\n", last_period, last_width);

  pwmEnableChannel(&PWMD1, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 7500));	// 75%
  chThdSleepMilliseconds(5000);
  chprintf((BaseSequentialStream *) &SD1, "period=%d, width=%d\r\n", last_period, last_width);

  pwmChangePeriod(&PWMD1, 5000);
  chThdSleepMilliseconds(5000);

  pwmDisableChannel(&PWMD1, 1);
  pwmStop(&PWMD1);
  icuStopCapture(&ICUD1);
  icuStop(&ICUD1);

  while (true) {
    chThdSleepMilliseconds(500);
  }
}
