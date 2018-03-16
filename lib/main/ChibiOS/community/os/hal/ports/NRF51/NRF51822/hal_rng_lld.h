/*
    RNG for ChibiOS - Copyright (C) 2016 Stephane D'Alu

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
 * @file    NRF51/NRF51822/rng_lld.h
 * @brief   NRF51 RNG subsystem low level driver header.
 *
 * @addtogroup RNG
 * @{
 */

#ifndef HAL_RNG_LLD_H
#define HAL_RNG_LLD_H

#if (HAL_USE_RNG == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   RNG1 driver enable switch.
 * @details If set to @p TRUE the support for RNG1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(NRF51_RNG_USE_RNG1) || defined(__DOXYGEN__)
#define NRF51_RNG_USE_RNG1                  FALSE
#endif

/**
 * @brief   RNG1 driver enable switch.
 * @details If set to @p TRUE the support for RNG1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(NRF51_RNG_USE_RNG1) || defined(__DOXYGEN__)
#define NRF51_RNG_USE_POWER_ON_WRITE        FALSE
#endif

/**
 * @brief   RNG1 interrupt priority level setting.
 */
#if !defined(NRF51_RNG_RNG1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define NRF51_RNG_RNG1_IRQ_PRIORITY         3
#endif


/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if NRF51_RNG_USE_RNG1 &&					\
    !OSAL_IRQ_IS_VALID_PRIORITY(NRF51_RNG_RNG1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to RNG1"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a structure representing an RNG driver.
 */
typedef struct RNGDriver RNGDriver;

/**
 * @brief   Driver configuration structure.
 */
typedef struct {
  /* End of the mandatory fields.*/
  /**
   * @brief   Activate the digital error correction
   *
   * @details A digital corrector algorithm is employed to remove any
   *          bias toward '1' or '0'. Disabling it offers a substantial
   *          speed advantage, but may result in a statistical distribution
   *          that is not perfectly uniform.
   *
   * @note    On average, it take 167µs to get a byte without digitial
   *          error correction and 677µs with, but no garantee is made
   *          on the necessary time to generate one byte.
   */
  uint8_t digital_error_correction:1;
  /**
   * @brief   Only power the RNG device when requeting random bytes
   *
   * @details Device will not be powered when started/stopped 
   *          but only when writint bytes.
   */
  uint8_t power_on_write:1;
} RNGConfig;


/**
 * @brief   Structure representing an RNG driver.
 */
struct RNGDriver {
  /**
   * @brief Driver state.
   */
  rngstate_t                state;
  /**
   * @brief Current configuration data.
   */
  const RNGConfig           *config;
#if RNG_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
  /**
   * @brief   Mutex protecting the peripheral.
   */
  mutex_t                   mutex;
#endif /* RNG_USE_MUTUAL_EXCLUSION */
  /* End of the mandatory fields.*/
  /**
   * @brief Pointer to the RNGx registers block.
   */
  NRF_RNG_Type             *rng;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if NRF51_RNG_USE_RNG1 && !defined(__DOXYGEN__)
extern RNGDriver RNGD1;
#endif /* NRF51_RNG_USE_RNG1 */

#ifdef __cplusplus
extern "C" {
#endif
  void rng_lld_init(void);
  void rng_lld_start(RNGDriver *rngp);
  void rng_lld_stop(RNGDriver *rngp);
  msg_t rng_lld_write(RNGDriver *rngp, uint8_t *buf, size_t n,
                      systime_t timeout);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_RNG */

#endif /* HAL_RNG_LLD_H */

/** @} */
