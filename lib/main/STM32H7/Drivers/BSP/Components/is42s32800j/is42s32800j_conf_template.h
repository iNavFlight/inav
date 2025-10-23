/**
  ******************************************************************************
  * @file    is42s32800j_conf.h
  * @author  MCD Application Team
  * @brief   This file contains some configurations required for the
  *          IS42S32800J SDRAM memory.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
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
#ifndef IS42S32800J_CONF_H
#define IS42S32800J_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
   
/** @addtogroup BSP
  * @{
  */
  
/** @addtogroup Components
  * @{
  */

/** @addtogroup IS42S32800J
  * @{
  */

/** @addtogroup IS42S32800J_Exported_Constants
  * @{
  */  
#define REFRESH_COUNT                    ((uint32_t)0x0603)   /* SDRAM refresh counter (100Mhz SD clock) */
   
#define IS42S32800J_TIMEOUT             ((uint32_t)0xFFFF)

#ifdef __cplusplus
}
#endif

#endif /* IS42S32800J_CONF_H */
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
