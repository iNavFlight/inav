/*
    ChibiOS - Copyright (C) 2014-2015 Fabio Utzig

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
 * @file    templates/hal_lld.c
 * @brief   HAL Driver subsystem low level driver source template.
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

#ifdef __CC_ARM
__attribute__ ((section(".ARM.__at_0x400")))
#else
__attribute__ ((section(".cfmconfig")))
#endif
const uint8_t _cfm[0x10] = {
  0xFF,  /* NV_BACKKEY3: KEY=0xFF */
  0xFF,  /* NV_BACKKEY2: KEY=0xFF */
  0xFF,  /* NV_BACKKEY1: KEY=0xFF */
  0xFF,  /* NV_BACKKEY0: KEY=0xFF */
  0xFF,  /* NV_BACKKEY7: KEY=0xFF */
  0xFF,  /* NV_BACKKEY6: KEY=0xFF */
  0xFF,  /* NV_BACKKEY5: KEY=0xFF */
  0xFF,  /* NV_BACKKEY4: KEY=0xFF */
  0xFF,  /* NV_FPROT3: PROT=0xFF */
  0xFF,  /* NV_FPROT2: PROT=0xFF */
  0xFF,  /* NV_FPROT1: PROT=0xFF */
  0xFF,  /* NV_FPROT0: PROT=0xFF */
  0x7E,  /* NV_FSEC: KEYEN=1,MEEN=3,FSLACC=3,SEC=2 */
  0xFF,  /* NV_FOPT: ??=1,??=1,FAST_INIT=1,LPBOOT1=1,RESET_PIN_CFG=1,
                      NMI_DIS=1,EZPORT_DIS=1,LPBOOT0=1 */
  0xFF,
  0xFF
};

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
 * @todo    Use a macro to define the system clock frequency.
 *
 * @notapi
 */
void hal_lld_init(void) {

}

/**
 * @brief   MK20D5 clock initialization.
 * @note    All the involved constants come from the file @p board.h.
 * @note    This function is meant to be invoked early during the system
 *          initialization, it is usually invoked from the file
 *          @p board.c.
 * @todo    This function needs to be more generic.
 *
 * @special
 */
void mk20d50_clock_init(void) {
#if !KINETIS_NO_INIT

#if KINETIS_MCG_MODE == KINETIS_MCG_MODE_PEE
  uint32_t ratio, frdiv;
  uint32_t ratios[] = { 32, 64, 128, 256, 512, 1024, 1280, 1536 };
  int ratio_quantity = sizeof(ratios) / sizeof(ratios[0]);
  int i;
#endif /* KINETIS_MCG_MODE == KINETIS_MCG_MODE_PEE */

  /* Disable the watchdog */
  WDOG->UNLOCK = 0xC520;
  WDOG->UNLOCK = 0xD928;
  WDOG->STCTRLH &= ~WDOG_STCTRLH_WDOGEN;

  SIM->SCGC5 |= SIM_SCGC5_PORTA |
                SIM_SCGC5_PORTB |
                SIM_SCGC5_PORTC |
                SIM_SCGC5_PORTD |
                SIM_SCGC5_PORTE;

#if KINETIS_MCG_MODE == KINETIS_MCG_MODE_FEI

  /* Configure FEI mode */
  MCG->C4 = MCG_C4_DRST_DRS(KINETIS_MCG_FLL_DRS) |
            (KINETIS_MCG_FLL_DMX32 ? MCG_C4_DMX32 : 0);

#endif /* KINETIS_MCG_MODE == KINETIS_MCG_MODE_FEI */

#if KINETIS_MCG_MODE == KINETIS_MCG_MODE_PEE

  /* EXTAL0 and XTAL0 */
  PORTA->PCR[18] = 0;
  PORTA->PCR[19] = 0;

  /*
   * Start in FEI mode
   */

  /* Disable capacitors for crystal */
  OSC->CR = 0;

  /* TODO: need to add more flexible calculation, specially regarding
   *       divisors which may not be available depending on the XTAL
   *       frequency, which would required other registers to be modified.
   */
  /* Enable OSC, low power mode */
  MCG->C2 = MCG_C2_LOCRE0 | MCG_C2_EREFS0;
  if (KINETIS_XTAL_FREQUENCY > 8000000)
    MCG->C2 |= MCG_C2_RANGE0(2);
  else
    MCG->C2 |= MCG_C2_RANGE0(1);

  frdiv = 7;
  ratio = KINETIS_XTAL_FREQUENCY / 31250;
  for (i = 0; i < ratio_quantity; ++i) {
    if (ratio == ratios[i]) {
      frdiv = i;
      break;
    }
  }

  /* Switch to crystal as clock source, FLL input of 31.25 KHz */
  MCG->C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(frdiv);

  /* Wait for crystal oscillator to begin */
  while (!(MCG->S & MCG_S_OSCINIT0));

  /* Wait for the FLL to use the oscillator */
  while (MCG->S & MCG_S_IREFST);

  /* Wait for the MCGOUTCLK to use the oscillator */
  while ((MCG->S & MCG_S_CLKST_MASK) != MCG_S_CLKST(2));

  /*
   * Now in FBE mode
   */

  /* Config PLL input for 2 MHz */
  MCG->C5 = MCG_C5_PRDIV0((KINETIS_XTAL_FREQUENCY / 2000000) - 1);

  /* Config PLL for 96 MHz output */
  MCG->C6 = MCG_C6_PLLS | MCG_C6_VDIV0(0);

  /* Wait for PLL to start using crystal as its input */
  while (!(MCG->S & MCG_S_PLLST));

  /* Wait for PLL to lock */
  while (!(MCG->S & MCG_S_LOCK0));

  /*
   * Now in PBE mode
   */

  /* Switch to PLL as clock source */
  MCG->C1 = MCG_C1_CLKS(0);

  /* Wait for PLL clock to be used */
  while ((MCG->S & MCG_S_CLKST_MASK) != MCG_S_CLKST_PLL);

  /*
   * Now in PEE mode
   */
#endif /* KINETIS_MCG_MODE == KINETIS_MCG_MODE_PEE */

#endif /* !KINETIS_NO_INIT */
}

/** @} */
