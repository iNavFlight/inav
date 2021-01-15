/*
    Copyright (C) 2014..2017 Marco Veeneman

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
 * @file    TIVA/TM4C129x/hal_lld.c
 * @brief   TM4C129x HAL Driver subsystem low level driver source.
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
void hal_lld_init(void)
{
}

/**
 * @brief   TM4C129x clocks and PLL initialization.
 * @note    All the involved constants come from the file @p board.h and 
 *          @p mcuconf.h.
 * @note    This function should be invoked just after the system reset.
 *
 * @special
 */
void tiva_clock_init(void)
{
  uint32_t moscctl, rsclkcfg;

  /*
   * 1. Once POR has completed, the PIOSC is acting as the system clock.
   */

  /*
   * 2. Power up the MOSC by clearing the NOXTAL bit in the MOSCCTL register.
   */
  moscctl = HWREG(SYSCTL_MOSCCTL);
  moscctl &= ~SYSCTL_MOSCCTL_NOXTAL;

  /*
   * 3. If single-ended MOSC mode is required, the MOSC is ready to use. If crystal mode is required,
   * clear the PWRDN bit and wait for the MOSCPUPRIS bit to be set in the Raw Interrupt Status
   * (RIS), indicating MOSC crystal mode is ready.
   */
#if TIVA_MOSC_SINGLE_ENDED
  HWREG(SYSCTL_MOSCCTL) = moscctl;
#else
  moscctl &= ~SYSCTL_MOSCCTL_PWRDN;
  HWREG(SYSCTL_MOSCCTL) = moscctl;

  while (!(HWREG(SYSCTL_RIS) & SYSCTL_RIS_MOSCPUPRIS));
#endif

  /*
   * 4. Set the OSCSRC field to 0x3 in the RSCLKCFG register at offset 0x0B0.
   */
  rsclkcfg = HWREG(SYSCTL_RSCLKCFG);

  rsclkcfg |= TIVA_RSCLKCFG_OSCSRC;

  /*
   * 5. If the application also requires the MOSC to be the deep-sleep clock source, then program the
   * DSOSCSRC field in the DSCLKCFG register to 0x3.
   */

  /*
   * 6. Write the PLLFREQ0 and PLLFREQ1 registers with the values of Q, N, MINT, and MFRAC to
   * the configure the desired VCO frequency setting.
   */
  HWREG(SYSCTL_PLLFREQ1) = (0x04 << 0); // 5 - 1
  HWREG(SYSCTL_PLLFREQ0) = (0x60 << 0) | SYSCTL_PLLFREQ0_PLLPWR;

  /*
   * 7. Write the MEMTIM0 register to correspond to the new system clock setting.
   */
  HWREG(SYSCTL_MEMTIM0) = (SYSCTL_MEMTIM0_FBCHT_3_5 | (5 << SYSCTL_MEMTIM0_FWS_S) | SYSCTL_MEMTIM0_EBCHT_3_5 | (5 << SYSCTL_MEMTIM0_EWS_S) | SYSCTL_MEMTIM0_MB1);

  /*
   * Wait for the PLLSTAT register to indicate the PLL has reached lock at the new operating point
   * (or that a timeout period has passed and lock has failed, in which case an error condition exists
   * and this sequence is abandoned and error processing is initiated).
   */
  while (!HWREG(SYSCTL_PLLSTAT) & SYSCTL_PLLSTAT_LOCK);

  /*
   * 9. Write the RSCLKCFG register's PSYSDIV value, set the USEPLL bit to enabled, and MEMTIMU
   * bit.
   */

  rsclkcfg = HWREG(SYSCTL_RSCLKCFG);

  rsclkcfg |= (SYSCTL_RSCLKCFG_USEPLL | (0x03 << 0) | (0x03 << 20) | (0x03 << 24));

  //rsclkcfg |= ((0x03 << 0) | (1 << 28) | (0x03 << 20));

  rsclkcfg |= SYSCTL_RSCLKCFG_MEMTIMU;

  // set new configuration
  HWREG(SYSCTL_RSCLKCFG) = rsclkcfg;

#if HAL_USE_PWM
#if TIVA_PWM_USE_PWM0
  HWREG(PWM0_CC) = TIVA_PWM_FIELDS;
#endif
#endif
}

/** @} */
