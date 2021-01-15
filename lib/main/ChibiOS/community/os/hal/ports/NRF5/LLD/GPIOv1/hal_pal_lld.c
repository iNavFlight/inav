/*
    Copyright (C) 2018 Konstantin Oblaukhov
    Copyright (C) 2015 Fabio Utzig

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
 * @file    GPIOv1/hal_pal_lld.c
 * @brief   NRF5 PAL subsystem low level driver source.
 *
 * @addtogroup PAL
 * @{
 */

#include "osal.h"
#include "hal.h"

#if (HAL_USE_PAL == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   Event records for the GPIOTE channels.
 */
palevent_t _pal_events[NRF5_GPIOTE_NUM_CHANNELS];

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

void _pal_lld_setpadmode(ioportid_t port, uint8_t pad, iomode_t mode)
{
  (void)port;
  osalDbgAssert(pad < PAL_IOPORTS_WIDTH, "pal_lld_setpadmode() - invalid pad");

  switch (mode) {
  case PAL_MODE_RESET:
  case PAL_MODE_UNCONNECTED:
    IOPORT1->PIN_CNF[pad] =
      (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
      (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
      (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
      (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
      (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
    break;
  case PAL_MODE_INPUT:
  case PAL_MODE_INPUT_ANALOG:
    IOPORT1->PIN_CNF[pad] =
      (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
      (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
      (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
      (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
      (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
    break;
  case PAL_MODE_INPUT_PULLUP:
    IOPORT1->PIN_CNF[pad] =
      (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
      (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
      (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos) |
      (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
      (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
    break;
  case PAL_MODE_INPUT_PULLDOWN:
    IOPORT1->PIN_CNF[pad] =
      (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
      (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
      (GPIO_PIN_CNF_PULL_Pulldown << GPIO_PIN_CNF_PULL_Pos) |
      (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
      (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
    break;
  case PAL_MODE_OUTPUT_PUSHPULL:
    IOPORT1->PIN_CNF[pad] =
      (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
      (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
      (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
      (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
      (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
    break;
  case PAL_MODE_OUTPUT_OPENDRAIN:
    IOPORT1->PIN_CNF[pad] =
      (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
      (GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos) |
      (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
      (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
      (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
    break;
  default:
    osalDbgAssert(FALSE, "invalid pal mode");
    break;
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   NRF5 I/O ports configuration.
 *
 * @param[in] config    the NRF5 ports configuration
 *
 * @notapi
 */
void _pal_lld_init(const PALConfig *config)
{
  uint8_t i;

  for (i = 0; i < TOTAL_GPIO_PADS; i++) {
    pal_lld_setpadmode(IOPORT1, i, config->pads[i]);
  }
  
#if PAL_USE_CALLBACKS || PAL_USE_WAIT || defined(__DOXYGEN__)
  for (i = 0; i < NRF5_GPIOTE_NUM_CHANNELS; i++) {
    _pal_init_event(i);
  }
#endif
}

/**
 * @brief   Pads mode setup.
 * @details This function programs a pads group belonging to the same port
 *          with the specified mode.
 *
 * @param[in] port      the port identifier
 * @param[in] mask      the group mask
 * @param[in] mode      the mode
 *
 * @notapi
 */
void _pal_lld_setgroupmode(ioportid_t port,
                           ioportmask_t mask,
                           iomode_t mode)
{
  uint8_t i;

  for (i = 0; i < TOTAL_GPIO_PADS; i++, mask >>= 1) {
    if (mask & 1) {
      pal_lld_setpadmode(port, i, mode);
    }
  }
}

#if PAL_USE_CALLBACKS || PAL_USE_WAIT || defined(__DOXYGEN__)
/**
 * @brief   Pad event enable.
 * @note    Programming an unknown or unsupported mode is silently ignored.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 * @param[in] mode      pad event mode
 *
 * @notapi
 */
void _pal_lld_enablepadevent(ioportid_t port,
                             iopadid_t pad,
                             ioeventmode_t mode) {
    (void)port;

    int ch = NRF5_PAL_PAD_TO_EVENT(pad);
    uint32_t config = NRF_GPIOTE->CONFIG[ch];

    osalDbgAssert((((config & GPIOTE_CONFIG_PSEL_Msk) >> GPIOTE_CONFIG_PSEL_Pos) == pad) ||
                  (((config & GPIOTE_CONFIG_MODE_Msk) >> GPIOTE_CONFIG_MODE_Pos) != GPIOTE_CONFIG_MODE_Event),
                  "channel already in use");

    if ((mode & PAL_EVENT_MODE_RISING_EDGE) && (mode & PAL_EVENT_MODE_FALLING_EDGE))
      config |= (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos);
    else if (mode & PAL_EVENT_MODE_RISING_EDGE)
      config |= (GPIOTE_CONFIG_POLARITY_LoToHi << GPIOTE_CONFIG_POLARITY_Pos);
    else
      config |= (GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos);

    config |= (pad << GPIOTE_CONFIG_PSEL_Pos);

    config |= (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);

    NRF_GPIOTE->CONFIG[ch] = config;
    NRF_GPIOTE->EVENTS_PORT = 0;
    NRF_GPIOTE->EVENTS_IN[ch] = 0;
    NRF_GPIOTE->INTENSET = (1 << ch);
}

/**
 * @brief   Pad event disable.
 * @details This function disables previously programmed event callbacks.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 *
 * @notapi
 */
void _pal_lld_disablepadevent(ioportid_t port, iopadid_t pad) {
  (void)port;
  int ch = NRF5_PAL_PAD_TO_EVENT(pad);
  uint32_t config = NRF_GPIOTE->CONFIG[ch];

  if (((config & GPIOTE_CONFIG_MODE_Msk) >> GPIOTE_CONFIG_MODE_Pos) == GPIOTE_CONFIG_MODE_Event)
  {
    osalDbgAssert(((config & GPIOTE_CONFIG_PSEL_Msk) >> GPIOTE_CONFIG_PSEL_Pos) == pad,
                  "channel mapped on different pad");

    NRF_GPIOTE->INTENSET &= ~(1 << ch);
    NRF_GPIOTE->CONFIG[ch] = 0;

#if PAL_USE_CALLBACKS || PAL_USE_WAIT
  /* Callback cleared and/or thread reset.*/
  _pal_clear_event(pad);
#endif
  }
}
#endif /* PAL_USE_CALLBACKS || PAL_USE_WAIT */

#endif /* HAL_USE_PAL == TRUE */

/** @} */
