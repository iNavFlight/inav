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

/**
 * @file    STM32/LLD/ADCv3/adc_lld.c
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

#define ADC1_DMA_CHANNEL                                                    \
  STM32_DMA_GETCHANNEL(STM32_ADC_ADC1_DMA_STREAM, STM32_ADC1_DMA_CHN)

#define ADC2_DMA_CHANNEL                                                    \
  STM32_DMA_GETCHANNEL(STM32_ADC_ADC2_DMA_STREAM, STM32_ADC2_DMA_CHN)

#define ADC3_DMA_CHANNEL                                                    \
  STM32_DMA_GETCHANNEL(STM32_ADC_ADC3_DMA_STREAM, STM32_ADC3_DMA_CHN)

#define ADC4_DMA_CHANNEL                                                    \
  STM32_DMA_GETCHANNEL(STM32_ADC_ADC4_DMA_STREAM, STM32_ADC4_DMA_CHN)

#if STM32_ADC_DUAL_MODE
#if STM32_ADC_COMPACT_SAMPLES
/* Compact type dual mode.*/
#define ADC_DMA_SIZE    (STM32_DMA_CR_MSIZE_HWORD | STM32_DMA_CR_PSIZE_HWORD)
#define ADC_DMA_MDMA    ADC_CCR_MDMA_HWORD

#else /* !STM32_ADC_COMPACT_SAMPLES */
/* Large type dual mode.*/
#define ADC_DMA_SIZE    (STM32_DMA_CR_MSIZE_WORD | STM32_DMA_CR_PSIZE_WORD)
#define ADC_DMA_MDMA    ADC_CCR_MDMA_WORD
#endif /* !STM32_ADC_COMPACT_SAMPLES */

#else /* !STM32_ADC_DUAL_MODE */
#if STM32_ADC_COMPACT_SAMPLES
/* Compact type single mode.*/
#define ADC_DMA_SIZE    (STM32_DMA_CR_MSIZE_BYTE | STM32_DMA_CR_PSIZE_BYTE)
#define ADC_DMA_MDMA    ADC_CCR_MDMA_DISABLED

#else /* !STM32_ADC_COMPACT_SAMPLES */
/* Large type single mode.*/
#define ADC_DMA_SIZE    (STM32_DMA_CR_MSIZE_HWORD | STM32_DMA_CR_PSIZE_HWORD)
#define ADC_DMA_MDMA    ADC_CCR_MDMA_DISABLED
#endif /* !STM32_ADC_COMPACT_SAMPLES */
#endif /* !STM32_ADC_DUAL_MODE */

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief ADC1 driver identifier.*/
#if STM32_ADC_USE_ADC1 || defined(__DOXYGEN__)
ADCDriver ADCD1;
#endif

/** @brief ADC2 driver identifier.*/
#if STM32_ADC_USE_ADC2 || defined(__DOXYGEN__)
ADCDriver ADCD2;
#endif

/** @brief ADC3 driver identifier.*/
#if STM32_ADC_USE_ADC3 || defined(__DOXYGEN__)
ADCDriver ADCD3;
#endif

/** @brief ADC4 driver identifier.*/
#if STM32_ADC_USE_ADC4 || defined(__DOXYGEN__)
ADCDriver ADCD4;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

static const ADCConfig default_config = {
  difsel: 0
};

static uint32_t clkmask;

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Enables the ADC voltage regulator.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 */
static void adc_lld_vreg_on(ADCDriver *adcp) {

#if defined(STM32F3XX)
  adcp->adcm->CR = 0;   /* RM 12.4.3.*/
  adcp->adcm->CR = ADC_CR_ADVREGEN_0;
#if STM32_ADC_DUAL_MODE
  adcp->adcs->CR = ADC_CR_ADVREGEN_0;
#endif
  osalSysPolledDelayX(OSAL_US2RTC(STM32_HCLK, 10));
#endif

#if defined(STM32L4XX)
  adcp->adcm->CR = 0;   /* RM 16.3.6.*/
  adcp->adcm->CR = ADC_CR_ADVREGEN;
#if STM32_ADC_DUAL_MODE
  adcp->adcs->CR = ADC_CR_ADVREGEN;
#endif
  osalSysPolledDelayX(OSAL_US2RTC(STM32_HCLK, 20));
#endif
}

/**
 * @brief   Disables the ADC voltage regulator.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 */
static void adc_lld_vreg_off(ADCDriver *adcp) {

#if defined(STM32F3XX)
  adcp->adcm->CR = 0;   /* RM 12.4.3.*/
  adcp->adcm->CR = ADC_CR_ADVREGEN_1;
#if STM32_ADC_DUAL_MODE
  adcp->adcs->CR = 0;
  adcp->adcs->CR = ADC_CR_ADVREGEN_1;
#endif
#endif

#if defined(STM32L4XX)
  adcp->adcm->CR = 0;   /* RM 12.4.3.*/
  adcp->adcm->CR = ADC_CR_DEEPPWD;
#if STM32_ADC_DUAL_MODE
  adcp->adcs->CR = 0;
  adcp->adcs->CR = ADC_CR_DEEPPWD;
#endif
#endif
}

/**
 * @brief   Enables the ADC analog circuit.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 */
static void adc_lld_analog_on(ADCDriver *adcp) {

#if defined(STM32F3XX)
  adcp->adcm->CR |= ADC_CR_ADEN;
  while ((adcp->adcm->ISR & ADC_ISR_ADRD) == 0)
    ;
#if STM32_ADC_DUAL_MODE
  adcp->adcs->CR |= ADC_CR_ADEN;
  while ((adcp->adcs->ISR & ADC_ISR_ADRD) == 0)
    ;
#endif
#endif

#if defined(STM32L4XX)
  adcp->adcm->CR |= ADC_CR_ADEN;
  while ((adcp->adcm->ISR & ADC_ISR_ADRDY) == 0)
    ;
#if STM32_ADC_DUAL_MODE
  adcp->adcs->CR |= ADC_CR_ADEN;
  while ((adcp->adcs->ISR & ADC_ISR_ADRDY) == 0)
    ;
#endif
#endif
}

/**
 * @brief   Disables the ADC  analog circuit.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 */
static void adc_lld_analog_off(ADCDriver *adcp) {

  adcp->adcm->CR |= ADC_CR_ADDIS;
  while ((adcp->adcm->CR & ADC_CR_ADDIS) != 0)
    ;
#if STM32_ADC_DUAL_MODE
  adcp->adcs->CR |= ADC_CR_ADDIS;
  while ((adcp->adcs->CR & ADC_CR_ADDIS) != 0)
    ;
#endif
}

/**
 * @brief   Calibrates and ADC unit.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 */
static void adc_lld_calibrate(ADCDriver *adcp) {

#if defined(STM32F3XX)
  osalDbgAssert(adcp->adcm->CR == ADC_CR_ADVREGEN_0, "invalid register state");
  adcp->adcm->CR |= ADC_CR_ADCAL;
  while ((adcp->adcm->CR & ADC_CR_ADCAL) != 0)
    ;
#if STM32_ADC_DUAL_MODE
  osalDbgAssert(adcp->adcs->CR == ADC_CR_ADVREGEN_0, "invalid register state");
  adcp->adcs->CR |= ADC_CR_ADCAL;
  while ((adcp->adcs->CR & ADC_CR_ADCAL) != 0)
    ;
#endif
#endif

#if defined(STM32L4XX)
  osalDbgAssert(adcp->adcm->CR == ADC_CR_ADVREGEN, "invalid register state");
  adcp->adcm->CR |= ADC_CR_ADCAL;
  while ((adcp->adcm->CR & ADC_CR_ADCAL) != 0)
    ;
#if STM32_ADC_DUAL_MODE
  osalDbgAssert(adcp->adcs->CR == ADC_CR_ADVREGEN, "invalid register state");
  adcp->adcs->CR |= ADC_CR_ADCAL;
  while ((adcp->adcs->CR & ADC_CR_ADCAL) != 0)
    ;
#endif
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
       is able to stop the ADC, this is why the DMA channel is checked too.*/
    if ((isr & ADC_ISR_OVR) &&
        (dmaStreamGetTransactionSize(adcp->dmastp) > 0)) {
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

#if STM32_ADC_USE_ADC1 || STM32_ADC_USE_ADC2 || defined(__DOXYGEN__)
/**
 * @brief   ADC1/ADC2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_ADC1_HANDLER) {
  uint32_t isr;

  OSAL_IRQ_PROLOGUE();

#if STM32_ADC_DUAL_MODE
  isr  = ADC1->ISR;
  isr |= ADC2->ISR;
  ADC1->ISR = isr;
  ADC2->ISR = isr;

  adc_lld_serve_interrupt(&ADCD1, isr);
#else /* !STM32_ADC_DUAL_MODE */
#if STM32_ADC_USE_ADC1
  isr  = ADC1->ISR;
  ADC1->ISR = isr;

  adc_lld_serve_interrupt(&ADCD1, isr);
#endif
#if STM32_ADC_USE_ADC2
  isr  = ADC2->ISR;
  ADC2->ISR = isr;

  adc_lld_serve_interrupt(&ADCD2, isr);
#endif
#endif /* !STM32_ADC_DUAL_MODE */

  OSAL_IRQ_EPILOGUE();
}
#endif /* STM32_ADC_USE_ADC1 */

#if STM32_ADC_USE_ADC3 || defined(__DOXYGEN__)
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

  adc_lld_serve_interrupt(&ADCD3, isr);

  OSAL_IRQ_EPILOGUE();
}

#if STM32_ADC_DUAL_MODE
/**
 * @brief   ADC4 interrupt handler (as ADC3 slave).
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_ADC4_HANDLER) {
  uint32_t isr;

  OSAL_IRQ_PROLOGUE();

  isr  = ADC4->ISR;
  ADC4->ISR = isr;

  adc_lld_serve_interrupt(&ADCD3, isr);

  OSAL_IRQ_EPILOGUE();
}
#endif /* STM32_ADC_DUAL_MODE */
#endif /* STM32_ADC_USE_ADC3 */

#if STM32_ADC_USE_ADC4 || defined(__DOXYGEN__)
/**
 * @brief   ADC4 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_ADC4_HANDLER) {
  uint32_t isr;

  OSAL_IRQ_PROLOGUE();

  isr  = ADC4->ISR;
  ADC4->ISR = isr;

  adc_lld_serve_interrupt(&ADCD4, isr);

  OSAL_IRQ_EPILOGUE();
}
#endif /* STM32_ADC_USE_ADC4 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level ADC driver initialization.
 *
 * @notapi
 */
void adc_lld_init(void) {

  clkmask = 0;

#if STM32_ADC_USE_ADC1
  /* Driver initialization.*/
  adcObjectInit(&ADCD1);
#if defined(ADC1_2_COMMON)
  ADCD1.adcc = ADC1_2_COMMON;
#elif defined(ADC123_COMMON)
  ADCD1.adcc = ADC123_COMMON;
#else
  ADCD1.adcc = ADC1_COMMON;
#endif
  ADCD1.adcm    = ADC1;
#if STM32_ADC_DUAL_MODE
  ADCD1.adcs    = ADC2;
#endif
  ADCD1.dmastp  = STM32_DMA_STREAM(STM32_ADC_ADC1_DMA_STREAM);
  ADCD1.dmamode = ADC_DMA_SIZE |
                  STM32_DMA_CR_PL(STM32_ADC_ADC1_DMA_PRIORITY) |
                  STM32_DMA_CR_DIR_P2M |
                  STM32_DMA_CR_MINC        | STM32_DMA_CR_TCIE        |
                  STM32_DMA_CR_DMEIE       | STM32_DMA_CR_TEIE;
#endif /* STM32_ADC_USE_ADC1 */

#if STM32_ADC_USE_ADC2
  /* Driver initialization.*/
  adcObjectInit(&ADCD2);
#if defined(ADC1_2_COMMON)
  ADCD2.adcc = ADC1_2_COMMON;
#elif defined(ADC123_COMMON)
  ADCD2.adcc = ADC123_COMMON;
#endif
  ADCD2.adcm    = ADC2;
  ADCD2.dmastp  = STM32_DMA_STREAM(STM32_ADC_ADC2_DMA_STREAM);
  ADCD2.dmamode = ADC_DMA_SIZE |
                  STM32_DMA_CR_PL(STM32_ADC_ADC2_DMA_PRIORITY) |
                  STM32_DMA_CR_DIR_P2M |
                  STM32_DMA_CR_MINC        | STM32_DMA_CR_TCIE        |
                  STM32_DMA_CR_DMEIE       | STM32_DMA_CR_TEIE;
#endif /* STM32_ADC_USE_ADC2 */

#if STM32_ADC_USE_ADC3
  /* Driver initialization.*/
  adcObjectInit(&ADCD3);
#if defined(ADC3_4_COMMON)
  ADCD3.adcc = ADC3_4_COMMON;
#elif defined(ADC123_COMMON)
  ADCD1.adcc = ADC123_COMMON;
#else
  ADCD3.adcc = ADC3_COMMON;
#endif
  ADCD3.adcm    = ADC3;
#if STM32_ADC_DUAL_MODE
  ADCD3.adcs    = ADC4;
#endif
  ADCD3.dmastp  = STM32_DMA_STREAM(STM32_ADC_ADC3_DMA_STREAM);
  ADCD3.dmamode = ADC_DMA_SIZE |
                  STM32_DMA_CR_PL(STM32_ADC_ADC3_DMA_PRIORITY) |
                  STM32_DMA_CR_DIR_P2M |
                  STM32_DMA_CR_MINC        | STM32_DMA_CR_TCIE        |
                  STM32_DMA_CR_DMEIE       | STM32_DMA_CR_TEIE;
#endif /* STM32_ADC_USE_ADC3 */

#if STM32_ADC_USE_ADC4
  /* Driver initialization.*/
  adcObjectInit(&ADCD4);
  ADCD4.adcc = ADC3_4_COMMON;
  ADCD4.adcm    = ADC4;
  ADCD4.dmastp  = STM32_DMA_STREAM(STM32_ADC_ADC4_DMA_STREAM);
  ADCD4.dmamode = ADC_DMA_SIZE |
                  STM32_DMA_CR_PL(STM32_ADC_ADC4_DMA_PRIORITY) |
                  STM32_DMA_CR_DIR_P2M |
                  STM32_DMA_CR_MINC        | STM32_DMA_CR_TCIE        |
                  STM32_DMA_CR_DMEIE       | STM32_DMA_CR_TEIE;
#endif /* STM32_ADC_USE_ADC4 */

  /* IRQs setup.*/
#if STM32_ADC_USE_ADC1 || STM32_ADC_USE_ADC2
  nvicEnableVector(STM32_ADC1_NUMBER, STM32_ADC_ADC12_IRQ_PRIORITY);
#endif
#if STM32_ADC_USE_ADC3
  nvicEnableVector(STM32_ADC3_NUMBER, STM32_ADC_ADC3_IRQ_PRIORITY);
#if STM32_ADC_DUAL_MODE
  nvicEnableVector(STM32_ADC4_NUMBER, STM32_ADC_ADC3_IRQ_PRIORITY);
#endif
#endif
#if STM32_ADC_USE_ADC4
  nvicEnableVector(STM32_ADC4_NUMBER, STM32_ADC_ADC3_IRQ_PRIORITY);
#endif

  /* ADC units pre-initializations.*/
#if defined(STM32F3XX)
#if STM32_HAS_ADC1 && STM32_HAS_ADC2
#if STM32_ADC_USE_ADC1 || STM32_ADC_USE_ADC2
  rccEnableADC12(FALSE);
  rccResetADC12();
  ADC1_2_COMMON->CCR = STM32_ADC_ADC12_CLOCK_MODE | ADC_DMA_MDMA;
  rccDisableADC12(FALSE);
#endif
#else
#if STM32_ADC_USE_ADC1
  rccEnableADC12(FALSE);
  rccResetADC12();
  ADC1_COMMON->CCR = STM32_ADC_ADC12_CLOCK_MODE | ADC_DMA_MDMA;
  rccDisableADC12(FALSE);
#endif
#endif
#if STM32_ADC_USE_ADC3 || STM32_ADC_USE_ADC4
  rccEnableADC34(FALSE);
  rccResetADC34();
  ADC3_4_COMMON->CCR = STM32_ADC_ADC34_CLOCK_MODE | ADC_DMA_MDMA;
  rccDisableADC34(FALSE);
#endif
#endif

#if defined(STM32L4XX)
  rccEnableADC123(FALSE);
  rccResetADC123();
  ADC123_COMMON->CCR = STM32_ADC_ADC123_CLOCK_MODE | ADC_DMA_MDMA;
  rccDisableADC123(FALSE);
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
#if STM32_ADC_USE_ADC1
    if (&ADCD1 == adcp) {
      bool b;
      b = dmaStreamAllocate(adcp->dmastp,
                            STM32_ADC_ADC1_DMA_IRQ_PRIORITY,
                            (stm32_dmaisr_t)adc_lld_serve_dma_interrupt,
                            (void *)adcp);
      osalDbgAssert(!b, "stream already allocated");

      clkmask |= (1 << 0);
#if defined(STM32F3XX)
      rccEnableADC12(FALSE);
#endif
#if defined(STM32L4XX)
      rccEnableADC123(FALSE);
#endif
    }
#endif /* STM32_ADC_USE_ADC1 */

#if STM32_ADC_USE_ADC2
    if (&ADCD2 == adcp) {
      bool b;
      b = dmaStreamAllocate(adcp->dmastp,
                            STM32_ADC_ADC2_DMA_IRQ_PRIORITY,
                            (stm32_dmaisr_t)adc_lld_serve_dma_interrupt,
                            (void *)adcp);
      osalDbgAssert(!b, "stream already allocated");

      clkmask |= (1 << 1);
#if defined(STM32F3XX)
      rccEnableADC12(FALSE);
#endif
#if defined(STM32L4XX)
      rccEnableADC123(FALSE);
#endif
    }
#endif /* STM32_ADC_USE_ADC2 */

#if STM32_ADC_USE_ADC3
    if (&ADCD3 == adcp) {
      bool b;
      b = dmaStreamAllocate(adcp->dmastp,
                            STM32_ADC_ADC3_DMA_IRQ_PRIORITY,
                            (stm32_dmaisr_t)adc_lld_serve_dma_interrupt,
                            (void *)adcp);
      osalDbgAssert(!b, "stream already allocated");

      clkmask |= (1 << 2);
#if defined(STM32F3XX)
      rccEnableADC34(FALSE);
#endif
#if defined(STM32L4XX)
      rccEnableADC123(FALSE);
#endif
    }
#endif /* STM32_ADC_USE_ADC3 */

#if STM32_ADC_USE_ADC4
    if (&ADCD4 == adcp) {
      bool b;
      b = dmaStreamAllocate(adcp->dmastp,
                            STM32_ADC_ADC4_DMA_IRQ_PRIORITY,
                            (stm32_dmaisr_t)adc_lld_serve_dma_interrupt,
                            (void *)adcp);
      osalDbgAssert(!b, "stream already allocated");

      clkmask |= (1 << 3);
#if defined(STM32F3XX)
      rccEnableADC34(FALSE);
#endif
#if defined(STM32L4XX)
      rccEnableADC123(FALSE);
#endif
    }
#endif /* STM32_ADC_USE_ADC4 */

    /* Setting DMA peripheral-side pointer.*/
#if STM32_ADC_DUAL_MODE
    dmaStreamSetPeripheral(adcp->dmastp, &adcp->adcc->CDR);
#else
    dmaStreamSetPeripheral(adcp->dmastp, &adcp->adcm->DR);
#endif

    /* Differential channels setting.*/
#if STM32_ADC_DUAL_MODE
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

    /* Releasing the associated DMA channel.*/
    dmaStreamRelease(adcp->dmastp);

    /* Stopping the ongoing conversion, if any.*/
    adc_lld_stop_adc(adcp);

    /* Disabling ADC analog circuit and regulator.*/
    adc_lld_analog_off(adcp);
    adc_lld_vreg_off(adcp);

#if defined(STM32L4XX)
    /* Resetting CCR options except default ones.*/
    adcp->adcc->CCR = STM32_ADC_ADC123_CLOCK_MODE | ADC_DMA_MDMA;
#endif

#if STM32_ADC_USE_ADC1
    if (&ADCD1 == adcp) {
#if defined(STM32F3XX)
      /* Resetting CCR options except default ones.*/
      adcp->adcc->CCR = STM32_ADC_ADC12_CLOCK_MODE | ADC_DMA_MDMA;
#endif
      clkmask &= ~(1 << 0);
    }
#endif

#if STM32_ADC_USE_ADC2
    if (&ADCD2 == adcp) {
#if defined(STM32F3XX)
      /* Resetting CCR options except default ones.*/
      adcp->adcc->CCR = STM32_ADC_ADC12_CLOCK_MODE | ADC_DMA_MDMA;
#endif
      clkmask &= ~(1 << 1);
    }
#endif

#if STM32_ADC_USE_ADC3
    if (&ADCD3 == adcp) {
#if defined(STM32F3XX)
      /* Resetting CCR options except default ones.*/
      adcp->adcc->CCR = STM32_ADC_ADC34_CLOCK_MODE | ADC_DMA_MDMA;
#endif
      clkmask &= ~(1 << 2);
    }
#endif

#if STM32_ADC_USE_ADC4
    if (&ADCD4 == adcp) {
#if defined(STM32F3XX)
      /* Resetting CCR options except default ones.*/
      adcp->adcc->CCR = STM32_ADC_ADC34_CLOCK_MODE | ADC_DMA_MDMA;
#endif
      clkmask &= ~(1 << 3);
    }
#endif

#if defined(STM32F3XX)
#if STM32_HAS_ADC1 || STM32_HAS_ADC2
    if ((clkmask & 0x3) == 0) {
      rccDisableADC12(FALSE);
    }
#endif

#if STM32_HAS_ADC3 || STM32_HAS_ADC4
    if ((clkmask & 0xC) == 0) {
      rccDisableADC34(FALSE);
    }
#endif
#endif

#if defined(STM32L4XX)
    if ((clkmask & 0x7) == 0) {
      rccDisableADC123(FALSE);
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
  cfgr    = grpp->cfgr | ADC_CFGR_DMAEN;
  if (grpp->circular) {
    dmamode |= STM32_DMA_CR_CIRC;
#if STM32_ADC_DUAL_MODE
    ccr  |= ADC_CCR_DMACFG_CIRCULAR;
#else
    cfgr |= ADC_CFGR_DMACFG_CIRCULAR;
#endif
    if (adcp->depth > 1) {
      /* If circular buffer depth > 1, then the half transfer interrupt
         is enabled in order to allow streaming processing.*/
      dmamode |= STM32_DMA_CR_HTIE;
    }
  }

  /* DMA setup.*/
  dmaStreamSetMemory0(adcp->dmastp, adcp->samples);
#if STM32_ADC_DUAL_MODE
  dmaStreamSetTransactionSize(adcp->dmastp, ((uint32_t)grpp->num_channels/2) *
                                            (uint32_t)adcp->depth);
#else
  dmaStreamSetTransactionSize(adcp->dmastp, (uint32_t)grpp->num_channels *
                                            (uint32_t)adcp->depth);
#endif
  dmaStreamSetMode(adcp->dmastp, dmamode);
  dmaStreamEnable(adcp->dmastp);

  /* ADC setup, if it is defined a callback for the analog watch dog then it
     is enabled.*/
  adcp->adcm->ISR   = adcp->adcm->ISR;
  adcp->adcm->IER   = ADC_IER_OVR | ADC_IER_AWD1;
  adcp->adcm->TR1   = grpp->tr1;
#if STM32_ADC_DUAL_MODE

  /* Configuring the CCR register with the user-specified settings
     in the conversion group configuration structure, static settings are
     preserved.*/
  adcp->adcc->CCR   = (adcp->adcc->CCR &
                       (ADC_CCR_CKMODE_MASK | ADC_CCR_MDMA_MASK)) | ccr;

  adcp->adcm->SMPR1 = grpp->smpr[0];
  adcp->adcm->SMPR2 = grpp->smpr[1];
  adcp->adcm->SQR1  = grpp->sqr[0] | ADC_SQR1_NUM_CH(grpp->num_channels / 2);
  adcp->adcm->SQR2  = grpp->sqr[1];
  adcp->adcm->SQR3  = grpp->sqr[2];
  adcp->adcm->SQR4  = grpp->sqr[3];
  adcp->adcs->SMPR1 = grpp->ssmpr[0];
  adcp->adcs->SMPR2 = grpp->ssmpr[1];
  adcp->adcs->SQR1  = grpp->ssqr[0] | ADC_SQR1_NUM_CH(grpp->num_channels / 2);
  adcp->adcs->SQR2  = grpp->ssqr[1];
  adcp->adcs->SQR3  = grpp->ssqr[2];
  adcp->adcs->SQR4  = grpp->ssqr[3];

#else /* !STM32_ADC_DUAL_MODE */
  adcp->adcm->SMPR1 = grpp->smpr[0];
  adcp->adcm->SMPR2 = grpp->smpr[1];
  adcp->adcm->SQR1  = grpp->sqr[0] | ADC_SQR1_NUM_CH(grpp->num_channels);
  adcp->adcm->SQR2  = grpp->sqr[1];
  adcp->adcm->SQR3  = grpp->sqr[2];
  adcp->adcm->SQR4  = grpp->sqr[3];
#endif /* !STM32_ADC_DUAL_MODE */

  /* ADC configuration.*/
  adcp->adcm->CFGR  = cfgr;

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

  dmaStreamDisable(adcp->dmastp);
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
