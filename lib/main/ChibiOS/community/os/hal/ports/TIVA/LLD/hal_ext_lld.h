/*
    Copyright (C) 2014..2016 Marco Veeneman

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
 * @file    Tiva/ext_lld.h
 * @brief   Tiva EXT subsystem low level driver header.
 *
 * @addtogroup EXT
 * @{
 */

#ifndef HAL_EXT_LLD_H
#define HAL_EXT_LLD_H

#if HAL_USE_EXT || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Number of EXT per port.
 */
#define EXT_MAX_CHANNELS    TIVA_GPIO_PINS

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   GPIOA interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOA_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOB interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOB_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOB_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOC interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOC_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOC_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOD interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOD_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOD_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOE interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOE_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOE_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOF interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOF_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOF_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOG interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOG_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOG_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOH interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOH_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOH_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOJ interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOJ_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOJ_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOK interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOK_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOK_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOL interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOL_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOL_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOM interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOM_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOM_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPION interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPION_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPION_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOP0 interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOP0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOP0_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOP1 interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOP1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOP1_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOP2 interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOP2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOP2_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOP3 interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOP3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOP3_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOP4 interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOP4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOP4_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOP5 interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOP5_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOP5_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOP6 interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOP6_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOP6_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOP7 interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOP7_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOP7_IRQ_PRIORITY        3
#endif
/** @} */

/**
 * @brief   GPIOQ0 interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOQ0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOQ0_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOQ1 interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOQ1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOQ1_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOQ2 interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOQ2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOQ2_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOQ3 interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOQ3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOQ3_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOQ4 interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOQ4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOQ4_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOQ5 interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOQ5_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOQ5_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOQ6 interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOQ6_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOQ6_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOQ7 interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOQ7_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOQ7_IRQ_PRIORITY        3
#endif

/**
 * @brief   GPIOR interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOR_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOR_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOS interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOS_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOS_IRQ_PRIORITY         3
#endif

/**
 * @brief   GPIOT interrupt priority level setting.
 */
#if !defined(TIVA_EXT_GPIOT_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_EXT_GPIOT_IRQ_PRIORITY         3
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if TIVA_HAS_GPIOA &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOA_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOA"
#endif

#if TIVA_HAS_GPIOB &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOB_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOB"
#endif

#if TIVA_HAS_GPIOC &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOC_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOC"
#endif

#if TIVA_HAS_GPIOD &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOD_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOD"
#endif

#if TIVA_HAS_GPIOE &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOE_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOE"
#endif

#if TIVA_HAS_GPIOF &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOF_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOF"
#endif

#if TIVA_HAS_GPIOG &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOG_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOG"
#endif

#if TIVA_HAS_GPIOH &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOH_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOH"
#endif

#if TIVA_HAS_GPIOJ &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOJ_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOJ"
#endif

#if TIVA_HAS_GPIOK &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOK_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOK"
#endif

#if TIVA_HAS_GPIOL &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOL_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOL"
#endif

#if TIVA_HAS_GPIOM &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOM_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOM"
#endif

#if TIVA_HAS_GPION &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPION_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPION"
#endif

#if TIVA_HAS_GPIOP0 &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOP0_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOP0"
#endif

#if TIVA_HAS_GPIOP1 &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOP1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOP1"
#endif

#if TIVA_HAS_GPIOP2 &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOP2_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOP2"
#endif

#if TIVA_HAS_GPIOP3 &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOP3_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOP3"
#endif

#if TIVA_HAS_GPIOP4 &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOP4_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOP4"
#endif

#if TIVA_HAS_GPIOP5 &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOP5_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOP5"
#endif

#if TIVA_HAS_GPIOP6 &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOP6_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOP6"
#endif

#if TIVA_HAS_GPIOP7 &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOP7_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOP7"
#endif

#if TIVA_HAS_GPIOQ0 &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOQ0_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOQ0"
#endif

#if TIVA_HAS_GPIOQ1 &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOQ1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOQ1"
#endif

#if TIVA_HAS_GPIOQ2 &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOQ2_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOQ2"
#endif

#if TIVA_HAS_GPIOQ3 &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOQ3_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOQ3"
#endif

#if TIVA_HAS_GPIOQ4 &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOQ4_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOQ4"
#endif

#if TIVA_HAS_GPIOQ5 &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOQ5_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOQ5"
#endif

#if TIVA_HAS_GPIOQ6 &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOQ6_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOQ6"
#endif

#if TIVA_HAS_GPIOQ7 &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOQ7_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOQ7"
#endif

#if TIVA_HAS_GPIOR &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOR_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOR"
#endif

#if TIVA_HAS_GPIOS &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOS_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOS"
#endif

#if TIVA_HAS_GPIOT &&                                                         \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_EXT_GPIOT_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to GPIOT"
#endif

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

#endif /* HAL_EXT_LLD_H */

/** @} */
