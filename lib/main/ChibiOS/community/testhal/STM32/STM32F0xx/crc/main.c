/*
    ChibiOS - Copyright (C) 2015 Michael D. Spradling

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


/*
 * Data used for CRC calculation.
 */
uint8_t data[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

uint32_t gCrc = 0;


/*
 * CRC Callback used with DMA testing
 */
void crc_callback(CRCDriver *crcp, uint32_t crc) {
  (void)crcp;
  gCrc = crc;
}


/*
 * CRC32 configuration
 */
static const CRCConfig crc32_config = {
  .poly_size         = 32,
  .poly              = 0x04C11DB7,
  .initial_val       = 0xFFFFFFFF,
  .final_val         = 0xFFFFFFFF,
  .reflect_data      = 1,
  .reflect_remainder = 1
};

/*
 * CRC16 configuration
 */
static const CRCConfig crc16_config = {
  .poly_size         = 16,
  .poly              = 0x8005,
  .initial_val       = 0x0,
  .final_val         = 0x0,
  .reflect_data      = 1,
  .reflect_remainder = 1
};

/*
 * CRC OpenPGP 
 */
static const CRCConfig crc8_config = {
  .poly_size         = 8,
  .poly              = 0x07,
  .initial_val       = 0x0,
  .final_val         = 0x0,
  .reflect_data      = 0,
  .reflect_remainder = 0
};


#if CRC_USE_DMA == TRUE
/*
 * CRC32 configuration with DMA
 */
static const CRCConfig crc32_dma_config = {
  .poly_size         = 32,
  .poly              = 0x04C11DB7,
  .initial_val       = 0xFFFFFFFF,
  .final_val         = 0xFFFFFFFF,
  .reflect_data      = 1,
  .reflect_remainder = 1,
  .end_cb = crc_callback
};

/*
 * CRC16 configuration with DMA
 */
static const CRCConfig crc16_dma_config = {
  .poly_size         = 16,
  .poly              = 0x8005,
  .initial_val       = 0x0,
  .final_val         = 0x0,
  .reflect_data      = 1,
  .reflect_remainder = 1,
  .end_cb = crc_callback
};
#endif


static void testCrc(const CRCConfig *config, uint32_t result) {
  uint32_t crc;

  crcAcquireUnit(&CRCD1);             /* Acquire ownership of the bus.    */
  crcStart(&CRCD1, config);           /* Activate CRC driver              */
  crcReset(&CRCD1);
  crc = crcCalc(&CRCD1, sizeof(data), &data);
  osalDbgAssert(crc == result, "CRC does not match expected result");
  crcStop(&CRCD1);                    /* Deactive CRC driver);            */
  crcReleaseUnit(&CRCD1);             /* Acquire ownership of the bus.    */
}


#if CRC_USE_DMA
static void testCrcDma(const CRCConfig *config, uint32_t result) {
  gCrc = 0;

  crcAcquireUnit(&CRCD1);             /* Acquire ownership of the bus.    */
  crcStart(&CRCD1, config);           /* Activate CRC driver              */
  crcReset(&CRCD1);
  crcStartCalc(&CRCD1, sizeof(data), &data);
  while (gCrc == 0);                  /* Wait for callback to verify      */
  crcStop(&CRCD1);                    /* Deactive CRC driver);            */
  crcReleaseUnit(&CRCD1);             /* Acquire ownership of the bus.    */

  osalDbgAssert(gCrc == result, "CRC does not match expected result");
}
#endif

/*
 * CRC thread
 */
static THD_WORKING_AREA(crc_thread_1_wa, 256);
static THD_FUNCTION(crc_thread_1, p) {
  (void)p;
  chRegSetThreadName("CRC thread 1");
  while (true) {

/* Test ST hardware CRC */
/* if CRC_USE_DMA == TRUE these sync function internally use DMA and put the
 * calling thread to sleep */
#if STM32_CRC_USE_CRC1 == TRUE
    /* CRC32 Calculation */
    testCrc(&crc32_config, 0x91267e8a);
    /* CRC16 Calculation */
    testCrc(&crc16_config, 0xc36a);
    /* CRC8 Calculation */
    testCrc(&crc8_config, 0x06);

/* Test ST CRC with DMA */
#if CRC_USE_DMA == TRUE
    /* CRC32 Calculation */
    testCrcDma(&crc32_dma_config, 0x91267e8a);
    /* CRC16 Calculation */
    testCrcDma(&crc16_dma_config, 0xc36a);
#endif

#endif /* STM32_CRC_USE_CRC1 */


/* Test software CRC */
#if CRCSW_USE_CRC1 == TRUE
/* Test CRCSW with compute CRC */
#if CRCSW_PROGRAMMABLE == TRUE
    /* CRC32 Calculation */
    testCrc(&crc32_config, 0x91267e8a);
    /* CRC16 Calculation */
    testCrc(&crc16_config, 0xc36a);
    testCrc(&crc8_config, 0x06);
#endif
/* Test CRCSW with table lookups.  */
#if CRCSW_CRC32_TABLE == TRUE
    /* CRC32 Calculation with table lookup */
    testCrc(CRCSW_CRC32_TABLE_CONFIG, 0x91267e8a);
#endif
#if CRCSW_CRC16_TABLE == TRUE
    /* CRC16 Calculation with table lookup */
    testCrc(CRCSW_CRC16_TABLE_CONFIG, 0xc36a);
#endif

#endif /* CRCSW_USE_CRC1 */
  }
}


/*
 * Red LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    palClearPad(GPIOC, GPIOC_LED_RED);
    chThdSleepMilliseconds(500);
    palSetPad(GPIOC, GPIOC_LED_RED);
    chThdSleepMilliseconds(500);
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

  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Starting the CRC thread
   */
  chThdCreateStatic(crc_thread_1_wa, sizeof(crc_thread_1_wa),
                    NORMALPRIO + 1, crc_thread_1, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing.
   */
  while (true) {
    chThdSleepMilliseconds(500);
  }

  return 0;
}
