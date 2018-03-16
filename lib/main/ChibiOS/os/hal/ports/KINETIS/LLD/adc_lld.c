/*
    ChibiOS - Copyright (C) 2014 Derek Mulcahy

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
 * @file    KINETIS/LLD/adc_lld.c
 * @brief   KINETIS ADC subsystem low level driver source.
 *
 * @addtogroup ADC
 * @{
 */

#include "hal.h"

#if HAL_USE_ADC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define ADC_CHANNEL_MASK                    0x1f

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief ADC1 driver identifier.*/
#if KINETIS_ADC_USE_ADC0 || defined(__DOXYGEN__)
ADCDriver ADCD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static void calibrate(ADCDriver *adcp) {

  /* Clock Divide by 8, Use Bus Clock Div 2 */
  /* At 48MHz this results in ADCCLK of 48/8/2 == 3MHz */
  adcp->adc->CFG1 = ADCx_CFG1_ADIV(ADCx_CFG1_ADIV_DIV_8) |
      ADCx_CFG1_ADICLK(ADCx_CFG1_ADIVCLK_BUS_CLOCK_DIV_2);

  /* Use software trigger and disable DMA etc. */
  adcp->adc->SC2 = 0;

  /* Enable Hardware Average, Average 32 Samples, Calibrate */
  adcp->adc->SC3 = ADCx_SC3_AVGE |
      ADCx_SC3_AVGS(ADCx_SC3_AVGS_AVERAGE_32_SAMPLES) |
      ADCx_SC3_CAL;

  /* FIXME: May take several ms. Use an interrupt instead of busy wait */
  /* Wait for calibration completion */
  while (!(adcp->adc->SC1A & ADCx_SC1n_COCO))
    ;

  uint16_t gain = ((adcp->adc->CLP0 + adcp->adc->CLP1 + adcp->adc->CLP2 +
      adcp->adc->CLP3 + adcp->adc->CLP4 + adcp->adc->CLPS) / 2) | 0x8000;
  adcp->adc->PG = gain;

  gain = ((adcp->adc->CLM0 + adcp->adc->CLM1 + adcp->adc->CLM2 +
      adcp->adc->CLM3 + adcp->adc->CLM4 + adcp->adc->CLMS) / 2) | 0x8000;
  adcp->adc->MG = gain;

}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if KINETIS_ADC_USE_ADC0 || defined(__DOXYGEN__)
/**
 * @brief   ADC interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(KINETIS_ADC0_IRQ_VECTOR) {
  OSAL_IRQ_PROLOGUE();

  ADCDriver *adcp = &ADCD1;

  /* Disable Interrupt, Disable Channel */
  adcp->adc->SC1A = ADCx_SC1n_ADCH(ADCx_SC1n_ADCH_DISABLED);

  /* Read the sample into the buffer */
  adcp->samples[adcp->current_index++] = adcp->adc->RA;

  bool more = true;

  /*  At the end of the buffer then we may be finished */
  if (adcp->current_index == adcp->number_of_samples) {
    _adc_isr_full_code(&ADCD1);

    adcp->current_index = 0;

    /* We are never finished in circular mode */
    more = ADCD1.grpp->circular;
  }

  if (more) {

    /* Signal half completion in circular mode. */
    if (ADCD1.grpp->circular &&
        (adcp->current_index == (adcp->number_of_samples / 2))) {

        _adc_isr_half_code(&ADCD1);
    }

    /* Skip to the next channel */
    do {
      adcp->current_channel = (adcp->current_channel + 1) & ADC_CHANNEL_MASK;
    } while (((1 << adcp->current_channel) & adcp->grpp->channel_mask) == 0);

    /* Enable Interrupt, Select the Channel */
    adcp->adc->SC1A = ADCx_SC1n_AIEN | ADCx_SC1n_ADCH(adcp->current_channel);
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

#if KINETIS_ADC_USE_ADC0
  /* Driver initialization.*/
  adcObjectInit(&ADCD1);
#endif

  /* The shared vector is initialized on driver initialization and never
     disabled.*/
  nvicEnableVector(ADC0_IRQn, KINETIS_ADC_IRQ_PRIORITY);
}

/**
 * @brief   Configures and activates the ADC peripheral.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_start(ADCDriver *adcp) {

  /* If in stopped state then enables the ADC clock.*/
  if (adcp->state == ADC_STOP) {
    SIM->SCGC6 |= SIM_SCGC6_ADC0;

#if KINETIS_ADC_USE_ADC0
    if (&ADCD1 == adcp) {
      adcp->adc = ADC0;
      if (adcp->config->calibrate) {
        calibrate(adcp);
      }
    }
#endif /* KINETIS_ADC_USE_ADC0 */
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

  /* If in ready state then disables the ADC clock.*/
  if (adcp->state == ADC_READY) {
    SIM->SCGC6 &= ~SIM_SCGC6_ADC0;

#if KINETIS_ADC_USE_ADC0
    if (&ADCD1 == adcp) {
      /* Disable Interrupt, Disable Channel */
      adcp->adc->SC1A = ADCx_SC1n_ADCH(ADCx_SC1n_ADCH_DISABLED);
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
  const ADCConversionGroup *grpp = adcp->grpp;

  /* Enable the Bandgap Buffer if channel mask includes BANDGAP */
  if (grpp->channel_mask & ADC_BANDGAP) {
    PMC->REGSC |= PMC_REGSC_BGBE;
  }

  adcp->number_of_samples = adcp->depth * grpp->num_channels;
  adcp->current_index = 0;

  /* Skip to the next channel */
  adcp->current_channel = 0;
  while (((1 << adcp->current_channel) & grpp->channel_mask) == 0) {
    adcp->current_channel = (adcp->current_channel + 1) & ADC_CHANNEL_MASK;
  }

  /* Set clock speed and conversion size */
  adcp->adc->CFG1 = grpp->cfg1;

  /* Set averaging */
  adcp->adc->SC3 = grpp->sc3;

  /* Enable Interrupt, Select Channel */
  adcp->adc->SC1A = ADCx_SC1n_AIEN | ADCx_SC1n_ADCH(adcp->current_channel);
}

/**
 * @brief   Stops an ongoing conversion.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_stop_conversion(ADCDriver *adcp) {
  const ADCConversionGroup *grpp = adcp->grpp;

  /* Disable the Bandgap buffer if channel mask includes BANDGAP */
  if (grpp->channel_mask & ADC_BANDGAP) {
    /* Clear BGBE, ACKISO is w1c, avoid setting */
    PMC->REGSC &= ~(PMC_REGSC_BGBE | PMC_REGSC_ACKISO);
  }

}

#endif /* HAL_USE_ADC */

/** @} */
