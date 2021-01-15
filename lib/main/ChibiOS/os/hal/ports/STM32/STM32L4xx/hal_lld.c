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
 * @file    STM32L4xx/hal_lld.c
 * @brief   STM32L4xx HAL subsystem low level driver source.
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
uint32_t SystemCoreClock = STM32_HCLK;

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Initializes the backup domain.
 * @note    WARNING! Changing RTC clock source impossible without resetting
 *          of the whole BKP domain.
 */
static void hal_lld_backup_domain_init(void) {

  /* Reset BKP domain if different clock source selected.*/
  if ((RCC->BDCR & STM32_RTCSEL_MASK) != STM32_RTCSEL) {
    /* Backup domain reset.*/
    RCC->BDCR = RCC_BDCR_BDRST;
    RCC->BDCR = 0;
  }

#if STM32_LSE_ENABLED
  /* LSE activation.*/
#if defined(STM32_LSE_BYPASS)
  /* LSE Bypass.*/
  RCC->BDCR |= STM32_LSEDRV | RCC_BDCR_LSEON | RCC_BDCR_LSEBYP;
#else
  /* No LSE Bypass.*/
  RCC->BDCR |= STM32_LSEDRV | RCC_BDCR_LSEON;
#endif
  while ((RCC->BDCR & RCC_BDCR_LSERDY) == 0)
    ;                                       /* Wait until LSE is stable.    */
#endif

#if STM32_MSIPLL_ENABLED
  /* MSI PLL activation depends on LSE. Reactivating and checking for
     MSI stability.*/
  RCC->CR |= RCC_CR_MSIPLLEN;
  while ((RCC->CR & RCC_CR_MSIRDY) == 0)
    ;                                       /* Wait until MSI is stable.    */
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

  /* Low speed output mode.*/
  RCC->BDCR |= STM32_LSCOSEL;
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

  /* Reset of all peripherals.
     Note, GPIOs are not reset because initialized before this point in
     board files.*/
  rccResetAHB1(~0);
  rccResetAHB2(~STM32_GPIO_EN_MASK);
  rccResetAHB3(~0);
  rccResetAPB1R1(~RCC_APB1RSTR1_PWRRST);
  rccResetAPB1R2(~0);
  rccResetAPB2(~0);

  /* PWR clock enabled.*/
  rccEnablePWRInterface(true);

  /* Initializes the backup domain.*/
  hal_lld_backup_domain_init();

  /* DMA subsystems initialization.*/
#if defined(STM32_DMA_REQUIRED)
  dmaInit();
#endif

  /* IRQ subsystem initialization.*/
  irqInit();

  /* Programmable voltage detector enable.*/
#if STM32_PVD_ENABLE
  PWR->CR2 = PWR_CR2_PVDE | (STM32_PLS & STM32_PLS_MASK);
#else
  PWR->CR2 = 0;
#endif /* STM32_PVD_ENABLE */

  /* Enabling independent VDDUSB.*/
#if HAL_USE_USB
  PWR->CR2 |= PWR_CR2_USV;
#endif /* HAL_USE_USB */

  /* Enabling independent VDDIO2 required by GPIOG.*/
#if STM32_HAS_GPIOG
  PWR->CR2 |= PWR_CR2_IOSV;
#endif /* STM32_HAS_GPIOG */
}

/**
 * @brief   STM32L4xx clocks and PLL initialization.
 * @note    All the involved constants come from the file @p board.h.
 * @note    This function should be invoked just after the system reset.
 *
 * @special
 */
void stm32_clock_init(void) {

#if !STM32_NO_INIT
  /* PWR clock enable.*/
#if defined(HAL_USE_RTC) && defined(RCC_APB1ENR1_RTCAPBEN)
  RCC->APB1ENR1 = RCC_APB1ENR1_PWREN | RCC_APB1ENR1_RTCAPBEN;
#else
  RCC->APB1ENR1 = RCC_APB1ENR1_PWREN;
#endif

  /* Initial clocks setup and wait for MSI stabilization, the MSI clock is
     always enabled because it is the fall back clock when PLL the fails.
     Trim fields are not altered from reset values.*/

  /* MSIRANGE can be set only when MSI is OFF or READY.*/
  RCC->CR = RCC_CR_MSION;
  while ((RCC->CR & RCC_CR_MSIRDY) == 0)
    ;                                       /* Wait until MSI is stable.    */

  /* Clocking from MSI, in case MSI was not the default source.*/
  RCC->CFGR = 0;
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_MSI)
    ;                                       /* Wait until MSI is selected.  */

  /* Core voltage setup.*/
  PWR->CR1 = STM32_VOS;
  while ((PWR->SR2 & PWR_SR2_VOSF) != 0)    /* Wait until regulator is      */
    ;                                       /* stable.                      */

#if STM32_HSI16_ENABLED
  /* HSI activation.*/
  RCC->CR |= RCC_CR_HSION;
  while ((RCC->CR & RCC_CR_HSIRDY) == 0)
    ;                                       /* Wait until HSI16 is stable.  */
#endif

#if STM32_CLOCK_HAS_HSI48
#if STM32_HSI48_ENABLED
  /* HSI activation.*/
  RCC->CRRCR |= RCC_CRRCR_HSI48ON;
  while ((RCC->CRRCR & RCC_CRRCR_HSI48RDY) == 0)
    ;                                       /* Wait until HSI48 is stable.  */
#endif
#endif

#if STM32_HSE_ENABLED
#if defined(STM32_HSE_BYPASS)
  /* HSE Bypass.*/
  RCC->CR |= RCC_CR_HSEON | RCC_CR_HSEBYP;
#endif
  /* HSE activation.*/
  RCC->CR |= RCC_CR_HSEON;
  while ((RCC->CR & RCC_CR_HSERDY) == 0)
    ;                                       /* Wait until HSE is stable.    */
#endif

#if STM32_LSI_ENABLED
  /* LSI activation.*/
  RCC->CSR |= RCC_CSR_LSION;
  while ((RCC->CSR & RCC_CSR_LSIRDY) == 0)
    ;                                       /* Wait until LSI is stable.    */
#endif

  /* Backup domain access enabled and left open.*/
  PWR->CR1 |= PWR_CR1_DBP;

#if STM32_LSE_ENABLED
  /* LSE activation.*/
#if defined(STM32_LSE_BYPASS)
  /* LSE Bypass.*/
  RCC->BDCR |= STM32_LSEDRV | RCC_BDCR_LSEON | RCC_BDCR_LSEBYP;
#else
  /* No LSE Bypass.*/
  RCC->BDCR |= STM32_LSEDRV | RCC_BDCR_LSEON;
#endif
  while ((RCC->BDCR & RCC_BDCR_LSERDY) == 0)
    ;                                       /* Wait until LSE is stable.    */
#endif

  /* Flash setup for selected MSI speed setting.*/
  FLASH->ACR = FLASH_ACR_DCEN | FLASH_ACR_ICEN | FLASH_ACR_PRFTEN |
               STM32_MSI_FLASHBITS;

  /* Changing MSIRANGE to configured value.*/
  RCC->CR |= STM32_MSIRANGE;

  /* Switching from MSISRANGE to MSIRANGE.*/
  RCC->CR |= RCC_CR_MSIRGSEL;
  while ((RCC->CR & RCC_CR_MSIRDY) == 0)
    ;

  /* MSI is configured SYSCLK source so wait for it to be stable as well.*/
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_MSI)
    ;

#if STM32_MSIPLL_ENABLED
  /* MSI PLL (to LSE) activation */
  RCC->CR |= RCC_CR_MSIPLLEN;
#endif

  /* Updating MSISRANGE value. MSISRANGE can be set only when MSIRGSEL is high.
     This range is used exiting the Standby mode until MSIRGSEL is set.*/
  RCC->CSR |= STM32_MSISRANGE;

#if STM32_ACTIVATE_PLL || STM32_ACTIVATE_PLLSAI1 || STM32_ACTIVATE_PLLSAI2
  /* PLLM and PLLSRC are common to all PLLs.*/
#if defined(STM32L496xx) || defined(STM32L4A6xx)
  RCC->PLLCFGR = STM32_PLLPDIV | STM32_PLLR  |
                 STM32_PLLREN  | STM32_PLLQ  |
                 STM32_PLLQEN  | STM32_PLLP  |
                 STM32_PLLPEN  | STM32_PLLN  |
                 STM32_PLLM    | STM32_PLLSRC;
#else
  RCC->PLLCFGR = STM32_PLLR   | STM32_PLLREN |
                 STM32_PLLQ   | STM32_PLLQEN |
                 STM32_PLLP   | STM32_PLLPEN |
                 STM32_PLLN   | STM32_PLLM   |
                 STM32_PLLSRC;
#endif
#endif

#if STM32_ACTIVATE_PLL
  /* PLL activation.*/
  RCC->CR |= RCC_CR_PLLON;

  /* Waiting for PLL lock.*/
  while ((RCC->CR & RCC_CR_PLLRDY) == 0)
    ;
#endif

#if STM32_ACTIVATE_PLLSAI1
  /* PLLSAI1 activation.*/
#if defined(STM32L496xx) || defined(STM32L4A6xx)
  RCC->PLLSAI1CFGR = STM32_PLLSAI1PDIV | STM32_PLLSAI1R |
                     STM32_PLLSAI1REN  | STM32_PLLSAI1Q |
                     STM32_PLLSAI1QEN  | STM32_PLLSAI1P |
                     STM32_PLLSAI1PEN  | STM32_PLLSAI1N;
#else
  RCC->PLLSAI1CFGR = STM32_PLLSAI1R | STM32_PLLSAI1REN |
                     STM32_PLLSAI1Q | STM32_PLLSAI1QEN |
                     STM32_PLLSAI1P | STM32_PLLSAI1PEN |
                     STM32_PLLSAI1N;
#endif
  RCC->CR |= RCC_CR_PLLSAI1ON;

  /* Waiting for PLL lock.*/
  while ((RCC->CR & RCC_CR_PLLSAI1RDY) == 0)
    ;
#endif

#if STM32_ACTIVATE_PLLSAI2
  /* PLLSAI2 activation.*/
#if defined(STM32L496xx) || defined(STM32L4A6xx)
  RCC->PLLSAI2CFGR = STM32_PLLSAI2PDIV | STM32_PLLSAI2R |
                     STM32_PLLSAI2REN  | STM32_PLLSAI2P |
                     STM32_PLLSAI2PEN  | STM32_PLLSAI2N;
#else
  RCC->PLLSAI2CFGR = STM32_PLLSAI2R | STM32_PLLSAI2REN |
                     STM32_PLLSAI2P | STM32_PLLSAI2PEN |
                     STM32_PLLSAI2N;
#endif
  RCC->CR |= RCC_CR_PLLSAI2ON;

  /* Waiting for PLL lock.*/
  while ((RCC->CR & RCC_CR_PLLSAI2RDY) == 0)
    ;
#endif

  /* Other clock-related settings (dividers, MCO etc).*/
  RCC->CFGR = STM32_MCOPRE | STM32_MCOSEL | STM32_STOPWUCK |
              STM32_PPRE2  | STM32_PPRE1  | STM32_HPRE;

  /* CCIPR register initialization, note, must take care of the _OFF
     pseudo settings.*/
  {
    uint32_t ccipr = STM32_DFSDMSEL  | STM32_SWPMI1SEL | STM32_ADCSEL    |
                     STM32_CLK48SEL  | STM32_LPTIM2SEL | STM32_LPTIM1SEL |
                     STM32_I2C3SEL   | STM32_I2C2SEL   | STM32_I2C1SEL   |
                     STM32_UART5SEL  | STM32_UART4SEL  | STM32_USART3SEL |
                     STM32_USART2SEL | STM32_USART1SEL | STM32_LPUART1SEL;
#if STM32_SAI2SEL != STM32_SAI2SEL_OFF
    ccipr |= STM32_SAI2SEL;
#endif
#if STM32_SAI1SEL != STM32_SAI1SEL_OFF
    ccipr |= STM32_SAI1SEL;
#endif
    RCC->CCIPR = ccipr;
  }

  /* Set flash WS's for SYSCLK source */
  if (STM32_FLASHBITS > STM32_MSI_FLASHBITS) {
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY_Msk) | STM32_FLASHBITS;
    while ((FLASH->ACR & FLASH_ACR_LATENCY_Msk) !=
           (STM32_FLASHBITS & FLASH_ACR_LATENCY_Msk)) {
    }
  }

  /* Switching to the configured SYSCLK source if it is different from MSI.*/
#if (STM32_SW != STM32_SW_MSI)
  RCC->CFGR |= STM32_SW;        /* Switches on the selected clock source.   */
  /* Wait until SYSCLK is stable.*/
  while ((RCC->CFGR & RCC_CFGR_SWS) != (STM32_SW << 2))
    ;
#endif

  /* Reduce the flash WS's for SYSCLK source if they are less than MSI WSs */
  if (STM32_FLASHBITS < STM32_MSI_FLASHBITS) {
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY_Msk) | STM32_FLASHBITS;
    while ((FLASH->ACR & FLASH_ACR_LATENCY_Msk) !=
           (STM32_FLASHBITS & FLASH_ACR_LATENCY_Msk)) {
    }
  }

#endif /* STM32_NO_INIT */

  /* SYSCFG clock enabled here because it is a multi-functional unit shared
     among multiple drivers.*/
  rccEnableAPB2(RCC_APB2ENR_SYSCFGEN, true);
}

/** @} */
