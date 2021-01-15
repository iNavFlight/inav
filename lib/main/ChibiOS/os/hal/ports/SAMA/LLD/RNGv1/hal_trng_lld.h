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
 * @file    hal_trng_lld.h
 * @brief   SAMA TRNG subsystem low level driver header.
 *
 * @addtogroup TRNG
 * @{
 */

#ifndef HAL_TRNG_LLD_H
#define HAL_TRNG_LLD_H

#if (HAL_USE_TRNG == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    SAMA configuration options
 * @{
 */
/**
 * @brief   TRNGD0 driver enable switch.
 * @details If set to @p TRUE the support for TRNGD0 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SAMA_TRNG_USE_TRNG0) || defined(__DOXYGEN__)
#define SAMA_TRNG_USE_TRNG0                 FALSE
#endif

/**
 * @brief   TRNGD0 data available timeout counter.
 * @details Number of status register fetches before failing.
 */
#if !defined(SAMA_DATA_FETCH_ATTEMPTS) || defined(__DOXYGEN__)
#define SAMA_DATA_FETCH_ATTEMPTS            1000
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !SAMA_TRNG_USE_TRNG0
#error "TRNG driver activated but no RNG peripheral assigned"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Low level fields of the TRNG configuration structure.
 */
#define trng_lld_config_fields                                              \
  /* Dummy configuration, it is not needed.*/                               \
  uint32_t                   dummy

/**
 * @brief   Low level fields of the TRNG driver structure.
 */
#define trng_lld_driver_fields                                              \
  /* Pointer to the RNG registers block.*/                                  \
  Trng                       *trng;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (SAMA_TRNG_USE_TRNG0 == TRUE) && !defined(__DOXYGEN__)
extern TRNGDriver TRNGD0;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void trng_lld_init(void);
  void trng_lld_start(TRNGDriver *trngp);
  void trng_lld_stop(TRNGDriver *trngp);
  bool trng_lld_generate(TRNGDriver *trngp, size_t size, uint8_t *out);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_TRNG == TRUE */

#endif /* HAL_TRNG_LLD_H */

/** @} */
