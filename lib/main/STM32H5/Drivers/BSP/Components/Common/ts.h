/**
  ******************************************************************************
  * @file    ts.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the Touch Screen driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
  int32_t (*Init)(void *);
  int32_t (*DeInit)(void *);
  int32_t (*GestureConfig)(void *, void *);
  int32_t (*ReadID)(void *, uint32_t *);
  int32_t (*GetState)(void *, void *);
  int32_t (*GetMultiTouchState)(void *, void *);
  int32_t (*GetGesture)(void *, void *);
  int32_t (*GetCapabilities)(void *, void *);
  int32_t (*EnableIT)(void *);
  int32_t (*DisableIT)(void *);
  int32_t (*ClearIT)(void *);
  int32_t (*ITStatus)(void *);
} TS_Drv_t;


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
