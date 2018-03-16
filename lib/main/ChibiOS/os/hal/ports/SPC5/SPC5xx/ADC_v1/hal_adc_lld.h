/*
    SPC5 HAL - Copyright (C) 2013,2014 STMicroelectronics

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
 * @file    SPC5xx/ADC_v1/hal_adc_lld.h
 * @brief   ADC Driver subsystem low level driver header.
 *
 * @addtogroup ADC
 * @{
 */

#ifndef HAL_ADC_LLD_H
#define HAL_ADC_LLD_H

#if HAL_USE_ADC || defined(__DOXYGEN__)

#include "spc5_adc.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    ADC DMA modes
 * @{
 */
#define SPC5_ADC_DMA_OFF            0
#define SPC5_ADC_DMA_ON             1
/** @} */

/**
 * @name    Analog channel identifiers
 * @{
 */
#define ADC_CHN_AN0                 0U
#define ADC_CHN_AN1                 1U
#define ADC_CHN_AN2                 2U
#define ADC_CHN_AN3                 3U
#define ADC_CHN_AN4                 4U
#define ADC_CHN_AN5                 5U
#define ADC_CHN_AN6                 6U
#define ADC_CHN_AN7                 7U
#define ADC_CHN_AN8                 8U
#define ADC_CHN_AN9                 9U
#define ADC_CHN_AN10                10U
#define ADC_CHN_AN11                11U
#define ADC_CHN_AN12                12U
#define ADC_CHN_AN13                13U
#define ADC_CHN_AN14                14U
#define ADC_CHN_AN15                15U
#define ADC_CHN_AN16                16U
#define ADC_CHN_AN17                17U
#define ADC_CHN_AN18                18U
#define ADC_CHN_AN19                19U
#define ADC_CHN_AN20                20U
#define ADC_CHN_AN21                21U
#define ADC_CHN_AN22                22U
#define ADC_CHN_AN23                23U
#define ADC_CHN_AN24                24U
#define ADC_CHN_AN25                25U
#define ADC_CHN_AN26                26U
#define ADC_CHN_AN27                27U
#define ADC_CHN_AN28                28U
#define ADC_CHN_AN29                29U
#define ADC_CHN_AN30                30U
#define ADC_CHN_AN31                31U
#define ADC_CHN_AN32                32U
#define ADC_CHN_AN33                33U
#define ADC_CHN_AN34                34U
#define ADC_CHN_AN35                35U
#define ADC_CHN_AN36                36U
#define ADC_CHN_AN37                37U
#define ADC_CHN_AN38                38U
#define ADC_CHN_AN39                39U
#define ADC_CHN_AN40                40U
#define ADC_CHN_AN41                41U
#define ADC_CHN_AN42                42U
#define ADC_CHN_AN43                43U
#define ADC_CHN_AN44                44U
#define ADC_CHN_AN45                45U
#define ADC_CHN_AN46                46U
#define ADC_CHN_AN47                47U
#define ADC_CHN_AN48                48U
#define ADC_CHN_AN49                49U
#define ADC_CHN_AN50                50U
#define ADC_CHN_AN51                51U
#define ADC_CHN_AN52                52U
#define ADC_CHN_AN53                53U
#define ADC_CHN_AN54                54U
#define ADC_CHN_AN55                55U
#define ADC_CHN_AN56                56U
#define ADC_CHN_AN57                57U
#define ADC_CHN_AN58                58U
#define ADC_CHN_AN59                59U
#define ADC_CHN_AN60                60U
#define ADC_CHN_AN61                61U
#define ADC_CHN_AN62                62U
#define ADC_CHN_AN63                63U
#define ADC_CHN_AN64                64U
#define ADC_CHN_AN65                65U
#define ADC_CHN_AN66                66U
#define ADC_CHN_AN67                67U
#define ADC_CHN_AN68                68U
#define ADC_CHN_AN69                69U
#define ADC_CHN_AN70                70U
#define ADC_CHN_AN71                71U
#define ADC_CHN_AN72                72U
#define ADC_CHN_AN73                73U
#define ADC_CHN_AN74                74U
#define ADC_CHN_AN75                75U
#define ADC_CHN_AN76                76U
#define ADC_CHN_AN77                77U
#define ADC_CHN_AN78                78U
#define ADC_CHN_AN79                79U
#define ADC_CHN_AN80                80U
#define ADC_CHN_AN81                81U
#define ADC_CHN_AN82                82U
#define ADC_CHN_AN83                83U
#define ADC_CHN_AN84                84U
#define ADC_CHN_AN85                85U
#define ADC_CHN_AN86                86U
#define ADC_CHN_AN87                87U
#define ADC_CHN_AN88                88U
#define ADC_CHN_AN89                89U
#define ADC_CHN_AN90                90U
#define ADC_CHN_AN91                91U
#define ADC_CHN_AN92                92U
#define ADC_CHN_AN93                93U
#define ADC_CHN_AN94                94U
#define ADC_CHN_AN95                95U
/** @} */

/**
 * @name    Watchdog threshold identifiers
 * @{
 */
#define ADC_WDT0                    0U
#define ADC_WDT1                    1U
#define ADC_WDT2                    2U
#define ADC_WDT3                    3U
#define ADC_WDT4                    4U
#define ADC_WDT5                    5U
#define ADC_WDT6                    6U
#define ADC_WDT7                    7U
#define ADC_WDT8                    8U
#define ADC_WDT9                    9U
#define ADC_WDT10                   10U
#define ADC_WDT11                   11U
#define ADC_WDT12                   12U
#define ADC_WDT13                   13U
#define ADC_WDT14                   14U
#define ADC_WDT15                   15U
/** @} */

/**
 * @name    ADC MCR register definitions
 * @{
 */
#define ADC_MCR_OWREN               (1U << 31)
#define ADC_MCR_WLSIDE              (1U << 30)
#define ADC_MCR_MODE                (1U << 29)
#define ADC_MCR_NSTART              (1U << 24)
#define ADC_MCR_JTRGEN              (1U << 22)
#define ADC_MCR_JEDGE               (1U << 21)
#define ADC_MCR_JSTART              (1U << 20)
#define ADC_MCR_CTUEN               (1U << 17)
#define ADC_MCR_ADCLKSEL            (1U << 8)
#define ADC_MCR_ABORTCHAIN          (1U << 7)
#define ADC_MCR_ABORT               (1U << 6)
#define ADC_MCR_ACKO                (1U << 5)
#define ADC_MCR_PWDN                (1U << 0)
/** @} */

/**
 * @name    ADC MSR register definitions
 * @{
 */
#define ADC_MSR_NSTART              (1U << 24)
#define ADC_MSR_JABORT              (1U << 23)
#define ADC_MSR_JSTART              (1U << 20)
#define ADC_MSR_CTUSTART            (1U << 16)
#define ADC_MSR_CHADDR              (1U << 9)
#define ADC_MSR_ACKO                (1U << 5)
#define ADC_MSR_ADCSTATUS           (1U << 0)
/** @} */

/**
 * @name    ADC ISR register definitions
 * @{
 */
#define ADC_ISR_EOCTU               (1U << 4)
#define ADC_ISR_JEOC                (1U << 3)
#define ADC_ISR_JECH                (1U << 2)
#define ADC_ISR_EOC                 (1U << 1)
#define ADC_ISR_ECH                 (1U << 0)
/** @} */

/**
 * @name    ADC IMR register definitions
 * @{
 */
#define ADC_IMR_MSKEOCTU            (1U << 4)
#define ADC_IMR_MSKJEOC             (1U << 3)
#define ADC_IMR_MSKJECH             (1U << 2)
#define ADC_IMR_MSKEOC              (1U << 1)
#define ADC_IMR_MSKECH              (1U << 0)
/** @} */

/**
 * @name    ADC DMAE register definitions
 * @{
 */
#define ADC_DMAE_DCLR               (1U << 1)
#define ADC_DMAE_DMAEN              (1U << 0)
/** @} */

/**
 * @name    ADC CDR register definitions
 * @{
 */
#define ADC_CDR_VALID               (1U << 19)
#define ADC_CDR_OVERW               (1U << 18)
#define ADC_CDR_RESULT              (1U << 16)
#define ADC_CDR_CDATA_LEFT          (1U << 6)
#define ADC_CDR_CDATA_RIGHT         (1U << 0)
/** @} */

/**
 * @name    ADC TRC registers definitions
 * @{
 */
#define ADC_TRC_THREN               (1U << 15)
#define ADC_TRC_THRINV              (1U << 14)
#define ADC_TRC_THROP               (1U << 13)
#define ADC_TRC_THRCH(n)            ((n) << 0)
/** @} */

/**
 * @name    ADC THRHLR registers definitions
 * @{
 */
#define ADC_THRHLR_THRH(n)          ((n) << 16)
#define ADC_THRHLR_THRL(n)          ((n) << 0)
/** @} */

/**
 * @name    ADC CWSEL registers definitions
 * @{
 */
#define ADC_CWSEL0_WSEL_CHN(n)      (((n) - 0) * 4U)
#define ADC_CWSEL1_WSEL_CHN(n)      (((n) - 8U) * 4U)
#define ADC_CWSEL2_WSEL_CHN(n)      (((n) - 16U) * 4U)
#define ADC_CWSEL3_WSEL_CHN(n)      (((n) - 24U) * 4U)
#define ADC_CWSEL4_WSEL_CHN(n)      (((n) - 32U) * 4U)
#define ADC_CWSEL5_WSEL_CHN(n)      (((n) - 40U) * 4U)
#define ADC_CWSEL6_WSEL_CHN(n)      (((n) - 48U) * 4U)
#define ADC_CWSEL7_WSEL_CHN(n)      (((n) - 56U) * 4U)
#define ADC_CWSEL8_WSEL_CHN(n)      (((n) - 64U) * 4U)
#define ADC_CWSEL9_WSEL_CHN(n)      (((n) - 72U) * 4U)
#define ADC_CWSEL10_WSEL_CHN(n)     (((n) - 80U) * 4U)
#define ADC_CWSEL11_WSEL_CHN(n)     (((n) - 88U) * 4U)
/** @} */

/**
 * @name    ADC CWENR registers definitions
 * @{
 */
#define ADC_CWENR0_CWEN_CHN(n)      (1U << (n))
#define ADC_CWENR1_CWEN_CHN(n)      (1U << ((n) - 32U))
#define ADC_CWENR2_CWEN_CHN(n)      (1U << ((n) - 64U))
/** @} */

/**
 * @name    ADC CTR registers definitions
 * @{
 */
#define ADC_CTR_INPLATCH            (1U << 15)
#define ADC_CTR_OFFSHIFT(n)         ((n) << 12)
#define ADC_CTR_INPCMP(n)           ((n) << 9)
#define ADC_CTR_INPSAMP(n)          ((n) << 0)
/** @} */

/**
 * @name    ADC WTIMR registers definitions
 * @{
 */
#if SPC5_ADC_HAS_TRC
#define ADC_WTIMR_MSKWDG0_LOW       (1U << 0)
#define ADC_WTIMR_MSKWDG0_HIGH      (1U << 4)
#define ADC_WTIMR_MSKWDG0_BOTH      (17U << 0)
#define ADC_WTIMR_MSKWDG1_LOW       (1U << 1)
#define ADC_WTIMR_MSKWDG1_HIGH      (1U << 5)
#define ADC_WTIMR_MSKWDG1_BOTH      (17U << 1)
#define ADC_WTIMR_MSKWDG2_LOW       (1U << 2)
#define ADC_WTIMR_MSKWDG2_HIGH      (1U << 6)
#define ADC_WTIMR_MSKWDG2_BOTH      (17U << 2)
#define ADC_WTIMR_MSKWDG3_LOW       (1U << 3)
#define ADC_WTIMR_MSKWDG3_HIGH      (1U << 7)
#define ADC_WTIMR_MSKWDG3_BOTH      (17U << 3)
#else
#define ADC_WTIMR_MSKWDG0_LOW       (1U << 0)
#define ADC_WTIMR_MSKWDG0_HIGH      (1U << 1)
#define ADC_WTIMR_MSKWDG0_BOTH      (3U << 0)
#define ADC_WTIMR_MSKWDG1_LOW       (1U << 2)
#define ADC_WTIMR_MSKWDG1_HIGH      (1U << 3)
#define ADC_WTIMR_MSKWDG1_BOTH      (3U << 2)
#define ADC_WTIMR_MSKWDG2_LOW       (1U << 4)
#define ADC_WTIMR_MSKWDG2_HIGH      (1U << 5)
#define ADC_WTIMR_MSKWDG2_BOTH      (3U << 4)
#define ADC_WTIMR_MSKWDG3_LOW       (1U << 6)
#define ADC_WTIMR_MSKWDG3_HIGH      (1U << 7)
#define ADC_WTIMR_MSKWDG3_BOTH      (3U << 6)
#define ADC_WTIMR_MSKWDG4_LOW       (1U << 8)
#define ADC_WTIMR_MSKWDG4_HIGH      (1U << 9)
#define ADC_WTIMR_MSKWDG4_BOTH      (3U << 8)
#define ADC_WTIMR_MSKWDG5_LOW       (1U << 10)
#define ADC_WTIMR_MSKWDG5_HIGH      (1U << 11)
#define ADC_WTIMR_MSKWDG5_BOTH      (3U << 10)
#define ADC_WTIMR_MSKWDG6_LOW       (1U << 12)
#define ADC_WTIMR_MSKWDG6_HIGH      (1U << 13)
#define ADC_WTIMR_MSKWDG6_BOTH      (3U << 12)
#define ADC_WTIMR_MSKWDG7_LOW       (1U << 14)
#define ADC_WTIMR_MSKWDG7_HIGH      (1U << 15)
#define ADC_WTIMR_MSKWDG7_BOTH      (3U << 14)
#define ADC_WTIMR_MSKWDG8_LOW       (1U << 16)
#define ADC_WTIMR_MSKWDG8_HIGH      (1U << 17)
#define ADC_WTIMR_MSKWDG8_BOTH      (3U << 16)
#define ADC_WTIMR_MSKWDG9_LOW       (1U << 18)
#define ADC_WTIMR_MSKWDG9_HIGH      (1U << 19)
#define ADC_WTIMR_MSKWDG9_BOTH      (3U << 18)
#define ADC_WTIMR_MSKWDG10_LOW      (1U << 20)
#define ADC_WTIMR_MSKWDG10_HIGH     (1U << 21)
#define ADC_WTIMR_MSKWDG10_BOTH     (3U << 20)
#define ADC_WTIMR_MSKWDG11_LOW      (1U << 22)
#define ADC_WTIMR_MSKWDG11_HIGH     (1U << 23)
#define ADC_WTIMR_MSKWDG11_BOTH     (3U << 22)
#define ADC_WTIMR_MSKWDG12_LOW      (1U << 24)
#define ADC_WTIMR_MSKWDG12_HIGH     (1U << 25)
#define ADC_WTIMR_MSKWDG12_BOTH     (3U << 24)
#define ADC_WTIMR_MSKWDG13_LOW      (1U << 26)
#define ADC_WTIMR_MSKWDG13_HIGH     (1U << 27)
#define ADC_WTIMR_MSKWDG13_BOTH     (3U << 26)
#define ADC_WTIMR_MSKWDG14_LOW      (1U << 28)
#define ADC_WTIMR_MSKWDG14_HIGH     (1U << 29)
#define ADC_WTIMR_MSKWDG14_BOTH     (3U << 28)
#define ADC_WTIMR_MSKWDG15_LOW      (1U << 30)
#define ADC_WTIMR_MSKWDG15_HIGH     (1U << 31)
#define ADC_WTIMR_MSKWDG15_BOTH     (3U << 30)
#endif
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   ADCD1 driver enable switch.
 * @details If set to @p TRUE the support for ADC0 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ADC_USE_ADC0) || defined(__DOXYGEN__)
#define SPC5_ADC_USE_ADC0                   FALSE
#endif

/**
 * @brief   ADC0 WD interrupt priority level setting.
 */
#if !defined(SPC5_ADC_ADC0_WD_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_ADC_ADC0_WD_PRIORITY           12
#endif

/**
 * @brief   ADC0 DMA IRQ priority.
 */
#if !defined(SPC5_ADC_ADC0_DMA_IRQ_PRIO) || defined(__DOXYGEN__)
#define SPC5_ADC_ADC0_DMA_IRQ_PRIO          12
#endif

/**
 * @brief   ADC0 peripheral configuration when started.
 * @note    The default configuration is 1 (always run) in run mode and
 *          2 (only halt) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_ADC_ADC0_START_PCTL) || defined(__DOXYGEN__)
#define SPC5_ADC_ADC0_START_PCTL            (SPC5_ME_PCTL_RUN(1) |          \
                                             SPC5_ME_PCTL_LP(2))
#endif

/**
 * @brief   ADC0 peripheral configuration when stopped.
 * @note    The default configuration is 0 (never run) in run mode and
 *          0 (never run) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_ADC_ADC0_STOP_PCTL) || defined(__DOXYGEN__)
#define SPC5_ADC_ADC0_STOP_PCTL             (SPC5_ME_PCTL_RUN(0) |          \
                                             SPC5_ME_PCTL_LP(0))
#endif

/**
 * @brief   ADCD2 driver enable switch.
 * @details If set to @p TRUE the support for ADC1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ADC_USE_ADC1) || defined(__DOXYGEN__)
#define SPC5_ADC_USE_ADC1                   FALSE
#endif

/**
 * @brief   ADC1 WD interrupt priority level setting.
 */
#if !defined(SPC5_ADC_ADC1_WD_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_ADC_ADC1_WD_PRIORITY           12
#endif

/**
 * @brief   ADC1 DMA IRQ priority.
 */
#if !defined(SPC5_ADC_ADC1_DMA_IRQ_PRIO) || defined(__DOXYGEN__)
#define SPC5_ADC_ADC1_DMA_IRQ_PRIO          12
#endif

/**
 * @brief   ADC1 peripheral configuration when started.
 * @note    The default configuration is 1 (always run) in run mode and
 *          2 (only halt) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_ADC_ADC1_START_PCTL) || defined(__DOXYGEN__)
#define SPC5_ADC_ADC1_START_PCTL            (SPC5_ME_PCTL_RUN(1) |          \
                                             SPC5_ME_PCTL_LP(2))
#endif

/**
 * @brief   ADC1 peripheral configuration when stopped.
 * @note    The default configuration is 0 (never run) in run mode and
 *          0 (never run) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_ADC_ADC1_STOP_PCTL) || defined(__DOXYGEN__)
#define SPC5_ADC_ADC1_STOP_PCTL             (SPC5_ME_PCTL_RUN(0) |          \
                                             SPC5_ME_PCTL_LP(0))
#endif

/**
 * @brief   Selects the DMA mode for the ADC driver.
 * @details The driver is able to work 1 only mode:
 *          - @p SPC5_ADC_DMA_OFF, the DMA is not used at all, the drivers
 *            works in a fully interrupt-driven way.
 *          - @p SPC5_ADC_DMA_ON, 1 DMA channel is used.
 *          .
 * @note    DMA modes are only possible on those platforms where a DMA
 *          controllers is present.
 */
#if !defined(SPC5_ADC_DMA_MODE) || defined(__DOXYGEN__)
#define SPC5_ADC_DMA_MODE                   SPC5_ADC_DMA_OFF
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !(SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF) &&                            \
    !(SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON)
#error "invalid SPC5_ADC_DMA_MODE selected"
#endif

#if !SPC5_HAS_EDMA && (SPC5_ADC_DMA_MODE != SPC5_ADC_DMA_OFF)
#error "ADC with DMA is not supported on this device, no DMA found"
#endif

#if !SPC5_HAS_ADC0 && SPC5_ADC_USE_ADC0
#error "ADC0 not present in the selected device"
#endif

#if !SPC5_HAS_ADC1  && SPC5_ADC_USE_ADC1
#error "ADC1 not present in the selected device"
#endif

#if !SPC5_ADC_USE_ADC0 && !SPC5_ADC_USE_ADC1
#error "ADC driver activated but no ADC peripheral assigned"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   ADC clock frequency.
 */
typedef enum {
  HALF_PERIPHERAL_SET_CLOCK_FREQUENCY = 0,  /**< ADC clock frequency is
                                                 half Peripheral Set Clock
                                                 frequency.                   */
  PERIPHERAL_SET_CLOCK_FREQUENCY = 1        /**< ADC clock frequency is equal
                                                 to Peripheral Set Clock
                                                 frequency.                   */
} adc_clock;

/**
 * @brief   ADC sample data type.
 */
typedef uint16_t adcsample_t;

/**
 * @brief   Channels number in a conversion group.
 */
typedef uint16_t adc_channels_num_t;

/**
 * @brief   Possible ADC failure causes.
 * @note    Error codes are architecture dependent and should not relied
 *          upon.
 */
typedef enum {
  ADC_ERR_DMAFAILURE = 0,                   /**< DMA operations failure.*/
  ADC_ERR_AWD = 1,                          /**< Watchdog triggered.    */
} adcerror_t;

/**
 * @brief   Type of a structure representing an ADC driver.
 */
typedef struct ADCDriver ADCDriver;

/**
 * @brief   ADC notification callback type.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object triggering the
 *                      callback
 * @param[in] buffer    pointer to the most recent samples data
 * @param[in] n         number of buffer rows available starting from @p buffer
 */
typedef void (*adccallback_t)(ADCDriver *adcp, adcsample_t *buffer, size_t n);

/**
 * @brief   ADC error callback type.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object triggering the
 *                      callback
 * @param[in] err       ADC error code
 */
typedef void (*adcerrorcallback_t)(ADCDriver *adcp, adcerror_t err);

/**
 * @brief   Conversion group configuration structure.
 * @details This implementation-dependent structure describes a conversion
 *          operation.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
typedef struct {
  /**
   * @brief   Enables the circular buffer mode for the group.
   */
  bool                      circular;
  /**
   * @brief   Number of the analog channels belonging to the conversion group.
   */
  adc_channels_num_t        num_channels;
  /**
   * @brief   Callback function associated to the group or @p NULL.
   */
  adccallback_t             end_cb;
  /**
   * @brief   Error callback or @p NULL.
   */
  adcerrorcallback_t        error_cb;
  /* End of the mandatory fields.*/
#if SPC5_ADC_HAS_TRC
  /**
   * @brief   ADC TRC registers initialization data.
   */
  uint32_t                  trc[4];
  /**
   * @brief   ADC THRHLR registers initialization data.
   */
  uint32_t                  thrhlr[4];
  /**
   * @brief   ADC WTIMR register initialization data.
   */
  uint32_t                  wtimr;
#else
  /**
   * @brief   ADC CWSEL registers initialization data.
   */
  uint32_t                  cwsel[12];
  /**
   * @brief   ADC THRHLR registers initialization data.
   */
  uint32_t                  thrhlr[16];
  /**
   * @brief   ADC CWENR registers initialization data.
   */
  uint32_t                  cwenr[3];
  /**
   * @brief   ADC WTIMR register initialization data.
   */
  uint32_t                  wtimr;
#endif
  /**
   * @brief   ADC CTR registers initialization data.
   */
  uint32_t                  ctr[3];
  /**
   * @brief   ADC Initial conversion channel.
   * @note    Only the conversion of contiguous channels is implemented.
   *          Specify initial conversion channel.
   */
  uint32_t                  init_channel;
} ADCConversionGroup;

/**
 * @brief   Driver configuration structure.
 * @note    Empty in this implementation can be ignored.
 */
typedef struct {
  uint32_t                  dummy;
} ADCConfig;

/**
 * @brief   Structure representing an ADC driver.
 */
struct ADCDriver {
  /**
   * @brief   Driver state.
   */
  volatile adcstate_t       state;
  /**
   * @brief   Current configuration data.
   */
  const ADCConfig           *config;
  /**
   * @brief   Current samples buffer pointer or @p NULL.
   */
  adcsample_t               *samples;
  /**
   * @brief   Current samples buffer depth or @p 0.
   */
  size_t                    depth;
  /**
   * @brief   Current conversion group pointer or @p NULL.
   */
  const ADCConversionGroup  *grpp;
#if ADC_USE_WAIT || defined(__DOXYGEN__)
  /**
   * @brief   Waiting thread.
   */
  thread_reference_t        thread;
#endif
#if ADC_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
  /**
   * @brief   Mutex protecting the peripheral.
   */
  mutex_t                   mutex;
#endif /* ADC_USE_MUTUAL_EXCLUSION */
#if defined(ADC_DRIVER_EXT_FIELDS)
  ADC_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/

#if (SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_ON) || defined(__DOXYGEN__)
  /**
   * @brief   EDMA channel used for the ADC.
   */
  edma_channel_t            adc_dma_channel;
#else /* SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF */
  /**
   * @brief   Memory pointer for RX operations.
   */
  adcsample_t               *rx_ptr;
  /**
   * @brief   Remaining frames to be received.
   */
  size_t                    rx_cnt;
#endif /* SPC5_ADC_DMA_MODE == SPC5_ADC_DMA_OFF */
  /**
   * @brief   ADC Analog Watchdog Threshold Interrupt Status Register.
   */
  uint32_t                  adc_awd_err;
  /**
   * @brief Pointer to the ADCx registers block.
   */
  volatile struct spc5_adc  *adc_tagp;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if SPC5_ADC_USE_ADC0 && !defined(__DOXYGEN__)
extern ADCDriver ADCD1;
#endif

#if SPC5_ADC_USE_ADC1 && !defined(__DOXYGEN__)
extern ADCDriver ADCD2;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void adc_lld_init(void);
  void adc_lld_start(ADCDriver *adcp);
  void adc_lld_stop(ADCDriver *adcp);
  void adc_lld_start_conversion(ADCDriver *adcp);
  void adc_lld_stop_conversion(ADCDriver *adcp);
  uint32_t adc_get_awd_err(ADCDriver *adcp);
  void adc_clear_awd_err(ADCDriver *adcp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_ADC */

#endif /* HAL_ADC_LLD_H */

/** @} */
