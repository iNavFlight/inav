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
 * @file    MSP430X/hal_lld.h
 * @brief   MSP430X HAL subsystem low level driver header.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef _HAL_LLD_H_
#define _HAL_LLD_H_

#include "hal_dma_lld.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Defines the support for realtime counters in the HAL.
 */
/* someday*/
#define HAL_IMPLEMENTS_COUNTERS FALSE

/**
 * @name    Platform identification macros
 * @{
 */
#define PLATFORM_NAME           "MSP430X"
/** @} */

#define MSP430X_LFXTCLK   0
#define MSP430X_VLOCLK    1
#define MSP430X_LFMODCLK  2
#define MSP430X_DCOCLK    3
#define MSP430X_MODCLK    4
#define MSP430X_HFXTCLK   5

#if !defined(MSP430X_LFXTCLK_FREQ)
  #define MSP430X_LFXTCLK_FREQ 32768
  #warning "LFXTCLK freqency not defined - assuming 32768 Hz"
#endif
#define MSP430X_VLOCLK_FREQ 10000
#define MSP430X_MODCLK_FREQ 5000000
#define MSP430X_LFMODCLK_FREQ (MSP430X_MODCLK_FREQ/128)
#if !defined(MSP430X_DCOCLK_FREQ)
  #define MSP430X_DCOCLK_FREQ 8000000
  #warning "DCOCLK frequency not defined - assuming 8 MHz"
#endif
#if !defined(MSP430X_HFXTCLK_FREQ)
  #define MSP430X_HFXTCLK_FREQ 0
  #warning "HFXTCLK frequency not defined - assuming disabled"
#endif

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    MSP430X configuration options
 * @{
 */

/* Clock dividers */
#if !defined(MSP430X_ACLK_DIV)
  #define MSP430X_ACLK_DIV 1
#endif
#if !defined(MSP430X_MCLK_DIV)
  #define MSP430X_MCLK_DIV 8
#endif
#if !defined(MSP430X_SMCLK_DIV)
  #define MSP430X_SMCLK_DIV 8
#endif

/* Clock sources */
#if !defined(MSP430X_ACLK_SRC)
  #define MSP430X_ACLK_SRC MSP430X_LFXTCLK
#endif
#if !defined(MSP430X_MCLK_SRC)
  #define MSP430X_MCLK_SRC MSP430X_DCOCLK
#endif
#if !defined(MSP430X_SMCLK_SRC)
  #define MSP430X_SMCLK_SRC MSP430X_DCOCLK
#endif

/* HFXT and LFXT settings */
#if !defined(MSP430X_LFXTCLK_BYPASS)
  #define MSP430X_LFXTCLK_BYPASS 0
#endif
#if !defined(MSP430X_LFXTCLK_DRIVE)
  #define MSP430X_LFXTCLK_DRIVE 3
#endif
#if !defined(MSP430X_HFXTCLK_BYPASS)
  #define MSP430X_HFXTCLK_BYPASS 0
#endif
#if !defined(MSP430X_HFXTCLK_DRIVE)
  #define MSP430X_HFXTCLK_DRIVE 3
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*
 * Configuration-related checks.
 */
#if (MSP430X_ACLK_SRC == MSP430X_LFXTCLK) || (MSP430X_MCLK_SRC == MSP430X_LFXTCLK) || (MSP430X_SMCLK_SRC == MSP430X_LFXTCLK)
  #define MSP430X_USE_LFXTCLK
#endif
#if (MSP430X_MCLK_SRC == MSP430X_HFXTCLK) || (MSP430X_SMCLK_SRC == MSP430X_HFXTCLK)
  #define MSP430X_USE_HFXTCLK
#endif

#if defined(MSP430X_USE_HFXTCLK) && MSP430X_HFXTCLK_FREQ == 0
  #error "HFXT requested as clock source but disabled"
#endif

/* Clock speeds */
#if (MSP430X_ACLK_SRC == MSP430X_LFXTCLK)
  #define MSP430X_ACLK_FREQ (MSP430X_LFXTCLK_FREQ / MSP430X_ACLK_DIV)
#elif (MSP430X_ACLK_SRC == MSP430X_VLOCLK)
  #define MSP430X_ACLK_FREQ (MSP430X_VLOCLK_FREQ / MSP430X_ACLK_DIV)
#elif (MSP430X_ACLK_SRC == MSP430X_LFMODCLK)
  #define MSP430X_ACLK_FREQ (MSP430X_LFMODCLK_FREQ / MSP430X_ACLK_DIV)
#else
  #error "ACLK source invalid!"
#endif
#if (MSP430X_MCLK_SRC == MSP430X_LFXTCLK)
  #define MSP430X_MCLK_FREQ (MSP430X_LFXTCLK_FREQ / MSP430X_MCLK_DIV)
#elif (MSP430X_MCLK_SRC == MSP430X_VLOCLK)
  #define MSP430X_MCLK_FREQ (MSP430X_VLOCLK_FREQ / MSP430X_MCLK_DIV)
#elif (MSP430X_MCLK_SRC == MSP430X_LFMODCLK)
  #define MSP430X_MCLK_FREQ (MSP430X_LFMODCLK_FREQ / MSP430X_MCLK_DIV)
#elif (MSP430X_MCLK_SRC == MSP430X_DCOCLK)
  #define MSP430X_MCLK_FREQ (MSP430X_DCOCLK_FREQ / MSP430X_MCLK_DIV)
#elif (MSP430X_MCLK_SRC == MSP430X_MODCLK)
  #define MSP430X_MCLK_FREQ (MSP430X_MODCLK_FREQ / MSP430X_MCLK_DIV)
#elif (MSP430X_MCLK_SRC == MSP430X_HFXTCLK)
  #define MSP430X_MCLK_FREQ (MSP430X_HFXTCLK_FREQ / MSP430X_MCLK_DIV)
#else
  #error "MCLK source invalid!"
#endif
#if (MSP430X_SMCLK_SRC == MSP430X_LFXTCLK)
  #define MSP430X_SMCLK_FREQ (MSP430X_LFXTCLK_FREQ / MSP430X_SMCLK_DIV)
#elif (MSP430X_SMCLK_SRC == MSP430X_VLOCLK)
  #define MSP430X_SMCLK_FREQ (MSP430X_VLOCLK_FREQ / MSP430X_SMCLK_DIV)
#elif (MSP430X_SMCLK_SRC == MSP430X_LFMODCLK)
  #define MSP430X_SMCLK_FREQ (MSP430X_LFMODCLK_FREQ / MSP430X_SMCLK_DIV)
#elif (MSP430X_SMCLK_SRC == MSP430X_DCOCLK)
  #define MSP430X_SMCLK_FREQ (MSP430X_DCOCLK_FREQ / MSP430X_SMCLK_DIV)
#elif (MSP430X_SMCLK_SRC == MSP430X_MODCLK)
  #define MSP430X_SMCLK_FREQ (MSP430X_MODCLK_FREQ / MSP430X_SMCLK_DIV)
#elif (MSP430X_SMCLK_SRC == MSP430X_HFXTCLK)
  #define MSP430X_SMCLK_FREQ (MSP430X_HFXTCLK_FREQ / MSP430X_SMCLK_DIV)
#else
  #error "SMCLK source invalid!"
#endif

#if !defined(MSP430X_MCUCONF)
#error "Using an incorrect mcuconf.h file, MSP430X_MCUCONF not defined"
#endif

/* HFXT-specific settings */
#if MSP430X_HFXTCLK_FREQ <= 4000000
  #define MSP430X_HFFREQ HFFREQ_0
#elif MSP430X_HFXTCLK_FREQ <= 8000000
  #define MSP430X_HFFREQ HFFREQ_1
#elif MSP430X_HFXTCLK_FREQ <= 16000000
  #define MSP430X_HFFREQ HFFREQ_2
#elif MSP430X_HFXTCLK_FREQ <= 24000000
  #define MSP430X_HFFREQ HFFREQ_3
#else
  #error "HFXT frequency too high - must be <= 24000000"
#endif

/* DCO-specific settings */
#if MSP430X_DCOCLK_FREQ == 1000000
  #define MSP430X_DCOSEL DCOFSEL_0
#elif MSP430X_DCOCLK_FREQ == 2670000
  #define MSP430X_DCOSEL DCOFSEL_1
#elif MSP430X_DCOCLK_FREQ == 3330000
  #define MSP430X_DCOSEL DCOFSEL_2
#elif MSP430X_DCOCLK_FREQ == 4000000
  #define MSP430X_DCOSEL DCOFSEL_3
#elif MSP430X_DCOCLK_FREQ == 5330000
  #define MSP430X_DCOSEL DCOFSEL_4
#elif MSP430X_DCOCLK_FREQ == 6670000
  #define MSP430X_DCOSEL DCOFSEL_5
#elif MSP430X_DCOCLK_FREQ == 8000000
  #define MSP430X_DCOSEL DCOFSEL_6
#elif MSP430X_DCOCLK_FREQ == 16000000
  #define MSP430X_DCOSEL (DCORSEL | DCOFSEL_4)
#elif MSP430X_DCOCLK_FREQ == 21000000
  #define MSP430X_DCOSEL (DCORSEL | DCOFSEL_5)
#elif MSP430X_DCOCLK_FREQ == 24000000
  #define MSP430X_DCOSEL (DCORSEL | DCOFSEL_6)
#else
  #error "DCO frequency invalid"
#endif

#if MSP430X_LFXTCLK_FREQ > 50000
  #error "LFXT frequency too high - must be <= 5000"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

#define DIVIDER(x) DIV_HELPER(x)
#define DIV_HELPER(x) DIVM__ ## x

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void hal_lld_init(void);
#ifdef __cplusplus
}
#endif

#endif /* _HAL_LLD_H_ */

/** @} */
