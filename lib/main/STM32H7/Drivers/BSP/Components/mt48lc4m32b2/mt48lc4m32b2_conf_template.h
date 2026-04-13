/**
  ******************************************************************************
  * @file    mt48lc4m32b2_conf.h
  * @author  MCD Application Team
  * @brief   This file contains some configurations required for the
  *          MT48LC4M32B2 SDRAM memory.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MT48LC4M32B2_CONF_H
#define MT48LC4M32B2_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Update the below line with adequate hal file. ie: #include "stm32f7xx_hal.h" */
#include "stm32xxxx_hal.h"
   
/** @addtogroup BSP
  * @{
  */
  
/** @addtogroup Components
  * @{
  */

/** @addtogroup MT48LC4M32B2
  * @{
  */

/** @addtogroup MT48LC4M32B2_Exported_Constants
  * @{
  */  
#define REFRESH_COUNT                    ((uint32_t)0x0603)   /* SDRAM refresh counter */
   
#define MT48LC4M32B2_TIMEOUT             ((uint32_t)0xFFFF)

#ifdef __cplusplus
}
#endif

#endif /* mt48lc4m32b2_CONF_H */
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
