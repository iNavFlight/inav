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
 * @file    SPC5xx/eMIOS200_v1/spc5_emios.h
 * @brief   eMIOS200 helper driver header.
 *
 * @addtogroup SPC5xx_eMIOS200
 * @{
 */

#ifndef _SPC5_EMIOS_H_
#define _SPC5_EMIOS_H_

#if HAL_USE_ICU || HAL_USE_PWM || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define EMIOSMCR_MDIS                       (1 << 30)
#define EMIOSMCR_FRZ                        (1 << 29)
#define EMIOSMCR_GTBE                       (1 << 28)
#define EMIOSMCR_GPREN                      (1 << 26)
#define EMIOSMCR_GPRE(n)                    ((n) << 8)

#define EMIOSC_FREN                         (1 << 31)
#define EMIOSC_UCPRE(n)                     ((n) << 26)
#define EMIOSC_UCPREN                       (1 << 25)
#define EMIOSC_DMA                          (1 << 24)
#define EMIOSC_IF(n)                        ((n) << 19)
#define EMIOSC_FCK                          (1 << 18)
#define EMIOSC_FEN                          (1 << 17)
#define EMIOSC_FORCMA                       (1 << 13)
#define EMIOSC_FORCMB                       (1 << 12)
#define EMIOSC_BSL(n)                       ((n) << 9)
#define EMIOSC_EDSEL                        (1 << 8)
#define EMIOSC_EDPOL                        (1 << 7)
#define EMIOSC_MODE(n)                      ((n) << 0)

#define EMIOS_BSL_COUNTER_BUS_A             0
#define EMIOS_BSL_COUNTER_BUS_2             1
#define EMIOS_BSL_INTERNAL_COUNTER          3

#define EMIOS_CCR_MODE_GPIO_IN              0
#define EMIOS_CCR_MODE_GPIO_OUT             1
#define EMIOS_CCR_MODE_SAIC                 2
#define EMIOS_CCR_MODE_SAOC                 3
#define EMIOS_CCR_MODE_IPWM                 4
#define EMIOS_CCR_MODE_IPM                  5
#define EMIOS_CCR_MODE_DAOC_B_MATCH         6
#define EMIOS_CCR_MODE_DAOC_BOTH_MATCH      7
#define EMIOS_CCR_MODE_MC_CMS               16
#define EMIOS_CCR_MODE_MC_CME               17
#define EMIOS_CCR_MODE_MC_UP_DOWN           18
#define EMIOS_CCR_MODE_OPWMT                38
#define EMIOS_CCR_MODE_MCB_UP               80
#define EMIOS_CCR_MODE_MCB_UP_DOWN          84
#define EMIOS_CCR_MODE_OPWFMB               88
#define EMIOS_CCR_MODE_OPWMCB_TE            92
#define EMIOS_CCR_MODE_OPWMCB_LE            93
#define EMIOS_CCR_MODE_OPWMB                96

#define EMIOSS_OVR                          (1 << 31)
#define EMIOSS_OVRC                         (1 << 31)
#define EMIOSS_OVFL                         (1 << 15)
#define EMIOSS_OVFLC                        (1 << 15)
#define EMIOSS_FLAG                         (1 << 0)
#define EMIOSS_FLAGC                        (1 << 0)

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

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

void reset_emios_active_channels(void);
uint32_t get_emios_active_channels(void);;
void increase_emios_active_channels(void);
void decrease_emios_active_channels(void);
#if HAL_USE_ICU
void icu_active_emios_clock(ICUDriver *icup);
#endif
#if HAL_USE_PWM
void pwm_active_emios_clock(PWMDriver *pwmp);
#endif
void deactive_emios_clock(void);

#endif /* HAL_USE_ICU || HAL_USE_PWM */

#endif /* _SPC5_EMIOS_H_ */

/** @} */
