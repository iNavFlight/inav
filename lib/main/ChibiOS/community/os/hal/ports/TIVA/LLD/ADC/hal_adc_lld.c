/*
    Copyright (C) 2014..2017 Marco Veeneman

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
 * @file    hal_adc_lld.c
 * @brief   PLATFORM ADC subsystem low level driver source.
 *
 * @addtogroup ADC
 * @{
 */

#include "hal.h"

#if (HAL_USE_ADC == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief   ADC0 driver identifier.*/
#if TIVA_ADC_USE_ADC0 || defined(__DOXYGEN__)
ADCDriver ADCD1;
#endif

/** @brief   ADC1 driver identifier.*/
#if TIVA_ADC_USE_ADC1 || defined(__DOXYGEN__)
ADCDriver ADCD2;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Common IRQ handler.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 */
static void serve_interrupt(ADCDriver *adcp)
{
  tiva_udma_table_entry_t *pri = &udmaControlTable.primary[adcp->dmanr];
  tiva_udma_table_entry_t *alt = &udmaControlTable.alternate[adcp->dmanr];

  if ((pri->chctl & UDMA_CHCTL_XFERMODE_M) == UDMA_CHCTL_XFERMODE_STOP) {
    /* Primary is used only for circular transfers */
    if (adcp->grpp->circular) {
      if (adcp->depth > 1) {
        _adc_isr_half_code(adcp);
      }

      /* Reconfigure DMA for new lower half transfer */
      pri->chctl = adcp->prictl;
    }
  }

  if ((alt->chctl & UDMA_CHCTL_XFERMODE_M) == UDMA_CHCTL_XFERMODE_STOP) {
    /* Alternate is used for both linear and circular transfers */
    _adc_isr_full_code(adcp);

    if (adcp->grpp->circular) {
      /* Reconfigure DMA for new upper half transfer */
      alt->chctl = adcp->altctl;
    }
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if TIVA_ADC_USE_ADC0
/**
 * @brief   ADC0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_ADC0_SEQ0_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  serve_interrupt(&ADCD1);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_ADC_USE_ADC1
/**
 * @brief   ADC1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_ADC1_SEQ0_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  serve_interrupt(&ADCD2);

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
void adc_lld_init(void)
{
#if TIVA_ADC_USE_ADC0
  /* Driver initialization.*/
  adcObjectInit(&ADCD1);
  ADCD1.adc = ADC0_BASE;
  ADCD1.dmanr = TIVA_ADC_ADC0_SS0_UDMA_CHANNEL;
  ADCD1.chnmap = TIVA_ADC_ADC0_SS0_UDMA_MAPPING;
#endif

#if TIVA_ADC_USE_ADC1
  /* Driver initialization.*/
  adcObjectInit(&ADCD2);
  ADCD2.adc = ADC1_BASE;
  ADCD2.dmanr = TIVA_ADC_ADC1_SS0_UDMA_CHANNEL;
  ADCD2.chnmap = TIVA_ADC_ADC1_SS0_UDMA_MAPPING;
#endif
}

/**
 * @brief   Configures and activates the ADC peripheral.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_start(ADCDriver *adcp)
{
  if (adcp->state == ADC_STOP) {
    /* Enables the peripheral.*/
#if TIVA_ADC_USE_ADC0
    if (&ADCD1 == adcp) {
      bool b;
      b = udmaChannelAllocate(adcp->dmanr);
      osalDbgAssert(!b, "channel already allocated");

      HWREG(SYSCTL_RCGCADC) |= (1 << 0);

      while (!(HWREG(SYSCTL_PRADC) & (1 << 0)))
        ;

      /* Only sequencer 0 is supported */
      nvicEnableVector(TIVA_ADC0_SEQ0_NUMBER, TIVA_ADC0_SEQ0_PRIORITY);
    }
#endif

#if TIVA_ADC_USE_ADC1
    if (&ADCD2 == adcp) {
      bool b;
      b = udmaChannelAllocate(adcp->dmanr);
      osalDbgAssert(!b, "channel already allocated");

      HWREG(SYSCTL_RCGCADC) |= (1 << 1);

      while (!(HWREG(SYSCTL_PRADC) & (1 << 1)))
        ;

      /* Only sequencer 0 is supported */
      nvicEnableVector(TIVA_ADC1_SEQ0_NUMBER, TIVA_ADC1_SEQ0_PRIORITY);
    }
#endif

    HWREG(UDMA_CHMAP0 + (adcp->dmanr / 8) * 4) |= (adcp->chnmap << (adcp->dmanr % 8));
  }
}

/**
 * @brief   Deactivates the ADC peripheral.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_stop(ADCDriver *adcp)
{
  if (adcp->state == ADC_READY) {
    /* Resets the peripheral.*/

    udmaChannelRelease(adcp->dmanr);

    /* Disables the peripheral.*/
#if TIVA_ADC_USE_ADC0
    if (&ADCD1 == adcp) {
      nvicDisableVector(TIVA_ADC0_SEQ0_NUMBER);
    }
#endif

#if TIVA_ADC_USE_ADC1
    if (&ADCD2 == adcp) {
      nvicDisableVector(TIVA_ADC1_SEQ0_NUMBER);
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
void adc_lld_start_conversion(ADCDriver *adcp)
{
  uint32_t adc = adcp->adc;
  tiva_udma_table_entry_t *primary = &udmaControlTable.primary[adcp->dmanr];
  tiva_udma_table_entry_t *alternate = &udmaControlTable.alternate[adcp->dmanr];

  /* Disable sample sequencer 0 */
  HWREG(adc + ADC_O_ACTSS) &= (1 << 0);

  /* Configure the sample sequencer 0 trigger */
  HWREG(adc + ADC_O_EMUX) = adcp->grpp->emux & 0xff;

  /* If pwm is used as trigger, select in which block the pwm generator is
     located */
  if (adcp->grpp->emux >= 6 && adcp->grpp->emux <= 9) {
    HWREG(adc + ADC_O_TSSEL) = 0;
  }

  /* For each sample in the sample sequencer, select the input source */
  HWREG(adc + ADC_O_SSMUX0) = adcp->grpp->ssmux;

  /* Configure the sample control bits */
  HWREG(adc + ADC_O_SSCTL0) = adcp->grpp->ssctl | 0x44444444; /* Enforce IEn bits */

  /* Alternate source endpoint is the same for all transfers */
  alternate->srcendp = (void *)(adcp->adc + ADC_O_SSFIFO0);

  /* Configure DMA */
  if ((adcp->grpp->circular) && (adcp->depth > 1)) {
    /* Configure DMA in ping-pong mode.
       Ping (1st half) is configured in the primary control structure.
       Pong (2nd half) is configured in the alternate control structure. */

    uint32_t ctl;

    /* configure the primary source endpoint */
    primary->srcendp = (void *)(adcp->adc + ADC_O_SSFIFO0);

    /* sample buffer is split in half, the upper half is used here */
    primary->dstendp = (void *)(adcp->samples +
                               (adcp->grpp->num_channels * adcp->depth / 2) - 1);
    /* the lower half is used here */
    alternate->dstendp = (void *)(adcp->samples +
                                 (adcp->grpp->num_channels * adcp->depth) - 1);

    ctl = UDMA_CHCTL_DSTSIZE_32 | UDMA_CHCTL_DSTINC_32 |
          UDMA_CHCTL_SRCSIZE_32 | UDMA_CHCTL_SRCINC_NONE |
          UDMA_CHCTL_ARBSIZE_1 |
          UDMA_CHCTL_XFERSIZE(adcp->grpp->num_channels * adcp->depth / 2) |
          UDMA_CHCTL_XFERMODE_PINGPONG;

    adcp->prictl = ctl;
    adcp->altctl = ctl;

    dmaChannelPrimary(adcp->dmanr);
  }
  else {
    /* Configure the DMA in basic mode.
       This is used for both circular buffers with a depth of 1 and linear
       buffers.*/
    alternate->dstendp = (void *)(adcp->samples +
                                 (adcp->grpp->num_channels * adcp->depth) - 1);
    adcp->prictl = UDMA_CHCTL_XFERMODE_STOP;
    adcp->altctl = UDMA_CHCTL_DSTSIZE_32 | UDMA_CHCTL_DSTINC_32 |
                   UDMA_CHCTL_SRCSIZE_32 | UDMA_CHCTL_SRCINC_NONE |
                   UDMA_CHCTL_ARBSIZE_1 |
                   UDMA_CHCTL_XFERSIZE(adcp->grpp->num_channels * adcp->depth) |
                   UDMA_CHCTL_XFERMODE_BASIC;

    dmaChannelAlternate(adcp->dmanr);
  }

  /* Configure primary and alternate channel control fields */
  primary->chctl = adcp->prictl;
  alternate->chctl = adcp->altctl;

  /* Configure DMA channel */
  dmaChannelBurstOnly(adcp->dmanr);
  dmaChannelPriorityDefault(adcp->dmanr);
  dmaChannelEnableRequest(adcp->dmanr);

  /* Enable DMA channel */
  dmaChannelEnable(adcp->dmanr);

  /* Enable the sample sequencer */
  HWREG(adc + ADC_O_ACTSS) |= (1 << 0);

  /* Enable DMA on the sample sequencer, is this for 129x only?*/
  //HWREG(adc + ADC_O_ACTSS) |= (1 << 8);

  /* Start conversion if configured for CPU trigger */
  if ((adcp->grpp->emux & 0xff) == 0) {
    HWREG(adc + ADC_O_PSSI) = ADC_PSSI_SS0;
  }
}

/**
 * @brief   Stops an ongoing conversion.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_stop_conversion(ADCDriver *adcp)
{
  uint32_t adc = adcp->adc;

  /* Stop ongoing DMA transfer */
  dmaChannelDisable(adcp->dmanr);

  /* Stop ongoing ADC conversion by disabling the active sample sequencer */
  HWREG(adc + ADC_O_ACTSS) &= ~(1 << 0);
}

#endif /* HAL_USE_ADC == TRUE */

/** @} */
