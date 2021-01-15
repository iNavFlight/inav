/*
    ChibiOS - Copyright (C) 2016 Andrew Wygle aka awygle

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
 * @brief   MSP430X ADC subsystem low level driver source.
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

/**
 * @brief   ADC1 driver identifier.
 */
#if (MSP430X_ADC_USE_ADC1 == TRUE) || defined(__DOXYGEN__)
ADCDriver ADCD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static void restart_dma(ADCDriver * adcp) {
  /* TODO timeouts? */
  /* Restart DMA transfer */
  if (adcp->dma.registers == NULL) {
    /* Acquire a DMA stream because dmaTransfer can be called from ISRs */
    osalSysLockFromISR();
    dmaAcquireI(&(adcp->dma), adcp->dma.index);
    osalSysUnlockFromISR();
    dmaTransfer(&(adcp->dma), &(adcp->req));
  }
  else {
    dmaTransfer(&(adcp->dma), &(adcp->req));
  }
}

static void dma_callback(void * args) {
  ADCDriver * adcp = (ADCDriver *)args;

  if (adcp->grpp == NULL)
    return;

  adcp->count++;

  if (adcp->count == adcp->depth / 2) {
    /* half-full interrupt */
    _adc_isr_half_code(adcp);
  }

  if (adcp->count == adcp->depth) {
    /* full interrupt */

    /* adc_lld_stop_conversion is called automatically here if needed */
    _adc_isr_full_code(adcp);
    /* after isr_full, adcp->grpp is only non-NULL if it's a circular group */
    if (adcp->grpp) {
      /* Reset the buffer pointer */
      adcp->req.dest_addr = adcp->samples;

      restart_dma(adcp);

      /* Reset the count */
      adcp->count = 0;

      /* Start next sequence */
      adcp->regs->ctl[0] |= ADC12SC;
    }
  }
  else {
    /* Advance the buffer pointer */
    adcp->req.dest_addr = adcp->samples + (adcp->req.size * adcp->count);

    restart_dma(adcp);

    /* Start next sequence */
    adcp->regs->ctl[0] |= ADC12SC;
  }
}

static void populate_tlv(ADCDriver * adcp) {
  uint8_t * tlv_addr = (uint8_t *)TLV_START;

  while (*tlv_addr != TLV_TAGEND && tlv_addr < (uint8_t *)TLV_END) {
    if (*tlv_addr == TLV_ADC12CAL) {
      adcp->adc_cal = (msp430x_adc_cal_t *)(tlv_addr + 2);
    }
    else if (*tlv_addr == TLV_REFCAL) {
      adcp->ref_cal = (msp430x_ref_cal_t *)(tlv_addr + 2);
    }
    tlv_addr += (tlv_addr[1] + 2);
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

PORT_IRQ_HANDLER(ADC12_VECTOR) {

  OSAL_IRQ_PROLOGUE();

  switch (__even_in_range(ADC12IV, ADC12IV_ADC12TOVIFG)) {

  case ADC12IV_ADC12OVIFG: {
    if (ADCD1.grpp == NULL)
      break;
    _adc_isr_error_code(&ADCD1, ADC_ERR_OVERFLOW);
    break;
  }
  case ADC12IV_ADC12TOVIFG: {
    if (ADCD1.grpp == NULL)
      break;
    _adc_isr_error_code(&ADCD1, ADC_ERR_AWD);
    break;
  }
  default:
    osalDbgAssert(false, "unhandled ADC exception");
    _adc_isr_error_code(&ADCD1, ADC_ERR_UNKNOWN);
  }

  OSAL_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level ADC driver initialization.
 *
 * @notapi
 */
void adc_lld_init(void) {

#if MSP430X_ADC_USE_ADC1 == TRUE
  /* Driver initialization.*/
  adcObjectInit(&ADCD1);
  ADCD1.regs = (msp430x_adc_reg_t *)(&ADC12CTL0);
  populate_tlv(&ADCD1);
#endif
}

/**
 * @brief   Configures and activates the ADC peripheral.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_start(ADCDriver * adcp) {

  if (adcp->state == ADC_STOP) {
    /* Enables the peripheral.*/
    adcp->regs->ctl[0] = ADC12ON | ADC12MSC;
    adcp->regs->ctl[1] =
        MSP430X_ADC1_PDIV | MSP430X_ADC1_DIV | MSP430X_ADC1_SSEL | ADC12SHP;
    adcp->regs->ctl[3] = ADC12ICH3MAP | ADC12ICH2MAP | ADC12ICH1MAP |
                         ADC12ICH0MAP | ADC12TCMAP | ADC12BATMAP;
    adcp->regs->ier[2] = ADC12TOVIE | ADC12OVIE;
    adcp->req.trigger  = DMA_TRIGGER_MNEM(ADC12IFG);
#if MSP430X_ADC_COMPACT_SAMPLES == TRUE
    adcp->req.data_mode = MSP430X_DMA_SRCWORD | MSP430X_DMA_DSTBYTE;
#else
    adcp->req.data_mode = MSP430X_DMA_SRCWORD | MSP430X_DMA_DSTWORD;
#endif
    adcp->req.addr_mode         = MSP430X_DMA_SRCINCR | MSP430X_DMA_DSTINCR;
    adcp->req.transfer_mode     = MSP430X_DMA_SINGLE;
    adcp->req.callback.callback = dma_callback;
    adcp->req.callback.args     = adcp;

#if MSP430X_ADC_EXCLUSIVE_DMA == TRUE
    bool b;
    if (adcp->config->dma_index < MSP430X_DMA_CHANNELS) {
      b = dmaAcquireI(&adcp->dma, adcp->config->dma_index);
      osalDbgAssert(!b, "stream already allocated");
    }
    else {
#endif
      adcp->dma.registers = NULL;
#if MSP430X_ADC_EXCLUSIVE_DMA == TRUE
    }
#endif
  }
  /* Configures the peripheral.*/
}

/**
 * @brief   Deactivates the ADC peripheral.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_stop(ADCDriver * adcp) {

  if (adcp->state == ADC_READY) {
/* Resets the peripheral.*/

/* Disables the peripheral.*/
#if MSP430X_ADC_EXCLUSIVE_DMA == TRUE
    if (adcp->config->dma_index < MSP430X_DMA_CHANNELS) {
      dmaRelease(&(adcp->dma));
    }
#endif
    adcp->regs->ctl[0] = 0;
  }
}

/**
 * @brief   Starts an ADC conversion.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_start_conversion(ADCDriver * adcp) {

  /* always use sequential transfer mode - this is fine */
  adcp->regs->ctl[1] |= ADC12CONSEQ0;

  /* set resolution */
  adcp->regs->ctl[2] |= adcp->grpp->res;
  /* start from MEM0 */
  adcp->regs->ctl[3] &= ~(ADC12CSTARTADD_31);

  /* Configure voltage reference */
  while (REFCTL0 & REFGENBUSY)
    ;
  REFCTL0 = adcp->grpp->vref_src;

  for (int i = 0; i < adcp->grpp->num_channels; i++) {
    osalDbgAssert(adcp->grpp->channels[i] < 32, "invalid channel number");
    adcp->regs->mctl[i] = adcp->grpp->ref | adcp->grpp->channels[i];
  }

  adcp->regs->mctl[adcp->grpp->num_channels - 1] |= ADC12EOS;

  adcp->req.source_addr = adcp->regs->mem;
  adcp->req.dest_addr   = adcp->samples;
  adcp->req.size        = adcp->grpp->num_channels;
  adcp->count           = 0;

/* TODO timeouts? */
#if MSP430X_ADC_EXCLUSIVE_DMA == TRUE
  if (adcp->config->dma_index >= MSP430X_DMA_CHANNELS) {
    adcp->dma.index = dmaRequestS(&(adcp->req), TIME_INFINITE);
  }
  else {
    dmaTransfer(&(adcp->dma), &(adcp->req));
  }
#else
  adcp->dma.index       = dmaRequestS(&(adcp->req), TIME_INFINITE);
#endif

  adcp->regs->ctl[0] |= adcp->grpp->rate | ADC12MSC | ADC12ENC | ADC12SC;
}

/**
 * @brief   Stops an ongoing conversion.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object
 *
 * @notapi
 */
void adc_lld_stop_conversion(ADCDriver * adcp) {

  /* TODO stop DMA transfers here */
  adcp->regs->ctl[0] &= ~(ADC12ENC | ADC12SC);

#if MSP430X_ADC_EXCLUSIVE_DMA == TRUE
  if (adcp->config->dma_index >= MSP430X_DMA_CHANNELS) {
#endif
    if (adcp->dma.registers != NULL) {
      dmaRelease(&(adcp->dma));
      adcp->dma.registers = NULL;
    }
#if MSP430X_ADC_EXCLUSIVE_DMA == TRUE
  }
#endif
}

adcsample_t adcMSP430XAdjustResult(ADCConversionGroup * grpp,
                                   adcsample_t sample) {
  uint32_t tmp;
  uint16_t fact;
  if (grpp->ref == MSP430X_ADC_VSS_VREF_BUF ||
      grpp->ref == MSP430X_ADC_VEREF_P_VREF_BUF ||
      grpp->ref == MSP430X_ADC_VREF_BUF_VCC ||
      grpp->ref == MSP430X_ADC_VREF_BUF_VEREF_P ||
      grpp->ref == MSP430X_ADC_VEREF_N_VREF_BUF) {
    /* Retrieve proper reference correction factor from TLV */
    fact = (&(ADCD1.ref_cal->CAL_ADC_12VREF_FACTOR))[grpp->vref_src >> 4];
    /* Calculate corrected value */
    tmp    = (uint32_t)(sample << 1) * (uint32_t)fact;
    sample = tmp >> 16;
  }

  /* Gain correction */
  fact   = ADCD1.adc_cal->CAL_ADC_GAIN_FACTOR;
  tmp    = (uint32_t)(sample << 1) * (uint32_t)fact;
  sample = tmp >> 16;

  /* Offset correction */
  sample += ADCD1.adc_cal->CAL_ADC_OFFSET;

  return sample;
}

adcsample_t adcMSP430XAdjustTemp(ADCConversionGroup * grpp,
                                 adcsample_t sample) {
  uint16_t t30;
  uint16_t t85;

  /* Retrieve proper T = 30 correction value from TLV */
  t30 = (&(ADCD1.adc_cal->CAL_ADC_12T30))[grpp->vref_src >> 3];
  /* Retrieve proper T = 85 correction value from TLV */
  t85 = (&(ADCD1.adc_cal->CAL_ADC_12T30))[(grpp->vref_src >> 3) + 1];

  return ((((int32_t)sample - (int32_t)t30) * (85 - 30)) / (t85 - t30)) + 30;
}

#endif /* HAL_USE_ADC == TRUE */

/** @} */
