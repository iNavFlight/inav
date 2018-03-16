/*
    Copyright (C) 2015 Stephen Caudle

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

#ifndef DEBOUNCE_TIME
#define DEBOUNCE_TIME 100
#endif

static virtual_timer_t vt1;
static virtual_timer_t vt2;
static bool debouncing1;
static bool debouncing2;

/* LED1 toggled after debounce.*/
static void led1toggle(void *arg) {

  (void)arg;
  palTogglePad(IOPORT1, LED1);
  debouncing1 = false;
}

/* LED2 toggled after debounce.*/
static void led2toggle(void *arg) {

  (void)arg;
  palTogglePad(IOPORT1, LED2);
  debouncing2 = false;
}

static void extcb1(EXTDriver *extp, expchannel_t channel)
{
  (void)extp;
  (void)channel;

  uint8_t pad1 = palReadPad(IOPORT1, KEY1);

  if (!debouncing1 && (pad1 == PAL_LOW)) {
    debouncing1 = true;
    chSysLockFromISR();
    chVTSetI(&vt1, MS2ST(DEBOUNCE_TIME), led1toggle, NULL);
    chSysUnlockFromISR();
  } else if (debouncing1 && (pad1 == PAL_HIGH)) {
    chSysLockFromISR();
    if (chVTIsArmedI(&vt1))
      chVTResetI(&vt1);
    chSysUnlockFromISR();
    debouncing1 = false;
  }
}

static void extcb2(EXTDriver *extp, expchannel_t channel)
{
  (void)extp;
  (void)channel;

  uint8_t pad2 = palReadPad(IOPORT1, KEY2);

  if (!debouncing2 && (pad2 == PAL_LOW)) {
    debouncing2 = true;
    chSysLockFromISR();
    chVTSetI(&vt2, MS2ST(DEBOUNCE_TIME), led2toggle, NULL);
    chSysUnlockFromISR();
  } else if (debouncing2 && (pad2 == PAL_HIGH)) {
    chSysLockFromISR();
    if (chVTIsArmedI(&vt2))
      chVTResetI(&vt2);
    chSysUnlockFromISR();
    debouncing2 = false;
  }
}

static const EXTConfig extcfg =
{
  {
    {EXT_CH_MODE_FALLING_EDGE | EXT_CH_MODE_AUTOSTART | (KEY1 << EXT_MODE_GPIO_OFFSET), extcb1},
    {EXT_CH_MODE_RISING_EDGE | EXT_CH_MODE_AUTOSTART | (KEY2 << EXT_MODE_GPIO_OFFSET), extcb2},
  }
};

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
   */
  halInit();
  chSysInit();

  extStart(&EXTD1, &extcfg);

  /*
   * Normal main() thread activity, in this demo it enables and disables the
   * button EXT channel using 5 seconds intervals.
   */
  while (TRUE) {
    palSetPad(IOPORT1, LED0);
    chThdSleepMilliseconds(5000);
    extChannelDisable(&EXTD1, 0);
    extChannelDisable(&EXTD1, 1);
    palClearPad(IOPORT1, LED0);
    chThdSleepMilliseconds(5000);
    extChannelEnable(&EXTD1, 0);
    extChannelEnable(&EXTD1, 1);
  }

  return 0;
}
