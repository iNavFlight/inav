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
 * @file    SAMA5D2x/hal_tc_lld.h
 * @brief   SAMA TC subsystem low level driver header.
 *
 * @addtogroup TC
 * @{
 */

#ifndef HAL_TC_LLD_H
#define HAL_TC_LLD_H

#if HAL_USE_TC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Number of TC channels per TC driver.
 */
#define TC_CHANNELS                             TCCHANNEL_NUMBER

/**
 * @name    TC output mode macros
 * @{
 */
/**
 * @brief   Standard output modes mask.
 */
#define TC_OUTPUT_MASK                          0x0FU

/**
 * @brief   Output not driven, callback only.
 */
#define TC_OUTPUT_DISABLED                      0x00U

/**
 * @brief   Output active.
 */
#define TC_OUTPUT_ACTIVE                        0x01U

/** @} */

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  TC_UNINIT = 0,                    /**< Not initialized.                   */
  TC_STOP = 1,                      /**< Stopped.                           */
  TC_READY = 2                      /**< Ready.                             */
} tcstate_t;

/**
 * @brief   Type of a structure representing a TC driver.
 */
typedef struct TCDriver TCDriver;

/**
 * @brief   Type of a TC notification callback.
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 */
typedef void (*tccallback_t)(TCDriver *tcp);


/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   TCD0 driver enable switch.
 * @details If set to @p TRUE the support for TCD0 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(SAMA_USE_TC0) || defined(__DOXYGEN__)
#define SAMA_USE_TC0                        FALSE
#endif

/**
 * @brief   TCD1 driver enable switch.
 * @details If set to @p TRUE the support for TCD1 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(SAMA_USE_TC1) || defined(__DOXYGEN__)
#define SAMA_USE_TC1                        FALSE
#endif

/**
 * @brief   TCD0 interrupt priority level setting.
 */
#if !defined(SAMA_TC0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_TC0_IRQ_PRIORITY               2
#endif

/**
 * @brief   TCD1 interrupt priority level setting.
 */
#if !defined(SAMA_TC1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_TC1_IRQ_PRIORITY               2
#endif

/** @} */

/*===========================================================================*/
/* Configuration checks.                                                     */
/*===========================================================================*/

#if !SAMA_USE_TC0 && !SAMA_USE_TC1
#error "TC driver activated but no TC peripheral assigned"
#endif

/* Checks on allocation of TCx units.*/
#if SAMA_USE_TC0
#if defined(SAMA_TC0_IS_USED)
#error "TC0 is already used"
#else
#define SAMA_TC0_IS_USED
#endif
#endif

/* Checks on allocation of TCx units.*/
#if SAMA_USE_TC1
#if defined(SAMA_TC1_IS_USED)
#error "TC1 is already used"
#else
#define SAMA_TC1_IS_USED
#endif
#endif
/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a TC mode.
 */
typedef uint32_t tcmode_t;

/**
 * @brief   Type of a TC channel.
 */
typedef uint8_t tcchannel_t;

/**
 * @brief   Type of a channels mask.
 */
typedef uint32_t tcchnmsk_t;

/**
 * @brief   Type of a TC counter.
 */
typedef uint32_t tccnt_t;

/**
 * @brief   Type of a TC driver channel configuration structure.
 */
typedef struct {
  /**
   * @brief Channel active logic level.
   */
  tcmode_t                  mode;
  /**
   * @brief   Timer clock in Hz.
   * @note    The low level can use assertions in order to catch invalid
   *          frequency specifications.
   */
  uint32_t                  frequency;
  /**
   * @brief Channel callback pointer.
   * @note  This callback is invoked on the channel compare event. If set to
   *        @p NULL then the callback is disabled.
   */
  tccallback_t              callback;
  /* End of the mandatory fields.*/
} TCChannelConfig;

/**
 * @brief   Type of a TC driver configuration structure.
 */
typedef struct {
  /**
   * @brief Channels configurations.
   */
  TCChannelConfig           channels[TC_CHANNELS];
  /* End of the mandatory fields.*/
} TCConfig;

/**
 * @brief   Structure representing a TC driver.
 */
struct TCDriver {
  /**
   * @brief Driver state.
   */
  tcstate_t                 state;
  /**
   * @brief Current driver configuration data.
   */
  const TCConfig            *config;
  /**
   * @brief   Mask of the enabled channels.
   */
  tcchnmsk_t                enabled;
  /**
   * @brief   Number of channels in this instance.
   */
  tcchannel_t              channels;
  /* End of the mandatory fields.*/
  /**
   * @brief Timer base clock.
   */
  uint32_t                  clock;
  /**
   * @brief Pointer to the TCx registers block.
   */
  Tc                        *tim;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/
/**
 * @brief   Enables a TC channel.
 * @pre     The TC unit must have been activated using @p tcStart().
 * @post    The channel is active using the specified configuration.
 * @note    Depending on the hardware implementation this function has
 *          effect starting on the next cycle (recommended implementation)
 *          or immediately (fallback implementation).
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 * @param[in] channel   TC channel identifier (0...channels-1)
 * @param[in] width     TC pulse width as clock pulses number
 *
 * @iclass
 */
#define tcEnableChannelI(tcp, channel, width) do {                          \
  (tcp)->enabled |= ((tcchnmsk_t)1U << (tcchnmsk_t)(channel));              \
  tc_lld_enable_channel(tcp, channel, width);                               \
} while (false)

/**
 * @brief   Disables a TC channel.
 * @pre     The TC unit must have been activated using @p tcStart().
 * @post    The channel is disabled and its output line returned to the
 *          idle state.
 * @note    Depending on the hardware implementation this function has
 *          effect starting on the next cycle (recommended implementation)
 *          or immediately (fallback implementation).
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 * @param[in] channel   TC channel identifier (0...channels-1)
 *
 * @iclass
 */
#define tcDisableChannelI(tcp, channel) do {                                \
  (tcp)->enabled &= ~((tcchnmsk_t)1U << (tcchnmsk_t)(channel));             \
  tc_lld_disable_channel(tcp, channel);                                     \
} while (false)

/**
 * @brief   Returns a TC channel status.
 * @pre     The TC unit must have been activated using @p tcStart().
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 * @param[in] channel   TC channel identifier (0...channels-1)
 *
 * @iclass
 */
#define tcIsChannelEnabledI(tcp, channel)                                   \
  (((tcp)->enabled & ((tcchnmsk_t)1U << (tcchnmsk_t)(channel))) != 0U)

/**
 * @brief   Enables a channel de-activation edge notification.
 * @pre     The TC unit must have been activated using @p tcStart().
 * @pre     The channel must have been activated using @p tcEnableChannel().
 * @note    If the notification is already enabled then the call has no effect.
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 * @param[in] channel   TC channel identifier (0...channels-1)
 *
 * @iclass
 */
#define tcEnableChannelNotificationI(tcp, channel)                          \
  tc_lld_enable_channel_notification(tcp, channel)

/**
 * @brief   Disables a channel de-activation edge notification.
 * @pre     The TC unit must have been activated using @p tcStart().
 * @pre     The channel must have been activated using @p tcEnableChannel().
 * @note    If the notification is already disabled then the call has no effect.
 *
 * @param[in] tcp       pointer to a @p TCDriver object
 * @param[in] channel   TC channel identifier (0...channels-1)
 *
 * @iclass
 */
#define tcDisableChannelNotificationI(tcp, channel)                         \
  tc_lld_disable_channel_notification(tcp, channel)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if SAMA_USE_TC0 && !defined(__DOXYGEN__)
extern TCDriver TCD0;
#endif

#if SAMA_USE_TC1 && !defined(__DOXYGEN__)
extern TCDriver TCD1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void tcInit(void);
  void tcObjectInit(TCDriver *tcp);
  void tcStart(TCDriver *tcp, const TCConfig *config);
  void tcStop(TCDriver *tcp);
  void tcEnableChannel(TCDriver *tcp,
                       tcchannel_t channel,
                       tccnt_t width);
  void tcDisableChannel(TCDriver *tcp, tcchannel_t channel);
  void tcEnableChannelNotification(TCDriver *tcp, tcchannel_t channel);
  void tcDisableChannelNotification(TCDriver *tcp, tcchannel_t channel);
  void tcChangeChannelFrequency(TCDriver *tcp, tcchannel_t channel, uint32_t frequency);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_TC */

#endif /* HAL_TC_LLD_H */

/** @} */
