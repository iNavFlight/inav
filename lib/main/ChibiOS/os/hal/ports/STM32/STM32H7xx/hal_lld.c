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
 * @file    STM32H7xx/hal_lld.c
 * @brief   STM32H7xx HAL subsystem low level driver source.
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

/**
 * @brief   CMSIS system core clock variable.
 * @note    It is declared in system_stm32f7xx.h.
 */
// uint32_t SystemCoreClock = STM32_CORE_CK;

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Initializes the backup domain.
 * @note    WARNING! Changing clock source impossible without resetting
 *          of the whole BKP domain.
 */
static inline void init_bkp_domain(void) {

  /* Backup domain access enabled and left open.*/
  PWR->CR1 |= PWR_CR1_DBP;

  /* Reset BKP domain if different clock source selected.*/
  if ((RCC->BDCR & STM32_RTCSEL_MASK) != STM32_RTCSEL) {
    /* Backup domain reset.*/
    RCC->BDCR = RCC_BDCR_BDRST;
    RCC->BDCR = 0;
  }

#if STM32_LSE_ENABLED
#if defined(STM32_LSE_BYPASS)
  /* LSE Bypass.*/
  RCC->BDCR |= STM32_LSEDRV | RCC_BDCR_LSEON | RCC_BDCR_LSEBYP;
#else
  /* No LSE Bypass.*/
  RCC->BDCR |= STM32_LSEDRV | RCC_BDCR_LSEON;
#endif
  while ((RCC->BDCR & RCC_BDCR_LSERDY) == 0)
    ;                                       /* Waits until LSE is stable.   */
#endif

#if HAL_USE_RTC
  /* If the backup domain hasn't been initialized yet then proceed with
     initialization.*/
  if ((RCC->BDCR & RCC_BDCR_RTCEN) == 0) {
    /* Selects clock source.*/
    RCC->BDCR |= STM32_RTCSEL;

    /* RTC clock enabled.*/
    RCC->BDCR |= RCC_BDCR_RTCEN;
  }
#endif /* HAL_USE_RTC */
}

/**
 * @brief   Initializes the PWR unit.
 */
static inline void init_pwr(void) {
#if 0
  PWR_TypeDef *pwr = PWR; /* For inspection.*/
  (void)pwr;
#endif

  PWR->CR1   = STM32_PWR_CR1 | 0xF0000000;
  PWR->CR2   = STM32_PWR_CR2;
  PWR->CR3   = STM32_PWR_CR3;
  PWR->CPUCR = STM32_PWR_CPUCR;
  PWR->D3CR  = STM32_VOS;
  while ((PWR->D3CR & PWR_D3CR_VOSRDY) == 0)
    ;
#if STM32_PWR_CR2 & PWR_CR2_BREN
//  while ((PWR->CR2 & PWR_CR2_BRRDY) == 0)
//    ;
//  rccEnableBKPRAM(true);
#endif
}

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

#if STM32_NO_INIT == FALSE
  /* Reset of all peripherals. AHB3 is not reset entirely because FMC could
     have been initialized in the board initialization file (board.c).
     Note, GPIOs are not reset because initialized before this point in
     board files.*/
  rccResetAHB1(~0);
  rccResetAHB2(~0);
  rccResetAHB3(~(RCC_AHB3RSTR_CPURST | RCC_AHB3RSTR_FMCRST));
  rccResetAHB4(~(STM32_GPIO_EN_MASK));
  rccResetAPB1L(~0);
  rccResetAPB1H(~0);
  rccResetAPB2(~0);
  rccResetAPB3(~0);
  rccResetAPB4(~0);

  /* DMA subsystems initialization.*/
#if defined(STM32_BDMA_REQUIRED)
  bdmaInit();
#endif
#if defined(STM32_DMA_REQUIRED)
  dmaInit();
#endif

  /* IRQ subsystem initialization.*/
  irqInit();

  /* MPU initialization.*/
#if (STM32_NOCACHE_SRAM1_SRAM2 == TRUE) || (STM32_NOCACHE_SRAM3 == TRUE)
  {
    uint32_t base, size;

#if (STM32_NOCACHE_SRAM1_SRAM2 == TRUE) && (STM32_NOCACHE_SRAM3 == TRUE)
    base = 0x30000000U;
    size = MPU_RASR_SIZE_512K;
#elif (STM32_NOCACHE_SRAM1_SRAM2 == TRUE) && (STM32_NOCACHE_SRAM3 == FALSE)
    base = 0x30000000U;
    size = MPU_RASR_SIZE_256K;
#elif (STM32_NOCACHE_SRAM1_SRAM2 == FALSE) && (STM32_NOCACHE_SRAM3 == TRUE)
    base = 0x30040000U;
    size = MPU_RASR_SIZE_16K;
#else
#error "invalid constants used in mcuconf.h"
#endif

    /* The SRAM2 bank can optionally made a non cache-able area for use by
       DMA engines.*/
    mpuConfigureRegion(MPU_REGION_7,
                       base,
                       MPU_RASR_ATTR_AP_RW_RW |
                       MPU_RASR_ATTR_NON_CACHEABLE |
                       size |
                       MPU_RASR_ENABLE);
    mpuEnable(MPU_CTRL_PRIVDEFENA);

    /* Invalidating data cache to make sure that the MPU settings are taken
       immediately.*/
    SCB_CleanInvalidateDCache();
  }
#endif
#endif /* STM32_NO_INIT == FALSE */
}

/**
 * @brief   STM32H7xx clocks and PLL initialization.
 * @note    All the involved constants come from the file @p board.h.
 * @note    This function should be invoked just after the system reset.
 *
 * @special
 */
void stm32_clock_init(void) {
  uint32_t cfgr;

#if 0
  RCC_TypeDef *rcc = RCC; /* For inspection.*/
  (void)rcc;
#endif

#if STM32_NO_INIT == FALSE
#if !defined(STM32_DISABLE_ERRATA_2_2_15)
  /* Fix for errata 2.2.15: Reading from AXI SRAM might lead to data
     read corruption.
     AXI->TARG7_FN_MOD.*/
  *((volatile uint32_t *)(0x51000000 + 0x1108 + 0x7000)) = 0x00000001U;
#endif

  /* PWR initialization.*/
  init_pwr();

  /* Backup domain initialization.*/
  init_bkp_domain();

  /* HSI setup, it enforces the reset situation in order to handle possible
     problems with JTAG probes and re-initializations.*/
  RCC->CR |= RCC_CR_HSION;                  /* Make sure HSI is ON.         */
  while (!(RCC->CR & RCC_CR_HSIRDY))
    ;                                       /* Wait until HSI is stable.    */

  /* HSI is selected as new source without touching the other fields in
     CFGR. This is only required when using a debugger than can cause
     restarts.*/
  RCC->CFGR    = 0x00000000U;               /* Reset SW to HSI.             */
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI)
    ;                                       /* Wait until HSI is selected.  */

  /* Registers cleared to reset values.*/
  RCC->CR      = RCC_CR_HSION;             /* CR Reset value.              */
  RCC->ICSCR   = 0x40000000U;              /* ICSCR Reset value.           */
  RCC->CSR     = 0x00000000U;              /* CSR reset value.             */
  RCC->PLLCFGR = 0x01FF0000U;              /* PLLCFGR reset value.         */

  /* Other clock-related settings, done before other things because
     recommended in the RM.*/
  cfgr = STM32_MCO2SEL | RCC_CFGR_MCO2PRE_VALUE(STM32_MCO2PRE_VALUE) |
         STM32_MCO1SEL | RCC_CFGR_MCO1PRE_VALUE(STM32_MCO1PRE_VALUE) |
         RCC_CFGR_RTCPRE_VALUE(STM32_RTCPRE_VALUE) |
         STM32_HRTIMSEL | STM32_STOPKERWUCK | STM32_STOPWUCK;
#if STM32_TIMPRE_ENABLE == TRUE
  cfgr |= RCC_CFGR_TIMPRE;
#endif
  RCC->CFGR = cfgr;

  /* HSE activation with optional bypass.*/
#if STM32_HSE_ENABLED == TRUE
#if defined(STM32_HSE_BYPASS)
  RCC->CR |= RCC_CR_HSEON | RCC_CR_HSEBYP;
#else
  RCC->CR |= RCC_CR_HSEON;
#endif
  while ((RCC->CR & RCC_CR_HSERDY) == 0)
    ;                           /* Waits until HSE is stable.               */
#endif

  /* CSI activation.*/
#if STM32_CSI_ENABLED == TRUE
  RCC->CR |= RCC_CR_CSION;
  while ((RCC->CR & RCC_CR_CSIRDY) == 0)
    ;                           /* Waits until CSI is stable.               */
#endif /* STM32_HSE_ENABLED == TRUE */

  /* LSI activation.*/
#if STM32_LSI_ENABLED == TRUE
  RCC->CSR |= RCC_CSR_LSION;
  while ((RCC->CSR & RCC_CSR_LSIRDY) == 0)
    ;                           /* Waits until LSI is stable.               */
#endif /* STM32_LSI_ENABLED == TRUE */

  /* PLLs activation, it happens in parallel in order to
     reduce boot time.*/
#if (STM32_PLL1_ENABLED == TRUE) ||                                         \
    (STM32_PLL2_ENABLED == TRUE) ||                                         \
    (STM32_PLL3_ENABLED == TRUE)
  {
    uint32_t onmask = 0;
    uint32_t rdymask = 0;
    uint32_t cfgmask = 0;

    RCC->PLLCKSELR = RCC_PLLCKSELR_DIVM3_VALUE(STM32_PLL3_DIVM_VALUE) |
                     RCC_PLLCKSELR_DIVM2_VALUE(STM32_PLL2_DIVM_VALUE) |
                     RCC_PLLCKSELR_DIVM1_VALUE(STM32_PLL1_DIVM_VALUE) |
                     RCC_PLLCKSELR_PLLSRC_VALUE(STM32_PLLSRC);

    cfgmask = STM32_PLLCFGR_PLL3RGE | STM32_PLLCFGR_PLL3VCOSEL | RCC_PLLCFGR_PLL3FRACEN |
              STM32_PLLCFGR_PLL2RGE | STM32_PLLCFGR_PLL2VCOSEL | RCC_PLLCFGR_PLL2FRACEN |
              STM32_PLLCFGR_PLL1RGE | STM32_PLLCFGR_PLL1VCOSEL | RCC_PLLCFGR_PLL1FRACEN;

#if STM32_PLL1_ENABLED == TRUE
    RCC->PLL1FRACR = STM32_PLL1_FRACN;
    RCC->PLL1DIVR  = STM32_PLL1_DIVR | STM32_PLL1_DIVQ |
                     STM32_PLL1_DIVP | STM32_PLL1_DIVN;
    onmask  |= RCC_CR_PLL1ON;
    rdymask |= RCC_CR_PLL1RDY;
#if STM32_PLL1_P_ENABLED == TRUE
    cfgmask |= RCC_PLLCFGR_DIVP1EN;
#endif
#if STM32_PLL1_Q_ENABLED == TRUE
    cfgmask |= RCC_PLLCFGR_DIVQ1EN;
#endif
#if STM32_PLL1_R_ENABLED == TRUE
    cfgmask |= RCC_PLLCFGR_DIVR1EN;
#endif
#endif /* STM32_PLL1_ENABLED == TRUE */

#if STM32_PLL2_ENABLED == TRUE
    RCC->PLL2FRACR = STM32_PLL2_FRACN;
    RCC->PLL2DIVR  = STM32_PLL2_DIVR | STM32_PLL2_DIVQ |
                     STM32_PLL2_DIVP | STM32_PLL2_DIVN;
    onmask  |= RCC_CR_PLL2ON;
    rdymask |= RCC_CR_PLL2RDY;
#if STM32_PLL2_P_ENABLED == TRUE
    cfgmask |= RCC_PLLCFGR_DIVP2EN;
#endif
#if STM32_PLL2_Q_ENABLED == TRUE
    cfgmask |= RCC_PLLCFGR_DIVQ2EN;
#endif
#if STM32_PLL2_R_ENABLED == TRUE
    cfgmask |= RCC_PLLCFGR_DIVR2EN;
#endif
#endif /* STM32_PLL2_ENABLED == TRUE */

#if STM32_PLL3_ENABLED == TRUE
    RCC->PLL3FRACR = STM32_PLL3_FRACN;
    RCC->PLL3DIVR  = STM32_PLL3_DIVR | STM32_PLL3_DIVQ |
                     STM32_PLL3_DIVP | STM32_PLL3_DIVN;
    onmask  |= RCC_CR_PLL3ON;
    rdymask |= RCC_CR_PLL3RDY;
#if STM32_PLL3_P_ENABLED == TRUE
    cfgmask |= RCC_PLLCFGR_DIVP3EN;
#endif
#if STM32_PLL3_Q_ENABLED == TRUE
    cfgmask |= RCC_PLLCFGR_DIVQ3EN;
#endif
#if STM32_PLL3_R_ENABLED == TRUE
    cfgmask |= RCC_PLLCFGR_DIVR3EN;
#endif
#endif /* STM32_PLL3_ENABLED == TRUE */

    /* Activating enabled PLLs and waiting for all of them to become ready.*/
    RCC->PLLCFGR = cfgmask & STM32_PLLCFGR_MASK;
    RCC->CR     |= onmask;
    while ((RCC->CR & rdymask) != rdymask)
      ;
  }
#endif /* STM32_PLL1_ENABLED || STM32_PLL2_ENABLED || STM32_PLL3_ENABLED */

  /* AHB and APB dividers.*/
  RCC->D1CFGR = STM32_D1CPRE  | STM32_D1PPRE3 | STM32_D1HPRE;
  RCC->D2CFGR = STM32_D2PPRE2 | STM32_D2PPRE1;
  RCC->D3CFGR = STM32_D3PPRE4;

  /* Peripherals clocks.*/
  RCC->D1CCIPR  = STM32_CKPERSEL  | STM32_SDMMCSEL    | STM32_QSPISEL    |
                  STM32_FMCSEL;
  RCC->D2CCIP1R = STM32_SWPSEL    | STM32_FDCANSEL    | STM32_DFSDM1SEL  |
                  STM32_SPDIFSEL  | STM32_SPDIFSEL    | STM32_SPI45SEL   |
                  STM32_SPI123SEL | STM32_SAI23SEL    | STM32_SAI1SEL;
  RCC->D2CCIP2R = STM32_LPTIM1SEL | STM32_CECSEL      | STM32_USBSEL     |
                  STM32_I2C123SEL | STM32_RNGSEL      | STM32_USART16SEL |
                  STM32_USART234578SEL;
  RCC->D3CCIPR  = STM32_SPI6SEL   | STM32_SAI4BSEL    | STM32_SAI4ASEL   |
                  STM32_ADCSEL    | STM32_LPTIM345SEL | STM32_LPTIM2SEL  |
                  STM32_I2C4SEL   | STM32_LPUART1SEL;

  /* Flash setup.*/
  FLASH->ACR = FLASH_ACR_WRHIGHFREQ_2 | STM32_FLASHBITS;
  while ((FLASH->ACR & FLASH_ACR_LATENCY) !=
         (STM32_FLASHBITS & FLASH_ACR_LATENCY)) {
  }

  /* Switching to the configured clock source if it is different
     from HSI.*/
#if STM32_SW != STM32_SW_HSI_CK
  RCC->CFGR |= STM32_SW;        /* Switches on the selected clock source.   */
  while ((RCC->CFGR & RCC_CFGR_SWS) != (STM32_SW << 3U))
    ;
#endif

#if 0
  /* Peripheral clock sources.*/
  RCC->DCKCFGR2 = STM32_SDMMCSEL  | STM32_CK48MSEL  | STM32_CECSEL    |
                  STM32_LPTIM1SEL | STM32_I2C4SEL   | STM32_I2C3SEL   |
                  STM32_I2C2SEL   | STM32_I2C1SEL   | STM32_UART8SEL  |
                  STM32_UART7SEL  | STM32_USART6SEL | STM32_UART5SEL  |
                  STM32_UART4SEL  | STM32_USART3SEL | STM32_USART2SEL |
                  STM32_USART1SEL;
#endif
  /* RAM1 2 and 3 clocks enabled.*/
  rccEnableSRAM1(true);
  rccEnableSRAM2(true);
  rccEnableSRAM3(true);

  /* SYSCFG clock enabled here because it is a multi-functional unit shared
     among multiple drivers.*/
  rccEnableAPB4(RCC_APB4ENR_SYSCFGEN, true);
#else
  (void)cfgr;
#endif /* STM32_NO_INIT */
}

/** @} */
