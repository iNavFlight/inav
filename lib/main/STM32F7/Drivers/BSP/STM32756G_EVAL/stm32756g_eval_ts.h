/**
  ******************************************************************************
  * @file    stm32756g_eval_ts.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32756g_eval_ts.c driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32756G_EVAL_TS_H
#define __STM32756G_EVAL_TS_H

#ifdef __cplusplus
 extern "C" {
#endif   
   
/* Includes ------------------------------------------------------------------*/
#include "stm32756g_eval.h"
/* Include IOExpander(STMPE811) component Driver */ 
#include "../Components/stmpe811/stmpe811.h"
/* Include TouchScreen component drivers */
#include "../Components/ts3510/ts3510.h"
#include "../Components/exc7200/exc7200.h"	 
   
/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup STM32756G_EVAL
  * @{
  */
    
/** @addtogroup STM32756G_EVAL_TS
  * @{
  */    

/** @defgroup STM32756G_EVAL_TS_Exported_Types TS Exported Types
  * @{
  */
typedef struct
{
  uint16_t TouchDetected;
  uint16_t x;
  uint16_t y;
  uint16_t z;
}TS_StateTypeDef;
/**
  * @}
  */ 

/** @defgroup STM32756G_EVAL_TS_Exported_Constants TS Exported Constants
  * @{
  */
#define TS_SWAP_NONE                    ((uint8_t)0x00)
#define TS_SWAP_X                       ((uint8_t)0x01)
#define TS_SWAP_Y                       ((uint8_t)0x02)
#define TS_SWAP_XY                      ((uint8_t)0x04)

typedef enum 
{
  TS_OK       = 0x00,
  TS_ERROR    = 0x01,
  TS_TIMEOUT  = 0x02
}TS_StatusTypeDef;

/* Interrupt sources pins definition */
#define TS_INT_PIN                      LCD_INT_PIN
/**
  * @}
  */ 

/** @defgroup STM32756G_EVAL_TS_Exported_Macros TS Exported Macros
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup STM32756G_EVAL_TS_Exported_Functions TS Exported Functions
  * @{
  */
uint8_t BSP_TS_Init(uint16_t xSize, uint16_t ySize);
uint8_t BSP_TS_DeInit(void);
uint8_t BSP_TS_GetState(TS_StateTypeDef *TS_State);
uint8_t BSP_TS_ITConfig(void);
uint8_t BSP_TS_ITGetStatus(void);
void    BSP_TS_ITClear(void);

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

#endif /* __STM32756G_EVAL_TS_H */

