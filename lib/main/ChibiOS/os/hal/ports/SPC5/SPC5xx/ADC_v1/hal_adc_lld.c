/*
    SPC5 HAL - Copyright (C) 2013,2014 STMicroelectronics

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
 * @file    SPC5xx/ADC_v1/hal_adc_lld.c
 * @brief   ADC Driver subsystem low level driver source.
 *
 * @addtogroup ADC
 * @{
 */

#include "hal.h"

#if HAL_USE_ADC || defined(__DOXYGEN__)

/* Some forward declarations.*/
#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON
static void adc_serve_adc_irq(edma_channel_t channel, void *p);
static void adc_serve_dma_error_irq(edma_channel_t channel,
                                    void *p,
                                    uint32_t esr);
#endif

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   ADC1 driver identifier.
 */
#if SPC5_ADC_USE_ADC0 || defined(__DOXYGEN__)
ADCDriver ADCD1;
#endif

#if SPC5_ADC_USE_ADC1 || defined(__DOXYGEN__)
ADCDriver ADCD2;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   DMA configuration for ADC0.
 */
#if SPC5_ADC_USE_ADC0 || defined(__DOXYGEN__)
#if (SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON) || defined(__DOXYGEN__)
static const edma_channel_config_t adc_adc0_dma_config = {
  SPC5_ADC_ADC0_DMA_CH_ID,
  SPC5_ADC0_DMA_DEV_ID,
  SPC5_ADC_ADC0_DMA_IRQ_PRIO,
  adc_serve_adc_irq, adc_serve_dma_error_irq, &ADCD1
};
#endif /* SPC5_ADC_DMA_MODE */
#endif /* SPC5_ADC_USE_ADC0 */

/**
 * @brief   DMA configuration for ADC1.
 */
#if SPC5_ADC_USE_ADC1 || defined(__DOXYGEN__)
#if (SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON) || defined(__DOXYGEN__)
static const edma_channel_config_t adc_adc1_dma_config = {
  SPC5_ADC_ADC1_DMA_CH_ID,
  SPC5_ADC1_DMA_DEV_ID,
  SPC5_ADC_ADC1_DMA_IRQ_PRIO,
  adc_serve_adc_irq, adc_serve_dma_error_irq, &ADCD2
};
#endif /* SPC5_ADC_DMA_MODE */
#endif /* SPC5_ADC_USE_ADC1 */

/*===========================================================================*/
/* Driver local functions and macros.                                        */
/*===========================================================================*/

/**
 * @brief   Unsigned two's complement.
 *
 * @param[in] n         the value to be complemented
 *
 * @notapi
 */
#define CPL2(n) ((~(uint32_t)(n)) + 1)

#if (SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON) || defined(__DOXYGEN__)
/**
 * @brief   Shared ISR for ADC events.
 *
 * @param[in] channel   the channel number
 * @param[in] p         parameter for the registered function
 *
 * @notapi
 */
static void adc_serve_adc_irq(edma_channel_t channel, void *p) {
  ADCDriver *adcp = (ADCDriver *)p;
  edma_tcd_t *tcdp = edmaGetTCD(channel);

  if (adcp->grpp != NULL) {
    if ((tcdp->word[5] >> 16) != (tcdp->word[7] >> 16)) {
      /* Half transfer processing.*/
      _adc_isr_half_code(adcp);
    }
    else {
      /* Re-starting DMA channel if in circular mode.*/
      if (adcp->grpp->circular) {
        edmaChannelStart(adcp->adc_dma_channel);
      }

      /* Transfer complete processing.*/
      _adc_isr_full_code(adcp);
    }
  }
}

/**
 * @brief   Shared ISR for DMA error events.
 *
 * @param[in] channel   the channel number
 * @param[in] p         parameter for the registered function
 * @param[in] esr       content of the ESR register
 *
 * @notapi
 */
static void adc_serve_dma_error_irq(edma_channel_t channel,
                                    void *p,
                                    uint32_t esr) {
  ADCDriver *adcp = (ADCDriver *)p;

  (void)channel;
  (void)esr;

  _adc_isr_error_code(adcp, ADC_ERR_DMAFAILURE);
}

#else /* SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF */
/**
 * @brief   Shared ISR for ADC events.
 *
 * @notapi
 */
static void adc_serve_adc_irq_non_dma(ADCDriver *adcp) {

  if (adcp->grpp != NULL) {
    if (adcp->rx_cnt == 0) {
        /* Stop if in circular mode.*/
         if (adcp->grpp->circular)
         {
            /* Reset the rx_ptr to prevent from the coredump */
            adcp->rx_ptr=adcp->samples;
            adcp->rx_cnt=adcp->depth * adcp->grpp->num_channels;
         }

        /* Transfer complete processing.*/
        _adc_isr_full_code(adcp);
    }
    else
    {
        if (adcp->rx_cnt == (adcp->depth * adcp->grpp->num_channels)/2) {
            /* Half Transfer*/
            _adc_isr_half_code(adcp);
        }
    }
  }
}
#endif /* SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF */

/**
 * @brief   ADC ISR service routine.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 * @param[in] isr       content of the ISR register
 *
 * @notapi
 */
static void adc_lld_serve_interrupt(ADCDriver *adcp, uint32_t isr) {

  /* It could be a spurious interrupt caused by overflows after DMA disabling,
     just ignore it in this case.*/
  if (adcp->grpp != NULL) {
    if (isr != 0) {
      /* Analog watchdog error.*/
      adcp->adc_awd_err = isr;
      _adc_isr_error_code(adcp, ADC_ERR_AWD);
    }
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if SPC5_ADC_USE_ADC0
#if !defined(SPC5_ADC0_WD_HANDLER)
#error "SPC5_ADC0_WD_HANDLER not defined"
#endif
/**
 * @brief   ADC0 Watch Dog interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_ADC0_WD_HANDLER) {
  uint32_t isr;

  OSAL_IRQ_PROLOGUE();

  isr = ADCD1.adc_tagp->WTISR.R;
  ADCD1.adc_tagp->WTISR.R = isr;
  adc_lld_serve_interrupt(&ADCD1, isr);

  OSAL_IRQ_EPILOGUE();
}

#if (SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF) || defined(__DOXYGEN__)
#if !defined(SPC5_ADC0_EOC_HANDLER)
#error "SPC5_ADC0_EOC_HANDLER not defined"
#endif
/**
 * @brief   ADC0 End of Conversion interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_ADC0_EOC_HANDLER) {

#if SPC5_ADC_ADC0_HAS_CEOCFR0
  uint32_t sr0;
#endif
#if SPC5_ADC_ADC0_HAS_CEOCFR1
  uint32_t sr1;
#endif
#if SPC5_ADC_ADC0_HAS_CEOCFR2
  uint32_t sr2;
#endif

  uint32_t start,n,i;

  OSAL_IRQ_PROLOGUE();

  /* Reading status bits.*/
#if SPC5_ADC_ADC0_HAS_CEOCFR0
  sr0 = ADCD1.adc_tagp->CEOCFR[0].R;
#endif
#if SPC5_ADC_ADC0_HAS_CEOCFR1
  sr1 = ADCD1.adc_tagp->CEOCFR[1].R;
#endif
#if SPC5_ADC_ADC0_HAS_CEOCFR2
  sr2 = ADCD1.adc_tagp->CEOCFR[2].R;
#endif

  /* Clearing read status bits.*/
#if SPC5_ADC_ADC0_HAS_CEOCFR0
  ADCD1.adc_tagp->CEOCFR[0].R = sr0;
#endif
#if SPC5_ADC_ADC0_HAS_CEOCFR1
  ADCD1.adc_tagp->CEOCFR[1].R = sr1;
#endif
#if SPC5_ADC_ADC0_HAS_CEOCFR2
  ADCD1.adc_tagp->CEOCFR[2].R = sr2;
#endif

  /* Clearing the read status bits.*/
  ADCD1.adc_tagp->ISR.B.EOC = 1;
  ADCD1.adc_tagp->ISR.B.ECH = 1;

  start = ADCD1.grpp->init_channel;
  n = ADCD1.grpp->num_channels;

  for (i = start; i < start + n; i++) {
      *(ADCD1.rx_ptr++) =  ADCD1.adc_tagp->CDR[i].B.CDATA;
      ADCD1.rx_cnt--;
  }

  adc_serve_adc_irq_non_dma(&ADCD1);

  OSAL_IRQ_EPILOGUE();
}

#endif /* SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF */
#endif /* SPC5_ADC_USE_ADC0 */

#if SPC5_ADC_USE_ADC1
#if !defined(SPC5_ADC1_WD_HANDLER)
#error "SPC5_ADC1_WD_HANDLER not defined"
#endif
/**
 * @brief   ADC1 Watch Dog interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_ADC1_WD_HANDLER) {
  uint32_t isr;

  OSAL_IRQ_PROLOGUE();

  isr = ADCD2.adc_tagp->WTISR.R;
  ADCD2.adc_tagp->WTISR.R = isr;
  adc_lld_serve_interrupt(&ADCD2, isr);

  OSAL_IRQ_EPILOGUE();
}

#if (SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF) || defined(__DOXYGEN__)
#if !defined(SPC5_ADC1_EOC_HANDLER)
#error "SPC5_ADC1_EOC_HANDLER not defined"
#endif
/**
 * @brief   ADC1 End of Conversion interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_ADC1_EOC_HANDLER) {
#if SPC5_ADC_ADC1_HAS_CEOCFR0
  uint32_t sr0;
#endif
#if SPC5_ADC_ADC1_HAS_CEOCFR1
  uint32_t sr1;
#endif
#if SPC5_ADC_ADC1_HAS_CEOCFR2
  uint32_t sr2;
#endif
  uint32_t start,n,i;

  OSAL_IRQ_PROLOGUE();

  /* Reading status bits.*/
#if SPC5_ADC_ADC1_HAS_CEOCFR0
  sr0 = ADCD2.adc_tagp->CEOCFR[0].R;
#endif
#if SPC5_ADC_ADC1_HAS_CEOCFR1
  sr1 = ADCD2.adc_tagp->CEOCFR[1].R;
#endif
#if SPC5_ADC_ADC1_HAS_CEOCFR2
  sr2 = ADCD2.adc_tagp->CEOCFR[2].R;
#endif

  /* Clearing read status bits.*/
#if SPC5_ADC_ADC1_HAS_CEOCFR0
  ADCD2.adc_tagp->CEOCFR[0].R = sr0;
#endif
#if SPC5_ADC_ADC1_HAS_CEOCFR1
  ADCD2.adc_tagp->CEOCFR[1].R = sr1;
#endif
#if SPC5_ADC_ADC1_HAS_CEOCFR2
  ADCD2.adc_tagp->CEOCFR[2].R = sr2;
#endif

  /* Clearing the read status bits.*/
  ADCD2.adc_tagp->ISR.B.EOC = 1;
  ADCD2.adc_tagp->ISR.B.ECH = 1;

  start = ADCD2.grpp->init_channel;
  n = ADCD2.grpp->num_channels;

  for (i = start; i < start + n; i++) {
    *(ADCD2.rx_ptr++) =  ADCD2.adc_tagp->CDR[i].B.CDATA;
    ADCD2.rx_cnt--;
  }

  adc_serve_adc_irq_non_dma(&ADCD2);

  OSAL_IRQ_EPILOGUE();
}

#endif /* SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF */
#endif /* SPC5_ADC_USE_ADC1 */


/*===========================================================================*/
/* Driver registers update                                                   */
/*===========================================================================*/

#if (SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON) || defined(__DOXYGEN__)
/**
 * @brief   Initialize registers for DMA case
 *
 * @param[in] adcp      parameter for the registered function
 *
 * @notapi
 */
static void adc_initialize_registers_dma(ADCDriver *adcp) {
  uint32_t ch_mask;
  uint32_t dma_mask;

  /* Sets thresholds’ values and active watchdog threshold interrupts if any.*/
#if SPC5_ADC_HAS_TRC
  adcp->adc_tagp->TRC[0].R = adcp->grpp->trc[0];
  adcp->adc_tagp->TRC[1].R = adcp->grpp->trc[1];
  adcp->adc_tagp->TRC[2].R = adcp->grpp->trc[2];
  adcp->adc_tagp->TRC[3].R = adcp->grpp->trc[3];
#if SPC5_ADC_USE_ADC0
  if (&ADCD1 == adcp) {
#if SPC5_ADC_ADC0_HAS_THRHLR0
    adcp->adc_tagp->THRHLR[0].R = adcp->grpp->thrhlr[0];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR1
    adcp->adc_tagp->THRHLR[1].R = adcp->grpp->thrhlr[1];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR2
    adcp->adc_tagp->THRHLR[2].R = adcp->grpp->thrhlr[2];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR3
    adcp->adc_tagp->THRHLR[3].R = adcp->grpp->thrhlr[3];
#endif
  }
#endif
#if SPC5_ADC_USE_ADC1
  if (&ADCD2 == adcp) {
#if SPC5_ADC_ADC1_HAS_THRHLR0
    adcp->adc_tagp->THRHLR[0].R = adcp->grpp->thrhlr[0];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR1
    adcp->adc_tagp->THRHLR[1].R = adcp->grpp->thrhlr[1];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR2
    adcp->adc_tagp->THRHLR[2].R = adcp->grpp->thrhlr[2];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR3
    adcp->adc_tagp->THRHLR[3].R = adcp->grpp->thrhlr[3];
#endif
  }
#endif
#else
#if SPC5_ADC_USE_ADC0
  if (&ADCD1 == adcp) {
#if SPC5_ADC_ADC0_HAS_THRHLR0
    adcp->adc_tagp->THRHLR[0].R = adcp->grpp->thrhlr[0];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR1
    adcp->adc_tagp->THRHLR[1].R = adcp->grpp->thrhlr[1];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR2
    adcp->adc_tagp->THRHLR[2].R = adcp->grpp->thrhlr[2];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR3
    adcp->adc_tagp->THRHLR[3].R = adcp->grpp->thrhlr[3];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR4
    adcp->adc_tagp->THRHLR_2[0].R = adcp->grpp->thrhlr[4];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR5
    adcp->adc_tagp->THRHLR_2[1].R = adcp->grpp->thrhlr[5];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR6
    adcp->adc_tagp->THRHLR_2[2].R = adcp->grpp->thrhlr[6];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR7
    adcp->adc_tagp->THRHLR_2[3].R = adcp->grpp->thrhlr[7];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR8
    adcp->adc_tagp->THRHLR_2[4].R = adcp->grpp->thrhlr[8];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR9
    adcp->adc_tagp->THRHLR_2[5].R = adcp->grpp->thrhlr[9];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR10
    adcp->adc_tagp->THRHLR_2[6].R = adcp->grpp->thrhlr[10];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR11
    adcp->adc_tagp->THRHLR_2[7].R = adcp->grpp->thrhlr[11];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR12
    adcp->adc_tagp->THRHLR_2[8].R = adcp->grpp->thrhlr[12];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR13
    adcp->adc_tagp->THRHLR_2[9].R = adcp->grpp->thrhlr[13];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR14
    adcp->adc_tagp->THRHLR_2[10].R = adcp->grpp->thrhlr[14];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR15
    adcp->adc_tagp->THRHLR_2[11].R = adcp->grpp->thrhlr[15];
#endif
#if SPC5_ADC_ADC0_HAS_CWENR0
    adcp->adc_tagp->CWENR[0].R = adcp->grpp->cwenr[0];
#endif
#if SPC5_ADC_ADC0_HAS_CWENR1
    adcp->adc_tagp->CWENR[1].R = adcp->grpp->cwenr[1];
#endif
#if SPC5_ADC_ADC0_HAS_CWENR2
    adcp->adc_tagp->CWENR[2].R = adcp->grpp->cwenr[2];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL0
    adcp->adc_tagp->CWSEL[0].R = adcp->grpp->cwsel[0];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL1
    adcp->adc_tagp->CWSEL[1].R = adcp->grpp->cwsel[1];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL2
    adcp->adc_tagp->CWSEL[2].R = adcp->grpp->cwsel[2];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL3
    adcp->adc_tagp->CWSEL[3].R = adcp->grpp->cwsel[3];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL4
    adcp->adc_tagp->CWSEL[4].R = adcp->grpp->cwsel[4];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL5
    adcp->adc_tagp->CWSEL[5].R = adcp->grpp->cwsel[5];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL6
    adcp->adc_tagp->CWSEL[6].R = adcp->grpp->cwsel[6];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL7
    adcp->adc_tagp->CWSEL[7].R = adcp->grpp->cwsel[7];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL8
    adcp->adc_tagp->CWSEL[8].R = adcp->grpp->cwsel[8];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL9
    adcp->adc_tagp->CWSEL[9].R = adcp->grpp->cwsel[9];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL10
    adcp->adc_tagp->CWSEL[10].R = adcp->grpp->cwsel[10];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL11
    adcp->adc_tagp->CWSEL[11].R = adcp->grpp->cwsel[11];
#endif
  }
#endif
#if SPC5_ADC_USE_ADC1
  if (&ADCD2 == adcp) {
#if SPC5_ADC_ADC1_HAS_THRHLR0
    adcp->adc_tagp->THRHLR[0].R = adcp->grpp->thrhlr[0];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR1
    adcp->adc_tagp->THRHLR[1].R = adcp->grpp->thrhlr[1];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR2
    adcp->adc_tagp->THRHLR[2].R = adcp->grpp->thrhlr[2];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR3
    adcp->adc_tagp->THRHLR[3].R = adcp->grpp->thrhlr[3];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR4
    adcp->adc_tagp->THRHLR_2[0].R = adcp->grpp->thrhlr[4];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR5
    adcp->adc_tagp->THRHLR_2[1].R = adcp->grpp->thrhlr[5];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR6
    adcp->adc_tagp->THRHLR_2[2].R = adcp->grpp->thrhlr[6];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR7
    adcp->adc_tagp->THRHLR_2[3].R = adcp->grpp->thrhlr[7];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR8
    adcp->adc_tagp->THRHLR_2[4].R = adcp->grpp->thrhlr[8];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR9
    adcp->adc_tagp->THRHLR_2[5].R = adcp->grpp->thrhlr[9];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR10
    adcp->adc_tagp->THRHLR_2[6].R = adcp->grpp->thrhlr[10];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR11
    adcp->adc_tagp->THRHLR_2[7].R = adcp->grpp->thrhlr[11];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR12
    adcp->adc_tagp->THRHLR_2[8].R = adcp->grpp->thrhlr[12];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR13
    adcp->adc_tagp->THRHLR_2[9].R = adcp->grpp->thrhlr[13];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR14
    adcp->adc_tagp->THRHLR_2[10].R = adcp->grpp->thrhlr[14];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR15
    adcp->adc_tagp->THRHLR_2[11].R = adcp->grpp->thrhlr[15];
#endif
#if SPC5_ADC_ADC1_HAS_CWENR0
    adcp->adc_tagp->CWENR[0].R = adcp->grpp->cwenr[0];
#endif
#if SPC5_ADC_ADC1_HAS_CWENR1
    adcp->adc_tagp->CWENR[1].R = adcp->grpp->cwenr[1];
#endif
#if SPC5_ADC_ADC1_HAS_CWENR2
    adcp->adc_tagp->CWENR[2].R = adcp->grpp->cwenr[2];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL0
    adcp->adc_tagp->CWSEL[0].R = adcp->grpp->cwsel[0];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL1
    adcp->adc_tagp->CWSEL[1].R = adcp->grpp->cwsel[1];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL2
    adcp->adc_tagp->CWSEL[2].R = adcp->grpp->cwsel[2];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL3
    adcp->adc_tagp->CWSEL[3].R = adcp->grpp->cwsel[3];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL4
    adcp->adc_tagp->CWSEL[4].R = adcp->grpp->cwsel[4];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL5
    adcp->adc_tagp->CWSEL[5].R = adcp->grpp->cwsel[5];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL6
    adcp->adc_tagp->CWSEL[6].R = adcp->grpp->cwsel[6];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL7
    adcp->adc_tagp->CWSEL[7].R = adcp->grpp->cwsel[7];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL8
    adcp->adc_tagp->CWSEL[8].R = adcp->grpp->cwsel[8];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL9
    adcp->adc_tagp->CWSEL[9].R = adcp->grpp->cwsel[9];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL10
    adcp->adc_tagp->CWSEL[10].R = adcp->grpp->cwsel[10];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL11
    adcp->adc_tagp->CWSEL[11].R = adcp->grpp->cwsel[11];
#endif
  }
#endif
#endif

  /* Enables the watchdog interrupts.*/
  adcp->adc_tagp->WTIMR.R = adcp->grpp->wtimr;

  /* Active ADC channels for the conversion and sets the ADC DMA channels. (if enable)*/
  if (adcp->grpp->init_channel < 32U) {
    ch_mask = ((1U << adcp->grpp->num_channels) - 1U) << adcp->grpp->init_channel;
    dma_mask = 1U << (adcp->grpp->init_channel + adcp->grpp->num_channels -1U);

#if SPC5_ADC_USE_ADC0
    if (&ADCD1 == adcp) {
#if SPC5_ADC_ADC0_HAS_NCMR0
      adcp->adc_tagp->NCMR[0].R = ch_mask;
      adcp->adc_tagp->DMAR[0].R = dma_mask;
#endif
    }
#endif
#if SPC5_ADC_USE_ADC1
    if (&ADCD2 == adcp) {
#if SPC5_ADC_ADC1_HAS_NCMR0
      adcp->adc_tagp->NCMR[0].R = ch_mask;
      adcp->adc_tagp->DMAR[0].R = dma_mask;
#endif
    }
#endif
  } else if ((adcp->grpp->init_channel >= 32U) && (adcp->grpp->init_channel < 64U)) {
    ch_mask = ((1U << adcp->grpp->num_channels) - 1U) << (adcp->grpp->init_channel - 32U);
    dma_mask = 1U << (adcp->grpp->init_channel - 32U + adcp->grpp->num_channels -1U);
#if SPC5_ADC_USE_ADC0
    if (&ADCD1 == adcp) {
#if SPC5_ADC_ADC0_HAS_NCMR1
      adcp->adc_tagp->NCMR[1].R = ch_mask;
      adcp->adc_tagp->DMAR[1].R = dma_mask;
#endif
    }
#endif
#if SPC5_ADC_USE_ADC1
    if (&ADCD2 == adcp) {
#if SPC5_ADC_ADC1_HAS_NCMR1
      adcp->adc_tagp->NCMR[1].R = ch_mask;
      adcp->adc_tagp->DMAR[1].R = dma_mask;
#endif
    }
#endif
  } else if ((adcp->grpp->init_channel >= 64U) && (adcp->grpp->init_channel < 96U)) {
    ch_mask = ((1U << adcp->grpp->num_channels) - 1U) << (adcp->grpp->init_channel - 64U);
    dma_mask = 1U << (adcp->grpp->init_channel - 64U + adcp->grpp->num_channels -1U);
#if SPC5_ADC_USE_ADC0
    if (&ADCD1 == adcp) {
#if SPC5_ADC_ADC0_HAS_NCMR2
      adcp->adc_tagp->NCMR[2].R = ch_mask;
      adcp->adc_tagp->DMAR[2].R = dma_mask;
#endif
    }
#endif
#if SPC5_ADC_USE_ADC1
    if (&ADCD2 == adcp) {
#if SPC5_ADC_ADC1_HAS_NCMR2
      adcp->adc_tagp->NCMR[2].R = ch_mask;
      adcp->adc_tagp->DMAR[2].R = dma_mask;
#endif
    }
#endif
  }

  /* Sets ADC conversion timing register.*/
#if SPC5_ADC_USE_ADC0
  if (&ADCD1 == adcp) {
#if SPC5_ADC_ADC0_HAS_CTR0
    adcp->adc_tagp->CTR[0].R = adcp->grpp->ctr[0];
#endif
#if SPC5_ADC_ADC0_HAS_CTR1
    adcp->adc_tagp->CTR[1].R = adcp->grpp->ctr[1];
#endif
#if SPC5_ADC_ADC0_HAS_CTR2
    adcp->adc_tagp->CTR[2].R = adcp->grpp->ctr[2];
#endif
  }
#endif
#if SPC5_ADC_USE_ADC1
  if (&ADCD2 == adcp) {
#if SPC5_ADC_ADC1_HAS_CTR0
    adcp->adc_tagp->CTR[0].R = adcp->grpp->ctr[0];
#endif
#if SPC5_ADC_ADC1_HAS_CTR1
    adcp->adc_tagp->CTR[1].R = adcp->grpp->ctr[1];
#endif
#if SPC5_ADC_ADC1_HAS_CTR2
    adcp->adc_tagp->CTR[2].R = adcp->grpp->ctr[2];
#endif
  }
#endif
}

#else /* SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF */
/**
 * @brief   Initialize registers for non-DMA case
 *
 * @param[in] adcp      parameter for the registered function
 *
 * @notapi
 */
static void adc_initialize_registers_nondma(ADCDriver *adcp) {
  uint32_t ch_mask;

  /* Sets thresholds’ values and active watchdog threshold interrupts if any.*/
#if SPC5_ADC_HAS_TRC
  adcp->adc_tagp->TRC[0].R = adcp->grpp->trc[0];
  adcp->adc_tagp->TRC[1].R = adcp->grpp->trc[1];
  adcp->adc_tagp->TRC[2].R = adcp->grpp->trc[2];
  adcp->adc_tagp->TRC[3].R = adcp->grpp->trc[3];
#if SPC5_ADC_USE_ADC0
  if (&ADCD1 == adcp) {
#if SPC5_ADC_ADC0_HAS_THRHLR0
    adcp->adc_tagp->THRHLR[0].R = adcp->grpp->thrhlr[0];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR1
    adcp->adc_tagp->THRHLR[1].R = adcp->grpp->thrhlr[1];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR2
    adcp->adc_tagp->THRHLR[2].R = adcp->grpp->thrhlr[2];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR3
    adcp->adc_tagp->THRHLR[3].R = adcp->grpp->thrhlr[3];
#endif
  }
#endif
#if SPC5_ADC_USE_ADC1
  if (&ADCD2 == adcp) {
#if SPC5_ADC_ADC1_HAS_THRHLR0
    adcp->adc_tagp->THRHLR[0].R = adcp->grpp->thrhlr[0];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR1
    adcp->adc_tagp->THRHLR[1].R = adcp->grpp->thrhlr[1];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR2
    adcp->adc_tagp->THRHLR[2].R = adcp->grpp->thrhlr[2];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR3
    adcp->adc_tagp->THRHLR[3].R = adcp->grpp->thrhlr[3];
#endif
  }
#endif
#else
#if SPC5_ADC_USE_ADC0
  if (&ADCD1 == adcp) {
#if SPC5_ADC_ADC0_HAS_THRHLR0
    adcp->adc_tagp->THRHLR[0].R = adcp->grpp->thrhlr[0];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR1
    adcp->adc_tagp->THRHLR[1].R = adcp->grpp->thrhlr[1];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR2
    adcp->adc_tagp->THRHLR[2].R = adcp->grpp->thrhlr[2];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR3
    adcp->adc_tagp->THRHLR[3].R = adcp->grpp->thrhlr[3];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR4
    adcp->adc_tagp->THRHLR_2[0].R = adcp->grpp->thrhlr[4];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR5
    adcp->adc_tagp->THRHLR_2[1].R = adcp->grpp->thrhlr[5];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR6
    adcp->adc_tagp->THRHLR_2[2].R = adcp->grpp->thrhlr[6];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR7
    adcp->adc_tagp->THRHLR_2[3].R = adcp->grpp->thrhlr[7];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR8
    adcp->adc_tagp->THRHLR_2[4].R = adcp->grpp->thrhlr[8];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR9
    adcp->adc_tagp->THRHLR_2[5].R = adcp->grpp->thrhlr[9];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR10
    adcp->adc_tagp->THRHLR_2[6].R = adcp->grpp->thrhlr[10];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR11
    adcp->adc_tagp->THRHLR_2[7].R = adcp->grpp->thrhlr[11];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR12
    adcp->adc_tagp->THRHLR_2[8].R = adcp->grpp->thrhlr[12];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR13
    adcp->adc_tagp->THRHLR_2[9].R = adcp->grpp->thrhlr[13];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR14
    adcp->adc_tagp->THRHLR_2[10].R = adcp->grpp->thrhlr[14];
#endif
#if SPC5_ADC_ADC0_HAS_THRHLR15
    adcp->adc_tagp->THRHLR_2[11].R = adcp->grpp->thrhlr[15];
#endif
#if SPC5_ADC_ADC0_HAS_CWENR0
    adcp->adc_tagp->CWENR[0].R = adcp->grpp->cwenr[0];
#endif
#if SPC5_ADC_ADC0_HAS_CWENR1
    adcp->adc_tagp->CWENR[1].R = adcp->grpp->cwenr[1];
#endif
#if SPC5_ADC_ADC0_HAS_CWENR2
    adcp->adc_tagp->CWENR[2].R = adcp->grpp->cwenr[2];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL0
    adcp->adc_tagp->CWSEL[0].R = adcp->grpp->cwsel[0];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL1
    adcp->adc_tagp->CWSEL[1].R = adcp->grpp->cwsel[1];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL2
    adcp->adc_tagp->CWSEL[2].R = adcp->grpp->cwsel[2];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL3
    adcp->adc_tagp->CWSEL[3].R = adcp->grpp->cwsel[3];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL4
    adcp->adc_tagp->CWSEL[4].R = adcp->grpp->cwsel[4];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL5
    adcp->adc_tagp->CWSEL[5].R = adcp->grpp->cwsel[5];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL6
    adcp->adc_tagp->CWSEL[6].R = adcp->grpp->cwsel[6];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL7
    adcp->adc_tagp->CWSEL[7].R = adcp->grpp->cwsel[7];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL8
    adcp->adc_tagp->CWSEL[8].R = adcp->grpp->cwsel[8];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL9
    adcp->adc_tagp->CWSEL[9].R = adcp->grpp->cwsel[9];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL10
    adcp->adc_tagp->CWSEL[10].R = adcp->grpp->cwsel[10];
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL11
    adcp->adc_tagp->CWSEL[11].R = adcp->grpp->cwsel[11];
#endif
  }
#endif
#if SPC5_ADC_USE_ADC1
  if (&ADCD2 == adcp) {
#if SPC5_ADC_ADC1_HAS_THRHLR0
    adcp->adc_tagp->THRHLR[0].R = adcp->grpp->thrhlr[0];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR1
    adcp->adc_tagp->THRHLR[1].R = adcp->grpp->thrhlr[1];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR2
    adcp->adc_tagp->THRHLR[2].R = adcp->grpp->thrhlr[2];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR3
    adcp->adc_tagp->THRHLR[3].R = adcp->grpp->thrhlr[3];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR4
    adcp->adc_tagp->THRHLR_2[0].R = adcp->grpp->thrhlr[4];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR5
    adcp->adc_tagp->THRHLR_2[1].R = adcp->grpp->thrhlr[5];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR6
    adcp->adc_tagp->THRHLR_2[2].R = adcp->grpp->thrhlr[6];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR7
    adcp->adc_tagp->THRHLR_2[3].R = adcp->grpp->thrhlr[7];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR8
    adcp->adc_tagp->THRHLR_2[4].R = adcp->grpp->thrhlr[8];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR9
    adcp->adc_tagp->THRHLR_2[5].R = adcp->grpp->thrhlr[9];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR10
    adcp->adc_tagp->THRHLR_2[6].R = adcp->grpp->thrhlr[10];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR11
    adcp->adc_tagp->THRHLR_2[7].R = adcp->grpp->thrhlr[11];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR12
    adcp->adc_tagp->THRHLR_2[8].R = adcp->grpp->thrhlr[12];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR13
    adcp->adc_tagp->THRHLR_2[9].R = adcp->grpp->thrhlr[13];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR14
    adcp->adc_tagp->THRHLR_2[10].R = adcp->grpp->thrhlr[14];
#endif
#if SPC5_ADC_ADC1_HAS_THRHLR15
    adcp->adc_tagp->THRHLR_2[11].R = adcp->grpp->thrhlr[15];
#endif
#if SPC5_ADC_ADC1_HAS_CWENR0
    adcp->adc_tagp->CWENR[0].R = adcp->grpp->cwenr[0];
#endif
#if SPC5_ADC_ADC1_HAS_CWENR1
    adcp->adc_tagp->CWENR[1].R = adcp->grpp->cwenr[1];
#endif
#if SPC5_ADC_ADC1_HAS_CWENR2
    adcp->adc_tagp->CWENR[2].R = adcp->grpp->cwenr[2];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL0
    adcp->adc_tagp->CWSEL[0].R = adcp->grpp->cwsel[0];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL1
    adcp->adc_tagp->CWSEL[1].R = adcp->grpp->cwsel[1];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL2
    adcp->adc_tagp->CWSEL[2].R = adcp->grpp->cwsel[2];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL3
    adcp->adc_tagp->CWSEL[3].R = adcp->grpp->cwsel[3];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL4
    adcp->adc_tagp->CWSEL[4].R = adcp->grpp->cwsel[4];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL5
    adcp->adc_tagp->CWSEL[5].R = adcp->grpp->cwsel[5];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL6
    adcp->adc_tagp->CWSEL[6].R = adcp->grpp->cwsel[6];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL7
    adcp->adc_tagp->CWSEL[7].R = adcp->grpp->cwsel[7];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL8
    adcp->adc_tagp->CWSEL[8].R = adcp->grpp->cwsel[8];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL9
    adcp->adc_tagp->CWSEL[9].R = adcp->grpp->cwsel[9];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL10
    adcp->adc_tagp->CWSEL[10].R = adcp->grpp->cwsel[10];
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL11
    adcp->adc_tagp->CWSEL[11].R = adcp->grpp->cwsel[11];
#endif
  }
#endif
#endif

  /* Enables the watchdog interrupts.*/
  adcp->adc_tagp->WTIMR.R = adcp->grpp->wtimr;

  /* Enable the ADC Interrupt */
  /* Set the interrupt Mask Register */
  adcp->adc_tagp->IMR.R=ADC_IMR_MSKEOC|ADC_IMR_MSKECH;

  /* Set the Channel Interrupt Mask register */
#if SPC5_ADC_ADC0_HAS_CIMR0
  adcp->adc_tagp->CIMR[0].R=0x0000FFFF;
#endif
#if SPC5_ADC_ADC0_HAS_CIMR1
  adcp->adc_tagp->CIMR[1].R=0x0000FFFF;
#endif
#if SPC5_ADC_ADC0_HAS_CIMR2
  adcp->adc_tagp->CIMR[2].R=0x0000FFFF;
#endif

  /* Active ADC channels for the conversion and sets the ADC DMA channels. (if enable)*/
  if (adcp->grpp->init_channel < 32U) {
    ch_mask = ((1U << adcp->grpp->num_channels) - 1U) << adcp->grpp->init_channel;
#if SPC5_ADC_USE_ADC0
    if (&ADCD1 == adcp) {
#if SPC5_ADC_ADC0_HAS_NCMR0
      adcp->adc_tagp->NCMR[0].R = ch_mask;
#endif
    }
#endif
#if SPC5_ADC_USE_ADC1
    if (&ADCD2 == adcp) {
#if SPC5_ADC_ADC1_HAS_NCMR0
      adcp->adc_tagp->NCMR[0].R = ch_mask;
#endif
    }
#endif
  } else if ((adcp->grpp->init_channel >= 32U) && (adcp->grpp->init_channel < 64U)) {
    ch_mask = ((1U << adcp->grpp->num_channels) - 1U) << (adcp->grpp->init_channel - 32U);
#if SPC5_ADC_USE_ADC0
    if (&ADCD1 == adcp) {
#if SPC5_ADC_ADC0_HAS_NCMR1
      adcp->adc_tagp->NCMR[1].R = ch_mask;
#endif
    }
#endif
#if SPC5_ADC_USE_ADC1
    if (&ADCD2 == adcp) {
#if SPC5_ADC_ADC1_HAS_NCMR1
      adcp->adc_tagp->NCMR[1].R = ch_mask;
#endif
    }
#endif
  } else if ((adcp->grpp->init_channel >= 64U) && (adcp->grpp->init_channel < 96U)) {
    ch_mask = ((1U << adcp->grpp->num_channels) - 1U) << (adcp->grpp->init_channel - 64U);
#if SPC5_ADC_USE_ADC0
    if (&ADCD1 == adcp) {
#if SPC5_ADC_ADC0_HAS_NCMR2
      adcp->adc_tagp->NCMR[2].R = ch_mask;
#endif
    }
#endif
#if SPC5_ADC_USE_ADC1
    if (&ADCD2 == adcp) {
#if SPC5_ADC_ADC1_HAS_NCMR2
      adcp->adc_tagp->NCMR[2].R = ch_mask;
#endif
    }
#endif
  }

  /* Sets ADC conversion timing register.*/
#if SPC5_ADC_USE_ADC0
  if (&ADCD1 == adcp) {
#if SPC5_ADC_ADC0_HAS_CTR0
    adcp->adc_tagp->CTR[0].R = adcp->grpp->ctr[0];
#endif
#if SPC5_ADC_ADC0_HAS_CTR1
    adcp->adc_tagp->CTR[1].R = adcp->grpp->ctr[1];
#endif
#if SPC5_ADC_ADC0_HAS_CTR2
    adcp->adc_tagp->CTR[2].R = adcp->grpp->ctr[2];
#endif
  }
#endif
#if SPC5_ADC_USE_ADC1
  if (&ADCD2 == adcp) {
#if SPC5_ADC_ADC1_HAS_CTR0
    adcp->adc_tagp->CTR[0].R = adcp->grpp->ctr[0];
#endif
#if SPC5_ADC_ADC1_HAS_CTR1
    adcp->adc_tagp->CTR[1].R = adcp->grpp->ctr[1];
#endif
#if SPC5_ADC_ADC1_HAS_CTR2
    adcp->adc_tagp->CTR[2].R = adcp->grpp->ctr[2];
#endif
  }
#endif
}
#endif /* SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level ADC driver initialization.
 *
 * @notapi
 */
void adc_lld_init(void) {

#if SPC5_ADC_USE_ADC0
  /* Driver initialization.*/
  adcObjectInit(&ADCD1);
#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON
  ADCD1.adc_dma_channel = EDMA_ERROR;
#else /* SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF */
  ADCD1.rx_ptr   = NULL;
  ADCD1.rx_cnt   = 0;
#endif /* SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF */
  ADCD1.adc_tagp = &SPC5_ADC_0;
#endif /* SPC5_ADC_USE_ADC0 */

#if SPC5_ADC_USE_ADC1
  /* Driver initialization.*/
  adcObjectInit(&ADCD2);
#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON
  ADCD2.adc_dma_channel = EDMA_ERROR;
#else /* SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF */
  ADCD2.rx_ptr   = NULL;
  ADCD2.rx_cnt   = 0;
#endif /* SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF */
  ADCD2.adc_tagp = &SPC5_ADC_1;
#endif /* SPC5_ADC_USE_ADC1 */

#if SPC5_ADC_USE_ADC0
  INTC.PSR[SPC5_ADC0_WD_NUMBER].R = SPC5_ADC_ADC0_WD_PRIORITY;
#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF
  INTC.PSR[SPC5_ADC0_EOC_NUMBER].R = SPC5_ADC_ADC0_EOC_PRIORITY;
#endif

  /* Sets peripheral clock.*/
  halSPCSetPeripheralClockMode(SPC5_ADC0_PCTL, SPC5_ADC_ADC0_START_PCTL);
  ADCD1.adc_tagp->MCR.B.ADCLKSEL = SPC5_ADC_ADC0_CLK_FREQUENCY;
  halSPCSetPeripheralClockMode(SPC5_ADC0_PCTL, SPC5_ADC_ADC0_STOP_PCTL);
#endif

#if SPC5_ADC_USE_ADC1
  INTC.PSR[SPC5_ADC1_WD_NUMBER].R = SPC5_ADC_ADC1_WD_PRIORITY;
#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF
  INTC.PSR[SPC5_ADC1_EOC_NUMBER].R = SPC5_ADC_ADC1_EOC_PRIORITY;
#endif

  /* Sets peripheral clock.*/
  halSPCSetPeripheralClockMode(SPC5_ADC1_PCTL, SPC5_ADC_ADC1_START_PCTL);
  ADCD2.adc_tagp->MCR.B.ADCLKSEL = SPC5_ADC_ADC1_CLK_FREQUENCY;
  halSPCSetPeripheralClockMode(SPC5_ADC1_PCTL, SPC5_ADC_ADC1_STOP_PCTL);
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

  if (adcp->state == ADC_STOP) {
    /* Enables the peripheral.*/
#if SPC5_ADC_USE_ADC0
    if (&ADCD1 == adcp) {
#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON
      adcp->adc_dma_channel = edmaChannelAllocate(&adc_adc0_dma_config);
#endif /* SPC5_ADC_DMA_MODE */
      halSPCSetPeripheralClockMode(SPC5_ADC0_PCTL,
                                   SPC5_ADC_ADC0_START_PCTL);
    }
#endif /* SPC5_ADC_USE_ADC0 */

#if SPC5_ADC_USE_ADC1
    if (&ADCD2 == adcp) {
#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON
      adcp->adc_dma_channel = edmaChannelAllocate(&adc_adc1_dma_config);
#endif
      halSPCSetPeripheralClockMode(SPC5_ADC1_PCTL,
                                   SPC5_ADC_ADC1_START_PCTL);
    }
#endif /* SPC5_ADC_USE_ADC1 */

#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON
    osalDbgAssert((adcp->adc_dma_channel != EDMA_ERROR),
                  "adc_lld_start(), DMA channel cannot be allocated");
#endif

    /* Sets ADC Normal Mode.*/
    adcp->adc_tagp->MCR.B.PWDN = 0;

    /* Sets MCR Register.*/
    adcp->adc_tagp->MCR.R = ADC_MCR_OWREN | ADC_MCR_MODE;

    /* Sets the Auto-Clock-off mode.*/
#if SPC5_ADC_USE_ADC0
    if (&ADCD1 == adcp) {
      if (SPC5_ADC_ADC0_AUTO_CLOCK_OFF) {
        adcp->adc_tagp->MCR.R = ADC_MCR_ACKO;
      }
    }
#endif /* SPC5_ADC_USE_ADC0 */

#if SPC5_ADC_USE_ADC1
    if (&ADCD2 == adcp) {
      if (SPC5_ADC_ADC1_AUTO_CLOCK_OFF) {
        adcp->adc_tagp->MCR.R = ADC_MCR_ACKO;
      }
    }
#endif /* SPC5_ADC_USE_ADC1 */

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

  if (adcp->state == ADC_READY) {
    /* Resets the peripheral.*/

#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON
    /* Releases the allocated DMA channel.*/
    edmaChannelRelease(adcp->adc_dma_channel);
#endif

    /* Clears thresholds’ values and deactive watchdog threshold interrupts.*/
#if SPC5_ADC_HAS_TRC
    adcp->adc_tagp->TRC[0].R = 0;
    adcp->adc_tagp->TRC[1].R = 0;
    adcp->adc_tagp->TRC[2].R = 0;
    adcp->adc_tagp->TRC[3].R = 0;
#else
#if SPC5_ADC_USE_ADC0
    if (&ADCD1 == adcp) {
#if SPC5_ADC_ADC0_HAS_CWSEL0
      adcp->adc_tagp->CWSEL[0].R = 0;
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL1
      adcp->adc_tagp->CWSEL[1].R = 0;
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL2
      adcp->adc_tagp->CWSEL[2].R = 0;
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL3
      adcp->adc_tagp->CWSEL[3].R = 0;
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL4
      adcp->adc_tagp->CWSEL[4].R = 0;
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL5
      adcp->adc_tagp->CWSEL[5].R = 0;
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL6
      adcp->adc_tagp->CWSEL[6].R = 0;
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL7
      adcp->adc_tagp->CWSEL[7].R = 0;
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL8
      adcp->adc_tagp->CWSEL[8].R = 0;
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL9
      adcp->adc_tagp->CWSEL[9].R = 0;
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL10
      adcp->adc_tagp->CWSEL[10].R = 0;
#endif
#if SPC5_ADC_ADC0_HAS_CWSEL11
      adcp->adc_tagp->CWSEL[11].R = 0;
#endif
#if SPC5_ADC_ADC0_HAS_CWENR0
      adcp->adc_tagp->CWENR[0].R = 0;
#endif
#if SPC5_ADC_ADC0_HAS_CWENR1
      adcp->adc_tagp->CWENR[1].R = 0;
#endif
#if SPC5_ADC_ADC0_HAS_CWENR2
      adcp->adc_tagp->CWENR[2].R = 0;
#endif
    }
#endif
#if SPC5_ADC_USE_ADC1
    if (&ADCD2 == adcp) {
#if SPC5_ADC_ADC1_HAS_CWSEL0
      adcp->adc_tagp->CWSEL[0].R = 0;
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL1
      adcp->adc_tagp->CWSEL[1].R = 0;
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL2
      adcp->adc_tagp->CWSEL[2].R = 0;
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL3
      adcp->adc_tagp->CWSEL[3].R = 0;
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL4
      adcp->adc_tagp->CWSEL[4].R = 0;
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL5
      adcp->adc_tagp->CWSEL[5].R = 0;
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL6
      adcp->adc_tagp->CWSEL[6].R = 0;
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL7
      adcp->adc_tagp->CWSEL[7].R = 0;
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL8
      adcp->adc_tagp->CWSEL[8].R = 0;
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL9
      adcp->adc_tagp->CWSEL[9].R = 0;
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL10
      adcp->adc_tagp->CWSEL[10].R = 0;
#endif
#if SPC5_ADC_ADC1_HAS_CWSEL11
      adcp->adc_tagp->CWSEL[11].R = 0;
#endif
#if SPC5_ADC_ADC1_HAS_CWENR0
      adcp->adc_tagp->CWENR[0].R = 0;
#endif
#if SPC5_ADC_ADC1_HAS_CWENR1
      adcp->adc_tagp->CWENR[1].R = 0;
#endif
#if SPC5_ADC_ADC1_HAS_CWENR2
      adcp->adc_tagp->CWENR[2].R = 0;
#endif
    }
#endif
#endif

    /* Disables ADC channels and the ADC DMA channels.*/
#if SPC5_ADC_USE_ADC0
    if (&ADCD1 == adcp) {
#if SPC5_ADC_ADC0_HAS_NCMR0
      adcp->adc_tagp->NCMR[0].R = 0;
#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON
      adcp->adc_tagp->DMAR[0].R = 0;
#endif /* SPC5_ADC_DMA_MODE */
#endif
#if SPC5_ADC_ADC0_HAS_NCMR1
      adcp->adc_tagp->NCMR[1].R = 0;
#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON
      adcp->adc_tagp->DMAR[1].R = 0;
#endif /* SPC5_ADC_DMA_MODE */
#endif
#if SPC5_ADC_ADC0_HAS_NCMR2
      adcp->adc_tagp->NCMR[2].R = 0;
#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON
      adcp->adc_tagp->DMAR[2].R = 0;
#endif /* SPC5_ADC_DMA_MODE */
#endif
    }
#endif
#if SPC5_ADC_USE_ADC1
    if (&ADCD2 == adcp) {
#if SPC5_ADC_ADC1_HAS_NCMR0
      adcp->adc_tagp->NCMR[0].R = 0;
#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON
      adcp->adc_tagp->DMAR[0].R = 0;
#endif /* SPC5_ADC_DMA_MODE */
#endif
#if SPC5_ADC_ADC1_HAS_NCMR1
      adcp->adc_tagp->NCMR[1].R = 0;
#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON
      adcp->adc_tagp->DMAR[1].R = 0;
#endif /* SPC5_ADC_DMA_MODE */
#endif
#if SPC5_ADC_ADC1_HAS_NCMR2
      adcp->adc_tagp->NCMR[2].R = 0;
#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON
      adcp->adc_tagp->DMAR[2].R = 0;
#endif /* SPC5_ADC_DMA_MODE */
#endif
    }
#endif

    /* Disables the watchdog interrupts if any.*/
    adcp->adc_tagp->WTIMR.R = 0;

    /* Clears watchdog interrupts if any.*/
    adcp->adc_tagp->WTISR.R = 0xFFFFFFFF;

    /* Puts the ADC Peripheral in Power-Down Mode.*/
    adcp->adc_tagp->MCR.B.PWDN = 1U;

    /* Disables the peripheral.*/
#if SPC5_ADC_USE_ADC0
    if (&ADCD1 == adcp) {
      halSPCSetPeripheralClockMode(SPC5_ADC0_PCTL,
                                   SPC5_ADC_ADC0_STOP_PCTL);
    }
#endif

#if SPC5_ADC_USE_ADC1
    if (&ADCD2 == adcp) {
      halSPCSetPeripheralClockMode(SPC5_ADC1_PCTL,
                                   SPC5_ADC_ADC1_STOP_PCTL);
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

#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON
  /* Setting up DMA TCD parameters.*/
  edmaChannelSetup(adcp->adc_dma_channel,                                   /* channel.                 */
                   ((uint8_t *)&adcp->adc_tagp->CDR[adcp->grpp->init_channel].R) + 2,   /* src.         */
                   adcp->samples,                                           /* dst.                     */
                   4,                                                       /* soff, advance by four.   */
                   2,                                                       /* doff, advance by two.    */
                   1,                                                       /* ssize, 16 bits transfers.*/
                   1,                                                       /* dsize, 16 bits transfers.*/
                   (0xBFFFFFFF) &                                            \
                   ((CPL2((uint32_t)adcp->grpp->num_channels * 4) << 10U) |  \
                   (2U * adcp->grpp->num_channels)),                        /* mloff and nbytes.        */
                   (uint32_t)adcp->depth,                                   /* iter.                    */
                   CPL2((uint32_t)adcp->grpp->num_channels * 4),            /* slast.                      */
                   CPL2((uint32_t)adcp->grpp->num_channels *
                        (uint32_t)adcp->depth *
                        sizeof(adcsample_t)),                               /* dlast.                   */
                   EDMA_TCD_MODE_DREQ | EDMA_TCD_MODE_INT_END |
                   ((adcp->depth > 1) ? EDMA_TCD_MODE_INT_HALF: 0));        /* mode.                    */

  /* Active DMA.*/
  adcp->adc_tagp->DMAE.R = ADC_DMAE_DMAEN;

  /* initialize registers for dma cases */
  adc_initialize_registers_dma(adcp);
#else /* SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF */
  /* initialize rx_ptr and rx_cnt */
  adcp->rx_ptr   = adcp->samples;
  adcp->rx_cnt   = adcp->depth * adcp->grpp->num_channels;

  /* initialize registers for non-dma cases */
  adc_initialize_registers_nondma(adcp);
#endif /* SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF */

#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON
  /* Starting DMA channels.*/
  edmaChannelStart(adcp->adc_dma_channel);
#endif /* SPC5_ADC_DMA_MODE */

  /* Starts conversion.*/
  adcp->adc_tagp->MCR.B.NSTART = 1U;
}

/**
 * @brief   Stops an ongoing conversion.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_stop_conversion(ADCDriver *adcp) {

#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON
  /* Stop DMA channel.*/
  edmaChannelStop(adcp->adc_dma_channel);
#endif /* SPC5_ADC_DMA_MODE */

  /* Stops conversion.*/
  adcp->adc_tagp->MCR.B.NSTART = 0;

  /* Abort the conversion */
  adcp->adc_tagp->MCR.B.ABORT = 1;

  /* Disables Interrupts and DMA.*/
  adcp->adc_tagp->WTIMR.R = 0;

#if SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON
  adcp->adc_tagp->DMAE.R = 0;
#else
  /* Disable ADC Interrupts */
  adcp->adc_tagp->IMR.R=0;

  /* Disable the Channel Interrupt Mask register */
#if SPC5_ADC_ADC0_HAS_CIMR0
  adcp->adc_tagp->CIMR[0].R=0;
#endif
#if SPC5_ADC_ADC0_HAS_CIMR1
  adcp->adc_tagp->CIMR[1].R=0;
#endif
#if SPC5_ADC_ADC0_HAS_CIMR2
  adcp->adc_tagp->CIMR[2].R=0;
#endif

  /* Clears all Interrupts.*/
  adcp->adc_tagp->ISR.R=0;
#endif /* SPC5_ADC_DMA_MODE */

  /* Clears all Interrupts.*/
  adcp->adc_tagp->WTISR.R = 0;
}

/**
 * @brief   Returns the Watchdog Threshold Interrupt Status Register image.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 * @return              the WTISR register image.
 *
 * @notapi
 */
uint32_t adc_get_awd_err(ADCDriver *adcp) {

  return adcp->adc_awd_err;
}

/**
 * @brief   Clear the Watchdog Threshold Interrupt Status Register image.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_clear_awd_err(ADCDriver *adcp) {

  adcp->adc_awd_err = 0;
}

#endif /* HAL_USE_ADC */

/** @} */
