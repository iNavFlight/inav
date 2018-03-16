/*
    ChibiOS - Copyright (C) 2013..2015 Fabio Utzig

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
 * @file    KL2x/pal_lld.c
 * @brief   Kinetis KL2x PAL subsystem low level driver.
 *
 * @addtogroup PAL
 * @{
 */

#include "osal.h"
#include "hal.h"

#if HAL_USE_PAL || defined(__DOXYGEN__)

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

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   STM32 I/O ports configuration.
 * @details Ports A-D(E, F, G, H) clocks enabled.
 *
 * @param[in] config    the STM32 ports configuration
 *
 * @notapi
 */
void _pal_lld_init(const PALConfig *config) {

  int i, j;

  /* Enable clocking of all Ports */
  SIM->SCGC5 |= SIM_SCGC5_PORTA |
                SIM_SCGC5_PORTB |
                SIM_SCGC5_PORTC |
                SIM_SCGC5_PORTD |
                SIM_SCGC5_PORTE;

  for (i = 0; i < TOTAL_PORTS; i++) {
    for (j = 0; j < PADS_PER_PORT; j++) {
      pal_lld_setpadmode(config->ports[i].port,
                         j,
                         config->ports[i].pads[j]);
    }
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
                           iomode_t mode) {

  (void)port;
  (void)mask;
  (void)mode;

}

/**
 * @brief   Reads a logical state from an I/O pad.
 * @note    The @ref PAL provides a default software implementation of this
 *          functionality, implement this function if can optimize it by using
 *          special hardware functionalities or special coding.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 * @return              The logical state.
 * @retval PAL_LOW      low logical state.
 * @retval PAL_HIGH     high logical state.
 *
 * @notapi
 */
uint8_t pal_lld_readpad(ioportid_t port, uint8_t pad)
{
  return (port->PDIR & ((uint32_t) 1 << pad)) ? PAL_HIGH : PAL_LOW;
}

/**
 * @brief   Writes a logical state on an output pad.
 * @note    This function is not meant to be invoked directly by the
 *          application  code.
 * @note    The @ref PAL provides a default software implementation of this
 *          functionality, implement this function if can optimize it by using
 *          special hardware functionalities or special coding.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 * @param[in] bit       logical value, the value must be @p PAL_LOW or
 *                      @p PAL_HIGH
 *
 * @notapi
 */
void pal_lld_writepad(ioportid_t port, uint8_t pad, uint8_t bit)
{
  if (bit == PAL_HIGH)
    port->PDOR |= ((uint32_t) 1 << pad);
  else
    port->PDOR &= ~((uint32_t) 1 << pad);
}

/**
 * @brief   Pad mode setup.
 * @details This function programs a pad with the specified mode.
 * @note    The @ref PAL provides a default software implementation of this
 *          functionality, implement this function if can optimize it by using
 *          special hardware functionalities or special coding.
 * @note    Programming an unknown or unsupported mode is silently ignored.
 *
 * @param[in] port      port identifier
 * @param[in] pad       pad number within the port
 * @param[in] mode      pad mode
 *
 * @notapi
 */
void _pal_lld_setpadmode(ioportid_t port, uint8_t pad, iomode_t mode)
{
  PORT_TypeDef *portcfg = NULL;

  osalDbgAssert(pad <= 31, "pal_lld_setpadmode() - invalid pad");

  if (mode == PAL_MODE_OUTPUT_PUSHPULL)
    port->PDDR |=  ((uint32_t) 1 << pad);
  else
    port->PDDR &= ~((uint32_t) 1 << pad);

  if (port == IOPORT1)
    portcfg = PORTA;
  else if (port == IOPORT2)
    portcfg = PORTB;
  else if (port == IOPORT3)
    portcfg = PORTC;
  else if (port == IOPORT4)
    portcfg = PORTD;
  else if (port == IOPORT5)
    portcfg = PORTE;

  osalDbgAssert(portcfg != NULL, "pal_lld_setpadmode() - invalid port");

  switch (mode) {
  case PAL_MODE_RESET:
  case PAL_MODE_INPUT:
  case PAL_MODE_OUTPUT_PUSHPULL:
    portcfg->PCR[pad] = PORTx_PCRn_MUX(1);
    break;
  case PAL_MODE_INPUT_PULLUP:
    portcfg->PCR[pad] = PORTx_PCRn_MUX(1) | PORTx_PCRn_PE | PORTx_PCRn_PS;
    break;
  case PAL_MODE_INPUT_PULLDOWN:
    portcfg->PCR[pad] = PORTx_PCRn_MUX(1) | PORTx_PCRn_PE;
    break;
  case PAL_MODE_UNCONNECTED:
  case PAL_MODE_INPUT_ANALOG:
    portcfg->PCR[pad] = PORTx_PCRn_MUX(0);
    break;
  case PAL_MODE_ALTERNATIVE_1:
    portcfg->PCR[pad] = PORTx_PCRn_MUX(1);
    break;
  case PAL_MODE_ALTERNATIVE_2:
    portcfg->PCR[pad] = PORTx_PCRn_MUX(2);
    break;
  case PAL_MODE_ALTERNATIVE_3:
    portcfg->PCR[pad] = PORTx_PCRn_MUX(3);
    break;
  case PAL_MODE_ALTERNATIVE_4:
    portcfg->PCR[pad] = PORTx_PCRn_MUX(4);
    break;
  case PAL_MODE_ALTERNATIVE_5:
    portcfg->PCR[pad] = PORTx_PCRn_MUX(5);
    break;
  case PAL_MODE_ALTERNATIVE_6:
    portcfg->PCR[pad] = PORTx_PCRn_MUX(6);
    break;
  case PAL_MODE_ALTERNATIVE_7:
    portcfg->PCR[pad] = PORTx_PCRn_MUX(7);
    break;
  }
}

#endif /* HAL_USE_PAL */

/** @} */
