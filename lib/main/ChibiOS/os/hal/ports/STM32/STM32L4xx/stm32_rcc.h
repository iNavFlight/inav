/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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
 * @file    STM32L4xx/stm32_rcc.h
 * @brief   RCC helper driver header.
 * @note    This file requires definitions from the ST header file
 *          @p stm32l4xx.h.
 *
 * @addtogroup STM32L4xx_RCC
 * @{
 */
#ifndef _STM32_RCC_
#define _STM32_RCC_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    Generic RCC operations
 * @{
 */
/**
 * @brief   Enables the clock of one or more peripheral on the APB1 bus (R1).
 *
 * @param[in] mask      APB1 R1 peripherals mask
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableAPB1R1(mask, lp) {                                         \
  RCC->APB1ENR1 |= (mask);                                                  \
}

/**
 * @brief   Disables the clock of one or more peripheral on the APB1 bus (R1).
 *
 * @param[in] mask      APB1 R1 peripherals mask
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableAPB1R1(mask, lp) {                                        \
  RCC->APB1ENR1 &= ~(mask);                                                 \
}

/**
 * @brief   Resets one or more peripheral on the APB1 bus (R1).
 *
 * @param[in] mask      APB1 R1 peripherals mask
 *
 * @api
 */
#define rccResetAPB1R1(mask) {                                              \
  RCC->APB1RSTR1 |= (mask);                                                 \
  RCC->APB1RSTR1 = 0;                                                       \
}

/**
 * @brief   Enables the clock of one or more peripheral on the APB1 bus (R2).
 *
 * @param[in] mask      APB1 R2 peripherals mask
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableAPB1R2(mask, lp) {                                         \
  RCC->APB1ENR2 |= (mask);                                                  \
}

/**
 * @brief   Disables the clock of one or more peripheral on the APB1 bus (R2).
 *
 * @param[in] mask      APB1 R2 peripherals mask
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableAPB1R2(mask, lp) {                                        \
  RCC->APB1ENR2 &= ~(mask);                                                 \
}

/**
 * @brief   Resets one or more peripheral on the APB1 bus (R2).
 *
 * @param[in] mask      APB1 R2 peripherals mask
 *
 * @api
 */
#define rccResetAPB1R2(mask) {                                              \
  RCC->APB1RSTR2 |= (mask);                                                 \
  RCC->APB1RSTR2 = 0;                                                       \
}

/**
 * @brief   Enables the clock of one or more peripheral on the APB2 bus.
 *
 * @param[in] mask      APB2 peripherals mask
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableAPB2(mask, lp) {                                           \
  RCC->APB2ENR |= (mask);                                                   \
}

/**
 * @brief   Disables the clock of one or more peripheral on the APB2 bus.
 *
 * @param[in] mask      APB2 peripherals mask
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableAPB2(mask, lp) {                                          \
  RCC->APB2ENR &= ~(mask);                                                  \
}

/**
 * @brief   Resets one or more peripheral on the APB2 bus.
 *
 * @param[in] mask      APB2 peripherals mask
 *
 * @api
 */
#define rccResetAPB2(mask) {                                                \
  RCC->APB2RSTR |= (mask);                                                  \
  RCC->APB2RSTR = 0;                                                        \
}

/**
 * @brief   Enables the clock of one or more peripheral on the AHB1 bus.
 *
 * @param[in] mask      AHB1 peripherals mask
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableAHB1(mask, lp) {                                           \
  RCC->AHB1ENR |= (mask);                                                   \
}

/**
 * @brief   Disables the clock of one or more peripheral on the AHB1 bus.
 *
 * @param[in] mask      AHB1 peripherals mask
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableAHB1(mask, lp) {                                          \
  RCC->AHB1ENR &= ~(mask);                                                  \
}

/**
 * @brief   Resets one or more peripheral on the AHB1 bus.
 *
 * @param[in] mask      AHB1 peripherals mask
 *
 * @api
 */
#define rccResetAHB1(mask) {                                                \
  RCC->AHB1RSTR |= (mask);                                                  \
  RCC->AHB1RSTR = 0;                                                        \
}

/**
 * @brief   Enables the clock of one or more peripheral on the AHB2 bus.
 *
 * @param[in] mask      AHB2 peripherals mask
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableAHB2(mask, lp) {                                           \
  RCC->AHB2ENR |= (mask);                                                   \
}

/**
 * @brief   Disables the clock of one or more peripheral on the AHB2 bus.
 *
 * @param[in] mask      AHB2 peripherals mask
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableAHB2(mask, lp) {                                          \
  RCC->AHB2ENR &= ~(mask);                                                  \
}

/**
 * @brief   Resets one or more peripheral on the AHB2 bus.
 *
 * @param[in] mask      AHB2 peripherals mask
 *
 * @api
 */
#define rccResetAHB2(mask) {                                                \
  RCC->AHB2RSTR |= (mask);                                                  \
  RCC->AHB2RSTR = 0;                                                        \
}

/**
 * @brief   Enables the clock of one or more peripheral on the AHB3 (FSMC) bus.
 *
 * @param[in] mask      AHB3 peripherals mask
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableAHB3(mask, lp) {                                           \
  RCC->AHB3ENR |= (mask);                                                   \
}

/**
 * @brief   Disables the clock of one or more peripheral on the AHB3 (FSMC) bus.
 *
 * @param[in] mask      AHB3 peripherals mask
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableAHB3(mask, lp) {                                          \
  RCC->AHB3ENR &= ~(mask);                                                  \
}

/**
 * @brief   Resets one or more peripheral on the AHB3 (FSMC) bus.
 *
 * @param[in] mask      AHB3 peripherals mask
 *
 * @api
 */
#define rccResetAHB3(mask) {                                                \
  RCC->AHB3RSTR |= (mask);                                                  \
  RCC->AHB3RSTR = 0;                                                        \
}
/** @} */


/**
 * @name    ADC peripherals specific RCC operations
 * @{
 */
/**
 * @brief   Enables the ADC1/ADC2/ADC3 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableADC123(lp) rccEnableAHB2(RCC_AHB2ENR_ADCEN, lp)

/**
 * @brief   Disables the ADC1/ADC2/ADC3 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableADC123(lp) rccDisableAHB2(RCC_AHB2ENR_ADCEN, lp)

/**
 * @brief   Resets the ADC1/ADC2/ADC3 peripheral.
 *
 * @api
 */
#define rccResetADC123() rccResetAHB2(RCC_AHB2RSTR_ADCRST)
/** @} */

/**
 * @name    DAC peripheral specific RCC operations
 * @{
 */
/**
 * @brief   Enables the DAC1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableDAC1(lp) rccEnableAPB1R1(RCC_APB1ENR1_DAC1EN, lp)

/**
 * @brief   Disables the DAC1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableDAC1(lp) rccDisableAPB1R1(RCC_APB1ENR1_DAC1EN, lp)

/**
 * @brief   Resets the DAC1 peripheral.
 *
 * @api
 */
#define rccResetDAC1() rccResetAPB1R1(RCC_APB1RSTR1_DAC1RST)
/** @} */

/**
 * @name    DMA peripheral specific RCC operations
 * @{
 */
/**
 * @brief   Enables the DMA1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableDMA1(lp) rccEnableAHB1(RCC_AHB1ENR_DMA1EN, lp)

/**
 * @brief   Disables the DMA1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableDMA1(lp) rccDisableAHB1(RCC_AHB1ENR_DMA1EN, lp)

/**
 * @brief   Resets the DMA1 peripheral.
 *
 * @api
 */
#define rccResetDMA1() rccResetAHB1(RCC_AHB1RSTR_DMA1RST)

/**
 * @brief   Enables the DMA2 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableDMA2(lp) rccEnableAHB1(RCC_AHB1ENR_DMA2EN, lp)

/**
 * @brief   Disables the DMA2 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableDMA2(lp) rccDisableAHB1(RCC_AHB1ENR_DMA2EN, lp)

/**
 * @brief   Resets the DMA2 peripheral.
 *
 * @api
 */
#define rccResetDMA2() rccResetAHB1(RCC_AHB1RSTR_DMA2RST)
/** @} */

/**
 * @name    PWR interface specific RCC operations
 * @{
 */
/**
 * @brief   Enables the PWR interface clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnablePWRInterface(lp) rccEnableAPB1R1(RCC_APB1ENR1_PWREN, lp)

/**
 * @brief   Disables PWR interface clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisablePWRInterface(lp) rccDisableAPB1R1(RCC_APB1ENR1_PWREN, lp)

/**
 * @brief   Resets the PWR interface.
 *
 * @api
 */
#define rccResetPWRInterface() rccResetAPB1R1(RCC_APB1RSTR1_PWRRST)
/** @} */


/**
 * @name    CAN peripherals specific RCC operations
 * @{
 */
/**
 * @brief   Enables the CAN1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableCAN1(lp) rccEnableAPB1R1(RCC_APB1ENR1_CAN1EN, lp)

/**
 * @brief   Disables the CAN1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableCAN1(lp) rccDisableAPB1R1(RCC_APB1ENR1_CAN1EN, lp)

/**
 * @brief   Resets the CAN1 peripheral.
 *
 * @api
 */
#define rccResetCAN1() rccResetAPB1R1(RCC_APB1RSTR1_CAN1RST)
/** @} */

/**
 * @name    I2C peripherals specific RCC operations
 * @{
 */
/**
 * @brief   Enables the I2C1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableI2C1(lp) rccEnableAPB1R1(RCC_APB1ENR1_I2C1EN, lp)

/**
 * @brief   Disables the I2C1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableI2C1(lp) rccDisableAPB1R1(RCC_APB1ENR1_I2C1EN, lp)

/**
 * @brief   Resets the I2C1 peripheral.
 *
 * @api
 */
#define rccResetI2C1() rccResetAPB1R1(RCC_APB1RSTR1_I2C1RST)

/**
 * @brief   Enables the I2C2 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableI2C2(lp) rccEnableAPB1R1(RCC_APB1ENR1_I2C2EN, lp)

/**
 * @brief   Disables the I2C2 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableI2C2(lp) rccDisableAPB1R1(RCC_APB1ENR1_I2C2EN, lp)

/**
 * @brief   Resets the I2C2 peripheral.
 *
 * @api
 */
#define rccResetI2C2() rccResetAPB1R1(RCC_APB1RSTR1_I2C2RST)

/**
 * @brief   Enables the I2C3 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableI2C3(lp) rccEnableAPB1R1(RCC_APB1ENR1_I2C3EN, lp)

/**
 * @brief   Disables the I2C3 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableI2C3(lp) rccDisableAPB1R1(RCC_APB1ENR1_I2C3EN, lp)

/**
 * @brief   Resets the I2C3 peripheral.
 *
 * @api
 */
#define rccResetI2C3() rccResetAPB1R1(RCC_APB1RSTR1_I2C3RST)
/** @} */

/**
 * @name    OTG peripherals specific RCC operations
 * @{
 */
/**
 * @brief   Enables the OTG_FS peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableOTG_FS(lp) rccEnableAHB2(RCC_AHB2ENR_OTGFSEN, lp)

/**
 * @brief   Disables the OTG_FS peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableOTG_FS(lp) rccDisableAHB2(RCC_AHB2ENR_OTGFSEN, lp)

/**
 * @brief   Resets the OTG_FS peripheral.
 *
 * @api
 */
#define rccResetOTG_FS() rccResetAHB2(RCC_AHB2RSTR_OTGFSRST)
/** @} */

/**
 * @name    SDMMC peripheral specific RCC operations
 * @{
 */
/**
 * @brief   Enables the SDMMC1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableSDMMC1(lp) rccEnableAPB2(RCC_APB2ENR_SDMMC1EN, lp)

/**
 * @brief   Disables the SDMMC1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableSDMMC1(lp) rccDisableAPB2(RCC_APB2ENR_SDMMC1EN, lp)

/**
 * @brief   Resets the SDMMC1 peripheral.
 *
 * @api
 */
#define rccResetSDMMC1() rccResetAPB2(RCC_APB2RSTR_SDMMC1RST)
/** @} */

/**
 * @name    SPI peripherals specific RCC operations
 * @{
 */
/**
 * @brief   Enables the SPI1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableSPI1(lp) rccEnableAPB2(RCC_APB2ENR_SPI1EN, lp)

/**
 * @brief   Disables the SPI1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableSPI1(lp) rccDisableAPB2(RCC_APB2ENR_SPI1EN, lp)

/**
 * @brief   Resets the SPI1 peripheral.
 *
 * @api
 */
#define rccResetSPI1() rccResetAPB2(RCC_APB2RSTR_SPI1RST)

/**
 * @brief   Enables the SPI2 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableSPI2(lp) rccEnableAPB1R1(RCC_APB1ENR1_SPI2EN, lp)

/**
 * @brief   Disables the SPI2 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableSPI2(lp) rccDisableAPB1R1(RCC_APB1ENR1_SPI2EN, lp)

/**
 * @brief   Resets the SPI2 peripheral.
 *
 * @api
 */
#define rccResetSPI2() rccResetAPB1R1(RCC_APB1RSTR1_SPI2RST)

/**
 * @brief   Enables the SPI3 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableSPI3(lp) rccEnableAPB1R1(RCC_APB1ENR1_SPI3EN, lp)

/**
 * @brief   Disables the SPI3 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableSPI3(lp) rccDisableAPB1R1(RCC_APB1ENR1_SPI3EN, lp)

/**
 * @brief   Resets the SPI3 peripheral.
 *
 * @api
 */
#define rccResetSPI3() rccResetAPB1R1(RCC_APB1RSTR1_SPI3RST)
/** @} */

/**
 * @name    TIM peripherals specific RCC operations
 * @{
 */
/**
 * @brief   Enables the TIM1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM1(lp) rccEnableAPB2(RCC_APB2ENR_TIM1EN, lp)

/**
 * @brief   Disables the TIM1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableTIM1(lp) rccDisableAPB2(RCC_APB2ENR_TIM1EN, lp)

/**
 * @brief   Resets the TIM1 peripheral.
 *
 * @api
 */
#define rccResetTIM1() rccResetAPB2(RCC_APB2RSTR_TIM1RST)

/**
 * @brief   Enables the TIM2 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM2(lp) rccEnableAPB1R1(RCC_APB1ENR1_TIM2EN, lp)

/**
 * @brief   Disables the TIM2 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableTIM2(lp) rccDisableAPB1R1(RCC_APB1ENR1_TIM2EN, lp)

/**
 * @brief   Resets the TIM2 peripheral.
 *
 * @api
 */
#define rccResetTIM2() rccResetAPB1R1(RCC_APB1RSTR1_TIM2RST)

/**
 * @brief   Enables the TIM3 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM3(lp) rccEnableAPB1R1(RCC_APB1ENR1_TIM3EN, lp)

/**
 * @brief   Disables the TIM3 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableTIM3(lp) rccDisableAPB1R1(RCC_APB1ENR1_TIM3EN, lp)

/**
 * @brief   Resets the TIM3 peripheral.
 *
 * @api
 */
#define rccResetTIM3() rccResetAPB1R1(RCC_APB1RSTR1_TIM3RST)

/**
 * @brief   Enables the TIM4 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM4(lp) rccEnableAPB1R1(RCC_APB1ENR1_TIM4EN, lp)

/**
 * @brief   Disables the TIM4 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableTIM4(lp) rccDisableAPB1R1(RCC_APB1ENR1_TIM4EN, lp)

/**
 * @brief   Resets the TIM4 peripheral.
 *
 * @api
 */
#define rccResetTIM4() rccResetAPB1R1(RCC_APB1RSTR1_TIM4RST)

/**
 * @brief   Enables the TIM5 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM5(lp) rccEnableAPB1R1(RCC_APB1ENR1_TIM5EN, lp)

/**
 * @brief   Disables the TIM5 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableTIM5(lp) rccDisableAPB1R1(RCC_APB1ENR1_TIM5EN, lp)

/**
 * @brief   Resets the TIM5 peripheral.
 *
 * @api
 */
#define rccResetTIM5() rccResetAPB1R1(RCC_APB1RSTR1_TIM5RST)

/**
 * @brief   Enables the TIM6 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM6(lp) rccEnableAPB1R1(RCC_APB1ENR1_TIM6EN, lp)

/**
 * @brief   Disables the TIM6 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableTIM6(lp) rccDisableAPB1R1(RCC_APB1ENR1_TIM6EN, lp)

/**
 * @brief   Resets the TIM6 peripheral.
 *
 * @api
 */
#define rccResetTIM6() rccResetAPB1R1(RCC_APB1RSTR1_TIM6RST)

/**
 * @brief   Enables the TIM7 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM7(lp) rccEnableAPB1R1(RCC_APB1ENR1_TIM7EN, lp)

/**
 * @brief   Disables the TIM7 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableTIM7(lp) rccDisableAPB1R1(RCC_APB1ENR1_TIM7EN, lp)

/**
 * @brief   Resets the TIM7 peripheral.
 *
 * @api
 */
#define rccResetTIM7() rccResetAPB1R1(RCC_APB1RSTR1_TIM7RST)

/**
 * @brief   Enables the TIM8 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM8(lp) rccEnableAPB2(RCC_APB2ENR_TIM8EN, lp)

/**
 * @brief   Disables the TIM8 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableTIM8(lp) rccDisableAPB2(RCC_APB2ENR_TIM8EN, lp)

/**
 * @brief   Resets the TIM8 peripheral.
 *
 * @api
 */
#define rccResetTIM8() rccResetAPB2(RCC_APB2RSTR_TIM8RST)

/**
 * @brief   Enables the TIM15 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM15(lp) rccEnableAPB2(RCC_APB2ENR_TIM15EN, lp)

/**
 * @brief   Disables the TIM15 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableTIM15(lp) rccDisableAPB2(RCC_APB2ENR_TIM15EN, lp)

/**
 * @brief   Resets the TIM15 peripheral.
 *
 * @api
 */
#define rccResetTIM15() rccResetAPB2(RCC_APB2RSTR_TIM15RST)

/**
 * @brief   Enables the TIM16 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM16(lp) rccEnableAPB2(RCC_APB2ENR_TIM16EN, lp)

/**
 * @brief   Disables the TIM16 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableTIM16(lp) rccDisableAPB2(RCC_APB2ENR_TIM16EN, lp)

/**
 * @brief   Resets the TIM16 peripheral.
 *
 * @api
 */
#define rccResetTIM16() rccResetAPB2(RCC_APB2RSTR_TIM16RST)

/**
 * @brief   Enables the TIM17 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM17(lp) rccEnableAPB2(RCC_APB2ENR_TIM17EN, lp)

/**
 * @brief   Disables the TIM17 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableTIM17(lp) rccDisableAPB2(RCC_APB2ENR_TIM17EN, lp)

/**
 * @brief   Resets the TIM17 peripheral.
 *
 * @api
 */
#define rccResetTIM17() rccResetAPB2(RCC_APB2RSTR_TIM17RST)
/** @} */

/**
 * @name    USART/UART peripherals specific RCC operations
 * @{
 */
/**
 * @brief   Enables the USART1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableUSART1(lp) rccEnableAPB2(RCC_APB2ENR_USART1EN, lp)

/**
 * @brief   Disables the USART1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableUSART1(lp) rccDisableAPB2(RCC_APB2ENR_USART1EN, lp)

/**
 * @brief   Resets the USART1 peripheral.
 *
 * @api
 */
#define rccResetUSART1() rccResetAPB2(RCC_APB2RSTR_USART1RST)

/**
 * @brief   Enables the USART2 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableUSART2(lp) rccEnableAPB1R1(RCC_APB1ENR1_USART2EN, lp)

/**
 * @brief   Disables the USART2 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableUSART2(lp) rccDisableAPB1R1(RCC_APB1ENR1_USART2EN, lp)

/**
 * @brief   Resets the USART2 peripheral.
 *
 * @api
 */
#define rccResetUSART2() rccResetAPB1R1(RCC_APB1RSTR1_USART2RST)

/**
 * @brief   Enables the USART3 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableUSART3(lp) rccEnableAPB1R1(RCC_APB1ENR1_USART3EN, lp)

/**
 * @brief   Disables the USART3 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableUSART3(lp) rccDisableAPB1R1(RCC_APB1ENR1_USART3EN, lp)

/**
 * @brief   Resets the USART3 peripheral.
 *
 * @api
 */
#define rccResetUSART3() rccResetAPB1R1(RCC_APB1RSTR1_USART3RST)

/**
 * @brief   Enables the UART4 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableUART4(lp) rccEnableAPB1R1(RCC_APB1ENR1_UART4EN, lp)

/**
 * @brief   Disables the UART4 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableUART4(lp) rccDisableAPB1R1(RCC_APB1ENR1_UART4EN, lp)

/**
 * @brief   Resets the UART4 peripheral.
 *
 * @api
 */
#define rccResetUART4() rccResetAPB1R1(RCC_APB1RSTR1_UART4RST)

/**
 * @brief   Enables the UART5 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableUART5(lp) rccEnableAPB1R1(RCC_APB1ENR1_UART5EN, lp)

/**
 * @brief   Disables the UART5 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableUART5(lp) rccDisableAPB1R1(RCC_APB1ENR1_UART5EN, lp)

/**
 * @brief   Resets the UART5 peripheral.
 *
 * @api
 */
#define rccResetUART5() rccResetAPB1R1(RCC_APB1RSTR1_UART5RST)

/**
 * @brief   Enables the LPUART1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableLPUART1(lp) rccEnableAPB1R2(RCC_APB1ENR2_LPUART1EN, lp)

/**
 * @brief   Disables the LPUART1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableLPUART1(lp) rccDisableAPB1R2(RCC_APB1ENR2_LPUART1EN, lp)

/**
 * @brief   Resets the USART1 peripheral.
 *
 * @api
 */
#define rccResetLPUART1() rccResetAPB1R2(RCC_APB1RSTR2_LPUART1RST)
/** @} */

/**
 * @name    FSMC peripherals specific RCC operations
 * @{
 */
/**
 * @brief   Enables the FSMC peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableFSMC(lp) rccEnableAHB3(RCC_AHB3ENR_FMCEN, lp)

/**
 * @brief   Disables the FSMC peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccDisableFSMC(lp) rccDisableAHB3(RCC_AHB3ENR_FMCEN, lp)

/**
 * @brief   Resets the FSMC peripheral.
 *
 * @api
 */
#define rccResetFSMC() rccResetAHB3(RCC_AHB3RSTR_FMCRST)
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

#endif /* _STM32_RCC_ */

/** @} */
