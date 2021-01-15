/*
    ChibiOS - Copyright (C) 2006..2017 Giovanni Di Sirio
              Copyright (C) 2017 Fabien Poussin (fabien.poussin (at) google's mail)

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
 * @file    STM32/hal_comp_lld.c
 * @brief   STM32 Comp subsystem low level driver header.
 *
 * @addtogroup COMP
 * @{
 */

#include "hal.h"

#if HAL_USE_COMP || defined(__DOXYGEN__)

#include "hal_comp.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   COMPD1 driver identifier.
 * @note    The driver COMPD1 allocates the comparator COMP1 when enabled.
 */
#if STM32_COMP_USE_COMP1 || defined(__DOXYGEN__)
COMPDriver COMPD1;
#endif

/**
 * @brief   COMPD2 driver identifier.
 * @note    The driver COMPD2 allocates the comparator COMP2 when enabled.
 */
#if STM32_COMP_USE_COMP2 || defined(__DOXYGEN__)
COMPDriver COMPD2;
#endif

/**
 * @brief   COMPD3 driver identifier.
 * @note    The driver COMPD3 allocates the comparator COMP3 when enabled.
 */
#if STM32_COMP_USE_COMP3 || defined(__DOXYGEN__)
COMPDriver COMPD3;
#endif

/**
 * @brief   COMPD4 driver identifier.
 * @note    The driver COMPD4 allocates the comparator COMP4 when enabled.
 */
#if STM32_COMP_USE_COMP4 || defined(__DOXYGEN__)
COMPDriver COMPD4;
#endif

/**
 * @brief   COMPD5 driver identifier.
 * @note    The driver COMPD5 allocates the comparator COMP5 when enabled.
 */
#if STM32_COMP_USE_COMP5 || defined(__DOXYGEN__)
COMPDriver COMPD5;
#endif

/**
 * @brief   COMPD6 driver identifier.
 * @note    The driver COMPD6 allocates the comparator COMP6 when enabled.
 */
#if STM32_COMP_USE_COMP6 || defined(__DOXYGEN__)
COMPDriver COMPD6;
#endif

/**
 * @brief   COMPD7 driver identifier.
 * @note    The driver COMPD7 allocates the comparator COMP7 when enabled.
 */
#if STM32_COMP_USE_COMP7 || defined(__DOXYGEN__)
COMPDriver COMPD7;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/


/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/


/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level COMP driver initialization.
 *
 * @notapi
 */
void comp_lld_init(void) {

#if STM32_COMP_USE_COMP1
  /* Driver initialization.*/
  compObjectInit(&COMPD1);
  COMPD1.reg = COMP;
  COMPD1.reg->CSR = 0;
#if STM32_COMP_USE_INTERRUPTS
  nvicEnableVector(COMP1_2_3_IRQn, STM32_COMP_1_2_3_IRQ_PRIORITY);
#endif
#endif

#if STM32_COMP_USE_COMP2
  /* Driver initialization.*/
  compObjectInit(&COMPD2);
  COMPD2.reg = COMP2;
  COMPD2.reg->CSR = 0;
#if STM32_COMP_USE_INTERRUPTS
  nvicEnableVector(COMP1_2_3_IRQn, STM32_COMP_1_2_3_IRQ_PRIORITY);
#endif
#endif

#if STM32_COMP_USE_COMP3
  /* Driver initialization.*/
  compObjectInit(&COMPD3);
  COMPD3.reg = COMP3;
  COMPD3.reg->CSR = 0;
#if STM32_COMP_USE_INTERRUPTS
  nvicEnableVector(COMP1_2_3_IRQn, STM32_COMP_1_2_3_IRQ_PRIORITY);
#endif
#endif

#if STM32_COMP_USE_COMP4
  /* Driver initialization.*/
  compObjectInit(&COMPD4);
  COMPD4.reg = COMP4;
  COMPD4.reg->CSR = 0;
#if STM32_COMP_USE_INTERRUPTS
  nvicEnableVector(COMP4_5_6_IRQn, STM32_COMP_1_2_3_IRQ_PRIORITY);
#endif
#endif

#if STM32_COMP_USE_COMP5
  /* Driver initialization.*/
  compObjectInit(&COMPD5);
  COMPD5.reg = COMP5;
  COMPD5.reg->CSR = 0;
#if STM32_COMP_USE_INTERRUPTS
  nvicEnableVector(COMP4_5_6_IRQn, STM32_COMP_1_2_3_IRQ_PRIORITY);
#endif
#endif

#if STM32_COMP_USE_COMP6
  /* Driver initialization.*/
  compObjectInit(&COMPD6);
  COMPD6.reg = COMP6;
  COMPD6.reg->CSR = 0;
#if STM32_COMP_USE_INTERRUPTS
  nvicEnableVector(COMP4_5_6_IRQn, STM32_COMP_1_2_3_IRQ_PRIORITY);
#endif
#endif

#if STM32_COMP_USE_COMP7
  /* Driver initialization.*/
  compObjectInit(&COMPD7);
  COMPD7.reg = COMP7;
  COMPD7.reg->CSR = 0;
#if STM32_COMP_USE_INTERRUPTS
  nvicEnableVector(COMP7_IRQn, STM32_COMP_7_IRQ_PRIORITY);
#endif
#endif

}

/**
 * @brief  COMP1, COMP2, COMP3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector140) {
  uint32_t pr;

  OSAL_IRQ_PROLOGUE();

  pr = EXTI->PR;
  pr &= EXTI->IMR & ((1U << 21) | (1U << 22) | (1U << 29));
  EXTI->PR = pr;
#if STM32_COMP_USE_COMP1
  if (pr & (1U << 21) && COMPD1.config->cb != NULL)
    COMPD1.config->cb(&COMPD1);
#endif
#if STM32_COMP_USE_COMP2
  if (pr & (1U << 22) && COMPD2.config->cb != NULL)
    COMPD2.config->cb(&COMPD2);
#endif
#if STM32_COMP_USE_COMP3
  if (pr & (1U << 29) && COMPD3.config->cb != NULL)
    COMPD3.config->cb(&COMPD3);
#endif

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   COMP4, COMP5, COMP6 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector144) {
  uint32_t pr;

  OSAL_IRQ_PROLOGUE();

  pr = EXTI->PR;
  pr &= EXTI->IMR & ((1U << 30) | (1U << 31));
  EXTI->PR = pr;
#if STM32_COMP_USE_COMP4
  if (pr & (1U << 30) && COMPD4.config->cb != NULL)
    COMPD4.config->cb(&COMPD4);
#endif
#if STM32_COMP_USE_COMP5
  if (pr & (1U << 31) && COMPD5.config->cb != NULL)
    COMPD5.config->cb(&COMPD5);
#endif

#if STM32_COMP_USE_COMP6
  pr = EXTI->PR2 & EXTI->IMR2 & (1U << 0);
  EXTI->PR2 = pr;
  if (pr & (1U << 0) && COMPD6.config->cb != NULL)
    COMPD6.config->cb(&COMPD6);
#endif

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   COMP7 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector148) {
  uint32_t pr2;

  OSAL_IRQ_PROLOGUE();

  pr2 = EXTI->PR2;
  pr2 = EXTI->IMR & (1U << 1);
  EXTI->PR2 = pr2;
#if STM32_COMP_USE_COMP7
  if (pr2 & (1U << 1) && COMPD7.config->cb != NULL)
    COMPD7.config->cb(&COMPD7);
#endif

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   Configures and activates an EXT channel (used by comp)
 *
 * @param[in] compp      pointer to the @p COMPDriver object
 * @param[in] channel    EXT channel
 *
 * @notapi
 */
void comp_ext_lld_channel_enable(COMPDriver *compp, uint32_t channel) {
  uint32_t cmask = (1 << (channel & 0x1F));

  /* Don't touch other channels */
  if (channel < 21 || channel > 33) {
      return;
  }

#if STM32_EXTI_NUM_LINES > 32
  if (channel < 32) {
#endif
    /* Masked out lines must not be touched by this driver.*/
    if ((cmask & STM32_EXTI_IMR_MASK) != 0U) {
      return;
    }

    /* Programming edge registers.*/
    if (compp->config->irq_mode == COMP_IRQ_RISING || compp->config->irq_mode == COMP_IRQ_BOTH)
      EXTI->RTSR |= cmask;
    else
      EXTI->RTSR &= ~cmask;
    if (compp->config->irq_mode == COMP_IRQ_FALLING || compp->config->irq_mode == COMP_IRQ_BOTH)
      EXTI->FTSR |= cmask;
    else
      EXTI->FTSR &= ~cmask;

    /* Programming interrupt and event registers.*/
    EXTI->IMR |= cmask;
    EXTI->EMR &= ~cmask;

#if STM32_EXTI_NUM_LINES > 32
  }
  else {
    /* Masked out lines must not be touched by this driver.*/
    if ((cmask & STM32_EXTI_IMR2_MASK) != 0U) {
      return;
    }

    /* Programming edge registers.*/
    if (compp->config->irq_mode == COMP_IRQ_RISING || compp->config->irq_mode == COMP_IRQ_BOTH)
      EXTI->RTSR2 |= cmask;
    else
      EXTI->RTSR2 &= ~cmask;
    if (compp->config->irq_mode == COMP_IRQ_FALLING || compp->config->irq_mode == COMP_IRQ_BOTH)
      EXTI->FTSR2 |= cmask;
    else
      EXTI->FTSR2 &= ~cmask;

    /* Programming interrupt and event registers.*/
    EXTI->IMR2 |= cmask;
    EXTI->EMR2 &= ~cmask;
  }
#endif
}

/**
 * @brief   Deactivate an EXT channel (used by comp)
 *
 * @param[in] compp      pointer to the @p COMPDriver object
 * @param[in] channel    EXT channel
 *
 * @notapi
 */
void comp_ext_lld_channel_disable(COMPDriver *compp, uint32_t channel) {

  (void) compp;
  uint32_t cmask = (1 << (channel & 0x1F));

#if STM32_EXTI_NUM_LINES > 32
  if (channel < 32) {
#endif
    EXTI->IMR  &= ~cmask;
    EXTI->EMR  &= ~cmask;
    EXTI->RTSR &= ~cmask;
    EXTI->FTSR &= ~cmask;
    EXTI->PR    =  cmask;
#if STM32_EXTI_NUM_LINES > 32
  }
  else {
    EXTI->IMR2  &= ~cmask;
    EXTI->EMR2  &= ~cmask;
    EXTI->RTSR2 &= ~cmask;
    EXTI->FTSR2 &= ~cmask;
    EXTI->PR2    =  cmask;
  }
#endif
}

/**
 * @brief   Configures and activates the COMP peripheral.
 *
 * @param[in] compp      pointer to the @p COMPDriver object
 *
 * @notapi
 */
void comp_lld_start(COMPDriver *compp) {

  // Apply CSR Execpt the enable bit.
  compp->reg->CSR = compp->config->csr & ~COMP_CSR_COMPxEN;

  // Inverted output
  if (compp->config->output_mode == COMP_OUTPUT_INVERTED)
    compp->reg->CSR |= COMP_CSR_COMPxPOL;

#if STM32_COMP_USE_INTERRUPTS
#if STM32_COMP_USE_COMP1
  if (compp == &COMPD1) {
    comp_ext_lld_channel_enable(compp, 21);
  }
#endif

#if STM32_COMP_USE_COMP2
  if (compp == &COMPD2) {
    comp_ext_lld_channel_enable(compp, 22);
  }
#endif

#if STM32_COMP_USE_COMP3
  if (compp == &COMPD3) {
    comp_ext_lld_channel_enable(compp, 29);
  }
#endif

#if STM32_COMP_USE_COMP4
  if (compp == &COMPD4) {
    comp_ext_lld_channel_enable(compp, 30);
  }
#endif

#if STM32_COMP_USE_COMP5
  if (compp == &COMPD5) {
    comp_ext_lld_channel_enable(compp, 31);
  }
#endif

#if STM32_COMP_USE_COMP6
  if (compp == &COMPD6) {
    comp_ext_lld_channel_enable(compp, 32);
  }
#endif

#if STM32_COMP_USE_COMP7
  if (compp == &COMPD7) {
    comp_ext_lld_channel_enable(compp, 33);
  }
#endif
#endif

}

/**
 * @brief   Deactivates the comp peripheral.
 *
 * @param[in] compp      pointer to the @p COMPDriver object
 *
 * @notapi
 */
void comp_lld_stop(COMPDriver *compp) {

  if (compp->state == COMP_READY) {

    compp->reg->CSR = 0;
  }

#if STM32_COMP_USE_INTERRUPTS
#if STM32_COMP_USE_COMP1
  if (compp == &COMPD1) {
    comp_ext_lld_channel_disable(compp, 21);
  }
#endif

#if STM32_COMP_USE_COMP2
  if (compp == &COMPD2) {
    comp_ext_lld_channel_disable(compp, 22);
  }
#endif

#if STM32_COMP_USE_COMP3
  if (compp == &COMPD3) {
    comp_ext_lld_channel_disable(compp, 29);
  }
#endif

#if STM32_COMP_USE_COMP4
  if (compp == &COMPD4) {
    comp_ext_lld_channel_disable(compp, 30);
  }
#endif

#if STM32_COMP_USE_COMP5
  if (compp == &COMPD5) {
    comp_ext_lld_channel_disable(compp, 31);
  }
#endif

#if STM32_COMP_USE_COMP6
  if (compp == &COMPD6) {
    comp_ext_lld_channel_disable(compp, 32);
  }
#endif

#if STM32_COMP_USE_COMP7
  if (compp == &COMPD7) {
    comp_ext_lld_channel_disable(compp, 33);
  }
#endif
#endif

}

/**
 * @brief   Enables the output.
 *
 * @param[in] compp      pointer to the @p COMPDriver object
 *
 * @notapi
 */
void comp_lld_enable(COMPDriver *compp) {

   compp->reg->CSR |= COMP_CSR_COMPxEN; /* Enable */
}

/**
 * @brief   Disables the output.
 *
 * @param[in] compp      pointer to the @p COMPDriver object
 *
 * @notapi
 */
void comp_lld_disable(COMPDriver *compp) {

  compp->reg->CSR &= ~COMP_CSR_COMPxEN; /* Disable */
}

#endif /* HAL_USE_COMP */

/** @} */
