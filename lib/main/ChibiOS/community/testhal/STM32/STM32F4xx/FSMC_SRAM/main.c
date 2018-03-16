/*
    ChibiOS/RT - Copyright (C) 2013-2014 Uladzimir Pylinsky aka barthess

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

#include "hal_fsmc_sram.h"
#include "membench.h"
#include "memtest.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define SRAM_SIZE       (512 * 1024)
#define SRAM_START      ((void *)FSMC_Bank1_4_MAP)

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

static void mem_error_cb(memtest_t *memp, testtype type, size_t index,
                         size_t width, uint32_t got, uint32_t expect);

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

static size_t errors = 0;

/*
 *
 */
static uint8_t int_buf[64*1024];

/*
 * SRAM driver configuration structure.
 */
static const SRAMConfig sram_cfg = {
    .bcr  = (FSMC_BCR_MWID_16 | FSMC_BCR_MTYP_SRAM | FSMC_BCR_WREN),
    .btr  = (0 << 16) | (2 << 8) | (1 << 0),
    .bwtr = (0 << 16) | (2 << 8) | (1 << 0)
};

/*
 *
 */
static memtest_t memtest_struct = {
    SRAM_START,
    SRAM_SIZE,
    MEMTEST_WIDTH_32,
    mem_error_cb
};

/*
 *
 */
static membench_t membench_ext = {
    SRAM_START,
    SRAM_SIZE,
};

/*
 *
 */
static membench_t membench_int = {
    int_buf,
    sizeof(int_buf),
};

/*
 *
 */
static membench_result_t membench_result_ext2int;
static membench_result_t membench_result_int2ext;

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */

static inline void red_led_on(void)       {palSetPad(GPIOI,     GPIOI_LED_R);}
static inline void red_led_off(void)      {palClearPad(GPIOI,   GPIOI_LED_R);}
static inline void green_led_on(void)     {palSetPad(GPIOI,     GPIOI_LED_G);}
static inline void green_led_off(void)    {palClearPad(GPIOI,   GPIOI_LED_G);}
static inline void green_led_toggle(void) {palTogglePad(GPIOI,  GPIOI_LED_G);}

static void mem_error_cb(memtest_t *memp, testtype type, size_t index,
                         size_t width, uint32_t got, uint32_t expect) {
  (void)memp;
  (void)type;
  (void)index;
  (void)width;
  (void)got;
  (void)expect;

  green_led_off();
  red_led_on();
  osalThreadSleepMilliseconds(10);
  errors++;
  osalSysHalt("Memory broken");
}

/*
 *
 */
static void memtest(void) {

  red_led_off();

  while (true) {
    memtest_run(&memtest_struct, MEMTEST_RUN_ALL);
    green_led_toggle();
  }

  green_led_on();
  green_led_off();
}

/*
 *
 */
static void membench(void) {
  membench_run(&membench_ext, &membench_int, &membench_result_int2ext);
  membench_run(&membench_int, &membench_ext, &membench_result_ext2int);
}

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

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

  fsmcSramInit();
  fsmcSramStart(&SRAMD4, &sram_cfg);

  membench();
  memtest();

  /*
   * Normal main() thread activity, in this demo it does nothing.
   */
  while (TRUE) {
    chThdSleepMilliseconds(500);
  }
}


