/*
    SPC5 HAL - Copyright (C) 2013 STMicroelectronics

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
 * @file    SPC56ELxx/hal_lld.c
 * @brief   SPC56ELxx HAL subsystem low level driver source.
 *
 * @addtogroup HAL
 * @{
 */

#include "hal.h"

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

/**
 * @brief   PIT channel 0 interrupt handler.
 *
 * @isr
 */

OSAL_IRQ_HANDLER(vector226) {

  OSAL_IRQ_PROLOGUE();

  osalSysLockFromISR();
  osalOsTimerHandlerI();
  osalSysUnlockFromISR();

  /* Resets the PIT channel 0 IRQ flag.*/
  PIT_0.TIMER[0].TFLG.R = 1;

  OSAL_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level HAL driver initialization.
 *
 * @notapi
 */
void hal_lld_init(void) {
  uint32_t n;

  /* The system is switched to the RUN0 mode, the default for normal
     operations.*/
  if (halSPCSetRunMode(SPC5_RUNMODE_RUN0) == OSAL_FAILED) {
    SPC5_CLOCK_FAILURE_HOOK();
  }

  /* PIT_0 clock initialization.*/
  halSPCSetPeripheralClockMode(30, SPC5_ME_PCTL_RUN(2) | SPC5_ME_PCTL_LP(2));

  /* TB counter enabled for debug and measurements.*/
  asm volatile ("li      %%r3, 0x4000       \t\n"   /* TBEN bit.            */
                "mtspr   1008, %%r3"                /* HID0 register.       */
                : : : "r3");

  /* PIT channel 0 initialization for Kernel ticks, the PIT is configured
     to run in DRUN,RUN0...RUN3 and HALT0 modes, the clock is gated in other
     modes.*/
  INTC_PSR(226)      = SPC5_PIT0_IRQ_PRIORITY;
  n = SPC5_PER_CLK / OSAL_ST_FREQUENCY - 1;
  PIT_0.MCR.R            = 1;       /* Clock enabled, stop while debugging. */
  PIT_0.TIMER[0].LDVAL.R = n;
  PIT_0.TIMER[0].CVAL.R  = n;
  PIT_0.TIMER[0].TFLG.R  = 1;       /* Interrupt flag cleared.              */
  PIT_0.TIMER[0].TCTRL.R = 3;       /* Timer active, interrupt enabled.     */

  /* EDMA initialization.*/
//  edmaInit();
}

/**
 * @brief   SPC56ELxx early initialization.
 * @note    All the involved constants come from the file @p board.h and
 *          @p hal_lld.h
 * @note    This function must be invoked only after the system reset.
 *
 * @special
 */
void spc_clock_init(void) {

  /* Waiting for IRC stabilization before attempting anything else.*/
  while (!MC_ME.GS.B.S_IRC)
    ;

#if !SPC5_NO_INIT

#if SPC5_DISABLE_WATCHDOG
  /* SWTs disabled.*/
  SWT_2.SR.R        = 0xC520;
  SWT_2.SR.R        = 0xD928;
  SWT_2.CR.R        = 0xFF000002;
#endif

  /* SSCM initialization from configuration data.*/
  SSCM.ERROR.R      = SPC5_SSCM_ERROR_INIT;

  /* RGM errors clearing.*/
  RGM.FES.R      = 0xFFFF;
  RGM.DES.R      = 0xFFFF;

  /* The system must be in DRUN mode on entry, if this is not the case then
     it is considered a serious anomaly.*/
  if (MC_ME.GS.B.S_CURRENT_MODE != SPC5_RUNMODE_DRUN) {
    SPC5_CLOCK_FAILURE_HOOK();
  }

#if defined(SPC5_OSC_BYPASS)
  /* If the board is equipped with an oscillator instead of a crystal then the
     bypass must be activated.*/
  XOSC.CTL.B.OSCBYP = TRUE;
#endif /* SPC5_OSC_BYPASS */

  /* Setting the system dividers to their final values.*/
  MC_CGM.SC_DC0.R   = SPC5_CGM_SC_DC0_BITS;
  MC_CGM.SC_DC1.R   = SPC5_CGM_SC_DC1_BITS;
  MC_CGM.SC_DC2.R   = SPC5_CGM_SC_DC2_BITS;
  
  /* Setting the auxiliary dividers to their final values.*/
  MC_CGM.AC0_DC0.R  = SPC5_CGM_AC0_DC0_BITS;
  MC_CGM.AC0_DC1.R  = SPC5_CGM_AC0_DC1_BITS;
  MC_CGM.AC0_DC2.R  = SPC5_CGM_AC0_DC2_BITS;
  MC_CGM.AC0_DC3.R  = SPC5_CGM_AC0_DC3_BITS;
  MC_CGM.AC0_DC4.R  = SPC5_CGM_AC0_DC4_BITS;
  MC_CGM.AC0_DC5.R  = SPC5_CGM_AC0_DC5_BITS;
  MC_CGM.AC1_DC0.R  = SPC5_CGM_AC1_DC0_BITS;
  
  /* Setting the clock selectors to their final sources.*/
  MC_CGM.AC0_SC.R   = SPC5_CGM_AC0_SC_BITS;
  MC_CGM.AC1_SC.R   = SPC5_CGM_AC1_SC_BITS;
  MC_CGM.AC2_SC.R   = SPC5_CGM_AC1_SC_BITS;
  MC_CGM.AC3_SC.R   = SPC5_CGM_AC3_SC_BITS;
  
  /* Enables the XOSC in order to check its functionality before proceeding
     with the initialization.*/
  MC_ME.DRUN_MC.R   = SPC5_ME_MC_SYSCLK_IRC | SPC5_ME_MC_IRCON |
                      SPC5_ME_MC_XOSC0ON | SPC5_ME_MC_FLAON_NORMAL |
                      SPC5_ME_MC_MVRON;
  if (halSPCSetRunMode(SPC5_RUNMODE_DRUN) == OSAL_FAILED) {
    SPC5_CLOCK_FAILURE_HOOK();
  }

  /* PLLs initialization, the changes will have effect on mode switch.*/
  PLLDIG.PLL0CR.R   = 0;
  PLLDIG.PLL0DV.R   = SPC5_PLL0_DV_RFDPHI1(SPC5_PLL0_RFDPHI1_VALUE) |
                      SPC5_PLL0_DV_RFDPHI(SPC5_PLL0_RFDPHI_VALUE) |
                      SPC5_PLL0_DV_PREDIV(SPC5_PLL0_PREDIV_VALUE) |
                      SPC5_PLL0_DV_MFD(SPC5_PLL0_MFD_VALUE);
  PLLDIG.PLL1CR.R   = 0;
  PLLDIG.PLL1DV.R   = SPC5_PLL1_DV_RFDPHI(SPC5_PLL1_RFDPHI_VALUE) |
                      SPC5_PLL1_DV_MFD(SPC5_PLL1_MFD_VALUE);

  /* Run modes initialization, note writes to the MC registers are verified
     by a protection mechanism, the operation success is verified at the
     end of the sequence.*/
  MC_ME.IS.R        = 8;                        /* Resetting I_ICONF status.*/
  MC_ME.ME.R        = SPC5_ME_ME_BITS;
  MC_ME.SAFE_MC.R   = SPC5_ME_SAFE_MC_BITS;
  MC_ME.DRUN_MC.R   = SPC5_ME_DRUN_MC_BITS;
  MC_ME.RUN_MC[0].R = SPC5_ME_RUN0_MC_BITS;
  MC_ME.RUN_MC[1].R = SPC5_ME_RUN1_MC_BITS;
  MC_ME.RUN_MC[2].R = SPC5_ME_RUN2_MC_BITS;
  MC_ME.RUN_MC[3].R = SPC5_ME_RUN3_MC_BITS;
  MC_ME.HALT_MC.R   = SPC5_ME_HALT0_MC_BITS;
  MC_ME.STOP_MC.R   = SPC5_ME_STOP0_MC_BITS;

  if (MC_ME.IS.B.I_ICONF) {
    /* Configuration rejected.*/
    SPC5_CLOCK_FAILURE_HOOK();
  }

  /* Peripherals run and low power modes initialization.*/
  MC_ME.RUN_PC[0].R = SPC5_ME_RUN_PC0_BITS;
  MC_ME.RUN_PC[1].R = SPC5_ME_RUN_PC1_BITS;
  MC_ME.RUN_PC[2].R = SPC5_ME_RUN_PC2_BITS;
  MC_ME.RUN_PC[3].R = SPC5_ME_RUN_PC3_BITS;
  MC_ME.RUN_PC[4].R = SPC5_ME_RUN_PC4_BITS;
  MC_ME.RUN_PC[5].R = SPC5_ME_RUN_PC5_BITS;
  MC_ME.RUN_PC[6].R = SPC5_ME_RUN_PC6_BITS;
  MC_ME.RUN_PC[7].R = SPC5_ME_RUN_PC7_BITS;
  MC_ME.LP_PC[0].R  = SPC5_ME_LP_PC0_BITS;
  MC_ME.LP_PC[1].R  = SPC5_ME_LP_PC1_BITS;
  MC_ME.LP_PC[2].R  = SPC5_ME_LP_PC2_BITS;
  MC_ME.LP_PC[3].R  = SPC5_ME_LP_PC3_BITS;
  MC_ME.LP_PC[4].R  = SPC5_ME_LP_PC4_BITS;
  MC_ME.LP_PC[5].R  = SPC5_ME_LP_PC5_BITS;
  MC_ME.LP_PC[6].R  = SPC5_ME_LP_PC6_BITS;
  MC_ME.LP_PC[7].R  = SPC5_ME_LP_PC7_BITS;

  /* Switches again to DRUN mode (current mode) in order to update the
     settings.*/
  if (halSPCSetRunMode(SPC5_RUNMODE_DRUN) == OSAL_FAILED) {
    SPC5_CLOCK_FAILURE_HOOK();
  }

#endif /* !SPC5_NO_INIT */
}

/**
 * @brief   Switches the system to the specified run mode.
 *
 * @param[in] mode      one of the possible run modes
 *
 * @return              The operation status.
 * @retval OSAL_SUCCESS   if the switch operation has been completed.
 * @retval OSAL_FAILED    if the switch operation failed.
 */
bool halSPCSetRunMode(spc5_runmode_t mode) {

  /* Clearing status register bits I_IMODE(4) and I_IMTC(1).*/
  MC_ME.IS.R = 5;

  /* Starts a transition process.*/
  MC_ME.MCTL.R = SPC5_ME_MCTL_MODE(mode) | SPC5_ME_MCTL_KEY;
  MC_ME.MCTL.R = SPC5_ME_MCTL_MODE(mode) | SPC5_ME_MCTL_KEY_INV;

  /* Waits for the mode switch or an error condition.*/
  while (TRUE) {
    uint32_t r = MC_ME.IS.R;
    if (r & 1)
      return OSAL_SUCCESS;
    if (r & 4)
      return OSAL_FAILED;
  }
}

/**
 * @brief   Changes the clock mode of a peripheral.
 *
 * @param[in] n         index of the @p PCTL register
 * @param[in] pctl      new value for the @p PCTL register
 *
 * @notapi
 */
void halSPCSetPeripheralClockMode(uint32_t n, uint32_t pctl) {
  uint32_t mode;

  MC_ME.PCTL[n].R = pctl;
  mode = MC_ME.MCTL.B.TARGET_MODE;
  MC_ME.MCTL.R = SPC5_ME_MCTL_MODE(mode) | SPC5_ME_MCTL_KEY;
  MC_ME.MCTL.R = SPC5_ME_MCTL_MODE(mode) | SPC5_ME_MCTL_KEY_INV;
}

#if !SPC5_NO_INIT || defined(__DOXYGEN__)
/**
 * @brief   Returns the system clock under the current run mode.
 *
 * @return              The system clock in Hertz.
 */
uint32_t halSPCGetSystemClock(void) {
  uint32_t sysclk;

  sysclk = MC_ME.GS.B.S_SYSCLK;
  switch (sysclk) {
  case SPC5_ME_GS_SYSCLK_IRC:
    return SPC5_IRC_CLK / MC_CGM.SC_DC0.B.DIV;
  case SPC5_ME_GS_SYSCLK_XOSC:
    return SPC5_XOSC_CLK / MC_CGM.SC_DC0.B.DIV;
  case SPC5_ME_GS_SYSCLK_PLL0PHI:
    return SPC5_PLL0_PHI_CLK / MC_CGM.SC_DC0.B.DIV;
  case SPC5_ME_GS_SYSCLK_PLL1PHI:
    return SPC5_PLL1_PHI_CLK / MC_CGM.SC_DC0.B.DIV;
  default:
    return 0;
  }
}
#endif /* !SPC5_NO_INIT */

/** @} */
