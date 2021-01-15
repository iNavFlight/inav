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
 * @file    STM32F7xx/stm32_rcc.h
 * @brief   RCC helper driver header.
 * @note    This file requires definitions from the ST header file
 *          @p stm32f7xx.h.
 *
 * @addtogroup STM32F7xx_RCC
 * @{
 */
#ifndef STM32_RCC_H
#define STM32_RCC_H

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
 * @brief   Enables the clock of one or more peripheral on the APB1 bus.
 *
 * @param[in] mask      APB1 peripherals mask, low set
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableAPB1L(mask, lp) {                                          \
  RCC->APB1LENR |= (mask);                                                  \
  if (lp)                                                                   \
    RCC->APB1LLPENR |= (mask);                                              \
  else                                                                      \
    RCC->APB1LLPENR &= ~(mask);                                             \
  (void)RCC->APB1LLPENR;                                                    \
}

/**
 * @brief   Enables the clock of one or more peripheral on the APB1 bus.
 *
 * @param[in] mask      APB1 peripherals mask, high set
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableAPB1H(mask, lp) {                                          \
  RCC->APB1HENR |= (mask);                                                  \
  if (lp)                                                                   \
    RCC->APB1HLPENR |= (mask);                                              \
  else                                                                      \
    RCC->APB1HLPENR &= ~(mask);                                             \
  (void)RCC->APB1HLPENR;                                                    \
}

/**
 * @brief   Disables the clock of one or more peripheral on the APB1 bus.
 *
 * @param[in] mask      APB1 peripherals mask, low set
 *
 * @api
 */
#define rccDisableAPB1L(mask) {                                             \
  RCC->APB1LENR &= ~(mask);                                                 \
  RCC->APB1LLPENR &= ~(mask);                                               \
  (void)RCC->APB1LLPENR;                                                    \
}

/**
 * @brief   Disables the clock of one or more peripheral on the APB1 bus.
 *
 * @param[in] mask      APB1 peripherals mask, high set
 *
 * @api
 */
#define rccDisableAPB1H(mask) {                                             \
  RCC->APB1HENR &= ~(mask);                                                 \
  RCC->APB1HLPENR &= ~(mask);                                               \
  (void)RCC->APB1HLPENR;                                                    \
}

/**
 * @brief   Resets one or more peripheral on the APB1 bus.
 *
 * @param[in] mask      APB1 peripherals mask, low set
 *
 * @api
 */
#define rccResetAPB1L(mask) {                                               \
  RCC->APB1LRSTR |= (mask);                                                 \
  RCC->APB1LRSTR &= ~(mask);                                                \
  (void)RCC->APB1LRSTR;                                                     \
}

/**
 * @brief   Resets one or more peripheral on the APB1 bus.
 *
 * @param[in] mask      APB1 peripherals mask, high set
 *
 * @api
 */
#define rccResetAPB1H(mask) {                                               \
  RCC->APB1HRSTR |= (mask);                                                 \
  RCC->APB1HRSTR &= ~(mask);                                                \
  (void)RCC->APB1HRSTR;                                                     \
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
  if (lp)                                                                   \
    RCC->APB2LPENR |= (mask);                                               \
  else                                                                      \
    RCC->APB2LPENR &= ~(mask);                                              \
  (void)RCC->APB2LPENR;                                                     \
}

/**
 * @brief   Disables the clock of one or more peripheral on the APB2 bus.
 *
 * @param[in] mask      APB2 peripherals mask
 *
 * @api
 */
#define rccDisableAPB2(mask) {                                              \
  RCC->APB2ENR &= ~(mask);                                                  \
  RCC->APB2LPENR &= ~(mask);                                                \
  (void)RCC->APB2LPENR;                                                     \
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
  RCC->APB2RSTR &= ~(mask);                                                 \
  (void)RCC->APB2RSTR;                                                      \
}

/**
 * @brief   Enables the clock of one or more peripheral on the APB3 bus.
 *
 * @param[in] mask      APB3 peripherals mask
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableAPB3(mask, lp) {                                           \
  RCC->APB3ENR |= (mask);                                                   \
  if (lp)                                                                   \
    RCC->APB3LPENR |= (mask);                                               \
  else                                                                      \
    RCC->APB3LPENR &= ~(mask);                                              \
  (void)RCC->APB3LPENR;                                                     \
}

/**
 * @brief   Disables the clock of one or more peripheral on the APB3 bus.
 *
 * @param[in] mask      APB3 peripherals mask
 *
 * @api
 */
#define rccDisableAPB3(mask) {                                              \
  RCC->APB3ENR &= ~(mask);                                                  \
  RCC->APB3LPENR &= ~(mask);                                                \
  (void)RCC->APB3LPENR;                                                     \
}

/**
 * @brief   Resets one or more peripheral on the APB3 bus.
 *
 * @param[in] mask      APB2 peripherals mask
 *
 * @api
 */
#define rccResetAPB3(mask) {                                                \
  RCC->APB3RSTR |= (mask);                                                  \
  RCC->APB3RSTR &= ~(mask);                                                 \
  (void)RCC->APB3RSTR;                                                      \
}

/**
 * @brief   Enables the clock of one or more peripheral on the APB4 bus.
 *
 * @param[in] mask      APB4 peripherals mask
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableAPB4(mask, lp) {                                           \
  RCC->APB4ENR |= (mask);                                                   \
  if (lp)                                                                   \
    RCC->APB4LPENR |= (mask);                                               \
  else                                                                      \
    RCC->APB4LPENR &= ~(mask);                                              \
  (void)RCC->APB4LPENR;                                                     \
}

/**
 * @brief   Disables the clock of one or more peripheral on the APB4 bus.
 *
 * @param[in] mask      APB4 peripherals mask
 *
 * @api
 */
#define rccDisableAPB4(mask) {                                              \
  RCC->APB4ENR &= ~(mask);                                                  \
  RCC->APB4LPENR &= ~(mask);                                                \
  (void)RCC->APB4LPENR;                                                     \
}

/**
 * @brief   Resets one or more peripheral on the APB4 bus.
 *
 * @param[in] mask      APB4 peripherals mask
 *
 * @api
 */
#define rccResetAPB4(mask) {                                                \
  RCC->APB4RSTR |= (mask);                                                  \
  RCC->APB4RSTR &= ~(mask);                                                 \
  (void)RCC->APB4RSTR;                                                      \
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
  if (lp)                                                                   \
    RCC->AHB1LPENR |= (mask);                                               \
  else                                                                      \
    RCC->AHB1LPENR &= ~(mask);                                              \
  (void)RCC->AHB1LPENR;                                                     \
}

/**
 * @brief   Disables the clock of one or more peripheral on the AHB1 bus.
 *
 * @param[in] mask      AHB1 peripherals mask
 *
 * @api
 */
#define rccDisableAHB1(mask) {                                              \
  RCC->AHB1ENR &= ~(mask);                                                  \
  RCC->AHB1LPENR &= ~(mask);                                                \
  (void)RCC->AHB1LPENR;                                                     \
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
  RCC->AHB1RSTR &= ~(mask);                                                 \
  (void)RCC->AHB1RSTR;                                                      \
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
  if (lp)                                                                   \
    RCC->AHB2LPENR |= (mask);                                               \
  else                                                                      \
    RCC->AHB2LPENR &= ~(mask);                                              \
  (void)RCC->AHB2LPENR;                                                     \
}

/**
 * @brief   Disables the clock of one or more peripheral on the AHB2 bus.
 *
 * @param[in] mask      AHB2 peripherals mask
 *
 * @api
 */
#define rccDisableAHB2(mask) {                                              \
  RCC->AHB2ENR &= ~(mask);                                                  \
  RCC->AHB2LPENR &= ~(mask);                                                \
  (void)RCC->AHB2LPENR;                                                     \
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
  RCC->AHB2RSTR &= ~(mask);                                                 \
  (void)RCC->AHB2RSTR;                                                      \
}

/**
 * @brief   Enables the clock of one or more peripheral on the AHB3 bus.
 *
 * @param[in] mask      AHB3 peripherals mask
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableAHB3(mask, lp) {                                           \
  RCC->AHB3ENR |= (mask);                                                   \
  if (lp)                                                                   \
    RCC->AHB3LPENR |= (mask);                                               \
  else                                                                      \
    RCC->AHB3LPENR &= ~(mask);                                              \
  (void)RCC->AHB3LPENR;                                                     \
}

/**
 * @brief   Disables the clock of one or more peripheral on the AHB3 bus.
 *
 * @param[in] mask      AHB3 peripherals mask
 *
 * @api
 */
#define rccDisableAHB3(mask) {                                              \
  RCC->AHB3ENR &= ~(mask);                                                  \
  RCC->AHB3LPENR &= ~(mask);                                                \
  (void)RCC->AHB3LPENR;                                                     \
}

/**
 * @brief   Resets one or more peripheral on the AHB3 bus.
 *
 * @param[in] mask      AHB3 peripherals mask
 *
 * @api
 */
#define rccResetAHB3(mask) {                                                \
  RCC->AHB3RSTR |= (mask);                                                  \
  RCC->AHB3RSTR &= ~(mask);                                                 \
  (void)RCC->AHB3RSTR;                                                      \
}

/**
 * @brief   Enables the clock of one or more peripheral on the AHB4 bus.
 *
 * @param[in] mask      AHB4 peripherals mask
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableAHB4(mask, lp) {                                           \
  RCC->AHB4ENR |= (mask);                                                   \
  if (lp)                                                                   \
    RCC->AHB4LPENR |= (mask);                                               \
  else                                                                      \
    RCC->AHB4LPENR &= ~(mask);                                              \
  (void)RCC->AHB4LPENR;                                                     \
}

/**
 * @brief   Disables the clock of one or more peripheral on the AHB4 bus.
 *
 * @param[in] mask      AHB4 peripherals mask
 *
 * @api
 */
#define rccDisableAHB4(mask) {                                              \
  RCC->AHB4ENR &= ~(mask);                                                  \
  RCC->AHB4LPENR &= ~(mask);                                                \
  (void)RCC->AHB4LPENR;                                                     \
}

/**
 * @brief   Resets one or more peripheral on the AHB4 bus.
 *
 * @param[in] mask      AHB4 peripherals mask
 *
 * @api
 */
#define rccResetAHB4(mask) {                                                \
  RCC->AHB4RSTR |= (mask);                                                  \
  RCC->AHB4RSTR &= ~(mask);                                                 \
  (void)RCC->AHB4RSTR;                                                      \
}
/** @} */

/**
 * @name    ADC peripherals specific RCC operations
 * @{
 */
/**
 * @brief   Enables the ADC1/ADC2 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableADC12(lp) rccEnableAHB1(RCC_AHB1ENR_ADC12EN, lp)

/**
 * @brief   Disables the ADC1/ADC2 peripheral clock.
 *
 * @api
 */
#define rccDisableADC12() rccDisableAHB1(RCC_AHB1ENR_ADC12EN)

/**
 * @brief   Resets the ADC1/ADC2 peripheral.
 *
 * @api
 */
#define rccResetADC12() rccResetAHB1(RCC_AHB1RSTR_ADC12RST)

/**
 * @brief   Enables the ADC3 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableADC3(lp) rccEnableAHB4(RCC_AHB4ENR_ADC3EN, lp)

/**
 * @brief   Disables the ADC3 peripheral clock.
 *
 * @api
 */
#define rccDisableADC3() rccDisableAHB4(RCC_AHB4ENR_ADC3EN)

/**
 * @brief   Resets the ADC3 peripheral.
 *
 * @api
 */
#define rccResetADC3() rccResetAHB4(RCC_AHB4RSTR_ADC3RST)
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
#define rccEnableDAC1(lp) rccEnableAPB1L(RCC_APB1LENR_DAC12EN, lp)

/**
 * @brief   Disables the DAC1 peripheral clock.
 *
 * @api
 */
#define rccDisableDAC1() rccDisableAPB1L(RCC_APB1LENR_DAC12EN)

/**
 * @brief   Resets the DAC1 peripheral.
 *
 * @api
 */
#define rccResetDAC1() rccResetAPB1L(RCC_APB1LRSTR_DAC12RST)
/** @} */

/**
 * @name    DMA peripheral specific RCC operations
 * @{
 */
/**
 * @brief   Enables the BDMA1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableBDMA1(lp) rccEnableAHB4(RCC_AHB4ENR_BDMAEN, lp)

/**
 * @brief   Disables the BDMA1 peripheral clock.
 *
 * @api
 */
#define rccDisableBDMA1() rccDisableAHB4(RCC_AHB4ENR_BDMAEN)

/**
 * @brief   Resets the BDMA1 peripheral.
 *
 * @api
 */
#define rccResetBDMA1() rccResetAHB4(RCC_AHB4RSTR_BDMARST)

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
 * @api
 */
#define rccDisableDMA1() rccDisableAHB1(RCC_AHB1ENR_DMA1EN)

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
 * @api
 */
#define rccDisableDMA2() rccDisableAHB1(RCC_AHB1ENR_DMA2EN)

/**
 * @brief   Resets the DMA2 peripheral.
 *
 * @api
 */
#define rccResetDMA2() rccResetAHB1(RCC_AHB1RSTR_DMA2RST)
/** @} */

/**
 * @name    RAM specific RCC operations
 * @{
 */
/**
 * @brief   Enables the BKPRAM clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableBKPRAM(lp) rccEnableAHB4(RCC_AHB4ENR_BKPRAMEN, lp)

/**
 * @brief   Disables the BKPRAM clock.
 *
 * @api
 */
#define rccDisableBKPRAM() rccDisableAHB4(RCC_AHB4ENR_BKPRAMEN)

/**
 * @brief   Enables the SRAM1 clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableSRAM1(lp) rccEnableAHB2(RCC_AHB2ENR_D2SRAM1EN, lp)

/**
 * @brief   Disables the SRAM1 clock.
 *
 * @api
 */
#define rccDisableSRAM1() rccDisableAHB2(RCC_AHB2ENR_D2SRAM1EN)

/**
 * @brief   Enables the SRAM2 clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableSRAM2(lp) rccEnableAHB2(RCC_AHB2ENR_D2SRAM2EN, lp)

/**
 * @brief   Disables the SRAM2 clock.
 *
 * @api
 */
#define rccDisableSRAM2() rccDisableAHB2(RCC_AHB2ENR_D2SRAM2EN)

/**
 * @brief   Enables the SRAM3 clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableSRAM3(lp) rccEnableAHB2(RCC_AHB2ENR_D2SRAM3EN, lp)

/**
 * @brief   Disables the SRAM3 clock.
 *
 * @api
 */
#define rccDisableSRAM3() rccDisableAHB2(RCC_AHB2ENR_D2SRAM3EN)
/** @} */

/**
 * @name    ETH peripheral specific RCC operations
 * @{
 */
/**
 * @brief   Enables the ETH peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableETH(lp) rccEnableAHB1(RCC_AHB1ENR_ETHMACEN |               \
                                       RCC_AHB1ENR_ETHMACTXEN |             \
                                       RCC_AHB1ENR_ETHMACRXEN, lp)

/**
 * @brief   Disables the ETH peripheral clock.
 *
 * @api
 */
#define rccDisableETH() rccDisableAHB1(RCC_AHB1ENR_ETHMACEN |            \
                                          RCC_AHB1ENR_ETHMACTXEN |          \
                                          RCC_AHB1ENR_ETHMACRXEN)

/**
 * @brief   Resets the ETH peripheral.
 *
 * @api
 */
#define rccResetETH() rccResetAHB1(RCC_AHB1RSTR_ETHMACRST)
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
#define rccEnableI2C1(lp) rccEnableAPB1L(RCC_APB1LENR_I2C1EN, lp)

/**
 * @brief   Disables the I2C1 peripheral clock.
 *
 * @api
 */
#define rccDisableI2C1() rccDisableAPB1L(RCC_APB1LENR_I2C1EN)

/**
 * @brief   Resets the I2C1 peripheral.
 *
 * @api
 */
#define rccResetI2C1() rccResetAPB1L(RCC_APB1LRSTR_I2C1RST)

/**
 * @brief   Enables the I2C2 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableI2C2(lp) rccEnableAPB1L(RCC_APB1LENR_I2C2EN, lp)

/**
 * @brief   Disables the I2C2 peripheral clock.
 *
 * @api
 */
#define rccDisableI2C2() rccDisableAPB1L(RCC_APB1LENR_I2C2EN)

/**
 * @brief   Resets the I2C2 peripheral.
 *
 * @api
 */
#define rccResetI2C2() rccResetAPB1L(RCC_APB1LRSTR_I2C2RST)

/**
 * @brief   Enables the I2C3 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableI2C3(lp) rccEnableAPB1L(RCC_APB1LENR_I2C3EN, lp)

/**
 * @brief   Disables the I2C3 peripheral clock.
 *
 * @api
 */
#define rccDisableI2C3() rccDisableAPB1L(RCC_APB1LENR_I2C3EN)

/**
 * @brief   Resets the I2C3 peripheral.
 *
 * @api
 */
#define rccResetI2C3() rccResetAPB1L(RCC_APB1LRSTR_I2C3RST)

/**
 * @brief   Enables the I2C4 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableI2C4(lp) rccEnableAPB4(RCC_APB4ENR_I2C4EN, lp)

/**
 * @brief   Disables the I2C4 peripheral clock.
 *
 * @api
 */
#define rccDisableI2C4() rccDisableAPB4(RCC_APB4ENR_I2C4EN)

/**
 * @brief   Resets the I2C4 peripheral.
 *
 * @api
 */
#define rccResetI2C4() rccResetAPB4(RCC_APB4RSTR_I2C4RST)
/** @} */

/**
 * @name    OTG peripherals specific RCC operations
 * @{
 */
/**
 * @brief   Enables the USB1_OTG_HS peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableUSB1_OTG_HS(lp) rccEnableAHB1(RCC_AHB1ENR_USB1OTGHSEN, lp)

/**
 * @brief   Disables the USB1_OTG_HS peripheral clock.
 *
 * @api
 */
#define rccDisableUSB1_OTG_HS() rccDisableAHB1(RCC_AHB1ENR_USB1OTGHSEN)

/**
 * @brief   Resets the USB1_OTG_HS peripheral.
 *
 * @api
 */
#define rccResetUSB1_OTG_HS() rccResetAHB1(RCC_AHB1RSTR_USB1OTGHSRST)

/**
 * @brief   Enables the USB2_OTG_HS peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableUSB2_OTG_HS(lp) rccEnableAHB1(RCC_AHB1ENR_USB2OTGHSEN, lp)

/**
 * @brief   Disables the USB2_OTG_HS peripheral clock.
 *
 * @api
 */
#define rccDisableUSB2_OTG_HS() rccDisableAHB1(RCC_AHB1ENR_USB2OTGHSEN)

/**
 * @brief   Resets the USB2_OTG_HS peripheral.
 *
 * @api
 */
#define rccResetUSB2_OTG_HS() rccResetAHB1(RCC_AHB1RSTR_USB2OTGHSRST)

/**
 * @brief   Enables the USB1_OTG_HS ULPI peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableUSB1_HSULPI(lp) rccEnableAHB1(RCC_AHB1ENR_USB1OTGHSULPIEN, lp)

/**
 * @brief   Disables the USB1_OTG_HS peripheral clock.
 *
 * @api
 */
#define rccDisableUSB1_HSULPI() rccDisableAHB1(RCC_AHB1ENR_USB1OTGHSULPIEN)

/**
 * @brief   Enables the USB2_OTG_HS ULPI peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableUSB2_HSULPI(lp) rccEnableAHB1(RCC_AHB1ENR_USB2OTGHSULPIEN, lp)

/**
 * @brief   Disables the USB2_OTG_HS peripheral clock.
 *
 * @api
 */
#define rccDisableUSB2_HSULPI() rccDisableAHB1(RCC_AHB1ENR_USB2OTGHSULPIEN)
/** @} */

/**
 * @name    QUADSPI peripherals specific RCC operations
 * @{
 */
/**
 * @brief   Enables the QUADSPI1 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableQUADSPI1(lp) rccEnableAHB3(RCC_AHB3ENR_QSPIEN, lp)

/**
 * @brief   Disables the QUADSPI1 peripheral clock.
 *
 * @api
 */
#define rccDisableQUADSPI1() rccDisableAHB3(RCC_AHB3ENR_QSPIEN)

/**
 * @brief   Resets the QUADSPI1 peripheral.
 *
 * @api
 */
#define rccResetQUADSPI1() rccResetAHB3(RCC_AHB3RSTR_QSPIRST)
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
#define rccEnableSDMMC1(lp) rccEnableAHB3(RCC_AHB3ENR_SDMMC1EN, lp)

/**
 * @brief   Disables the SDMMC1 peripheral clock.
 *
 * @api
 */
#define rccDisableSDMMC1() rccDisableAHB3(RCC_AHB3ENR_SDMMC1EN)

/**
 * @brief   Resets the SDMMC1 peripheral.
 *
 * @api
 */
#define rccResetSDMMC1() rccResetAHB3(RCC_AHB3RSTR_SDMMC1RST)

/**
 * @brief   Enables the SDMMC2 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableSDMMC2(lp) rccEnableAHB3(RCC_AHB3ENR_SDMMC2EN, lp)

/**
 * @brief   Disables the SDMMC2 peripheral clock.
 *
 * @api
 */
#define rccDisableSDMMC2() rccDisableAHB3(RCC_AHB3ENR_SDMMC2EN)

/**
 * @brief   Resets the SDMMC2 peripheral.
 *
 * @api
 */
#define rccResetSDMMC2() rccResetAHB3(RCC_AHB3RSTR_SDMMC2RST)
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
 * @api
 */
#define rccDisableSPI1() rccDisableAPB2(RCC_APB2ENR_SPI1EN)

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
#define rccEnableSPI2(lp) rccEnableAPB1L(RCC_APB1LENR_SPI2EN, lp)

/**
 * @brief   Disables the SPI2 peripheral clock.
 *
 * @api
 */
#define rccDisableSPI2() rccDisableAPB1L(RCC_APB1LENR_SPI2EN)

/**
 * @brief   Resets the SPI2 peripheral.
 *
 * @api
 */
#define rccResetSPI2() rccResetAPB1L(RCC_APB1LRSTR_SPI2RST)

/**
 * @brief   Enables the SPI3 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableSPI3(lp) rccEnableAPB1L(RCC_APB1LENR_SPI3EN, lp)

/**
 * @brief   Disables the SPI3 peripheral clock.
 *
 * @api
 */
#define rccDisableSPI3() rccDisableAPB1L(RCC_APB1LENR_SPI3EN)

/**
 * @brief   Resets the SPI3 peripheral.
 *
 * @api
 */
#define rccResetSPI3() rccResetAPB1L(RCC_APB1LRSTR_SPI3RST)

/**
 * @brief   Enables the SPI4 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableSPI4(lp) rccEnableAPB2(RCC_APB2ENR_SPI4EN, lp)

/**
 * @brief   Disables the SPI4 peripheral clock.
 *
 * @api
 */
#define rccDisableSPI4() rccDisableAPB2(RCC_APB2ENR_SPI4EN)

/**
 * @brief   Resets the SPI4 peripheral.
 *
 * @api
 */
#define rccResetSPI4() rccResetAPB2(RCC_APB2RSTR_SPI4RST)

/**
 * @brief   Enables the SPI5 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableSPI5(lp) rccEnableAPB2(RCC_APB2ENR_SPI5EN, lp)

/**
 * @brief   Disables the SPI5 peripheral clock.
 *
 * @api
 */
#define rccDisableSPI5() rccDisableAPB2(RCC_APB2ENR_SPI5EN)

/**
 * @brief   Resets the SPI5 peripheral.
 *
 * @api
 */
#define rccResetSPI5() rccResetAPB2(RCC_APB2RSTR_SPI5RST)

/**
 * @brief   Enables the SPI6 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableSPI6(lp) rccEnableAPB4(RCC_APB4ENR_SPI6EN, lp)

/**
 * @brief   Disables the SPI6 peripheral clock.
 *
 * @api
 */
#define rccDisableSPI6() rccDisableAPB4(RCC_APB4ENR_SPI6EN)

/**
 * @brief   Resets the SPI6 peripheral.
 *
 * @api
 */
#define rccResetSPI6() rccResetAPB4(RCC_APB4RSTR_SPI6RST)
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
 * @api
 */
#define rccDisableTIM1() rccDisableAPB2(RCC_APB2ENR_TIM1EN)

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
#define rccEnableTIM2(lp) rccEnableAPB1L(RCC_APB1LENR_TIM2EN, lp)

/**
 * @brief   Disables the TIM2 peripheral clock.
 *
 * @api
 */
#define rccDisableTIM2() rccDisableAPB1L(RCC_APB1LENR_TIM2EN)

/**
 * @brief   Resets the TIM2 peripheral.
 *
 * @api
 */
#define rccResetTIM2() rccResetAPB1L(RCC_APB1LRSTR_TIM2RST)

/**
 * @brief   Enables the TIM3 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM3(lp) rccEnableAPB1L(RCC_APB1LENR_TIM3EN, lp)

/**
 * @brief   Disables the TIM3 peripheral clock.
 *
 * @api
 */
#define rccDisableTIM3() rccDisableAPB1L(RCC_APB1LENR_TIM3EN)

/**
 * @brief   Resets the TIM3 peripheral.
 *
 * @api
 */
#define rccResetTIM3() rccResetAPB1L(RCC_APB1LRSTR_TIM3RST)

/**
 * @brief   Enables the TIM4 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM4(lp) rccEnableAPB1L(RCC_APB1LENR_TIM4EN, lp)

/**
 * @brief   Disables the TIM4 peripheral clock.
 *
 * @api
 */
#define rccDisableTIM4() rccDisableAPB1L(RCC_APB1LENR_TIM4EN)

/**
 * @brief   Resets the TIM4 peripheral.
 *
 * @api
 */
#define rccResetTIM4() rccResetAPB1L(RCC_APB1LRSTR_TIM4RST)

/**
 * @brief   Enables the TIM5 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM5(lp) rccEnableAPB1L(RCC_APB1LENR_TIM5EN, lp)

/**
 * @brief   Disables the TIM5 peripheral clock.
 *
 * @api
 */
#define rccDisableTIM5() rccDisableAPB1L(RCC_APB1LENR_TIM5EN)

/**
 * @brief   Resets the TIM5 peripheral.
 *
 * @api
 */
#define rccResetTIM5() rccResetAPB1L(RCC_APB1LRSTR_TIM5RST)

/**
 * @brief   Enables the TIM6 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM6(lp) rccEnableAPB1L(RCC_APB1LENR_TIM6EN, lp)

/**
 * @brief   Disables the TIM6 peripheral clock.
 *
 * @api
 */
#define rccDisableTIM6() rccDisableAPB1L(RCC_APB1LENR_TIM6EN)

/**
 * @brief   Resets the TIM6 peripheral.
 *
 * @api
 */
#define rccResetTIM6() rccResetAPB1L(RCC_APB1LRSTR_TIM6RST)

/**
 * @brief   Enables the TIM7 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM7(lp) rccEnableAPB1L(RCC_APB1LENR_TIM7EN, lp)

/**
 * @brief   Disables the TIM7 peripheral clock.
 *
 * @api
 */
#define rccDisableTIM7() rccDisableAPB1L(RCC_APB1LENR_TIM7EN)

/**
 * @brief   Resets the TIM7 peripheral.
 *
 * @api
 */
#define rccResetTIM7() rccResetAPB1L(RCC_APB1LRSTR_TIM7RST)

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
 * @api
 */
#define rccDisableTIM8() rccDisableAPB2(RCC_APB2ENR_TIM8EN)

/**
 * @brief   Resets the TIM8 peripheral.
 *
 * @api
 */
#define rccResetTIM8() rccResetAPB2(RCC_APB2RSTR_TIM8RST)

/**
 * @brief   Enables the TIM12 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM12(lp) rccEnableAPB1L(RCC_APB1LENR_TIM12EN, lp)

/**
 * @brief   Disables the TIM12 peripheral clock.
 *
 * @api
 */
#define rccDisableTIM12() rccDisableAPB1L(RCC_APB1LENR_TIM12EN)

/**
 * @brief   Resets the TIM12 peripheral.
 *
 * @api
 */
#define rccResetTIM12() rccResetAPB1L(RCC_APB1LRSTR_TIM12RST)

/**
 * @brief   Enables the TIM13 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM13(lp) rccEnableAPB1L(RCC_APB1LENR_TIM13EN, lp)

/**
 * @brief   Disables the TIM13 peripheral clock.
 *
 * @api
 */
#define rccDisableTIM13() rccDisableAPB1L(RCC_APB1LENR_TIM13EN)

/**
 * @brief   Resets the TIM13 peripheral.
 *
 * @api
 */
#define rccResetTIM13() rccResetAPB1L(RCC_APB1LRSTR_TIM13RST)

/**
 * @brief   Enables the TIM14 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableTIM14(lp) rccEnableAPB1L(RCC_APB1LENR_TIM14EN, lp)

/**
 * @brief   Disables the TIM14 peripheral clock.
 *
 * @api
 */
#define rccDisableTIM14() rccDisableAPB1L(RCC_APB1LENR_TIM14EN)

/**
 * @brief   Resets the TIM14 peripheral.
 *
 * @api
 */
#define rccResetTIM14() rccResetAPB1L(RCC_APB1LRSTR_TIM14RST)

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
 * @api
 */
#define rccDisableTIM15() rccDisableAPB2(RCC_APB2ENR_TIM15EN)

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
 * @api
 */
#define rccDisableTIM16() rccDisableAPB2(RCC_APB2ENR_TIM16EN)

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
 * @api
 */
#define rccDisableTIM17() rccDisableAPB2(RCC_APB2ENR_TIM17EN)

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
 * @api
 */
#define rccDisableUSART1() rccDisableAPB2(RCC_APB2ENR_USART1EN)

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
#define rccEnableUSART2(lp) rccEnableAPB1L(RCC_APB1LENR_USART2EN, lp)

/**
 * @brief   Disables the USART2 peripheral clock.
 *
 * @api
 */
#define rccDisableUSART2() rccDisableAPB1L(RCC_APB1LENR_USART2EN)

/**
 * @brief   Resets the USART2 peripheral.
 *
 * @api
 */
#define rccResetUSART2() rccResetAPB1L(RCC_APB1LRSTR_USART2RST)

/**
 * @brief   Enables the USART3 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableUSART3(lp) rccEnableAPB1L(RCC_APB1LENR_USART3EN, lp)

/**
 * @brief   Disables the USART3 peripheral clock.
 *
 * @api
 */
#define rccDisableUSART3() rccDisableAPB1L(RCC_APB1LENR_USART3EN)

/**
 * @brief   Resets the USART3 peripheral.
 *
 * @api
 */
#define rccResetUSART3() rccResetAPB1L(RCC_APB1LRSTR_USART3RST)

/**
 * @brief   Enables the UART4 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableUART4(lp) rccEnableAPB1L(RCC_APB1LENR_UART4EN, lp)

/**
 * @brief   Disables the UART4 peripheral clock.
 *
 * @api
 */
#define rccDisableUART4() rccDisableAPB1L(RCC_APB1LENR_UART4EN)

/**
 * @brief   Resets the UART4 peripheral.
 *
 * @api
 */
#define rccResetUART4() rccResetAPB1L(RCC_APB1LRSTR_UART4RST)

/**
 * @brief   Enables the UART5 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableUART5(lp) rccEnableAPB1L(RCC_APB1LENR_UART5EN, lp)

/**
 * @brief   Disables the UART5 peripheral clock.
 *
 * @api
 */
#define rccDisableUART5() rccDisableAPB1L(RCC_APB1LENR_UART5EN)

/**
 * @brief   Resets the UART5 peripheral.
 *
 * @api
 */
#define rccResetUART5() rccResetAPB1L(RCC_APB1LRSTR_UART5RST)

/**
 * @brief   Enables the USART6 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableUSART6(lp) rccEnableAPB2(RCC_APB2ENR_USART6EN, lp)

/**
 * @brief   Disables the USART6 peripheral clock.
 *
 * @api
 */
#define rccDisableUSART6() rccDisableAPB2(RCC_APB2ENR_USART6EN)

/**
 * @brief   Resets the USART6 peripheral.
 *
 * @api
 */
#define rccResetUSART6() rccResetAPB2(RCC_APB2RSTR_USART6RST)

/**
 * @brief   Enables the UART7 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableUART7(lp) rccEnableAPB1L(RCC_APB1LENR_UART7EN, lp)

/**
 * @brief   Disables the UART7 peripheral clock.
 *
 * @api
 */
#define rccDisableUART7() rccDisableAPB1L(RCC_APB1LENR_UART7EN)

/**
 * @brief   Resets the UART7 peripheral.
 *
 * @api
 */
#define rccResetUART7() rccResetAPB1L(RCC_APB1LRSTR_UART7RST)

/**
 * @brief   Enables the UART8 peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableUART8(lp) rccEnableAPB1L(RCC_APB1LENR_UART8EN, lp)

/**
 * @brief   Disables the UART8 peripheral clock.
 *
 * @api
 */
#define rccDisableUART8() rccDisableAPB1L(RCC_APB1LENR_UART8EN)

/**
 * @brief   Resets the UART8 peripheral.
 *
 * @api
 */
#define rccResetUART8() rccResetAPB1L(RCC_APB1LRSTR_UART8RST)
/** @} */

/**
 * @name    LTDC peripheral specific RCC operations
 * @{
 */
/**
 * @brief   Enables the LTDC peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableLTDC(lp) rccEnableAPB3(RCC_APB3ENR_LTDCEN, lp)

/**
 * @brief   Disables the LTDC peripheral clock.
. *
 * @api
 */
#define rccDisableLTDC() rccDisableAPB3(RCC_APB3ENR_LTDCEN)

/**
 * @brief   Resets the LTDC peripheral.
 *
 * @api
 */
#define rccResetLTDC() rccResetAPB3(RCC_APB3RSTR_LTDCRST)

/**
 * @name    DMA2D peripheral specific RCC operations
 * @{
 */
/**
 * @brief   Enables the DMA2D peripheral clock.
 *
 * @param[in] lp        low power enable flag
 *
 * @api
 */
#define rccEnableDMA2D(lp) rccEnableAHB3(RCC_AHB3ENR_DMA2DEN, lp)

/**
 * @brief   Disables the DMA2D peripheral clock.
 *
 * @api
 */
#define rccDisableDMA2D() rccDisableAHB3(RCC_AHB3ENR_DMA2DEN)

/**
 * @brief   Resets the DMA2D peripheral.
 *
 * @api
 */
#define rccResetDMA2D() rccResetAHB3(RCC_AHB3RSTR_DMA2DRST)
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
#if defined(STM32_FSMC_IS_FMC)
  #define rccEnableFSMC(lp) rccEnableAHB3(RCC_AHB3ENR_FMCEN, lp)
#else
  #define rccEnableFSMC(lp) rccEnableAHB3(RCC_AHB3ENR_FSMCEN, lp)
#endif

/**
 * @brief   Disables the FSMC peripheral clock.
 *
 * @api
 */
#if defined(STM32_FSMC_IS_FMC)
  #define rccDisableFSMC() rccDisableAHB3(RCC_AHB3ENR_FMCEN)
#else
  #define rccDisableFSMC() rccDisableAHB3(RCC_AHB3ENR_FSMCEN)
#endif

/**
 * @brief   Resets the FSMC peripheral.
 *
 * @api
 */
#if defined(STM32_FSMC_IS_FMC)
  #define rccResetFSMC() rccResetAHB3(RCC_AHB3RSTR_FMCRST)
#else
  #define rccResetFSMC() rccResetAHB3(RCC_AHB3RSTR_FSMCRST)
#endif
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

#endif /* STM32_RCC_H */

/** @} */
