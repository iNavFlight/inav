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
 * @file    ext.h
 * @brief   EXT Driver macros and structures.
 *
 * @addtogroup EXT
 * @{
 */

#ifndef _EXT_H_
#define _EXT_H_

#if (HAL_USE_EXT == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    EXT channel modes
 * @{
 */
#define EXT_CH_MODE_EDGES_MASK      3U  /**< @brief Mask of edges field.    */
#define EXT_CH_MODE_DISABLED        0U  /**< @brief Channel disabled.       */
#define EXT_CH_MODE_RISING_EDGE     1U  /**< @brief Rising edge callback.   */
#define EXT_CH_MODE_FALLING_EDGE    2U  /**< @brief Falling edge callback.  */
#define EXT_CH_MODE_BOTH_EDGES      3U  /**< @brief Both edges callback.    */

#define EXT_CH_MODE_AUTOSTART       4U  /**< @brief Channel started
                                             automatically on driver start. */
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  EXT_UNINIT = 0,                   /**< Not initialized.                   */
  EXT_STOP = 1,                     /**< Stopped.                           */
  EXT_ACTIVE = 2                    /**< Active.                            */
} extstate_t;

/**
 * @brief   Type of a structure representing a EXT driver.
 */
typedef struct EXTDriver EXTDriver;

#include "ext_lld.h"

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    Macro Functions
 * @{
 */
/**
 * @brief   Enables an EXT channel.
 *
 * @param[in] extp      pointer to the @p EXTDriver object
 * @param[in] channel   channel to be enabled
 *
 * @iclass
 */
#define extChannelEnableI(extp, channel) ext_lld_channel_enable(extp, channel)

/**
 * @brief   Disables an EXT channel.
 *
 * @param[in] extp      pointer to the @p EXTDriver object
 * @param[in] channel   channel to be disabled
 *
 * @iclass
 */
#define extChannelDisableI(extp, channel) ext_lld_channel_disable(extp, channel)

/**
 * @brief   Changes the operation mode of a channel.
 * @note    This function attempts to write over the current configuration
 *          structure that must have been not declared constant. This
 *          violates the @p const qualifier in @p extStart() but it is
 *          intentional. This function cannot be used if the configuration
 *          structure is declared @p const.
 *
 * @param[in] extp      pointer to the @p EXTDriver object
 * @param[in] channel   channel to be changed
 * @param[in] extcp     new configuration for the channel
 *
 * @api
 */
#define extSetChannelMode(extp, channel, extcp) {                           \
  osalSysLock();                                                            \
  extSetChannelModeI(extp, channel, extcp);                                 \
  osalSysUnlock();                                                          \
}

/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void extInit(void);
  void extObjectInit(EXTDriver *extp);
  void extStart(EXTDriver *extp, const EXTConfig *config);
  void extStop(EXTDriver *extp);
  void extChannelEnable(EXTDriver *extp, expchannel_t channel);
  void extChannelDisable(EXTDriver *extp, expchannel_t channel);
  void extSetChannelModeI(EXTDriver *extp,
                          expchannel_t channel,
                          const EXTChannelConfig *extcp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_EXT == TRUE */

#endif /* _EXT_H_ */

/** @} */
