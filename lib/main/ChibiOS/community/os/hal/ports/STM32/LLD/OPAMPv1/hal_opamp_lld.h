/*
    ChibiOS - Copyright (C) 2006..2019 Giovanni Di Sirio
              Copyright (C) 2019 Fabien Poussin (fabien.poussin (at) google's mail)

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
 * @file    STM32/opamp_lld.h
 * @brief   STM32 Operational Amplifier subsystem low level driver header.
 *
 * @addtogroup OPAMP
 * @{
 */

#ifndef HAL_OPAMP_LLD_H_
#define HAL_OPAMP_LLD_H_

#include "hal.h"

#if HAL_USE_OPAMP || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define OPAMPx_CSR_PGAGAIN_x2                    ((uint32_t)0x00000000)
#define OPAMPx_CSR_PGAGAIN_x4                    OPAMP_CSR_PGGAIN_0
#define OPAMPx_CSR_PGAGAIN_x8                    OPAMP_CSR_PGGAIN_1
#define OPAMPx_CSR_PGAGAIN_x16                   ((uint32_t)0x0000C000)
#define OPAMPx_CSR_PGAGAIN_LESS_IFB_VM0          (0b10 << 16)
#define OPAMPx_CSR_PGAGAIN_LESS_IFB_VM1          (0b11 << 16)

#define OPAMPx_CSR_PGACONNECT_GROUND             ((uint32_t)0x00000000)
#define OPAMPx_CSR_PGACONNECT_IO1                OPAMP_CSR_PGGAIN_3
#define OPAMPx_CSR_PGACONNECT_IO2                ((uint32_t)0x00030000)

#define OPAMPx_CSR_CALSEL_3P3                    ((uint32_t)0x00000000)
#define OPAMPx_CSR_CALSEL_10P                    OPAMP_CSR_CALSEL_0
#define OPAMPx_CSR_CALSEL_50P                    OPAMP_CSR_CALSEL_1
#define OPAMPx_CSR_CALSEL_90P                    OPAMP_CSR_CALSEL

#define OPAMPx_CSR_TRIM_FACTORY                  ((uint32_t)0x00000000)
#define OPAMPx_CSR_TRIM_USER                     OPAMP_CSR_USERTRIM     /*!< User trimming */

#define OPAMPx_CSR_OUTPUT_NORMAL                 ((uint32_t)0x00000000)
#define OPAMPx_CSR_OUTPUT_INVERTED               OPAMP_CSR_OUTCAL

#define OPAMPx_CSR_LOCK                          OPAMP_CSR_LOCK


#if defined(STM32F302xB) || defined(STM32F302xC) || defined(STM32F302xD) \
|| defined(STM32F302xE) || defined(STM32F302xc) || defined(STM32F302xe) \
|| defined(STM32L1XX) || defined(STM32L4XX) || defined(STM32H7XX)
#define STM32_HAS_OPAMP1 TRUE
#define STM32_HAS_OPAMP2 TRUE
#define STM32_HAS_OPAMP3 FALSE
#define STM32_HAS_OPAMP4 FALSE

#elif defined(STM32F303xB) || defined(STM32F303xC) || defined(STM32F303xE) \
|| defined(STM32F358xx) || defined(STM32F398xx)
#define STM32_HAS_OPAMP1 TRUE
#define STM32_HAS_OPAMP2 TRUE
#define STM32_HAS_OPAMP3 TRUE
#define STM32_HAS_OPAMP4 TRUE

#else
#define STM32_HAS_OPAMP1 FALSE
#define STM32_HAS_OPAMP2 FALSE
#define STM32_HAS_OPAMP3 FALSE
#define STM32_HAS_OPAMP4 FALSE
#endif


#if STM32_HAS_OPAMP1
#define OPAMP1_CSR_VPSEL_PA07       	((uint32_t)0x00000000)
#define OPAMP1_CSR_VPSEL_PA05     		OPAMP_CSR_VPSEL_0
#define OPAMP1_CSR_VPSEL_PA03     		OPAMP_CSR_VPSEL_1
#define OPAMP1_CSR_VPSEL_PA01     		OPAMP_CSR_VPSEL

#define OPAMP1_CSR_VMSEL_PC05         ((uint32_t)0x00000000)
#define OPAMP1_CSR_VMSEL_PA03     		OPAMP_CSR_VMSEL_0
#define OPAMP1_CSR_VMSEL_PGA  				OPAMP_CSR_VMSEL_1
#define OPAMP1_CSR_VMSEL_FOLWR				OPAMP_CSR_VMSEL

#define OPAMP1_CSR_VMSSEL_PC05				((uint32_t)0x00000000)
#define OPAMP1_CSR_VMSSEL_PA03				OPAMP_CSR_VMSSEL

#define OPAMP1_CSR_VPSSEL_PA07				((uint32_t)0x00000000)
#define OPAMP1_CSR_VPSSEL_PA05				OPAMP_CSR_VPSSEL_0
#define OPAMP1_CSR_VPSSEL_PA03				OPAMP_CSR_VPSSEL_1
#define OPAMP1_CSR_VPSSEL_PA01				OPAMP_CSR_VPSSEL
#endif

#if STM32_HAS_OPAMP2
#define OPAMP2_CSR_VPSEL_PD14       	((uint32_t)0x00000000)
#define OPAMP2_CSR_VPSEL_PB14   			OPAMP_CSR_VPSEL_0
#define OPAMP2_CSR_VPSEL_PB00       	OPAMP_CSR_VPSEL_1
#define OPAMP2_CSR_VPSEL_PA07     		OPAMP_CSR_VPSEL

#define OPAMP2_CSR_VMSEL_PC05     		((uint32_t)0x00000000)
#define OPAMP2_CSR_VMSEL_PA05     		OPAMP_CSR_VMSEL_0
#define OPAMP2_CSR_VMSEL_PGA		  		OPAMP_CSR_VMSEL_1
#define OPAMP2_CSR_VMSEL_FOLWR				OPAMP_CSR_VMSEL

#define OPAMP2_CSR_VMSSEL_PC05				((uint32_t)0x00000000)
#define OPAMP2_CSR_VMSSEL_PA05				OPAMP_CSR_VMSSEL

#define OPAMP2_CSR_VPSSEL_PD14				((uint32_t)0x00000000)
#define OPAMP2_CSR_VPSSEL_PB14				OPAMP_CSR_VPSSEL_0
#define OPAMP2_CSR_VPSSEL_PB00				OPAMP_CSR_VPSSEL_1
#define OPAMP2_CSR_VPSSEL_PA07				OPAMP_CSR_VPSSEL
#endif

#if STM32_HAS_OPAMP3
#define OPAMP3_CSR_VPSEL_PB13       	((uint32_t)0x00000000)
#define OPAMP3_CSR_VPSEL_PA05       	OPAMP_CSR_VPSEL_0
#define OPAMP3_CSR_VPSEL_PA01     		OPAMP_CSR_VPSEL_1
#define OPAMP3_CSR_VPSEL_PB00       	OPAMP_CSR_VPSEL

#define OPAMP3_CSR_VMSEL_PB10         ((uint32_t)0x00000000)
#define OPAMP3_CSR_VMSEL_PB02       	OPAMP_CSR_VMSEL_0
#define OPAMP3_CSR_VMSEL_PGA		  		OPAMP_CSR_VMSEL_1
#define OPAMP3_CSR_VMSEL_FOLWR				OPAMP_CSR_VMSEL

#define OPAMP3_CSR_VMSSEL_PB10				((uint32_t)0x00000000)
#define OPAMP3_CSR_VMSSEL_PB02				OPAMP_CSR_VMSSEL

#define OPAMP3_CSR_VPSSEL_PB13				((uint32_t)0x00000000)
#define OPAMP3_CSR_VPSSEL_PA05				OPAMP_CSR_VPSSEL_0
#define OPAMP3_CSR_VPSSEL_PA01				OPAMP_CSR_VPSSEL_1
#define OPAMP3_CSR_VPSSEL_PB00				OPAMP_CSR_VPSSEL
#endif

#if STM32_HAS_OPAMP4
#define OPAMP4_CSR_VPSEL_PD11         ((uint32_t)0x00000000)
#define OPAMP4_CSR_VPSEL_PB11         OPAMP_CSR_VPSEL_0
#define OPAMP4_CSR_VPSEL_PA04       	OPAMP_CSR_VPSEL_1
#define OPAMP4_CSR_VPSEL_PB13       	OPAMP_CSR_VPSEL

#define OPAMP4_CSR_VMSEL_PB10       	((uint32_t)0x00000000)
#define OPAMP4_CSR_VMSEL_PD08       	OPAMP_CSR_VMSEL_0
#define OPAMP4_CSR_VMSEL_PGA	  			OPAMP_CSR_VMSEL_1
#define OPAMP4_CSR_VMSEL_FOLWR				OPAMP_CSR_VMSEL

#define OPAMP4_CSRVMSSEL_PB10				((uint32_t)0x00000000)
#define OPAMP4_CSR_VMSSEL_PD08				OPAMP_CSR_VMSSEL

#define OPAMP4_CSR_VPSSEL_PD11				((uint32_t)0x00000000)
#define OPAMP4_CSR_VPSSEL_PB11				OPAMP_CSR_VPSSEL_0
#define OPAMP4_CSR_VPSSEL_PA04				OPAMP_CSR_VPSSEL_1
#define OPAMP4_CSR_VPSSEL_PB13				OPAMP_CSR_VPSSEL
#endif


/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */

/**
 * @brief   OPAMPD1 driver enable switch.
 * @details If set to @p TRUE the support for OPAMPD1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_OPAMP_USE_OPAMP1) || defined(__DOXYGEN__)
#define STM32_OPAMP_USE_OPAMP1                  FALSE
#endif

/**
 * @brief   OPAMPD2 driver enable switch.
 * @details If set to @p TRUE the support for OPAMPD2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_OPAMP_USE_OPAMP2) || defined(__DOXYGEN__)
#define STM32_OPAMP_USE_OPAMP2                  FALSE
#endif

/**
 * @brief   OPAMPD3 driver enable switch.
 * @details If set to @p TRUE the support for OPAMPD3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_OPAMP_USE_OPAMP3) || defined(__DOXYGEN__)
#define STM32_OPAMP_USE_OPAMP3                  FALSE
#endif

/**
 * @brief   OPAMPD4 driver enable switch.
 * @details If set to @p TRUE the support for OPAMPD4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_OPAMP_USE_OPAMP4) || defined(__DOXYGEN__)
#define STM32_OPAMP_USE_OPAMP4                  FALSE
#endif

/**
 * @brief   OPAMPD TRIM and CALIBRATION enable switch.
 * @details If set to @p TRUE the support for USER_TRIM is included and calibration is done @init
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_OPAMP_USER_TRIM_ENABLED) || defined(__DOXYGEN__)
#define STM32_OPAMP_USER_TRIM_ENABLED		TRUE
#endif


/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/


#if STM32_OPAMP_USE_OPAMP1 && !STM32_HAS_OPAMP1
#error "OPAMP1 not present in the selected device"
#endif

#if STM32_OPAMP_USE_OPAMP2 && !STM32_HAS_OPAMP2
#error "OPAMP2 not present in the selected device"
#endif

#if STM32_OPAMP_USE_OPAMP3 && !STM32_HAS_OPAMP3
#error "OPAMP3 not present in the selected device"
#endif

#if STM32_OPAMP_USE_OPAMP4 && !STM32_HAS_OPAMP4
#error "OPAMP4 not present in the selected device"
#endif

#if !STM32_OPAMP_USE_OPAMP1 && !STM32_OPAMP_USE_OPAMP2 &&                         \
    !STM32_OPAMP_USE_OPAMP3 && !STM32_OPAMP_USE_OPAMP4
#error "OPAMP driver activated but no OPAMP peripheral assigned"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  /**
   * @brief OPAMP CSR register initialization data.
   * @note  The value of this field should normally be equal to zero.
   */
  uint32_t                  csr;
} OPAMPConfig;

/**
 * @brief   Structure representing an OPAMP driver.
 */
struct OPAMPDriver {
  /**
   * @brief Driver state.
   */
  opampstate_t                state;
  /**
   * @brief Current configuration data.
   */
  const OPAMPConfig           *config;
#if defined(OPAMP_DRIVER_EXT_FIELDS)
  OPAMP_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief Pointer to the OPAMPx registers block.
   */
  OPAMP_TypeDef               *opamp;

#if STM32_OPAMP_USER_TRIM_ENABLED
  uint16_t			trim_p;
  uint16_t			trim_n;
#endif
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if STM32_OPAMP_USE_OPAMP1 && !defined(__DOXYGEN__)
extern OPAMPDriver OPAMPD1;
#endif

#if STM32_OPAMP_USE_OPAMP2 && !defined(__DOXYGEN__)
extern OPAMPDriver OPAMPD2;
#endif

#if STM32_OPAMP_USE_OPAMP3 && !defined(__DOXYGEN__)
extern OPAMPDriver OPAMPD3;
#endif

#if STM32_OPAMP_USE_OPAMP4 && !defined(__DOXYGEN__)
extern OPAMPDriver OPAMPD4;
#endif


#ifdef __cplusplus
extern "C" {
#endif
  void opamp_lld_init(void);
  void opamp_lld_start(OPAMPDriver *compp);
  void opamp_lld_stop(OPAMPDriver *compp);
  void opamp_lld_enable(OPAMPDriver *compp);
  void opamp_lld_disable(OPAMPDriver *compp);
#if STM32_OPAMP_USER_TRIM_ENABLED
  void opamp_lld_calibrate(void);
#endif
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_OPAMP */

#endif /* _opamp_lld_H_ */

/** @} */
