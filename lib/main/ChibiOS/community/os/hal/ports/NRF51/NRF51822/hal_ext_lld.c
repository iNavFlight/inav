/*
    Copyright (C) 2015 Stephen Caudle

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
 * @file    NRF51822/ext_lld.c
 * @brief   NRF51822 EXT subsystem low level driver source.
 *
 * @addtogroup EXT
 * @{
 */

#include "hal.h"

#if HAL_USE_EXT || defined(__DOXYGEN__)

#include "hal_ext_lld_isr.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

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

  unsigned i;

  ext_lld_exti_irq_enable();

  /* Configuration of automatic channels.*/
  for (i = 0; i < EXT_MAX_CHANNELS; i++) {
    uint32_t config = 0;
    uint32_t pad = (extp->config->channels[i].mode & EXT_MODE_GPIO_MASK)
      >> EXT_MODE_GPIO_OFFSET;

    if (extp->config->channels[i].mode & EXT_CH_MODE_BOTH_EDGES)
      config |= (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos);
    else if (extp->config->channels[i].mode & EXT_CH_MODE_RISING_EDGE)
      config |= (GPIOTE_CONFIG_POLARITY_LoToHi << GPIOTE_CONFIG_POLARITY_Pos);
    else
      config |= (GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos);

    config |= (pad << GPIOTE_CONFIG_PSEL_Pos);

    NRF_GPIOTE->CONFIG[i] = config;
    NRF_GPIOTE->EVENTS_PORT = 0;
    NRF_GPIOTE->EVENTS_IN[i] = 0;

    if (extp->config->channels[i].mode & EXT_CH_MODE_AUTOSTART)
      ext_lld_channel_enable(extp, i);
    else
      ext_lld_channel_disable(extp, i);
  }
}

/**
 * @brief   Deactivates the EXT peripheral.
 *
 * @param[in] extp      pointer to the @p EXTDriver object
 *
 * @notapi
 */
void ext_lld_stop(EXTDriver *extp) {

  unsigned i;

  (void)extp;
  ext_lld_exti_irq_disable();

  for (i = 0; i < EXT_MAX_CHANNELS; i++)
    NRF_GPIOTE->CONFIG[i] = 0;

  NRF_GPIOTE->INTENCLR =
    (GPIOTE_INTENCLR_IN3_Msk | GPIOTE_INTENCLR_IN2_Msk |
    GPIOTE_INTENCLR_IN1_Msk | GPIOTE_INTENCLR_IN0_Msk);
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

  uint32_t config = NRF_GPIOTE->CONFIG[channel] & ~GPIOTE_CONFIG_MODE_Msk;

  (void)extp;
  config |= (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);

  NRF_GPIOTE->CONFIG[channel] = config;
  NRF_GPIOTE->INTENSET = (1 << channel);
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

  (void)extp;
  NRF_GPIOTE->CONFIG[channel] &= ~GPIOTE_CONFIG_MODE_Msk;
  NRF_GPIOTE->INTENCLR = (1 << channel);
}

#endif /* HAL_USE_EXT */

/** @} */
