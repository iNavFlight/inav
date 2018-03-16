/*
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
 * @file    pal_lld.c
 * @brief   NRF51822 PAL subsystem low level driver source.
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

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

void _pal_lld_setpadmode(ioportid_t port, uint8_t pad, iomode_t mode)
{
  (void)port;
  osalDbgAssert(pad <= 31, "pal_lld_setpadmode() - invalid pad");

  switch (mode) {
  case PAL_MODE_RESET:
  case PAL_MODE_UNCONNECTED:
    NRF_GPIO->PIN_CNF[pad] =
      (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
      (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
      (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
      (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
      (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
    break;
  case PAL_MODE_INPUT:
  case PAL_MODE_INPUT_ANALOG:
    NRF_GPIO->PIN_CNF[pad] =
      (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
      (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
      (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
      (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
      (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
    break;
  case PAL_MODE_INPUT_PULLUP:
    NRF_GPIO->PIN_CNF[pad] =
      (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
      (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
      (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos) |
      (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
      (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
    break;
  case PAL_MODE_INPUT_PULLDOWN:
    NRF_GPIO->PIN_CNF[pad] =
      (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
      (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
      (GPIO_PIN_CNF_PULL_Pulldown << GPIO_PIN_CNF_PULL_Pos) |
      (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
      (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
    break;
  case PAL_MODE_OUTPUT_PUSHPULL:
    NRF_GPIO->PIN_CNF[pad] =
      (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) |
      (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
      (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
      (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
      (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
    break;
  case PAL_MODE_OUTPUT_OPENDRAIN:
    NRF_GPIO->PIN_CNF[pad] =
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
 * @brief   NRF51 I/O ports configuration.
 *
 * @param[in] config    the NRF51 ports configuration
 *
 * @notapi
 */
void _pal_lld_init(const PALConfig *config)
{
  uint8_t i;

  for (i = 0; i < TOTAL_GPIO_PADS; i++) {
    pal_lld_setpadmode(IOPORT1, i, config->pads[i]);
  }
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

#endif /* HAL_USE_PAL == TRUE */

/** @} */
