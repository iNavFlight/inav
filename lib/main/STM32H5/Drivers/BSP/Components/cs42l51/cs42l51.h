/**
  ******************************************************************************
  * @file    cs42l51.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the
  *          cs42l51.c driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017-2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CS42L51_H
#define CS42L51_H

/* Includes ------------------------------------------------------------------*/
#include "cs42l51_reg.h"
#include <stddef.h>

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @addtogroup CS42L51
  * @{
  */
typedef int32_t (*CS42L51_Init_Func)(void);
typedef int32_t (*CS42L51_DeInit_Func)(void);
typedef int32_t (*CS42L51_GetTick_Func)(void);
typedef int32_t (*CS42L51_WriteReg_Func)(uint16_t, uint16_t, uint8_t *, uint16_t);
typedef int32_t (*CS42L51_ReadReg_Func)(uint16_t, uint16_t, uint8_t *, uint16_t);

typedef struct
{
  CS42L51_Init_Func          Init;
  CS42L51_DeInit_Func        DeInit;
  uint16_t                   Address;
  CS42L51_WriteReg_Func      WriteReg;
  CS42L51_ReadReg_Func       ReadReg;
  CS42L51_GetTick_Func       GetTick;
} CS42L51_IO_t;


typedef struct
{
  CS42L51_IO_t         IO;
  cs42l51_ctx_t        Ctx;
  uint8_t              IsInitialized;
} CS42L51_Object_t;

typedef struct
{
  uint32_t   InputDevice;
  uint32_t   OutputDevice;
  uint32_t   Frequency;
  uint32_t   Resolution;
  uint32_t   Volume;
} CS42L51_Init_t;

typedef struct
{
  int32_t (*Init)(CS42L51_Object_t *, const CS42L51_Init_t *);
  int32_t (*DeInit)(CS42L51_Object_t *);
  int32_t (*ReadID)(CS42L51_Object_t *, uint32_t *);
  int32_t (*Play)(CS42L51_Object_t *);
  int32_t (*Pause)(CS42L51_Object_t *);
  int32_t (*Resume)(CS42L51_Object_t *);
  int32_t (*Stop)(CS42L51_Object_t *, uint32_t);
  int32_t (*SetFrequency)(const CS42L51_Object_t *, uint32_t);
  int32_t (*GetFrequency)(const CS42L51_Object_t *, const uint32_t *);
  int32_t (*SetVolume)(CS42L51_Object_t *, uint32_t, uint8_t);
  int32_t (*GetVolume)(CS42L51_Object_t *, uint32_t, uint8_t *);
  int32_t (*SetMute)(CS42L51_Object_t *, uint32_t);
  int32_t (*SetOutputMode)(const CS42L51_Object_t *, uint32_t);
  int32_t (*SetResolution)(const CS42L51_Object_t *, uint32_t);
  int32_t (*GetResolution)(const CS42L51_Object_t *, const uint32_t *);
  int32_t (*SetProtocol)(const CS42L51_Object_t *, uint32_t);
  int32_t (*GetProtocol)(const CS42L51_Object_t *, const uint32_t *);
  int32_t (*Reset)(const CS42L51_Object_t *);
} CS42L51_Drv_t;


/** @defgroup CS42L51_Exported_Constants
  * @{
  */
#define CS42L51_OK                (0)
#define CS42L51_ERROR             (-1)

/******************************************************************************/
/***************************  Codec User defines ******************************/
/******************************************************************************/
/* Audio Input Device */
#define CS42L51_IN_NONE           0x0000U
#define CS42L51_IN_MIC1           0x0100U
#define CS42L51_IN_MIC2           0x0200U
#define CS42L51_IN_LINE1          0x0400U
#define CS42L51_IN_LINE2          0x0800U
#define CS42L51_IN_LINE3          0x1000U

/* Audio Output Device */
#define CS42L51_OUT_NONE          0x0000U
#define CS42L51_OUT_HEADPHONE     0x0001U

/* AUDIO FREQUENCY */
#define CS42L51_FREQUENCY_192K    192000
#define CS42L51_FREQUENCY_176K    176400
#define CS42L51_FREQUENCY_96K     96000
#define CS42L51_FREQUENCY_88K     88200
#define CS42L51_FREQUENCY_48K     48000
#define CS42L51_FREQUENCY_44K     44100
#define CS42L51_FREQUENCY_32K     32000
#define CS42L51_FREQUENCY_22K     22050
#define CS42L51_FREQUENCY_16K     16000
#define CS42L51_FREQUENCY_11K     11025
#define CS42L51_FREQUENCY_8K      8000

/* AUDIO RESOLUTION */
#define CS42L51_RESOLUTION_16B    0x05U
#define CS42L51_RESOLUTION_18B    0x04U
#define CS42L51_RESOLUTION_20B    0x03U
#define CS42L51_RESOLUTION_24B    0x02U

/* Codec stop options */
#define CS42L51_PDWN_HW           0x00U
#define CS42L51_PDWN_SW           0x01U

/* Volume Input Output selection */
#define VOLUME_INPUT                  0U
#define VOLUME_OUTPUT                 1U

/* MUTE commands */
#define CS42L51_MUTE_ON                1U
#define CS42L51_MUTE_OFF               0U

/* AUDIO PROTOCOL */
#define CS42L51_PROTOCOL_L_JUSTIFIED   0U
#define CS42L51_PROTOCOL_I2S           1U
#define CS42L51_PROTOCOL_R_JUSTIFIED   2U

#define VOLUME_CONVERT(Volume)    (((Volume) >= 100U) ? 0U : ((uint8_t)((((Volume) * 2U) + 56U))))
#define VOLUME_INVERT(Volume)     (((Volume) == 0U) ? 100U : ((uint8_t)(((Volume) - 56U) / 2U)))

/**
  * @brief  CS42L51 ID
  */
#define  CS42L51_ID        0xD8U
#define  CS42L51_ID_MASK   0xF8U

/** @defgroup CS42L51_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup CS42L51_Exported_Functions
  * @{
  */

/*------------------------------------------------------------------------------
                           Audio Codec functions
------------------------------------------------------------------------------*/
/* High Layer codec functions */
int32_t CS42L51_RegisterBusIO(CS42L51_Object_t *pObj, CS42L51_IO_t *pIO);
int32_t CS42L51_Init(CS42L51_Object_t *pObj, const CS42L51_Init_t *pInit);
int32_t CS42L51_DeInit(CS42L51_Object_t *pObj);
int32_t CS42L51_ReadID(CS42L51_Object_t *pObj, uint32_t *Id);
int32_t CS42L51_Play(CS42L51_Object_t *pObj);
int32_t CS42L51_Pause(CS42L51_Object_t *pObj);
int32_t CS42L51_Resume(CS42L51_Object_t *pObj);
int32_t CS42L51_Stop(CS42L51_Object_t *pObj, uint32_t CodecPdwnMode);
int32_t CS42L51_SetVolume(CS42L51_Object_t *pObj, uint32_t InputOutput, uint8_t Volume);
int32_t CS42L51_GetVolume(CS42L51_Object_t *pObj, uint32_t InputOutput, uint8_t *Volume);
int32_t CS42L51_SetMute(CS42L51_Object_t *pObj, uint32_t Cmd);
int32_t CS42L51_SetOutputMode(const CS42L51_Object_t *pObj, uint32_t Output);
int32_t CS42L51_SetResolution(const CS42L51_Object_t *pObj, uint32_t Resolution);
int32_t CS42L51_GetResolution(const CS42L51_Object_t *pObj, const uint32_t *Resolution);
int32_t CS42L51_SetProtocol(const CS42L51_Object_t *pObj, uint32_t Protocol);
int32_t CS42L51_GetProtocol(const CS42L51_Object_t *pObj, const uint32_t *Protocol);
int32_t CS42L51_SetFrequency(const CS42L51_Object_t *pObj, uint32_t AudioFreq);
int32_t CS42L51_GetFrequency(const CS42L51_Object_t *pObj, const uint32_t *AudioFreq);
int32_t CS42L51_Reset(const CS42L51_Object_t *pObj);

/* Audio driver structure */
extern CS42L51_Drv_t CS42L51_Driver;

#endif /* CS42L51_H */

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
