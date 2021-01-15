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

#include <string.h>

#include "hal.h"
#include "boarddef.h"

/*
 ******************************************************************************
 * ERROR CHECKS
 ******************************************************************************
 */

#if defined(BOARD_ST_STM32F4_DISCOVERY) || \
    defined(BOARD_ST_STM32F0_DISCOVERY) || \
    defined(BOARD_ST_STM32F0308_DISCOVERY)
  #if ONEWIRE_USE_STRONG_PULLUP
  #error "This board has not enough voltage for this feature"
  #endif
#endif

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */

/*
 ******************************************************************************
 * PROTOTYPES
 ******************************************************************************
 */
/*
 * Forward declarations
 */
#if ONEWIRE_USE_STRONG_PULLUP
static void strong_pullup_assert(void);
static void strong_pullup_release(void);
#endif

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

static uint8_t testbuf[12];

/* stores 3 temperature values in millicelsius */
static int32_t temperature[3];

/*
 * Config for underlying PWM driver.
 * Note! It is NOT constant because 1-wire driver needs to change them
 * during functioning.
 */
static PWMConfig pwm_cfg = {
    0,
    0,
    NULL,
    {
     {PWM_OUTPUT_DISABLED, NULL},
     {PWM_OUTPUT_DISABLED, NULL},
     {PWM_OUTPUT_DISABLED, NULL},
     {PWM_OUTPUT_DISABLED, NULL}
    },
    0,
#if STM32_PWM_USE_ADVANCED
    0,
#endif
    0
};

/*
 *
 */
static const onewireConfig ow_cfg = {
    &PWMD3,
    &pwm_cfg,
    PWM_OUTPUT_ACTIVE_LOW,
    ONEWIRE_MASTER_CHANNEL,
    ONEWIRE_SAMPLE_CHANNEL,
    ONEWIRE_PORT,
    ONEWIRE_PIN,
#if defined(STM32F1XX)
    ONEWIRE_PAD_MODE_IDLE,
#endif
    ONEWIRE_PAD_MODE_ACTIVE,
#if ONEWIRE_USE_STRONG_PULLUP
    strong_pullup_assert,
    strong_pullup_release
#endif
};

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */

#if ONEWIRE_USE_STRONG_PULLUP
/**
 *
 */
static void strong_pullup_assert(void) {
  palSetPadMode(ONEWIRE_PORT, ONEWIRE_PIN, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
}

/**
 *
 */
static void strong_pullup_release(void) {
  palSetPadMode(ONEWIRE_PORT, ONEWIRE_PIN, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);
}
#endif /* ONEWIRE_USE_STRONG_PULLUP */

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

/*
 *
 */
void onewireTest(void) {

  int16_t tmp;
  uint8_t rombuf[24];
  size_t devices_on_bus = 0;
  size_t i = 0;
  bool presence;

  onewireObjectInit(&OWD1);
  onewireStart(&OWD1, &ow_cfg);

#if ONEWIRE_SYNTH_SEARCH_TEST
  synthSearchRomTest(&OWD1);
#endif

  for (i=0; i<3; i++)
    temperature[i] = -666;

  while (true) {
    if (true == onewireReset(&OWD1)){

      memset(rombuf, 0x55, sizeof(rombuf));
      search_led_on();
      devices_on_bus = onewireSearchRom(&OWD1, rombuf, 3);
      search_led_off();
      osalDbgCheck(devices_on_bus <= 3);
      osalDbgCheck(devices_on_bus  > 0);

      if (1 == devices_on_bus){
        /* test read rom command */
        presence = onewireReset(&OWD1);
        osalDbgCheck(true == presence);
        testbuf[0] = ONEWIRE_CMD_READ_ROM;
        onewireWrite(&OWD1, testbuf, 1, 0);
        onewireRead(&OWD1, testbuf, 8);
        osalDbgCheck(testbuf[7] == onewireCRC(testbuf, 7));
        osalDbgCheck(0 == memcmp(rombuf, testbuf, 8));
      }

      /* start temperature measurement on all connected devices at once */
      presence = onewireReset(&OWD1);
      osalDbgCheck(true == presence);
      testbuf[0] = ONEWIRE_CMD_SKIP_ROM;
      testbuf[1] = ONEWIRE_CMD_CONVERT_TEMP;

#if ONEWIRE_USE_STRONG_PULLUP
      onewireWrite(&OWD1, testbuf, 2, TIME_MS2I(750));
#else
      onewireWrite(&OWD1, testbuf, 2, 0);
      /* poll bus waiting ready signal from all connected devices */
      testbuf[0] = 0;
      while (testbuf[0] == 0){
        osalThreadSleepMilliseconds(50);
        onewireRead(&OWD1, testbuf, 1);
      }
#endif

      for (i=0; i<devices_on_bus; i++) {
        /* read temperature device by device from their scratchpads */
        presence = onewireReset(&OWD1);
        osalDbgCheck(true == presence);

        testbuf[0] = ONEWIRE_CMD_MATCH_ROM;
        memcpy(&testbuf[1], &rombuf[i*8], 8);
        testbuf[9] = ONEWIRE_CMD_READ_SCRATCHPAD;
        onewireWrite(&OWD1, testbuf, 10, 0);

        onewireRead(&OWD1, testbuf, 9);
        osalDbgCheck(testbuf[8] == onewireCRC(testbuf, 8));
        memcpy(&tmp, &testbuf, 2);
        temperature[i] = ((int32_t)tmp * 625) / 10;
      }
    }
    else {
      osalSysHalt("No devices found");
    }
    osalThreadSleep(1); /* enforce ChibiOS's stack overflow check */
  }

  onewireStop(&OWD1);
}
