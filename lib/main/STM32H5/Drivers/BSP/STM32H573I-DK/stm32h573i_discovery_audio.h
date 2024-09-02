/**
  ******************************************************************************
  * @file    stm32h573i_discovery_audio.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32h573i_discovery_audio.c driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32H573I_DK_AUDIO_H
#define STM32H573I_DK_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h573i_discovery_conf.h"
#include "stm32h573i_discovery_errno.h"
#include "../Components/Common/audio.h"
#ifndef USE_AUDIO_CODEC_CS42L51
#define USE_AUDIO_CODEC_CS42L51          1U
#endif

#if (USE_AUDIO_CODEC_CS42L51 == 1)
/* Include audio component Driver */
#include "../Components/cs42l51/cs42l51.h"
#endif

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H573I_DK
  * @{
  */

/** @addtogroup STM32H573I_DK_AUDIO
  * @{
  */

/** @defgroup STM32H573I_DK_AUDIO_Exported_Types AUDIO Exported Types
  * @{
  */
typedef struct
{
  uint32_t Device;        /* Output or input device */
  uint32_t SampleRate;    /* From 8kHz to 192 kHz */
  uint32_t BitsPerSample; /* From 8 bits per sample to 32 bits per sample */
  uint32_t ChannelsNbr;   /* 1 for mono and 2 for stereo */
  uint32_t Volume;        /* In percentage from 0 to 100 */
} BSP_AUDIO_Init_t;

typedef struct
{
  uint32_t                    Instance;            /* Audio OUT instance              */
  uint32_t                    Device;              /* Audio OUT device to be used     */
  uint32_t                    SampleRate;          /* Audio OUT Sample rate           */
  uint32_t                    BitsPerSample;       /* Audio OUT Sample Bit Per Sample */
  uint32_t                    Volume;              /* Audio OUT volume                */
  uint32_t                    ChannelsNbr;         /* Audio OUT number of channel     */
  uint32_t                    IsMute;              /* Mute state                      */
  uint32_t                    State;               /* Audio OUT State                 */
  uint32_t                    IsMspCallbacksValid; /* Is Msp Callbacks registered     */
}AUDIO_OUT_Ctx_t;

/* Audio in context */
typedef struct
{
  uint32_t  Device;              /* Audio IN device to be used     */
  uint32_t  SampleRate;          /* Audio IN Sample rate           */
  uint32_t  BitsPerSample;       /* Audio IN Sample resolution     */
  uint32_t  ChannelsNbr;         /* Audio IN number of channel     */
  uint8_t   *pBuff;              /* Audio IN record buffer         */
  uint32_t  Size;                /* Audio IN record buffer size    */
  uint32_t  Volume;              /* Audio IN volume                */
  uint32_t  State;               /* Audio IN State                 */
  uint32_t  IsMspCallbacksValid; /* Is Msp Callbacks registered    */  
} AUDIO_IN_Ctx_t;

typedef struct
{
  uint32_t AudioFrequency;
  uint32_t AudioMode;
  uint32_t DataSize;
  uint32_t MonoStereoMode;
  uint32_t ClockStrobing;
  uint32_t Synchro;
  uint32_t OutputDrive;
  uint32_t SynchroExt;
  uint32_t FrameLength;
  uint32_t ActiveFrameLength;
  uint32_t SlotActive;
} MX_SAI_Config_t;

#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1)
typedef struct
{
  pSAI_CallbackTypeDef  pMspSaiInitCb;
  pSAI_CallbackTypeDef  pMspSaiDeInitCb;
} BSP_AUDIO_OUT_Cb_t;

typedef struct
{
  pSAI_CallbackTypeDef  pMspSaiInitCb;
  pSAI_CallbackTypeDef  pMspSaiDeInitCb;
} BSP_AUDIO_IN_Cb_t;
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1) */

/**
  * @}
  */

/** @defgroup STM32H573I_DK_AUDIO_Exported_Constants AUDIO Exported Constants
  * @{
  */
#define AUDIO_I2C_ADDRESS                0x94U

/* Audio sample rate */
#define AUDIO_FREQUENCY_192K           192000U
#define AUDIO_FREQUENCY_176K           176400U
#define AUDIO_FREQUENCY_96K             96000U
#define AUDIO_FREQUENCY_88K             88200U
#define AUDIO_FREQUENCY_48K             48000U
#define AUDIO_FREQUENCY_44K             44100U
#define AUDIO_FREQUENCY_32K             32000U
#define AUDIO_FREQUENCY_22K             22050U
#define AUDIO_FREQUENCY_16K             16000U
#define AUDIO_FREQUENCY_11K             11025U
#define AUDIO_FREQUENCY_8K               8000U

/* Audio Out devices */
#define AUDIO_OUT_DEVICE_NONE               0U
#define AUDIO_OUT_DEVICE_SPEAKER            1U
#define AUDIO_OUT_DEVICE_HEADPHONE          2U
#define AUDIO_OUT_DEVICE_SPK_HP             3U
#define AUDIO_OUT_DEVICE_AUTO               4U

/* Audio bits per sample */
#define AUDIO_RESOLUTION_8B                 8U
#define AUDIO_RESOLUTION_16B               16U
#define AUDIO_RESOLUTION_24B               24U
#define AUDIO_RESOLUTION_32B               32U

/* Audio mute state */
#define AUDIO_MUTE_DISABLED                 0U
#define AUDIO_MUTE_ENABLED                  1U

/* Audio Out states */
#define AUDIO_OUT_STATE_RESET               0U
#define AUDIO_OUT_STATE_PLAYING             1U
#define AUDIO_OUT_STATE_STOP                2U
#define AUDIO_OUT_STATE_PAUSE               3U

/* Audio Out states */
/* Volume Input Output selection */
#define AUDIO_VOLUME_INPUT                  0U
#define AUDIO_VOLUME_OUTPUT                 1U

/* Codec commands */
#define CODEC_PDWN_SW                       1U
#define CODEC_MUTE_ON                       1U
#define CODEC_MUTE_OFF                      0U

/* Audio Out instances number */
#define AUDIO_OUT_INSTANCES_NBR             1U

/* SAI peripheral configuration defines */
#define AUDIO_OUT_SAI                           SAI2_Block_A
#define AUDIO_OUT_SAI_CLK_ENABLE()              __HAL_RCC_SAI2_CLK_ENABLE()
#define AUDIO_OUT_SAI_CLK_DISABLE()             __HAL_RCC_SAI2_CLK_DISABLE()

#define AUDIO_OUT_SAI_MCLK_ENABLE()             __HAL_RCC_GPIOI_CLK_ENABLE()
#define AUDIO_OUT_SAI_MCLK_GPIO_PORT            GPIOI
#define AUDIO_OUT_SAI_MCLK_PIN                  GPIO_PIN_4
#define AUDIO_OUT_SAI_MCLK_AF                   GPIO_AF10_SAI2

#define AUDIO_OUT_SAI_SCK_ENABLE()              __HAL_RCC_GPIOI_CLK_ENABLE()
#define AUDIO_OUT_SAI_SCK_GPIO_PORT             GPIOI
#define AUDIO_OUT_SAI_SCK_PIN                   GPIO_PIN_5
#define AUDIO_OUT_SAI_SCK_AF                    GPIO_AF10_SAI2

#define AUDIO_OUT_SAI_SD_ENABLE()               __HAL_RCC_GPIOI_CLK_ENABLE()
#define AUDIO_OUT_SAI_SD_GPIO_PORT              GPIOI
#define AUDIO_OUT_SAI_SD_PIN                    GPIO_PIN_6
#define AUDIO_OUT_SAI_SD_AF                     GPIO_AF10_SAI2

#define AUDIO_OUT_SAI_FS_ENABLE()               __HAL_RCC_GPIOI_CLK_ENABLE()
#define AUDIO_OUT_SAI_FS_GPIO_PORT              GPIOI
#define AUDIO_OUT_SAI_FS_PIN                    GPIO_PIN_7
#define AUDIO_OUT_SAI_FS_AF                     GPIO_AF10_SAI2

#define AUDIO_NRST_ENABLE()                     __HAL_RCC_GPIOI_CLK_ENABLE()
#define AUDIO_NRST_GPIO_PORT                    GPIOI
#define AUDIO_NRST_PIN                          GPIO_PIN_11

/* SAI DMA Stream definitions */
#define AUDIO_OUT_SAI_DMA_CLK_ENABLE()          __HAL_RCC_GPDMA1_CLK_ENABLE()
#define AUDIO_OUT_SAI_DMA_CHANNEL               GPDMA1_Channel2
#define AUDIO_OUT_SAI_DMA_REQUEST               GPDMA1_REQUEST_SAI2_A
#define AUDIO_OUT_SAI_DMA_IRQ                   GPDMA1_Channel2_IRQn
#define AUDIO_OUT_SAI_DMA_IRQHandler            GPDMA1_Channel2_IRQHandler


/*------------------------------------------------------------------------------
                        AUDIO IN CONFIGURATION
------------------------------------------------------------------------------*/
/* SAI peripheral configuration defines */
#define AUDIO_IN_SAI                                SAI2_Block_B
#define AUDIO_IN_SAI_CLK_ENABLE()                   __HAL_RCC_SAI2_CLK_ENABLE()
#define AUDIO_IN_SAI_CLK_DISABLE()                  __HAL_RCC_SAI2_CLK_DISABLE()
#define AUDIO_IN_SAI_AF                             GPIO_AF10_SAI2
#define AUDIO_IN_SAI_SD_ENABLE()                    __HAL_RCC_GPIOG_CLK_ENABLE()
#define AUDIO_IN_SAI_SD_GPIO_PORT                   GPIOG
#define AUDIO_IN_SAI_SD_PIN                         GPIO_PIN_10

/* SAI DMA Stream definitions */
#define AUDIO_IN_SAI_DMA_CLK_ENABLE()              __HAL_RCC_GPDMA1_CLK_ENABLE()
#define AUDIO_IN_SAI_DMA_CHANNEL                   GPDMA1_Channel1
#define AUDIO_IN_SAI_DMA_REQUEST                   GPDMA1_REQUEST_SAI2_B
#define AUDIO_IN_SAI_DMA_IRQ                       GPDMA1_Channel1_IRQn
#define AUDIO_IN_SAI_DMA_IRQHandler                GPDMA1_Channel1_IRQHandler

/* SAI PDM input definitions */
#define AUDIO_IN_SAI_PDM                            SAI1_Block_A
#define AUDIO_IN_SAI_PDM_CLK_ENABLE()               __HAL_RCC_SAI1_CLK_ENABLE()
#define AUDIO_IN_SAI_PDM_CLK_DISABLE()              __HAL_RCC_SAI1_CLK_DISABLE()

#define AUDIO_IN_SAI_PDM_CLK_IN_ENABLE()            __HAL_RCC_GPIOD_CLK_ENABLE()
#define AUDIO_IN_SAI_PDM_CLK_IN_PIN                 GPIO_PIN_11
#define AUDIO_IN_SAI_PDM_CLK_IN_PORT                GPIOD
#define AUDIO_IN_SAI_PDM_DATA_IN_ENABLE()           __HAL_RCC_GPIOD_CLK_ENABLE()
#define AUDIO_IN_SAI_PDM_DATA_IN_PIN                GPIO_PIN_6
#define AUDIO_IN_SAI_PDM_DATA_IN_PORT               GPIOD
#define AUDIO_IN_SAI_PDM_DATA_CLK_AF                GPIO_AF2_SAI1

#define AUDIO_IN_SAI_PDM_IRQHandler                 SAI1_IRQHandler
#define AUDIO_IN_SAI_PDM_IRQ                        SAI1_IRQn

/* SAI PDM DMA Stream definitions */
#define AUDIO_IN_SAI_PDM_DMA_CLK_ENABLE()          __HAL_RCC_GPDMA1_CLK_ENABLE()
#define AUDIO_IN_SAI_PDM_DMA_CHANNEL               GPDMA1_Channel3
#define AUDIO_IN_SAI_PDM_DMA_REQUEST               GPDMA1_REQUEST_SAI1_A
#define AUDIO_IN_SAI_PDM_DMA_IRQ                   GPDMA1_Channel3_IRQn
#define AUDIO_IN_SAI_PDM_DMA_IRQHandler            GPDMA1_Channel3_IRQHandler

/* Audio In instances number:
   Instance 0 is SAI path
   Instance 1 is SAI PDM path
 */
#define AUDIO_IN_INSTANCES_NBR 2U

/* Audio input devices count */
#define AUDIO_IN_DEVICE_NUMBER 2U

/* Analog microphone input from 3.5 audio jack connector */
#define AUDIO_IN_DEVICE_ANALOG_MIC        0x00U

/* MP34DT01TR digital microphone on PCB top side */
#define AUDIO_IN_DEVICE_DIGITAL_MIC1      0x01U /* digital microphone 1 */
#define AUDIO_IN_DEVICE_DIGITAL_MIC       AUDIO_IN_DEVICE_DIGITAL_MIC1

/* Audio in states */
#define AUDIO_IN_STATE_RESET     0U
#define AUDIO_IN_STATE_RECORDING 1U
#define AUDIO_IN_STATE_STOP      2U
#define AUDIO_IN_STATE_PAUSE     3U
/**
  * @}
  */

/** @addtogroup STM32H573I_DK_AUDIO_Exported_Variables
  * @{
  */

/* Audio in and out context */
extern AUDIO_OUT_Ctx_t  Audio_Out_Ctx[AUDIO_OUT_INSTANCES_NBR];
extern AUDIO_IN_Ctx_t  Audio_In_Ctx[AUDIO_IN_INSTANCES_NBR];

/* Audio component object */
extern void *Audio_CompObj;

/* Audio driver */
extern AUDIO_Drv_t *Audio_Drv;

/* Play  */
extern SAI_HandleTypeDef haudio_out_sai;

/* Record  */
extern SAI_HandleTypeDef haudio_in_sai;

/* Audio in and out DMA handles used by SAI */
extern DMA_HandleTypeDef hDmaSaiTx, hDmaSaiRx;

/**
  * @}
  */

/** @defgroup STM32H573I_DK_AUDIO_OUT_Exported_Functions AUDIO OUT Exported Functions
  * @{
  */
int32_t           BSP_AUDIO_OUT_Init(uint32_t Instance, BSP_AUDIO_Init_t *AudioInit);
int32_t           BSP_AUDIO_OUT_DeInit(uint32_t Instance);
int32_t           BSP_AUDIO_OUT_Play(uint32_t Instance, uint8_t *pData, uint32_t NbrOfBytes);
int32_t           BSP_AUDIO_OUT_Pause(uint32_t Instance);
int32_t           BSP_AUDIO_OUT_Resume(uint32_t Instance);
int32_t           BSP_AUDIO_OUT_Stop(uint32_t Instance);
int32_t           BSP_AUDIO_OUT_Mute(uint32_t Instance);
int32_t           BSP_AUDIO_OUT_UnMute(uint32_t Instance);
int32_t           BSP_AUDIO_OUT_IsMute(uint32_t Instance, uint32_t *IsMute);
int32_t           BSP_AUDIO_OUT_SetVolume(uint32_t Instance, uint32_t Volume);
int32_t           BSP_AUDIO_OUT_GetVolume(uint32_t Instance, uint32_t *Volume);
int32_t           BSP_AUDIO_OUT_SetSampleRate(uint32_t Instance, uint32_t SampleRate);
int32_t           BSP_AUDIO_OUT_GetSampleRate(uint32_t Instance, uint32_t *SampleRate);
int32_t           BSP_AUDIO_OUT_SetDevice(uint32_t Instance, uint32_t Device);
int32_t           BSP_AUDIO_OUT_GetDevice(uint32_t Instance, uint32_t *Device);
int32_t           BSP_AUDIO_OUT_SetBitsPerSample(uint32_t Instance, uint32_t BitsPerSample);
int32_t           BSP_AUDIO_OUT_GetBitsPerSample(uint32_t Instance, uint32_t *BitsPerSample);
int32_t           BSP_AUDIO_OUT_SetChannelsNbr(uint32_t Instance, uint32_t ChannelNbr);
int32_t           BSP_AUDIO_OUT_GetChannelsNbr(uint32_t Instance, uint32_t *ChannelNbr);
int32_t           BSP_AUDIO_OUT_GetState(uint32_t Instance, uint32_t *State);

#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1)
int32_t           BSP_AUDIO_OUT_RegisterDefaultMspCallbacks(uint32_t Instance);
int32_t           BSP_AUDIO_OUT_RegisterMspCallbacks(uint32_t Instance, BSP_AUDIO_OUT_Cb_t *CallBacks);
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1) */

void              BSP_AUDIO_OUT_TransferComplete_CallBack(uint32_t Instance);
void              BSP_AUDIO_OUT_HalfTransfer_CallBack(uint32_t Instance);
void              BSP_AUDIO_OUT_Error_CallBack(uint32_t Instance);

void              BSP_AUDIO_OUT_IRQHandler(uint32_t Instance, uint32_t Device);

HAL_StatusTypeDef MX_SAI2_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t SampleRate);
HAL_StatusTypeDef MX_SAI2_Init(SAI_HandleTypeDef *hsai, MX_SAI_Config_t *MXInit);
/**
  * @}
  */

/** @defgroup STM32H573I_DK_AUDIO_IN_Exported_Functions AUDIO IN Exported Functions
  * @{
  */
int32_t           BSP_AUDIO_IN_Init(uint32_t Instance, BSP_AUDIO_Init_t *AudioInit);
int32_t           BSP_AUDIO_IN_DeInit(uint32_t Instance);
int32_t           BSP_AUDIO_IN_Record(uint32_t Instance, uint8_t *pData, uint32_t NbrOfBytes);
int32_t           BSP_AUDIO_IN_Pause(uint32_t Instance);
int32_t           BSP_AUDIO_IN_Resume(uint32_t Instance);
int32_t           BSP_AUDIO_IN_Stop(uint32_t Instance);
int32_t           BSP_AUDIO_IN_SetVolume(uint32_t Instance, uint32_t Volume);
int32_t           BSP_AUDIO_IN_GetVolume(uint32_t Instance, uint32_t *Volume);
int32_t           BSP_AUDIO_IN_SetSampleRate(uint32_t Instance, uint32_t SampleRate);
int32_t           BSP_AUDIO_IN_GetSampleRate(uint32_t Instance, uint32_t *SampleRate);
int32_t           BSP_AUDIO_IN_SetDevice(uint32_t Instance, uint32_t Device);
int32_t           BSP_AUDIO_IN_GetDevice(uint32_t Instance, uint32_t *Device);
int32_t           BSP_AUDIO_IN_SetBitsPerSample(uint32_t Instance, uint32_t BitsPerSample);
int32_t           BSP_AUDIO_IN_GetBitsPerSample(uint32_t Instance, uint32_t *BitsPerSample);
int32_t           BSP_AUDIO_IN_SetChannelsNbr(uint32_t Instance, uint32_t ChannelNbr);
int32_t           BSP_AUDIO_IN_GetChannelsNbr(uint32_t Instance, uint32_t *ChannelNbr);
int32_t           BSP_AUDIO_IN_GetState(uint32_t Instance, uint32_t *State);

/* Specific PDM recodr APIs */
int32_t BSP_AUDIO_IN_RecordPDM(uint32_t Instance, uint8_t* pBuf, uint32_t NbrOfBytes);

#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1)
int32_t           BSP_AUDIO_IN_RegisterDefaultMspCallbacks(uint32_t Instance);
int32_t           BSP_AUDIO_IN_RegisterMspCallbacks(uint32_t Instance, BSP_AUDIO_IN_Cb_t *CallBacks);
#endif /* USE_HAL_SAI_REGISTER_CALLBACKS == 1 */

void              BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance);
void              BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance);
void              BSP_AUDIO_IN_Error_CallBack(uint32_t Instance);
void              BSP_AUDIO_IN_IRQHandler(uint32_t Instance, uint32_t Device);

HAL_StatusTypeDef MX_SAI1_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t SampleRate);
HAL_StatusTypeDef MX_SAI1_Init(SAI_HandleTypeDef* hsai, MX_SAI_Config_t *MXConfig);
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

#endif /* STM32H573I_DK_AUDIO_H */
