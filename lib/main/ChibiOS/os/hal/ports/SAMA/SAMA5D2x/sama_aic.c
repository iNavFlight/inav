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
 * @file    SAMA5D2x/aic.c
 * @brief   SAMA AIC support code.
 *
 * @addtogroup SAMA5D2x_AIC
 * @{
 */

#include "hal.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local macros.                                                      */
/*===========================================================================*/

/**
 * @brief   Enable write protection on AIC registers block.
 *
 * @param[in] aicp    pointer to a AIC register block
 *
 * @notapi
 */
#define aicEnableWP(aicp) {                                                  \
  aicp->AIC_WPMR = AIC_WPMR_WPKEY_PASSWD | AIC_WPMR_WPEN;                    \
}

/**
 * @brief   Disable write protection on AIC registers block.
 *
 * @param[in] aicp    pointer to a AIC register block
 *
 * @notapi
 */
#define aicDisableWP(aicp) {                                                 \
  aicp->AIC_WPMR = AIC_WPMR_WPKEY_PASSWD;                                    \
}

/**
 * @brief   Checks if a IRQ priority is within the valid range.
 * @param[in] prio      IRQ priority
 *
 * @retval              The check result.
 * @retval FALSE        invalid IRQ priority.
 * @retval TRUE         correct IRQ priority.
 */
#define SAMA_IRQ_IS_VALID_PRIORITY(prio) ((prio) <= 7U)

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

static OSAL_IRQ_HANDLER(aicSpuriousHandler) {
  OSAL_IRQ_PROLOGUE();
  osalSysHalt("Spurious interrupt");
  OSAL_IRQ_EPILOGUE();
}

static OSAL_IRQ_HANDLER(aicUnexpectedHandler) {
  OSAL_IRQ_PROLOGUE();
  osalSysHalt("Unexpected interrupt");
  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   AIC Initialization.
 * @note    Better reset everything in the AIC.
 *
 * @notapi
 */
void aicInit(void) {

#if SAMA_HAL_IS_SECURE
  Aic *aic = SAIC;
#else
  Aic *aic = AIC;
#endif

  aicDisableWP(aic);

  aic->AIC_SPU = (uint32_t)aicSpuriousHandler;
  aic->AIC_SSR = 0;
  aic->AIC_SVR = (uint32_t)aicUnexpectedHandler;

  unsigned i;
  /* Disable all interrupts */
  for (i = 1; i < ID_PERIPH_COUNT; i++) {
    aic->AIC_SSR = i;
    aic->AIC_IDCR = AIC_IDCR_INTD;

    /* Changes type */
    aic->AIC_SMR = AIC_SMR_SRCTYPE(EXT_NEGATIVE_EDGE);

    /* Clear pending interrupt */
    aic->AIC_ICCR = AIC_ICCR_INTCLR;

    /* Changes type */
    aic->AIC_SMR = AIC_SMR_SRCTYPE(INT_LEVEL_SENSITIVE);

    /* Default handler */
    aic->AIC_SVR = (uint32_t)aicUnexpectedHandler;
  }
  aicEnableWP(aic);
}

/**
 * @brief   Configures an interrupt in the AIC.
 * @note    Source cannot be ID_SAIC_FIQ (0).
 *
 * @param[in] source    interrupt source to configure
 * @param[in] priority  priority level of the selected source.
 */
void aicSetSourcePriority(uint32_t source, uint8_t priority) {

#if SAMA_HAL_IS_SECURE
  Aic *aic = SAIC;
#else
  Aic *aic = AIC;
#endif

  osalDbgCheck(source != ID_SAIC_FIQ);
  osalDbgAssert(SAMA_IRQ_IS_VALID_PRIORITY(priority), "invalid irq priority");
  /* Disable write protection */
  aicDisableWP(aic);
  /* Set source id */
  aic->AIC_SSR = source;
  /* Disable the interrupt first */
  aic->AIC_IDCR = AIC_IDCR_INTD;
  /* Configure priority */
  aic->AIC_SMR |= AIC_SMR_PRIOR(priority);
  /* Clear interrupt */
  aic->AIC_ICCR = AIC_ICCR_INTCLR;
  /* Enable write protection */
  aicEnableWP(aic);
}

/**
 * @brief   Configures type of interrupt in the AIC.
 *
 * @param[in] source    interrupt source to configure
 * @param[in] type      type interrupt of the selected source.
 */
void aicSetIntSourceType(uint32_t source, uint8_t type) {

#if SAMA_HAL_IS_SECURE
  Aic *aic = SAIC;
#else
  Aic *aic = AIC;
#endif
  /* Disable write protection */
  aicDisableWP(aic);
  /* Set source id */
  aic->AIC_SSR = source;
  /* Disable the interrupt first */
  aic->AIC_IDCR = AIC_IDCR_INTD;
  /* Configure priority */
  aic->AIC_SMR |= AIC_SMR_SRCTYPE(type);
  /* Clear interrupt */
  aic->AIC_ICCR = AIC_ICCR_INTCLR;
  /* Enable write protection */
  aicEnableWP(aic);
}

/**
 * @brief   Sets the source handler of an interrupt.
 *
 * @param[in] source    interrupt source to configure
 * @param[in] handler   handler for the interrupt source selected
 */
void aicSetSourceHandler(uint32_t source, bool (*handler)(void)) {

#if SAMA_HAL_IS_SECURE
  Aic *aic = SAIC;
#else
  Aic *aic = AIC;
#endif

  /* Disable write protection */
  aicDisableWP(aic);
  /* Select source and assign handler */
  aic->AIC_SSR = AIC_SSR_INTSEL(source);
  aic->AIC_SVR = (uint32_t)handler;
  /* Enable write protection */
  aicEnableWP(aic);
}

/**
 * @brief   Sets the spurious handler of an interrupt.
 *
 * @param[in] handler   handler for the interrupt
 */
void aicSetSpuriousHandler(bool (*handler)(void)) {

#if SAMA_HAL_IS_SECURE
  Aic *aic = SAIC;
#else
  Aic *aic = AIC;
#endif

  /* Disable write protection */
  aicDisableWP(aic);
  /* Assign handler */
  aic->AIC_SPU = (uint32_t)handler;
  /* Enable write protection */
  aicEnableWP(aic);
}

/**
 * @brief   Enables interrupts coming from the source.
 *
 * @param[in] source    interrupt source to enable
 */
void aicEnableInt(uint32_t source) {

#if SAMA_HAL_IS_SECURE
  Aic *aic = SAIC;
#else
  Aic *aic = AIC;
#endif

  aic->AIC_SSR = AIC_SSR_INTSEL(source);
  aic->AIC_IECR = AIC_IECR_INTEN;
}

/**
 * @brief   Disables interrupts coming from the selected source.
 *
 * @param[in] source    interrupt source to disable
 */
void aicDisableInt(uint32_t source) {

#if SAMA_HAL_IS_SECURE
  Aic *aic = SAIC;
#else
  Aic *aic = AIC;
#endif

  aic->AIC_SSR = AIC_SSR_INTSEL(source);
  aic->AIC_IDCR = AIC_IDCR_INTD;
}

/**
 * @brief   Clears interrupts coming from the selected source.
 *
 * @param[in] source    interrupt source to Clear
 */
void aicClearInt(uint32_t source) {

#if SAMA_HAL_IS_SECURE
  Aic *aic = SAIC;
#else
  Aic *aic = AIC;
#endif

  aic->AIC_SSR = AIC_SSR_INTSEL(source);
  aic->AIC_ICCR = AIC_ICCR_INTCLR;
}

/**
 * @brief   Sets interrupts coming from the selected source.
 *
 * @param[in] source    interrupt source to Set
 */
void aicSetInt(uint32_t source) {

#if SAMA_HAL_IS_SECURE
  Aic *aic = SAIC;
#else
  Aic *aic = AIC;
#endif

  aic->AIC_SSR = AIC_SSR_INTSEL(source);
  aic->AIC_ISCR = AIC_ISCR_INTSET;
}

/** @} */
