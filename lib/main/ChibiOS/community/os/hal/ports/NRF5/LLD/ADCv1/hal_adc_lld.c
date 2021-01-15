/*
    Copyright (C) 2015 Stephen Caudle

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
 * @file    ADCv1/adc_lld.c
 * @brief   NRF51 ADC subsystem low level driver source.
 *
 * @addtogroup ADC
 * @{
 */

#include "hal.h"

#if HAL_USE_ADC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/
#define ADC_CHANNEL_MASK                    0x7

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief ADC1 driver identifier.*/
#if NRF5_ADC_USE_ADC1 || defined(__DOXYGEN__)
ADCDriver ADCD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static void adc_lld_config_next_channel(ADCDriver *adcp, uint32_t config) {

  /* Default to all analog input pins disabled */
  config &= ~ADC_CONFIG_PSEL_Msk;

  if (adcp->grpp->channel_mask) {
    /* Skip to the next channel */
    while (((1 << adcp->current_channel) & adcp->grpp->channel_mask) == 0)
      adcp->current_channel = (adcp->current_channel + 1) & ADC_CHANNEL_MASK;
    config |= (((1 << adcp->current_channel) << ADC_CONFIG_PSEL_Pos) & ADC_CONFIG_PSEL_Msk);
  }

  /* Setup analog input pin select and user config values */
  adcp->adc->CONFIG = config;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if NRF5_ADC_USE_ADC1 || defined(__DOXYGEN__)
/**
 * @brief   ADC interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector5C) {

  ADCDriver *adcp = &ADCD1;
  NRF_ADC_Type *adc = adcp->adc;
  bool more = true;

  OSAL_IRQ_PROLOGUE();

  /* Clear the ADC event */
  adc->EVENTS_END = 0;

  /* Read the sample into the buffer */
  adcp->samples[adcp->current_index++] = adc->RESULT;

  /*  At the end of the buffer then we may be finished */
  if (adcp->current_index == adcp->number_of_samples) {
    _adc_isr_full_code(adcp);

    adcp->current_index = 0;

    /* We are never finished in circular mode */
    more = adcp->grpp->circular;
  }

  if (more) {

    /* Signal half completion in circular mode. */
    if (adcp->grpp->circular &&
        (adcp->current_index == (adcp->number_of_samples / 2))) {

      _adc_isr_half_code(adcp);
    }

    /* Skip to the next channel */
    adcp->current_channel = (adcp->current_channel + 1) & ADC_CHANNEL_MASK;
    adc_lld_config_next_channel(adcp, adcp->adc->CONFIG);
    adcp->adc->TASKS_START = 1;
  } else {
    adc_lld_stop_conversion(adcp);
  }

  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level ADC driver initialization.
 *
 * @notapi
 */
void adc_lld_init(void) {

#if NRF5_ADC_USE_ADC1
  /* Driver initialization.*/
  adcObjectInit(&ADCD1);
  ADCD1.adc = NRF_ADC;
#endif
}

/**
 * @brief   Configures and activates the ADC peripheral.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_start(ADCDriver *adcp) {

  /* If in stopped state then configures and enables the ADC. */
  if (adcp->state == ADC_STOP) {
#if NRF5_ADC_USE_ADC1
    if (&ADCD1 == adcp) {

      adcp->adc->INTENSET = ADC_INTENSET_END_Enabled << ADC_INTENSET_END_Pos;
      nvicEnableVector(ADC_IRQn, NRF5_ADC_IRQ_PRIORITY);
    }
#endif /* NRF5_ADC_USE_ADC1 */
  }
}

/**
 * @brief   Deactivates the ADC peripheral.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_stop(ADCDriver *adcp) {

  /* If in ready state then disables the ADC clock and analog part.*/
  if (adcp->state == ADC_READY) {

#if NRF5_ADC_USE_ADC1
    if (&ADCD1 == adcp) {

      nvicDisableVector(ADC_IRQn);
      adcp->adc->INTENCLR = ADC_INTENCLR_END_Clear << ADC_INTENCLR_END_Pos;
      adc_lld_stop_conversion(adcp);
    }
#endif
  }
}

/**
 * @brief   Starts an ADC conversion.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_start_conversion(ADCDriver *adcp) {

  NRF_ADC_Type *adc = adcp->adc;

  adcp->number_of_samples = adcp->depth * adcp->grpp->num_channels;
  adcp->current_index = 0;

  /* At least one sample must be configured */
  osalDbgAssert(adcp->number_of_samples, "must configure at least one sample");

  /* Skip to the next channel */
  adcp->current_channel = 0;
  adc_lld_config_next_channel(adcp, adcp->grpp->cfg);

  /* Enable and start the conversion */
  adc->ENABLE = ADC_ENABLE_ENABLE_Enabled << ADC_ENABLE_ENABLE_Pos;
  adc->TASKS_START = 1;
}

/**
 * @brief   Stops an ongoing conversion.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_stop_conversion(ADCDriver *adcp) {

  NRF_ADC_Type *adc = adcp->adc;

  adc->TASKS_STOP = 1;
  adc->ENABLE = ADC_ENABLE_ENABLE_Disabled << ADC_ENABLE_ENABLE_Pos;
}

#endif /* HAL_USE_ADC */

/** @} */
