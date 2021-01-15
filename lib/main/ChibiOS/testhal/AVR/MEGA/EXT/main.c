/*
    ChibiOS - Copyright (C) 2016 Theodore Ateba

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

/**
 * @brief Global variables.
 */
BaseSequentialStream* chp = (BaseSequentialStream*) &SD1;
static int isrCpt; /**< Interrupt counter. */

/**
 * @brief  This is the External Interruption 4 callback.
 *         The onboard LED is toggled every time an interruption is
 *         detected. A varible is also incremented.
 */
static void extcb(EXTDriver *extp, expchannel_t channel) {
  (void)extp;
  (void)channel;

  chSysLockFromISR();
  palTogglePad(IOPORT2, PORTB_LED1);
  isrCpt++;
  chSysUnlockFromISR();
}

/**
 * @brief  EXT Driver configurations.
 */
static const EXTConfig extcfg = {
  {
    {EXT_CH_MODE_DISABLED , NULL},      /* INT0 Config. */
    {EXT_CH_MODE_DISABLED , NULL},      /* INT1 Config. */
    {EXT_CH_MODE_DISABLED , NULL},      /* INT2 Config. */
    {EXT_CH_MODE_DISABLED , NULL},      /* INT3 Config. */
    {EXT_CH_MODE_RISING_EDGE , extcb},  /* INT4 Config. */
    {EXT_CH_MODE_DISABLED , NULL},      /* INT5 Config. */
  }
};

/**
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
   * External interrupt on the Pin D2 of the Arduino Mega2560.
   */
  palSetPadMode(IOPORT5, PE4, PAL_MODE_INPUT);  /* INT4 pin. */

  /*
   * Start the serial driver.
   */
  sdStart(&SD1, NULL);

  /*
   * Active the EXT driver 1 and select INT channel 4.
   */
  extStart(&EXTD1, &extcfg);
  extChannelEnable(&EXTD1, INT4);

  palClearPad(IOPORT2, PORTB_LED1);
  chprintf(chp, "\n\r EXT hal test started...");

  /*
   * Normal main() thread activity, in this demo it print IRQ counter every
   * seconde.
   */
  while (TRUE) {
    chThdSleepMilliseconds(1000);
    chprintf(chp, "\n\r External Interrupt counter: %d", isrCpt);
  }
}
