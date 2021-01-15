/*
    ChibiOS/RT - Copyright (C) 2014 Uladzimir Pylinsky aka barthess

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

/*
 * Make following connections on your discovery board:
 * PD13-PB4
 * PD12-PB5
 * PD14-PB0
 * PD15-PB1
 */

#include "ch.h"
#include "hal.h"
#include "stdlib.h"

typedef struct {
  systime_t high;
  systime_t low;
  uint32_t pad;
} pulse_t;

/*
 * Chose values suitable for measurement using 16-bit timer on 1MHz
 */
static pulse_t pulse_led3 = {TIME_MS2I(2), TIME_MS2I(59), GPIOD_LED3};
static pulse_t pulse_led4 = {TIME_MS2I(3), TIME_MS2I(53), GPIOD_LED4};
static pulse_t pulse_led5 = {TIME_MS2I(5), TIME_MS2I(47), GPIOD_LED5};
static pulse_t pulse_led6 = {TIME_MS2I(7), TIME_MS2I(43), GPIOD_LED6};

/*
 *
 */
static THD_WORKING_AREA(PulseThreadWA_LED3, 128);
static THD_WORKING_AREA(PulseThreadWA_LED4, 128);
static THD_WORKING_AREA(PulseThreadWA_LED5, 128);
static THD_WORKING_AREA(PulseThreadWA_LED6, 128);

static THD_FUNCTION(PulseThread, arg) {
  chRegSetThreadName("Pulse");
  pulse_t *pulse = arg;

  systime_t t = chVTGetSystemTimeX();

  while (!chThdShouldTerminateX()) {
    t += pulse->high;
    palSetPad(GPIOD, pulse->pad);
    chThdSleepUntil(t);
    palClearPad(GPIOD, pulse->pad);
    t+= pulse->low;
    chThdSleepUntil(t);
  }

  chThdExit(MSG_OK);
}

static const int32_t tolerance = 20; // uS
void eicu_cb(EICUDriver *eicup, eicuchannel_t channel, uint32_t w, uint32_t p) {
  (void)eicup;
  (void)p;
  switch (channel) {
  case EICU_CHANNEL_1:
    if (abs((int32_t)w - (int32_t)TIME_I2US(pulse_led3.high)) > tolerance)
      osalSysHalt("ch1");
    break;
  case EICU_CHANNEL_2:
    if (abs((int32_t)w - (int32_t)TIME_I2US(pulse_led4.high)) > tolerance)
      osalSysHalt("ch2");
    break;
  case EICU_CHANNEL_3:
    if (abs((int32_t)w - (int32_t)TIME_I2US(pulse_led5.high)) > tolerance)
      osalSysHalt("ch3");
    break;
  case EICU_CHANNEL_4:
    if (abs((int32_t)w - (int32_t)TIME_I2US(pulse_led6.high)) > tolerance)
      osalSysHalt("ch4");
    break;
  default:
    osalSysHalt("unhandled case");
    break;
  }
}

static const EICUChannelConfig led3cfg = {
    EICU_INPUT_ACTIVE_HIGH,
    EICU_INPUT_PULSE,
    eicu_cb
};

static const EICUConfig eicucfg = {
    1000000,    /* EICU clock frequency in Hz.*/
    {
        &led3cfg,
        &led3cfg,
        &led3cfg,
        &led3cfg
    },
    0
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

  palSetPadMode(GPIOB, GPIOB_PIN0, PAL_MODE_ALTERNATE(2));
  palSetPadMode(GPIOB, GPIOB_PIN1, PAL_MODE_ALTERNATE(2));
  palSetPadMode(GPIOB, GPIOB_PIN4, PAL_MODE_ALTERNATE(2));
  palSetPadMode(GPIOB, GPIOB_PIN5, PAL_MODE_ALTERNATE(2));

  eicuStart(&EICUD3, &eicucfg);
  eicuEnable(&EICUD3);

  osalThreadSleepMicroseconds(10); // need to stabilize input puns

  chThdCreateStatic(PulseThreadWA_LED3, sizeof(PulseThreadWA_LED3),
                    NORMALPRIO+1, PulseThread, &pulse_led3);
  chThdCreateStatic(PulseThreadWA_LED4, sizeof(PulseThreadWA_LED4),
                    NORMALPRIO+1, PulseThread, &pulse_led4);
  chThdCreateStatic(PulseThreadWA_LED5, sizeof(PulseThreadWA_LED5),
                    NORMALPRIO+1, PulseThread, &pulse_led5);
  chThdCreateStatic(PulseThreadWA_LED6, sizeof(PulseThreadWA_LED6),
                    NORMALPRIO+1, PulseThread, &pulse_led6);

  while (true) {
    osalThreadSleepMilliseconds(500);
  }

  return 0;
}
