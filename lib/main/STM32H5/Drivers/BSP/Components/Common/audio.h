/**
  ******************************************************************************
  * @file    audio.h
  * @author  MCD Application Team
  * @brief   This header file contains the common defines and functions prototypes
  *          for the Audio driver.
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
#ifndef AUDIO_H
#define AUDIO_H

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

/** @addtogroup AUDIO
  * @{
  */

/** @defgroup AUDIO_Exported_Constants
  * @{
  */

/**
  * @}
  */

/** @defgroup AUDIO_Exported_Types
  * @{
  */



/** @defgroup AUDIO_Driver_structure  Audio Driver structure
  * @{
  */
typedef struct
{
  int32_t (*Init)(void *, void *);
  int32_t (*DeInit)(void *);
  int32_t (*ReadID)(void *, uint32_t *);
  int32_t (*Play)(void *);
  int32_t (*Pause)(void *);
  int32_t (*Resume)(void *);
  int32_t (*Stop)(void *, uint32_t);
  int32_t (*SetFrequency)(void *, uint32_t);
  int32_t (*GetFrequency)(void *);
  int32_t (*SetVolume)(void *, uint32_t, uint8_t);
  int32_t (*GetVolume)(void *, uint32_t, uint8_t *);
  int32_t (*SetMute)(void *, uint32_t);
  int32_t (*SetOutputMode)(void *, uint32_t);
  int32_t (*SetResolution)(void *, uint32_t);
  int32_t (*GetResolution)(void *, uint32_t *);
  int32_t (*SetProtocol)(void *, uint32_t);
  int32_t (*GetProtocol)(void *, uint32_t *);
  int32_t (*Reset)(void *);
} AUDIO_Drv_t;
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

#endif /* AUDIO_H */
