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
 * @file    STM32/EXTIv1/ext_lld.c
 * @brief   STM32 EXT subsystem low level driver source.
 *
 * @addtogroup EXT
 * @{
 */

#include "hal.h"

#if HAL_USE_EXT || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/* Handling a difference in ST headers.*/
#if defined(STM32L4XX)
#define EMR     EMR1
#define IMR     IMR1
#define PR      PR1
#define RTSR    RTSR1
#define FTSR    FTSR1
#endif

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   EXTD1 driver identifier.
 */
EXTDriver EXTD1;

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
 * @brief   Low level EXT driver initialization.
 *
 * @notapi
 */
void ext_lld_init(void) {

  /* Driver initialization.*/
  extObjectInit(&EXTD1);
}

/**
 * @brief   Configures and activates the EXT peripheral.
 *
 * @param[in] extp      pointer to the @p EXTDriver object
 *
 * @notapi
 */
void ext_lld_start(EXTDriver *extp) {
  expchannel_t line;

  if (extp->state == EXT_STOP)
    ext_lld_exti_irq_enable();

  /* Configuration of automatic channels.*/
  for (line = 0; line < EXT_MAX_CHANNELS; line++)
    if (extp->config->channels[line].mode & EXT_CH_MODE_AUTOSTART)
      ext_lld_channel_enable(extp, line);
    else
      ext_lld_channel_disable(extp, line);
}

/**
 * @brief   Deactivates the EXT peripheral.
 *
 * @param[in] extp      pointer to the @p EXTDriver object
 *
 * @notapi
 */
void ext_lld_stop(EXTDriver *extp) {

  if (extp->state == EXT_ACTIVE)
    ext_lld_exti_irq_disable();

  EXTI->EMR = 0;
  EXTI->IMR = STM32_EXTI_IMR_MASK;
  EXTI->PR  = ~STM32_EXTI_IMR_MASK;
#if STM32_EXTI_NUM_LINES > 32
  EXTI->IMR2 = STM32_EXTI_IMR2_MASK;
  EXTI->PR2  = ~STM32_EXTI_IMR2_MASK;
#endif
}

/**
 * @brief   Enables an EXT channel.
 *
 * @param[in] extp      pointer to the @p EXTDriver object
 * @param[in] channel   channel to be enabled
 *
 * @notapi
 */
void ext_lld_channel_enable(EXTDriver *extp, expchannel_t channel) {
  uint32_t cmask = (1 << (channel & 0x1F));

  /* Setting the associated GPIO for external channels.*/
  if (channel < 16) {
    uint32_t n = channel >> 2;
    uint32_t mask = ~(0xF << ((channel & 3) * 4));
    uint32_t port = ((extp->config->channels[channel].mode &
                      EXT_MODE_GPIO_MASK) >>
                     EXT_MODE_GPIO_OFF) << ((channel & 3) * 4);

#if defined(STM32F1XX)
    AFIO->EXTICR[n] = (AFIO->EXTICR[n] & mask) | port;
#else /* !defined(STM32F1XX) */
    SYSCFG->EXTICR[n] = (SYSCFG->EXTICR[n] & mask) | port;
#endif /* !defined(STM32F1XX) */
  }

#if STM32_EXTI_NUM_LINES > 32
  if (channel < 32) {
#endif
    /* Masked out lines must not be touched by this driver.*/
    if ((cmask & STM32_EXTI_IMR_MASK) != 0U) {
      return;
    }

    /* Programming edge registers.*/
    if (extp->config->channels[channel].mode & EXT_CH_MODE_RISING_EDGE)
      EXTI->RTSR |= cmask;
    else
      EXTI->RTSR &= ~cmask;
    if (extp->config->channels[channel].mode & EXT_CH_MODE_FALLING_EDGE)
      EXTI->FTSR |= cmask;
    else
      EXTI->FTSR &= ~cmask;

    /* Programming interrupt and event registers.*/
    if (extp->config->channels[channel].cb != NULL) {
      EXTI->IMR |= cmask;
      EXTI->EMR &= ~cmask;
    }
    else {
      EXTI->EMR |= cmask;
      EXTI->IMR &= ~cmask;
    }
#if STM32_EXTI_NUM_LINES > 32
  }
  else {
    /* Masked out lines must not be touched by this driver.*/
    if ((cmask & STM32_EXTI_IMR2_MASK) != 0U) {
      return;
    }

    /* Programming edge registers.*/
    if (extp->config->channels[channel].mode & EXT_CH_MODE_RISING_EDGE)
      EXTI->RTSR2 |= cmask;
    else
      EXTI->RTSR2 &= ~cmask;
    if (extp->config->channels[channel].mode & EXT_CH_MODE_FALLING_EDGE)
      EXTI->FTSR2 |= cmask;
    else
      EXTI->FTSR2 &= ~cmask;

    /* Programming interrupt and event registers.*/
    if (extp->config->channels[channel].cb != NULL) {
      EXTI->IMR2 |= cmask;
      EXTI->EMR2 &= ~cmask;
    }
    else {
      EXTI->EMR2 |= cmask;
      EXTI->IMR2 &= ~cmask;
    }
  }
#endif
}

/**
 * @brief   Disables an EXT channel.
 *
 * @param[in] extp      pointer to the @p EXTDriver object
 * @param[in] channel   channel to be disabled
 *
 * @notapi
 */
void ext_lld_channel_disable(EXTDriver *extp, expchannel_t channel) {
  uint32_t cmask = (1 << (channel & 0x1F));

  (void)extp;

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

#endif /* HAL_USE_EXT */

/** @} */
