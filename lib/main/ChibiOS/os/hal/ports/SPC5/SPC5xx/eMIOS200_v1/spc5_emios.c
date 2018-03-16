/*
    SPC5 HAL - Copyright (C) 2013 STMicroelectronics

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

/**
 * @file    SPC5xx/eMIOS200_v1/spc5_emios.c
 * @brief   eMIOS200 helper driver code.
 *
 * @addtogroup SPC5xx_eMIOS200
 * @{
 */

#include "hal.h"

#if HAL_USE_ICU || HAL_USE_PWM || defined(__DOXYGEN__)

#include "spc5_emios.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Number of active eMIOSx Channels.
 */
static uint32_t emios_active_channels;

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

void reset_emios_active_channels() {
  emios_active_channels = 0;
}

uint32_t get_emios_active_channels() {
  return emios_active_channels;
}

void increase_emios_active_channels() {
  emios_active_channels++;
}

void decrease_emios_active_channels() {
  emios_active_channels--;
}

#if HAL_USE_ICU
void icu_active_emios_clock(ICUDriver *icup) {
  /* If this is the first Channel activated then the eMIOS0 is enabled.*/
  if (emios_active_channels == 1) {
    SPC5_EMIOS_ENABLE_CLOCK();

    /* Disable all unified channels.*/
    icup->emiosp->MCR.B.GPREN = 0;
    icup->emiosp->MCR.R = EMIOSMCR_GPRE(SPC5_EMIOS_GPRE_VALUE - 1);
    icup->emiosp->MCR.R |= EMIOSMCR_GPREN;

    icup->emiosp->MCR.B.GTBE = 1U;

    icup->emiosp->UCDIS.R = 0xFFFFFFFF;

  }
}
#endif

#if HAL_USE_PWM
void pwm_active_emios_clock(PWMDriver *pwmp) {
  /* If this is the first Channel activated then the eMIOS0 is enabled.*/
  if (emios_active_channels == 1) {
    SPC5_EMIOS_ENABLE_CLOCK();

    /* Disable all unified channels.*/
    pwmp->emiosp->MCR.B.GPREN = 0;
    pwmp->emiosp->MCR.R = EMIOSMCR_GPRE(SPC5_EMIOS_GPRE_VALUE - 1);
    pwmp->emiosp->MCR.R |= EMIOSMCR_GPREN;

    pwmp->emiosp->MCR.B.GTBE = 1U;

    pwmp->emiosp->UCDIS.R = 0xFFFFFFFF;

  }
}
#endif

void deactive_emios_clock() {
  /* If it is the last active channels then the eMIOS0 is disabled.*/
  if (emios_active_channels == 0) {
    SPC5_EMIOS_DISABLE_CLOCK();

  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/


#endif /* HAL_USE_ICU || HAL_USE_PWM */

/** @} */
