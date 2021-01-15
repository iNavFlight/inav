/*
    ChibiOS - Copyright (C) 2016 Andrew Wygle aka awygle

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
 * @file    hal_adc_lld.h
 * @brief   MSP430X ADC subsystem low level driver header.
 *
 * @addtogroup ADC
 * @{
 */

#ifndef HAL_ADC_LLD_H
#define HAL_ADC_LLD_H

#if (HAL_USE_ADC == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    Sampling rates
 * @{
 */
typedef enum {
  MSP430X_ADC_SHT_4   = 0x0000,
  MSP430X_ADC_SHT_8   = 0x1100,
  MSP430X_ADC_SHT_16  = 0x2200,
  MSP430X_ADC_SHT_32  = 0x3300,
  MSP430X_ADC_SHT_64  = 0x4400,
  MSP430X_ADC_SHT_96  = 0x5500,
  MSP430X_ADC_SHT_128 = 0x6600,
  MSP430X_ADC_SHT_192 = 0x7700,
  MSP430X_ADC_SHT_256 = 0x8800,
  MSP430X_ADC_SHT_384 = 0x9900,
  MSP430X_ADC_SHT_512 = 0xAA00
} MSP430XADCSampleRates;
/** @} */

/**
 * @name    Resolution
 * @{
 */
typedef enum {
  MSP430X_ADC_RES_8BIT  = 0x0000,
  MSP430X_ADC_RES_10BIT = 0x0010,
  MSP430X_ADC_RES_12BIT = 0x0020
} MSP430XADCResolution;
/** @} */

/**
 * @name    References
 * @{
 */
typedef enum {
  MSP430X_ADC_VSS_VCC             = 0x0000,
  MSP430X_ADC_VSS_VREF_BUF        = 0x0100,
  MSP430X_ADC_VSS_VEREF_N         = 0x0200,
  MSP430X_ADC_VSS_VEREF_P_BUF     = 0x0300,
  MSP430X_ADC_VSS_VEREF_P         = 0x0400,
  MSP430X_ADC_VEREF_P_BUF_VCC     = 0x0500,
  MSP430X_ADC_VEREF_P_VCC         = 0x0600,
  MSP430X_ADC_VEREF_P_VREF_BUF    = 0x0700,
  MSP430X_ADC_VREF_BUF_VCC        = 0x0900,
  MSP430X_ADC_VREF_BUF_VEREF_P    = 0x0B00,
  MSP430X_ADC_VEREF_N_VCC         = 0x0C00,
  MSP430X_ADC_VEREF_N_VREF_BUF    = 0x0D00,
  MSP430X_ADC_VEREF_N_VEREF_P     = 0x0E00,
  MSP430X_ADC_VEREF_N_VEREF_P_BUF = 0x0F00
} MSP430XADCReferences;

typedef enum {
  MSP430X_REF_1V2     = 0x0000,
  MSP430X_REF_2V0     = 0x0010,
  MSP430X_REF_2V5     = 0x0020,
  MSP430X_REF_1V2_EXT = 0x0002,
  MSP430X_REF_2V0_EXT = 0x0012,
  MSP430X_REF_2V5_EXT = 0x0022
} MSP430XREFSources;

#define MSP430X_REF_NONE MSP430X_REF_1V2

/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    MSP430X configuration options
 * @{
 */
/**
 * @brief   Stores ADC samples in an 8 bit integer.
 * @note    10 and 12 bit sampling modes must not be used when this option is
 *          enabled.
 */
#if !defined(MSP430X_ADC_COMPACT_SAMPLES) || defined(__DOXYGEN__)
#define MSP430X_ADC_COMPACT_SAMPLES FALSE
#endif

/**
 * @brief   ADC1 driver enable switch.
 * @details If set to @p TRUE the support for ADC1 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(MSP430X_ADC_USE_ADC1) || defined(__DOXYGEN__)
#define MSP430X_ADC_USE_ADC1 TRUE
#endif

/**]
 * @brief   Exclusive DMA enable switch.
 * @details If set to @p TRUE the support for exclusive DMA is included.
 * @note    This increases the size of the compiled executable somewhat.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_ADC_EXCLUSIVE_DMA) || defined(__DOXYGEN__)
#define MSP430X_ADC_EXCLUSIVE_DMA FALSE
#endif

#if MSP430X_ADC_USE_ADC1

/**
 * @brief   ADC1 clock source configuration
 */
#if !defined(MSP430X_ADC1_CLK_SRC)
#define MSP430X_ADC1_CLK_SRC MSP430X_MODCLK
#endif

#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if MSP430X_ADC_USE_ADC1

#if !defined(__MSP430_HAS_ADC12_B__)
#error "No ADC present or ADC version not supported"
#endif

#if (MSP430X_ADC1_CLK_SRC == MSP430X_MODCLK)
#define MSP430X_ADC1_CLK_FREQ MSP430X_MODCLK_FREQ
#define MSP430X_ADC1_SSEL ADC12SSEL_0
#elif (MSP430X_ADC1_CLK_SRC == MSP430X_ACLK)
#define MSP430X_ADC1_CLK_FREQ MSP430X_ACLK_FREQ
#define MSP430X_ADC1_SSEL ADC12SSEL_1
#elif (MSP430X_ADC1_CLK_SRC == MSP430X_MCLK)
#define MSP$30X_ADC1_CLK_FREQ MSP430X_MCLK_FREQ
#define MSP430X_ADC1_SSEL ADC12SSEL_2
#elif (MSP430X_ADC1_CLK_SRC == MSP430SMCLK)
#define MSP430X_ADC1_CLK_FREQ MSP430X_SMCLK_FREQ
#define MSP430X_ADC1_SSEL ADC12SSEL_3
#else
#error "Invalid ADC1 clock source requested!"
#endif

#if !defined(MSP430X_ADC1_FREQ)
#warning "ADC clock frequency not defined - assuming 1 for all dividers"
#define MSP430X_ADC1_DIV_CALC(x) (x == 1)
#else
#define MSP430X_ADC1_DIV_CALC(x)                                               \
  ((MSP430X_ADC1_CLK_FREQ / x) == MSP430X_ADC1_FREQ)
#endif

/**
 * @brief   ADC1 prescaler calculations
 */
#if MSP430X_ADC1_DIV_CALC(1)
#define MSP430X_ADC1_PDIV ADC12PDIV__1
#define MSP430X_ADC1_DIV ADC12DIV_0
#elif MSP430X_ADC1_DIV_CALC(2)
#define MSP430X_ADC1_PDIV ADC12PDIV__1
#define MSP430X_ADC1_DIV ADC12DIV_1
#elif MSP430X_ADC1_DIV_CALC(3)
#define MSP430X_ADC1_PDIV ADC12PDIV__1
#define MSP430X_ADC1_DIV ADC12DIV_2
#elif MSP430X_ADC1_DIV_CALC(4)
#define MSP430X_ADC1_PDIV ADC12PDIV__4
#define MSP430X_ADC1_DIV ADC12DIV_0
#elif MSP430X_ADC1_DIV_CALC(5)
#define MSP430X_ADC1_PDIV ADC12PDIV__1
#define MSP430X_ADC1_DIV ADC12DIV_4
#elif MSP430X_ADC1_DIV_CALC(6)
#define MSP430X_ADC1_PDIV ADC12PDIV__1
#define MSP430X_ADC1_DIV ADC12DIV_5
#elif MSP430X_ADC1_DIV_CALC(7)
#define MSP430X_ADC1_PDIV ADC12PDIV__1
#define MSP430X_ADC1_DIV ADC12DIV_6
#elif MSP430X_ADC1_DIV_CALC(8)
#define MSP430X_ADC1_PDIV ADC12PDIV__4
#define MSP430X_ADC1_DIV ADC12DIV_2
#elif MSP430X_ADC1_DIV_CALC(12)
#define MSP430X_ADC1_PDIV ADC12PDIV__4
#define MSP430X_ADC1_DIV ADC12DIV_2
#elif MSP430X_ADC1_DIV_CALC(16)
#define MSP430X_ADC1_PDIV ADC12PDIV__4
#define MSP430X_ADC1_DIV ADC12DIV_3
#elif MSP430X_ADC1_DIV_CALC(20)
#define MSP430X_ADC1_PDIV ADC12PDIV__4
#define MSP430X_ADC1_DIV ADC12DIV_4
#elif MSP430X_ADC1_DIV_CALC(24)
#define MSP430X_ADC1_PDIV ADC12PDIV__4
#define MSP430X_ADC1_DIV ADC12DIV_5
#elif MSP430X_ADC1_DIV_CALC(28)
#define MSP430X_ADC1_PDIV ADC12PDIV__4
#define MSP430X_ADC1_DIV ADC12DIV_6
#elif MSP430X_ADC1_DIV_CALC(32)
#define MSP430X_ADC1_PDIV ADC12PDIV__32
#define MSP430X_ADC1_DIV ADC12DIV_0
#elif MSP430X_ADC1_DIV_CALC(64)
#define MSP430X_ADC1_PDIV ADC12PDIV__64
#define MSP430X_ADC1_DIV ADC12DIV_0
#elif MSP430X_ADC1_DIV_CALC(96)
#define MSP430X_ADC1_PDIV ADC12PDIV__32
#define MSP430X_ADC1_DIV ADC12DIV_2
#elif MSP430X_ADC1_DIV_CALC(128)
#define MSP430X_ADC1_PDIV ADC12PDIV__64
#define MSP430X_ADC1_DIV ADC12DIV_1
#elif MSP430X_ADC1_DIV_CALC(160)
#define MSP430X_ADC1_PDIV ADC12PDIV__32
#define MSP430X_ADC1_DIV ADC12DIV_4
#elif MSP430X_ADC1_DIV_CALC(192)
#define MSP430X_ADC1_PDIV ADC12PDIV__64
#define MSP430X_ADC1_DIV ADC12DIV_2
#elif MSP430X_ADC1_DIV_CALC(224)
#define MSP430X_ADC1_PDIV ADC12PDIV__32
#define MSP430X_ADC1_DIV ADC12DIV_6
#elif MSP430X_ADC1_DIV_CALC(256)
#define MSP430X_ADC1_PDIV ADC12PDIV__64
#define MSP430X_ADC1_DIV ADC12DIV_3
#elif MSP430X_ADC1_DIV_CALC(320)
#define MSP430X_ADC1_PDIV ADC12PDIV__64
#define MSP430X_ADC1_DIV ADC12DIV_4
#elif MSP430X_ADC1_DIV_CALC(384)
#define MSP430X_ADC1_PDIV ADC12PDIV__64
#define MSP430X_ADC1_DIV ADC12DIV_5
#elif MSP430X_ADC1_DIV_CALC(448)
#define MSP430X_ADC1_PDIV ADC12PDIV__64
#define MSP430X_ADC1_DIV ADC12DIV_6
#elif MSP430X_ADC1_DIV_CALC(512)
#define MSP430X_ADC1_PDIV ADC12PDIV__64
#define MSP430X_ADC1_DIV ADC12DIV_7
#else
#error "MSP430X_ADC1_FREQ not achievable with MSP430X_ADC1_CLK_SRC"
#endif

#endif /* MSP430X_ADC_USE_ADC1 */

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   ADC sample data type.
 */
#if !MSP430X_ADC_COMPACT_SAMPLES || defined(__DOXYGEN__)
typedef uint16_t adcsample_t;
#else
typedef uint8_t adcsample_t;
#endif

/**
 * @brief   Channels number in a conversion group.
 */
typedef uint8_t adc_channels_num_t;

/**
 * @brief   Possible ADC failure causes.
 * @note    Error codes are architecture dependent and should not relied
 *          upon.
 */
typedef enum {
  ADC_ERR_UNKNOWN  = 0, /**< Unknown error has occurred */
  ADC_ERR_OVERFLOW = 1, /**< ADC overflow condition.    */
  ADC_ERR_AWD      = 2  /**< Analog watchdog triggered. */
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
typedef void (*adccallback_t)(ADCDriver * adcp, adcsample_t * buffer, size_t n);

/**
 * @brief   ADC error callback type.
 *
 * @param[in] adcp      pointer to the @p ADCDriver object triggering the
 *                      callback
 * @param[in] err       ADC error code
 */
typedef void (*adcerrorcallback_t)(ADCDriver * adcp, adcerror_t err);

/**
 * @brief MSP430X ADC register structure.
 */
typedef struct {
  uint16_t ctl[4];
  uint16_t lo;
  uint16_t hi;
  uint16_t ifgr[3];
  uint16_t ier[3];
  uint16_t iv;
  uint16_t padding[3];
  uint16_t mctl[32];
  uint16_t mem[32];
} msp430x_adc_reg_t;

/**
 * @brief MSP430X ADC calibration structure.
 */
typedef struct {
  uint16_t CAL_ADC_GAIN_FACTOR;
  uint16_t CAL_ADC_OFFSET;
  uint16_t CAL_ADC_12T30;
  uint16_t CAL_ADC_12T85;
  uint16_t CAL_ADC_20T30;
  uint16_t CAL_ADC_20T85;
  uint16_t CAL_ADC_25T30;
  uint16_t CAL_ADC_25T85;
} msp430x_adc_cal_t;

/**
 * @brief MSP430X REF calibration structure.
 */
typedef struct {
  uint16_t CAL_ADC_12VREF_FACTOR;
  uint16_t CAL_ADC_20VREF_FACTOR;
  uint16_t CAL_ADC_25VREF_FACTOR;
} msp430x_ref_cal_t;

/**
 * @brief   Conversion group configuration structure.
 * @details This implementation-dependent structure describes a conversion
 *          operation.
 * @note    The use of this configuration structure requires knowledge of
 *          MSP430X ADC cell registers interface, please refer to the MSP430X
 *          reference manual for details.
 */
typedef struct {
  /**
   * @brief   Enables the circular buffer mode for the group.
   */
  bool circular;
  /**
   * @brief   Number of the analog channels belonging to the conversion group.
   */
  adc_channels_num_t num_channels;
  /**
   * @brief   Callback function associated to the group or @p NULL.
   */
  adccallback_t end_cb;
  /**
   * @brief   Error callback or @p NULL.
   */
  adcerrorcallback_t error_cb;
  /* End of the mandatory fields.*/
  /**
   * @brief   Sequence of analog channels belonging to the conversion group.
   * @note    Only the first num_channels are valid.
   */
  uint8_t channels[32];
  /**
   * @brief   Sample resolution
   */
  MSP430XADCResolution res;
  /**
   * @brief   Sampling time in clock cycles
   */
  MSP430XADCSampleRates rate;
  /**
   * @brief   Voltage references to use
   */
  MSP430XADCReferences ref;
  /**
   * @brief   VREF source
   */
  MSP430XREFSources vref_src;
} ADCConversionGroup;

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
#if MSP430X_ADC_EXCLUSIVE_DMA == TRUE || defined(__DOXYGEN__)
  /**
   * @brief The index of the DMA channel.
   * @note  This may be >MSP430X_DMA_CHANNELS to indicate that exclusive DMA
   *        is not used.
   */
  uint8_t dma_index;
#endif
} ADCConfig;

/**
 * @brief   Structure representing an ADC driver.
 */
struct ADCDriver {
  /**
   * @brief Driver state.
   */
  adcstate_t state;
  /**
   * @brief Current configuration data.
   */
  const ADCConfig * config;
  /**
   * @brief Current samples buffer pointer or @p NULL.
   */
  adcsample_t * samples;
  /**
   * @brief Current samples buffer depth or @p 0.
   */
  size_t depth;
  /**
   * @brief Current conversion group pointer or @p NULL.
   */
  const ADCConversionGroup * grpp;
#if (ADC_USE_WAIT == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief Waiting thread.
   */
  thread_reference_t thread;
#endif
#if (ADC_USE_MUTUAL_EXCLUSION == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief Mutex protecting the peripheral.
   */
  mutex_t mutex;
#endif
#if defined(ADC_DRIVER_EXT_FIELDS)
  ADC_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief Base address of ADC12_B registers
   */
  msp430x_adc_reg_t * regs;
  /**
   * @brief DMA request structure
   */
  msp430x_dma_req_t req;
  /**
   * @brief ADC calibration structure from TLV
   */
  msp430x_adc_cal_t * adc_cal;
  /**
   * @brief REF calibration structure from TLV
   */
  msp430x_ref_cal_t * ref_cal;
  /**
   * @brief Count of times DMA callback has been called
   */
  uint8_t count;
  /**
   * @brief DMA stream
   */
  msp430x_dma_ch_t dma;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (MSP430X_ADC_USE_ADC1 == TRUE) && !defined(__DOXYGEN__)
extern ADCDriver ADCD1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
void adc_lld_init(void);
void adc_lld_start(ADCDriver * adcp);
void adc_lld_stop(ADCDriver * adcp);
void adc_lld_start_conversion(ADCDriver * adcp);
void adc_lld_stop_conversion(ADCDriver * adcp);
adcsample_t adcMSP430XAdjustResult(ADCConversionGroup * grpp,
                                   adcsample_t sample);
adcsample_t adcMSP430XAdjustTemp(ADCConversionGroup * grpp, adcsample_t sample);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_ADC == TRUE */

#endif /* HAL_ADC_LLD_H */

/** @} */
