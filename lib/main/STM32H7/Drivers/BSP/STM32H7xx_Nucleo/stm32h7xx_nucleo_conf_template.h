/** 
  ******************************************************************************
  * @file    stm32h7xx_nucleo_conf.h
  * @author  MCD Application Team
  * @brief   STM32H7xx_Nuleo board configuration file.
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
#ifndef STM32H7XX_NUCLEO_CONF_H
#define STM32H7XX_NUCLEO_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/** @addtogroup BSP
  * @{
  */
  
/** @addtogroup STM32H7XX_NUCLEO
  * @{
  */

/** @defgroup STM32H7XX_NUCLEO_CONFIG Config
  * @{
  */ 
  
/** @defgroup STM32H7XX_NUCLEO_CONFIG_Exported_Constants Exported Constants
  * @{
  */ 
/* Nucleo pin and part number defines */
#define USE_NUCLEO_144
#define USE_NUCLEO_H7A3ZI_Q

/* COM define */
#define USE_COM_LOG                         0U
#define USE_BSP_COM_FEATURE                 0U

/* IRQ priorities */
#define BSP_BUTTON_USER_IT_PRIORITY         15U

#define BUS_SPI1_BAUDRATE                   18000000
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
  
#endif /* STM32H7XX_NUCLEO_CONF_H */
