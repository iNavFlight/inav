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

const char * start_msg = "\r\n\r\nExecuting DMA test suite...\r\n";
const char * test_1_msg =
    "TEST 1: Word-to-word memcpy with DMA engine, no callbacks\r\n";
const char * test_2_msg =
    "TEST 2: Byte-to-byte memcpy with DMA engine, no callbacks\r\n";
const char * test_3_msg =
    "TEST 3: Byte-to-byte memset with DMA engine, no callbacks\r\n";
const char * test_4_msg =
    "TEST 4: Word-to-word memcpy with DMA engine, with callback\r\n";
const char * test_5_msg =
    "TEST 5: Claim DMA channel 0, perform a Word-to-word memcpy\r\n";
const char * test_6_msg = "TEST 6: Attempt to claim already claimed DMA "
                          "channel, fail. Release it, try to claim it again, "
                          "and succeed.\r\n";
const char * test_7_msg = "TEST 7: Claim DMA channel 1, perform a Word-to-word "
                          "memcpy, and release it\r\n";
const char * test_8_msg = "TEST 8: Claim all three DMA channels, try to issue dmaRequest, "
                          "fail\r\n";

const char * succeed_string = "SUCCESS\r\n\r\n";
const char * fail_string    = "FAILURE\r\n\r\n";

char instring[256];
char outstring[256];
msp430x_dma_req_t * request;
uint8_t cb_arg = 1;
bool result;
int result_i;

void dma_callback_test(void * args) {

  *((uint8_t *)args) = 0;
}

msp430x_dma_req_t test_1_req = {
  instring,                                  /* source address */
  outstring,                                 /* destination address */
  9,                                         /* number of words */
  MSP430X_DMA_SRCINCR | MSP430X_DMA_DSTINCR, /* address mode - dual increment */
  MSP430X_DMA_SRCWORD | MSP430X_DMA_DSTWORD, /* word transfer */
  MSP430X_DMA_BLOCK,                         /* block (and blocking) transfer */
  DMA_TRIGGER_MNEM(DMAREQ),                  /* software-requested trigger */
  {
      NULL, /* no callback */
      NULL  /* no arguments */
  }
};

msp430x_dma_req_t test_2_req = {
  instring,                                  /* source address */
  outstring,                                 /* destination address */
  18,                                        /* number of bytes */
  MSP430X_DMA_SRCINCR | MSP430X_DMA_DSTINCR, /* address mode - dual increment */
  MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE, /* byte transfer */
  MSP430X_DMA_BLOCK,                         /* block (and blocking) transfer */
  DMA_TRIGGER_MNEM(DMAREQ),                  /* software-requested trigger */
  {
      NULL, /* no callback */
      NULL  /* no arguments */
  }
};

msp430x_dma_req_t test_3_req = {
  instring,            /* source address */
  outstring,           /* destination address */
  16,                  /* number of words */
  MSP430X_DMA_DSTINCR, /* address mode - dest increment only */
  MSP430X_DMA_SRCBYTE | MSP430X_DMA_DSTBYTE, /* word transfer */
  MSP430X_DMA_BLOCK,                         /* block (and blocking) transfer */
  DMA_TRIGGER_MNEM(DMAREQ),                  /* software-requested trigger */
  {
      NULL, /* no callback */
      NULL  /* no arguments */
  }
};

msp430x_dma_req_t test_4_req = {
  instring,                                  /* source address */
  outstring,                                 /* destination address */
  9,                                         /* number of words */
  MSP430X_DMA_SRCINCR | MSP430X_DMA_DSTINCR, /* address mode - dual increment */
  MSP430X_DMA_SRCWORD | MSP430X_DMA_DSTWORD, /* word transfer */
  MSP430X_DMA_BLOCK,                         /* block (and blocking) transfer */
  DMA_TRIGGER_MNEM(DMAREQ),                  /* software-requested trigger */
  {
      &dma_callback_test, /* test callback */
      &cb_arg             /* test arguments */
  }
};

msp430x_dma_req_t test_5_req = {
  instring,                                  /* source address */
  outstring,                                 /* destination address */
  9,                                         /* number of words */
  MSP430X_DMA_SRCINCR | MSP430X_DMA_DSTINCR, /* address mode - dual increment */
  MSP430X_DMA_SRCWORD | MSP430X_DMA_DSTWORD, /* word transfer */
  MSP430X_DMA_BLOCK,                         /* block (and blocking) transfer */
  DMA_TRIGGER_MNEM(DMAREQ),                  /* software-requested trigger */
  {
      NULL, /* no callback */
      NULL  /* no arguments */
  }
};

msp430x_dma_ch_t ch = { NULL, 0, NULL };
msp430x_dma_ch_t ch1 = { NULL, 0, NULL };
msp430x_dma_ch_t ch2 = { NULL, 0, NULL };

/*
 * Thread 2.
 */
THD_WORKING_AREA(waThread1, 2048);
THD_FUNCTION(Thread1, arg) {

  (void)arg;

  /*
   * Activate the serial driver 0 using the driver default configuration.
   */
  sdStart(&SD0, NULL);

  while (chnGetTimeout(&SD0, TIME_INFINITE)) {
    chnWrite(&SD0, (const uint8_t *)start_msg, strlen(start_msg));
    chThdSleepMilliseconds(2000);

    /* Test 1 - use DMA engine to execute a word-wise memory-to-memory copy. */
    chnWrite(&SD0, (const uint8_t *)test_1_msg, strlen(test_1_msg));
    strcpy(instring, "After DMA test  \r\n");
    strcpy(outstring, "Before DMA test \r\n");
    if (strcmp("Before DMA test \r\n", outstring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    request = &test_1_req;
    chSysLock();
    dmaRequestS(request, TIME_INFINITE);
    chSysUnlock();
    if (strcmp("After DMA test  \r\n", outstring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Test 2 - use DMA engine to execute a byte-wise memory-to-memory copy. */
    chnWrite(&SD0, (const uint8_t *)test_2_msg, strlen(test_2_msg));
    strcpy(instring, "After DMA test  \r\n");
    strcpy(outstring, "Before DMA test \r\n");
    if (strcmp("Before DMA test \r\n", outstring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    request = &test_2_req;
    chSysLock();
    dmaRequestS(request, TIME_INFINITE);
    chSysUnlock();
    if (strcmp("After DMA test  \r\n", outstring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Test 3 - use DMA engine to execute a word-wise memory-to-memory set. */
    chnWrite(&SD0, (const uint8_t *)test_3_msg, strlen(test_3_msg));
    strcpy(instring, "After DMA test  \r\n");
    strcpy(outstring, "Before DMA test \r\n");
    if (strcmp("Before DMA test \r\n", outstring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    request = &test_3_req;
    chSysLock();
    dmaRequestS(request, TIME_INFINITE);
    chSysUnlock();
    if (strcmp("AAAAAAAAAAAAAAAA\r\n", outstring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Test 4 - use DMA engine to execute a word-wise memory-to-memory copy,
     * then call a callback. */
    chnWrite(&SD0, (const uint8_t *)test_4_msg, strlen(test_4_msg));
    strcpy(instring, "After DMA test  \r\n");
    strcpy(outstring, "Before DMA test \r\n");
    cb_arg = 1;
    if (strcmp("Before DMA test \r\n", outstring) || (cb_arg != 1)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    request = &test_4_req;
    chSysLock();
    dmaRequestS(request, TIME_INFINITE);
    chSysUnlock();
    if (strcmp("After DMA test  \r\n", outstring) || cb_arg) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Test 5 - use exclusive DMA channel 0 to execute a word-wise
     * memory-to-memory copy. */
    chnWrite(&SD0, (const uint8_t *)test_5_msg, strlen(test_5_msg));
    strcpy(instring, "After DMA test  \r\n");
    strcpy(outstring, "Before DMA test \r\n");
    if (strcmp("Before DMA test \r\n", outstring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    request = &test_5_req;
    chSysLock();
    dmaAcquireI(&ch, 0);
    chSysUnlock();
    dmaTransfer(&ch, request);
    if (strcmp("After DMA test  \r\n", outstring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }

    /* Test 6 - Attempt to claim DMA channel 0, fail, release it, attempt to
     * claim it again */
    chnWrite(&SD0, (const uint8_t *)test_6_msg, strlen(test_6_msg));
    chSysLock();
    result = dmaAcquireI(&ch, 0);
    chSysUnlock();
    if (!result) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    dmaRelease(&ch);
    chSysLock();
    result = dmaAcquireI(&ch, 0);
    chSysUnlock();
    if (result) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    dmaRelease(&ch);

    /* Test 7 - use exclusive DMA channel 1 to execute a word-wise
     * memory-to-memory copy. */
    chnWrite(&SD0, (const uint8_t *)test_7_msg, strlen(test_7_msg));
    strcpy(instring, "After DMA test  \r\n");
    strcpy(outstring, "Before DMA test \r\n");
    if (strcmp("Before DMA test \r\n", outstring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    request = &test_5_req;
    chSysLock();
    dmaAcquireI(&ch, 1);
    chSysUnlock();
    dmaTransfer(&ch, request);
    if (strcmp("After DMA test  \r\n", outstring)) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    dmaRelease(&ch);
    
    /* Test 8 - Claim all 3 DMA channels, attempt dmaRequest, fail */
    chnWrite(&SD0, (const uint8_t *)test_8_msg, strlen(test_8_msg));
    chSysLock();
    result = dmaAcquireI(&ch, 0);
    chSysUnlock();
    if (result) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    chSysLock();
    result = dmaAcquireI(&ch1, 1);
    chSysUnlock();
    if (result) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    chSysLock();
    result = dmaAcquireI(&ch2, 2);
    chSysUnlock();
    if (result) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    chSysLock();
    result_i = dmaRequestS(request, TIME_IMMEDIATE);
    chSysUnlock();
    if (result_i > 0) {
      chnWrite(&SD0, (const uint8_t *)fail_string, strlen(fail_string));
    }
    else {
      chnWrite(&SD0, (const uint8_t *)succeed_string, strlen(succeed_string));
    }
    dmaRelease(&ch);
    dmaRelease(&ch1);
    dmaRelease(&ch2);

  }
}

/*
 * Threads static table, one entry per thread. The number of entries must
 * match NIL_CFG_NUM_THREADS.
 */
THD_TABLE_BEGIN
  THD_TABLE_ENTRY(waThread1, "dma_test", Thread1, NULL)
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
