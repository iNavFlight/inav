/*
    ChibiOS - Copyright (C) 2006..2017 Giovanni Di Sirio
              Copyright (C) 2017 Fabien Poussin (fabien.poussin (at) google's mail)

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
 * @file    STM32/comp_lld.h
 * @brief   STM32 Comparator subsystem low level driver header.
 *
 * @addtogroup COMP
 * @{
 */

#ifndef HAL_COMP_LLD_H_
#define HAL_COMP_LLD_H_

#include "hal.h"

#if HAL_USE_COMP || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/


#define STM32_COMP_InvertingInput_1_4VREFINT          ((uint32_t)0x00000000) /*!< 1/4 VREFINT connected to comparator inverting input */
#define STM32_COMP_InvertingInput_1_2VREFINT          COMP_CSR_COMPxINSEL_0  /*!< 1/2 VREFINT connected to comparator inverting input */
#define STM32_COMP_InvertingInput_3_4VREFINT          COMP_CSR_COMPxINSEL_1  /*!< 3/4 VREFINT connected to comparator inverting input */
#define STM32_COMP_InvertingInput_VREFINT             ((uint32_t)0x00000030) /*!< VREFINT connected to comparator inverting input */
#define STM32_COMP_InvertingInput_DAC1OUT1            COMP_CSR_COMPxINSEL_2  /*!< DAC1_OUT1 (PA4) connected to comparator inverting input */
#define STM32_COMP_InvertingInput_DAC1OUT2            ((uint32_t)0x00000050) /*!< DAC1_OUT2 (PA5) connected to comparator inverting input */

#define STM32_COMP_InvertingInput_IO1                 ((uint32_t)0x00000060) /*!< I/O1 (PA0 for COMP1, PA2 for COMP2, PD15 for COMP3,
                                                                            PE8 for COMP4, PD13 for COMP5, PD10 for COMP6,
                                                                            PC0 for COMP7) connected to comparator inverting input */

#define STM32_COMP_InvertingInput_IO2                 COMP_CSR_COMPxINSEL    /*!< I/O2 (PB12 for COMP3, PB2 for COMP4, PB10 for COMP5,
                                                                            PB15 for COMP6) connected to comparator inverting input.
                                                                            It is valid only for STM32F303xC devices */

#define STM32_COMP_InvertingInput_DAC2OUT1            COMP_CSR_COMPxINSEL_3  /*!< DAC2_OUT1 (PA6) connected to comparator inverting input */


#define STM32_COMP_NonInvertingInput_IO1                 ((uint32_t)0x00000000) /*!< I/O1 (PA1 for COMP1, PA7 for COMP2, PB14 for COMP3,
                                                                               PB0 for COMP4, PD12 for COMP5, PD11 for COMP6,
                                                                               PA0 for COMP7) connected to comparator non inverting input */

#define STM32_COMP_NonInvertingInput_IO2                 COMP_CSR_COMPxNONINSEL /*!< I/O2 (PA3 for COMP2, PD14 for COMP3, PE7 for COMP4, PB13 for COMP5,
                                                                               PB11 for COMP6, PC1 for COMP7) connected to comparator non inverting input */


#define STM32_COMP_Output_None                            ((uint32_t)0x00000000)   /*!< COMP output isn't connected to other peripherals */

/* Output Redirection common for all comparators COMP1...COMP7 */
#define STM32_COMP_Output_TIM1BKIN                        COMP_CSR_COMPxOUTSEL_0   /*!< COMP output connected to TIM1 Break Input (BKIN) */
#define STM32_COMP_Output_TIM1BKIN2                       ((uint32_t)0x00000800)   /*!< COMP output connected to TIM1 Break Input 2 (BKIN2) */
#define STM32_COMP_Output_TIM8BKIN                        ((uint32_t)0x00000C00)   /*!< COMP output connected to TIM8 Break Input (BKIN) */
#define STM32_COMP_Output_TIM8BKIN2                       ((uint32_t)0x00001000)   /*!< COMP output connected to TIM8 Break Input 2 (BKIN2) */
#define STM32_COMP_Output_TIM1BKIN2_TIM8BKIN2             ((uint32_t)0x00001400)   /*!< COMP output connected to TIM1 Break Input 2 and TIM8 Break Input 2 */
#define STM32_COMP_Output_TIM20BKIN                       ((uint32_t)0x00003000)   /*!< COMP output connected to TIM20 Break Input (BKIN) */
#define STM32_COMP_Output_TIM20BKIN2                      ((uint32_t)0x00003400)  /*!< COMP output connected to TIM20 Break Input 2 (BKIN2) */
#define STM32_COMP_Output_TIM1BKIN2_TIM8BKIN2_TIM20BKIN2  ((uint32_t)0x00001400)   /*!< COMP output connected to TIM1 Break Input 2, TIM8 Break Input 2 and TIM20 Break Input2 */

/* Output Redirection common for COMP1 and COMP2 */
#define STM32_COMP_Output_TIM1OCREFCLR                    ((uint32_t)0x00001800)   /*!< COMP output connected to TIM1 OCREF Clear */
#define STM32_COMP_Output_TIM1IC1                         ((uint32_t)0x00001C00)   /*!< COMP output connected to TIM1 Input Capture 1 */
#define STM32_COMP_Output_TIM2IC4                         ((uint32_t)0x00002000)   /*!< COMP output connected to TIM2 Input Capture 4 */
#define STM32_COMP_Output_TIM2OCREFCLR                    ((uint32_t)0x00002400)   /*!< COMP output connected to TIM2 OCREF Clear */
#define STM32_COMP_Output_TIM3IC1                         ((uint32_t)0x00002800)   /*!< COMP output connected to TIM3 Input Capture 1 */
#define STM32_COMP_Output_TIM3OCREFCLR                    ((uint32_t)0x00002C00)   /*!< COMP output connected to TIM3 OCREF Clear */

/* Output Redirection specific to COMP2 */
#define STM32_COMP_Output_HRTIM1_FLT6                     ((uint32_t)0x00003000)   /*!< COMP output connected to HRTIM1 FLT6 */
#define STM32_COMP_Output_HRTIM1_EE1_2                    ((uint32_t)0x00003400)   /*!< COMP output connected to HRTIM1 EE1_2*/
#define STM32_COMP_Output_HRTIM1_EE6_2                    ((uint32_t)0x00003800)   /*!< COMP output connected to HRTIM1 EE6_2 */
#define STM32_COMP_Output_TIM20OCREFCLR                   ((uint32_t)0x00003C00)   /*!< COMP output connected to TIM20 OCREF Clear */

/* Output Redirection specific to COMP3 */
#define STM32_COMP_Output_TIM4IC1                         ((uint32_t)0x00001C00)   /*!< COMP output connected to TIM4 Input Capture 1 */
#define STM32_COMP_Output_TIM3IC2                         ((uint32_t)0x00002000)   /*!< COMP output connected to TIM3 Input Capture 2 */
#define STM32_COMP_Output_TIM15IC1                        ((uint32_t)0x00002800)   /*!< COMP output connected to TIM15 Input Capture 1 */
#define STM32_COMP_Output_TIM15BKIN                       ((uint32_t)0x00002C00)   /*!< COMP output connected to TIM15 Break Input (BKIN) */

/* Output Redirection specific to COMP4 */
#define STM32_COMP_Output_TIM3IC3                         ((uint32_t)0x00001800)   /*!< COMP output connected to TIM3 Input Capture 3 */
#define STM32_COMP_Output_TIM8OCREFCLR                    ((uint32_t)0x00001C00)   /*!< COMP output connected to TIM8 OCREF Clear */
#define STM32_COMP_Output_TIM15IC2                        ((uint32_t)0x00002000)   /*!< COMP output connected to TIM15 Input Capture 2 */
#define STM32_COMP_Output_TIM4IC2                         ((uint32_t)0x00002400)   /*!< COMP output connected to TIM4 Input Capture 2 */
#define STM32_COMP_Output_TIM15OCREFCLR                   ((uint32_t)0x00002800)   /*!< COMP output connected to TIM15 OCREF Clear */

#define STM32_COMP_Output_HRTIM1_FLT7                     ((uint32_t)0x00003000)   /*!< COMP output connected to HRTIM1 FLT7 */
#define STM32_COMP_Output_HRTIM1_EE2_2                    ((uint32_t)0x00003400)   /*!< COMP output connected to HRTIM1 EE2_2*/
#define STM32_COMP_Output_HRTIM1_EE7_2                    ((uint32_t)0x00003800)   /*!< COMP output connected to HRTIM1 EE7_2 */

/* Output Redirection specific to COMP5 */
#define STM32_COMP_Output_TIM2IC1                         ((uint32_t)0x00001800)   /*!< COMP output connected to TIM2 Input Capture 1 */
#define STM32_COMP_Output_TIM17IC1                        ((uint32_t)0x00002000)   /*!< COMP output connected to TIM17 Input Capture 1 */
#define STM32_COMP_Output_TIM4IC3                         ((uint32_t)0x00002400)   /*!< COMP output connected to TIM4 Input Capture 3 */
#define STM32_COMP_Output_TIM16BKIN                       ((uint32_t)0x00002800)   /*!< COMP output connected to TIM16 Break Input (BKIN) */

/* Output Redirection specific to COMP6 */
#define STM32_COMP_Output_TIM2IC2                         ((uint32_t)0x00001800)   /*!< COMP output connected to TIM2 Input Capture 2 */
#define STM32_COMP_Output_COMP6TIM2OCREFCLR               ((uint32_t)0x00002000)   /*!< COMP output connected to TIM2 OCREF Clear */
#define STM32_COMP_Output_TIM16OCREFCLR                   ((uint32_t)0x00002400)   /*!< COMP output connected to TIM16 OCREF Clear */
#define STM32_COMP_Output_TIM16IC1                        ((uint32_t)0x00002800)   /*!< COMP output connected to TIM16 Input Capture 1 */
#define STM32_COMP_Output_TIM4IC4                         ((uint32_t)0x00002C00)   /*!< COMP output connected to TIM4 Input Capture 4 */

#define STM32_COMP_Output_HRTIM1_FLT8                     ((uint32_t)0x00003000)   /*!< COMP output connected to HRTIM1 FLT8 */
#define STM32_COMP_Output_HRTIM1_EE3_2                    ((uint32_t)0x00003400)   /*!< COMP output connected to HRTIM1 EE3_2*/
#define STM32_COMP_Output_HRTIM1_EE8_2                    ((uint32_t)0x00003800)   /*!< COMP output connected to HRTIM1 EE8_2 */

/* Output Redirection specific to COMP7 */
#define STM32_COMP_Output_TIM2IC3                         ((uint32_t)0x00002000)   /*!< COMP output connected to TIM2 Input Capture 3 */
#define STM32_COMP_Output_TIM1IC2                         ((uint32_t)0x00002400)   /*!< COMP output connected to TIM1 Input Capture 2 */
#define STM32_COMP_Output_TIM17OCREFCLR                   ((uint32_t)0x00002800)   /*!< COMP output connected to TIM16 OCREF Clear */
#define STM32_COMP_Output_TIM17BKIN                       ((uint32_t)0x00002C00)   /*!< COMP output connected to TIM16 Break Input (BKIN) */

/* No blanking source can be selected for all comparators */
#define STM32_COMP_BlankingSrce_None                   ((uint32_t)0x00000000)    /*!< No blanking source */

/* Blanking source common for COMP1, COMP2, COMP3 and COMP7 */
#define STM32_COMP_BlankingSrce_TIM1OC5                COMP_CSR_COMPxBLANKING_0  /*!< TIM1 OC5 selected as blanking source for compartor */

/* Blanking source common for COMP1 and COMP2 */
#define STM32_COMP_BlankingSrce_TIM2OC3                COMP_CSR_COMPxBLANKING_1  /*!< TIM2 OC5 selected as blanking source for compartor */

/* Blanking source common for COMP1, COMP2 and COMP5 */
#define STM32_COMP_BlankingSrce_TIM3OC3                ((uint32_t)0x000C0000)    /*!< TIM2 OC3 selected as blanking source for compartor */

/* Blanking source common for COMP3 and COMP6 */
#define STM32_COMP_BlankingSrce_TIM2OC4                ((uint32_t)0x000C0000)  /*!< TIM2 OC4 selected as blanking source for compartor */

/* Blanking source common for COMP4, COMP5, COMP6 and COMP7 */
#define STM32_COMP_BlankingSrce_TIM8OC5                COMP_CSR_COMPxBLANKING_1  /*!< TIM8 OC5 selected as blanking source for compartor */

/* Blanking source for COMP4 */
#define STM32_COMP_BlankingSrce_TIM3OC4                COMP_CSR_COMPxBLANKING_0  /*!< TIM3 OC4 selected as blanking source for compartor */
#define STM32_COMP_BlankingSrce_TIM15OC1               ((uint32_t)0x000C0000)    /*!< TIM15 OC1 selected as blanking source for compartor */

/* Blanking source common for COMP6 and COMP7 */
#define STM32_COMP_BlankingSrce_TIM15OC2               COMP_CSR_COMPxBLANKING_2    /*!< TIM15 OC2 selected as blanking source for compartor */

#define STM32_COMP_OutputPol_NonInverted              ((uint32_t)0x00000000)  /*!< COMP output on GPIO isn't inverted */
#define STM32_COMP_OutputPol_Inverted                 COMP_CSR_COMPxPOL       /*!< COMP output on GPIO is inverted */

#define STM32_COMP_Hysteresis_No                      0x00000000           /*!< No hysteresis */
#define STM32_COMP_Hysteresis_Low                     COMP_CSR_COMPxHYST_0 /*!< Hysteresis level low */
#define STM32_COMP_Hysteresis_Medium                  COMP_CSR_COMPxHYST_1 /*!< Hysteresis level medium */
#define STM32_COMP_Hysteresis_High                    COMP_CSR_COMPxHYST   /*!< Hysteresis level high */

#define STM32_COMP_Mode_HighSpeed                     0x00000000            /*!< High Speed */
#define STM32_COMP_Mode_MediumSpeed                   COMP_CSR_COMPxMODE_0  /*!< Medium Speed */
#define STM32_COMP_Mode_LowPower                      COMP_CSR_COMPxMODE_1  /*!< Low power mode */
#define STM32_COMP_Mode_UltraLowPower                 COMP_CSR_COMPxMODE    /*!< Ultra-low power mode */

/* When output polarity is not inverted, comparator output is high when
   the non-inverting input is at a higher voltage than the inverting input */
#define STM32_COMP_OutputLevel_High                   COMP_CSR_COMPxOUT
/* When output polarity is not inverted, comparator output is low when
   the non-inverting input is at a lower voltage than the inverting input*/
#define STM32_COMP_OutputLevel_Low                    ((uint32_t)0x00000000)


#if defined(STM32F301x8) || defined(STM32F302x8) || defined(STM32F303x8) \
|| defined(STM32F318xx) || defined(STM32F328xx) || defined(STM32F334x8)
#define STM32_HAS_COMP1 FALSE
#define STM32_HAS_COMP2 TRUE
#define STM32_HAS_COMP3 FALSE
#define STM32_HAS_COMP4 TRUE
#define STM32_HAS_COMP5 FALSE
#define STM32_HAS_COMP6 TRUE
#define STM32_HAS_COMP7 FALSE

#elif defined(STM32F302xc) || defined(STM32F302xe)
#define STM32_HAS_COMP1 TRUE
#define STM32_HAS_COMP2 TRUE
#define STM32_HAS_COMP3 FALSE
#define STM32_HAS_COMP4 TRUE
#define STM32_HAS_COMP5 FALSE
#define STM32_HAS_COMP6 TRUE
#define STM32_HAS_COMP7 FALSE

#elif defined(STM32F303xC) || defined(STM32F303xE) || defined(STM32F358xx) || defined(STM32F398xx)
#define STM32_HAS_COMP1 TRUE
#define STM32_HAS_COMP2 TRUE
#define STM32_HAS_COMP3 TRUE
#define STM32_HAS_COMP4 TRUE
#define STM32_HAS_COMP5 TRUE
#define STM32_HAS_COMP6 TRUE
#define STM32_HAS_COMP7 TRUE

#elif defined(STM32F373xx) || defined(STM32F378xx) || defined(STM32L0XX) || defined(STM32L1XX) \
  || defined(STM32F051x8) || defined(STM32F048xx) || defined(STM32F058xx) || defined(STM32F078xx) \
  || defined(STM32F072xb) || defined(STM32F071xb)
#define STM32_HAS_COMP1 TRUE
#define STM32_HAS_COMP2 TRUE
#define STM32_HAS_COMP3 FALSE
#define STM32_HAS_COMP4 FALSE
#define STM32_HAS_COMP5 FALSE
#define STM32_HAS_COMP6 FALSE
#define STM32_HAS_COMP7 FALSE

#else
#define STM32_HAS_COMP1 FALSE
#define STM32_HAS_COMP2 FALSE
#define STM32_HAS_COMP3 FALSE
#define STM32_HAS_COMP4 FALSE
#define STM32_HAS_COMP5 FALSE
#define STM32_HAS_COMP6 FALSE
#define STM32_HAS_COMP7 FALSE

#endif

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */

/**
 * @brief   COMP INTERRUPTS.
 * @details If set to @p TRUE the support for COMPD1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_COMP_USE_INTERRUPTS) || defined(__DOXYGEN__)
#define STM32_COMP_USE_INTERRUPTS             FALSE
#endif

/**
 * @brief   COMPD1 driver enable switch.
 * @details If set to @p TRUE the support for COMPD1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_COMP_USE_COMP1) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP1                  FALSE
#endif

/**
 * @brief   COMPD2 driver enable switch.
 * @details If set to @p TRUE the support for COMPD2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_COMP_USE_COMP2) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP2                  FALSE
#endif

/**
 * @brief   COMPD3 driver enable switch.
 * @details If set to @p TRUE the support for COMPD3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_COMP_USE_COMP3) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP3                  FALSE
#endif

/**
 * @brief   COMPD4 driver enable switch.
 * @details If set to @p TRUE the support for COMPD4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_COMP_USE_COMP4) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP4                  FALSE
#endif

/**
 * @brief   COMPD5 driver enable switch.
 * @details If set to @p TRUE the support for COMPD4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_COMP_USE_COMP5) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP5                  FALSE
#endif

/**
 * @brief   COMPD6 driver enable switch.
 * @details If set to @p TRUE the support for COMPD4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_COMP_USE_COMP6) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP6                  FALSE
#endif

/**
 * @brief   COMPD7 driver enable switch.
 * @details If set to @p TRUE the support for COMPD4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(STM32_COMP_USE_COMP7) || defined(__DOXYGEN__)
#define STM32_COMP_USE_COMP7                  FALSE
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if STM32_COMP_USE_INTERRUPTS && defined(STM32F0XX)
#error "Interrupts are shared with EXTI on F0s (lines 21-22)"
#endif

#if STM32_COMP_USE_INTERRUPTS
#if !defined(STM32_DISABLE_EXTI21_22_29_HANDLER) || !defined(STM32_DISABLE_EXTI30_32_HANDLER) || !defined(STM32_DISABLE_EXTI33_HANDLER)
#error "COMP needs these defines in mcuconf to use interrupts: STM32_DISABLE_EXTI21_22_29_HANDLER STM32_DISABLE_EXTI30_32_HANDLER STM32_DISABLE_EXTI33_HANDLER"
#endif
#endif

#if STM32_COMP_USE_COMP1 && !STM32_HAS_COMP1
#error "COMP1 not present in the selected device"
#endif

#if STM32_COMP_USE_COMP2 && !STM32_HAS_COMP2
#error "COMP2 not present in the selected device"
#endif

#if STM32_COMP_USE_COMP3 && !STM32_HAS_COMP3
#error "COMP3 not present in the selected device"
#endif

#if STM32_COMP_USE_COMP4 && !STM32_HAS_COMP4
#error "COMP4 not present in the selected device"
#endif

#if STM32_COMP_USE_COMP5 && !STM32_HAS_COMP5
#error "COMP5 not present in the selected device"
#endif

#if STM32_COMP_USE_COMP6 && !STM32_HAS_COMP6
#error "COMP6 not present in the selected device"
#endif

#if STM32_COMP_USE_COMP7 && !STM32_HAS_COMP7
#error "COMP7 not present in the selected device"
#endif

#if !STM32_COMP_USE_COMP1 && !STM32_COMP_USE_COMP2 &&                         \
    !STM32_COMP_USE_COMP3 && !STM32_COMP_USE_COMP4 &&                         \
    !STM32_COMP_USE_COMP6 && !STM32_COMP_USE_COMP6 &&                         \
    !STM32_COMP_USE_COMP7
#error "COMP driver activated but no COMP peripheral assigned"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   COMP output mode.
 */
typedef enum {
  COMP_OUTPUT_NORMAL = 0,
  COMP_OUTPUT_INVERTED = 1
} comp_output_mode_t;

/**
 * @brief   COMP interrupt mode.
 */
typedef enum {
  COMP_IRQ_RISING = 0,
  COMP_IRQ_FALLING = 1,
  COMP_IRQ_BOTH = 2
} comp_irq_mode_t;

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  /**
   * @brief   Ouput mode.
   */
  comp_output_mode_t        output_mode;

  /**
   * @brief   Ouput mode.
   */
  comp_irq_mode_t           irq_mode;

  /**
   * @brief   Callback.
   */
  compcallback_t             cb;

  /* End of the mandatory fields.*/

  /**
   * @brief COMP CSR register initialization data.
   * @note  The value of this field should normally be equal to zero.
   */
  uint32_t                  csr;
} COMPConfig;

/**
 * @brief   Structure representing an COMP driver.
 */
struct COMPDriver {
  /**
   * @brief Driver state.
   */
  compstate_t                state;
  /**
   * @brief Current configuration data.
   */
  const COMPConfig           *config;
#if defined(COMP_DRIVER_EXT_FIELDS)
  COMP_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief Pointer to the COMPx registers block.
   */
  COMP_TypeDef               *reg;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if STM32_COMP_USE_COMP1 && !defined(__DOXYGEN__)
extern COMPDriver COMPD1;
#endif

#if STM32_COMP_USE_COMP2 && !defined(__DOXYGEN__)
extern COMPDriver COMPD2;
#endif

#if STM32_COMP_USE_COMP3 && !defined(__DOXYGEN__)
extern COMPDriver COMPD3;
#endif

#if STM32_COMP_USE_COMP4 && !defined(__DOXYGEN__)
extern COMPDriver COMPD4;
#endif

#if STM32_COMP_USE_COMP5 && !defined(__DOXYGEN__)
extern COMPDriver COMPD5;
#endif

#if STM32_COMP_USE_COMP6 && !defined(__DOXYGEN__)
extern COMPDriver COMPD6;
#endif

#if STM32_COMP_USE_COMP7 && !defined(__DOXYGEN__)
extern COMPDriver COMPD7;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void comp_lld_init(void);
  void comp_lld_start(COMPDriver *compp);
  void comp_lld_stop(COMPDriver *compp);
  void comp_lld_enable(COMPDriver *compp);
  void comp_lld_disable(COMPDriver *compp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_COMP */

#endif /* _comp_lld_H_ */

/** @} */
