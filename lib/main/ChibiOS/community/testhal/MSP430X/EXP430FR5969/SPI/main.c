/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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
#include "hal_dma_lld.h"
#include "string.h"

/* Disable watchdog because of lousy startup code in newlib */
static void __attribute__((naked, section(".crt_0042disable_watchdog"), used))
disable_watchdog(void) {
  WDTCTL = WDTPW | WDTHOLD;
}

const char * start_msg  = "\r\n\r\nExecuting SPI test suite...\r\n";
const char * test_1_msg = "TEST 1: spiStartIgnore, with callback\r\n";
const char * test_2_msg = "TEST 2: spiStartExchange, with callback\r\n";
const char * test_3_msg = "TEST 3: spiStartSend, with callback\r\n";
const char * test_4_msg = "TEST 4: spiStartReceive, with callback\r\n";
const char * test_5_msg = "TEST 5: spiIgnore\r\n";
const char * test_6_msg = "TEST 6: spiExchange\r\n";
const char * test_7_msg = "TEST 7: spiSend\r\n";
const char * test_8_msg = "TEST 8: spiReceive\r\n";
const char * test_9_msg = "TEST 9: spiPolledExchange\r\n";
const char * test_10_msg = "TEST 10: spiStartExchange with exclusive DMA\r\n";
const char * test_11_msg =
    "TEST 11: spiStartExchange with exclusive DMA for TX\r\n";
const char * test_12_msg =
    "TEST 12: spiStartExchange with exclusive DMA for RX\r\n";

const char * succeed_string = "SUCCESS\r\n\r\n";
const char * fail_string    = "FAILURE\r\n\r\n";

char instring[256];
char outstring[256];
uint8_t cb_arg = 1;

void spi_callback(SPIDriver * spip) {
  (void)spip;
  cb_arg = 0;
}

SPIConfig SPIDA1_config = {
  spi_callback,         /* callback */
  PAL_NOLINE,           /* hardware slave select line */
  250000,               /* data rate */
  MSP430X_SPI_BO_LSB,   /* bit order */
  MSP430X_SPI_DS_EIGHT, /* data size */
  0,                    /* SPI mode */
  0xFFU,                /* no exclusive TX DMA */
  0xFFU                 /* no exclusive RX DMA */
};

SPIConfig SPIDB0_config = {
  NULL,                 /* callback */
  LINE_LED_G,           /* GPIO slave select line */
  1000,                 /* data rate */
  MSP430X_SPI_BO_MSB,   /* bit order */
  MSP430X_SPI_DS_SEVEN, /* data size */
  3,                    /* SPI mode */
  0xFF,                 /* no exclusive TX DMA */
  0xFF                  /* no exclusive RX DMA */
};

/*
 * Thread 2.
 */
THD_WORKING_AREA(waThread1, 4096);
THD_FUNCTION(Thread1, arg) {

  (void)arg;

  /* Set up loopback mode for testing */
  SPIDA1.regs->statw_a |= UCLISTEN;
  SPIDB0.regs->statw_b |= UCLISTEN;

  /*
   * Activate the serial driver 0 using the driver default configuration.
   */
  sdStart(&SD0, NULL);

  /* Activate the SPI driver A1 using its config */
  spiStart(&SPIDA1, &SPIDA1_config);
  /* Activate the SPI driver B0 using its config */
  spiStart(&SPIDB0, &SPIDB0_config);

  while (chnGetTimeout(&SD0, TIME_INFINITE)) {
    chnWrite(&SD0, (const uint8_t *)start_msg, strlen(start_msg));
    chThdSleepMilliseconds(2000);

    /* Test 1 - spiStartIgnore with callback */
    chnWrite(&SD0, (const uint8_t *)test_1_msg, strlen(test_1_msg));
    strcpy(outstring, "After SPI test  \r\n");
    strcpy(instring, "Before SPI test \r\n");
    cb_arg = 1;
    if (strcmp("Before SPI test \r\n", instring) ||
        strcmp("After SPI test  \r\n", outstring) || cb_arg != 1) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    spiSelect(&SPIDA1);
    spiStartIgnore(&SPIDA1, strlen(outstring));
    while (SPIDA1.state != SPI_READY)
      ; /* wait for transaction to finish */
    spiUnselect(&SPIDA1);
    if (strcmp("Before SPI test \r\n", instring) ||
        strcmp("After SPI test  \r\n", outstring) || cb_arg != 0) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Test 2 - spiStartExchange with callback */
    chnWrite(&SD0, (const uint8_t *)test_2_msg, strlen(test_2_msg));
    strcpy(outstring, "After SPI test  \r\n");
    strcpy(instring, "Before SPI test \r\n");
    cb_arg = 1;
    if (strcmp("Before SPI test \r\n", instring) ||
        strcmp("After SPI test  \r\n", outstring) || cb_arg != 1) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    spiSelect(&SPIDA1);
    spiStartExchange(&SPIDA1, strlen(instring), outstring, instring);
    while (SPIDA1.state != SPI_READY)
      ; /* wait for transaction to finish */
    spiUnselect(&SPIDA1);
    if (strcmp("After SPI test  \r\n", instring) ||
        strcmp("After SPI test  \r\n", outstring) || cb_arg != 0) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Test 3 - spiStartSend with callback */
    chnWrite(&SD0, (const uint8_t *)test_3_msg, strlen(test_3_msg));
    strcpy(outstring, "After SPI test  \r\n");
    strcpy(instring, "Before SPI test \r\n");
    cb_arg = 1;
    if (strcmp("Before SPI test \r\n", instring) ||
        strcmp("After SPI test  \r\n", outstring) || cb_arg != 1) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    spiSelect(&SPIDA1);
    spiStartSend(&SPIDA1, strlen(outstring), outstring);
    while (SPIDA1.state != SPI_READY)
      ; /* wait for transaction to finish */
    spiUnselect(&SPIDA1);
    if (strcmp("Before SPI test \r\n", instring) ||
        strcmp("After SPI test  \r\n", outstring) || cb_arg != 0) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Test 4 - spiStartReceive with callback */
    chnWrite(&SD0, (const uint8_t *)test_4_msg, strlen(test_4_msg));
    strcpy(outstring, "After SPI test  \r\n");
    strcpy(instring, "Before SPI test \r\n");
    cb_arg = 1;
    if (strcmp("Before SPI test \r\n", instring) ||
        strcmp("After SPI test  \r\n", outstring) || cb_arg != 1) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    spiSelect(&SPIDA1);
    chThdSleepMilliseconds(2000);
    spiStartReceive(&SPIDA1, strlen(instring), instring);
    while (SPIDA1.state != SPI_READY)
      ; /* wait for transaction to finish */
    spiUnselect(&SPIDA1);
    if (strcmp("After SPI test  \r\n", outstring) ||
        strcmp("\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
               "\xff\xff\xff",
               instring) ||
        cb_arg != 0) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Test 5 - spiIgnore */
    chnWrite(&SD0, (const uint8_t *)test_5_msg, strlen(test_5_msg));
    strcpy(instring, "After SPI test  \r\n");
    strcpy(outstring, "Before SPI test \r\n");
    if (strcmp("Before SPI test \r\n", outstring) ||
        strcmp("After SPI test  \r\n", instring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    spiSelect(&SPIDB0);
    chThdSleepMilliseconds(2000);
    spiIgnore(&SPIDB0, strlen(outstring));
    spiUnselect(&SPIDB0);
    if (strcmp("After SPI test  \r\n", instring) ||
        strcmp("Before SPI test \r\n", outstring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Test 6 - spiExchange */
    chnWrite(&SD0, (const uint8_t *)test_6_msg, strlen(test_6_msg));
    strcpy(outstring, "After SPI test  \r\n");
    strcpy(instring, "Before SPI test \r\n");
    if (strcmp("Before SPI test \r\n", instring) ||
        strcmp("After SPI test  \r\n", outstring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    spiSelect(&SPIDB0);
    spiExchange(&SPIDB0, strlen(outstring), outstring, instring);
    spiUnselect(&SPIDB0);
    if (strcmp("After SPI test  \r\n", instring) ||
        strcmp("After SPI test  \r\n", outstring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Test 7 - spiSend */
    chnWrite(&SD0, (const uint8_t *)test_7_msg, strlen(test_7_msg));
    strcpy(outstring, "After SPI test  \r\n");
    strcpy(instring, "Before SPI test \r\n");
    if (strcmp("Before SPI test \r\n", instring) ||
        strcmp("After SPI test  \r\n", outstring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    spiSelect(&SPIDB0);
    spiSend(&SPIDB0, strlen(outstring), outstring);
    spiUnselect(&SPIDB0);
    if (strcmp("After SPI test  \r\n", outstring) ||
        strcmp("Before SPI test \r\n", instring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Test 8 - spiReceive */
    chnWrite(&SD0, (const uint8_t *)test_8_msg, strlen(test_8_msg));
    strcpy(outstring, "After SPI test  \r\n");
    strcpy(instring, "Before SPI test \r\n");
    if (strcmp("Before SPI test \r\n", instring) ||
        strcmp("After SPI test  \r\n", outstring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    spiSelect(&SPIDB0);
    spiReceive(&SPIDB0, strlen(instring), instring);
    spiUnselect(&SPIDB0);
    if (strcmp("After SPI test  \r\n", outstring) ||
        strcmp("\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f"
               "\x7f\x7f\x7f",
               instring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    
    /* Test 9 - spiPolledExchange */
    chnWrite(&SD0, (const uint8_t *)test_9_msg, strlen(test_9_msg));
    strcpy(outstring, "After SPI test  \r\n");
    strcpy(instring, "Before SPI test \r\n");
    if (strcmp("Before SPI test \r\n", instring) ||
        strcmp("After SPI test  \r\n", outstring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    spiSelect(&SPIDB0);
    outstring[0] = spiPolledExchange(&SPIDB0, instring[0]);
    spiUnselect(&SPIDB0);
    if (strcmp("Bfter SPI test  \r\n", outstring) ||
        strcmp("Before SPI test \r\n", instring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Reconfigure SPIDA1 to use exclusive DMA for both */
    spiStop(&SPIDA1);
    SPIDA1_config.dmatx_index = 0;
    SPIDA1_config.dmarx_index = 1;
    SPIDA1_config.spi_mode    = 1; /* because why not get coverage */
    spiStart(&SPIDA1, &SPIDA1_config);

    /* Test 10 - spiStartExchange with exclusive DMA */
    chnWrite(&SD0, (const uint8_t *)test_10_msg, strlen(test_10_msg));
    strcpy(outstring, "After SPI test  \r\n");
    strcpy(instring, "Before SPI test \r\n");
    cb_arg = 1;
    if (strcmp("Before SPI test \r\n", instring) ||
        strcmp("After SPI test  \r\n", outstring) || cb_arg != 1) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    spiSelect(&SPIDA1);
    spiStartExchange(&SPIDA1, strlen(outstring), outstring, instring);
    while (SPIDA1.state != SPI_READY)
      ; /* wait for transaction to finish */
    spiUnselect(&SPIDA1);
    if (strcmp("After SPI test  \r\n", instring) ||
        strcmp("After SPI test  \r\n", outstring) || cb_arg != 0) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Reconfigure SPIDA1 to use exclusive DMA for TX only */
    spiStop(&SPIDA1);
    SPIDA1_config.dmatx_index = 0;
    SPIDA1_config.dmarx_index = 0xFFU;
    SPIDA1_config.spi_mode    = 2; /* because why not get coverage */
    spiStart(&SPIDA1, &SPIDA1_config);

    /* Test 11 - spiStartExchange with exclusive DMA for TX */
    chnWrite(&SD0, (const uint8_t *)test_11_msg, strlen(test_11_msg));
    strcpy(outstring, "After SPI test  \r\n");
    strcpy(instring, "Before SPI test \r\n");
    cb_arg = 1;
    if (strcmp("Before SPI test \r\n", instring) ||
        strcmp("After SPI test  \r\n", outstring) || cb_arg != 1) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    spiSelect(&SPIDA1);
    spiStartExchange(&SPIDA1, strlen(outstring), outstring, instring);
    while (SPIDA1.state != SPI_READY)
      ; /* wait for transaction to finish */
    spiUnselect(&SPIDA1);
    if (strcmp("After SPI test  \r\n", instring) ||
        strcmp("After SPI test  \r\n", outstring) || cb_arg != 0) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Reconfigure SPIDA1 to use exclusive DMA for TX only */
    spiStop(&SPIDA1);
    SPIDA1_config.dmatx_index = 0xFFU;
    SPIDA1_config.dmarx_index = 1;
    SPIDA1_config.spi_mode    = 3; /* because why not get coverage */
    spiStart(&SPIDA1, &SPIDA1_config);

    /* Test 12 - spiStartExchange with exclusive DMA for RX */
    chnWrite(&SD0, (const uint8_t *)test_12_msg, strlen(test_12_msg));
    strcpy(outstring, "After SPI test  \r\n");
    strcpy(instring, "Before SPI test \r\n");
    cb_arg = 1;
    if (strcmp("Before SPI test \r\n", instring) ||
        strcmp("After SPI test  \r\n", outstring) || cb_arg != 1) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    spiSelect(&SPIDA1);
    spiStartExchange(&SPIDA1, strlen(outstring), outstring, instring);
    while (SPIDA1.state != SPI_READY)
      ; /* wait for transaction to finish */
    spiUnselect(&SPIDA1);
    if (strcmp("After SPI test  \r\n", instring) ||
        strcmp("After SPI test  \r\n", outstring) || cb_arg != 0) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
  }
}

/*
 * Threads static table, one entry per thread. The number of entries must
 * match NIL_CFG_NUM_THREADS.
 */
THD_TABLE_BEGIN
  THD_TABLE_ENTRY(waThread1, "spi_test", Thread1, NULL)
THD_TABLE_END

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
  WDTCTL = WDTPW | WDTHOLD;

  halInit();
  chSysInit();
  dmaInit();

  /* This is now the idle thread loop, you may perform here a low priority
     task but you must never try to sleep or wait in this loop. Note that
     this tasks runs at the lowest priority level so any instruction added
     here will be executed after all other tasks have been started.*/
  while (true) {
  }
}
