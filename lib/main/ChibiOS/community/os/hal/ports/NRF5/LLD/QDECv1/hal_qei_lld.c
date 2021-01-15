/*
    ChibiOS - Copyright (C) 2016..2016 Stéphane D'Alu

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
 * @file    QDECv1/hal_qei_lld.c
 * @brief   NRF5 QEI subsystem low level driver.
 *
 * @addtogroup QEI
 * @{
 */

#include "hal.h"

#if (HAL_USE_QEI == TRUE) || defined(__DOXYGEN__)


/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   QEID1 driver identifier.
 */
#if NRF5_QEI_USE_QDEC0 || defined(__DOXYGEN__)
QEIDriver QEID1;
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
 * @param[in] qeip         pointer to an QEIDriver
 */
static void serve_interrupt(QEIDriver *qeip) {
  NRF_QDEC_Type *qdec = qeip->qdec;

#if NRF5_QEI_USE_ACC_OVERFLOWED_CB == TRUE
  /* Accumulator overflowed
   */
  if (qdec->EVENTS_ACCOF) {
    qdec->EVENTS_ACCOF = 0;
#if CORTEX_MODEL >= 4
    (void)qdec->EVENTS_ACCOF;
#endif
    
    qeip->overflowed++;
    if (qeip->config->overflowed_cb)
      qeip->config->overflowed_cb(qeip);
  }
#endif
  
  /* Report ready
   */
  if (qdec->EVENTS_REPORTRDY) {
    qdec->EVENTS_REPORTRDY = 0;
#if CORTEX_MODEL >= 4
    (void)qdec->EVENTS_REPORTRDY;
#endif
    
    /* Read (and clear counters due to shortcut) */
    int16_t  acc    = ( int16_t)qdec->ACCREAD;
    uint16_t accdbl = (uint16_t)qdec->ACCDBLREAD;

    /* Inverse direction if requested */
    if (qeip->config->dirinv)
      acc = -acc; // acc is [-1024..+1023], its okay on int16_t

    /* Adjust counter */
    qeiAdjustI(qeip, acc);
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if NRF5_QEI_USE_QDEC0 == TRUE
/**
 * @brief   Quadrature decoder vector (QDEC)
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector88) {

  OSAL_IRQ_PROLOGUE();
  serve_interrupt(&QEID1);
  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level QEI driver initialization.
 *
 * @notapi
 */
void qei_lld_init(void) {

#if NRF5_QEI_USE_QDEC0 == TRUE
  /* Driver initialization.*/
  qeiObjectInit(&QEID1);
  QEID1.qdec = NRF_QDEC; 
#endif
}

/**
 * @brief   Configures and activates the QEI peripheral.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 *
 * @notapi
 */
void qei_lld_start(QEIDriver *qeip) {
  NRF_QDEC_Type *qdec = qeip->qdec; 
  const QEIConfig *cfg = qeip->config;

  if (qeip->state == QEI_STOP) {
    /* Set Pins */
    palSetLineMode(cfg->phase_a, PAL_MODE_INPUT);
    palSetLineMode(cfg->phase_b, PAL_MODE_INPUT);
#if NRF5_QEI_USE_LED == TRUE
    if (cfg->led != PAL_NOLINE) {
      palSetLineMode(cfg->led, PAL_MODE_INPUT);
    }
#endif
      
    /* Set interrupt masks and enable interrupt */
#if NRF5_QEI_USE_ACC_OVERFLOWED_CB == TRUE
    qdec->INTENSET = QDEC_INTENSET_REPORTRDY_Msk |
	             QDEC_INTENSET_ACCOF_Msk;
#else
    qdec->INTENSET = QDEC_INTENSET_REPORTRDY_Msk;
#endif
#if NRF5_QEI_USE_QDEC0 == TRUE
    if (&QEID1 == qeip) {
      nvicEnableVector(QDEC_IRQn, NRF5_QEI_QDEC0_IRQ_PRIORITY);
    }
#endif

    /* Select pin for Phase A and Phase B */
#if   NRF_SERIES == 51
    qdec->PSELA      = PAL_PAD(cfg->phase_a);
    qdec->PSELB      = PAL_PAD(cfg->phase_b);
#else
    qdec->PSEL.A     = PAL_PAD(cfg->phase_a);
    qdec->PSEL.B     = PAL_PAD(cfg->phase_b);
#endif
    /* Select (optional) pin for LED, and configure it */
#if NRF5_QEI_USE_LED == TRUE
#if   NRF_SERIES == 51
    qdec->PSELLED    = PAL_PAD(cfg->led);
#else
    qdec->PSEL.LED   = PAL_PAD(cfg->led);
#endif
    qdec->LEDPOL     = ((cfg->led_polarity == QEI_LED_POLARITY_LOW)
                         ? QDEC_LEDPOL_LEDPOL_ActiveLow 
		         : QDEC_LEDPOL_LEDPOL_ActiveHigh)
                       << QDEC_LEDPOL_LEDPOL_Pos; 
    qdec->LEDPRE     = cfg->led_warming;
#else
#if   NRF_SERIES == 51
    qdec->PSELLED    = (uint32_t)-1;
#else
    qdec->PSEL.LED   = (uint32_t)-1;
#endif
#endif
    
    /* Set sampling resolution and debouncing */
    qdec->SAMPLEPER  = cfg->resolution;
    qdec->DBFEN      = (cfg->debouncing ? QDEC_DBFEN_DBFEN_Enabled
			                : QDEC_DBFEN_DBFEN_Disabled)
                       << QDEC_DBFEN_DBFEN_Pos;

    /* Define minimum sampling before reporting
       and create shortcut to clear accumulation */
    qdec->REPORTPER  = cfg->report;
    qdec->SHORTS     = QDEC_SHORTS_REPORTRDY_READCLRACC_Msk;

    /* Enable peripheric */
    qdec->ENABLE     = 1;
  }

  /* Initially state is stopped, events cleared */
  qdec->TASKS_STOP       = 1;
  qdec->EVENTS_SAMPLERDY = 0;
  qdec->EVENTS_REPORTRDY = 0;
  qdec->EVENTS_ACCOF     = 0;
#if CORTEX_MODEL >= 4
  (void)qdec->EVENTS_SAMPLERDY;
  (void)qdec->EVENTS_REPORTRDY;
  (void)qdec->EVENTS_ACCOF;
#endif
}

/**
 * @brief   Deactivates the QEI peripheral.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 *
 * @notapi
 */
void qei_lld_stop(QEIDriver *qeip) {

  NRF_QDEC_Type *qdec = qeip->qdec;
  const QEIConfig *cfg = qeip->config;

  if (qeip->state == QEI_READY) {
    qdec->TASKS_STOP = 1;
    qdec->ENABLE     = 0;

    /* Unset interrupt masks and disable interrupt */
#if NRF5_QEI_USE_QDEC0 == TRUE
    if (&QEID1 == qeip) {
      nvicDisableVector(QDEC_IRQn);
    }
#endif
#if NRF5_QEI_USE_ACC_OVERFLOWED_CB == TRUE
    qdec->INTENCLR = QDEC_INTENCLR_REPORTRDY_Msk |
	             QDEC_INTENCLR_ACCOF_Msk;
#else
    qdec->INTENCLR = QDEC_INTENCLR_REPORTRDY_Msk;
#endif
    
    /* Return pins to reset state */
    palSetLineMode(cfg->phase_a, PAL_MODE_RESET);
    palSetLineMode(cfg->phase_b, PAL_MODE_RESET);
#if NRF5_QEI_USE_LED == TRUE
    if (cfg->led != PAL_NOLINE) {
      palSetLineMode(cfg->led, PAL_MODE_RESET);
    }
#endif
  }
}

/**
 * @brief   Enables the input capture.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 *
 * @notapi
 */
void qei_lld_enable(QEIDriver *qeip) {
#if NRF5_QEI_USE_ACC_OVERFLOWED_CB == TRUE
  qeip->overflowed = 0;
#endif
  
  qeip->qdec->EVENTS_SAMPLERDY = 0;
  qeip->qdec->EVENTS_REPORTRDY = 0;
  qeip->qdec->EVENTS_ACCOF = 0;
#if CORTEX_MODEL >= 4
  (void)qeip->qdec->EVENTS_SAMPLERDY;
  (void)qeip->qdec->EVENTS_REPORTRDY;
  (void)qeip->qdec->EVENTS_ACCOF;
#endif
  qeip->qdec->TASKS_START = 1;
}

/**
 * @brief   Disables the input capture.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 *
 * @notapi
 */
void qei_lld_disable(QEIDriver *qeip) {
  qeip->qdec->TASKS_STOP = 1;
}


#endif /* HAL_USE_QEI */

/** @} */
