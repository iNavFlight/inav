/**
  ******************************************************************************
  * @file    usbtypecswitch.h
  * @author  MCD Application Team
  * @brief   This header file contains the functions prototypes for the
  *          crossbar switch device for USB Type-C systems.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBTYPECSWITCH_H
#define __USBTYPECSWITCH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup TYPECSWITCH
  * @{
  */

/** @defgroup TYPECSWITCH_Exported_Types
  * @{
  */
typedef enum
{
  USB_NORMAL = 0,
  USB_FLIPPED,
  DFP_D_PIN_ASSIGNMENT_A_NORMAL,
  DFP_D_PIN_ASSIGNMENT_A_FLIPPED,
  DFP_D_PIN_ASSIGNMENT_B_NORMAL,
  DFP_D_PIN_ASSIGNMENT_B_FLIPPED,
  DFP_D_PIN_ASSIGNMENT_C_NORMAL,
  DFP_D_PIN_ASSIGNMENT_C_FLIPPED,
  DFP_D_PIN_ASSIGNMENT_D_NORMAL,
  DFP_D_PIN_ASSIGNMENT_D_FLIPPED,
  DFP_D_PIN_ASSIGNMENT_E_NORMAL,
  DFP_D_PIN_ASSIGNMENT_E_FLIPPED,
  DFP_D_PIN_ASSIGNMENT_F_NORMAL,
  DFP_D_PIN_ASSIGNMENT_F_FLIPPED,
  UFP_D_PIN_ASSIGNMENT_A_NORMAL,
  UFP_D_PIN_ASSIGNMENT_A_FLIPPED,
  UFP_D_PIN_ASSIGNMENT_B_NORMAL,
  UFP_D_PIN_ASSIGNMENT_B_FLIPPED,
  UFP_D_PIN_ASSIGNMENT_C_NORMAL,
  UFP_D_PIN_ASSIGNMENT_C_FLIPPED,
  UFP_D_PIN_ASSIGNMENT_D_NORMAL,
  UFP_D_PIN_ASSIGNMENT_D_FLIPPED,
  UFP_D_PIN_ASSIGNMENT_E_NORMAL,
  UFP_D_PIN_ASSIGNMENT_E_FLIPPED,
  UFP_D_PIN_ASSIGNMENT_F_NORMAL,
  UFP_D_PIN_ASSIGNMENT_F_FLIPPED
} TYPECSWITCH_Mode_t;

/** @defgroup TYPECSWITCH_Driver_structure  USB Type-C Crossbar Switch Driver structure
  * @{
  */
typedef struct
{
  uint32_t (*Init)(uint16_t);
  void (*DeInit)(uint16_t);
  uint32_t (*PowerOn)(uint16_t);
  uint32_t (*PowerOff)(uint16_t);
  uint32_t (*SetMode)(uint16_t, TYPECSWITCH_Mode_t);
  uint32_t (*IsSupportedMode)(TYPECSWITCH_Mode_t);
} TYPECSWITCH_Drv_t;
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBTYPECSWITCH_H */
