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
 * @file    SAMA5D2x/aic.h
 * @brief   SAMA AIC support macros and structures.
 *
 * @addtogroup SAMA5D2x_AIC
 * @{
 */

#ifndef AIC_H
#define AIC_H

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/
/**
 * @name    INTERRUPT SOURCE TYPE mode macros
 * @{
 */
/**
 * @brief   High-level sensitive for internal source.
 *          Low-level sensitive for external source.
 */
#define INT_LEVEL_SENSITIVE                     0x0U

/**
 * @brief   Negative-edge triggered for external source.
 */
#define EXT_NEGATIVE_EDGE                       0x1U

/**
 * @brief   High-level sensitive for internal source.
 *          High-level sensitive for external source.
 */
#define EXT_HIGH_LEVEL                          0x2U

/**
 * @brief   Positive-edge triggered for external source.
 */
#define EXT_POSITIVE_EDGE                       0x3U
/** @} */

/**
 * @brief   AIC unique redirect key.
 */
#define AIC_REDIR_KEY                       0x5B6C0E26U

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
 * @brief   Acknowledge the current interrupt.
 */
#if SAMA_HAL_IS_SECURE
#define aicAckInt() {                                                     \
  SAIC->AIC_EOICR = AIC_EOICR_ENDIT;                                      \
}
#else
#define aicAckInt() {                                                     \
  AIC->AIC_EOICR = AIC_EOICR_ENDIT;                                       \
}
#endif

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void aicInit(void);
  void aicSetSourcePriority(uint32_t source, uint8_t priority);
  void aicSetIntSourceType(uint32_t source, uint8_t type);
  void aicSetSourceHandler(uint32_t source, bool (*handler)(void));
  void aicSetSpuriousHandler(bool (*handler)(void));
  void aicEnableInt(uint32_t source);
  void aicDisableInt(uint32_t source);
  void aicClearInt(uint32_t source);
  void aicSetInt(uint32_t source);
#ifdef __cplusplus
}
#endif

#endif /* AIC_H */

/** @} */
