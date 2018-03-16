/*
    ChibiOS - Copyright (C) 2015 Michael D. Spradling

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
 * @file    crcsw.h
 * @brief   CRC software driver.
 *
 * @addtogroup CRC
 * @{
 */

#include "hal.h"

#if HAL_USE_CRC || defined(__DOXYGEN__)

#if CRCSW_USE_CRC1 || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   CRC1 software driver enable switch.
 * @details If set to @p TRUE the support for CRC1 is included.
 * @note    The default is @p FALSE
 */
#if !defined(CRCSW_USE_CRC1) || defined(__DOXYGEN__)
#define CRCSW_USE_CRC1                  FALSE
#endif

/**
 * @brief Enables software CRC32
 */
#if !defined(CRCSW_CRC32_TABLE) || defined(__DOXYGEN__)
#define CRCSW_CRC32_TABLE               FALSE
#endif

/**
 * @brief Enables software CRC16
 */
#if !defined(CRCSW_CRC16_TABLE) || defined(__DOXYGEN__)
#define CRCSW_CRC16_TABLE               FALSE
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if CRCSW_USE_CRC1 && CRC_USE_DMA
#error "Software CRC does not support DMA(CRC_USE_DMA)"
#endif

#if CRCSW_CRC32_TABLE == FALSE && CRCSW_CRC16_TABLE == FALSE &&                         \
    CRCSW_PROGRAMMABLE == FALSE
#error "At least one of CRCSW_PROGRAMMABLE, CRCSW_CRC32_TABLE, or CRCSW_CRC16_TABLE must be defined"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/
/**
 * @brief   Type of a structure representing an CRC driver.
 */
typedef struct CRCDriver CRCDriver;

/**
 * @brief   Driver configuration structure.
 */
typedef struct {
  /**
   * @brief The size of polynomial to be used for CRC.
   */
  uint32_t                 poly_size;
  /**
   * @brief The coefficients of the polynomial to be used for CRC.
   */
  uint32_t                 poly;
  /**
   * @brief The inital value
   */
  uint32_t                 initial_val;
  /**
   * @brief The final XOR value
   */
  uint32_t                 final_val;
  /**
   * @brief Reflect bit order data going into CRC
   */
  bool                     reflect_data;
  /**
   * @brief Reflect bit order of final remainder
   */
  bool                     reflect_remainder;
  /* End of the mandatory fields.*/
  /**
   * @brief The crc lookup table to use when calculating CRC.
   */
  const uint32_t           *table;
} CRCConfig;


/**
 * @brief   Structure representing an CRC driver.
 */
struct CRCDriver {
  /**
   * @brief Driver state.
   */
  crcstate_t                state;
  /**
   * @brief Current configuration data.
   */
  const CRCConfig           *config;
#if CRC_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
  /**
   * @brief   Mutex protecting the peripheral.
   */
  mutex_t                   mutex;
#endif /* CRC_USE_MUTUAL_EXCLUSION */
  /* End of the mandatory fields.*/
  /**
   * @brief Current value of calculated CRC.
   */
  uint32_t                  crc;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/
#if CRCSW_CRC32_TABLE || defined(__DOXYGEN__)
/**
 * @brief Configuration that represents CRC32
 */
#define CRCSW_CRC32_TABLE_CONFIG (&crcsw_crc32_config)
#endif

#if CRCSW_CRC16_TABLE || defined(__DOXYGEN__)
/**
 * @brief Configuration that represents CRC16
 */
#define CRCSW_CRC16_TABLE_CONFIG (&crcsw_crc16_config)
#endif

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern CRCDriver CRCD1;

#if CRCSW_CRC32_TABLE
extern const CRCConfig crcsw_crc32_config;
#endif

#if CRCSW_CRC16_TABLE
extern const CRCConfig crcsw_crc16_config;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void crc_lld_init(void);
  void crc_lld_start(CRCDriver *crcp);
  void crc_lld_stop(CRCDriver *crcp);
  void crc_lld_reset(CRCDriver *crcp);
  uint32_t crc_lld_calc(CRCDriver *crcp, size_t n, const void *buf);
#ifdef __cplusplus
}
#endif

#endif /* CRCSW_USE_CRC1 */

#endif /* HAL_USE_CRC */

/** @} */
