/**
  ******************************************************************************
  * @file    motion_sensor.h
  * @author  MCD Application Team
  * @brief   This header file contains the functions prototypes for the
  *          motion sensor driver
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MOTION_SENSOR_H
#define MOTION_SENSOR_H

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

/** @addtogroup MOTION_SENSOR MOTION SENSOR
  * @{
  */

/** @addtogroup MOTION_SENSOR_Public_Types MOTION SENSOR Public types
  * @{
  */

/**
  * @brief  MOTION SENSOR driver structure definition
  */
typedef struct
{
  int32_t (*Init)(void *);
  int32_t (*DeInit)(void *);
  int32_t (*ReadID)(void *, uint8_t *);
  int32_t (*GetCapabilities)(void *, void *);
} MOTION_SENSOR_CommonDrv_t;

typedef struct
{
  int32_t (*Enable)(void *);
  int32_t (*Disable)(void *);
  int32_t (*GetSensitivity)(void *, float *);
  int32_t (*GetOutputDataRate)(void *, float *);
  int32_t (*SetOutputDataRate)(void *, float);
  int32_t (*GetFullScale)(void *, int32_t *);
  int32_t (*SetFullScale)(void *, int32_t);
  int32_t (*GetAxes)(void *, void *);
  int32_t (*GetAxesRaw)(void *, void *);
} MOTION_SENSOR_FuncDrv_t;

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

#endif /* MOTION_SENSOR_H */
