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
 * @file    STM32L1xx/ext_lld_isr.c
 * @brief   STM32L1xx EXT subsystem low level driver ISR code.
 *
 * @addtogroup EXT
 * @{
 */

#include "hal.h"

#if HAL_USE_EXT || defined(__DOXYGEN__)

#include "ext_lld_isr.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/**
 * @brief   EXTI[0] interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector58) {
  uint32_t pr;

  OSAL_IRQ_PROLOGUE();

  pr = EXTI->PR;
  pr &= EXTI->IMR & (1U << 0);
  EXTI->PR = pr;
  if (pr & (1U << 0))
    EXTD1.config->channels[0].cb(&EXTD1, 0);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   EXTI[1] interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector5C) {
  uint32_t pr;

  OSAL_IRQ_PROLOGUE();

  pr = EXTI->PR;
  pr &= EXTI->IMR & (1U << 1);
  EXTI->PR = pr;
  if (pr & (1U << 1))
    EXTD1.config->channels[1].cb(&EXTD1, 1);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   EXTI[2] interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector60) {
  uint32_t pr;

  OSAL_IRQ_PROLOGUE();

  pr = EXTI->PR;
  pr &= EXTI->IMR & (1U << 2);
  EXTI->PR = pr;
  if (pr & (1U << 2))
    EXTD1.config->channels[2].cb(&EXTD1, 2);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   EXTI[3] interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector64) {
  uint32_t pr;

  OSAL_IRQ_PROLOGUE();

  pr = EXTI->PR;
  pr &= EXTI->IMR & (1U << 3);
  EXTI->PR = pr;
  if (pr & (1U << 3))
    EXTD1.config->channels[3].cb(&EXTD1, 3);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   EXTI[4] interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector68) {
  uint32_t pr;

  OSAL_IRQ_PROLOGUE();

  pr = EXTI->PR;
  pr &= EXTI->IMR & (1U << 4);
  EXTI->PR = pr;
  if (pr & (1U << 4))
    EXTD1.config->channels[4].cb(&EXTD1, 4);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   EXTI[5]...EXTI[9] interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector9C) {
  uint32_t pr;

  OSAL_IRQ_PROLOGUE();

  pr = EXTI->PR;
  pr &= EXTI->IMR & ((1U << 5) | (1U << 6) | (1U << 7) | (1U << 8) |
                     (1U << 9));
  EXTI->PR = pr;
  if (pr & (1U << 5))
    EXTD1.config->channels[5].cb(&EXTD1, 5);
  if (pr & (1U << 6))
    EXTD1.config->channels[6].cb(&EXTD1, 6);
  if (pr & (1U << 7))
    EXTD1.config->channels[7].cb(&EXTD1, 7);
  if (pr & (1U << 8))
    EXTD1.config->channels[8].cb(&EXTD1, 8);
  if (pr & (1U << 9))
    EXTD1.config->channels[9].cb(&EXTD1, 9);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   EXTI[10]...EXTI[15] interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(VectorE0) {
  uint32_t pr;

  OSAL_IRQ_PROLOGUE();

  pr = EXTI->PR;
  pr &= EXTI->IMR & ((1U << 10) | (1U << 11) | (1U << 12) | (1U << 13) |
                     (1U << 14) | (1U << 15));
  EXTI->PR = pr;
  if (pr & (1U << 10))
    EXTD1.config->channels[10].cb(&EXTD1, 10);
  if (pr & (1U << 11))
    EXTD1.config->channels[11].cb(&EXTD1, 11);
  if (pr & (1U << 12))
    EXTD1.config->channels[12].cb(&EXTD1, 12);
  if (pr & (1U << 13))
    EXTD1.config->channels[13].cb(&EXTD1, 13);
  if (pr & (1U << 14))
    EXTD1.config->channels[14].cb(&EXTD1, 14);
  if (pr & (1U << 15))
    EXTD1.config->channels[15].cb(&EXTD1, 15);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   EXTI[16] interrupt handler (PVD).
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector44) {
  uint32_t pr;

  OSAL_IRQ_PROLOGUE();

  pr = EXTI->PR;
  pr &= EXTI->IMR & (1U << 16);
  EXTI->PR = pr;
  if (pr & (1U << 16))
    EXTD1.config->channels[16].cb(&EXTD1, 16);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   EXTI[17] interrupt handler (RTC).
 *
 * @isr
 */
OSAL_IRQ_HANDLER(VectorE4) {
  uint32_t pr;

  OSAL_IRQ_PROLOGUE();

  pr = EXTI->PR;
  pr &= EXTI->IMR & (1U << 17);
  EXTI->PR = pr;
  if (pr & (1U << 17))
    EXTD1.config->channels[17].cb(&EXTD1, 17);

  OSAL_IRQ_EPILOGUE();
}
/**
 * @brief   EXTI[18] interrupt handler (USB_FS_WKUP).
 *
 * @isr
 */
OSAL_IRQ_HANDLER(VectorE8) {
  uint32_t pr;

  OSAL_IRQ_PROLOGUE();

  pr = EXTI->PR;
  pr &= EXTI->IMR & (1U << 18);
  EXTI->PR = pr;
  if (pr & (1U << 18))
    EXTD1.config->channels[18].cb(&EXTD1, 18);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   EXTI[19] interrupt handler (TAMPER_STAMP).
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector48) {
  uint32_t pr;

  OSAL_IRQ_PROLOGUE();

  pr = EXTI->PR;
  pr &= EXTI->IMR & (1U << 19);
  EXTI->PR = pr;
  if (pr & (1U << 19))
    EXTD1.config->channels[19].cb(&EXTD1, 19);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   EXTI[20] interrupt handler (RTC_WKUP).
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector4C) {
  uint32_t pr;

  OSAL_IRQ_PROLOGUE();

  pr = EXTI->PR;
  pr &= EXTI->IMR & (1U << 20);
  EXTI->PR = pr;
  if (pr & (1U << 20))
    EXTD1.config->channels[20].cb(&EXTD1, 20);

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   EXTI[21]...EXTI[22] interrupt handler (COMP).
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector98) {
  uint32_t pr;

  OSAL_IRQ_PROLOGUE();

  pr = EXTI->PR;
  pr &= EXTI->IMR & ((1U << 21) | (1U << 22));
  EXTI->PR = pr;
  if (pr & (1U << 21))
    EXTD1.config->channels[21].cb(&EXTD1, 21);
  if (pr & (1U << 22))
    EXTD1.config->channels[22].cb(&EXTD1, 22);

  OSAL_IRQ_EPILOGUE();
}

#if (STM32_EXTI_NUM_LINES > 23) || defined(__DOXYGEN__)
/**
 * @brief   EXTI[23] interrupt handler (Channel Acquisition).
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector120) {
  uint32_t pr;

  OSAL_IRQ_PROLOGUE();

  pr = EXTI->PR;
  pr &= EXTI->IMR & (1U << 23);
  EXTI->PR = pr;
  if (pr & (1U << 23))
    EXTD1.config->channels[23].cb(&EXTD1, 23);

  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Enables EXTI IRQ sources.
 *
 * @notapi
 */
void ext_lld_exti_irq_enable(void) {

  nvicEnableVector(EXTI0_IRQn, STM32_EXT_EXTI0_IRQ_PRIORITY);
  nvicEnableVector(EXTI1_IRQn, STM32_EXT_EXTI1_IRQ_PRIORITY);
  nvicEnableVector(EXTI2_IRQn, STM32_EXT_EXTI2_IRQ_PRIORITY);
  nvicEnableVector(EXTI3_IRQn, STM32_EXT_EXTI3_IRQ_PRIORITY);
  nvicEnableVector(EXTI4_IRQn, STM32_EXT_EXTI4_IRQ_PRIORITY);
  nvicEnableVector(EXTI9_5_IRQn, STM32_EXT_EXTI5_9_IRQ_PRIORITY);
  nvicEnableVector(EXTI15_10_IRQn, STM32_EXT_EXTI10_15_IRQ_PRIORITY);
  nvicEnableVector(PVD_IRQn, STM32_EXT_EXTI16_IRQ_PRIORITY);
  nvicEnableVector(RTC_Alarm_IRQn, STM32_EXT_EXTI17_IRQ_PRIORITY);
  nvicEnableVector(USB_FS_WKUP_IRQn, STM32_EXT_EXTI18_IRQ_PRIORITY);
  nvicEnableVector(TAMPER_STAMP_IRQn, STM32_EXT_EXTI19_IRQ_PRIORITY);
  nvicEnableVector(RTC_WKUP_IRQn, STM32_EXT_EXTI20_IRQ_PRIORITY);
  nvicEnableVector(COMP_IRQn, STM32_EXT_EXTI21_22_IRQ_PRIORITY);
#if STM32_EXTI_NUM_LINES > 23
  nvicEnableVector(COMP_ACQ_IRQn, STM32_EXT_EXTI23_IRQ_PRIORITY);
#endif
}

/**
 * @brief   Disables EXTI IRQ sources.
 *
 * @notapi
 */
void ext_lld_exti_irq_disable(void) {

  nvicDisableVector(EXTI0_IRQn);
  nvicDisableVector(EXTI1_IRQn);
  nvicDisableVector(EXTI2_IRQn);
  nvicDisableVector(EXTI3_IRQn);
  nvicDisableVector(EXTI4_IRQn);
  nvicDisableVector(EXTI9_5_IRQn);
  nvicDisableVector(EXTI15_10_IRQn);
  nvicDisableVector(PVD_IRQn);
  nvicDisableVector(RTC_Alarm_IRQn);
  nvicDisableVector(USB_FS_WKUP_IRQn);
  nvicDisableVector(TAMPER_STAMP_IRQn);
  nvicDisableVector(RTC_WKUP_IRQn);
  nvicDisableVector(COMP_IRQn);
#if STM32_EXTI_NUM_LINES > 23
  nvicDisableVector(COMP_ACQ_IRQn);
#endif
}

#endif /* HAL_USE_EXT */

/** @} */
