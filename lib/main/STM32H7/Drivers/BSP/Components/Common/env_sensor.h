/**
 ******************************************************************************
 * @file    env_sensor.h
 * @author  MCD Application Team
 * @brief   This header file contains the functions prototypes for the
 *          temperature driver
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
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
#ifndef ENV_SENSORS_H
#define ENV_SENSORS_H

#ifdef __cplusplus
extern "C" {
#endif



/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/** @addtogroup BSP BSP
 * @{
 */

/** @addtogroup COMPONENTS COMPONENTS
 * @{
 */

/** @addtogroup COMMON COMMON
 * @{
 */

/** @addtogroup ENV_SENSORS ENV_SENSORS
 * @{
 */

/** @addtogroup ENV_SENSORS_Public_Types ENV_SENSORS Public types
 * @{
 */

/**
 * @brief  ENV_SENSORS driver structure definition
 */
typedef struct
{
  int32_t ( *Init              ) ( void * );
  int32_t ( *DeInit            ) ( void * );
  int32_t ( *ReadID            ) ( void *, uint8_t * ); 
  int32_t ( *GetCapabilities   ) ( void *, void * );
} ENV_SENSOR_CommonDrv_t;

typedef struct
{
  int32_t ( *Enable            ) ( void * );
  int32_t ( *Disable           ) ( void * );
  int32_t ( *GetOutputDataRate ) ( void *, float * );
  int32_t ( *SetOutputDataRate ) ( void *, float );
  int32_t ( *GetValue          ) ( void *, float * );
} ENV_SENSOR_FuncDrv_t;

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

#endif /* ENV_SENSORS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
