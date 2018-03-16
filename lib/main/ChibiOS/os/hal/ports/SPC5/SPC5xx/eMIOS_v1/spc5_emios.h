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
 * @file    eMIOS_v1/spc5_emios.h
 * @brief   SPC5xx low level ICU - PWM driver common header.
 *
 * @addtogroup SPC5xx_eMIOS
 * @{
 */

#ifndef _SPC5_EMIOS_H_
#define _SPC5_EMIOS_H_

#if HAL_USE_ICU || HAL_USE_PWM || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define EMIOSMCR_MDIS                       (1U << 30U)
#define EMIOSMCR_FRZ                        (1U << 29U)
#define EMIOSMCR_GTBE                       (1U << 28U)
#define EMIOSMCR_GPREN                      (1U << 26U)
#define EMIOSMCR_GPRE(n)                    ((n) << 8U)

#define EMIOSC_FREN                         (1U << 31U)
#define EMIOSC_UCPRE(n)                     ((n) << 26U)
#define EMIOSC_UCPREN                       (1U << 25U)
#define EMIOSC_DMA                          (1U << 24U)
#define EMIOSC_IF(n)                        ((n) << 19U)
#define EMIOSC_FCK                          (1U << 18U)
#define EMIOSC_FEN                          (1U << 17U)
#define EMIOSC_FORCMA                       (1U << 13U)
#define EMIOSC_FORCMB                       (1U << 12U)
#define EMIOSC_BSL(n)                       ((n) << 9U)
#define EMIOSC_EDSEL                        (1U << 8U)
#define EMIOSC_EDPOL                        (1U << 7U)
#define EMIOSC_MODE(n)                      ((n) << 0)

#define EMIOS_BSL_COUNTER_BUS_A             0
#define EMIOS_BSL_COUNTER_BUS_2             1U
#define EMIOS_BSL_INTERNAL_COUNTER          3U

#define EMIOS_CCR_MODE_GPIO_IN              0
#define EMIOS_CCR_MODE_GPIO_OUT             1U
#define EMIOS_CCR_MODE_SAIC                 2U
#define EMIOS_CCR_MODE_SAOC                 3U
#define EMIOS_CCR_MODE_IPWM                 4U
#define EMIOS_CCR_MODE_IPM                  5U
#define EMIOS_CCR_MODE_DAOC_B_MATCH         6U
#define EMIOS_CCR_MODE_DAOC_BOTH_MATCH      7U
#define EMIOS_CCR_MODE_MC_CMS               16U
#define EMIOS_CCR_MODE_MC_CME               17U
#define EMIOS_CCR_MODE_MC_UP_DOWN           18U
#define EMIOS_CCR_MODE_OPWMT                38U
#define EMIOS_CCR_MODE_MCB_UP               80U
#define EMIOS_CCR_MODE_MCB_UP_DOWN          84U
#define EMIOS_CCR_MODE_OPWFMB               88U
#define EMIOS_CCR_MODE_OPWMCB_TE            92U
#define EMIOS_CCR_MODE_OPWMCB_LE            93U
#define EMIOS_CCR_MODE_OPWMB                96U

#define EMIOSS_OVR                          (1U << 31U)
#define EMIOSS_OVRC                         (1U << 31U)
#define EMIOSS_OVFL                         (1U << 15U)
#define EMIOSS_OVFLC                        (1U << 15U)
#define EMIOSS_FLAG                         (1U << 0)
#define EMIOSS_FLAGC                        (1U << 0)

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

#if SPC5_HAS_EMIOS0
/**
 * @brief   eMIOS0 peripheral configuration when started.
 * @note    The default configuration is 1 (always run) in run mode and
 *          2 (only halt) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_EMIOS0_START_PCTL) || defined(__DOXYGEN__)
#define SPC5_EMIOS0_START_PCTL              (SPC5_ME_PCTL_RUN(1) |          \
                                             SPC5_ME_PCTL_LP(2))
#endif

/**
 * @brief   eMIOS0 peripheral configuration when stopped.
 * @note    The default configuration is 0 (never run) in run mode and
 *          0 (never run) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_EMIOS0_STOP_PCTL) || defined(__DOXYGEN__)
#define SPC5_EMIOS0_STOP_PCTL               (SPC5_ME_PCTL_RUN(0) |          \
                                             SPC5_ME_PCTL_LP(0))
#endif
#endif

#if SPC5_HAS_EMIOS1
/**
 * @brief   eMIOS1 peripheral configuration when started.
 * @note    The default configuration is 1 (always run) in run mode and
 *          2 (only halt) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_EMIOS1_START_PCTL) || defined(__DOXYGEN__)
#define SPC5_EMIOS1_START_PCTL              (SPC5_ME_PCTL_RUN(1) |  \
                                             SPC5_ME_PCTL_LP(2))
#endif

/**
 * @brief   eMIOS1 peripheral configuration when stopped.
 * @note    The default configuration is 0 (never run) in run mode and
 *          0 (never run) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_EMIOS1_STOP_PCTL) || defined(__DOXYGEN__)
#define SPC5_EMIOS1_STOP_PCTL               (SPC5_ME_PCTL_RUN(0) |  \
                                             SPC5_ME_PCTL_LP(0))
#endif
#endif

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

#if SPC5_HAS_EMIOS0
void reset_emios0_active_channels(void);
uint32_t get_emios0_active_channels(void);
void increase_emios0_active_channels(void);
void decrease_emios0_active_channels(void);
#if HAL_USE_ICU
void icu_active_emios0_clock(ICUDriver *icup);
void icu_deactive_emios0_clock(ICUDriver *icup);
#endif
#if HAL_USE_PWM
void pwm_active_emios0_clock(PWMDriver *pwmp);
void pwm_deactive_emios0_clock(PWMDriver *pwmp);
#endif
#endif
#if SPC5_HAS_EMIOS1
void reset_emios1_active_channels(void);
uint32_t get_emios1_active_channels(void);
void increase_emios1_active_channels(void);
void decrease_emios1_active_channels(void);
#if HAL_USE_ICU
void icu_active_emios1_clock(ICUDriver *icup);
void icu_deactive_emios1_clock(ICUDriver *icup);
#endif
#if HAL_USE_PWM
void pwm_active_emios1_clock(PWMDriver *pwmp);
void pwm_deactive_emios1_clock(PWMDriver *pwmp);
#endif
#endif

#endif /* HAL_USE_ICU || HAL_USE_PWM */

#endif /* _SPC5_EMIOS_H_ */

/** @} */
