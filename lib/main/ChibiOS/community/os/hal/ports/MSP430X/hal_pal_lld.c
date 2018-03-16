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
 * @file    MSP430X/hal_pal_lld.c
 * @brief   MSP430X PAL subsystem low level driver source.
 *
 * @addtogroup PAL
 * @{
 */

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

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   MSP430X I/O ports configuration.
 * @details GPIO registers initialization
 *
 * @param[in] config    the MSP430X ports configuration
 *
 * @notapi
 */
void _pal_lld_init(const PALConfig *config) {

#if defined(PA_BASE) || defined(__DOXYGEN__)
  PAOUT = config->porta.out;
  PADIR = config->porta.dir;
  PAREN = config->porta.ren;
  PASEL0 = config->porta.sel0;
  PASEL1 = config->porta.sel1;
  PAIES = config->porta.ies;
  PAIE = config->porta.ie;
  PAIFG = 0;
#endif
#if defined(PB_BASE) || defined(__DOXYGEN__)
  PBOUT = config->portb.out;
  PBDIR = config->portb.dir;
  PBREN = config->portb.ren;
  PBSEL0 = config->portb.sel0;
  PBSEL1 = config->portb.sel1;
  PBIES = config->portb.ies;
  PBIE = config->portb.ie;
  PBIFG = 0;
#endif
#if defined(PC_BASE) || defined(__DOXYGEN__)
  PCOUT = config->portc.out;
  PCDIR = config->portc.dir;
  PCREN = config->portc.ren;
  PCSEL0 = config->portc.sel0;
  PCSEL1 = config->portc.sel1;
#if defined(PCIE) || defined(__DOXYGEN__)
  PCIES = config->portc.ies;
  PCIE = config->portc.ie;
  PCIFG = 0;
#endif
#endif
#if defined(PD_BASE) || defined(__DOXYGEN__)
  PDOUT = config->portd.out;
  PDDIR = config->portd.dir;
  PDREN = config->portd.ren;
  PDSEL0 = config->portd.sel0;
  PDSEL1 = config->portd.sel1;
#if defined(PDIE) || defined(__DOXYGEN__)
  PDIES = config->portd.ies;
  PDIE = config->portd.ie;
  PDIFG = 0;
#endif
#endif
#if defined(PE_BASE) || defined(__DOXYGEN__)
  PEOUT = config->porte.out;
  PEDIR = config->porte.dir;
  PEREN = config->porte.ren;
  PESEL0 = config->porte.sel0;
  PESEL1 = config->porte.sel1;
#if defined(PEIE) || defined(__DOXYGEN__)
  PEIES = config->porte.ies;
  PEIE = config->porte.ie;
  PEIFG = 0;
#endif
#endif
#if defined(PF_BASE) || defined(__DOXYGEN__)
  PFOUT = config->portf.out;
  PFDIR = config->portf.dir;
  PFREN = config->portf.ren;
  PFSEL0 = config->portf.sel0;
  PFSEL1 = config->portf.sel1;
#if defined(PFIE) || defined(__DOXYGEN__)
  PFIES = config->portf.ies;
  PFIE = config->portf.ie;
  PFIFG = 0;
#endif
#endif
  PJOUT = config->portj.out;
  PJDIR = config->portj.dir;
  PJREN = config->portj.ren;
  PJSEL0 = config->portj.sel0;
  PJSEL1 = config->portj.sel1;
  
  PM5CTL0 &= ~LOCKLPM5;
}

/**
 * @brief   Pads mode setup.
 * @details This function programs a pads group belonging to the same port
 *          with the specified mode.
 * @note    @p PAL_MODE_UNCONNECTED is implemented as input with pullup.
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
  
  switch (mode) {
    case PAL_MODE_RESET:
    case PAL_MODE_INPUT:
      port->dir &= ~mask;
      port->ren &= ~mask;
      if ((port->sel0 & mask) && (port->sel1 & mask))
        port->selc = mask;
      else {
        port->sel0 &= ~mask;
        port->sel1 &= ~mask;
      }
      break;
    case PAL_MODE_UNCONNECTED:
    case PAL_MODE_INPUT_PULLUP:
      port->dir &= ~mask;
      port->ren |= mask;
      port->out |= mask;
      if ((port->sel0 & mask) && (port->sel1 & mask))
        port->selc = mask;
      else {
        port->sel0 &= ~mask;
        port->sel1 &= ~mask;
      }
      break;
    case PAL_MODE_INPUT_PULLDOWN:
      port->dir &= ~mask;
      port->ren |= mask;
      port->out &= ~mask;
      if ((port->sel0 & mask) && (port->sel1 & mask))
        port->selc = mask;
      else {
        port->sel0 &= ~mask;
        port->sel1 &= ~mask;
      }
      break;
    case PAL_MODE_OUTPUT_PUSHPULL:
      port->dir |= mask;
      if ((port->sel0 & mask) && (port->sel1 & mask))
        port->selc = mask;
      else {
        port->sel0 &= ~mask;
        port->sel1 &= ~mask;
      }
      break;
    case PAL_MSP430X_ALTERNATE_1:
      if (!(port->sel0 & mask) && (port->sel1 & mask)) 
        port->selc = mask;
      else {
        port->sel0 |= mask;
        port->sel1 &= ~mask;
      }
      break;
    case PAL_MSP430X_ALTERNATE_2:
      if ((port->sel0 & mask) && !(port->sel1 & mask))
        port->selc = mask;
      else {
        port->sel0 &= ~mask;
        port->sel1 |= mask;
      }
      break;
    case PAL_MSP430X_ALTERNATE_3:
      if (!(port->sel0 & mask) && !(port->sel1 & mask))
        port->selc = mask;
      else {
        port->sel0 |= mask;
        port->sel1 |= mask;
      }
      break;
  }
}

#endif /* HAL_USE_PAL == TRUE */

/** @} */
