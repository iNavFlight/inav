/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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
 * @file    ADCv4/hal_adc_lld.c
 * @brief   STM32 ADC subsystem low level driver source.
 *
 * @addtogroup ADC
 * @{
 */

#include "hal.h"

#if HAL_USE_ADC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#if STM32_ADC_DUAL_MODE == TRUE
#if STM32_ADC_COMPACT_SAMPLES == TRUE
/* Compact type dual mode.*/
#define ADC_DMA_SIZE    (STM32_DMA_CR_MSIZE_HWORD | STM32_DMA_CR_PSIZE_HWORD)
#define ADC_DMA_DAMDF   ADC_CCR_DAMDF_HWORD

#else /* STM32_ADC_COMPACT_SAMPLES == FALSE */
/* Large type dual mode.*/
#define ADC_DMA_SIZE    (STM32_DMA_CR_MSIZE_WORD | STM32_DMA_CR_PSIZE_WORD)
#define ADC_DMA_DAMDF   ADC_CCR_DAMDF_WORD
#endif /* !STM32_ADC_COMPACT_SAMPLES */

#else /* STM32_ADC_DUAL_MODE == FALSE */
#if STM32_ADC_COMPACT_SAMPLES
/* Compact type single mode.*/
#define ADC_DMA_SIZE    (STM32_DMA_CR_MSIZE_BYTE | STM32_DMA_CR_PSIZE_BYTE)
#define ADC_DMA_DAMDF   ADC_CCR_DAMDF_DISABLED

#else /* STM32_ADC_COMPACT_SAMPLES == FALSE */
/* Large type single mode.*/
#define ADC_DMA_SIZE    (STM32_DMA_CR_MSIZE_HWORD | STM32_DMA_CR_PSIZE_HWORD)
#define ADC_DMA_DAMDF   ADC_CCR_DAMDF_DISABLED
#endif /* STM32_ADC_COMPACT_SAMPLES == FALSE */
#endif /* STM32_ADC_DUAL_MODE == FALSE */

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief ADC1 driver identifier.*/
#if STM32_ADC_USE_ADC12 || defined(__DOXYGEN__)
ADCDriver ADCD1;
#endif

/** @brief ADC3 driver identifier.*/
#if STM32_ADC_USE_ADC3 || defined(__DOXYGEN__)
ADCDriver ADCD3;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

static const ADCConfig default_config = {
  .difsel = 0U
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Enables the ADC voltage regulator.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 */
static void adc_lld_vreg_on(ADCDriver *adcp) {

  adcp->adcm->CR = ADC_CR_ADVREGEN;
#if STM32_ADC_DUAL_MODE
  adcp->adcs->CR = ADC_CR_ADVREGEN;
#endif
  osalSysPolledDelayX(OSAL_US2RTC(STM32_SYS_CK, 10U));
}

/**
 * @brief   Disables the ADC voltage regulator.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 */
static void adc_lld_vreg_off(ADCDriver *adcp) {

  adcp->adcm->CR = ADC_CR_DEEPPWD;
#if STM32_ADC_DUAL_MODE
  adcp->adcs->CR = ADC_CR_DEEPPWD;
#endif
}

/**
 * @brief   Enables the ADC analog circuit.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 */
static void adc_lld_analog_on(ADCDriver *adcp) {

  adcp->adcm->ISR = ADC_ISR_ADRD;
  adcp->adcm->CR |= ADC_CR_ADEN;
  while ((adcp->adcm->ISR & ADC_ISR_ADRD) == 0U)
    ;
#if STM32_ADC_DUAL_MODE
  adcp->adcs->ISR = ADC_ISR_ADRD;
  adcp->adcs->CR |= ADC_CR_ADEN;
  while ((adcp->adcs->ISR & ADC_ISR_ADRD) == 0U)
    ;
#endif
}

/**
 * @brief   Disables the ADC  analog circuit.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 */
static void adc_lld_analog_off(ADCDriver *adcp) {

  adcp->adcm->CR |= ADC_CR_ADDIS;
  while ((adcp->adcm->CR & ADC_CR_ADDIS) != 0U)
    ;
#if STM32_ADC_DUAL_MODE
  adcp->adcs->CR |= ADC_CR_ADDIS;
  while ((adcp->adcs->CR & ADC_CR_ADDIS) != 0U)
    ;
#endif
}

/**
 * @brief   Calibrates and ADC unit.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 */
static void adc_lld_calibrate(ADCDriver *adcp) {

  osalDbgAssert(adcp->adcm->CR == ADC_CR_ADVREGEN, "invalid register state");

  adcp->adcm->CR &= ~(ADC_CR_ADCALDIF | ADC_CR_ADCALLIN);
  adcp->adcm->CR |= adcp->config->calibration & (ADC_CR_ADCALDIF |
                                                 ADC_CR_ADCALLIN);
  adcp->adcm->CR |= ADC_CR_ADCAL;
  while ((adcp->adcm->CR & ADC_CR_ADCAL) != 0U)
    ;
#if STM32_ADC_DUAL_MODE
  osalDbgAssert(adcp->adcs->CR == ADC_CR_ADVREGEN, "invalid register state");

  adcp->adcs->CR &= ~(ADC_CR_ADCALDIF | ADC_CR_ADCALLIN);
  adcp->adcs->CR |= adcp->config->calibration & (ADC_CR_ADCALDIF |
                                                 ADC_CR_ADCALLIN);
  adcp->adcs->CR |= ADC_CR_ADCAL;
  while ((adcp->adcs->CR & ADC_CR_ADCAL) != 0U)
    ;
#endif
}

/**
 * @brief   Stops an ongoing conversion, if any.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 */
static void adc_lld_stop_adc(ADCDriver *adcp) {

  if (adcp->adcm->CR & ADC_CR_ADSTART) {
    adcp->adcm->CR |= ADC_CR_ADSTP;
    while (adcp->adcm->CR & ADC_CR_ADSTP)
      ;
  }
  adcp->adcm->PCSEL = 0U;
}

/**
 * @brief   ADC DMA ISR service routine.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 * @param[in] flags     pre-shifted content of the ISR register
 */
static void adc_lld_serve_dma_interrupt(ADCDriver *adcp, uint32_t flags) {

  /* DMA errors handling.*/
  if ((flags & (STM32_DMA_ISR_TEIF | STM32_DMA_ISR_DMEIF)) != 0) {
    /* DMA, this could help only if the DMA tries to access an unmapped
       address space or violates alignment rules.*/
    _adc_isr_error_code(adcp, ADC_ERR_DMAFAILURE);
  }
  else {
    /* It is possible that the conversion group has already be reset by the
       ADC error handler, in this case this interrupt is spurious.*/
    if (adcp->grpp != NULL) {
      if ((flags & STM32_DMA_ISR_TCIF) != 0) {
        /* Transfer complete processing.*/
        _adc_isr_full_code(adcp);
      }
      else if ((flags & STM32_DMA_ISR_HTIF) != 0) {
        /* Half transfer processing.*/
        _adc_isr_half_code(adcp);
      }
    }
  }
}

/**
 * @brief   ADC BDMA ISR service routine.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 * @param[in] flags     pre-shifted content of the ISR register
 */
static void adc_lld_serve_bdma_interrupt(ADCDriver *adcp, uint32_t flags) {

  /* DMA errors handling.*/
  if ((flags & STM32_BDMA_ISR_TEIF) != 0) {
    /* DMA, this could help only if the DMA tries to access an unmapped
       address space or violates alignment rules.*/
    _adc_isr_error_code(adcp, ADC_ERR_DMAFAILURE);
  }
  else {
    /* It is possible that the conversion group has already be reset by the
       ADC error handler, in this case this interrupt is spurious.*/
    if (adcp->grpp != NULL) {
      if ((flags & STM32_BDMA_ISR_TCIF) != 0) {
        /* Transfer complete processing.*/
        _adc_isr_full_code(adcp);
      }
      else if ((flags & STM32_BDMA_ISR_HTIF) != 0) {
        /* Half transfer processing.*/
        _adc_isr_half_code(adcp);
      }
    }
  }
}

/**
 * @brief   ADC ISR service routine.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 * @param[in] isr       content of the ISR register
 */
static void adc_lld_serve_interrupt(ADCDriver *adcp, uint32_t isr) {

  /* It could be a spurious interrupt caused by overflows after DMA disabling,
     just ignore it in this case.*/
  if (adcp->grpp != NULL) {
    /* Note, an overflow may occur after the conversion ended before the driver
       is able to stop the ADC, this is why the state is checked too.*/
    if ((isr & ADC_ISR_OVR) && (adcp->state == ADC_ACTIVE)) {
      /* ADC overflow condition, this could happen only if the DMA is unable
         to read data fast enough.*/
      _adc_isr_error_code(adcp, ADC_ERR_OVERFLOW);
    }
    if (isr & ADC_ISR_AWD1) {
      /* Analog watchdog error.*/
      _adc_isr_error_code(adcp, ADC_ERR_AWD1);
    }
    if (isr & ADC_ISR_AWD2) {
      /* Analog watchdog error.*/
      _adc_isr_error_code(adcp, ADC_ERR_AWD2);
    }
    if (isr & ADC_ISR_AWD3) {
      /* Analog watchdog error.*/
      _adc_isr_error_code(adcp, ADC_ERR_AWD3);
    }
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if (STM32_ADC_USE_ADC12 == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   ADC1/ADC2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_ADC12_HANDLER) {
  uint32_t isr;

  OSAL_IRQ_PROLOGUE();

  isr  = ADC1->ISR;
#if STM32_ADC_DUAL_MODE == TRUE
  isr |= ADC2->ISR;
#endif
  ADC1->ISR = isr;
#if STM32_ADC_DUAL_MODE == TRUE
  ADC2->ISR = isr;
#endif
#if defined(STM32_ADC_ADC12_IRQ_HOOK)
  STM32_ADC_ADC12_IRQ_HOOK
#endif
  adc_lld_serve_interrupt(&ADCD1, isr);

  OSAL_IRQ_EPILOGUE();
}
#endif /* STM32_ADC_USE_ADC12 == TRUE */

#if (STM32_ADC_USE_ADC3 == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   ADC3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_ADC3_HANDLER) {
  uint32_t isr;

  OSAL_IRQ_PROLOGUE();

  isr  = ADC3->ISR;
  ADC3->ISR = isr;
#if defined(STM32_ADC_ADC3_IRQ_HOOK)
  STM32_ADC_ADC3_IRQ_HOOK
#endif
  adc_lld_serve_interrupt(&ADCD3, isr);

  OSAL_IRQ_EPILOGUE();
}
#endif /* STM32_ADC_USE_ADC3 == TRUE */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level ADC driver initialization.
 *
 * @notapi
 */
void adc_lld_init(void) {

#if STM32_ADC_USE_ADC12 == TRUE
  /* Driver initialization.*/
  adcObjectInit(&ADCD1);
  ADCD1.adcc        = ADC12_COMMON;
  ADCD1.adcm        = ADC1;
#if STM32_ADC_DUAL_MODE
  ADCD1.adcs        = ADC2;
#endif
  ADCD1.data.dma    = NULL;
  ADCD1.dmamode     = ADC_DMA_SIZE |
                      STM32_DMA_CR_PL(STM32_ADC_ADC12_DMA_PRIORITY) |
                      STM32_DMA_CR_DIR_P2M  |
                      STM32_DMA_CR_MINC     | STM32_DMA_CR_TCIE     |
                      STM32_DMA_CR_DMEIE    | STM32_DMA_CR_TEIE;
  nvicEnableVector(STM32_ADC12_NUMBER, STM32_ADC_ADC12_IRQ_PRIORITY);
#endif /* STM32_ADC_USE_ADC12 == TRUE */

#if STM32_ADC_USE_ADC3 == TRUE
  /* Driver initialization.*/
  adcObjectInit(&ADCD3);
  ADCD3.adcc        = ADC3_COMMON;
  ADCD3.adcm        = ADC3;
  ADCD3.data.bdma   = NULL;
  ADCD3.dmamode     = ADC_DMA_SIZE |
                      STM32_DMA_CR_PL(STM32_ADC_ADC3_DMA_PRIORITY)  |
                      STM32_DMA_CR_DIR_P2M  |
                      STM32_DMA_CR_MINC     | STM32_DMA_CR_TCIE     |
                      STM32_DMA_CR_DMEIE    | STM32_DMA_CR_TEIE;
  nvicEnableVector(STM32_ADC3_NUMBER, STM32_ADC_ADC3_IRQ_PRIORITY);
#endif /* STM32_ADC_USE_ADC3 == TRUE */

  /* ADC units pre-initializations.*/
#if (STM32_HAS_ADC1 == TRUE) && (STM32_HAS_ADC2 == TRUE)
#if STM32_ADC_USE_ADC12 == TRUE
  rccEnableADC12(true);
  rccResetADC12();
  ADC12_COMMON->CCR = STM32_ADC_ADC12_CLOCK_MODE | ADC_DMA_DAMDF;
  rccDisableADC12();
#endif
#if STM32_ADC_USE_ADC3 == TRUE
  rccEnableADC3(true);
  rccResetADC3();
  ADC3_COMMON->CCR = STM32_ADC_ADC3_CLOCK_MODE | ADC_DMA_DAMDF;
  rccDisableADC3();
#endif
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

  /* Handling the default configuration.*/
  if (adcp->config == NULL) {
    adcp->config = &default_config;
  }

  /* If in stopped state then enables the ADC and DMA clocks.*/
  if (adcp->state == ADC_STOP) {
#if STM32_ADC_USE_ADC12 == TRUE
    if (&ADCD1 == adcp) {
      adcp->data.dma = dmaStreamAllocI(STM32_ADC_ADC12_DMA_STREAM,
                                       STM32_ADC_ADC12_IRQ_PRIORITY,
                                       (stm32_dmaisr_t)adc_lld_serve_dma_interrupt,
                                       (void *)adcp);
      osalDbgAssert(adcp->data.dma != NULL, "unable to allocate stream");
      rccEnableADC12(true);
      dmaSetRequestSource(adcp->data.dma, STM32_DMAMUX1_ADC1);
    }
#endif /* STM32_ADC_USE_ADC12 == TRUE */

#if STM32_ADC_USE_ADC3 == TRUE
    if (&ADCD3 == adcp) {
      adcp->data.bdma = bdmaStreamAllocI(STM32_ADC_ADC3_BDMA_STREAM,
                                         STM32_ADC_ADC3_IRQ_PRIORITY,
                                         (stm32_dmaisr_t)adc_lld_serve_bdma_interrupt,
                                         (void *)adcp);
      osalDbgAssert(adcp->data.bdma != NULL, "unable to allocate stream");
      rccEnableADC3(true);
      bdmaSetRequestSource(adcp->data.bdma, STM32_DMAMUX2_ADC3_REQ);
    }
#endif /* STM32_ADC_USE_ADC3 == TRUE */

    /* Setting DMA peripheral-side pointer.*/
#if STM32_ADC_DUAL_MODE == TRUE
    dmaStreamSetPeripheral(adcp->data.dma, &adcp->adcc->CDR);
#else
    dmaStreamSetPeripheral(adcp->data.dma, &adcp->adcm->DR);
#endif

    /* Differential channels setting.*/
#if STM32_ADC_DUAL_MODE == TRUE
    adcp->adcm->DIFSEL = adcp->config->difsel;
    adcp->adcs->DIFSEL = adcp->config->difsel;
#else
    adcp->adcm->DIFSEL = adcp->config->difsel;
#endif

    /* Master ADC calibration.*/
    adc_lld_vreg_on(adcp);
    adc_lld_calibrate(adcp);

    /* Master ADC enabled here in order to reduce conversions latencies.*/
    adc_lld_analog_on(adcp);
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

    /* Stopping the ongoing conversion, if any.*/
    adc_lld_stop_adc(adcp);

    /* Disabling ADC analog circuit and regulator.*/
    adc_lld_analog_off(adcp);
    adc_lld_vreg_off(adcp);

#if STM32_ADC_USE_ADC12 == TRUE
    if (&ADCD1 == adcp) {

      /* Releasing the associated DMA channel.*/
      dmaStreamFreeI(adcp->data.dma);
      adcp->data.dma = NULL;

      /* Resetting CCR options except default ones.*/
      adcp->adcc->CCR = STM32_ADC_ADC12_CLOCK_MODE | ADC_DMA_DAMDF;
      rccDisableADC12();
    }
#endif

#if STM32_ADC_USE_ADC3 == TRUE
    if (&ADCD3 == adcp) {

      /* Releasing the associated BDMA channel.*/
      bdmaStreamFreeI(adcp->data.bdma);
      adcp->data.bdma = NULL;

      /* Resetting CCR options except default ones.*/
      adcp->adcc->CCR = STM32_ADC_ADC3_CLOCK_MODE | ADC_DMA_DAMDF;
      rccDisableADC3();
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
  uint32_t dmamode, cfgr;
  const ADCConversionGroup *grpp = adcp->grpp;
#if STM32_ADC_DUAL_MODE
  uint32_t ccr = grpp->ccr & ~(ADC_CCR_CKMODE_MASK | ADC_CCR_MDMA_MASK);
#endif

  osalDbgAssert(!STM32_ADC_DUAL_MODE || ((grpp->num_channels & 1) == 0),
                "odd number of channels in dual mode");

  /* Calculating control registers values.*/
  dmamode = adcp->dmamode;
  if (grpp->circular) {
    dmamode |= STM32_DMA_CR_CIRC;
    cfgr = grpp->cfgr | ADC_CFGR_DMNGT_CIRCULAR;
    if (adcp->depth > 1) {
      /* If circular buffer depth > 1, then the half transfer interrupt
         is enabled in order to allow streaming processing.*/
      dmamode |= STM32_DMA_CR_HTIE;
    }
    else {
      cfgr = grpp->cfgr | ADC_CFGR_DMNGT_ONESHOT;
    }
  }

  /* DMA setup.*/
  dmaStreamSetMemory0(adcp->data.dma, adcp->samples);
#if STM32_ADC_DUAL_MODE
  dmaStreamSetTransactionSize(adcp->data.dma, ((uint32_t)grpp->num_channels / 2U) *
                                              (uint32_t)adcp->depth);
#else
  dmaStreamSetTransactionSize(adcp->data.dma, (uint32_t)grpp->num_channels *
                                              (uint32_t)adcp->depth);
#endif
  dmaStreamSetMode(adcp->data.dma, dmamode);
  dmaStreamEnable(adcp->data.dma);

  /* ADC setup, if it is defined a callback for the analog watch dog then it
     is enabled.*/
  adcp->adcm->ISR   = adcp->adcm->ISR;
  adcp->adcm->IER   = ADC_IER_OVR | ADC_IER_AWD1;
#if STM32_ADC_DUAL_MODE

  /* Configuring the CCR register with the user-specified settings
     in the conversion group configuration structure, static settings are
     preserved.*/
  adcp->adcc->CCR   = (adcp->adcc->CCR &
                       (ADC_CCR_CKMODE_MASK | ADC_CCR_MDMA_MASK)) | ccr;

  adcp->adcm->CFGR2 = grpp->cfgr2;
  adcp->adcm->PCSEL = grpp->pcsel;
  adcp->adcm->LTR1  = grpp->ltr1;
  adcp->adcm->HTR1  = grpp->htr1;
  adcp->adcm->LTR1  = grpp->ltr2;
  adcp->adcm->HTR1  = grpp->htr2;
  adcp->adcm->LTR1  = grpp->ltr3;
  adcp->adcm->HTR1  = grpp->htr3;
  adcp->adcm->SMPR1 = grpp->smpr[0];
  adcp->adcm->SMPR2 = grpp->smpr[1];
  adcp->adcm->SQR1  = grpp->sqr[0] | ADC_SQR1_NUM_CH(grpp->num_channels / 2);
  adcp->adcm->SQR2  = grpp->sqr[1];
  adcp->adcm->SQR3  = grpp->sqr[2];
  adcp->adcm->SQR4  = grpp->sqr[3];
  adcp->adcs->CFGR2 = grpp->cfgr2;
  adcp->adcs->PCSEL = grpp->spcsel;
  adcp->adcs->LTR1  = grpp->sltr1;
  adcp->adcs->HTR1  = grpp->shtr1;
  adcp->adcs->LTR1  = grpp->sltr2;
  adcp->adcs->HTR1  = grpp->shtr2;
  adcp->adcs->LTR1  = grpp->sltr3;
  adcp->adcs->HTR1  = grpp->shtr3;
  adcp->adcs->SMPR1 = grpp->ssmpr[0];
  adcp->adcs->SMPR2 = grpp->ssmpr[1];
  adcp->adcs->SQR1  = grpp->ssqr[0] | ADC_SQR1_NUM_CH(grpp->num_channels / 2);
  adcp->adcs->SQR2  = grpp->ssqr[1];
  adcp->adcs->SQR3  = grpp->ssqr[2];
  adcp->adcs->SQR4  = grpp->ssqr[3];

  /* ADC configuration.*/
  adcp->adcm->CFGR  = cfgr;
  adcp->adcs->CFGR  = cfgr;

#else /* !STM32_ADC_DUAL_MODE */
  adcp->adcm->CFGR2 = grpp->cfgr2;
  adcp->adcm->PCSEL = grpp->pcsel;
  adcp->adcm->LTR1  = grpp->ltr1;
  adcp->adcm->HTR1  = grpp->htr1;
  adcp->adcm->LTR1  = grpp->ltr2;
  adcp->adcm->HTR1  = grpp->htr2;
  adcp->adcm->LTR1  = grpp->ltr3;
  adcp->adcm->HTR1  = grpp->htr3;
  adcp->adcm->SMPR1 = grpp->smpr[0];
  adcp->adcm->SMPR2 = grpp->smpr[1];
  adcp->adcm->SQR1  = grpp->sqr[0] | ADC_SQR1_NUM_CH(grpp->num_channels);
  adcp->adcm->SQR2  = grpp->sqr[1];
  adcp->adcm->SQR3  = grpp->sqr[2];
  adcp->adcm->SQR4  = grpp->sqr[3];

  /* ADC configuration.*/
  adcp->adcm->CFGR  = cfgr;
#endif /* !STM32_ADC_DUAL_MODE */

  /* Starting conversion.*/
  adcp->adcm->CR   |= ADC_CR_ADSTART;
}

/**
 * @brief   Stops an ongoing conversion.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_stop_conversion(ADCDriver *adcp) {

  dmaStreamDisable(adcp->data.dma);
  adc_lld_stop_adc(adcp);
}

/**
 * @brief   Enables the VREFEN bit.
 * @details The VREFEN bit is required in order to sample the VREF channel.
 * @note    This is an STM32-only functionality.
 * @note    This function is meant to be called after @p adcStart().
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adcSTM32EnableVREF(ADCDriver *adcp) {

  adcp->adcc->CCR |= ADC_CCR_VREFEN;
}

/**
 * @brief   Disables the VREFEN bit.
 * @details The VREFEN bit is required in order to sample the VREF channel.
 * @note    This is an STM32-only functionality.
 * @note    This function is meant to be called after @p adcStart().
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adcSTM32DisableVREF(ADCDriver *adcp) {

  adcp->adcc->CCR &= ~ADC_CCR_VREFEN;
}

/**
 * @brief   Enables the TSEN bit.
 * @details The TSEN bit is required in order to sample the internal
 *          temperature sensor and internal reference voltage.
 * @note    This is an STM32-only functionality.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adcSTM32EnableTS(ADCDriver *adcp) {

  adcp->adcc->CCR |= ADC_CCR_TSEN;
}

/**
 * @brief   Disables the TSEN bit.
 * @details The TSEN bit is required in order to sample the internal
 *          temperature sensor and internal reference voltage.
 * @note    This is an STM32-only functionality.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adcSTM32DisableTS(ADCDriver *adcp) {

  adcp->adcc->CCR &= ~ADC_CCR_TSEN;
}

/**
 * @brief   Enables the VBATEN bit.
 * @details The VBATEN bit is required in order to sample the VBAT channel.
 * @note    This is an STM32-only functionality.
 * @note    This function is meant to be called after @p adcStart().
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adcSTM32EnableVBAT(ADCDriver *adcp) {

  adcp->adcc->CCR |= ADC_CCR_VBATEN;
}

/**
 * @brief   Disables the VBATEN bit.
 * @details The VBATEN bit is required in order to sample the VBAT channel.
 * @note    This is an STM32-only functionality.
 * @note    This function is meant to be called after @p adcStart().
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adcSTM32DisableVBAT(ADCDriver *adcp) {

  adcp->adcc->CCR &= ~ADC_CCR_VBATEN;
}

#endif /* HAL_USE_ADC */

/** @} */
