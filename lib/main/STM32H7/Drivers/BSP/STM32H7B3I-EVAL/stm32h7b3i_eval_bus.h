/**
  ******************************************************************************
  * @file    stm32h7b3i_eval_bus.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for STM32H7B3I_EVAL LEDs,
  *          push-buttons hardware resources.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32H7B3I_EVAL_BUS_H
#define STM32H7B3I_EVAL_BUS_H

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "stm32h7b3i_eval_conf.h"
#if defined(BSP_USE_CMSIS_OS)
#include "cmsis_os.h"
#endif
/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H7B3I_EVAL
  * @{
  */

/** @addtogroup STM32H7B3I_EVAL_BUS
  * @{
  */
/** @defgroup STM32H7B3I_EVAL_BUS_Exported_Types BUS Exported Types
  * @{
  */
#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1)
typedef struct
{
  pI2C_CallbackTypeDef  pMspI2cInitCb;
  pI2C_CallbackTypeDef  pMspI2cDeInitCb;
} BSP_I2C_Cb_t;
#endif /* (USE_HAL_I2C_REGISTER_CALLBACKS == 1) */

/**
  * @}
  */
/** @defgroup STM32H7B3I_EVAL_BUS_Exported_Constants BUS Exported Constants
  * @{
  */
/* Definition for I2C2 clock resources */
#define BUS_I2C2                              I2C2
#define BUS_I2C2_CLK_ENABLE()                  __HAL_RCC_I2C2_CLK_ENABLE()
#define BUS_I2C2_CLK_DISABLE()                 __HAL_RCC_I2C2_CLK_DISABLE()
#define BUS_I2C2_SCL_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOH_CLK_ENABLE()
#define BUS_I2C2_SCL_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOH_CLK_DISABLE()
#define BUS_I2C2_SDA_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOH_CLK_ENABLE()
#define BUS_I2C2_SDA_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOH_CLK_DISABLE()

#define BUS_I2C2_FORCE_RESET()                 __HAL_RCC_I2C2_FORCE_RESET()
#define BUS_I2C2_RELEASE_RESET()               __HAL_RCC_I2C2_RELEASE_RESET()

/* Definition for I2C2 Pins */
#define BUS_I2C2_SCL_PIN                       GPIO_PIN_4
#define BUS_I2C2_SDA_PIN                       GPIO_PIN_5
#define BUS_I2C2_SCL_GPIO_PORT                 GPIOH
#define BUS_I2C2_SDA_GPIO_PORT                 GPIOH
#define BUS_I2C2_SCL_AF                        GPIO_AF4_I2C2
#define BUS_I2C2_SDA_AF                        GPIO_AF4_I2C2

#ifndef BUS_I2C2_FREQUENCY
#define BUS_I2C2_FREQUENCY  100000U /* Frequency of I2Cn = 100 KHz*/
#endif

/**
  * @}
  */

/** @addtogroup STM32H7B3I_EVAL_BUS_Exported_Variables
  * @{
  */
extern I2C_HandleTypeDef hbus_i2c2;
/**
  * @}
  */

/** @addtogroup STM32H7B3I_EVAL_BUS_Exported_Functions
  * @{
  */
int32_t BSP_I2C2_Init(void);
int32_t BSP_I2C2_DeInit(void);
int32_t BSP_I2C2_WriteReg(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length);
int32_t BSP_I2C2_ReadReg(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length);
int32_t BSP_I2C2_WriteReg16(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length);
int32_t BSP_I2C2_ReadReg16(uint16_t DevAddr, uint16_t Reg, uint8_t *pData, uint16_t Length);
int32_t BSP_I2C2_Recv(uint16_t DevAddr, uint16_t Reg, uint16_t MemAddSize, uint8_t *pData, uint16_t Length);
int32_t BSP_I2C2_Send(uint16_t DevAddr, uint16_t Reg, uint16_t MemAddSize, uint8_t *pData, uint16_t Length);
int32_t BSP_I2C2_IsReady(uint16_t DevAddr, uint32_t Trials);
int32_t BSP_GetTick(void);

#if (USE_HAL_I2C_REGISTER_CALLBACKS == 1)
int32_t BSP_I2C2_RegisterDefaultMspCallbacks(void);
int32_t BSP_I2C2_RegisterMspCallbacks(BSP_I2C_Cb_t *Callback);
#endif /* USE_HAL_I2C_REGISTER_CALLBACKS */
__weak HAL_StatusTypeDef MX_I2C2_Init(I2C_HandleTypeDef *hI2c, uint32_t timing);


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

#endif /* STM32H7B3I_EVAL_BUS_H */
