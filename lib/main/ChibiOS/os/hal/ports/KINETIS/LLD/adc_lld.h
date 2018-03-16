/*
    ChibiOS - Copyright (C) 2014 Derek Mulcahy

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
 * @file    KINETIS/LLD/adc_lld.h
 * @brief   KINETIS ADC subsystem low level driver header.
 *
 * @addtogroup ADC
 * @{
 */

#ifndef _ADC_LLD_H_
#define _ADC_LLD_H_

#if HAL_USE_ADC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    Absolute Maximum Ratings
 * @{
 */
/**
 * @brief   Minimum ADC clock frequency.
 */
#define KINETIS_ADCCLK_MIN      600000

/**
 * @brief   Maximum ADC clock frequency.
 */
#define KINETIS_ADCCLK_MAX        36000000

#define ADCx_SC3_AVGS_AVERAGE_4_SAMPLES     0
#define ADCx_SC3_AVGS_AVERAGE_8_SAMPLES     1
#define ADCx_SC3_AVGS_AVERAGE_16_SAMPLES    2
#define ADCx_SC3_AVGS_AVERAGE_32_SAMPLES    3

#define ADCx_CFG1_ADIV_DIV_1                0
#define ADCx_CFG1_ADIV_DIV_2                1
#define ADCx_CFG1_ADIV_DIV_4                2
#define ADCx_CFG1_ADIV_DIV_8                3

#define ADCx_CFG1_ADIVCLK_BUS_CLOCK         0
#define ADCx_CFG1_ADIVCLK_BUS_CLOCK_DIV_2   1
#define ADCx_CFG1_ADIVCLK_BUS_ALTCLK        2
#define ADCx_CFG1_ADIVCLK_BUS_ADACK         3

#define ADCx_CFG1_MODE_8_OR_9_BITS          0
#define ADCx_CFG1_MODE_12_OR_13_BITS        1
#define ADCx_CFG1_MODE_10_OR_11_BITS        2
#define ADCx_CFG1_MODE_16_BITS              3

#define ADCx_SC1n_ADCH_DAD0             0
#define ADCx_SC1n_ADCH_DAD1             1
#define ADCx_SC1n_ADCH_DAD2             2
#define ADCx_SC1n_ADCH_DAD3             3
#define ADCx_SC1n_ADCH_DADP0            0
#define ADCx_SC1n_ADCH_DADP1            1
#define ADCx_SC1n_ADCH_DADP2            2
#define ADCx_SC1n_ADCH_DADP3            3
#define ADCx_SC1n_ADCH_AD4              4
#define ADCx_SC1n_ADCH_AD5              5
#define ADCx_SC1n_ADCH_AD6              6
#define ADCx_SC1n_ADCH_AD7              7
#define ADCx_SC1n_ADCH_AD8              8
#define ADCx_SC1n_ADCH_AD9              9
#define ADCx_SC1n_ADCH_AD10             10
#define ADCx_SC1n_ADCH_AD11             11
#define ADCx_SC1n_ADCH_AD12             12
#define ADCx_SC1n_ADCH_AD13             13
#define ADCx_SC1n_ADCH_AD14             14
#define ADCx_SC1n_ADCH_AD15             15
#define ADCx_SC1n_ADCH_AD16             16
#define ADCx_SC1n_ADCH_AD17             17
#define ADCx_SC1n_ADCH_AD18             18
#define ADCx_SC1n_ADCH_AD19             19
#define ADCx_SC1n_ADCH_AD20             20
#define ADCx_SC1n_ADCH_AD21             21
#define ADCx_SC1n_ADCH_AD22             22
#define ADCx_SC1n_ADCH_AD23             23
#define ADCx_SC1n_ADCH_TEMP_SENSOR      26
#define ADCx_SC1n_ADCH_BANDGAP          27
#define ADCx_SC1n_ADCH_VREFSH           29
#define ADCx_SC1n_ADCH_VREFSL           30
#define ADCx_SC1n_ADCH_DISABLED         31

#define ADC_DAD0                        (1 << ADCx_SC1n_ADCH_DAD0)
#define ADC_DAD1                        (1 << ADCx_SC1n_ADCH_DAD1)
#define ADC_DAD2                        (1 << ADCx_SC1n_ADCH_DAD2)
#define ADC_DAD3                        (1 << ADCx_SC1n_ADCH_DAD3)
#define ADC_DADP0                       (1 << ADCx_SC1n_ADCH_DADP0)
#define ADC_DADP1                       (1 << ADCx_SC1n_ADCH_DADP1)
#define ADC_DADP2                       (1 << ADCx_SC1n_ADCH_DADP2)
#define ADC_DADP3                       (1 << ADCx_SC1n_ADCH_DADP3)
#define ADC_AD4                         (1 << ADCx_SC1n_ADCH_AD4)
#define ADC_AD5                         (1 << ADCx_SC1n_ADCH_AD5)
#define ADC_AD6                         (1 << ADCx_SC1n_ADCH_AD6)
#define ADC_AD7                         (1 << ADCx_SC1n_ADCH_AD7)
#define ADC_AD8                         (1 << ADCx_SC1n_ADCH_AD8)
#define ADC_AD9                         (1 << ADCx_SC1n_ADCH_AD9)
#define ADC_AD10                        (1 << ADCx_SC1n_ADCH_AD10)
#define ADC_AD11                        (1 << ADCx_SC1n_ADCH_AD11)
#define ADC_AD12                        (1 << ADCx_SC1n_ADCH_AD12)
#define ADC_AD13                        (1 << ADCx_SC1n_ADCH_AD13)
#define ADC_AD14                        (1 << ADCx_SC1n_ADCH_AD14)
#define ADC_AD15                        (1 << ADCx_SC1n_ADCH_AD15)
#define ADC_AD16                        (1 << ADCx_SC1n_ADCH_AD16)
#define ADC_AD17                        (1 << ADCx_SC1n_ADCH_AD17)
#define ADC_AD18                        (1 << ADCx_SC1n_ADCH_AD18)
#define ADC_AD19                        (1 << ADCx_SC1n_ADCH_AD19)
#define ADC_AD20                        (1 << ADCx_SC1n_ADCH_AD20)
#define ADC_AD21                        (1 << ADCx_SC1n_ADCH_AD21)
#define ADC_AD22                        (1 << ADCx_SC1n_ADCH_AD22)
#define ADC_AD23                        (1 << ADCx_SC1n_ADCH_AD23)
#define ADC_TEMP_SENSOR                 (1 << ADCx_SC1n_ADCH_TEMP_SENSOR)
#define ADC_BANDGAP                     (1 << ADCx_SC1n_ADCH_BANDGAP)
#define ADC_VREFSH                      (1 << ADCx_SC1n_ADCH_VREFSH)
#define ADC_VREFSL                      (1 << ADCx_SC1n_ADCH_VREFSL)
#define ADC_DISABLED                    (1 << ADCx_SC1n_ADCH_DISABLED)

/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */

/**
 * @brief   ADC1 driver enable switch.
 * @details If set to @p TRUE the support for ADC1 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(KINETIS_ADC_USE_ADC0) || defined(__DOXYGEN__)
#define KINETIS_ADC_USE_ADC0                FALSE
#endif

/**
 * @brief   ADC interrupt priority level setting.
 */
#if !defined(KINETIS_ADC_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define KINETIS_ADC_IRQ_PRIORITY            5
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if KINETIS_ADC_USE_ADC0 && !KINETIS_HAS_ADC0
#error "ADC1 not present in the selected device"
#endif

#if !KINETIS_ADC_USE_ADC0
#error "ADC driver activated but no ADC peripheral assigned"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

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
  ADC_ERR_DMAFAILURE = 0,                   /**< DMA operations failure.    */
  ADC_ERR_OVERFLOW = 1                      /**< ADC overflow condition.    */
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
  /**
   * @brief   Bitmask of channels for ADC conversion.
   */
  uint32_t                  channel_mask;
  /**
   * @brief   ADC CFG1 register initialization data.
   * @note    All the required bits must be defined into this field.
   */
  uint32_t                  cfg1;
  /**
   * @brief   ADC SC3 register initialization data.
   * @note    All the required bits must be defined into this field.
   */
  uint32_t                  sc3;
} ADCConversionGroup;

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  /* Perform first time calibration */
  bool                      calibrate;
} ADCConfig;

/**
 * @brief   Structure representing an ADC driver.
 */
struct ADCDriver {
  /**
   * @brief Driver state.
   */
  adcstate_t                state;
  /**
   * @brief Current configuration data.
   */
  const ADCConfig           *config;
  /**
   * @brief Current samples buffer pointer or @p NULL.
   */
  adcsample_t               *samples;
  /**
   * @brief Current samples buffer depth or @p 0.
   */
  size_t                    depth;
  /**
   * @brief Current conversion group pointer or @p NULL.
   */
  const ADCConversionGroup  *grpp;
#if ADC_USE_WAIT || defined(__DOXYGEN__)
  /**
   * @brief Waiting thread.
   */
  thread_reference_t        thread;
#endif
#if ADC_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
  /**
   * @brief Mutex protecting the peripheral.
   */
  mutex_t                   mutex;
#endif /* ADC_USE_MUTUAL_EXCLUSION */
#if defined(ADC_DRIVER_EXT_FIELDS)
  ADC_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief Pointer to the ADCx registers block.
   */
  ADC_TypeDef               *adc;
  /**
   * @brief Number of samples expected.
   */
  size_t                    number_of_samples;
  /**
   * @brief Current position in the buffer.
   */
  size_t                    current_index;
  /**
   * @brief Current channel index into group channel_mask.
   */
  size_t                    current_channel;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if KINETIS_ADC_USE_ADC0 && !defined(__DOXYGEN__)
extern ADCDriver ADCD1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void adc_lld_init(void);
  void adc_lld_start(ADCDriver *adcp);
  void adc_lld_stop(ADCDriver *adcp);
  void adc_lld_start_conversion(ADCDriver *adcp);
  void adc_lld_stop_conversion(ADCDriver *adcp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_ADC */

#endif /* _ADC_LLD_H_ */

/** @} */
