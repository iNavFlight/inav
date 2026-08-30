/**
  ******************************************************************************
  * @file    stm32h743i_eval_audio.c
  * @author  MCD Application Team
  * @brief   This file provides the Audio driver for the STM32H743I_EVAL
  *          board.
  @verbatim
  How To use this driver:
  -----------------------
   + This driver supports stm32h7xx devices on STM32H743I_EVAL (MB1246) boards.
   + Call the function BSP_AUDIO_OUT_Init() for AUDIO OUT initialization:
        Instance:  Select the output instance. It can only be 0 (SAI)
        AudioInit: Audio Out structure to select the following parameters
                   - Device: Select the output device (headphone, speaker, hdmi ..)
                   - SampleRate: Select the output sample rate (8Khz .. 96Khz)
                   - BitsPerSample: Select the output resolution (16 or 32bits per sample)
                   - ChannelsNbr: Select the output channels number(1 for mono, 2 for stereo)
                   - Volume: Select the output volume(0% .. 100%)

      This function configures all the hardware required for the audio application (codec, I2C, SAI,
      GPIOs, DMA and interrupt if needed). This function returns BSP_ERROR_NONE if configuration is OK.
      If the returned value is different from BSP_ERROR_NONE or the function is stuck then the communication with
      the codec or the MFX has failed (try to un-plug the power or reset device in this case).

      User can update the SAI or the clock configurations by overriding the weak MX functions MX_SAI1_Block_A_Init()
      and  MX_SAI1_ClockConfig()
      User can override the default MSP configuration and register his own MSP callbacks (defined at application level)
      by calling BSP_AUDIO_OUT_RegisterMspCallbacks() function
      User can restore the default MSP configuration by calling BSP_AUDIO_OUT_RegisterDefaultMspCallbacks()
      To use these two functions, user have to enable USE_HAL_SAI_REGISTER_CALLBACKS within stm32h7xx_hal_conf.h file


   + Call the function BSP_AUDIO_OUT_Play() to play audio stream:
        Instance:  Select the output instance. It can only be 0 (SAI)
        pBuf: pointer to the audio data file address
        NbrOfBytes: Total size of the buffer to be sent in Bytes

   + Call the function BSP_AUDIO_OUT_Pause() to pause playing
   + Call the function BSP_AUDIO_OUT_Resume() to resume playing.
       Note. After calling BSP_AUDIO_OUT_Pause() function for pause, only BSP_AUDIO_OUT_Resume() should be called
          for resume (it is not allowed to call BSP_AUDIO_OUT_Play() in this case).
       Note. This function should be called only when the audio file is played or paused (not stopped).
   + Call the function BSP_AUDIO_OUT_Stop() to stop playing.
   + Call the function BSP_AUDIO_OUT_Mute() to mute the player.
   + Call the function BSP_AUDIO_OUT_UnMute() to unmute the player.
   + Call the function BSP_AUDIO_OUT_IsMute() to get the mute state(BSP_AUDIO_MUTE_ENABLED or BSP_AUDIO_MUTE_DISABLED).
   + Call the function BSP_AUDIO_OUT_SetDevice() to update the AUDIO OUT device.
   + Call the function BSP_AUDIO_OUT_GetDevice() to get the AUDIO OUT device.
   + Call the function BSP_AUDIO_OUT_SetSampleRate() to update the AUDIO OUT sample rate.
   + Call the function BSP_AUDIO_OUT_GetSampleRate() to get the AUDIO OUT sample rate.
   + Call the function BSP_AUDIO_OUT_SetBitsPerSample() to update the AUDIO OUT resolution.
   + Call the function BSP_AUDIO_OUT_GetBitPerSample() to get the AUDIO OUT resolution.
   + Call the function BSP_AUDIO_OUT_SetChannelsNbr() to update the AUDIO OUT number of channels.
   + Call the function BSP_AUDIO_OUT_GetChannelsNbr() to get the AUDIO OUT number of channels.
   + Call the function BSP_AUDIO_OUT_SetVolume() to update the AUDIO OUT volume.
   + Call the function BSP_AUDIO_OUT_GetVolume() to get the AUDIO OUT volume.
   + Call the function BSP_AUDIO_OUT_GetState() to get the AUDIO OUT state.

   + BSP_AUDIO_OUT_SetDevice(), BSP_AUDIO_OUT_SetSampleRate(), BSP_AUDIO_OUT_SetBitsPerSample() and
     BSP_AUDIO_OUT_SetChannelsNbr() cannot be called while the state is AUDIO_OUT_STATE_PLAYING.
   + For each mode, you may need to implement the relative callback functions into your code.
      The Callback functions are named AUDIO_OUT_XXX_CallBack() and only their prototypes are declared in
      the STM32H743I_EVAL_AUDIO.h file. (refer to the example for more details on the callbacks implementations)


   + Call the function BSP_AUDIO_IN_Init() for AUDIO IN initialization:
          Instance : Select the input instance. Can be 0 (SAI) or 1 (DFSDM)
        AudioInit: Audio In structure to select the following parameters
                   - Device: Select the input device (analog, digital micx)
                   - SampleRate: Select the input sample rate (8Khz .. 96Khz)
                   - BitsPerSample: Select the input resolution (16 or 32bits per sample)
                   - ChannelsNbr: Select the input channels number(1 for mono, 2 for stereo)
                   - Volume: Select the input volume(0% .. 100%)


      This function configures all the hardware required for the audio application (codec, I2C, SAI, DFSDM
      GPIOs, DMA and interrupt if needed). This function returns BSP_ERROR_NONE if configuration is OK.
      If the returned value is different from BSP_ERROR_NONE or the function is stuck then the communication with
      the codec or the MFX has failed (try to un-plug the power or reset device in this case).
      User can update the DFSDM/SAI or the clock configurations by overriding the weak MX functions MX_SAIx_Init(),
      MX_SAIx_ClockConfig(), MX_DFSDMx_Init() and MX_DFSDMx_ClockConfig()
      User can override the default MSP configuration and register his own MSP callbacks (defined at application level)
      by calling BSP_AUDIO_IN_RegisterMspCallbacks() function
      User can restore the default MSP configuration by calling BSP_AUDIO_IN_RegisterDefaultMspCallbacks()
      To use these two functions, user have to enable USE_HAL_SAI_REGISTER_CALLBACKS and/or USE_HAL_DFSDM_REGISTER_CALLBACKS
      within stm32h7xx_hal_conf.h file

   + Call the function BSP_AUDIO_IN_Record() to record audio stream. The recorded data are stored to user buffer in raw
        (L, R, L, R ...)
          Instance : Select the input instance. Can be 0 (SAI) or 1 (DFSDM)
        pBuf: pointer to user buffer
        NbrOfBytes: Total size of the buffer to be sent in Bytes

   + Call the function BSP_AUDIO_IN_Pause() to pause recording
   + Call the function BSP_AUDIO_IN_Resume() to resume recording.
   + Call the function BSP_AUDIO_IN_Stop() to stop recording.
   + Call the function BSP_AUDIO_IN_SetDevice() to update the AUDIO IN device.
   + Call the function BSP_AUDIO_IN_GetDevice() to get the AUDIO IN device.
   + Call the function BSP_AUDIO_IN_SetSampleRate() to update the AUDIO IN sample rate.
   + Call the function BSP_AUDIO_IN_GetSampleRate() to get the AUDIO IN sample rate.
   + Call the function BSP_AUDIO_IN_SetBitPerSample() to update the AUDIO IN resolution.
   + Call the function BSP_AUDIO_IN_GetBitPerSample() to get the AUDIO IN resolution.
   + Call the function BSP_AUDIO_IN_SetChannelsNbr() to update the AUDIO IN number of channels.
   + Call the function BSP_AUDIO_IN_GetChannelsNbr() to get the AUDIO IN number of channels.
   + Call the function BSP_AUDIO_IN_SetVolume() to update the AUDIO IN volume.
   + Call the function BSP_AUDIO_IN_GetVolume() to get the AUDIO IN volume.
   + Call the function BSP_AUDIO_IN_GetState() to get the AUDIO IN state.
   + Call the function BSP_AUDIO_IN_RecordChannels() to record audio stream. The recorded data are stored to user buffers separately
        (L, L, ...) (R, R ...). User has to process his data at application level.
        Instance : Select the input instance. Can be 1 (DFSDM)
        pBuf: pointer to user buffers table
        NbrOfBytes: Total size of the buffer to be sent in Bytes
   + Call the function BSP_AUDIO_IN_PauseChannels() to pause recording:
        Instance : Select the input instance. Can be 1 (DFSDM)
        Device: Select the input device (digital micX)
   + Call the function BSP_AUDIO_IN_ResumeChannels() to resume recording.
        Instance : Select the input instance. Can be 1 (DFSDM)
        Device: Select the input device (digital micX)
   + Call the function BSP_AUDIO_IN_StopChannels() to stop recording.
        Instance : Select the input instance. Can be 1 (DFSDM)
        Device: Select the input device (digital micX)
   + For each mode, you may need to implement the relative callback functions into your code.
      The Callback functions are named AUDIO_IN_XXX_CallBack() and only their prototypes are declared in
      the STM32H743I_EVAL_AUDIO.h file. (refer to the example for more details on the callbacks implementations)

   + The driver API and the callback functions are at the end of the STM32H743I_EVAL_AUDIO.h file.
  Known Limitations:
  ------------------
   1- If the TDM Format used to play in parallel 2 audio Stream (the first Stream is configured in codec SLOT0 and second
      Stream in SLOT1) the Pause/Resume, volume and mute feature will control the both streams.
   2- Parsing of audio file is not implemented (in order to determine audio file properties: Mono/Stereo, Data size,
      File size, Audio Frequency, Audio Data header size ...). The configuration is fixed for the given audio file.

  @endverbatim
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32h743i_eval_audio.h"
#include "stm32h743i_eval_bus.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H743I_EVAL
  * @{
  */

/** @defgroup STM32H743I_EVAL_AUDIO AUDIO
  * @brief This file includes the low layer driver for wm8994 Audio Codec
  *        available on STM32H743I_EVAL board(MB1246).
  * @{
  */
/** @defgroup STM32H743I_EVAL_AUDIO_Private_Defines Private Defines
  * @{
  */
/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_AUDIO_Private_Macros Private Macros
  * @{
  */
/*### RECORD ###*/
#define DFSDM_OVER_SAMPLING(__FREQUENCY__) \
        ((__FREQUENCY__) == (AUDIO_FREQUENCY_8K))  ? (256U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_11K)) ? (256U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_16K)) ? (128U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_22K)) ? (128U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_32K)) ? (64U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_44K)) ? (64U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_48K)) ? (40U) : (20U)

#define DFSDM_CLOCK_DIVIDER(__FREQUENCY__) \
        ((__FREQUENCY__) == (AUDIO_FREQUENCY_8K))  ? (24U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_11K)) ? (4U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_16K)) ? (24U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_22K)) ? (4U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_32K)) ? (24U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_44K)) ? (4U)  \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_48K)) ? (25U) : (25U)

#define DFSDM_FILTER_ORDER(__FREQUENCY__) \
        ((__FREQUENCY__) == (AUDIO_FREQUENCY_8K))  ? (DFSDM_FILTER_SINC3_ORDER) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_11K)) ? (DFSDM_FILTER_SINC3_ORDER) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_16K)) ? (DFSDM_FILTER_SINC3_ORDER) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_22K)) ? (DFSDM_FILTER_SINC3_ORDER) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_32K)) ? (DFSDM_FILTER_SINC4_ORDER) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_44K)) ? (DFSDM_FILTER_SINC3_ORDER) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_48K)) ? (DFSDM_FILTER_SINC3_ORDER) : (DFSDM_FILTER_SINC5_ORDER)

#define DFSDM_MIC_BIT_SHIFT(__FREQUENCY__) \
        ((__FREQUENCY__) == (AUDIO_FREQUENCY_8K))  ? (6U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_11K)) ? (8U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_16K)) ? (3U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_22K)) ? (4U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_32K)) ? (7U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_44K)) ? (0U) \
      : ((__FREQUENCY__) == (AUDIO_FREQUENCY_48K)) ? (0U) : (4U)

/* Saturate the record PCM sample */
#define SaturaLH(N, L, H) (((N)<(L))?(L):(((N)>(H))?(H):(N)))
/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_AUDIO_Exported_Variables Exported Variables
  * @{
  */

/* Play  */
SAI_HandleTypeDef                      haudio_out_sai = {0};
AUDIO_OUT_Ctx_t                        Audio_Out_Ctx[AUDIO_OUT_INSTANCES_NBR];

/* Record */
DFSDM_Filter_HandleTypeDef             haudio_in_dfsdm_filter[DFSDM_MIC_NUMBER];
DFSDM_Channel_HandleTypeDef            haudio_in_dfsdm_channel[DFSDM_MIC_NUMBER];
SAI_HandleTypeDef                      haudio_in_sai;
AUDIO_IN_Ctx_t                         Audio_In_Ctx[AUDIO_IN_INSTANCES_NBR];

/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_AUDIO_Private_Variables Private Variables
  * @{
  */
static AUDIO_Drv_t                     *Audio_Drv = NULL;
/* Audio in and out component object */
void                            *Audio_CompObj = NULL;
/* Recording DFSDM DMA handles */
static DMA_HandleTypeDef               hDmaDfsdm[DFSDM_MIC_NUMBER];

/* Recording Buffer Trigger */
static __IO uint32_t                   RecBuffTrigger          = 0;
static __IO uint32_t                   RecBuffHalf             = 0;
ALIGN_32BYTES (static int32_t          MicRecBuff[2][DEFAULT_AUDIO_IN_BUFFER_SIZE]);

/* PDM filters params */
static PDM_Filter_Handler_t            PDM_FilterHandler[2];
static PDM_Filter_Config_t             PDM_FilterConfig[2];
/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_AUDIO_Private_Function_Prototypes Private Function Prototypes
  * @{
  */
/* SAI Msp config */
static void SAI_MspInit(SAI_HandleTypeDef *hsai);
static void SAI_MspDeInit(SAI_HandleTypeDef *hsai);
static void SAI_InitMXConfigStruct(SAI_HandleTypeDef* hsai, MX_SAI_Config *MXConfig);

/* SAI callbacks */
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1U)
static void SAI_TxCpltCallback(SAI_HandleTypeDef *hsai);
static void SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai);
static void SAI_RxCpltCallback(SAI_HandleTypeDef *hsai);
static void SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai);
static void SAI_ErrorCallback(SAI_HandleTypeDef *hsai);
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1U) */

/* DFSDM Channel Msp config */
static void DFSDM_ChannelMspInit(DFSDM_Channel_HandleTypeDef *hDfsdmChannel);
static void DFSDM_ChannelMspDeInit(DFSDM_Channel_HandleTypeDef *hDfsdmChannel);

/* DFSDM Filter Msp config */
static void DFSDM_FilterMspInit(DFSDM_Filter_HandleTypeDef *hDfsdmFilter);
static void DFSDM_FilterMspDeInit(DFSDM_Filter_HandleTypeDef *hDfsdmFilter);

/* DFSDM Filter conversion callbacks */
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
static void DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter);
static void DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter);
#endif /* (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1) */

#if (USE_AUDIO_CODEC_WM8994 == 1)
static int32_t WM8994_Probe(void);
#endif
#if (USE_AUDIO_CODEC_ADV7533 == 1)
static int32_t ADV7533_Probe(void);
#endif

/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_AUDIO_OUT_Exported_Functions AUDIO_OUT Exported Functions
  * @{
  */

/**
  * @brief  Configures the audio peripherals.
  * @param  Instance   AUDIO OUT Instance. It can only be 0 (SAI)
  * @param  AudioInit  AUDIO OUT init Structure
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_Init(uint32_t Instance, BSP_AUDIO_Init_t* AudioInit)
{
 if(Instance >= AUDIO_OUT_INSTANCES_NBR)
 {
   return BSP_ERROR_WRONG_PARAM;
 }
 else
 {
   /* Fill Audio_Out_Ctx structure */
   Audio_Out_Ctx[Instance].Device         = AudioInit->Device;
   Audio_Out_Ctx[Instance].Instance       = Instance;
   Audio_Out_Ctx[Instance].SampleRate     = AudioInit->SampleRate;
   Audio_Out_Ctx[Instance].BitsPerSample  = AudioInit->BitsPerSample;
   Audio_Out_Ctx[Instance].ChannelsNbr    = AudioInit->ChannelsNbr;
   Audio_Out_Ctx[Instance].Volume         = AudioInit->Volume;
   Audio_Out_Ctx[Instance].State          = AUDIO_OUT_STATE_RESET;

   if(AudioInit->Device != AUDIO_OUT_DEVICE_HDMI)
   {
#if (USE_AUDIO_CODEC_WM8994 == 1)
     if(WM8994_Probe() != BSP_ERROR_NONE)
     {
       return BSP_ERROR_COMPONENT_FAILURE;
     }
#endif
   }

   /* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */
   if(MX_SAI1_ClockConfig(&haudio_out_sai, AudioInit->SampleRate) != HAL_OK)
   {
     return BSP_ERROR_CLOCK_FAILURE;
   }
   /* SAI data transfer preparation:
   Prepare the Media to be used for the audio transfer from memory to SAI peripheral */
   haudio_out_sai.Instance = AUDIO_OUT_SAIx;

#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1U)
   /* Register the SAI MSP Callbacks */
   if(Audio_Out_Ctx[Instance].IsMspCallbacksValid == 0U)
   {
     if(BSP_AUDIO_OUT_RegisterDefaultMspCallbacks(Instance) != BSP_ERROR_NONE)
     {
       return BSP_ERROR_PERIPH_FAILURE;
     }
   }
#else
   SAI_MspInit(&haudio_out_sai);
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1U) */

   MX_SAI_Config mx_sai_config;

   /* Prepare haudio_out_sai handle */
   mx_sai_config.AudioFrequency    = AudioInit->SampleRate;
   mx_sai_config.AudioMode         = SAI_MODEMASTER_TX;
   mx_sai_config.ClockStrobing     = SAI_CLOCKSTROBING_RISINGEDGE;
   mx_sai_config.MonoStereoMode    = SAI_STEREOMODE;
   mx_sai_config.DataSize          = SAI_DATASIZE_16;
   mx_sai_config.FrameLength       = 128;
   mx_sai_config.ActiveFrameLength = 64;
   if (AudioInit->BitsPerSample == AUDIO_RESOLUTION_32B)
   {
     mx_sai_config.DataSize          = SAI_DATASIZE_32;
     mx_sai_config.FrameLength       = 128;
     mx_sai_config.ActiveFrameLength = 64;
   }
   if(AudioInit->ChannelsNbr == 1U)
   {
     mx_sai_config.MonoStereoMode = SAI_MONOMODE;
   }
   mx_sai_config.OutputDrive       = SAI_OUTPUTDRIVE_ENABLE;
   mx_sai_config.Synchro           = SAI_ASYNCHRONOUS;
   mx_sai_config.SynchroExt        = SAI_SYNCEXT_DISABLE;

   switch(AudioInit->Device)
   {
   case AUDIO_OUT_DEVICE_SPK_HP:
   case AUDIO_OUT_DEVICE_AUTO:
     mx_sai_config.SlotActive         = CODEC_AUDIOFRAME_SLOT_0123;
     break;
   case AUDIO_OUT_DEVICE_SPEAKER:
     mx_sai_config.SlotActive         = CODEC_AUDIOFRAME_SLOT_13;
     break;
   case AUDIO_OUT_DEVICE_HEADPHONE:
   default:
     mx_sai_config.SlotActive         = CODEC_AUDIOFRAME_SLOT_02;
     break;
   }

   /* SAI peripheral initialization: this __weak function can be redefined by the application  */
   if(MX_SAI1_Block_A_Init(&haudio_out_sai, &mx_sai_config) != HAL_OK)
   {
     return BSP_ERROR_PERIPH_FAILURE;
   }
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1U)
   /* Register SAI TC, HT and Error callbacks */
   if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_TX_COMPLETE_CB_ID, SAI_TxCpltCallback) != HAL_OK)
   {
     return BSP_ERROR_PERIPH_FAILURE;
   }
   if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_TX_HALFCOMPLETE_CB_ID, SAI_TxHalfCpltCallback) != HAL_OK)
   {
     return BSP_ERROR_PERIPH_FAILURE;
   }
   if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_ERROR_CB_ID, SAI_ErrorCallback) != HAL_OK)
   {
     return BSP_ERROR_PERIPH_FAILURE;
   }
#endif


#if (USE_AUDIO_CODEC_WM8994 == 1)
   WM8994_Init_t codec_init;
   if(AudioInit->Device != AUDIO_OUT_DEVICE_HDMI)
   {
     switch (AudioInit->BitsPerSample)
     {
     case AUDIO_RESOLUTION_32B:
       codec_init.Resolution   = 3;
       break;

     case AUDIO_RESOLUTION_16B:
     default:
       codec_init.Resolution   = 0;
       break;
     }
     /* Fill codec_init structure */
     codec_init.Frequency    = AudioInit->SampleRate;
     codec_init.InputDevice  = WM8994_IN_NONE;
     codec_init.OutputDevice = AudioInit->Device;

     /* Convert volume before sending to the codec */
     codec_init.Volume       = VOLUME_OUT_CONVERT(AudioInit->Volume);

     /* Initialize the codec internal registers */
     if(Audio_Drv->Init(Audio_CompObj, &codec_init) != 0)
     {
       return BSP_ERROR_COMPONENT_FAILURE;
     }
   }
#endif
#if (USE_AUDIO_CODEC_ADV7533 == 1)
    if(ADV7533_Probe() == BSP_ERROR_NONE)
    {
      ADV7533_Init_t codec_init;

      /* Fill codec_init structure */
      codec_init.Frequency    = AudioInit->SampleRate;

      /* Initialize the codec internal registers */
      if(Audio_Drv->Init(Audio_CompObj, &codec_init) != 0)
      {
        return BSP_ERROR_COMPONENT_FAILURE;
      }
    }
#endif
    /* Update BSP AUDIO OUT state */
    Audio_Out_Ctx[Instance].State = AUDIO_OUT_STATE_STOP;
  }

  return BSP_ERROR_NONE;
}

/**
  * @brief  De-initializes the audio out peripheral.
  * @param  Instance : AUDIO OUT Instance. It can only be 0 (SAI)
  * @retval None
  */
int32_t BSP_AUDIO_OUT_DeInit(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 0U)
    SAI_MspDeInit(&haudio_out_sai);
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 0U) */
    /* Initialize the haudio_out_sai Instance parameter */
    haudio_out_sai.Instance = AUDIO_OUT_SAIx;
    /* Call the Media layer stop function */
    if(Audio_Drv->DeInit(Audio_CompObj) != 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else if(HAL_SAI_DeInit(&haudio_out_sai) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      /* Update BSP AUDIO OUT state */
      Audio_Out_Ctx[Instance].State = AUDIO_OUT_STATE_RESET;
    }
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Initializes the Audio Codec audio out instance (SAI).
  * @param  hsai SAI handle
  * @param  MXConfig SAI configuration structure
  * @note   Being __weak it can be overwritten by the application
  * @retval HAL status
  */
__weak HAL_StatusTypeDef MX_SAI1_Block_A_Init(SAI_HandleTypeDef* hsai, MX_SAI_Config *MXConfig)
{
  HAL_StatusTypeDef ret = HAL_OK;

  /* Disable SAI peripheral to allow access to SAI internal registers */
  __HAL_SAI_DISABLE(hsai);

  /* Configure SAI1_Block_A */
  hsai->Init.MonoStereoMode       = MXConfig->MonoStereoMode;
  hsai->Init.AudioFrequency       = MXConfig->AudioFrequency;
  hsai->Init.AudioMode            = MXConfig->AudioMode;
  hsai->Init.NoDivider            = SAI_MASTERDIVIDER_ENABLE;
  hsai->Init.Protocol             = SAI_FREE_PROTOCOL;
  hsai->Init.DataSize             = MXConfig->DataSize;
  hsai->Init.FirstBit             = SAI_FIRSTBIT_MSB;
  hsai->Init.ClockStrobing        = MXConfig->ClockStrobing;
  hsai->Init.Synchro              = MXConfig->Synchro;
  hsai->Init.OutputDrive          = MXConfig->OutputDrive;
  hsai->Init.FIFOThreshold        = SAI_FIFOTHRESHOLD_1QF;
  hsai->Init.SynchroExt           = MXConfig->SynchroExt;
  hsai->Init.CompandingMode       = SAI_NOCOMPANDING;
  hsai->Init.TriState             = SAI_OUTPUT_NOTRELEASED;
  hsai->Init.Mckdiv               = 0;
  hsai->Init.MckOverSampling      = SAI_MCK_OVERSAMPLING_DISABLE;
  hsai->Init.MckOutput            = SAI_MCK_OUTPUT_DISABLE;
  hsai->Init.PdmInit.Activation   = DISABLE;
  hsai->Init.PdmInit.ClockEnable  = 0;
  hsai->Init.PdmInit.MicPairsNbr  = 0;

  /* Configure SAI_Block_x Frame */
  hsai->FrameInit.FrameLength       = MXConfig->FrameLength;
  hsai->FrameInit.ActiveFrameLength = MXConfig->ActiveFrameLength;
  hsai->FrameInit.FSDefinition      = SAI_FS_CHANNEL_IDENTIFICATION;
  hsai->FrameInit.FSPolarity        = SAI_FS_ACTIVE_LOW;
  hsai->FrameInit.FSOffset          = SAI_FS_BEFOREFIRSTBIT;

  /* Configure SAI Block_x Slot */
  hsai->SlotInit.FirstBitOffset     = 0;
  hsai->SlotInit.SlotSize           = SAI_SLOTSIZE_DATASIZE;
  hsai->SlotInit.SlotNumber         = 4;
  hsai->SlotInit.SlotActive         = MXConfig->SlotActive;

  if(HAL_SAI_Init(hsai) != HAL_OK)
  {
    ret = HAL_ERROR;
  }

  __HAL_SAI_ENABLE(hsai);

  return ret;
}

/**
  * @brief  SAI clock Config.
  * @param  hsai SAI handle
  * @param  SampleRate  Audio frequency used to play the audio stream.
  * @note   This API is called by BSP_AUDIO_OUT_Init() and BSP_AUDIO_OUT_SetFrequency()
  *         Being __weak it can be overwritten by the application
  * @retval HAL status
  */
__weak HAL_StatusTypeDef MX_SAI1_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t SampleRate)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsai);

  HAL_StatusTypeDef ret = HAL_OK;
  RCC_PeriphCLKInitTypeDef rcc_ex_clk_init_struct;
  HAL_RCCEx_GetPeriphCLKConfig(&rcc_ex_clk_init_struct);

  /* Set the PLL configuration according to the audio frequency */
  if((SampleRate == AUDIO_FREQUENCY_11K) || (SampleRate == AUDIO_FREQUENCY_22K) || (SampleRate == AUDIO_FREQUENCY_44K))
  {
    rcc_ex_clk_init_struct.PLL2.PLL2P = 38;
    rcc_ex_clk_init_struct.PLL2.PLL2N = 429;
  }
  else /* AUDIO_FREQUENCY_8K, AUDIO_FREQUENCY_16K, AUDIO_FREQUENCY_32K, AUDIO_FREQUENCY_48K, AUDIO_FREQUENCY_96K */
  {
    rcc_ex_clk_init_struct.PLL2.PLL2P = 7;
    rcc_ex_clk_init_struct.PLL2.PLL2N = 344;
  }
  rcc_ex_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
  rcc_ex_clk_init_struct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL2;
  rcc_ex_clk_init_struct.PLL2.PLL2Q = 1;
  rcc_ex_clk_init_struct.PLL2.PLL2R = 1;
  rcc_ex_clk_init_struct.PLL2.PLL2M = 25;
  if(HAL_RCCEx_PeriphCLKConfig(&rcc_ex_clk_init_struct) != HAL_OK)
  {
    ret = HAL_ERROR;
  }

  return ret;
}


/**
  * @brief  SAI clock Config.
  * @param  hsai SAI handle
  * @param  SampleRate  Audio frequency used to play the audio stream.
  * @note   This API is called by BSP_AUDIO_OUT_Init() and BSP_AUDIO_OUT_SetFrequency()
  *         Being __weak it can be overwritten by the application
  * @retval HAL status
  */
__weak HAL_StatusTypeDef MX_SAI4_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t SampleRate)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsai);

  HAL_StatusTypeDef ret = HAL_OK;
  RCC_PeriphCLKInitTypeDef rcc_ex_clk_init_struct;
  HAL_RCCEx_GetPeriphCLKConfig(&rcc_ex_clk_init_struct);

  /* Set the PLL configuration according to the audio frequency */
  if((SampleRate == AUDIO_FREQUENCY_11K) || (SampleRate == AUDIO_FREQUENCY_22K) || (SampleRate == AUDIO_FREQUENCY_44K))
  {
    rcc_ex_clk_init_struct.PLL2.PLL2P = 38;
    rcc_ex_clk_init_struct.PLL2.PLL2N = 429;
  }
  else /* AUDIO_FREQUENCY_8K, AUDIO_FREQUENCY_16K, AUDIO_FREQUENCY_32K, AUDIO_FREQUENCY_48K, AUDIO_FREQUENCY_96K */
  {
    rcc_ex_clk_init_struct.PLL2.PLL2P = 7;
    rcc_ex_clk_init_struct.PLL2.PLL2N = 344;
  }
  /* SAI clock config */
  rcc_ex_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_SAI4A;
  rcc_ex_clk_init_struct.Sai4AClockSelection = RCC_SAI4ACLKSOURCE_PLL2;
  rcc_ex_clk_init_struct.PLL2.PLL2Q = 1;
  rcc_ex_clk_init_struct.PLL2.PLL2R = 1;
  rcc_ex_clk_init_struct.PLL2.PLL2M = 25;
  if(HAL_RCCEx_PeriphCLKConfig(&rcc_ex_clk_init_struct) != HAL_OK)
  {
    ret = HAL_ERROR;
  }

  return ret;
}

/**
  * @brief Default BSP AUDIO OUT Msp Callbacks
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @retval BSP status
  */
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1U)
int32_t BSP_AUDIO_OUT_RegisterDefaultMspCallbacks (uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    __HAL_SAI_RESET_HANDLE_STATE(&haudio_out_sai);

    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_MSPINIT_CB_ID,SAI_MspInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_MSPDEINIT_CB_ID,SAI_MspDeInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      Audio_Out_Ctx[Instance].IsMspCallbacksValid = 1;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief BSP AUDIO OUT Msp Callback registering
  * @param Instance  AUDIO OUT Instance. It can only be 0
  * @param CallBacks   pointer to MspInit/MspDeInit callbacks functions
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_RegisterMspCallbacks (uint32_t Instance, BSP_AUDIO_OUT_Cb_t *CallBacks)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    __HAL_SAI_RESET_HANDLE_STATE(&haudio_out_sai);

    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_MSPINIT_CB_ID, CallBacks->pMspInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_MSPDEINIT_CB_ID, CallBacks->pMspDeInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      Audio_Out_Ctx[Instance].IsMspCallbacksValid = 1;
    }
  }

  /* Return BSP status */
  return ret;
}
#endif /*(USE_HAL_SAI_REGISTER_CALLBACKS == 1U)*/

/**
  * @brief  Starts playing audio stream from a data buffer for a determined size.
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @param  pData         pointer on data address
  * @param  NbrOfBytes   Size of total samples in bytes
  *                      BitsPerSample: 16 or 32
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_Play(uint32_t Instance, uint8_t* pData, uint32_t NbrOfBytes)
{
  int32_t ret = BSP_ERROR_NONE;

  if((Instance >= AUDIO_OUT_INSTANCES_NBR) || (((NbrOfBytes / (Audio_Out_Ctx[Instance].BitsPerSample/8U)) > 0xFFFFU)))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if((Audio_Out_Ctx[Instance].State == AUDIO_OUT_STATE_STOP) || (Audio_Out_Ctx[Instance].State == AUDIO_OUT_STATE_RESET))
  {
    if(HAL_SAI_Transmit_DMA(&haudio_out_sai, pData, (uint16_t)(NbrOfBytes /(Audio_Out_Ctx[Instance].BitsPerSample/8U))) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    if(ret == BSP_ERROR_NONE)
    {
      if(Audio_Drv->Play(Audio_CompObj) < 0)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else
      {
        /* Update BSP AUDIO OUT state */
        Audio_Out_Ctx[Instance].State = AUDIO_OUT_STATE_PLAYING;
      }
    }
  }
  else
  {
    ret = BSP_ERROR_BUSY;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  This function Pauses the audio file stream. In case
  *         of using DMA, the DMA Pause feature is used.
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @note   When calling BSP_AUDIO_OUT_Pause() function for pause, only
  *          BSP_AUDIO_OUT_Resume() function should be called for resume (use of BSP_AUDIO_OUT_Play()
  *          function for resume could lead to unexpected behavior).
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_Pause(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Call the Media layer pause function */
    if(HAL_SAI_DMAPause(&haudio_out_sai) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }

    if(ret == BSP_ERROR_NONE)
    {
      /* Call the Audio Codec Pause/Resume function */
      if(Audio_Drv->Pause(Audio_CompObj) < 0)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else
      {
        /* Update BSP AUDIO OUT state */
        Audio_Out_Ctx[Instance].State = AUDIO_OUT_STATE_PAUSE;
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief   Resumes the audio file stream.
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @note    When calling BSP_AUDIO_OUT_Pause() function for pause, only
  *          BSP_AUDIO_OUT_Resume() function should be called for resume (use of BSP_AUDIO_OUT_Play()
  *          function for resume could lead to unexpected behavior).
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_Resume(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Call the Media layer pause/resume function */
    if(HAL_SAI_DMAResume(&haudio_out_sai) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }

    if(ret == BSP_ERROR_NONE)
    {
      if(Audio_Drv->Resume(Audio_CompObj) < 0)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else
      {
        /* Update BSP AUDIO OUT state */
        Audio_Out_Ctx[Instance].State = AUDIO_OUT_STATE_PLAYING;
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Stops audio playing and Power down the Audio Codec.
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_Stop(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if (Audio_Out_Ctx[Instance].State == AUDIO_OUT_STATE_PLAYING)
  {
    /* Call the Media layer stop function */
    if(Audio_Drv->Stop(Audio_CompObj, CODEC_PDWN_SW) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      if(HAL_SAI_DMAStop(&haudio_out_sai)!= HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }

      if(ret == BSP_ERROR_NONE)
      {
        /* Update BSP AUDIO OUT state */
        Audio_Out_Ctx[Instance].State = AUDIO_OUT_STATE_STOP;
      }
    }
  }
  else
  {
    ret = BSP_ERROR_BUSY;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Controls the current audio volume level.
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @param  Volume    Volume level to be set in percentage from 0% to 100% (0 for
  *         Mute and 100 for Max volume level).
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_SetVolume(uint32_t Instance, uint32_t Volume)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Call the codec volume control function with converted volume value */
    if(Audio_Drv->SetVolume(Audio_CompObj, AUDIO_VOLUME_OUTPUT, VOLUME_OUT_CONVERT(Volume)) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else if(Volume == 0U)
    {
      /* Update Mute State */
      Audio_Out_Ctx[Instance].IsMute = BSP_AUDIO_MUTE_ENABLED;
    }
    else
    {
      /* Update Mute State */
      Audio_Out_Ctx[Instance].IsMute = BSP_AUDIO_MUTE_DISABLED;
    }
   Audio_Out_Ctx[Instance].Volume = Volume;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the current audio volume level.
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @param  Volume    pointer to volume to be returned
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_GetVolume(uint32_t Instance, uint32_t *Volume)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    *Volume = Audio_Out_Ctx[Instance].Volume;
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Enables the MUTE
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_Mute(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Call the Codec Mute function */
    if(Audio_Drv->SetMute(Audio_CompObj, CODEC_MUTE_ON) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Update Mute State */
      Audio_Out_Ctx[Instance].IsMute = BSP_AUDIO_MUTE_ENABLED;
    }
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Disables the MUTE mode
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_UnMute(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Call the Codec Mute function */
    if(Audio_Drv->SetMute(Audio_CompObj, CODEC_MUTE_OFF) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Update Mute State */
      Audio_Out_Ctx[Instance].IsMute = BSP_AUDIO_MUTE_DISABLED;
    }
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Check whether the MUTE mode is enabled or not
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @param  IsMute    pointer to mute state
  * @retval Mute status
  */
int32_t BSP_AUDIO_OUT_IsMute(uint32_t Instance, uint32_t *IsMute)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    *IsMute = Audio_Out_Ctx[Instance].IsMute;
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Switch dynamically (while audio file is played) the output target
  *         (speaker or headphone).
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @param  Device  The audio output device
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_SetDevice(uint32_t Instance, uint32_t Device)
{
  int32_t ret;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Audio_Out_Ctx[Instance].State != AUDIO_OUT_STATE_PLAYING)
  {
    /* Call the Codec output device function */
    if(Audio_Drv->SetOutputMode(Audio_CompObj, Device) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      MX_SAI_Config mx_out_config;
      /* Get SAI MX configuration */
      SAI_InitMXConfigStruct(&haudio_out_sai, &mx_out_config);
      switch(Device)
      {
      case AUDIO_OUT_DEVICE_SPK_HP:
      case AUDIO_OUT_DEVICE_AUTO:
        mx_out_config.SlotActive         = CODEC_AUDIOFRAME_SLOT_0123;
        break;
      case AUDIO_OUT_DEVICE_SPEAKER:
        mx_out_config.SlotActive         = CODEC_AUDIOFRAME_SLOT_13;
        break;
      case AUDIO_OUT_DEVICE_HEADPHONE:
      default:
        mx_out_config.SlotActive         = CODEC_AUDIOFRAME_SLOT_02;
        break;
      }
      if(MX_SAI1_Block_A_Init(&haudio_out_sai, &mx_out_config) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1U)
        /* Register SAI TC, HT and Error callbacks */
        if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_TX_COMPLETE_CB_ID, SAI_TxCpltCallback) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_TX_HALFCOMPLETE_CB_ID, SAI_TxHalfCpltCallback) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_ERROR_CB_ID, SAI_ErrorCallback) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else
        {
          ret = BSP_ERROR_NONE;
        }
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1U) */
        /* Update Audio_Out_Ctx structure */
        Audio_Out_Ctx[Instance].Device = Device;
      }
    }
  }
  else
  {
    ret = BSP_ERROR_BUSY;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the Output Device
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @param  Device    The audio output device
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_GetDevice(uint32_t Instance, uint32_t *Device)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Get Audio_Out_Ctx Device */
    *Device = Audio_Out_Ctx[Instance].Device;
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Updates the audio frequency.
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @param  SampleRate Audio frequency used to play the audio stream.
  * @note   This API should be called after the BSP_AUDIO_OUT_Init() to adjust the
  *         audio frequency.
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_SetSampleRate(uint32_t Instance, uint32_t SampleRate)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Audio_Out_Ctx[Instance].State != AUDIO_OUT_STATE_PLAYING)
  {
    /* Call the Codec output device function */
    if(Audio_Drv->SetFrequency(Audio_CompObj, SampleRate) != 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      MX_SAI_Config mx_sai_config;

      /* Get SAI MX configuration */
      SAI_InitMXConfigStruct(&haudio_out_sai, &mx_sai_config);
      /* Update MX config structure */
      mx_sai_config.AudioFrequency = SampleRate;
      /* Update the SAI audio frequency configuration */
      haudio_out_sai.Init.AudioFrequency = SampleRate;
      /* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */
      if(MX_SAI1_ClockConfig(&haudio_out_sai, SampleRate) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else if(MX_SAI1_Block_A_Init(&haudio_out_sai, &mx_sai_config) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1U)
        /* Register SAI TC, HT and Error callbacks */
        if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_TX_COMPLETE_CB_ID, SAI_TxCpltCallback) != HAL_OK)
        {
          return BSP_ERROR_PERIPH_FAILURE;
        }
        if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_TX_HALFCOMPLETE_CB_ID, SAI_TxHalfCpltCallback) != HAL_OK)
        {
          return BSP_ERROR_PERIPH_FAILURE;
        }
        if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_ERROR_CB_ID, SAI_ErrorCallback) != HAL_OK)
        {
          return BSP_ERROR_PERIPH_FAILURE;
        }
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1U) */

        /* Store new sample rate */
        Audio_Out_Ctx[Instance].SampleRate = SampleRate;
      }
    }
  }
  else
  {
    ret = BSP_ERROR_BUSY;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the audio frequency.
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @param  SampleRate  Audio frequency used to play the audio stream.
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_GetSampleRate(uint32_t Instance, uint32_t *SampleRate)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    *SampleRate = Audio_Out_Ctx[Instance].SampleRate;
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the audio Resolution.
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @param  BitsPerSample  Audio Resolution used to play the audio stream.
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_SetBitsPerSample(uint32_t Instance, uint32_t BitsPerSample)
{
  int32_t ret = BSP_ERROR_NONE;
  uint16_t resolution;
  MX_SAI_Config mx_out_config;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Audio_Out_Ctx[Instance].State != AUDIO_OUT_STATE_PLAYING)
  {
    /* Set audio Out resolution */
    if (BitsPerSample == AUDIO_RESOLUTION_32B)
    {
      resolution   = 3;
    }
    else /* AUDIO_RESOLUTION_16B */
    {
      resolution   = 0;
    }
    /* Call the Codec output device function */
    if(Audio_Drv->SetResolution(Audio_CompObj, resolution) != 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Get SAI MX configuration */
      SAI_InitMXConfigStruct(&haudio_out_sai, &mx_out_config);

      if (BitsPerSample == AUDIO_RESOLUTION_32B)
      {
        mx_out_config.DataSize          = SAI_DATASIZE_32;
        mx_out_config.FrameLength       = 128;
        mx_out_config.ActiveFrameLength = 64;
      }
      else /* AUDIO_RESOLUTION_16B */
      {
        mx_out_config.DataSize          = SAI_DATASIZE_16;
        mx_out_config.FrameLength       = 64;
        mx_out_config.ActiveFrameLength = 32;
      }

      /* Update Audio_Out_Ctx structure as required in Msp Init */
      Audio_Out_Ctx[Instance].BitsPerSample = BitsPerSample;

      if(MX_SAI1_Block_A_Init(&haudio_out_sai, &mx_out_config) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1U)
      /* Register SAI TC, HT and Error callbacks */
      if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_TX_COMPLETE_CB_ID, SAI_TxCpltCallback) != HAL_OK)
      {
        return BSP_ERROR_PERIPH_FAILURE;
      }
      if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_TX_HALFCOMPLETE_CB_ID, SAI_TxHalfCpltCallback) != HAL_OK)
      {
        return BSP_ERROR_PERIPH_FAILURE;
      }
      if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_ERROR_CB_ID, SAI_ErrorCallback) != HAL_OK)
      {
        return BSP_ERROR_PERIPH_FAILURE;
      }
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1U) */
    }
  }
  else
  {
    ret = BSP_ERROR_BUSY;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the audio Resolution.
  * @param  Instance : AUDIO OUT Instance. It can only be 0 (SAI)
  * @param  BitsPerSample : Audio Resolution used to play the audio stream.
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_GetBitsPerSample(uint32_t Instance, uint32_t *BitsPerSample)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Get audio Out resolution */
    *BitsPerSample = Audio_Out_Ctx[Instance].BitsPerSample;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set the audio Channels number.
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @param  ChannelNbr  Audio Channels number used to play the audio stream.
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_SetChannelsNbr(uint32_t Instance, uint32_t ChannelNbr)
{
  int32_t ret = BSP_ERROR_NONE;

  if((Instance >= AUDIO_OUT_INSTANCES_NBR) || (ChannelNbr > 2U))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Audio_Out_Ctx[Instance].State != AUDIO_OUT_STATE_PLAYING)
  {
    MX_SAI_Config mx_out_config;
    /* Get SAI MX configuration */
    SAI_InitMXConfigStruct(&haudio_out_sai, &mx_out_config);
    /* Set the audio Channels number */
    if(ChannelNbr == 1U)
    {
      mx_out_config.MonoStereoMode = SAI_MONOMODE;
    }
    else
    {
      mx_out_config.MonoStereoMode = SAI_STEREOMODE;
    }
    if(MX_SAI1_Block_A_Init(&haudio_out_sai, &mx_out_config) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1U)
    /* Register SAI TC, HT and Error callbacks */
    if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_TX_COMPLETE_CB_ID, SAI_TxCpltCallback) != HAL_OK)
    {
      return BSP_ERROR_PERIPH_FAILURE;
    }
    if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_TX_HALFCOMPLETE_CB_ID, SAI_TxHalfCpltCallback) != HAL_OK)
    {
      return BSP_ERROR_PERIPH_FAILURE;
    }
    if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_ERROR_CB_ID, SAI_ErrorCallback) != HAL_OK)
    {
      return BSP_ERROR_PERIPH_FAILURE;
    }
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1U) */

    /* Store new Channel number */
    Audio_Out_Ctx[Instance].ChannelsNbr = ChannelNbr;
    }
  }
  else
  {
    ret = BSP_ERROR_BUSY;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the audio Channels number.
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @param  ChannelNbr     Audio Channels number used to play the audio stream.
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_GetChannelsNbr(uint32_t Instance, uint32_t *ChannelNbr)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Get the audio Channels number */
    *ChannelNbr = Audio_Out_Ctx[Instance].ChannelsNbr;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get Audio Out state
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @param  State     Audio Out state
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_GetState(uint32_t Instance, uint32_t *State)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
  /* Return audio Output State */
  *State = Audio_Out_Ctx[Instance].State;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  This function handles Audio Out DMA interrupt requests.
  * @param  Instance Audio OUT instance
  * @retval None
  */
void BSP_AUDIO_OUT_IRQHandler(uint32_t Instance)
{
  if(Instance == 0U)
  {
    HAL_DMA_IRQHandler(haudio_out_sai.hdmatx);
  }
}

#if (USE_HAL_SAI_REGISTER_CALLBACKS == 0U)
/**
  * @brief  Tx Transfer completed callbacks.
  * @param  hsai SAI handle
  * @retval None
  */
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsai);

  /* Manage the remaining file size and new address offset: This function
     should be coded by user (its prototype is already declared in stm32h743i_eval_audio.h) */
  BSP_AUDIO_OUT_TransferComplete_CallBack(0);
}

/**
  * @brief  Tx Half Transfer completed callbacks.
  * @param  hsai  SAI handle
  * @retval None
  */
void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsai);

  /* Manage the remaining file size and new address offset: This function
     should be coded by user (its prototype is already declared in stm32h743i_eval_audio.h) */
  BSP_AUDIO_OUT_HalfTransfer_CallBack(0);
}

/**
  * @brief  SAI error callbacks.
  * @param  hsai  SAI handle
  * @retval None
  */
void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
  if(hsai->Instance == AUDIO_OUT_SAIx)
  {
  BSP_AUDIO_OUT_Error_CallBack(0);
  }
  else
  {
  BSP_AUDIO_IN_Error_CallBack(0);
  }
}
#endif /* USE_HAL_SAI_REGISTER_CALLBACKS == 0U) */


/**
  * @brief  Manages the DMA full Transfer complete event
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @retval None
  */
__weak void BSP_AUDIO_OUT_TransferComplete_CallBack(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);
}

/**
  * @brief  Manages the DMA Half Transfer complete event
  * @param  Instance : AUDIO OUT Instance It can only be 0 (SAI)
  * @retval None
  */
__weak void BSP_AUDIO_OUT_HalfTransfer_CallBack(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);
}

/**
  * @brief  Manages the DMA FIFO error event
  * @param  Instance : AUDIO OUT Instance. It can only be 0 (SAI)
  * @retval None
  */
__weak void BSP_AUDIO_OUT_Error_CallBack(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);
}

/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_AUDIO_IN_Exported_Functions AUDIO_IN Exported Functions
  * @{
  */

/**
  * @brief  Initialize wave recording.
  * @param  Instance  AUDIO IN Instance. It can be:
  *       - 0 when SAI is used
  *       - 1 if DFSDM is used
  *       - 2 if SAI PDM is used
  * @param  AudioInit Init structure
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_Init(uint32_t Instance, BSP_AUDIO_Init_t* AudioInit)
{
  uint32_t i;
  int32_t ret = BSP_ERROR_NONE;
  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Store the audio record context */
    Audio_In_Ctx[Instance].Device          = AudioInit->Device;
    Audio_In_Ctx[Instance].ChannelsNbr     = AudioInit->ChannelsNbr;
    Audio_In_Ctx[Instance].SampleRate      = AudioInit->SampleRate;
    Audio_In_Ctx[Instance].BitsPerSample   = AudioInit->BitsPerSample;
    Audio_In_Ctx[Instance].Volume          = AudioInit->Volume;
    Audio_In_Ctx[Instance].State           = AUDIO_IN_STATE_RESET;

    if(Instance == 0U)
    {
      /* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */
      if(MX_SAI1_ClockConfig(&haudio_in_sai, AudioInit->SampleRate) != HAL_OK)
      {
        ret = BSP_ERROR_CLOCK_FAILURE;
      }
      haudio_in_sai.Instance = AUDIO_IN_SAIx;
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1U)
      /* Register the default SAI MSP callbacks */
      if(Audio_In_Ctx[Instance].IsMspCallbacksValid == 0U)
      {
        if(BSP_AUDIO_IN_RegisterDefaultMspCallbacks(Instance) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
#else
      SAI_MspInit(&haudio_in_sai);
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1U) */
      MX_SAI_Config mx_config;

      mx_config.MonoStereoMode    = (AudioInit->ChannelsNbr == 1U) ? SAI_MONOMODE : SAI_STEREOMODE;
      mx_config.DataSize          = SAI_DATASIZE_16;
        mx_config.FrameLength       = 128;
        mx_config.ActiveFrameLength = 64;
      if (AudioInit->BitsPerSample == AUDIO_RESOLUTION_32B)
      {
        mx_config.DataSize          = SAI_DATASIZE_32;
        mx_config.FrameLength       = 128;
        mx_config.ActiveFrameLength = 64;
      }
      mx_config.OutputDrive       = SAI_OUTPUTDRIVE_DISABLE;
      switch(AudioInit->Device)
      {
      case AUDIO_IN_DEVICE_ANALOG_MIC:
        mx_config.SlotActive        = CODEC_AUDIOFRAME_SLOT_02;
        break;
      case AUDIO_IN_DEVICE_DIGITAL_MIC:
      default:
        mx_config.SlotActive        = CODEC_AUDIOFRAME_SLOT_13;
        break;
      }

      /* Prepare haudio_in_sai handle */
      haudio_in_sai.Instance      = SAI1_Block_B;
      mx_config.AudioFrequency    = Audio_In_Ctx[Instance].SampleRate;
      mx_config.AudioMode         = SAI_MODESLAVE_RX;
      mx_config.ClockStrobing     = SAI_CLOCKSTROBING_RISINGEDGE;
      mx_config.Synchro           = SAI_SYNCHRONOUS;
      mx_config.SynchroExt        = SAI_SYNCEXT_DISABLE;
      /* Disable SAI peripheral to allow access to SAI internal registers */
      __HAL_SAI_DISABLE(&haudio_in_sai);

      if(MX_SAI1_Block_B_Init(&haudio_in_sai, &mx_config) != HAL_OK)
      {
        /* Return BSP_ERROR_PERIPH_FAILURE when operations are not correctly done */
        ret = BSP_ERROR_PERIPH_FAILURE;
      }

      /* Prepare haudio_out_sai handle */
      haudio_out_sai.Instance     = SAI1_Block_A;
      mx_config.AudioMode         = SAI_MODEMASTER_TX;
      mx_config.ClockStrobing     = SAI_CLOCKSTROBING_FALLINGEDGE;
      mx_config.OutputDrive       = SAI_OUTPUTDRIVE_ENABLE;
      mx_config.Synchro           = SAI_ASYNCHRONOUS;
      mx_config.SynchroExt        = SAI_SYNCEXT_DISABLE;
      mx_config.SlotActive        = CODEC_AUDIOFRAME_SLOT_0123;
      if(MX_SAI1_Block_A_Init(&haudio_out_sai, &mx_config) != HAL_OK)
      {
        /* Return BSP_ERROR_PERIPH_FAILURE when operations are not correctly done */
        ret = BSP_ERROR_PERIPH_FAILURE;
      }

#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1U)
      /* Register SAI TC, HT and Error callbacks */
      if(HAL_SAI_RegisterCallback(&haudio_in_sai, HAL_SAI_RX_COMPLETE_CB_ID, SAI_RxCpltCallback) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      if(HAL_SAI_RegisterCallback(&haudio_in_sai, HAL_SAI_RX_HALFCOMPLETE_CB_ID, SAI_RxHalfCpltCallback) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      if(HAL_SAI_RegisterCallback(&haudio_in_sai, HAL_SAI_ERROR_CB_ID, SAI_ErrorCallback) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1U) */
#if (USE_AUDIO_CODEC_WM8994 == 1U)
      /* Initialize the codec internal registers */
      if(WM8994_Probe() == BSP_ERROR_NONE)
      {
        WM8994_Init_t codec_init;

        /* Fill codec_init structure */
        codec_init.Frequency    = AudioInit->SampleRate;
        codec_init.OutputDevice = WM8994_OUT_NONE;
        if(AudioInit->Device == AUDIO_IN_DEVICE_ANALOG_MIC)
        {
          codec_init.InputDevice = WM8994_IN_LINE1;
        }
        else /* (AudioInit->Device == AUDIO_IN_DIGITAL_MIC) */
        {
          codec_init.InputDevice = WM8994_IN_MIC2;
        }
        switch (AudioInit->BitsPerSample)
        {
        case AUDIO_RESOLUTION_32B:
          codec_init.Resolution   = 3;
          break;

        case AUDIO_RESOLUTION_16B:
        default:
          codec_init.Resolution   = 0;
          break;
        }
        /* Convert volume before sending to the codec */
        codec_init.Volume       = VOLUME_IN_CONVERT(AudioInit->Volume);

        /* Initialize the codec internal registers */
        if(Audio_Drv->Init(Audio_CompObj, &codec_init) != 0)
        {
          ret =BSP_ERROR_COMPONENT_FAILURE;
        }
      }
#endif  /*USE_AUDIO_CODEC_WM8994 == 1)*/
    }
    else if(Instance == 1U)
    {
      DFSDM_Filter_TypeDef* FilterInstnace[DFSDM_MIC_NUMBER] = {AUDIO_DFSDMx_MIC1_FILTER, AUDIO_DFSDMx_MIC2_FILTER};
      DFSDM_Channel_TypeDef* ChannelInstance[DFSDM_MIC_NUMBER] = {AUDIO_DFSDMx_MIC1_CHANNEL, AUDIO_DFSDMx_MIC2_CHANNEL};
      uint32_t DigitalMicPins[DFSDM_MIC_NUMBER] = {DFSDM_CHANNEL_SAME_CHANNEL_PINS, DFSDM_CHANNEL_FOLLOWING_CHANNEL_PINS};
      uint32_t DigitalMicType[DFSDM_MIC_NUMBER] = {DFSDM_CHANNEL_SPI_RISING, DFSDM_CHANNEL_SPI_FALLING};
      uint32_t Channel4Filter[DFSDM_MIC_NUMBER] = {AUDIO_DFSDMx_MIC1_CHANNEL_FOR_FILTER, AUDIO_DFSDMx_MIC2_CHANNEL_FOR_FILTER};
      MX_DFSDM_Config_t dfsdm_config;

      /* PLL clock is set depending on the AudioFreq (44.1khz vs 48khz groups) */
      if(MX_DFSDM1_ClockConfig(&haudio_in_dfsdm_channel[0], AudioInit->SampleRate) != HAL_OK)
      {
        ret = BSP_ERROR_CLOCK_FAILURE;
      }

#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1U)
      /* Register the default DFSDM MSP callbacks */
      if(Audio_In_Ctx[Instance].IsMspCallbacksValid == 0U)
      {
        if(BSP_AUDIO_IN_RegisterDefaultMspCallbacks(Instance) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
#else
      DFSDM_FilterMspInit(&haudio_in_dfsdm_filter[1]);
      DFSDM_ChannelMspInit(&haudio_in_dfsdm_channel[1]);
#endif /* (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1U) */

      for(i = 0; i < DFSDM_MIC_NUMBER; i ++)
      {
        dfsdm_config.FilterInstance  = FilterInstnace[i];
        dfsdm_config.ChannelInstance = ChannelInstance[i];
        dfsdm_config.DigitalMicPins  = DigitalMicPins[i];
        dfsdm_config.DigitalMicType  = DigitalMicType[i];
        dfsdm_config.Channel4Filter  = Channel4Filter[i];
        dfsdm_config.RegularTrigger  = DFSDM_FILTER_SW_TRIGGER;
        if((i == 1U) && (Audio_In_Ctx[Instance].Device == AUDIO_IN_DEVICE_DIGITAL_MIC))
        {
          dfsdm_config.RegularTrigger = DFSDM_FILTER_SYNC_TRIGGER;
        }
        dfsdm_config.SincOrder       = DFSDM_FILTER_ORDER(Audio_In_Ctx[Instance].SampleRate);
        dfsdm_config.Oversampling    = DFSDM_OVER_SAMPLING(Audio_In_Ctx[Instance].SampleRate);
        dfsdm_config.ClockDivider    = DFSDM_CLOCK_DIVIDER(Audio_In_Ctx[Instance].SampleRate);
        dfsdm_config.RightBitShift   = DFSDM_MIC_BIT_SHIFT(Audio_In_Ctx[Instance].SampleRate);

        if(((AudioInit->Device >> i) & AUDIO_IN_DEVICE_DIGITAL_MIC1) == AUDIO_IN_DEVICE_DIGITAL_MIC1)
        {
          /* Default configuration of DFSDM filters and channels */
          if(MX_DFSDM1_Init(&haudio_in_dfsdm_filter[i], &haudio_in_dfsdm_channel[i], &dfsdm_config) != HAL_OK)
          {
            /* Return BSP_ERROR_PERIPH_FAILURE when operations are not correctly done */
             ret = BSP_ERROR_PERIPH_FAILURE;
          }

#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1U)
                /* Register filter regular conversion callbacks */
                if(HAL_DFSDM_Filter_RegisterCallback(&haudio_in_dfsdm_filter[i], HAL_DFSDM_FILTER_REGCONV_COMPLETE_CB_ID, DFSDM_FilterRegConvCpltCallback) != HAL_OK)
                {
                  ret = BSP_ERROR_PERIPH_FAILURE;
                }
                else
                {
                  if(HAL_DFSDM_Filter_RegisterCallback(&haudio_in_dfsdm_filter[i], HAL_DFSDM_FILTER_REGCONV_HALFCOMPLETE_CB_ID, DFSDM_FilterRegConvHalfCpltCallback) != HAL_OK)
                  {
                    ret = BSP_ERROR_PERIPH_FAILURE;
                  }
                }
#endif /* (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1U) */
        }
      }
    }
    else /* Instance = 2 */
    {
      /* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */
      if(MX_SAI4_ClockConfig(&haudio_in_sai, AudioInit->SampleRate) != HAL_OK)
      {
        ret = BSP_ERROR_CLOCK_FAILURE;
      }
      haudio_in_sai.Instance = AUDIO_IN_SAI_PDMx;
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1U)
      /* Register the default SAI MSP callbacks */
      if(Audio_In_Ctx[Instance].IsMspCallbacksValid == 0U)
      {
        if(BSP_AUDIO_IN_RegisterDefaultMspCallbacks(Instance) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
#else
      SAI_MspInit(&haudio_in_sai);
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1U) */
      MX_SAI_Config mx_config;

      /* Prepare haudio_in_sai handle */
      mx_config.MonoStereoMode    = SAI_STEREOMODE;
      mx_config.DataSize          = SAI_DATASIZE_16;
      mx_config.FrameLength       = 16;
      mx_config.ActiveFrameLength = 1;
      mx_config.OutputDrive       = SAI_OUTPUTDRIVE_DISABLE;
      mx_config.SlotActive        = SAI_SLOTACTIVE_0;
      mx_config.AudioFrequency    = Audio_In_Ctx[Instance].SampleRate*8;
      mx_config.AudioMode         = SAI_MODEMASTER_RX;
      mx_config.ClockStrobing     = SAI_CLOCKSTROBING_FALLINGEDGE;
      mx_config.Synchro           = SAI_ASYNCHRONOUS;
      mx_config.SynchroExt        = SAI_SYNCEXT_DISABLE;

      if(MX_SAI4_Block_A_Init(&haudio_in_sai, &mx_config) != HAL_OK)
      {
        /* Return BSP_ERROR_PERIPH_FAILURE when operations are not correctly done */
        ret = BSP_ERROR_PERIPH_FAILURE;
      }

#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1U)
      /* Register SAI TC, HT and Error callbacks */
      if(HAL_SAI_RegisterCallback(&haudio_in_sai, HAL_SAI_RX_COMPLETE_CB_ID, SAI_RxCpltCallback) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      if(HAL_SAI_RegisterCallback(&haudio_in_sai, HAL_SAI_RX_HALFCOMPLETE_CB_ID, SAI_RxHalfCpltCallback) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      if(HAL_SAI_RegisterCallback(&haudio_in_sai, HAL_SAI_ERROR_CB_ID, SAI_ErrorCallback) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1U) */

      if(BSP_AUDIO_IN_PDMToPCM_Init(Instance, AudioInit->SampleRate, Audio_In_Ctx[Instance].ChannelsNbr, Audio_In_Ctx[Instance].ChannelsNbr) != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_NO_INIT;
      }
    }

    /* Update BSP AUDIO IN state */
    Audio_In_Ctx[Instance].State = AUDIO_IN_STATE_STOP;
    /* Return BSP status */
    ret = BSP_ERROR_NONE;
  }
  return ret ;
}

/**
  * @brief  Deinit the audio IN peripherals.
  * @param  Instance  AUDIO IN Instance. It can be 0 when SAI is used or 1 if DFSDM is used
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_DeInit(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t i;

  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(Instance == 0U)
    {
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 0U)
      SAI_MspDeInit(&haudio_in_sai);
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 0U) */

      /* Initialize the haudio_in_sai Instance parameter */
      haudio_in_sai.Instance = AUDIO_IN_SAIx;
      /* Initialize the haudio_out_sai Instance parameter */
      haudio_out_sai.Instance = AUDIO_OUT_SAIx;

      if(Audio_Drv->DeInit(Audio_CompObj) < 0)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }/* De-Initializes SAI handles */
      else if(HAL_SAI_DeInit(&haudio_in_sai) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
        if(HAL_SAI_DeInit(&haudio_out_sai) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
    }
    else if (Instance == 1U)
    {
      for(i = 0U; i < DFSDM_MIC_NUMBER; i++)
      {
        /* De-initializes DFSDM Filter handle */
        if(haudio_in_dfsdm_filter[i].Instance != NULL)
        {
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 0U)
        DFSDM_FilterMspDeInit(&haudio_in_dfsdm_filter[i]);
#endif /* (USE_HAL_DFSDM_REGISTER_CALLBACKS == 0U) */
          if(HAL_OK != HAL_DFSDM_FilterDeInit(&haudio_in_dfsdm_filter[i]))
          {
            return BSP_ERROR_PERIPH_FAILURE;
          }
          haudio_in_dfsdm_filter[i].Instance = NULL;
        }

        /* De-initializes DFSDM Channel handle */
        if(haudio_in_dfsdm_channel[i].Instance != NULL)
        {
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 0U)
        DFSDM_ChannelMspDeInit(&haudio_in_dfsdm_channel[i]);
#endif /* (USE_HAL_DFSDM_REGISTER_CALLBACKS == 0U) */
          if(HAL_OK != HAL_DFSDM_ChannelDeInit(&haudio_in_dfsdm_channel[i]))
          {
            return BSP_ERROR_PERIPH_FAILURE;
          }
          haudio_in_dfsdm_channel[i].Instance = NULL;
        }
      }

      /* Reset Audio_In_Ctx[1].IsMultiBuff if any */
      Audio_In_Ctx[1].IsMultiBuff = 0;
    }
    else /* if(Instance == 2U) */
    {
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 0U)
      SAI_MspDeInit(&haudio_in_sai);
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 0U) */

      /* Initialize the haudio_in_sai Instance parameter */
      haudio_in_sai.Instance = AUDIO_IN_SAI_PDMx;
      /* Initialize the haudio_out_sai Instance parameter */
      haudio_out_sai.Instance = AUDIO_OUT_SAIx;

      if(HAL_SAI_DeInit(&haudio_in_sai) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
        if(HAL_SAI_DeInit(&haudio_out_sai) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
    }
    /* Update BSP AUDIO IN state */
    Audio_In_Ctx[Instance].State = AUDIO_IN_STATE_RESET;
  }


  /* Return BSP status */
  return ret;
}

/**
  * @brief  Clock Config.
  * @param  hDfsdmChannel  DFSDM Channel Handle
  * @param  SampleRate     Audio frequency to be configured for the DFSDM Channel.
  * @note   This API is called by BSP_AUDIO_IN_Init()
  *         Being __weak it can be overwritten by the application
  * @retval HAL_status
  */
__weak HAL_StatusTypeDef MX_DFSDM1_ClockConfig(DFSDM_Channel_HandleTypeDef *hDfsdmChannel, uint32_t SampleRate)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hDfsdmChannel);

  HAL_StatusTypeDef ret = HAL_OK;
  RCC_PeriphCLKInitTypeDef rcc_ex_clk_init_struct;

  HAL_RCCEx_GetPeriphCLKConfig(&rcc_ex_clk_init_struct);

  /* Set the PLL configuration according to the audio frequency */
  if((SampleRate == AUDIO_FREQUENCY_11K) || (SampleRate == AUDIO_FREQUENCY_22K) || (SampleRate == AUDIO_FREQUENCY_44K))
  {
    rcc_ex_clk_init_struct.PLL2.PLL2P = 38;
    rcc_ex_clk_init_struct.PLL2.PLL2N = 429;
  }
  else /* AUDIO_FREQUENCY_8K, AUDIO_FREQUENCY_16K, AUDIO_FREQUENCY_32K, AUDIO_FREQUENCY_48K, AUDIO_FREQUENCY_96K */
  {
    rcc_ex_clk_init_struct.PLL2.PLL2P = 7;
    rcc_ex_clk_init_struct.PLL2.PLL2N = 344;
  }
  /* Configure prescalers */
  rcc_ex_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
  rcc_ex_clk_init_struct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL2;
  rcc_ex_clk_init_struct.PLL2.PLL2Q = 1;
  rcc_ex_clk_init_struct.PLL2.PLL2R = 1;
  rcc_ex_clk_init_struct.PLL2.PLL2M = 25;

  if(HAL_RCCEx_PeriphCLKConfig(&rcc_ex_clk_init_struct) != HAL_OK)
  {
    ret = HAL_ERROR;
  }

  return ret;
}

/**
  * @brief  Initializes the Audio instance (DFSDM).
  * @param  hDfsdmFilter  DFSDM Filter Handle
  * @param  hDfsdmChannel DFSDM Channel Handle
  * @param  MXConfig      DFSDM configuration structure
  * @note   Being __weak it can be overwritten by the application
  * @note   Channel output Clock Divider and Filter Oversampling are calculated as follow:
  *         - Clock_Divider = CLK(input DFSDM)/CLK(micro) with
  *           1MHZ < CLK(micro) < 3.2MHZ (TYP 2.4MHZ for MP34DT01TR)
  *         - Oversampling = CLK(input DFSDM)/(Clock_Divider * AudioFreq)
  * @retval HAL_status
  */
__weak HAL_StatusTypeDef MX_DFSDM1_Init(DFSDM_Filter_HandleTypeDef *hDfsdmFilter, DFSDM_Channel_HandleTypeDef *hDfsdmChannel, MX_DFSDM_Config_t *MXConfig)
{
  /* MIC filters  initialization */
  hDfsdmFilter->Instance                          = MXConfig->FilterInstance;
  hDfsdmFilter->Init.RegularParam.Trigger         = MXConfig->RegularTrigger;
  hDfsdmFilter->Init.RegularParam.FastMode        = ENABLE;
  hDfsdmFilter->Init.RegularParam.DmaMode         = ENABLE;
  hDfsdmFilter->Init.InjectedParam.Trigger        = DFSDM_FILTER_SW_TRIGGER;
  hDfsdmFilter->Init.InjectedParam.ScanMode       = DISABLE;
  hDfsdmFilter->Init.InjectedParam.DmaMode        = DISABLE;
  hDfsdmFilter->Init.InjectedParam.ExtTrigger     = DFSDM_FILTER_EXT_TRIG_TIM8_TRGO;
  hDfsdmFilter->Init.InjectedParam.ExtTriggerEdge = DFSDM_FILTER_EXT_TRIG_BOTH_EDGES;
  hDfsdmFilter->Init.FilterParam.SincOrder        = MXConfig->SincOrder;
  hDfsdmFilter->Init.FilterParam.Oversampling     = MXConfig->Oversampling;
  hDfsdmFilter->Init.FilterParam.IntOversampling  = 1;

  if(HAL_DFSDM_FilterInit(hDfsdmFilter) != HAL_OK)
  {
    return HAL_ERROR;
  }

  /* MIC channels initialization */
  hDfsdmChannel->Instance                      = MXConfig->ChannelInstance;
  hDfsdmChannel->Init.OutputClock.Activation   = ENABLE;
  hDfsdmChannel->Init.OutputClock.Selection    = DFSDM_CHANNEL_OUTPUT_CLOCK_AUDIO;
  hDfsdmChannel->Init.OutputClock.Divider      = MXConfig->ClockDivider;
  hDfsdmChannel->Init.Input.Multiplexer        = DFSDM_CHANNEL_EXTERNAL_INPUTS;
  hDfsdmChannel->Init.Input.DataPacking        = DFSDM_CHANNEL_STANDARD_MODE;
  hDfsdmChannel->Init.SerialInterface.SpiClock = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
  hDfsdmChannel->Init.Awd.FilterOrder          = DFSDM_CHANNEL_SINC1_ORDER;
  hDfsdmChannel->Init.Awd.Oversampling         = 10;
  hDfsdmChannel->Init.Offset                   = 0;
  hDfsdmChannel->Init.RightBitShift            = MXConfig->RightBitShift;
  hDfsdmChannel->Init.Input.Pins               = MXConfig->DigitalMicPins;
  hDfsdmChannel->Init.SerialInterface.Type     = MXConfig->DigitalMicType;

  if(HAL_OK != HAL_DFSDM_ChannelInit(hDfsdmChannel))
  {
    return HAL_ERROR;
  }

  /* Configure injected channel */
  if(HAL_DFSDM_FilterConfigRegChannel(hDfsdmFilter, MXConfig->Channel4Filter, DFSDM_CONTINUOUS_CONV_ON) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

/**
  * @brief  Initializes the Audio Codec audio in instance (SAI).
  * @param  hsai : SAI handle
  * @param  MXConfig SAI configuration structure
  * @note   Being __weak it can be overwritten by the application
  * @retval HAL status
  */
__weak HAL_StatusTypeDef MX_SAI1_Block_B_Init(SAI_HandleTypeDef* hsai, MX_SAI_Config *MXConfig)
{
  HAL_StatusTypeDef ret = HAL_OK;

  /* Disable SAI peripheral to allow access to SAI internal registers */
  __HAL_SAI_DISABLE(hsai);

  /* Configure SAI1_Block_B */
  hsai->Init.AudioFrequency         = MXConfig->AudioFrequency;
  hsai->Init.MonoStereoMode         = MXConfig->MonoStereoMode;
  hsai->Init.AudioMode              = MXConfig->AudioMode;
  hsai->Init.NoDivider              = SAI_MASTERDIVIDER_ENABLE;
  hsai->Init.Protocol               = SAI_FREE_PROTOCOL;
  hsai->Init.DataSize               = MXConfig->DataSize;
  hsai->Init.FirstBit               = SAI_FIRSTBIT_MSB;
  hsai->Init.ClockStrobing          = MXConfig->ClockStrobing;
  hsai->Init.Synchro                = MXConfig->Synchro;
  hsai->Init.OutputDrive            = MXConfig->OutputDrive;
  hsai->Init.FIFOThreshold          = SAI_FIFOTHRESHOLD_1QF;
  hsai->Init.SynchroExt             = MXConfig->SynchroExt;
  hsai->Init.CompandingMode         = SAI_NOCOMPANDING;
  hsai->Init.TriState               = SAI_OUTPUT_RELEASED;
  hsai->Init.Mckdiv                 = 0;
  hsai->Init.PdmInit.Activation     = DISABLE;

  /* Configure SAI_Block_x Frame */
  hsai->FrameInit.FrameLength       = MXConfig->FrameLength;
  hsai->FrameInit.ActiveFrameLength = MXConfig->ActiveFrameLength;
  hsai->FrameInit.FSDefinition      = SAI_FS_CHANNEL_IDENTIFICATION;
  hsai->FrameInit.FSPolarity        = SAI_FS_ACTIVE_LOW;
  hsai->FrameInit.FSOffset          = SAI_FS_BEFOREFIRSTBIT;

  /* Configure SAI Block_x Slot */
  hsai->SlotInit.FirstBitOffset     = 0;
  hsai->SlotInit.SlotSize           = SAI_SLOTSIZE_DATASIZE;
  hsai->SlotInit.SlotNumber         = 4;
  hsai->SlotInit.SlotActive        = MXConfig->SlotActive;

  if(HAL_SAI_Init(hsai) != HAL_OK)
  {
    ret = HAL_ERROR;
  }

  /* Enable SAI peripheral */
  __HAL_SAI_ENABLE(hsai);

  return ret;
}

/**
  * @brief  Initializes the Audio Codec audio in instance (SAI).
  * @param  hsai : SAI handle
  * @param  MXConfig SAI configuration structure
  * @note   Being __weak it can be overwritten by the application
  * @retval HAL status
  */
__weak HAL_StatusTypeDef MX_SAI4_Block_A_Init(SAI_HandleTypeDef* hsai, MX_SAI_Config *MXConfig)
{
  HAL_StatusTypeDef ret = HAL_OK;

  /* Disable SAI peripheral to allow access to SAI internal registers */
  __HAL_SAI_DISABLE(hsai);

  /* Configure SAI4_Block_A */
  hsai->Init.AudioFrequency         = MXConfig->AudioFrequency;
  hsai->Init.MonoStereoMode         = MXConfig->MonoStereoMode;
  hsai->Init.AudioMode              = MXConfig->AudioMode;
  hsai->Init.NoDivider              = SAI_MASTERDIVIDER_DISABLE;
  hsai->Init.Protocol               = SAI_FREE_PROTOCOL;
  hsai->Init.DataSize               = MXConfig->DataSize;
  hsai->Init.FirstBit               = SAI_FIRSTBIT_LSB;
  hsai->Init.ClockStrobing          = MXConfig->ClockStrobing;
  hsai->Init.Synchro                = MXConfig->Synchro;
  hsai->Init.OutputDrive            = MXConfig->OutputDrive;
  hsai->Init.FIFOThreshold          = SAI_FIFOTHRESHOLD_1QF;
  hsai->Init.SynchroExt             = MXConfig->SynchroExt;
  hsai->Init.CompandingMode         = SAI_NOCOMPANDING;
  hsai->Init.TriState               = SAI_OUTPUT_RELEASED;
  hsai->Init.Mckdiv                 = 0;
  hsai->Init.PdmInit.Activation     = ENABLE;
  hsai->Init.PdmInit.MicPairsNbr    = 1;
  hsai->Init.PdmInit.ClockEnable    = SAI_PDM_CLOCK1_ENABLE;


  /* Configure SAI_Block_x Frame */
  hsai->FrameInit.FrameLength       = MXConfig->FrameLength;
  hsai->FrameInit.ActiveFrameLength = MXConfig->ActiveFrameLength;
  hsai->FrameInit.FSDefinition      = SAI_FS_STARTFRAME;
  hsai->FrameInit.FSPolarity        = SAI_FS_ACTIVE_HIGH;
  hsai->FrameInit.FSOffset          = SAI_FS_FIRSTBIT;

  /* Configure SAI Block_x Slot */
  hsai->SlotInit.FirstBitOffset     = 0;
  hsai->SlotInit.SlotSize           = SAI_SLOTSIZE_DATASIZE;
  hsai->SlotInit.SlotNumber         = 1;
  hsai->SlotInit.SlotActive        = MXConfig->SlotActive;

  if(HAL_SAI_Init(hsai) != HAL_OK)
  {
    ret = HAL_ERROR;
  }

  /* Enable SAI peripheral */
  __HAL_SAI_ENABLE(hsai);

  return ret;
}


#if ((USE_HAL_DFSDM_REGISTER_CALLBACKS == 1U) || (USE_HAL_SAI_REGISTER_CALLBACKS == 1U))
/**
  * @brief Default BSP AUDIO IN Msp Callbacks
  * @param Instance BSP AUDIO IN Instance
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_RegisterDefaultMspCallbacks (uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t i;

  if(Instance == 1U)
  {
    for(i = 0; i < DFSDM_MIC_NUMBER; i ++)
    {
      if(((Audio_In_Ctx[Instance].Device >> i) & AUDIO_IN_DEVICE_DIGITAL_MIC1) == AUDIO_IN_DEVICE_DIGITAL_MIC1)
      {
        __HAL_DFSDM_CHANNEL_RESET_HANDLE_STATE(&haudio_in_dfsdm_channel[i]);
        __HAL_DFSDM_FILTER_RESET_HANDLE_STATE(&haudio_in_dfsdm_filter[i]);

#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
        /* Register MspInit/MspDeInit Callbacks */
        if(HAL_DFSDM_CHANNEL_RegisterCallback(&haudio_in_dfsdm_channel[i], HAL_DFSDM_CHANNEL_MSPINIT_CB_ID, DFSDM_ChannelMspInit) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else if(HAL_DFSDM_FILTER_RegisterCallback(&haudio_in_dfsdm_filter[i], HAL_DFSDM_FILTER_MSPINIT_CB_ID, DFSDM_FilterMspInit) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else if(HAL_DFSDM_CHANNEL_RegisterCallback(&haudio_in_dfsdm_channel[i], HAL_DFSDM_CHANNEL_MSPDEINIT_CB_ID, DFSDM_ChannelMspDeInit) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else
        {
          if(HAL_DFSDM_Filter_RegisterCallback(&haudio_in_dfsdm_filter[i], HAL_DFSDM_FILTER_MSPDEINIT_CB_ID, DFSDM_FilterMspDeInit) != HAL_OK)
          {
            ret = BSP_ERROR_PERIPH_FAILURE;
          }
        }
#endif /* (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)  */
      }
    }
  }
  else if(Instance == 0U)
  {
    __HAL_SAI_RESET_HANDLE_STATE(&haudio_in_sai);
    __HAL_SAI_RESET_HANDLE_STATE(&haudio_out_sai);

#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1U)
    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_SAI_RegisterCallback(&haudio_in_sai, HAL_SAI_MSPINIT_CB_ID, SAI_MspInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_MSPINIT_CB_ID, SAI_MspInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_SAI_RegisterCallback(&haudio_in_sai, HAL_SAI_MSPDEINIT_CB_ID, SAI_MspDeInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_MSPDEINIT_CB_ID, SAI_MspDeInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
    }
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1U) */
  }
  else
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }

  if(ret == BSP_ERROR_NONE)
  {
    Audio_In_Ctx[Instance].IsMspCallbacksValid = 1;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief BSP AUDIO In Filter Msp Callback registering
  * @param Instance    AUDIO IN Instance
  * @param CallBacks   pointer to filter MspInit/MspDeInit functions
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_RegisterMspCallbacks (uint32_t Instance, BSP_AUDIO_IN_Cb_t *CallBacks)
{
  int32_t ret = BSP_ERROR_NONE;


  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(Instance == 1U)
    {
#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1)
       uint32_t i;
        for(i = 0U; i < DFSDM_MIC_NUMBER; i ++)
    {
      __HAL_DFSDM_FILTER_RESET_HANDLE_STATE(&haudio_in_dfsdm_filter[i]);

      /* Register MspInit/MspDeInit Callback */
      if(HAL_DFSDM_Filter_RegisterCallback(&haudio_in_dfsdm_filter[i], HAL_DFSDM_FILTER_MSPINIT_CB_ID, CallBacks->pMspFltrInitCb) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else if(HAL_DFSDM_Filter_RegisterCallback(&haudio_in_dfsdm_filter[i], HAL_DFSDM_FILTER_MSPDEINIT_CB_ID, CallBacks->pMspFltrDeInitCb) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else if(HAL_DFSDM_Channel_RegisterCallback(&haudio_in_dfsdm_channel[i], HAL_DFSDM_CHANNEL_MSPINIT_CB_ID, CallBacks->pMspChInitCb) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
        if(HAL_DFSDM_Channel_RegisterCallback(&haudio_in_dfsdm_channel[i], HAL_DFSDM_CHANNEL_MSPDEINIT_CB_ID, CallBacks->pMspChDeInitCb) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
    }
#endif /* (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1) */
    }
    else /* (Instance == 0U) */
    {
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1U)
      if(HAL_SAI_RegisterCallback(&haudio_in_sai, HAL_SAI_MSPINIT_CB_ID, CallBacks->pMspSaiInitCb) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else if(HAL_SAI_RegisterCallback(&haudio_in_sai, HAL_SAI_MSPDEINIT_CB_ID, CallBacks->pMspSaiDeInitCb) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
      }
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1U) */
    }

    if(ret == BSP_ERROR_NONE)
    {
      Audio_In_Ctx[Instance].IsMspCallbacksValid = 1;
    }
  }

  /* Return BSP status */
  return ret;
}
#endif /* ((USE_HAL_DFSDM_REGISTER_CALLBACKS == 1) || (USE_HAL_SAI_REGISTER_CALLBACKS == 1U)) */

/**
  * @brief  Initialize the PDM library.
  * @param Instance    AUDIO IN Instance
  * @param  AudioFreq  Audio sampling frequency
  * @param  ChnlNbrIn  Number of input audio channels in the PDM buffer
  * @param  ChnlNbrOut Number of desired output audio channels in the  resulting PCM buffer
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_PDMToPCM_Init(uint32_t Instance, uint32_t AudioFreq, uint32_t ChnlNbrIn, uint32_t ChnlNbrOut)
{
  uint32_t index = 0;

  if(Instance != 2U)
  {
    return BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Enable CRC peripheral to unlock the PDM library */
    __HAL_RCC_CRC_CLK_ENABLE();

    for(index = 0; index < ChnlNbrIn; index++)
    {
      /* Init PDM filters */
      PDM_FilterHandler[index].bit_order  = PDM_FILTER_BIT_ORDER_MSB;
      PDM_FilterHandler[index].endianness = PDM_FILTER_ENDIANNESS_LE;
      PDM_FilterHandler[index].high_pass_tap = 2122358088;
      PDM_FilterHandler[index].out_ptr_channels = ChnlNbrOut;
      PDM_FilterHandler[index].in_ptr_channels  = ChnlNbrIn;
      PDM_Filter_Init((PDM_Filter_Handler_t *)(&PDM_FilterHandler[index]));

      /* PDM lib config phase */
      PDM_FilterConfig[index].output_samples_number = AudioFreq/1000;
      PDM_FilterConfig[index].mic_gain = 24;
      PDM_FilterConfig[index].decimation_factor = PDM_FILTER_DEC_FACTOR_64;
      PDM_Filter_setConfig((PDM_Filter_Handler_t *)&PDM_FilterHandler[index], &PDM_FilterConfig[index]);
    }
  }

  return BSP_ERROR_NONE;
}

/**
  * @brief  Converts audio format from PDM to PCM.
  * @param  Instance  AUDIO IN Instance
  * @param  PDMBuf    Pointer to PDM buffer data
  * @param  PCMBuf    Pointer to PCM buffer data
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_PDMToPCM(uint32_t Instance, uint16_t *PDMBuf, uint16_t *PCMBuf)
{
  uint32_t index = 0;

  if(Instance != 2U)
  {
    return BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    for(index = 0; index < Audio_In_Ctx[Instance].ChannelsNbr; index++)
    {
      PDM_Filter(&((uint8_t*)(PDMBuf))[index], (uint16_t*)&(PCMBuf[index]), &PDM_FilterHandler[index]);
    }
  }

  return BSP_ERROR_NONE;
}

/**
  * @brief  Start audio recording.
  * @param  Instance  AUDIO IN Instance. It can be 0 when SAI is used or 1 if DFSDM is used
  * @param  pBuf     Main buffer pointer for the recorded data storing
  * @param  NbrOfBytes  Size of the record buffer
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_Record(uint32_t Instance, uint8_t* pBuf, uint32_t NbrOfBytes)
{
  int32_t ret;

  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(Instance == 1U)
    {
      Audio_In_Ctx[Instance].pBuff = (uint16_t*)pBuf;
      Audio_In_Ctx[Instance].Size  = NbrOfBytes;
      /* Reset Buffer Trigger */
      RecBuffTrigger = 0;
      RecBuffHalf = 0;

      /* Call the Media layer start function for MIC2 channel */
      if(HAL_DFSDM_FilterRegularMsbStart_DMA(&haudio_in_dfsdm_filter[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)], \
        (int16_t*)MicRecBuff[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)], DEFAULT_AUDIO_IN_BUFFER_SIZE) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else if(HAL_DFSDM_FilterRegularMsbStart_DMA(&haudio_in_dfsdm_filter[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)], \
        (int16_t*)MicRecBuff[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)], DEFAULT_AUDIO_IN_BUFFER_SIZE) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
        /* Update BSP AUDIO IN state */
        Audio_In_Ctx[Instance].State = AUDIO_IN_STATE_RECORDING;
        ret = BSP_ERROR_NONE;
      }
    }
    else
    {
      uint8_t TxData[2] = {0x00U, 0x00U};
      if(HAL_SAI_Transmit(&haudio_out_sai, TxData, 2, 1000) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
        /* Call the audio Codec Play function */
        if (Audio_Drv->Play(Audio_CompObj) < 0)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }

        /* Start the process receive DMA */
        if(HAL_SAI_Receive_DMA(&haudio_in_sai, (uint8_t*)pBuf, (uint16_t)(NbrOfBytes/(Audio_In_Ctx[Instance].BitsPerSample/8U))) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
    }
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Stop audio recording.
  * @param  Instance  AUDIO IN Instance. It can be 0 when SAI is used or 1 if DFSDM is used
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_Stop(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(Instance == 1U) /* DFSDM */
    {
      /* Call the Media layer stop function */
      if(HAL_DFSDM_FilterRegularStop_DMA(&haudio_in_dfsdm_filter[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)]) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
        if(HAL_DFSDM_FilterRegularStop_DMA(&haudio_in_dfsdm_filter[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)]) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
    }
    else
    {
      if(Instance == 0U)
      {
        /* Call the Media layer stop function */
        if(Audio_Drv->Stop(Audio_CompObj, CODEC_PDWN_SW) < 0)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
      }

      if(ret == BSP_ERROR_NONE)
      {
        if(HAL_SAI_DMAStop(&haudio_in_sai) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
    }


    /* Update BSP AUDIO IN state */
    Audio_In_Ctx[Instance].State = AUDIO_IN_STATE_STOP;
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Pause the audio file stream.
  * @param  Instance  AUDIO IN Instance. It can be 0 when SAI is used or 1 if DFSDM is used
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_Pause(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(Instance == 1U)
    {
      if(HAL_DFSDM_FilterRegularStop_DMA(&haudio_in_dfsdm_filter[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)]) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
        if(HAL_DFSDM_FilterRegularStop_DMA(&haudio_in_dfsdm_filter[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)]) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
    }
    else /* 0 or 2*/
    {
      if(Instance == 0U)
      {
        /* Call Audio Codec Pause function */
        if(Audio_Drv->Pause(Audio_CompObj) < 0)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
      }

      if(ret == BSP_ERROR_NONE)
      {
        if(HAL_SAI_DMAPause(&haudio_in_sai) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
    }

    /* Update BSP AUDIO IN state */
    Audio_In_Ctx[Instance].State = AUDIO_IN_STATE_PAUSE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Resume the audio file stream.
  * @param  Instance  AUDIO IN Instance. It can be 0 when SAI is used or 1 if DFSDM is used
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_Resume(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(Instance == 1U)
    {
      /* Call the Media layer start function for MIC2/MIC1 channel */
      if(HAL_DFSDM_FilterRegularMsbStart_DMA(&haudio_in_dfsdm_filter[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)], \
        (int16_t*)MicRecBuff[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)], DEFAULT_AUDIO_IN_BUFFER_SIZE) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
        if(HAL_DFSDM_FilterRegularMsbStart_DMA(&haudio_in_dfsdm_filter[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)], \
          (int16_t*)MicRecBuff[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)], DEFAULT_AUDIO_IN_BUFFER_SIZE) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
    }
    else /* 0 or 2 */
    {
      if(Instance == 0U)
      {
        /* Call Audio Codec Resume function */
        if(Audio_Drv->Resume(Audio_CompObj) < 0)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
      }

      if(ret == BSP_ERROR_NONE)
      {
        if(HAL_SAI_DMAResume(&haudio_in_sai) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
    }

    /* Update BSP AUDIO IN state */
    Audio_In_Ctx[Instance].State = AUDIO_IN_STATE_RECORDING;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Starts audio recording.
  * @param  Instance  AUDIO IN Instance. It can be 1(DFSDM used)
  * @param  pBuf      Main buffer pointer for the recorded data storing
  * @param  NbrOfBytes  Size of the recorded buffer
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_RecordChannels(uint32_t Instance, uint8_t **pBuf, uint32_t NbrOfBytes)
{
  uint16_t i;
  int32_t ret = BSP_ERROR_NONE;
  uint32_t mic_init[DFSDM_MIC_NUMBER] = {0};
  uint32_t audio_in_digital_mic = AUDIO_IN_DEVICE_DIGITAL_MIC1, pbuf_index = 0;
  uint32_t enabled_mic = 0;

  if(Instance != 1U)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Get the number of activated microphones */
    for(i = 0U; i < DFSDM_MIC_NUMBER; i++)
    {
      if((Audio_In_Ctx[Instance].Device & audio_in_digital_mic) == audio_in_digital_mic)
      {
        enabled_mic++;
      }
      audio_in_digital_mic = audio_in_digital_mic << 1;
    }

    Audio_In_Ctx[Instance].pMultiBuff = pBuf;
    Audio_In_Ctx[Instance].Size  = NbrOfBytes;
    Audio_In_Ctx[Instance].IsMultiBuff = 1;

    audio_in_digital_mic = AUDIO_IN_DEVICE_DIGITAL_MIC_LAST;
    for(i = 0U; i < DFSDM_MIC_NUMBER; i++)
    {
      if(((Audio_In_Ctx[Instance].Device & audio_in_digital_mic) == audio_in_digital_mic) && (mic_init[POS_VAL(audio_in_digital_mic)] != 1U))
      {
        /* Call the Media layer start function for MICx channel */
        if(HAL_DFSDM_FilterRegularMsbStart_DMA(&haudio_in_dfsdm_filter[POS_VAL(audio_in_digital_mic)], (int16_t*)pBuf[enabled_mic - 1U - pbuf_index], NbrOfBytes) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
        else
        {
          mic_init[POS_VAL(audio_in_digital_mic)] = 1;
          pbuf_index++;
        }
      }
      audio_in_digital_mic = audio_in_digital_mic >> 1;
    }
    /* Update BSP AUDIO IN state */
    Audio_In_Ctx[Instance].State = AUDIO_IN_STATE_RECORDING;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Stop audio recording.
  * @param  Instance  AUDIO IN Instance. It can be 1(DFSDM used)
  * @param  Device    Digital input device to be stopped
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_StopChannels(uint32_t Instance, uint32_t Device)
{
  int32_t ret;

  /* Stop selected devices */
  ret = BSP_AUDIO_IN_PauseChannels(Instance, Device);

  if(ret == BSP_ERROR_NONE)
  {
    /* Update BSP AUDIO IN state */
    Audio_In_Ctx[Instance].State = AUDIO_IN_STATE_STOP;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Pause the audio file stream.
  * @param  Instance  AUDIO IN Instance. It can be 1(DFSDM used)
  * @param  Device    Digital mic to be paused
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_PauseChannels(uint32_t Instance, uint32_t Device)
{
  uint32_t audio_in_digital_mic = AUDIO_IN_DEVICE_DIGITAL_MIC1;
  uint32_t i;
  int32_t ret = BSP_ERROR_NONE;

  if((Instance != 1U) || ((Device < AUDIO_IN_DEVICE_DIGITAL_MIC1) && (Device > AUDIO_IN_DEVICE_DIGITAL_MIC_LAST)))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    for(i = 0; i < DFSDM_MIC_NUMBER; i++)
    {
      if((Device & audio_in_digital_mic) == audio_in_digital_mic)
      {
        /* Call the Media layer stop function */
        if(HAL_DFSDM_FilterRegularStop_DMA(&haudio_in_dfsdm_filter[POS_VAL(audio_in_digital_mic)]) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
      audio_in_digital_mic = audio_in_digital_mic << 1;

    }
    if(ret == BSP_ERROR_NONE)
    {
      /* Update BSP AUDIO IN state */
      Audio_In_Ctx[Instance].State = AUDIO_IN_STATE_PAUSE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Resume the audio file stream
  * @param  Instance  AUDIO IN Instance. It can be 1(DFSDM used)
  * @param  Device    Digital mic to be resumed
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_ResumeChannels(uint32_t Instance, uint32_t Device)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t audio_in_digital_mic = AUDIO_IN_DEVICE_DIGITAL_MIC_LAST;
  uint32_t i;
  if((Instance != 1U) || ((Device < AUDIO_IN_DEVICE_DIGITAL_MIC1) && (Device > AUDIO_IN_DEVICE_DIGITAL_MIC_LAST)))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    for(i = 0; i < DFSDM_MIC_NUMBER; i++)
    {
      if((Device & audio_in_digital_mic) == audio_in_digital_mic)
      {
        /* Start selected device channel */
        if(HAL_DFSDM_FilterRegularMsbStart_DMA(&haudio_in_dfsdm_filter[POS_VAL(audio_in_digital_mic)],\
          (int16_t*)Audio_In_Ctx[Instance].pMultiBuff[POS_VAL(audio_in_digital_mic)], Audio_In_Ctx[Instance].Size) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
      }
      audio_in_digital_mic = audio_in_digital_mic >> 1;
    }

    if(ret == BSP_ERROR_NONE)
    {
      /* Update BSP AUDIO IN state */
      Audio_In_Ctx[Instance].State = AUDIO_IN_STATE_RECORDING;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Start audio recording.
  * @param  Instance  AUDIO IN SAI PDM Instance. It can be only 2
  * @param  pBuf     Main buffer pointer for the recorded data storing
  * @param  NbrOfBytes  Size of the record buffer
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_RecordPDM(uint32_t Instance, uint8_t* pBuf, uint32_t NbrOfBytes)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance != 2U)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Start the process receive DMA */
    if(HAL_SAI_Receive_DMA(&haudio_in_sai, (uint8_t*)pBuf, (uint16_t)(NbrOfBytes/(Audio_In_Ctx[Instance].BitsPerSample/8U))) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  /* Return BSP status */
  return ret;
}


/**
  * @brief  Set Audio In device
  * @param  Instance  AUDIO IN Instance. It can be 0 when SAI is used or 1 if DFSDM is used
  * @param  Device    The audio input device to be used
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_SetDevice(uint32_t Instance, uint32_t Device)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t i;
  BSP_AUDIO_Init_t audio_init;

  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Audio_In_Ctx[Instance].State == AUDIO_IN_STATE_STOP)
  {
    if(Instance == 1U)
    {
      for(i = 0; i < DFSDM_MIC_NUMBER; i ++)
      {
        if(((Device >> i) & AUDIO_IN_DEVICE_DIGITAL_MIC1) == AUDIO_IN_DEVICE_DIGITAL_MIC1)
        {
          if(HAL_DFSDM_ChannelDeInit(&haudio_in_dfsdm_channel[i]) != HAL_OK)
          {
            return BSP_ERROR_PERIPH_FAILURE;
          }
        }
      }
    }
    audio_init.Device        = Device;
    audio_init.ChannelsNbr   = Audio_In_Ctx[Instance].ChannelsNbr;
    audio_init.SampleRate    = Audio_In_Ctx[Instance].SampleRate;
    audio_init.BitsPerSample = Audio_In_Ctx[Instance].BitsPerSample;
    audio_init.Volume        = Audio_In_Ctx[Instance].Volume;

    if(BSP_AUDIO_IN_Init(Instance, &audio_init) < 0)
    {
      ret = BSP_ERROR_NO_INIT;
    }
  }
  else
  {
    ret = BSP_ERROR_BUSY;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get Audio In device
  * @param  Instance  AUDIO IN Instance. It can be 0 when SAI is used or 1 if DFSDM is used
  * @param  Device    The audio input device used
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_GetDevice(uint32_t Instance, uint32_t *Device)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Return audio Input Device */
    *Device = Audio_In_Ctx[Instance].Device;
  }
  return ret;
}

/**
  * @brief  Set Audio In frequency
  * @param  Instance     Audio IN instance
  * @param  SampleRate  Input frequency to be set
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_SetSampleRate(uint32_t Instance, uint32_t  SampleRate)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t i;
  BSP_AUDIO_Init_t audio_init;

  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Audio_In_Ctx[Instance].State == AUDIO_IN_STATE_STOP)
  {
    if(Instance == 1U)
    {
      for(i = 0; i < DFSDM_MIC_NUMBER; i ++)
      {
        if(((Audio_In_Ctx[Instance].Device >> i) & AUDIO_IN_DEVICE_DIGITAL_MIC1) == AUDIO_IN_DEVICE_DIGITAL_MIC1)
        {
          if(HAL_DFSDM_ChannelDeInit(&haudio_in_dfsdm_channel[i]) != HAL_OK)
          {
            return BSP_ERROR_PERIPH_FAILURE;
          }
          if(HAL_DFSDM_FilterDeInit(&haudio_in_dfsdm_filter[i]) != HAL_OK)
          {
            return BSP_ERROR_PERIPH_FAILURE;
          }
        }
      }
    }
    audio_init.Device        = Audio_In_Ctx[Instance].Device;
    audio_init.ChannelsNbr   = Audio_In_Ctx[Instance].ChannelsNbr;
    audio_init.SampleRate    = SampleRate;
    audio_init.BitsPerSample = Audio_In_Ctx[Instance].BitsPerSample;
    audio_init.Volume        = Audio_In_Ctx[Instance].Volume;
    if(BSP_AUDIO_IN_Init(Instance, &audio_init) < 0)
    {
      ret = BSP_ERROR_NO_INIT;
    }
  }
  else
  {
    ret = BSP_ERROR_BUSY;
  }

  /* Return BSP status */
  return ret;
}

/**
* @brief  Get Audio In frequency
  * @param  Instance  AUDIO IN Instance. It can be 0 when SAI is used or 1 if DFSDM is used
* @param  SampleRate  Audio Input frequency to be returned
* @retval BSP status
*/
int32_t BSP_AUDIO_IN_GetSampleRate(uint32_t Instance, uint32_t *SampleRate)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Return audio in frequency */
    *SampleRate = Audio_In_Ctx[Instance].SampleRate;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Set Audio In Resolution
  * @param  Instance  AUDIO IN Instance. It can be 0 when SAI is used or 1 if DFSDM is used
  * @param  BitsPerSample  Input resolution to be set
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_SetBitsPerSample(uint32_t Instance, uint32_t BitsPerSample)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t i;
  BSP_AUDIO_Init_t audio_init;

  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Audio_In_Ctx[Instance].State == AUDIO_IN_STATE_STOP)
  {
    if(Instance == 1U)
    {
      for(i = 0; i < DFSDM_MIC_NUMBER; i ++)
      {
        if(((Audio_In_Ctx[Instance].Device >> i) & AUDIO_IN_DEVICE_DIGITAL_MIC1) == AUDIO_IN_DEVICE_DIGITAL_MIC1)
        {
          if(HAL_DFSDM_ChannelDeInit(&haudio_in_dfsdm_channel[i]) != HAL_OK)
          {
            return BSP_ERROR_PERIPH_FAILURE;
          }
        }
      }
    }
    audio_init.Device        = Audio_In_Ctx[Instance].Device;
    audio_init.ChannelsNbr   = Audio_In_Ctx[Instance].ChannelsNbr;
    audio_init.SampleRate    = Audio_In_Ctx[Instance].SampleRate;
    audio_init.BitsPerSample = BitsPerSample;
    audio_init.Volume        = Audio_In_Ctx[Instance].Volume;
    if(BSP_AUDIO_IN_Init(Instance, &audio_init) < 0)
    {
      ret = BSP_ERROR_NO_INIT;
    }
  }
  else
  {
    ret = BSP_ERROR_BUSY;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get Audio In Resolution
  * @param  Instance  AUDIO IN Instance. It can be 0 when SAI is used or 1 if DFSDM is used
  * @param  BitsPerSample  Input resolution to be returned
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_GetBitsPerSample(uint32_t Instance, uint32_t *BitsPerSample)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Return audio in resolution */
    *BitsPerSample = Audio_In_Ctx[Instance].BitsPerSample;
  }
  return ret;
}

/**
  * @brief  Set Audio In Channel number
  * @param  Instance  AUDIO IN Instance. It can be 0 when SAI is used or 1 if DFSDM is used
  * @param  ChannelNbr  Channel number to be used
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_SetChannelsNbr(uint32_t Instance, uint32_t ChannelNbr)
{
  int32_t ret = BSP_ERROR_NONE;

  if((Instance >= AUDIO_IN_INSTANCES_NBR) || (ChannelNbr > 2U))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Update AudioIn Context */
    Audio_In_Ctx[Instance].ChannelsNbr = ChannelNbr;
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get Audio In Channel number
  * @param  Instance  AUDIO IN Instance. It can be 0 when SAI is used or 1 if DFSDM is used
  * @param  ChannelNbr  Channel number to be used
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_GetChannelsNbr(uint32_t Instance, uint32_t *ChannelNbr)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Channel number to be returned */
    *ChannelNbr = Audio_In_Ctx[Instance].ChannelsNbr;
  }
  return ret;
}

/**
  * @brief  Set the current audio in volume level.
  * @param  Instance  AUDIO IN Instance. It can be 0 when SAI is used or 1 if DFSDM is used
  * @param  Volume    Volume level to be returned
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_SetVolume(uint32_t Instance, uint32_t Volume)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Audio_Drv->SetVolume(Audio_CompObj, AUDIO_VOLUME_INPUT, VOLUME_IN_CONVERT(Volume)) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    /* Update AudioIn Context */
    Audio_In_Ctx[Instance].Volume = Volume;
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the current audio in volume level.
  * @param  Instance  AUDIO IN Instance. It can be 0 when SAI is used or 1 if DFSDM is used
  * @param  Volume    Volume level to be returned
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_GetVolume(uint32_t Instance, uint32_t *Volume)
{
  int32_t ret = BSP_ERROR_NONE;

 if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Input Volume to be returned */
    *Volume = Audio_In_Ctx[Instance].Volume;
  }
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get Audio In device
  * @param  Instance  AUDIO IN Instance. It can be 0 when SAI is used or 1 if DFSDM is used
  * @param  State     Audio Out state
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_GetState(uint32_t Instance, uint32_t *State)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Input State to be returned */
    *State = Audio_In_Ctx[Instance].State;
  }
  return ret;
}

/**
  * @brief  This function handles Audio Out DMA interrupt requests.
  * @param  Instance Audio IN instance: 0 for SAI, 1 for DFSDM and 2 for SAI PDM
  * @param  InputDevice Can be:
  *         - AUDIO_IN_DEVICE_ANALOG_MIC
  *         - AUDIO_IN_DEVICE_DIGITAL_MIC
  *         - AUDIO_IN_DEVICE_DIGITAL_MIC1
  *         - AUDIO_IN_DEVICE_DIGITAL_MIC2
  * @retval None
  */
void BSP_AUDIO_IN_IRQHandler(uint32_t Instance, uint32_t InputDevice)
{
  if(Instance == 1U) /* DFSDM */
  {
    if((InputDevice == AUDIO_IN_DEVICE_DIGITAL_MIC) || (InputDevice == AUDIO_IN_DEVICE_DIGITAL_MIC1)\
      || (InputDevice == AUDIO_IN_DEVICE_DIGITAL_MIC2))
    {
      HAL_DMA_IRQHandler(haudio_in_dfsdm_filter[POS_VAL(InputDevice)].hdmaReg);
    }
  }
  else /* SAI or PDM*/
  {
    if((InputDevice == AUDIO_IN_DEVICE_DIGITAL_MIC) || (InputDevice == AUDIO_IN_DEVICE_ANALOG_MIC))
    {
      HAL_DMA_IRQHandler(haudio_in_sai.hdmarx);
    }
  }
}

#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 0) || !defined (USE_HAL_DFSDM_REGISTER_CALLBACKS)

/**
  * @brief  Regular conversion complete callback.
  * @note   In interrupt mode, user has to read conversion value in this function
            using HAL_DFSDM_FilterGetRegularValue.
  * @param  hdfsdm_filter   DFSDM filter handle.
  * @retval None
  */
void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  uint32_t index;
  static uint32_t DmaRecBuffCplt[DFSDM_MIC_NUMBER]  = {0};
  int32_t  tmp;

  if(Audio_In_Ctx[1].IsMultiBuff == 1U)
  {
    /* Call the record update function to get the second half */
    BSP_AUDIO_IN_TransferComplete_CallBack(2);
  }
  else
  {
    if(hdfsdm_filter == &haudio_in_dfsdm_filter[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)])
    {
      DmaRecBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)] = 1;
    }
    if(hdfsdm_filter == &haudio_in_dfsdm_filter[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)])
    {
      DmaRecBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)] = 1;
    }

    if(DmaRecBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)] == 1U)
    {
      if(DmaRecBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)] == 1U)
      {
#if (USE_BSP_CPU_CACHE_MAINTENANCE == 1)
      SCB_InvalidateDCache_by_Addr((uint32_t *)&MicRecBuff[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)][DEFAULT_AUDIO_IN_BUFFER_SIZE/2U], ((int32_t)DEFAULT_AUDIO_IN_BUFFER_SIZE/2)*4);
      SCB_InvalidateDCache_by_Addr((uint32_t *)&MicRecBuff[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)][DEFAULT_AUDIO_IN_BUFFER_SIZE/2U], ((int32_t)DEFAULT_AUDIO_IN_BUFFER_SIZE/2)*4);
#endif /* USE_BSP_CPU_CACHE_MAINTENANCE */
      for(index = (DEFAULT_AUDIO_IN_BUFFER_SIZE/2U) ; index < DEFAULT_AUDIO_IN_BUFFER_SIZE; index++)
      {
        if(Audio_In_Ctx[1].ChannelsNbr == 2U)
        {
          tmp = MicRecBuff[0][index] / 256;
          tmp = SaturaLH(tmp, -32768, 32767);
          Audio_In_Ctx[1].pBuff[RecBuffTrigger]     = (uint16_t)(tmp);
          tmp = MicRecBuff[1][index] / 256;
          tmp = SaturaLH(tmp, -32768, 32767);
          Audio_In_Ctx[1].pBuff[RecBuffTrigger + 1U] = (uint16_t)(tmp);
        }
        else
        {
          tmp = MicRecBuff[0][index] / 256;
          tmp = SaturaLH(tmp, -32768, 32767);
          Audio_In_Ctx[1].pBuff[RecBuffTrigger]      = (uint16_t)(tmp);
          Audio_In_Ctx[1].pBuff[RecBuffTrigger + 1U] = (uint16_t)(tmp);
        }
        RecBuffTrigger +=2U;
      }
#if (USE_BSP_CPU_CACHE_MAINTENANCE == 1)
      SCB_CleanDCache_by_Addr((uint32_t *)Audio_In_Ctx[1].pBuff,  (int32_t)Audio_In_Ctx[1].Size*2);
#endif /* USE_BSP_CPU_CACHE_MAINTENANCE */

      DmaRecBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)] = 0;
      DmaRecBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)] = 0;
    }
  }
  
      /* Call Transfer Complete callback */
    if(RecBuffTrigger >= Audio_In_Ctx[1].Size)
    {
      /* Reset Application Buffer Trigger */
      RecBuffTrigger = 0;
      RecBuffHalf = 0;
      /* Call the record update function to get the next buffer to fill and its size (size is ignored) */
      BSP_AUDIO_IN_TransferComplete_CallBack(1);
    }

    /* Call Half Transfer Complete callback */
    else if(RecBuffTrigger >= (Audio_In_Ctx[1].Size/2U))
    {
      if(RecBuffHalf == 0U)
      {
        RecBuffHalf = 1;
        BSP_AUDIO_IN_HalfTransfer_CallBack(2);
      }
    }

  }
}

/**
  * @brief  Half regular conversion complete callback.
  * @param  hdfsdm_filter   DFSDM filter handle.
  * @retval None
  */
void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  uint32_t index;
  static uint32_t DmaRecHalfBuffCplt[DFSDM_MIC_NUMBER]  = {0};
  int32_t  tmp;

  if(Audio_In_Ctx[1].IsMultiBuff == 1U)
  {
    /* Call the record update function to get the first half */
    BSP_AUDIO_IN_HalfTransfer_CallBack(1);
  }
  else
  {
    if(hdfsdm_filter == &haudio_in_dfsdm_filter[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)])
    {
      DmaRecHalfBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)] = 1;
    }
    if(hdfsdm_filter == &haudio_in_dfsdm_filter[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)])
    {
      DmaRecHalfBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)] = 1;
    }

    if(DmaRecHalfBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)] == 1U)
    {
      if(DmaRecHalfBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)] == 1U)
      {
        SCB_InvalidateDCache_by_Addr((uint32_t *)&MicRecBuff[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)][0], ((int32_t)DEFAULT_AUDIO_IN_BUFFER_SIZE/2)*4);
        SCB_InvalidateDCache_by_Addr((uint32_t *)&MicRecBuff[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)][0], ((int32_t)DEFAULT_AUDIO_IN_BUFFER_SIZE/2)*4);
        for(index = 0 ; index < (DEFAULT_AUDIO_IN_BUFFER_SIZE/2U); index++)
        {
          if(Audio_In_Ctx[1].ChannelsNbr == 2U)
          {
            tmp = MicRecBuff[0][index] / 256;
            tmp = SaturaLH(tmp, -32768, 32767);
            Audio_In_Ctx[1].pBuff[RecBuffTrigger]     = (uint16_t)(tmp);
            tmp = MicRecBuff[1][index] / 256;
            tmp = SaturaLH(tmp, -32768, 32767);
            Audio_In_Ctx[1].pBuff[RecBuffTrigger + 1U] = (uint16_t)(tmp);
          }
          else
          {
            tmp = MicRecBuff[0][index] / 256;
            tmp = SaturaLH(tmp, -32768, 32767);
            Audio_In_Ctx[1].pBuff[RecBuffTrigger]      = (uint16_t)(tmp);
            Audio_In_Ctx[1].pBuff[RecBuffTrigger + 1U] = (uint16_t)(tmp);
          }
          RecBuffTrigger +=2U;
        }

#if (USE_BSP_CPU_CACHE_MAINTENANCE == 1)
        SCB_CleanDCache_by_Addr((uint32_t *)Audio_In_Ctx[1].pBuff, ((int32_t)Audio_In_Ctx[1].Size*2));
#endif /* USE_BSP_CPU_CACHE_MAINTENANCE */

        DmaRecHalfBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)] = 0;
        DmaRecHalfBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)] = 0;
      }
    }
    /* Call Transfer Complete callback */
    if(RecBuffTrigger >= Audio_In_Ctx[1].Size)
    {
      /* Reset Application Buffer Trigger */
      RecBuffTrigger = 0;
      RecBuffHalf = 0;
      /* Call the record update function to get the next buffer to fill and its size (size is ignored) */
      BSP_AUDIO_IN_TransferComplete_CallBack(1);
    }
    
    /* Call Half Transfer Complete callback */
    else if(RecBuffTrigger >= (Audio_In_Ctx[1].Size/2U))
    {
      if(RecBuffHalf == 0U)
      {
        RecBuffHalf = 1;
        BSP_AUDIO_IN_HalfTransfer_CallBack(1);
      }
    }

  }
}
#endif /* (USE_HAL_DFSDM_REGISTER_CALLBACKS == 0) || !defined (USE_HAL_DFSDM_REGISTER_CALLBACKS) */

#if (USE_HAL_SAI_REGISTER_CALLBACKS == 0U)

/**
  * @brief  Half reception complete callback.
  * @param  hsai   SAI handle.
  * @retval None
  */
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Call the record update function to get the first half */
  if(hsai->Instance == AUDIO_IN_SAIx)
  {
    BSP_AUDIO_IN_HalfTransfer_CallBack(0);
  }
  else
  {
    BSP_AUDIO_IN_HalfTransfer_CallBack(2);
  }
}

/**
  * @brief  Reception complete callback.
  * @param  hsai   SAI handle.
  * @retval None
  */
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Call the record update function to get the second half */
  if(hsai->Instance == AUDIO_IN_SAIx)
  {
    BSP_AUDIO_IN_TransferComplete_CallBack(0);
  }
  else
  {
    BSP_AUDIO_IN_TransferComplete_CallBack(2);
  }
}
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 0U) */

/**
  * @brief  User callback when record buffer is filled.
  * @retval None
  */
__weak void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);

  /* This function should be implemented by the user application.
     It is called into this driver when the current buffer is filled
     to prepare the next buffer pointer and its size. */
}

/**
  * @brief  Manages the DMA Half Transfer complete event.
  * @retval None
  */
__weak void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);

  /* This function should be implemented by the user application.
     It is called into this driver when the current buffer is filled
     to prepare the next buffer pointer and its size. */
}

/**
  * @brief  Audio IN Error callback function.
  * @retval None
  */
__weak void BSP_AUDIO_IN_Error_CallBack(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);

  /* This function is called when an Interrupt due to transfer error on or peripheral
     error occurs. */
}
/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_AUDIO_Private_Functions Private Functions
  * @{
  */
/*******************************************************************************
                            Static Functions
*******************************************************************************/
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1U)
/**
  * @brief  Half reception complete callback.
  * @param  hsai   SAI handle.
  * @retval None
  */
static void SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Call the record update function to get the first half */
  if(hsai->Instance == AUDIO_IN_SAIx)
  {
    BSP_AUDIO_IN_HalfTransfer_CallBack(0);
  }
  else
  {
    BSP_AUDIO_IN_HalfTransfer_CallBack(2);
  }
}

/**
  * @brief  Reception complete callback.
  * @param  hsai   SAI handle.
  * @retval None
  */
static void SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Call the record update function to get the second half */
  if(hsai->Instance == AUDIO_IN_SAIx)
  {
    BSP_AUDIO_IN_TransferComplete_CallBack(0);
  }
  else
  {
    BSP_AUDIO_IN_TransferComplete_CallBack(2);
  }
}
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1U) */

#if (USE_AUDIO_CODEC_WM8994 == 1)
/**
  * @brief  Register Bus IOs if component ID is OK
  * @retval error status
  */
static int32_t WM8994_Probe(void)
{
  int32_t ret = BSP_ERROR_NONE;
  WM8994_IO_t              IOCtx;
  static WM8994_Object_t   WM8994Obj;
  uint32_t id;

  /* Configure the audio driver */
  IOCtx.Address     = AUDIO_I2C_ADDRESS;
  IOCtx.Init        = BSP_I2C1_Init;
  IOCtx.DeInit      = BSP_I2C1_DeInit;
  IOCtx.ReadReg     = BSP_I2C1_ReadReg16;
  IOCtx.WriteReg    = BSP_I2C1_WriteReg16;
  IOCtx.GetTick     = BSP_GetTick;

  if(WM8994_RegisterBusIO (&WM8994Obj, &IOCtx) != WM8994_OK)
  {
    ret = BSP_ERROR_BUS_FAILURE;
  }
  else
  {
    /* Reset the codec */
    if(WM8994_Reset(&WM8994Obj) != WM8994_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else if(WM8994_ReadID(&WM8994Obj, &id) != WM8994_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else if(id != WM8994_ID)
    {
      ret = BSP_ERROR_UNKNOWN_COMPONENT;
    }
    else
    {
      Audio_Drv = (AUDIO_Drv_t *) &WM8994_Driver;
      Audio_CompObj = &WM8994Obj;
    }
  }
  return ret;
}
#endif
#if (USE_AUDIO_CODEC_ADV7533 == 1)
/**
  * @brief  Register Bus IOs if component ID is OK
  * @retval error status
  */
static int32_t ADV7533_Probe(void)
{
  int32_t ret = BSP_ERROR_NONE;
  ADV7533_IO_t              IOCtx;
  static ADV7533_Object_t   ADV7533Obj;
  uint32_t id;

  /* Configure the audio driver */
  IOCtx.Address     = ADV7533_MAIN_I2C_ADDR;
  IOCtx.Init        = BSP_I2C1_Init;
  IOCtx.DeInit      = BSP_I2C1_DeInit;
  IOCtx.ReadReg     = BSP_I2C1_ReadReg;
  IOCtx.WriteReg    = BSP_I2C1_WriteReg;
  IOCtx.GetTick     = BSP_GetTick;

  if(ADV7533_RegisterBusIO (&ADV7533Obj, &IOCtx) != ADV7533_OK)
  {
    ret = BSP_ERROR_BUS_FAILURE;
  }
  else if(ADV7533_ReadID(&ADV7533Obj, &id) != ADV7533_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    if(id != ADV7533_ID)
    {
      ret = BSP_ERROR_UNKNOWN_COMPONENT;
    }
    else
    {
      Audio_Drv = (AUDIO_Drv_t *) &ADV7533_Driver;
      Audio_CompObj = &ADV7533Obj;
    }
  }

  return ret;
}
#endif

/**
  * @brief  Get SAI MX Init configuration
  * @param  hsai SAI handle
  * @param  MXConfig SAI configuration structure
  * @retval None
  */
static void SAI_InitMXConfigStruct(SAI_HandleTypeDef* hsai, MX_SAI_Config *MXConfig)
{
  MXConfig->AudioFrequency    = hsai->Init.AudioFrequency;
  MXConfig->ActiveFrameLength = hsai->FrameInit.ActiveFrameLength;
  MXConfig->AudioMode         = hsai->Init.AudioMode;
  MXConfig->ClockStrobing     = hsai->Init.ClockStrobing;
  MXConfig->DataSize          = hsai->Init.DataSize;
  MXConfig->FrameLength       = hsai->FrameInit.FrameLength;
  MXConfig->MonoStereoMode    = hsai->Init.MonoStereoMode;
  MXConfig->OutputDrive       = hsai->Init.OutputDrive;
  MXConfig->SlotActive        = hsai->SlotInit.SlotActive;
  MXConfig->Synchro           = hsai->Init.Synchro;
  MXConfig->SynchroExt        = hsai->Init.SynchroExt;
}

/**
  * @brief  Initialize BSP_AUDIO_OUT MSP.
  * @param  hsai  SAI handle
  * @retval None
  */
static void SAI_MspInit(SAI_HandleTypeDef *hsai)
{
  GPIO_InitTypeDef  gpio_init_structure;
  static DMA_HandleTypeDef hdma_sai_tx, hdma_sai_rx;

    /* Enable SAI clock */
    AUDIO_OUT_SAIx_CLK_ENABLE();

    /* Enable GPIO clock */
    AUDIO_OUT_SAIx_SCK_ENABLE();
    AUDIO_OUT_SAIx_SD_ENABLE();
    AUDIO_OUT_SAIx_FS_ENABLE();
    /* CODEC_SAI pins configuration: FS, SCK, MCK and SD pins ------------------*/
    gpio_init_structure.Pin = AUDIO_OUT_SAIx_FS_PIN;
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init_structure.Alternate = AUDIO_OUT_SAIx_FS_AF;
    HAL_GPIO_Init(AUDIO_OUT_SAIx_FS_GPIO_PORT, &gpio_init_structure);

    gpio_init_structure.Pin = AUDIO_OUT_SAIx_SCK_PIN;
    gpio_init_structure.Alternate = AUDIO_OUT_SAIx_SCK_AF;
    HAL_GPIO_Init(AUDIO_OUT_SAIx_SCK_GPIO_PORT, &gpio_init_structure);

    gpio_init_structure.Pin =  AUDIO_OUT_SAIx_SD_PIN;
    gpio_init_structure.Alternate = AUDIO_OUT_SAIx_SD_AF;
    HAL_GPIO_Init(AUDIO_OUT_SAIx_SD_GPIO_PORT, &gpio_init_structure);

    if(hsai->Instance != AUDIO_IN_SAI_PDMx)
    {
      if(haudio_in_sai.State != HAL_SAI_STATE_READY)
      {
        AUDIO_OUT_SAIx_MCLK_ENABLE();
        gpio_init_structure.Pin = AUDIO_OUT_SAIx_MCLK_PIN;
        gpio_init_structure.Alternate = AUDIO_OUT_SAIx_MCLK_AF;
        HAL_GPIO_Init(AUDIO_OUT_SAIx_MCLK_GPIO_PORT, &gpio_init_structure);
      }
    }

  if(hsai->Instance == AUDIO_OUT_SAIx)
  {
    /* Enable the DMA clock */
    AUDIO_OUT_SAIx_DMAx_CLK_ENABLE();

    /* Configure the hdma_saiTx handle parameters */
    if(Audio_Out_Ctx[0].BitsPerSample == AUDIO_RESOLUTION_16B)
    {
      hdma_sai_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
      hdma_sai_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    }
    else
    {
      hdma_sai_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
      hdma_sai_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    }

    hdma_sai_tx.Init.Request             = AUDIO_OUT_SAIx_DMAx_REQUEST;
    hdma_sai_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_sai_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_sai_tx.Init.Mode                = DMA_CIRCULAR;
    hdma_sai_tx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_sai_tx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    hdma_sai_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_sai_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_sai_tx.Instance                 = AUDIO_OUT_SAIx_DMAx_STREAM;
    hdma_sai_tx.Init.MemBurst            = DMA_MBURST_SINGLE;
    hdma_sai_tx.Init.PeriphBurst         = DMA_PBURST_SINGLE;

    /* Associate the DMA handle */
    __HAL_LINKDMA(hsai, hdmatx, hdma_sai_tx);

    /* Deinitialize the Stream for new transfer */
    (void)HAL_DMA_DeInit(&hdma_sai_tx);

    /* Configure the DMA Stream */
    (void)HAL_DMA_Init(&hdma_sai_tx);

    /* SAI DMA IRQ Channel configuration */
    HAL_NVIC_SetPriority(AUDIO_OUT_SAIx_DMAx_IRQ, BSP_AUDIO_OUT_IT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(AUDIO_OUT_SAIx_DMAx_IRQ);
  }

  /* Audio In Msp initialization */
  if(hsai->Instance == AUDIO_IN_SAIx)
  {
    /* Enable SAI clock */
    AUDIO_IN_SAIx_CLK_ENABLE();

    /* Enable SD GPIO clock */
    AUDIO_IN_SAIx_SD_ENABLE();
    /* CODEC_SAI pin configuration: SD pin */
    gpio_init_structure.Pin = AUDIO_IN_SAIx_SD_PIN;
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio_init_structure.Alternate = AUDIO_IN_SAIx_AF;
    HAL_GPIO_Init(AUDIO_IN_SAIx_SD_GPIO_PORT, &gpio_init_structure);

    /* Enable the DMA clock */
    AUDIO_IN_SAIx_DMAx_CLK_ENABLE();

    /* Configure the hdma_sai_rx handle parameters */
    hdma_sai_rx.Instance                 = AUDIO_IN_SAIx_DMAx_STREAM;
    hdma_sai_rx.Init.Request             = AUDIO_IN_SAIx_DMAx_REQUEST;
    hdma_sai_rx.Init.Mode                = DMA_CIRCULAR;
    hdma_sai_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_sai_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_sai_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_sai_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_sai_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_sai_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_sai_rx.Init.MemBurst            = DMA_MBURST_SINGLE;
    hdma_sai_rx.Init.PeriphBurst         = DMA_MBURST_SINGLE;

    if(Audio_In_Ctx[0].BitsPerSample == AUDIO_RESOLUTION_16B)
    {
      hdma_sai_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
      hdma_sai_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    }
    else
    {
      hdma_sai_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
      hdma_sai_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    }

    /* Associate the DMA handle */
    __HAL_LINKDMA(hsai, hdmarx, hdma_sai_rx);

    /* Deinitialize the Stream for new transfer */
    (void)HAL_DMA_DeInit(&hdma_sai_rx);

    /* Configure the DMA Stream */
    (void)HAL_DMA_Init(&hdma_sai_rx);

    /* SAI DMA IRQ Channel configuration */
    HAL_NVIC_SetPriority(AUDIO_IN_SAIx_DMAx_IRQ, BSP_AUDIO_IN_IT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(AUDIO_IN_SAIx_DMAx_IRQ);
  }

  if(hsai->Instance == AUDIO_IN_SAI_PDMx)
  {
     /* Enable SAI clock */
    AUDIO_IN_SAI_PDMx_CLK_ENABLE();

    AUDIO_IN_SAI_PDMx_CLK_IN_ENABLE();
    AUDIO_IN_SAI_PDMx_DATA_IN_ENABLE();

    gpio_init_structure.Pin = AUDIO_IN_SAI_PDMx_CLK_IN_PIN;
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio_init_structure.Alternate = AUDIO_IN_SAI_PDMx_DATA_CLK_AF;
    HAL_GPIO_Init(AUDIO_IN_SAI_PDMx_CLK_IN_PORT, &gpio_init_structure);

    gpio_init_structure.Pull = GPIO_PULLUP;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_MEDIUM;
    gpio_init_structure.Pin = AUDIO_IN_SAI_PDMx_DATA_IN_PIN;
    HAL_GPIO_Init(AUDIO_IN_SAI_PDMx_DATA_IN_PORT, &gpio_init_structure);

    AUDIO_IN_SAI_PDMx_FS_SCK_ENABLE();

    /* CODEC_SAI pins configuration: FS, SCK, MCK and SD pins ------------------*/
    gpio_init_structure.Pin = AUDIO_IN_SAI_PDMx_FS_PIN | AUDIO_IN_SAI_PDMx_SCK_PIN;
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio_init_structure.Alternate = AUDIO_IN_SAI_PDMx_FS_SCK_AF;
    HAL_GPIO_Init(AUDIO_IN_SAI_PDMx_FS_SCK_GPIO_PORT, &gpio_init_structure);

    /* Enable the DMA clock */
    AUDIO_IN_SAI_PDMx_DMAx_CLK_ENABLE();

    /* Configure the hdma_sai_rx handle parameters */
    hdma_sai_rx.Init.Request             = AUDIO_IN_SAI_PDMx_DMAx_REQUEST;
    hdma_sai_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_sai_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_sai_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_sai_rx.Init.PeriphDataAlignment = AUDIO_IN_SAI_PDMx_DMAx_PERIPH_DATA_SIZE;
    hdma_sai_rx.Init.MemDataAlignment    = AUDIO_IN_SAI_PDMx_DMAx_MEM_DATA_SIZE;
    hdma_sai_rx.Init.Mode                = DMA_CIRCULAR;
    hdma_sai_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_sai_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_sai_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_sai_rx.Init.MemBurst            = DMA_MBURST_SINGLE;
    hdma_sai_rx.Init.PeriphBurst         = DMA_MBURST_SINGLE;

    hdma_sai_rx.Instance = AUDIO_IN_SAI_PDMx_DMAx_STREAM;

    /* Associate the DMA handle */
    __HAL_LINKDMA(hsai, hdmarx, hdma_sai_rx);

    /* Deinitialize the Stream for new transfer */
    HAL_DMA_DeInit(&hdma_sai_rx);

    /* Configure the DMA Stream */
    HAL_DMA_Init(&hdma_sai_rx);

    /* SAI DMA IRQ Channel configuration */
    HAL_NVIC_SetPriority(AUDIO_IN_SAI_PDMx_DMAx_IRQ, BSP_AUDIO_IN_IT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(AUDIO_IN_SAI_PDMx_DMAx_IRQ);
  }
}

/**
  * @brief  Deinitializes SAI MSP.
  * @param  hsai  SAI handle
  * @retval HAL status
  */
static void SAI_MspDeInit(SAI_HandleTypeDef *hsai)
{
  GPIO_InitTypeDef  gpio_init_structure;
  if(hsai->Instance == AUDIO_OUT_SAIx)
  {
    /* SAI DMA IRQ Channel deactivation */
    HAL_NVIC_DisableIRQ(AUDIO_OUT_SAIx_DMAx_IRQ);

    /* Deinitialize the DMA stream */
    (void)HAL_DMA_DeInit(hsai->hdmatx);

    /* Disable SAI peripheral */
    __HAL_SAI_DISABLE(hsai);

    /* Deactivates CODEC_SAI pins FS, SCK, MCK and SD by putting them in input mode */
    gpio_init_structure.Pin = AUDIO_OUT_SAIx_FS_PIN;
    HAL_GPIO_DeInit(AUDIO_OUT_SAIx_FS_GPIO_PORT, gpio_init_structure.Pin);

    gpio_init_structure.Pin = AUDIO_OUT_SAIx_SCK_PIN;
    HAL_GPIO_DeInit(AUDIO_OUT_SAIx_SCK_GPIO_PORT, gpio_init_structure.Pin);

    gpio_init_structure.Pin =  AUDIO_OUT_SAIx_SD_PIN;
    HAL_GPIO_DeInit(AUDIO_OUT_SAIx_SD_GPIO_PORT, gpio_init_structure.Pin);

    gpio_init_structure.Pin = AUDIO_OUT_SAIx_MCLK_PIN;
    HAL_GPIO_DeInit(AUDIO_OUT_SAIx_MCLK_GPIO_PORT, gpio_init_structure.Pin);

    /* Disable SAI clock */
    AUDIO_OUT_SAIx_CLK_DISABLE();
  }
  if(hsai->Instance == AUDIO_IN_SAIx)
  {
    /* SAI DMA IRQ Channel deactivation */
    HAL_NVIC_DisableIRQ(AUDIO_IN_SAIx_DMAx_IRQ);

    /* Deinitialize the DMA stream */
    (void)HAL_DMA_DeInit(hsai->hdmarx);

    /* Disable SAI peripheral */
    __HAL_SAI_DISABLE(hsai);

    /* Deactivates CODEC_SAI pin SD by putting them in input mode */
    gpio_init_structure.Pin = AUDIO_IN_SAIx_SD_PIN;
    HAL_GPIO_DeInit(AUDIO_IN_SAIx_SD_GPIO_PORT, gpio_init_structure.Pin);

    /* Disable SAI clock */
    AUDIO_IN_SAIx_CLK_DISABLE();
  }
}


#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1U)
/**
  * @brief  Tx Transfer completed callbacks.
  * @param  hsai SAI handle
  * @retval None
  */
static void SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsai);

  /* Manage the remaining file size and new address offset: This function
     should be coded by user (its prototype is already declared in stm32h743i_eval_audio.h) */
  BSP_AUDIO_OUT_TransferComplete_CallBack(0);
}

/**
  * @brief  Tx Half Transfer completed callbacks.
  * @param  hsai  SAI handle
  * @retval None
  */
static void SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsai);

  /* Manage the remaining file size and new address offset: This function
     should be coded by user (its prototype is already declared in stm32h743i_eval_audio.h) */
  BSP_AUDIO_OUT_HalfTransfer_CallBack(0);
}

/**
  * @brief  SAI error callbacks.
  * @param  hsai SAI handle
  * @retval None
  */
static void SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
  if(hsai->Instance == AUDIO_OUT_SAIx)
  {
  BSP_AUDIO_OUT_Error_CallBack(0);
  }
  else
  {
  BSP_AUDIO_IN_Error_CallBack(0);
  }
}
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1U) */


#if (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1U)
/**
  * @brief  Regular conversion complete callback.
  * @note   In interrupt mode, user has to read conversion value in this function
            using HAL_DFSDM_FilterGetRegularValue.
  * @param  hdfsdm_filter   DFSDM filter handle.
  * @retval None
  */
static void DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  uint32_t index;
  static uint32_t DmaRecBuffCplt[DFSDM_MIC_NUMBER]  = {0};
  int32_t  tmp;

  if(Audio_In_Ctx[1].IsMultiBuff == 1U)
  {
    /* Call the record update function to get the second half */
    BSP_AUDIO_IN_TransferComplete_CallBack(2);
  }
  else
  {
    if(hdfsdm_filter == &haudio_in_dfsdm_filter[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)])
    {
      DmaRecBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)] = 1;
    }
    if(hdfsdm_filter == &haudio_in_dfsdm_filter[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)])
    {
      DmaRecBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)] = 1;
    }

    if(DmaRecBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)] == 1U)
    {
      if(DmaRecBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)] == 1U)
      {
#if (USE_BSP_CPU_CACHE_MAINTENANCE == 1)
      SCB_InvalidateDCache_by_Addr((uint32_t *)&MicRecBuff[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)][DEFAULT_AUDIO_IN_BUFFER_SIZE/2U], ((int32_t)DEFAULT_AUDIO_IN_BUFFER_SIZE/2)*4);
      SCB_InvalidateDCache_by_Addr((uint32_t *)&MicRecBuff[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)][DEFAULT_AUDIO_IN_BUFFER_SIZE/2U], ((int32_t)DEFAULT_AUDIO_IN_BUFFER_SIZE/2)*4);
#endif /* USE_BSP_CPU_CACHE_MAINTENANCE */
      for(index = (DEFAULT_AUDIO_IN_BUFFER_SIZE/2U) ; index < DEFAULT_AUDIO_IN_BUFFER_SIZE; index++)
      {
        if(Audio_In_Ctx[1].ChannelsNbr == 2U)
        {
          tmp = MicRecBuff[0][index] / 256;
          tmp = SaturaLH(tmp, -32768, 32767);
          Audio_In_Ctx[1].pBuff[RecBuffTrigger]     = (uint16_t)(tmp);
          tmp = MicRecBuff[1][index] / 256;
          tmp = SaturaLH(tmp, -32768, 32767);
          Audio_In_Ctx[1].pBuff[RecBuffTrigger + 1U] = (uint16_t)(tmp);
        }
        else
        {
          tmp = MicRecBuff[0][index] / 256;
          tmp = SaturaLH(tmp, -32768, 32767);
          Audio_In_Ctx[1].pBuff[RecBuffTrigger]      = (uint16_t)(tmp);
          Audio_In_Ctx[1].pBuff[RecBuffTrigger + 1U] = (uint16_t)(tmp);
        }
        RecBuffTrigger +=2U;
      }
#if (USE_BSP_CPU_CACHE_MAINTENANCE == 1)
      SCB_CleanDCache_by_Addr((uint32_t *)Audio_In_Ctx[1].pBuff,  (int32_t)Audio_In_Ctx[1].Size*2);
#endif /* USE_BSP_CPU_CACHE_MAINTENANCE */

      DmaRecBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)] = 0;
      DmaRecBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)] = 0;
    }
  }

    /* Call Half Transfer Complete callback */
    if(RecBuffTrigger == (Audio_In_Ctx[1].Size/4U))
    {
      if(RecBuffHalf == 0U)
      {
        RecBuffHalf = 1;
        BSP_AUDIO_IN_HalfTransfer_CallBack(2);
      }
    }
    /* Call Transfer Complete callback */
    if(RecBuffTrigger == Audio_In_Ctx[1].Size/2U)
    {
      /* Reset Application Buffer Trigger */
      RecBuffTrigger = 0;
      RecBuffHalf = 0;
      /* Call the record update function to get the next buffer to fill and its size (size is ignored) */
      BSP_AUDIO_IN_TransferComplete_CallBack(1);
    }
  }
}

/**
  * @brief  Half regular conversion complete callback.
  * @param  hdfsdm_filter   DFSDM filter handle.
  * @retval None
  */
static void DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{
  uint32_t index;
  static uint32_t DmaRecHalfBuffCplt[DFSDM_MIC_NUMBER]  = {0};
  int32_t  tmp;

  if(Audio_In_Ctx[1].IsMultiBuff == 1U)
  {
    /* Call the record update function to get the first half */
    BSP_AUDIO_IN_HalfTransfer_CallBack(1);
  }
  else
  {
    if(hdfsdm_filter == &haudio_in_dfsdm_filter[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)])
    {
      DmaRecHalfBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)] = 1;
    }
    if(hdfsdm_filter == &haudio_in_dfsdm_filter[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)])
    {
      DmaRecHalfBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)] = 1;
    }

    if(DmaRecHalfBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)] == 1U)
    {
      if(DmaRecHalfBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)] == 1U)
      {
        SCB_InvalidateDCache_by_Addr((uint32_t *)&MicRecBuff[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)][0], ((int32_t)DEFAULT_AUDIO_IN_BUFFER_SIZE/2)*4);
        SCB_InvalidateDCache_by_Addr((uint32_t *)&MicRecBuff[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)][0], ((int32_t)DEFAULT_AUDIO_IN_BUFFER_SIZE/2)*4);
        for(index = 0 ; index < (DEFAULT_AUDIO_IN_BUFFER_SIZE/2U); index++)
        {
          if(Audio_In_Ctx[1].ChannelsNbr == 2U)
          {
            tmp = MicRecBuff[0][index] / 256;
            tmp = SaturaLH(tmp, -32768, 32767);
            Audio_In_Ctx[1].pBuff[RecBuffTrigger]     = (uint16_t)(tmp);
            tmp = MicRecBuff[1][index] / 256;
            tmp = SaturaLH(tmp, -32768, 32767);
            Audio_In_Ctx[1].pBuff[RecBuffTrigger + 1U] = (uint16_t)(tmp);
          }
          else
          {
            tmp = MicRecBuff[0][index] / 256;
            tmp = SaturaLH(tmp, -32768, 32767);
            Audio_In_Ctx[1].pBuff[RecBuffTrigger]      = (uint16_t)(tmp);
            Audio_In_Ctx[1].pBuff[RecBuffTrigger + 1U] = (uint16_t)(tmp);
          }
          RecBuffTrigger +=2U;
        }

#if (USE_BSP_CPU_CACHE_MAINTENANCE == 1)
        SCB_CleanDCache_by_Addr((uint32_t *)Audio_In_Ctx[1].pBuff, ((int32_t)Audio_In_Ctx[1].Size*2));
#endif /* USE_BSP_CPU_CACHE_MAINTENANCE */

        DmaRecHalfBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)] = 0;
        DmaRecHalfBuffCplt[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)] = 0;
      }
    }

    /* Call Half Transfer Complete callback */
    if(RecBuffTrigger == (Audio_In_Ctx[1].Size/4U))
    {
      if(RecBuffHalf == 0U)
      {
        RecBuffHalf = 1;
        BSP_AUDIO_IN_HalfTransfer_CallBack(1);
      }
    }
    /* Call Transfer Complete callback */
    if(RecBuffTrigger == Audio_In_Ctx[1].Size/2U)
    {
      /* Reset Application Buffer Trigger */
      RecBuffTrigger = 0;
      RecBuffHalf = 0;
      /* Call the record update function to get the next buffer to fill and its size (size is ignored) */
      BSP_AUDIO_IN_TransferComplete_CallBack(1);
    }
  }
}
#endif /* (USE_HAL_DFSDM_REGISTER_CALLBACKS == 1U) */

/**
  * @brief  Initialize the DFSDM channel MSP.
  * @param  hDfsdmChannel DFSDM Channel handle
  * @retval None
  */
static void DFSDM_ChannelMspInit(DFSDM_Channel_HandleTypeDef *hDfsdmChannel)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(hDfsdmChannel);

  /* Enable DFSDM clock */
  AUDIO_DFSDMx_CLK_ENABLE();
  /* Enable GPIO clock */
 AUDIO_DFSDMx_CKOUT_GPIO_CLK_ENABLE();
 AUDIO_DFSDMx_DATIN_MIC1_GPIO_CLK_ENABLE();
 AUDIO_DFSDMx_DATIN_MIC2_GPIO_CLK_ENABLE();

  /* DFSDM pins configuration: DFSDM_CKOUT, DMIC_DATIN pins ------------------*/

  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

  GPIO_InitStruct.Pin = AUDIO_DFSDMx_CKOUT_PIN;
  GPIO_InitStruct.Alternate = AUDIO_DFSDMx_CKOUT_AF;
  HAL_GPIO_Init(AUDIO_DFSDMx_CKOUT_GPIO_PORT, &GPIO_InitStruct);

 GPIO_InitStruct.Pin = AUDIO_DFSDMx_DATIN_MIC1_PIN;
 GPIO_InitStruct.Alternate = AUDIO_DFSDMx_DATIN_MIC1_AF;
 HAL_GPIO_Init(AUDIO_DFSDMx_DATIN_MIC1_GPIO_PORT, &GPIO_InitStruct);
 GPIO_InitStruct.Pin = AUDIO_DFSDMx_DATIN_MIC2_PIN;
 GPIO_InitStruct.Alternate = AUDIO_DFSDMx_DATIN_MIC2_AF;
 HAL_GPIO_Init(AUDIO_DFSDMx_DATIN_MIC2_GPIO_PORT, &GPIO_InitStruct);
}

/**
  * @brief  DeInitialize the DFSDM channel MSP.
  * @param  hDfsdmChannel DFSDM Channel handle
  * @retval None
  */
static void DFSDM_ChannelMspDeInit(DFSDM_Channel_HandleTypeDef *hDfsdmChannel)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(hDfsdmChannel);

  /* DFSDM pins configuration: DFSDM_CKOUT, DMIC_DATIN pins ------------------*/
  GPIO_InitStruct.Pin = AUDIO_DFSDMx_CKOUT_PIN;
  HAL_GPIO_DeInit(AUDIO_DFSDMx_CKOUT_GPIO_PORT, GPIO_InitStruct.Pin);

  GPIO_InitStruct.Pin = AUDIO_DFSDMx_DATIN_MIC1_PIN;
  HAL_GPIO_DeInit(AUDIO_DFSDMx_DATIN_MIC1_GPIO_PORT, GPIO_InitStruct.Pin);
  GPIO_InitStruct.Pin = AUDIO_DFSDMx_DATIN_MIC2_PIN;
  HAL_GPIO_DeInit(AUDIO_DFSDMx_DATIN_MIC2_GPIO_PORT, GPIO_InitStruct.Pin);
}

/**
  * @brief  Initialize the DFSDM filter MSP.
  * @param  hDfsdmFilter DFSDM Filter handle
  * @retval None
  */
static void DFSDM_FilterMspInit(DFSDM_Filter_HandleTypeDef *hDfsdmFilter)
{
  uint32_t i, mic_num = 0, mic_init[DFSDM_MIC_NUMBER] = {0};
  IRQn_Type AUDIO_DFSDM_DMAx_MIC_IRQHandler[DFSDM_MIC_NUMBER] = {AUDIO_DFSDMx_DMAx_MIC1_IRQ, AUDIO_DFSDMx_DMAx_MIC2_IRQ};
  DMA_Stream_TypeDef* AUDIO_DFSDMx_DMAx_MIC_STREAM[DFSDM_MIC_NUMBER] = {AUDIO_DFSDMx_DMAx_MIC1_STREAM, AUDIO_DFSDMx_DMAx_MIC2_STREAM};
  uint32_t AUDIO_DFSDMx_DMAx_MIC_REQUEST[DFSDM_MIC_NUMBER] = {AUDIO_DFSDMx_DMAx_MIC1_REQUEST, AUDIO_DFSDMx_DMAx_MIC2_REQUEST};

  /* Prevent unused argument(s) compilation warning */
  UNUSED(hDfsdmFilter);

  /* Enable DFSDM clock */
  AUDIO_DFSDMx_CLK_ENABLE();
  /* Enable the DMA clock */
  AUDIO_DFSDMx_DMAx_CLK_ENABLE();

  for(i = 0; i < DFSDM_MIC_NUMBER; i++)
  {
    if(((Audio_In_Ctx[1].Device & AUDIO_IN_DEVICE_DIGITAL_MIC1) == AUDIO_IN_DEVICE_DIGITAL_MIC1) && (mic_init[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1)] != 1U))
    {
      mic_num = POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC1);
      mic_init[mic_num] = 1;
    }
    else if(((Audio_In_Ctx[1].Device & AUDIO_IN_DEVICE_DIGITAL_MIC2) == AUDIO_IN_DEVICE_DIGITAL_MIC2) && (mic_init[POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2)] != 1U))
    {
      mic_num = POS_VAL(AUDIO_IN_DEVICE_DIGITAL_MIC2);
      mic_init[mic_num] = 1;
    }
    else
    {
    }
    /* Configure the hDmaDfsdm[i] handle parameters */
    hDmaDfsdm[mic_num].Init.Request             = AUDIO_DFSDMx_DMAx_MIC_REQUEST[mic_num];
    hDmaDfsdm[mic_num].Instance                 = AUDIO_DFSDMx_DMAx_MIC_STREAM[mic_num];
    hDmaDfsdm[mic_num].Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hDmaDfsdm[mic_num].Init.PeriphInc           = DMA_PINC_DISABLE;
    hDmaDfsdm[mic_num].Init.MemInc              = DMA_MINC_ENABLE;
    hDmaDfsdm[mic_num].Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hDmaDfsdm[mic_num].Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    hDmaDfsdm[mic_num].Init.Mode                = DMA_CIRCULAR;
    hDmaDfsdm[mic_num].Init.Priority            = DMA_PRIORITY_HIGH;
    hDmaDfsdm[mic_num].Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hDmaDfsdm[mic_num].Init.MemBurst            = DMA_MBURST_SINGLE;
    hDmaDfsdm[mic_num].Init.PeriphBurst         = DMA_PBURST_SINGLE;
    hDmaDfsdm[mic_num].State                    = HAL_DMA_STATE_RESET;

    /* Associate the DMA handle */
    __HAL_LINKDMA(&haudio_in_dfsdm_filter[mic_num], hdmaReg, hDmaDfsdm[mic_num]);

    /* Reset DMA handle state */
    __HAL_DMA_RESET_HANDLE_STATE(&hDmaDfsdm[mic_num]);

    /* Configure the DMA Channel */
    (void)HAL_DMA_Init(&hDmaDfsdm[mic_num]);

    /* DMA IRQ Channel configuration */
    HAL_NVIC_SetPriority(AUDIO_DFSDM_DMAx_MIC_IRQHandler[mic_num], BSP_AUDIO_IN_IT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(AUDIO_DFSDM_DMAx_MIC_IRQHandler[mic_num]);
  }
}

/**
  * @brief  DeInitialize the DFSDM filter MSP.
  * @param  hDfsdmFilter DFSDM Filter handle
  * @retval None
  */
static void DFSDM_FilterMspDeInit(DFSDM_Filter_HandleTypeDef *hDfsdmFilter)
{
  uint32_t i;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(hDfsdmFilter);

  /* Configure the DMA Channel */
  for(i = 0; i < DFSDM_MIC_NUMBER; i++)
  {
    if(hDmaDfsdm[i].Instance != NULL)
    {
      (void)HAL_DMA_DeInit(&hDmaDfsdm[i]);
    }
  }
}

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
