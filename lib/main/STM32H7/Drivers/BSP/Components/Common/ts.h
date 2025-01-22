/**
  ******************************************************************************
  * @file    ts.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the Touch Screen driver.
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
#ifndef TS_H
#define TS_H

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
    
/** @addtogroup TS
  * @{
  */

/** @defgroup TS_Exported_Types
  * @{
  */

/** @defgroup TS_Driver_structure  Touch Sensor Driver structure
  * @{
  */
typedef struct
{  
  int32_t ( *Init                 ) (void *);  
  int32_t ( *DeInit               ) (void *);
  int32_t ( *GestureConfig        ) (void *, void*);
  int32_t ( *ReadID               ) (void *, uint32_t *);  
  int32_t ( *GetState             ) (void *, void*);
  int32_t ( *GetMultiTouchState   ) (void *, void*);
  int32_t ( *GetGesture           ) (void *, void*);
  int32_t ( *GetCapabilities      ) (void *, void*);  
  int32_t ( *EnableIT             ) (void *);
  int32_t ( *DisableIT            ) (void *);
  int32_t ( *ClearIT              ) (void *);
  int32_t ( *ITStatus             ) (void *);
}TS_Drv_t;

  
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

#endif /* TS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
