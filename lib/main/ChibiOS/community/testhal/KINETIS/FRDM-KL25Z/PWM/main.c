/*
 * (c) 2015 flabbergast <s3+flabbergast@sdfeu.org>
 * Based on ChibiOS 3.0.1 demo code, license below.
 * Licensed under the Apache License, Version 2.0.
 */

/*
 *  ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "ch.h"
#include "hal.h"

/*
 * on FRDM-KL25Z:
 *   red LED on PTB18/TPM2_CH0 (AF3)
 *   green LED on PTB19/TPM2_CH1 (AF3)
 */

#define PWM_DRIVER PWMD3

/* PWM config structure */
/* Note: the PWM clock frequency must be so that
 *   SYSCLK / FREQ is a power of 2 between 1 and 128.
 */
static const PWMConfig pwmcfg = {
  750000,                                /* 750kHz PWM clock frequency.  */
  1000,                                  /* PWM period is 1000 cycles.  */
                                         /* meaning PWM resolution is 750 */
  NULL,                                  /* no callback */
  {
   {PWM_OUTPUT_ACTIVE_LOW, NULL},        /* ch0: mode, no callback */
   {PWM_OUTPUT_ACTIVE_LOW, NULL},        /* ch1: mode, no callback */
   {PWM_OUTPUT_DISABLED, NULL},          /* ch2: mode, no callback */
   {PWM_OUTPUT_DISABLED, NULL},          /* ch3: mode, no callback */
   {PWM_OUTPUT_DISABLED, NULL},          /* ch4: mode, no callback */
   {PWM_OUTPUT_DISABLED, NULL}           /* ch5: mode, no callback */
  },
};

#define BREATHE_STEP 16 /* ms; = 4000ms/TABLE_SIZE */

/* Breathing Sleep LED brighness(PWM On period) table
 *
 * http://www.wolframalpha.com/input/?i=%28sin%28+x%2F64*pi%29**8+*+255%2C+x%3D0+to+63
 * (0..63).each {|x| p ((sin(x/64.0*PI)**8)*255).to_i }
 */
/* ruby -e "a = ((0..255).map{|x| Math.exp(Math.cos(Math::PI+(2*x*(Math::PI)/255)))-Math.exp(-1) }); m = a.max; a.map\!{|x| (10000*x/m).to_i}; p a" */
#define TABLE_SIZE 256
static const uint16_t breathing_table[TABLE_SIZE] = {
  0, 0, 1, 4, 7, 11, 17, 23, 30, 38, 47, 58, 69, 81, 94, 109, 124, 141, 159, 177, 197, 218, 241, 264, 289, 315, 343, 372, 402, 433, 466, 501, 537, 574, 613, 654, 696, 741, 786, 834, 883, 935, 988, 1043, 1100, 1159, 1220, 1283, 1349, 1416, 1486, 1558, 1632, 1709, 1788, 1870, 1954, 2040, 2129, 2220, 2314, 2411, 2510, 2611, 2715, 2822, 2932, 3044, 3158, 3275, 3395, 3517, 3641, 3768, 3897, 4028, 4162, 4298, 4436, 4576, 4717, 4861, 5006, 5152, 5300, 5449, 5600, 5751, 5903, 6055, 6208, 6361, 6513, 6666, 6818, 6970, 7120, 7269, 7417, 7563, 7708, 7850, 7990, 8127, 8261, 8391, 8519, 8643, 8762, 8878, 8989, 9095, 9196, 9293, 9383, 9469, 9548, 9622, 9689, 9750, 9805, 9853, 9895, 9930, 9957, 9978, 9992, 9999, 10000, 9992, 9978, 9957, 9930, 9895, 9853, 9805, 9750, 9689, 9622, 9548, 9469, 9383, 9293, 9196, 9095, 8989, 8878, 8762, 8643, 8519, 8391, 8261, 8127, 7990, 7850, 7708, 7563, 7417, 7269, 7120, 6970, 6818, 6666, 6513, 6361, 6208, 6055, 5903, 5751, 5600, 5449, 5300, 5152, 5006, 4861, 4717, 4576, 4436, 4298, 4162, 4028, 3897, 3768, 3641, 3517, 3395, 3275, 3158, 3044, 2932, 2822, 2715, 2611, 2510, 2411, 2314, 2220, 2129, 2040, 1954, 1870, 1788, 1709, 1632, 1558, 1486, 1416, 1349, 1283, 1220, 1159, 1100, 1043, 988, 935, 883, 834, 786, 741, 696, 654, 613, 574, 537, 501, 466, 433, 402, 372, 343, 315, 289, 264, 241, 218, 197, 177, 159, 141, 124, 109, 94, 81, 69, 58, 47, 38, 30, 23, 17, 11, 7, 4, 1, 0, 0
};

uint16_t table_pos = 0;
uint8_t active_led = 0;

static THD_WORKING_AREA(waBreatheThread, 128);
static THD_FUNCTION(BreatheThread, arg) {
  (void)arg;
  chRegSetThreadName("breatheThread");

  while(true) {
    pwmEnableChannel(&PWM_DRIVER, active_led, PWM_PERCENTAGE_TO_WIDTH(&PWM_DRIVER,breathing_table[table_pos]));
    table_pos++;
    if(table_pos == TABLE_SIZE) {
      table_pos = 0;
      active_led = (active_led+1) % 2;
    }
    chThdSleepMilliseconds(BREATHE_STEP);
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
   * Turn off the RGB LED.
   */
  palSetLine(LINE_LED_RED); /* red */
  palSetLine(LINE_LED_GREEN); /* green */
  palSetLine(LINE_LED_BLUE); /* blue */

  /*
   * Start the PWM driver, route TPM2 output to PTB18, PTB19.
   * Enable channels now to avoid a blink later.
   */
  pwmStart(&PWM_DRIVER, &pwmcfg);
  palSetLineMode(LINE_LED_RED, PAL_MODE_ALTERNATIVE_3);
  palSetLineMode(LINE_LED_GREEN, PAL_MODE_ALTERNATIVE_3);
  pwmEnableChannel(&PWM_DRIVER, 0, 0);
  pwmEnableChannel(&PWM_DRIVER, 1, 0);

  /*
   * Create the breathe thread.
   */
  chThdCreateStatic(waBreatheThread, sizeof(waBreatheThread), NORMALPRIO, BreatheThread, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop.
   */
  while(true) {
    chThdSleepMilliseconds(500);
  }
}
