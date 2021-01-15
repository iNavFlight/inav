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
#include "string.h"
#include "stdio.h" /* eesh */

/* Disable watchdog because of lousy startup code in newlib */
static void __attribute__((naked, section(".crt_0042disable_watchdog"), used))
disable_watchdog(void) {
  WDTCTL = WDTPW | WDTHOLD;
}

const char * start_msg  = "\r\n\r\nExecuting ADC test suite...\r\n";
const char * test_1_msg = "\r\nTEST 1: 1 channel, depth 1, no circular\r\n";
const char * test_2_msg = "\r\nTEST 2: 1 channel, depth 8, no circular\r\n";
const char * test_3_msg = "\r\nTEST 3: 4 channels, depth 1, no circular\r\n";
const char * test_4_msg = "\r\nTEST 4: 4 channels, depth 8, no circular\r\n";
const char * test_5_msg = "\r\nTEST 5: 1 channel, depth 1, circular\r\n";
const char * test_6_msg = "\r\nTEST 6: 1 channel, depth 8, circular\r\n";
const char * test_7_msg = "\r\nTEST 7: 4 channel, depth 1, circular\r\n";
const char * test_8_msg = "\r\nTEST 8: 4 channel, depth 8, circular\r\n";
const char * test_9_msg = "\r\nTEST 9: 1 channel, depth 1, synchronous\r\n";
const char * test_10_msg = "\r\nTEST 9: 1 channel, depth 1, exclusive\r\n";

const char * success_string = "\r\nSUCCESS\r\n";
const char * fail_string = "\r\nFAILURE\r\n";

char out_string[128];
const char * raw_fmt_string = "Raw Value: %d\r\n";
const char * cooked_fmt_string = "Cooked Value: %d\r\n";
const char * chn_fmt_string = "\r\nCHANNEL %d\r\n";

uint16_t buffer_margin[72];
uint16_t * buffer = buffer_margin + 4;
uint8_t depth;
uint8_t cb_arg = 0;
uint16_t cb_expect;

static const int test = 0;

ADCConfig config = {
  255 /* dma_index */
};

ADCConversionGroup group = {
  false, /* circular */
  1, /* num_channels */
  NULL, /* end_cb */
  NULL, /* error_cb */
  { 
    30, 31, 30, 31, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
  }, /* channels */
  MSP430X_ADC_RES_12BIT, /* res */
  MSP430X_ADC_SHT_32, /* rate */
  MSP430X_ADC_VSS_VREF_BUF, /* ref */
  MSP430X_REF_2V5 /* vref_src */
};

void print(const char * msg) {
    
  if (!test) {
    chnWrite(&SD0, (const uint8_t *)msg, strlen(msg));
  }
}

void adc_callback(ADCDriver * adcp, adcsample_t *buffer, size_t n) {
  (void)adcp;
  (void)buffer;
  (void)n;
  
  cb_arg++;
  
  if (adcp->grpp->circular && cb_arg == cb_expect) {
    osalSysLockFromISR();
    adcStopConversionI(adcp);
    osalSysUnlockFromISR();
  }
}

void run_test(const char * test_msg, uint8_t num_channels, uint8_t depth,
    bool circular) {
  print(test_msg);
  
  cb_arg = 0;
  
  group.num_channels = num_channels;
  group.circular = circular;
  group.end_cb = adc_callback;
  
  if (depth > 1) cb_expect = 2;
  else cb_expect = 1;
  if (circular) cb_expect *= 3;
  
  adcStartConversion(&ADCD1, &group, buffer, depth);
  
  while (ADCD1.state == ADC_ACTIVE) ;
  
  
  int index = 0;
  for (int j = 0; j < depth; j++) {
    for (int i = 0; i < group.num_channels; i++) {
      index = i + (j * group.num_channels);
      sniprintf(out_string, 128, chn_fmt_string, group.channels[i]);
      print(out_string);
      
      sniprintf(out_string, 128, raw_fmt_string, buffer[index]); 
      print(out_string);
      
      if (group.channels[i] == 30) { /* internal temp sensor */
        buffer[index] = adcMSP430XAdjustTemp(&group, buffer[index]);
      }
      else {
        buffer[index] = adcMSP430XAdjustResult(&group, buffer[index]);
      }
      
      sniprintf(out_string, 128, cooked_fmt_string, buffer[index]); 
      print(out_string);
    }
  }
  
  if (cb_arg == cb_expect) {
    print(success_string);
  }
  else {
    print(fail_string);
  }
}

/*
 * Thread 2.
 */
THD_WORKING_AREA(waThread1, 4096);
THD_FUNCTION(Thread1, arg) {

  (void)arg;

  /*
   * Activate the serial driver 0 using the driver default configuration.
   */
  sdStart(&SD0, NULL);


  while (chnGetTimeout(&SD0, TIME_INFINITE)) {
    print(start_msg);
    chThdSleepMilliseconds(2000);

    /* Activate the ADC driver 1 using its config */
    adcStart(&ADCD1, &config);
    
    /* Test 1 - 1ch1d, no circular */
    run_test(test_1_msg, 1, 1, false);
    
    /* Test 2 - 1ch8d, no circular */
    run_test(test_2_msg, 1, 8, false);
    
    /* Test 3 - 4chd1, no circular */
    run_test(test_3_msg, 4, 1, false);
    
    /* Test 4 - 4ch8d, no circular */
    run_test(test_4_msg, 4, 8, false);
    
    /* Test 5 - 1ch1d, circular */
    run_test(test_5_msg, 1, 1, true);
    
    /* Test 6 - 1ch8d, circular */
    run_test(test_6_msg, 1, 8, true);
    
    /* Test 7 - 4ch1d, circular */
    run_test(test_7_msg, 4, 1, true);
    
    /* Test 8 - 4ch8d, circular */
    run_test(test_8_msg, 4, 8, true);
    
    /* Test 9 - 1ch1d, synchronous */
    print(test_9_msg);
    cb_arg = 0;
    
    group.num_channels = 1;
    group.circular = false;
    group.end_cb = adc_callback;
    
    cb_expect = 1;
    
    adcConvert(&ADCD1, &group, buffer, 1);
    
    while (ADCD1.state == ADC_ACTIVE) ;
    
    sniprintf(out_string, 128, chn_fmt_string, group.channels[0]);
    print(out_string);
    
    sniprintf(out_string, 128, raw_fmt_string, buffer[0]); 
    print(out_string);
    
    buffer[0] = adcMSP430XAdjustTemp(&group, buffer[0]);
    
    sniprintf(out_string, 128, cooked_fmt_string, buffer[0]); 
    print(out_string);
    
    if (cb_arg == cb_expect) {
      print(success_string);
    }
    else {
      print(fail_string);
    }
    
    /* Test 10 - 1ch1d, exclusive */
    adcStop(&ADCD1);
    
    config.dma_index = 0;
    
    adcStart(&ADCD1, &config);
    
    run_test(test_10_msg, 1, 1, false);
    
    adcStop(&ADCD1);
    
    config.dma_index = 255;
  }
}

/*
 * Threads static table, one entry per thread. The number of entries must
 * match NIL_CFG_NUM_THREADS.
 */
THD_TABLE_BEGIN
  THD_TABLE_ENTRY(waThread1, "adc_test", Thread1, NULL)
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

  /* This is now the idle thread loop, you may perform here a low priority
     task but you must never try to sleep or wait in this loop. Note that
     this tasks runs at the lowest priority level so any instruction added
     here will be executed after all other tasks have been started.*/
  while (true) {
  }
}
