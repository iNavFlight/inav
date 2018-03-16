/*
    (C) 2015-2016 flabbergast <s3+flabbergast@sdfeu.org>

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
#include "hal.h"

#define PWM_DRIVER PWMD1

static void pwmpcb(PWMDriver *pwmp) {
  (void)pwmp;
  palSetPad(TEENSY_PIN13_IOPORT, TEENSY_PIN13);
}

static void pwmc0cb(PWMDriver *pwmp) {
  (void)pwmp;
  palClearPad(TEENSY_PIN13_IOPORT, TEENSY_PIN13);
}

static PWMConfig pwmcfg = {
  24000000,           /* 24MHz PWM clock frequency.   */
  12000,              /* Initial PWM period 1ms     */
  pwmpcb,
  {
    {PWM_OUTPUT_DISABLED, pwmc0cb},
    {PWM_OUTPUT_DISABLED, NULL},
  },
};

/* Breathing Sleep LED brighness(PWM On period) table
 *
 * http://www.wolframalpha.com/input/?i=%28sin%28+x%2F64*pi%29**8+*+255%2C+x%3D0+to+63
 * (0..63).each {|x| p ((sin(x/64.0*PI)**8)*255).to_i }
 */
/* ruby -e "a = ((0..255).map{|x| Math.exp(Math.cos(Math::PI+(2*x*(Math::PI)/255)))-Math.exp(-1) }); m = a.max; a.map\!{|x| (9900*x/m).to_i+1}; p a" */
#define BREATHE_STEP 16 /* ms; = 4000ms/TABLE_SIZE */
#define TABLE_SIZE 256
static const uint16_t breathing_table[TABLE_SIZE] = {
  1, 1, 2, 5, 8, 12, 17, 24, 31, 39, 48, 58, 69, 81, 95, 109, 124, 140, 158, 177, 196, 217, 239, 263, 287, 313, 340, 369, 399, 430, 463, 497, 532, 570, 608, 649, 691, 734, 779, 827, 875, 926, 979, 1033, 1090, 1148, 1209, 1271, 1336, 1403, 1472, 1543, 1617, 1693, 1771, 1852, 1935, 2021, 2109, 2199, 2292, 2387, 2486, 2586, 2689, 2795, 2903, 3014, 3127, 3243, 3362, 3482, 3606, 3731, 3859, 3989, 4122, 4256, 4392, 4531, 4671, 4813, 4957, 5102, 5248, 5396, 5545, 5694, 5845, 5995, 6147, 6298, 6449, 6600, 6751, 6901, 7050, 7198, 7344, 7489, 7632, 7772, 7911, 8046, 8179, 8309, 8435, 8557, 8676, 8790, 8900, 9005, 9105, 9201, 9291, 9375, 9454, 9527, 9593, 9654, 9708, 9756, 9797, 9831, 9859, 9880, 9894, 9900, 9901, 9894, 9880, 9859, 9831, 9797, 9756, 9708, 9654, 9593, 9527, 9454, 9375, 9291, 9201, 9105, 9005, 8900, 8790, 8676, 8557, 8435, 8309, 8179, 8046, 7911, 7772, 7632, 7489, 7344, 7198, 7050, 6901, 6751, 6600, 6449, 6298, 6147, 5995, 5845, 5694, 5545, 5396, 5248, 5102, 4957, 4813, 4671, 4531, 4392, 4256, 4122, 3989, 3859, 3731, 3606, 3482, 3362, 3243, 3127, 3014, 2903, 2795, 2689, 2586, 2486, 2387, 2292, 2199, 2109, 2021, 1935, 1852, 1771, 1693, 1617, 1543, 1472, 1403, 1336, 1271, 1209, 1148, 1090, 1033, 979, 926, 875, 827, 779, 734, 691, 649, 608, 570, 532, 497, 463, 430, 399, 369, 340, 313, 287, 263, 239, 217, 196, 177, 158, 140, 124, 109, 95, 81, 69, 58, 48, 39, 31, 24, 17, 12, 8, 5, 2, 1, 1
};

uint16_t table_pos = 0;

static THD_WORKING_AREA(waBreatheThread, 128);
static THD_FUNCTION(BreatheThread, arg) {
  (void)arg;
  chRegSetThreadName("breatheThread");

  while(!chThdShouldTerminateX()) {
    pwmEnableChannel(&PWM_DRIVER, 0, PWM_PERCENTAGE_TO_WIDTH(&PWM_DRIVER,breathing_table[table_pos]));
    table_pos = (table_pos+1) % TABLE_SIZE;
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
   * Initialize the PWM driver.
   */
  pwmStart(&PWM_DRIVER, &pwmcfg);
  pwmEnablePeriodicNotification(&PWM_DRIVER);

  /*
   * Starts the PWM channel 0; turn the LED off.
   */
  pwmEnableChannel(&PWM_DRIVER, 0, PWM_PERCENTAGE_TO_WIDTH(&PWM_DRIVER, 0));
  pwmEnableChannelNotification(&PWM_DRIVER, 0); // MUST be before EnableChannel...
  
  /*
   * Create the breathe thread.
   */
  thread_t *breathe_thread_p;
  breathe_thread_p = chThdCreateStatic(waBreatheThread, sizeof(waBreatheThread), NORMALPRIO, BreatheThread, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing.
   */
  while (!chThdShouldTerminateX()) {
    chThdSleepMilliseconds(500);
  }

  chThdTerminate(breathe_thread_p);
  chThdSleepMilliseconds(2*BREATHE_STEP);

  /*
   * Disables channel 0 and stops the drivers.
   */
  pwmDisableChannel(&PWM_DRIVER, 0);
  pwmStop(&PWM_DRIVER);

  return 0;
}
