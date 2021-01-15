/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

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
/*
*/

#ifndef HAL_ICU_LLD_H
#define HAL_ICU_LLD_H

#if (HAL_USE_ICU == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/
/**
 * @brief   Number of ICU channels per ICU driver.
 */
#define ICU_CHANNELS                            2 /* max channels */
#define ICU_WAIT_TIMEOUT						( 0xFFFF ) /* first edge wait timeout */

#define ICU_FREQUENCY_16MHZ  16000000 /** @brief   16MHz */
#define ICU_FREQUENCY_8MHZ    8000000 /** @brief    8MHz */
#define ICU_FREQUENCY_4MHZ    4000000 /** @brief    4MHz */
#define ICU_FREQUENCY_2MHZ    2000000 /** @brief    2MHz */
#define ICU_FREQUENCY_1MHZ    1000000 /** @brief    1MHz */
#define ICU_FREQUENCY_500KHZ   500000 /** @brief  500kHz */
#define ICU_FREQUENCY_250KHZ   250000 /** @brief  250kHz */
#define ICU_FREQUENCY_125KHZ   125000 /** @brief  125kHz */
#define ICU_FREQUENCY_62500HZ   62500 /** @brief 62500Hz */
#define ICU_FREQUENCY_31250HZ   31250 /** @brief 31250Hz */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   ICUD1 driver enable switch.
 * @details If set to @p TRUE the support for ICUD1 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(NRF5_ICU_USE_TIMER0) || defined(__DOXYGEN__)
#define NRF5_ICU_USE_TIMER0               FALSE
#endif

/**
 * @brief   ICUD2 driver enable switch.
 * @details If set to @p TRUE the support for ICUD2 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(NRF5_ICU_USE_TIMER1) || defined(__DOXYGEN__)
#define NRF5_ICU_USE_TIMER1                FALSE
#endif

/**
 * @brief   ICUD3 driver enable switch.
 * @details If set to @p TRUE the support for ICUD3 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(NRF5_ICU_USE_TIMER2) || defined(__DOXYGEN__)
#define NRF5_ICU_USE_TIMER2                FALSE
#endif

/**
 * @brief   ICUD4 driver enable switch.
 * @details If set to @p TRUE the support for ICUD4 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(NRF5_ICU_USE_TIMER3) || defined(__DOXYGEN__)
#define NRF5_ICU_USE_TIMER3                FALSE
#endif

/**
 * @brief   ICUD5 driver enable switch.
 * @details If set to @p TRUE the support for ICUD5 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(NRF5_ICU_USE_TIMER4) || defined(__DOXYGEN__)
#define NRF5_ICU_USE_TIMER4                FALSE
#endif

/**
 * @brief   ICUD1 interrupt priority level setting.
 */
#if !defined(NRF5_ICU_TIMER0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define NRF5_ICU_TIMER0_IRQ_PRIORITY         3
#endif

/**
 * @brief   ICUD2 interrupt priority level setting.
 */
#if !defined(NRF5_ICU_TIMER1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define NRF5_ICU_TIMER1_IRQ_PRIORITY         3
#endif

/**
 * @brief   ICUD3 interrupt priority level setting.
 */
#if !defined(NRF5_ICU_TIMER2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define NRF5_ICU_TIMER2_IRQ_PRIORITY         3
#endif

/**
 * @brief   ICUD4 interrupt priority level setting.
 */
#if !defined(NRF5_ICU_TIMER3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define NRF5_ICU_TIMER3_IRQ_PRIORITY         3
#endif

/**
 * @brief   ICUD5 interrupt priority level setting.
 */
#if !defined(NRF5_ICU_TIMER4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define NRF5_ICU_TIMER4_IRQ_PRIORITY         3
#endif

/**
 * @brief   Allow driver to use GPIOTE/PPI to capture PAL line
 */
#if !defined(NRF5_ICU_USE_GPIOTE_PPI)
#define NRF5_ICU_USE_GPIOTE_PPI               TRUE
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !NRF5_ICU_USE_TIMER0  && !NRF5_ICU_USE_TIMER1  &&                     \
    !NRF5_ICU_USE_TIMER2  && !NRF5_ICU_USE_TIMER3  &&                     \
    !NRF5_ICU_USE_TIMER4
#error "ICU driver activated but no TIMER peripheral assigned"
#endif

#if NRF5_ICU_USE_TIMER0 &&                                                 \
    !OSAL_IRQ_IS_VALID_PRIORITY(NRF5_ICU_TIMER0_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIMER0"
#endif

#if NRF5_ICU_USE_TIMER1 &&                                                 \
    !OSAL_IRQ_IS_VALID_PRIORITY(NRF5_ICU_TIMER1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIMER1"
#endif

#if NRF5_ICU_USE_TIMER2 &&                                                 \
    !OSAL_IRQ_IS_VALID_PRIORITY(NRF5_ICU_TIMER2_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIMER2"
#endif

#if NRF5_ICU_USE_TIMER3 &&                                                 \
    !OSAL_IRQ_IS_VALID_PRIORITY(NRF5_ICU_TIMER3_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIMER3"
#endif

#if NRF5_ICU_USE_TIMER4 &&                                                  \
    !OSAL_IRQ_IS_VALID_PRIORITY(NRF5_ICU_TIMER4_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIMER4"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/
/**
 * @brief   Active level selector.
 */
typedef enum {
  ICU_INPUT_DISABLED,              /**< Channel disabled      .            */
  ICU_INPUT_ACTIVE_HIGH,           /**< Trigger on rising edge.            */
  ICU_INPUT_ACTIVE_LOW,            /**< Trigger on falling edge.           */
} icumode_t;

/**
 * @brief   ICU channel state.
 */
typedef enum {
  ICU_CH_IDLE = 0,                     /**< Not initialized.                   */
  ICU_CH_ACTIVE = 1                    /**< First front detected.              */
} icuchannelstate_t;

/**
 * @brief   ICU frequency type.
 */
typedef uint32_t icufreq_t;

/**
 * @brief   ICU channel type.
 */
typedef enum {
  ICU_CHANNEL_1 = 0,              /**< Use TIMERx channel 0,2 */
  ICU_CHANNEL_2 = 1,              /**< Use TIMERx channel 1,3 */
} icuchannel_t;

/**
 * @brief   ICU counter type.
 */
typedef uint32_t icucnt_t;

/** 
 * @brief ICU captured width and (or) period.
 */
typedef struct {
  /**
   * @brief   Pulse width.
   */
  icucnt_t               width;
  /**
   * @brief   Pulse period.
   */
  icucnt_t               period;
} icuresult_t;

/**
 * @brief ICU Capture Channel Config structure definition.
 */
typedef struct {
  /**
   * @brief   Specifies the channel capture mode.
   */
  icumode_t       mode;

#if NRF5_ICU_USE_GPIOTE_PPI || defined(__DOXYGEN__)
  /**
   * @brief PAL line to capture.
   * @note  When NRF5_ICU_USE_GPIOTE_PPI is used and channel enabled,
   *        it wont be possible to access this PAL line using the PAL
   *        driver.
   */
  ioline_t ioline[2];

  /**
   * @brief Unique GPIOTE channel to use. (2 channel)
   * @note  Only 8 GPIOTE channels are available on nRF52.
   */
  uint8_t gpiote_channel[2];

  /**
   * @brief Unique PPI channels to use. (2 channels)
   * @note  Only 20 PPI channels are available on nRF52
   *        (When Softdevice is enabled, only channels 0-7 are available)
   */
  uint8_t ppi_channel[2];
#endif
} ICUChannelConfig;

/** 
 * @brief ICU Capture Channel structure definition.
 */
typedef struct {
  /**
   * @brief   Channel state for the internal state machine.
   */
  icuchannelstate_t      state;
  /**
   * @brief   Cached value for pulse width calculation.
   */
  icucnt_t               last_active;
  /**
   * @brief   Cached value for period calculation.
   */
  icucnt_t               last_idle;
  /**
   * @brief   Pointer to Input Capture channel configuration.
   */
//  const ICUChannelConfig *config;
} ICUChannel;

/**
 * @brief ICU Config structure definition.
 */
typedef struct {
  /**
   * @brief   Specifies the Timer clock in Hz.
   */
  icufreq_t                frequency;
  /**
   * @brief   Callback for pulse width measurement.
   */
  icucallback_t            width_cb;
  /**
   * @brief   Callback for cycle period measurement.
   */
  icucallback_t            period_cb;
  /**
   * @brief   Callback for timer overflow.
   */
  icucallback_t            overflow_cb;
  /**
   * @brief   Pointer to each Input Capture channel configuration.
   * @note    A NULL parameter indicates the channel as unused. 
   * @note    In ICU mode, only Channel 1 OR Channel 2 may be used.
   */
  const ICUChannelConfig   iccfgp[ICU_CHANNELS];
} ICUConfig;

/** 
 * @brief ICU Driver structure definition
 */
struct ICUDriver {
  /**
   * @brief   NRF52 timer peripheral for Input Capture.
   */
  NRF_TIMER_Type          *timer;
  /**
   * @brief   Driver state for the internal state machine.
   */
  icustate_t              state;
  /**
   * @brief   Channels' data structures.
   */
  ICUChannel              channel[ICU_CHANNELS];
  /**
   * @brief   Timer base clock.
   */
  uint32_t                clock;
  /**
   * @brief   Pointer to configuration for the driver.
   */
  const ICUConfig         *config;
  /**
   * @brief   Period, width last value.
   */
  icuresult_t			  result;
#if defined(ICU_DRIVER_EXT_FIELDS)
  ICU_DRIVER_EXT_FIELDS
#endif
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/
/**
 * @brief   Returns the width of the latest pulse.
 * @details The pulse width is defined as number of ticks between the start
 *          edge and the stop edge.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 * @return              The number of ticks.
 *
 * @notapi
 */
#define icu_lld_get_width(icup) ((uint32_t)((icup)->result.width))

/**
 * @brief   Returns the width of the latest cycle.
 * @details The cycle width is defined as number of ticks between a start
 *          edge and the next start edge.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 * @return              The number of ticks.
 *
 * @notapi
 */
#define icu_lld_get_period(icup) ((uint32_t)((icup)->result.period))

/**
 * @brief   Check on notifications status.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 * @return              The notifications status.
 * @retval false        if notifications are not enabled.
 * @retval true         if notifications are enabled.
 *
 * @notapi
 */
#define icu_lld_are_notifications_enabled(icup) ( 1 )

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/
#if NRF5_ICU_USE_TIMER0 && !defined(__DOXYGEN__)
extern ICUDriver ICUD1;
#endif

#if NRF5_ICU_USE_TIMER1 && !defined(__DOXYGEN__)
extern ICUDriver ICUD2;
#endif

#if NRF5_ICU_USE_TIMER2 && !defined(__DOXYGEN__)
extern ICUDriver ICUD3;
#endif

#if NRF5_ICU_USE_TIMER3 && !defined(__DOXYGEN__)
extern ICUDriver ICUD4;
#endif

#if NRF5_ICU_USE_TIMER4 && !defined(__DOXYGEN__)
extern ICUDriver ICUD5;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void icu_lld_init(void);
  void icu_lld_start(ICUDriver *icup);
  void icu_lld_stop(ICUDriver *icup);
  void icu_lld_start_capture(ICUDriver *icup);
  bool icu_lld_wait_capture(ICUDriver *icup);
  void icu_lld_stop_capture(ICUDriver *icup);
  void icu_lld_enable_notifications(ICUDriver *icup);
  void icu_lld_disable_notifications(ICUDriver *icup);
  void icu_lld_serve_interrupt(ICUDriver *icup);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_ICU */

#endif /* HAL_ICU_LLD_H */
