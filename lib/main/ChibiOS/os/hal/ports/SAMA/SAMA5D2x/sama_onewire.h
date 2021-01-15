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
 * @file    SAMA5D2x/sama_onewire.h
 * @brief   SAMA ONEWIRE support macros and structures.
 *
 * @addtogroup SAMA5D2x_ONEWIRE
 * @{
 */

#ifndef SAMA_ONEWIRE_LLD_H
#define SAMA_ONEWIRE_LLD_H

/**
 * @brief   Using the ONEWIRE driver.
 */
#if !defined(SAMA_USE_ONEWIRE) || defined(__DOXYGEN__)
#define SAMA_USE_ONEWIRE                        FALSE
#endif

#if (SAMA_USE_ONEWIRE) || defined(__DOXYGEN__)

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
/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  ONEW_UNINIT = 0,                   /**< Not initialized.                   */
  ONEW_STOP = 1,                     /**< Stopped.                           */
  ONEW_READY = 2,                     /**< Active.                            */
  ONEW_ACTIVE = 3                    /**< Active.                            */
} onewstate_t;

/**
 * @brief   Type of a structure representing a ONEWIRE driver.
 */
typedef struct ONEWIREDriver ONEWIREDriver;

/**
 * @brief   Driver configuration structure.
 */
typedef struct {
  /**
   * @brief   Line for the data IO
   */
  uint32_t                  line;
} ONEWIREConfig;


/**
 * @brief   Structure representing an ONEWIRE driver.
 */
struct ONEWIREDriver {
  /**
   * @brief Driver state.
   */
  onewstate_t               state;
  /**
   * @brief   Current configuration data.
   */
  const ONEWIREConfig       *config;
  /**
   * @brief Mutex protecting the bus.
   */
  mutex_t                   mutex;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern ONEWIREDriver ONEWD0;

#ifdef __cplusplus
extern "C" {
#endif
  void onewireInit(void);
  void onewireObjectInit(ONEWIREDriver *onewp);
  void onewireStart(ONEWIREDriver *onewp, const ONEWIREConfig *config);
  void onewireStop(ONEWIREDriver *onewp);
  bool onewireReset(ONEWIREDriver *onewp);
  void onewireCommand(ONEWIREDriver *onewp, uint8_t *cmdp, size_t n);
  void onewireWriteBlock(ONEWIREDriver *onewp, uint8_t *txbuf, size_t n);
  void onewireReadBlock(ONEWIREDriver *onewp, uint8_t *rxbuf, size_t n);
  void onewireAcquireBus(ONEWIREDriver *onewp);
  void onewireReleaseBus(ONEWIREDriver *onewp);
#ifdef __cplusplus
}
#endif

#endif /* SAMA_USE_ONEWIRE */

#endif /* SAMA_ONEWIRE_LLD_H */

/** @} */
