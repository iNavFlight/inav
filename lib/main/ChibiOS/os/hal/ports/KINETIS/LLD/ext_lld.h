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
 * @file    KINETIS/LLD/ext_lld.h
 * @brief   KINETIS EXT subsystem low level driver header.
 *
 * @addtogroup EXT
 * @{
 */

#ifndef _EXT_LLD_H_
#define _EXT_LLD_H_

#if HAL_USE_EXT || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Number of EXT channels required.
 */
#define EXT_MAX_CHANNELS    KINETIS_EXTI_NUM_CHANNELS

/**
 * @name    KINETIS-specific EXT channel modes
 * @{
 */
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   PORTA interrupt priority level setting.
 */
#if !defined(KINETIS_EXT_PORTA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define KINETIS_EXT_PORTA_IRQ_PRIORITY      3
#endif

/**
 * @brief   PORTB interrupt priority level setting.
 */
#if !defined(KINETIS_EXT_PORTB_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define KINETIS_EXT_PORTB_IRQ_PRIORITY      3
#endif

/**
 * @brief   PORTC interrupt priority level setting.
 */
#if !defined(KINETIS_EXT_PORTC_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define KINETIS_EXT_PORTC_IRQ_PRIORITY      3
#endif

/**
 * @brief   PORTD interrupt priority level setting.
 */
#if !defined(KINETIS_EXT_PORTD_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define KINETIS_EXT_PORTD_IRQ_PRIORITY       3
#endif

/**
 * @brief   PORTE interrupt priority level setting.
 */
#if !defined(KINETIS_EXT_PORTE_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define KINETIS_EXT_PORTE_IRQ_PRIORITY       3
#endif
/** @} */
/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   EXT channel identifier.
 */
typedef uint32_t expchannel_t;

/**
 * @brief   Type of an EXT generic notification callback.
 *
 * @param[in] extp      pointer to the @p EXPDriver object triggering the
 *                      callback
 */
typedef void (*extcallback_t)(EXTDriver *extp, expchannel_t channel);

/**
 * @brief   Channel configuration structure.
 */
typedef struct {
  /**
   * @brief Channel mode.
   */
  uint32_t              mode;
  /**
   * @brief Channel callback.
   */
  extcallback_t         cb;

  /**
   * @brief Port.
   */
  PORT_TypeDef          *port;

  /**
   * @brief Pin.
   */
  uint32_t              pin;
} EXTChannelConfig;

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  /**
   * @brief Channel configurations.
   */
  EXTChannelConfig      channels[EXT_MAX_CHANNELS];
  /* End of the mandatory fields.*/
} EXTConfig;

/**
 * @brief   Structure representing an EXT driver.
 */
struct EXTDriver {
  /**
   * @brief Driver state.
   */
  extstate_t                state;
  /**
   * @brief Current configuration data.
   */
  const EXTConfig           *config;
  /* End of the mandatory fields.*/
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(__DOXYGEN__)
extern EXTDriver EXTD1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void ext_lld_init(void);
  void ext_lld_start(EXTDriver *extp);
  void ext_lld_stop(EXTDriver *extp);
  void ext_lld_channel_enable(EXTDriver *extp, expchannel_t channel);
  void ext_lld_channel_disable(EXTDriver *extp, expchannel_t channel);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_EXT */

#endif /* _EXT_LLD_H_ */

/** @} */
