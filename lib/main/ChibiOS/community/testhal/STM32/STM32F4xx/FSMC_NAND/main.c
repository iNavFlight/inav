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

/*
 * Hardware notes.
 *
 * Use external pullup on ready/busy pin of NAND IC for a speed reason.
 *
 * Chose MCU with 140 (or more) pins package because 100 pins packages
 * has no dedicated interrupt pins for FSMC.
 *
 * If your hardware already done using 100 pin package than you have to:
 * 1) connect ready/busy pin to GPIOD6 (NWAIT in terms of STM32)
 * 2) set GPIOD6 pin as input with pullup and connect it to alternate
 * function0 (not function12)
 * 3) set up EXTI to catch raising edge on GPIOD6 and call NAND driver's
 * isr_handler() function from an EXTI callback.
 *
 * If you use MLC flash memory do NOT use ECC to detect/correct
 * errors because of its weakness. Use Rid-Solomon on BCH code instead.
 * Yes, you have to realize it in sowftware yourself.
 */

/*
 * Software notes.
 *
 * For correct calculation of timing values you need AN2784 document
 * from STMicro.
 */

#include "ch.h"
#include "hal.h"

#include "bitmap.h"

#include "dma_storm.h"
#include "string.h"
#include "stdlib.h"

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */

#define USE_KILL_BLOCK_TEST       FALSE

#define FSMCNAND_TIME_SET         ((uint32_t) 2) //(8nS)
#define FSMCNAND_TIME_WAIT        ((uint32_t) 6) //(30nS)
#define FSMCNAND_TIME_HOLD        ((uint32_t) 1) //(5nS)
#define FSMCNAND_TIME_HIZ         ((uint32_t) 4) //(20nS)

#define NAND_BLOCKS_COUNT         8192
#define NAND_PAGE_DATA_SIZE       2048
#define NAND_PAGE_SPARE_SIZE      64
#define NAND_PAGE_SIZE            (NAND_PAGE_SPARE_SIZE + NAND_PAGE_DATA_SIZE)
#define NAND_PAGES_PER_BLOCK      64
#define NAND_ROW_WRITE_CYCLES     3
#define NAND_COL_WRITE_CYCLES     2

#define NAND_TEST_START_BLOCK     1200
#define NAND_TEST_END_BLOCK       1300

#if USE_KILL_BLOCK_TEST
#define NAND_TEST_KILL_BLOCK      8000
#endif

#if STM32_NAND_USE_FSMC_NAND1
  #define NAND                    NANDD1
#elif STM32_NAND_USE_FSMC_NAND2
  #define NAND                    NANDD2
#else
#error "You should enable at least one NAND interface"
#endif

#define BAD_MAP_LEN     (NAND_BLOCKS_COUNT / (sizeof(bitmap_word_t) * 8))

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
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
/*
 *
 */
static uint8_t nand_buf[NAND_PAGE_SIZE];
static uint8_t ref_buf[NAND_PAGE_SIZE];

/*
 *
 */
static time_measurement_t tmu_erase;
static time_measurement_t tmu_write_data;
static time_measurement_t tmu_write_spare;
static time_measurement_t tmu_read_data;
static time_measurement_t tmu_read_spare;
static time_measurement_t tmu_driver_start;

/*
 *
 */
static bitmap_word_t badblock_map_array[BAD_MAP_LEN];
static bitmap_t badblock_map = {
    badblock_map_array,
    BAD_MAP_LEN
};

/*
 *
 */
static const NANDConfig nandcfg = {
    NAND_BLOCKS_COUNT,
    NAND_PAGE_DATA_SIZE,
    NAND_PAGE_SPARE_SIZE,
    NAND_PAGES_PER_BLOCK,
    NAND_ROW_WRITE_CYCLES,
    NAND_COL_WRITE_CYCLES,
    /* stm32 specific fields */
    ((FSMCNAND_TIME_HIZ << 24) | (FSMCNAND_TIME_HOLD << 16) | \
                                 (FSMCNAND_TIME_WAIT << 8) | FSMCNAND_TIME_SET)
};

static volatile uint32_t BackgroundThdCnt = 0;
static thread_reference_t background_thd_ptr = NULL;

#if USE_KILL_BLOCK_TEST
static uint32_t KillCycle = 0;
#endif

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */
static void nand_wp_assert(void)   {palClearPad(GPIOB, GPIOB_NAND_WP);}
static void nand_wp_release(void)  {palSetPad(GPIOB, GPIOB_NAND_WP);}
//static void red_led_on(void)       {palSetPad(GPIOI, GPIOI_LED_R);}
static void red_led_off(void)      {palClearPad(GPIOI, GPIOI_LED_R);}
static void red_led_toggle(void)   {palTogglePad(GPIOI, GPIOI_LED_R);}
static void green_led_toggle(void) {palTogglePad(GPIOI, GPIOI_LED_G);}

/**
 *
 */
static THD_WORKING_AREA(BackgroundThreadWA, 128);
static THD_FUNCTION(BackgroundThread, arg) {
  (void)arg;

  while(true){
    BackgroundThdCnt++;
  }
}

/*
 *
 */
static bool is_erased(NANDDriver *dp, size_t block){
  uint32_t page = 0;
  size_t i = 0;

  for (page=0; page<NAND.config->pages_per_block; page++){
    nandReadPageData(dp, block, page, nand_buf, NAND.config->page_data_size, NULL);
    nandReadPageSpare(dp, block, page, &nand_buf[2048], NAND.config->page_spare_size);
    for (i=0; i<sizeof(nand_buf); i++) {
      if (nand_buf[i] != 0xFF)
        return false;
    }
  }

  return true;
}

/*
 *
 */
static void pattern_fill(void) {

  size_t i;

  srand(chSysGetRealtimeCounterX());

  for(i=0; i<NAND_PAGE_SIZE; i++){
    ref_buf[i] = rand() & 0xFF;
  }

  /* protect bad mark */
  ref_buf[NAND_PAGE_DATA_SIZE]     = 0xFF;
  ref_buf[NAND_PAGE_DATA_SIZE + 1] = 0xFF;
  memcpy(nand_buf, ref_buf, NAND_PAGE_SIZE);

  /* paranoid mode ON */
  osalDbgCheck(0 == memcmp(ref_buf, nand_buf, NAND_PAGE_SIZE));
}

/*
 *
 */
#if USE_KILL_BLOCK_TEST
static void kill_block(NANDDriver *nandp, uint32_t block){

  size_t i = 0;
  size_t page = 0;
  uint8_t op_status;

  /* This test requires good block.*/
  osalDbgCheck(!nandIsBad(nandp, block));

  while(true){
    op_status = nandErase(&NAND, block);
    if (0 != (op_status & 1)){
      if(!is_erased(nandp, block))
        osalSysHalt("Block successfully killed");
    }
    if(!is_erased(nandp, block))
      osalSysHalt("Block block not erased, but erase operation report success");

    for (page=0; page<nandp->config->pages_per_block; page++){
      memset(nand_buf, 0, NAND_PAGE_SIZE);
      op_status = nandWritePageWhole(nandp, block, page, nand_buf, NAND_PAGE_SIZE);
      if (0 != (op_status & 1)){
        nandReadPageWhole(nandp, block, page, nand_buf, NAND_PAGE_SIZE);
        for (i=0; i<NAND_PAGE_SIZE; i++){
          if (nand_buf[i] != 0)
            osalSysHalt("Block successfully killed");
        }
      }

      nandReadPageWhole(nandp, block, page, nand_buf, NAND_PAGE_SIZE);
      for (i=0; i<NAND_PAGE_SIZE; i++){
        if (nand_buf[i] != 0)
          osalSysHalt("Page write failed, but write operation report success");
      }
    }
    KillCycle++;
  }
}
#endif /* USE_KILL_BLOCK_TEST */

/*
 *
 */
typedef enum {
  ECC_NO_ERROR = 0,
  ECC_CORRECTABLE_ERROR = 1,
  ECC_UNCORRECTABLE_ERROR = 2,
  ECC_CORRUPTED = 3,
} ecc_result_t;

/*
 *
 */
static ecc_result_t parse_ecc(uint32_t ecclen,
                    uint32_t ecc1, uint32_t ecc2, uint32_t *corrupted){

  size_t i = 0;
  uint32_t corr = 0;
  uint32_t e = 0;
  uint32_t shift = (32 - ecclen);
  uint32_t b0, b1;

  ecc1 <<= shift;
  ecc1 >>= shift;
  ecc2 <<= shift;
  ecc2 >>= shift;
  e = ecc1 ^ ecc2;

  if (0 == e){
    return ECC_NO_ERROR;
  }
  else if (((e - 1) & e) == 0){
    return ECC_CORRUPTED;
  }
  else {
    for (i=0; i<ecclen/2; i++){
      b0 = e & 1;
      e >>= 1;
      b1 = e & 1;
      e >>= 1;
      if ((b0 + b1) != 1)
        return ECC_UNCORRECTABLE_ERROR;
      corr |= b1 << i;
    }
    *corrupted = corr;
    return ECC_CORRECTABLE_ERROR;
  }
}

/*
 *
 */
static void invert_bit(uint8_t *buf, uint32_t byte, uint32_t bit){
  osalDbgCheck((byte < NAND_PAGE_DATA_SIZE) && (bit < 8));
  buf[byte] ^= ((uint8_t)1) << bit;
}

/*
 *
 */
static void ecc_test(NANDDriver *nandp, uint32_t block){

  uint32_t corrupted;
  uint32_t byte, bit;
  const uint32_t ecclen = 28;
  uint32_t ecc_ref, ecc_broken;
  uint8_t op_status;
  ecc_result_t ecc_result = ECC_NO_ERROR;

  /* This test requires good block.*/
  osalDbgCheck(!nandIsBad(nandp, block));
  if (!is_erased(nandp, block))
    nandErase(&NAND, block);

  pattern_fill();

  /*** Correctable errors ***/
  op_status = nandWritePageData(nandp, block, 0,
                nand_buf, nandp->config->page_data_size, &ecc_ref);
  osalDbgCheck(0 == (op_status & 1)); /* operation failed */
  nandReadPageData(nandp, block, 0,
                  nand_buf, nandp->config->page_data_size, &ecc_broken);
  ecc_result = parse_ecc(ecclen, ecc_ref, ecc_broken, &corrupted);
  osalDbgCheck(ECC_NO_ERROR == ecc_result); /* unexpected error */

  /**/
  byte = 0;
  bit = 7;
  invert_bit(nand_buf, byte, bit);
  op_status = nandWritePageData(nandp, block, 1,
                nand_buf, nandp->config->page_data_size, &ecc_broken);
  osalDbgCheck(0 == (op_status & 1)); /* operation failed */
  invert_bit(nand_buf, byte, bit);
  ecc_result = parse_ecc(ecclen, ecc_ref, ecc_broken, &corrupted);
  osalDbgCheck(ECC_CORRECTABLE_ERROR == ecc_result); /* this error must be correctable */
  osalDbgCheck(corrupted == (byte * 8 + bit)); /* wrong correction code */

  /**/
  byte = 2047;
  bit = 0;
  invert_bit(nand_buf, byte, bit);
  op_status = nandWritePageData(nandp, block, 2,
                nand_buf, nandp->config->page_data_size, &ecc_broken);
  osalDbgCheck(0 == (op_status & 1)); /* operation failed */
  invert_bit(nand_buf, byte, bit);
  ecc_result = parse_ecc(ecclen, ecc_ref, ecc_broken, &corrupted);
  osalDbgCheck(ECC_CORRECTABLE_ERROR == ecc_result); /* this error must be correctable */
  osalDbgCheck(corrupted == (byte * 8 + bit)); /* wrong correction code */

  /**/
  byte = 1027;
  bit = 3;
  invert_bit(nand_buf, byte, bit);
  op_status = nandWritePageData(nandp, block, 3,
                nand_buf, nandp->config->page_data_size, &ecc_broken);
  osalDbgCheck(0 == (op_status & 1)); /* operation failed */
  invert_bit(nand_buf, byte, bit);
  ecc_result = parse_ecc(ecclen, ecc_ref, ecc_broken, &corrupted);
  osalDbgCheck(ECC_CORRECTABLE_ERROR == ecc_result); /* this error must be correctable */
  osalDbgCheck(corrupted == (byte * 8 + bit)); /* wrong correction code */

  /*** Uncorrectable error ***/
  byte = 1027;
  invert_bit(nand_buf, byte, 3);
  invert_bit(nand_buf, byte, 4);
  op_status = nandWritePageData(nandp, block, 4,
                nand_buf, nandp->config->page_data_size, &ecc_broken);
  osalDbgCheck(0 == (op_status & 1)); /* operation failed */
  invert_bit(nand_buf, byte, 3);
  invert_bit(nand_buf, byte, 4);
  ecc_result = parse_ecc(28, ecc_ref, ecc_broken, &corrupted);
  osalDbgCheck(ECC_UNCORRECTABLE_ERROR == ecc_result); /* This error must be NOT correctable */

  /*** make clean ***/
  nandErase(&NAND, block);
}

/*
 *
 */
static void general_test (NANDDriver *nandp, size_t first,
                                      size_t last, size_t read_rounds){

  size_t block, page, round;
  bool status;
  uint8_t op_status;
  uint32_t recc, wecc;

  /* initialize time measurement units */
  chTMObjectInit(&tmu_erase);
  chTMObjectInit(&tmu_write_data);
  chTMObjectInit(&tmu_write_spare);
  chTMObjectInit(&tmu_read_data);
  chTMObjectInit(&tmu_read_spare);

  /* perform basic checks */
  for (block=first; block<last; block++){
    red_led_toggle();
    if (!nandIsBad(nandp, block)){
      if (!is_erased(nandp, block)){
        op_status = nandErase(nandp, block);
        osalDbgCheck(0 == (op_status & 1)); /* operation failed */
      }
    }
  }

  /* check fail status */
  for (block=first; block<last; block++){
    if (!nandIsBad(nandp, block)){
      if (!is_erased(nandp, block)){
        op_status = nandErase(nandp, block);
        osalDbgCheck(0 == (op_status & 1)); /* operation failed */
      }
      pattern_fill();
      op_status = nandWritePageData(nandp, block, 0,
                    nand_buf, nandp->config->page_data_size, &wecc);
      osalDbgCheck(0 == (op_status & 1));

      pattern_fill();
      op_status = nandWritePageData(nandp, block, 0,
                    nand_buf, nandp->config->page_data_size, &wecc);
      /* operation must failed here because of write in unerased space */
      osalDbgCheck(1 == (op_status & 1));
    }
  }

  /* write block with pattern, read it back and compare */
  for (block=first; block<last; block++){
    red_led_toggle();
    if (!nandIsBad(nandp, block)){
      for (page=0; page<nandp->config->pages_per_block; page++){
        pattern_fill();

        chTMStartMeasurementX(&tmu_write_data);
        op_status = nandWritePageData(nandp, block, page,
                      nand_buf, nandp->config->page_data_size, &wecc);
        chTMStopMeasurementX(&tmu_write_data);
        osalDbgCheck(0 == (op_status & 1)); /* operation failed */

        chTMStartMeasurementX(&tmu_write_spare);
        op_status = nandWritePageSpare(nandp, block, page,
                      nand_buf + nandp->config->page_data_size,
                      nandp->config->page_spare_size);
        chTMStopMeasurementX(&tmu_write_spare);
        osalDbgCheck(0 == (op_status & 1)); /* operation failed */

        /* read back and compare */
        for (round=0; round<read_rounds; round++){
          memset(nand_buf, 0, NAND_PAGE_SIZE);

          chTMStartMeasurementX(&tmu_read_data);
          nandReadPageData(nandp, block, page,
                      nand_buf, nandp->config->page_data_size, &recc);
          chTMStopMeasurementX(&tmu_read_data);
          osalDbgCheck(0 == (recc ^ wecc)); /* ECC error detected */

          chTMStartMeasurementX(&tmu_read_spare);
          nandReadPageSpare(nandp, block, page,
                      nand_buf + nandp->config->page_data_size,
                      nandp->config->page_spare_size);
          chTMStopMeasurementX(&tmu_read_spare);

          osalDbgCheck(0 == memcmp(ref_buf, nand_buf, NAND_PAGE_SIZE)); /* Read back failed */
        }
      }

      /* make clean */
      chTMStartMeasurementX(&tmu_erase);
      op_status = nandErase(nandp, block);
      chTMStopMeasurementX(&tmu_erase);
      osalDbgCheck(0 == (op_status & 1)); /* operation failed */

      status = is_erased(nandp, block);
      osalDbgCheck(true == status); /* blocks was not erased successfully */
    }/* if (!nandIsBad(nandp, block)){ */
  }
  red_led_off();
}

/*
 *
 */
static void nand_test(bool use_badblock_map) {

  /* performance counters */
  int32_t adc_ints = 0;
  int32_t uart_ints = 0;
  int32_t adc_idle_ints = 0;
  int32_t uart_idle_ints = 0;
  uint32_t background_cnt = 0;
  systime_t T = 0;

  chTMObjectInit(&tmu_driver_start);
  chTMStartMeasurementX(&tmu_driver_start);
  if (use_badblock_map) {
    nandStart(&NAND, &nandcfg, &badblock_map);
  }
  else {
    nandStart(&NAND, &nandcfg, NULL);
  }
  chTMStopMeasurementX(&tmu_driver_start);

  chThdSleepMilliseconds(4000);

  BackgroundThdCnt = 0;
  if (NULL != background_thd_ptr) {
    background_thd_ptr = chThdCreateStatic(BackgroundThreadWA,
        sizeof(BackgroundThreadWA), NORMALPRIO - 10, BackgroundThread, NULL);
  }

  /*
   * run NAND test in parallel with DMA loads and background thread
   */
  dma_storm_adc_start();
  dma_storm_uart_start();
  T = chVTGetSystemTimeX();
  general_test(&NAND, NAND_TEST_START_BLOCK, NAND_TEST_END_BLOCK, 1);
  T = chVTGetSystemTimeX() - T;
  adc_ints  = dma_storm_adc_stop();
  uart_ints = dma_storm_uart_stop();
  chSysLock();
  background_cnt = BackgroundThdCnt;
  BackgroundThdCnt = 0;
  chSysUnlock();

  /*
   * run DMA load and background thread _without_ NAND test
   */
  dma_storm_adc_start();
  dma_storm_uart_start();
  chThdSleep(T);
  adc_idle_ints = dma_storm_adc_stop();
  uart_idle_ints = dma_storm_uart_stop();

  /*
   * ensure that NAND code have negligible impact on other subsystems
   */
  osalDbgCheck(background_cnt > (BackgroundThdCnt / 4));
  osalDbgCheck(abs(adc_ints  - adc_idle_ints)  < (adc_idle_ints  / 20));
  osalDbgCheck(abs(uart_ints - uart_idle_ints) < (uart_idle_ints / 20));

  /*
   * perform ECC calculation test
   */
  ecc_test(&NAND, NAND_TEST_END_BLOCK);
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

  nand_wp_release();

  nand_test(true);
  nand_test(false);

#if USE_KILL_BLOCK_TEST
  kill_block(&NAND, NAND_TEST_KILL_BLOCK);
#endif

  nand_wp_assert();

  /*
   * Normal main() thread activity, in this demo it does nothing.
   */
  red_led_off();
  while (true) {
    green_led_toggle();
    chThdSleepMilliseconds(500);
  }
}


