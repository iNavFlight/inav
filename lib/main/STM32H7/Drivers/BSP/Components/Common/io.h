/**
  ******************************************************************************
  * @file    io.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the IO driver.
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
#ifndef IO_H
#define IO_H

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
    
/** @addtogroup IO
  * @{
  */

/** @defgroup IO_Exported_Types IO Exported Types
  * @{ 
  */

/** @defgroup IO_Driver_structure  IO Driver structure
  * @{
  */
typedef struct
{  
  int32_t   (*Init      )(void*, void*);
  int32_t   (*DeInit    )(void*);
  int32_t   (*ReadID    )(void*, uint32_t*);
  int32_t   (*Reset     )(void*);
  int32_t   (*Start     )(void*, uint32_t);
  int32_t   (*WritePin  )(void*, uint32_t, uint8_t);
  int32_t   (*ReadPin   )(void*, uint32_t);
  int32_t   (*EnableIT  )(void*);
  int32_t   (*DisableIT )(void*);
  int32_t   (*ITStatus  )(void*, uint32_t);
  int32_t   (*ClearIT   )(void*, uint32_t);    
}IO_Drv_t;
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

#endif /* IO_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
