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
 * @file    MSP430X/hal_lld.c
 * @brief   MSP430X HAL subsystem low level driver source.
 *
 * @addtogroup HAL
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
 * @brief   Low level HAL driver initialization.
 *
 * @notapi
 */
void hal_lld_init(void) {
  /* Disable watchdog */
  /* TODO Real watchdog support */
  WDTCTL = WDTPW | WDTHOLD;
  /* Init clock system */
  CSCTL0 = CSKEY; /* unlock clock system */
  CSCTL1 = MSP430X_DCOSEL;
  CSCTL2 = (MSP430X_ACLK_SRC << 8) | (MSP430X_SMCLK_SRC << 4) | (MSP430X_MCLK_SRC);
  CSCTL3 = (DIVIDER(MSP430X_ACLK_DIV) << 8) | (DIVIDER(MSP430X_SMCLK_DIV) << 4) | (DIVIDER(MSP430X_MCLK_DIV));
  CSCTL4 = (MSP430X_HFXTCLK_DRIVE << 14) | (MSP430X_HFXTCLK_BYPASS << 12) | (MSP430X_HFFREQ << 10) | HFXTOFF | \
           (MSP430X_LFXTCLK_DRIVE << 6) | (MSP430X_LFXTCLK_BYPASS << 4) | VLOOFF | LFXTOFF;
  CSCTL6 = (MODCLKREQEN) | (SMCLKREQEN) | (MCLKREQEN) | (ACLKREQEN);
  #if defined(MSP430X_USE_HFXT) && defined(MSP430X_USE_LFXT)
  do {
    CSCTL5 &= ~(HFXTOFFG | LFXTOFFG);
    SFRIFG1 &= ~OFIFG;
  } while (SFRIFG1 & OFIFG);
  #elif defined(MSP430X_USE_HFXT)
  do {
    CSCTL5 &= ~(HFXTOFFG);
    SFRIFG1 &= ~OFIFG;
  } while (SFRIFG1 & OFIFG);
  #elif defined(MSP430X_USE_LFXT)
  do {
    CSCTL5 &= ~(LFXTOFFG);
    SFRIFG1 &= ~OFIFG;
  } while (SFRIFG1 & OFIFG);
  #endif
  CSCTL0_H = 0xFF; /* Lock clock system */
  
#if (HAL_USE_DMA == TRUE)
  dmaInit();
#endif
}

/** @} */
