/**
  ******************************************************************************
  * @file    epd.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the
  *          EPD (E Paper Display) driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EPD_H
#define __EPD_H

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

/** @addtogroup Common
  * @{
  */

/** @addtogroup EPD
  * @{
  */

/** @defgroup EPD_Exported_Types
  * @{
  */

/** @defgroup EPD_Driver_structure  E Paper Display Driver structure
  * @{
  */
typedef struct
{
  void (*Init)(void);
  void (*WritePixel)(uint8_t);

  /* Optimized operation */
  void (*SetDisplayWindow)(uint16_t, uint16_t, uint16_t, uint16_t);
  void (*RefreshDisplay)(void);
  void (*CloseChargePump)(void);

  uint16_t (*GetEpdPixelWidth)(void);
  uint16_t (*GetEpdPixelHeight)(void);
  void (*DrawImage)(uint16_t, uint16_t, uint16_t, uint16_t, uint8_t *);
}
EPD_DrvTypeDef;
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

#endif /* EPD_H */
