/**
  ******************************************************************************
  * @file    idd.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the IDD driver.
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
#ifndef IDD_H
#define IDD_H

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

/** @addtogroup IDD
  * @{
  */

/** @defgroup IDD_Exported_Types IDD Exported Types
  * @{
  */

/** @defgroup IDD_Driver_structure  IDD Driver structure
  * @{
  */
typedef struct
{
  int32_t    (*Init             )(void*);
  int32_t    (*DeInit           )(void*);
  int32_t    (*ReadID           )(void*, uint32_t*);
  int32_t    (*Reset            )(void*);
  int32_t    (*LowPower         )(void*);
  int32_t    (*WakeUp           )(void*);
  int32_t    (*Start            )(void*);
  int32_t    (*Config           )(void*, void*);
  int32_t    (*GetValue         )(void*, uint32_t*);
  int32_t    (*EnableIT         )(void*);
  int32_t    (*DisableIT        )(void*); 
  int32_t    (*ITStatus         )(void*);   
  int32_t    (*ClearIT          )(void*);
  int32_t    (*ErrorEnableIT    )(void*);
  int32_t    (*ErrorClearIT     )(void*);
  int32_t    (*ErrorGetITStatus)(void*);
  int32_t    (*ErrorDisableIT   )(void*);
  int32_t    (*ErrorGetSrc      )(void*);
  int32_t    (*ErrorGetCode     )(void*);
}IDD_Drv_t;
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

#endif /* IDD_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
