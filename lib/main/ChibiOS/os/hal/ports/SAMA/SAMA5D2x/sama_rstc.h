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
 * @file    SAMA5D2x/sama_rstc.h
 * @brief   SAMA RSTC helper driver header.
 *
 * @addtogroup SAMA5D2x_RSTC
 * @{
 */

#ifndef _SAMA_RSTC_
#define _SAMA_RSTC_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/
/**
 * @name    RESET SOURCE MACROS
 * @{
 */
/**
 * @brief   No access allowed.
 */
#define RSTC_GENERAL                            0x0U

/**
 * @brief   Only write access allowed.
 */
#define RSTC_WKUP                               0x1U

/**
 * @brief   Only read access allowed.
 */
#define RSTC_WDT                                0x2U

/**
 * @brief   Read and Write access allowed.
 */
#define RSTC_SOFT                               0x3U

/**
 * @brief   Read and Write access allowed.
 */
#define RSTC_USER                               0x4U
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

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/
/**
 * @name    Generic RSTC operations
 * @{
 */
/**
 * @brief     Enable/Disable the detection of a low level on the pin NRST
 *            as User Reset.
 * @param[in] enable
 */
#define rstcSetUserResetEnable(enable) {                                    \
  if (enable) {                                                             \
    RSTC->RSTC_MR |= RSTC_MR_URSTEN | RSTC_MR_KEY_PASSWD;                   \
  } else {                                                                  \
    RSTC->RSTC_MR &= ~RSTC_MR_URSTEN;                                       \
    RSTC->RSTC_MR |= RSTC_MR_KEY_PASSWD;                                    \
  }                                                                         \
}

/**
 * @brief     Enable/Disable the interrupt of a User Reset.
 * @param[in] enable
 */
#define rstcSetUserResetInterruptEnable(enable) {                           \
  if (enable) {                                                             \
    RSTC->RSTC_MR |= RSTC_MR_URSTIEN | RSTC_MR_KEY_PASSWD;                  \
  } else {                                                                  \
    RSTC->RSTC_MR &= ~RSTC_MR_URSTIEN;                                      \
    RSTC->RSTC_MR |= RSTC_MR_KEY_PASSWD;                                    \
  }                                                                         \
}

/**
 * @brief   Perform a processor and peripheral reset.
 *
 * @notapi
 */
#define rstcResetProcessorAndPeripheral() {                                 \
  RSTC->RSTC_CR = RSTC_CR_PERRST | RSTC_CR_PROCRST | RSTC_MR_KEY_PASSWD;    \
}

/**
 * @brief   Perform a processor reset.
 *
 * @notapi
 */
#define rstcResetProcessor() {                                              \
  RSTC->RSTC_CR = RSTC_CR_PROCRST | RSTC_CR_KEY_PASSWD;                     \
}

/**
 * @brief   Perform a peripheral reset.
 *
 * @notapi
 */
#define rstcResetPeripheral() {                                             \
  RSTC->RSTC_CR = RSTC_CR_PERRST | RSTC_MR_KEY_PASSWD;                      \
}

/**
 * @brief   Report the cause of the last processor reset.
 *
 * @param[out] status    Cause of the reset
 *
 * @notapi
 */
#define rstcGetStatus(status) {                                             \
  uint32_t sr = RSTC->RSTC_SR & RSTC_SR_RSTTYP_Msk;                         \
  switch (sr) {                                                             \
  case RSTC_SR_RSTTYP_GENERAL_RST:                                          \
    status = RSTC_GENERAL;                                                  \
    break;                                                                  \
  case RSTC_SR_RSTTYP_WKUP_RST:                                             \
    status = RSTC_WKUP;                                                     \
    break;                                                                  \
  case RSTC_SR_RSTTYP_WDT_RST:                                              \
    status = RSTC_WDT;                                                      \
    break;                                                                  \
  case RSTC_SR_RSTTYP_SOFT_RST:                                             \
    status = RSTC_SOFT;                                                     \
    break;                                                                  \
  case RSTC_SR_RSTTYP_USER_RST:                                             \
    status = RSTC_USER;                                                     \
    break;                                                                  \
  default:                                                                  \
    break;                                                                  \
  }                                                                         \
}

/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

#endif /* SAMA_RSTC_H */

/** @} */
