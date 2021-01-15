/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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
 * @file    hal_lld.c
 * @brief   PLATFORM HAL subsystem low level driver source.
 *
 * @addtogroup HAL
 * @{
 */

#include "hal.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define SAMA_MOR_ONE                        (1 << 5)
#define SAMA_MOR_MOSCXTST                   CKGR_MOR_MOSCXTST(0xFF)

#define SAMA_PLLA_ONE                       (1 << 29)

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

#if (SAMA_HAL_IS_SECURE == TRUE)    /* The Matrix is PAS and PMC is always configured secure */
  /* Disabling PMC write protection. */
  pmcDisableWP();

  PIT->PIT_MR &= ~(PIT_MR_PITEN | PIT_MR_PITIEN);

  /* Configures peripherals as not-secure */
  MATRIX0->MATRIX_SPSELR[0] = 0xFFFFFFFF;
  MATRIX0->MATRIX_SPSELR[1] = 0xFFFFFFFF;
  MATRIX0->MATRIX_SPSELR[2] = 0xFFFFFFFF;
  MATRIX1->MATRIX_SPSELR[0] = 0xFFFFFFFF;
  MATRIX1->MATRIX_SPSELR[1] = 0xFFFFFFFF;
  MATRIX1->MATRIX_SPSELR[2] = 0xFFFFFFFF;

  /* Configures PMC and RTC as secure */
  //mtxConfigPeriphSecurity(MATRIX1, ID_SYSC, SECURE_PER);
  mtxConfigPeriphSecurity(MATRIX0, ID_PMC, SECURE_PER);
  mtxConfigPeriphSecurity(MATRIX1, ID_SFC, SECURE_PER);
  mtxConfigPeriphSecurity(MATRIX1, ID_SFR, SECURE_PER);

  /* It isn't necessary to make L2CC secure. L2C-310 cache
   * controller is secure mode aware.*/
  /*mtxConfigPeriphSecurity(MATRIX0, ID_L2CC, SECURE_PER);*/

  mtxConfigPeriphSecurity(MATRIX1, ID_SFRBU, SECURE_PER);
  /*mtxConfigPeriphSecurity(MATRIX1, ID_CHIPID, SECURE_PER);*/

  /* Enabling matrix clock */
  pmcEnableH32MX();
  pmcEnableH64MX();
  /* Enabling write protection.  */
  pmcEnableWP();
#endif

  /* Advanced interrupt controller init */
  aicInit();

#if defined(SAMA_DMA_REQUIRED)
  dmaInit();
#endif

}

/**
 * @brief   SAMA clocks and PLL initialization.
 * @note    All the involved constants come from the file @p board.h.
 * @note    This function should be invoked just after the system reset.
 *
 * @special
 */
void sama_clock_init(void) {
#if (!SAMA_NO_INIT && SAMA_HAL_IS_SECURE == TRUE)
  uint32_t mor, pllar, mckr, mainf;
  /* Disabling PMC write protection. */
  pmcDisableWP();

  /* 
   * Enforcing the reset default configuration of clock tree. 
   */  
  /* Setting Slow Clock source to OSCRC. */
  SCKC->SCKC_CR = 0U;

  /* Enabling MOSCRC. */
  PMC->CKGR_MOR |= (CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCRCEN);
  while (!(PMC->PMC_SR & PMC_SR_MOSCRCS))
    ;                                       /* Waits until MOSCRC is stable.*/

  /* Switching Main Oscillator Source to MOSRC. */
  mor = PMC->CKGR_MOR | CKGR_MOR_KEY_PASSWD;
  mor &= ~CKGR_MOR_MOSCSEL;
  mor |= SAMA_MOSC_MOSCRC;
  PMC->CKGR_MOR = mor;

  while (!(PMC->PMC_SR & PMC_SR_MOSCSELS))
    ;                                       /* Waits until MOSCSEL has changed.*/

  /* Switching Master Clock source to Main Clock. */
  mckr = PMC->PMC_MCKR;
  mckr &= ~PMC_MCKR_CSS_Msk;
  mckr |= PMC_MCKR_CSS_MAIN_CLK;
  PMC->PMC_MCKR = mckr;

  while (!(PMC->PMC_SR & PMC_SR_MCKRDY))
    ;                                       /* Waits until Master Clock is stable.*/

  /* Counter Clock Source to MOSCRC. */
  PMC->CKGR_MCFR &= ~CKGR_MCFR_CCSS;


  /*
   * Main oscillator configuration block.
   */
  /* Setting Slow clock source. */
  SCKC->SCKC_CR = SAMA_OSC_SEL;
  while ((SAMA_OSC_SEL && !(PMC->PMC_SR & PMC_SR_OSCSELS)) ||
        (!SAMA_OSC_SEL &&  (PMC->PMC_SR & PMC_SR_OSCSELS)))
    ;                                       /* Waits until MOSCxxS switch is done.*/
  mor = PMC->CKGR_MOR | CKGR_MOR_KEY_PASSWD;
#if SAMA_MOSCXT_ENABLED
  mor |= CKGR_MOR_MOSCXTEN;
  PMC->CKGR_MOR = mor;
  while (!(PMC->PMC_SR & PMC_SR_MOSCXTS))
    ;                                       /* Waits until MOSCXT is stable.*/
  /* Counter Clock Source to MOSCXT. */
  PMC->CKGR_MCFR |= CKGR_MCFR_CCSS;
#else
  mor &= ~CKGR_MOR_MOSCXTEN;
  PMC->CKGR_MOR = mor;
#endif

  PMC->CKGR_MCFR |= CKGR_MCFR_RCMEAS;
  while (!(PMC->CKGR_MCFR & CKGR_MCFR_MAINFRDY))
    ;
  mainf = CKGR_MCFR_MAINF(PMC->CKGR_MCFR);
  /*
   * @TODO: add mainf check and eventual clock source fallback. This mechanism
   * should be activable through a switch.
   */
  (void)mainf;

  /* Switching Main Clock source. */
  mor &= ~CKGR_MOR_MOSCSEL;
  mor |= SAMA_MOSC_SEL;
  PMC->CKGR_MOR = mor;

  /* Eventually disabling MOSCRC. */
#if !SAMA_MOSCRC_ENABLED
  PMC->CKGR_MOR &= ~ CKGR_MOR_MOSCRCEN;
#endif

/*
 * PLLA configuration block.
 */
  pllar = SAMA_PLLA_ONE | CKGR_PLLAR_PLLACOUNT(0x3F);
#if SAMA_ACTIVATE_PLLA
  pllar |= CKGR_PLLAR_DIVA_BYPASS | SAMA_PLLA_MUL;
#endif
  PMC->CKGR_PLLAR = pllar;                  /* Writing PLLA register.       */

#if SAMA_ACTIVATE_PLLA
  while (!(PMC->PMC_SR & PMC_SR_LOCKA))
    ;                                       /* Waits until PLLA is locked.  */
#endif

/*
 * Master clock configuration block.
 */
  mckr = PMC->PMC_MCKR;
  mckr &= ~PMC_MCKR_CSS_Msk;
  mckr |= SAMA_MCK_SEL;
  PMC->PMC_MCKR = mckr;
  while (!(PMC->PMC_SR & PMC_SR_MCKRDY))
    ;                                       /* Waits until MCK is stable.   */

  mckr &= ~(PMC_MCKR_PRES_Msk | PMC_MCKR_MDIV_Msk | PMC_MCKR_H32MXDIV);
  
  /* Note that prescaler and divider must be changed with separate accesses.*/
  mckr |= SAMA_MCK_PRES;
  mckr |= SAMA_MCK_MDIV;
  mckr |= SAMA_H64MX_H32MX_DIV;
#if SAMA_PLLADIV2_EN
  mckr |= PMC_MCKR_PLLADIV2;
#else
  mckr &= ~PMC_MCKR_PLLADIV2;
#endif
  PMC->PMC_MCKR = mckr;
  while (!(PMC->PMC_SR & PMC_SR_MCKRDY))
    ;                                       /* Waits until MCK is stable.   */

  /* Enabling write protection.  */
  pmcEnableWP();

#endif /* !SAMA_NO_INIT */
}
/** @} */
