/**
  ******************************************************************************
  * @file    stm32756g_eval_eeprom.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for 
  *          the stm32756g_eval_eeprom.c firmware driver.
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
#ifndef __STM32F756G_EVAL_EEPROM_H
#define __STM32F756G_EVAL_EEPROM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32756g_eval.h"

/** @addtogroup BSP
  * @{
  */
  
/** @addtogroup STM32756G_EVAL
  * @{
  */ 
  
/** @addtogroup STM32756G_EVAL_EEPROM STM32756G_EVAL EEPROM
  * @brief This file includes the I2C EEPROM driver of STM32756G-EVAL evaluation board.
  * @{
  */

/** @defgroup STM32756G_EVAL_EEPROM_Exported_Types EEPROM Exported Types
  * @{
  */ 
/**
  * @}
  */
  
/** @defgroup STM32756G_EVAL_EEPROM_Exported_Constants EEPROM Exported Constants
  * @{
  */
/* EEPROM hardware address and page size */ 
#define EEPROM_PAGESIZE             ((uint8_t)4)
#define EEPROM_MAX_SIZE             ((uint16_t)0x2000) /* 64Kbit */
                                

/* Maximum number of trials for EEPROM_WaitEepromStandbyState() function */
#define EEPROM_MAX_TRIALS           ((uint32_t)3000)
      
#define EEPROM_OK                   ((uint32_t)0)
#define EEPROM_FAIL                 ((uint32_t)1)
#define EEPROM_TIMEOUT              ((uint32_t)2)
/**
  * @}
  */ 
  
/** @defgroup STM32756G_EVAL_EEPROM_Exported_Macros EEPROM Exported Macros
  * @{
  */    
/**
  * @}
  */ 

/** @defgroup STM32756G_EVAL_EEPROM_Exported_Functions EEPROM Exported Functions
  * @{
  */ 
uint32_t BSP_EEPROM_Init(void);
uint8_t BSP_EEPROM_DeInit(void);
uint32_t BSP_EEPROM_ReadBuffer(uint8_t* pBuffer, uint16_t ReadAddr, uint16_t* NumByteToRead);
uint32_t BSP_EEPROM_WritePage(uint8_t* pBuffer, uint16_t WriteAddr, uint8_t* NumByteToWrite);
uint32_t BSP_EEPROM_WriteBuffer(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite);
uint32_t BSP_EEPROM_WaitEepromStandbyState(void);

/* USER Callbacks: This function is declared as __weak in EEPROM driver and 
   should be implemented into user application.  
   BSP_EEPROM_TIMEOUT_UserCallback() function is called whenever a timeout condition 
   occurs during communication (waiting on an event that doesn't occur, bus
   errors, busy devices ...). */
void     BSP_EEPROM_TIMEOUT_UserCallback(void);

/* Link function for I2C EEPROM peripheral */
void              EEPROM_IO_Init(void);
HAL_StatusTypeDef EEPROM_IO_WriteData(uint16_t DevAddress, uint16_t MemAddress, uint8_t *pBuffer, uint32_t BufferSize);
HAL_StatusTypeDef EEPROM_IO_ReadData(uint16_t DevAddress, uint16_t MemAddress, uint8_t *pBuffer, uint32_t BufferSize);
HAL_StatusTypeDef EEPROM_IO_IsDeviceReady(uint16_t DevAddress, uint32_t Trials);

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

#endif /* __STM32756G_EVAL_EEPROM_H */

