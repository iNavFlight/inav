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


static uint8_t TIM3CC1CaptureNumber, TIM3CC2CaptureNumber;
static uint16_t TIM3CC1ReadValue1, TIM3CC1ReadValue2;
static uint16_t TIM3CC2ReadValue1, TIM3CC2ReadValue2;
static bool TIM3CC1UD, TIM3CC2UD;

static uint16_t freq1, freq2;


void reEnableInputCapture(TIMCAPDriver *timcapp)
{

  if ((timcapp->tim->DIER & TIM_DIER_CC1IE) == 0)
  {
    TIM3CC1CaptureNumber = 0;
    TIM3CC1UD = false;
    timcapp->tim->DIER |= TIM_DIER_CC1IE;
  }

  if ((timcapp->tim->DIER & TIM_DIER_CC2IE) == 0)
  {
    TIM3CC2CaptureNumber = 0;
    TIM3CC2UD = false;
    timcapp->tim->DIER |= TIM_DIER_CC2IE;
  }

}

void captureOverflowCb(TIMCAPDriver *timcapp)
{
  if (TIM3CC1UD && (timcapp->tim->DIER & TIM_DIER_CC1IE))
  {
    timcapp->tim->DIER &= ~TIM_DIER_CC1IE;
    freq1 = 0;
  }

  if (TIM3CC2UD && (timcapp->tim->DIER & TIM_DIER_CC2IE))
  {
    timcapp->tim->DIER &= ~TIM_DIER_CC2IE;
    freq2 = 0;
  }

  TIM3CC1UD = true;
  TIM3CC2UD = true;
}

void capture1Cb(TIMCAPDriver *timcapp)
{
  if(TIM3CC1CaptureNumber == 0)
  {
    /* Get the Input Capture value */
    TIM3CC1ReadValue1 = timcapp->tim->CCR[0];
    TIM3CC1CaptureNumber = 1;
    TIM3CC1UD = false;
  }
  else if(TIM3CC1CaptureNumber == 1)
  {
      uint32_t Capture;
      /* Get the Input Capture value */
      TIM3CC1ReadValue2 = timcapp->tim->CCR[0];
      TIM3CC1UD = false;

      /* Capture computation */
      if (TIM3CC1ReadValue2 > TIM3CC1ReadValue1)
      {
          Capture = ((uint32_t)TIM3CC1ReadValue2 - (uint32_t)TIM3CC1ReadValue1);
      }
      else
      {
          Capture = (((uint32_t)TIM3CC1ReadValue2 + 0x10000) - (uint32_t)TIM3CC1ReadValue1);
      }

      /* Frequency computation */
      freq1 = (timcapp->config->frequency / Capture);

      TIM3CC1ReadValue1 = TIM3CC1ReadValue2;
      TIM3CC1CaptureNumber = 0;

      /* Disable CC1 interrupt */
      timcapp->tim->DIER &= ~TIM_DIER_CC1IE;
  }
}

void capture2Cb(TIMCAPDriver *timcapp)
{
  if(TIM3CC2CaptureNumber == 0)
  {
    /* Get the Input Capture value */
    TIM3CC2ReadValue1 = timcapp->tim->CCR[1];
    TIM3CC2CaptureNumber = 1;
    TIM3CC2UD = false;
  }
  else if(TIM3CC2CaptureNumber == 1)
  {
      uint32_t Capture;
      /* Get the Input Capture value */
      TIM3CC2ReadValue2 = timcapp->tim->CCR[1];
      TIM3CC2UD = false;

      /* Capture computation */
      if (TIM3CC2ReadValue2 > TIM3CC2ReadValue1)
      {
          Capture = ((uint32_t)TIM3CC2ReadValue2 - (uint32_t)TIM3CC2ReadValue1);
      }
      else
      {
          Capture = (((uint32_t)TIM3CC2ReadValue2 + 0x10000) - (uint32_t)TIM3CC2ReadValue1);
      }

      /* Frequency computation */
      freq2 = (timcapp->config->frequency / Capture);

      TIM3CC2ReadValue1 = TIM3CC2ReadValue2;
      TIM3CC2CaptureNumber = 0;

      /* Disable CC2 interrupt */
      timcapp->tim->DIER &= ~TIM_DIER_CC2IE;
  }
}

TIMCAPConfig tc_conf = {
   {TIMCAP_INPUT_ACTIVE_HIGH,
    TIMCAP_INPUT_ACTIVE_HIGH,
    TIMCAP_INPUT_DISABLED,
    TIMCAP_INPUT_DISABLED},
   200000, /* TIM3 Runs at 36Mhz max. (1/200000)*65536 = 0.32s Max, 3.12Hz Min */
   {capture1Cb, capture2Cb, NULL, NULL},
   captureOverflowCb,
   0,
   0
};

THD_WORKING_AREA(waThreadTimcap, 256);
static THD_FUNCTION(ThreadTimcap, arg)
{
  (void)arg;

  while (TRUE)
  {

    reEnableInputCapture(&TIMCAPD3);
    chThdSleepMilliseconds(200);
  }

  return;
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

  timcapStart(&TIMCAPD3, &tc_conf);

  chThdCreateStatic(waThreadTimcap, sizeof(waThreadTimcap), NORMALPRIO, ThreadTimcap, NULL);

  /*
   * Normal main() thread activity, it resets the watchdog.
   */
  while (true) {
    palToggleLine(LINE_LED4_BLUE);
    chThdSleepMilliseconds(500);
  }
  return 0;
}
