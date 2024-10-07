/**
  ******************************************************************************
  * @file    stm32h573i_discovery_audio.c
  * @author  MCD Application Team
  * @brief   This file provides the Audio driver for the STM32H573I_DK board.
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
  @verbatim
  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================
   + This driver supports STM32H5xx devices on STM32H573I_DK(MB1677) board.

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
      the codec has failed (try to un-plug the power or reset device in this case).

      User can update the SAI or the clock configurations by overriding the weak MX functions
      MX_SAI2_Init() and  MX_SAI2_ClockConfig()
      User can override the default MSP configuration and register his own MSP callbacks (defined at application level)
      by calling BSP_AUDIO_OUT_RegisterMspCallbacks() function
      User can restore the default MSP configuration by calling BSP_AUDIO_OUT_RegisterDefaultMspCallbacks()
      To use these two functions, user have to enable USE_HAL_SAI_REGISTER_CALLBACKS
      within stm32h5xx_hal_conf.h file


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
      the STM32H7B3I_EVAL_audio.h file. (refer to the example for more details on the callbacks implementations)


   + Call the function BSP_AUDIO_IN_Init() for AUDIO IN initialization:
        Instance : Select the input instance. Can be 0 (SAI) or 1 (SAI PDM)
        AudioInit: Audio In structure to select the following parameters
                   - Device: Select the input device (analog, digital micx)
                   - SampleRate: Select the input sample rate (8Khz .. 96Khz)
                   - BitsPerSample: Select the input resolution (16 or 32bits per sample)
                   - ChannelsNbr: Select the input channels number(1 for mono, 2 for stereo)
                   - Volume: Select the input volume(0% .. 100%)


      This function configures all the hardware required for the audio application (codec, I2C, SAI,
      GPIOs, DMA and interrupt if needed). This function returns BSP_ERROR_NONE if configuration is OK.
      If the returned value is different from BSP_ERROR_NONE or the function is stuck then the communication with
      the codec has failed (try to un-plug the power or reset device in this case).
      User can update the SAI or the clock configurations by overriding the weak MX functions MX_SAIx_Init() and
      MX_SAIx_ClockConfig()
      User can override the default MSP configuration and register his own MSP callbacks (defined at application level)
      by calling BSP_AUDIO_IN_RegisterMspCallbacks() function
      User can restore the default MSP configuration by calling BSP_AUDIO_IN_RegisterDefaultMspCallbacks()
      To use these two functions, user have to enable USE_HAL_SAI_REGISTER_CALLBACKS within stm32h5xx_hal_conf.h file

   + Call the function BSP_AUDIO_IN_Record() to record audio stream. The recorded data are stored to user buffer in raw
        (L, R, L, R ...)
        Instance : Select the input instance. Can be 0 (SAI)
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

   + For each mode, you may need to implement the relative callback functions into your code.
      The Callback functions are named AUDIO_IN_XXX_CallBack() and only their prototypes are declared in
      the stm32h573i_discovery_audio.h file. (refer to the example for more details on the callbacks implementations)

   + The driver API and the callback functions are at the end of the stm32h573i_discovery_audio.h file.

  @endverbatim
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32h573i_discovery_audio.h"
#include "stm32h573i_discovery_bus.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H573I_DK
  * @{
  */

/** @defgroup STM32H573I_DK_AUDIO AUDIO
  * @{
  */

/** @defgroup STM32H573I_DK_AUDIO_Private_Macros AUDIO Private Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup STM32H573I_DK_AUDIO_Exported_Variables AUDIO Exported Variables
  * @{
  */
/* Audio in and out context */
AUDIO_IN_Ctx_t  Audio_In_Ctx[AUDIO_IN_INSTANCES_NBR] = {{0}, {0}};

/* Audio component object */
void *Audio_CompObj = NULL;

/* Audio driver */
AUDIO_Drv_t *Audio_Drv = NULL;

/* Play  */
SAI_HandleTypeDef                      haudio_out_sai = {NULL};
AUDIO_OUT_Ctx_t                        Audio_Out_Ctx[AUDIO_OUT_INSTANCES_NBR];

/* Record  */
SAI_HandleTypeDef                      haudio_in_sai = {NULL};

/* Audio in and out DMA handles used by SAI */
DMA_HandleTypeDef hDmaSaiTx = {NULL}, hDmaSaiRx = {NULL};

/* Queue variables declaration */
static DMA_QListTypeDef SAITxQueue, SAIRxQueue;

/**
  * @}
  */

/** @defgroup STM32H573I_DK_AUDIO_Private_Variables AUDIO Private Variables
  * @{
  */

/**
  * @}
  */

/** @defgroup STM32H573I_DK_AUDIO_Private_Function_Prototypes AUDIO Private Function Prototypes
  * @{
  */
#if (USE_AUDIO_CODEC_CS42L51 == 1)
static int32_t CS42L51_Probe(void);
static int32_t CS42L51_PowerUp(void);
static int32_t CS42L51_PowerDown(void);
#endif /* USE_AUDIO_CODEC_CS42L51 */

static void SAI_MspInit(SAI_HandleTypeDef *hsai);
static void SAI_MspDeInit(SAI_HandleTypeDef *hsai);

/* SAI callbacks */
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1)
static void SAI_TxCpltCallback(SAI_HandleTypeDef *hsai);
static void SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai);
static void SAI_ErrorCallback(SAI_HandleTypeDef *hsai);
static void SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai);
static void SAI_RxCpltCallback(SAI_HandleTypeDef *hsai);
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1) */

/**
  * @}
  */

/** @addtogroup STM32H573I_DK_AUDIO_OUT_Exported_Functions
  * @{
  */
/**
  * @brief  Initialize the audio out peripherals.
  * @param  Instance Audio out instance.
  * @param  AudioInit Audio out init structure.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_Init(uint32_t Instance, BSP_AUDIO_Init_t *AudioInit)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else if ((Instance == 0U) && ((AudioInit->BitsPerSample == AUDIO_RESOLUTION_32B)
                                || (AudioInit->BitsPerSample == AUDIO_RESOLUTION_8B)))
  {
    status = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if ((Instance == 0U) && (Audio_In_Ctx[0].State != AUDIO_IN_STATE_RESET) &&
           ((Audio_In_Ctx[0].SampleRate != AudioInit->SampleRate) ||
            (Audio_In_Ctx[0].BitsPerSample != AudioInit->BitsPerSample)))
  {
    status = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  else if (Audio_Out_Ctx[Instance].State != AUDIO_OUT_STATE_RESET)
  {
    status = BSP_ERROR_BUSY;
  }
  else
  {
    /* Un-reset audio codec if not currently used by audio in instances */
    if (Audio_In_Ctx[0].State == AUDIO_IN_STATE_RESET)
    {
      (void)CS42L51_PowerUp();
    }

    /* Fill audio out context structure */
    Audio_Out_Ctx[Instance].Device         = AudioInit->Device;
    Audio_Out_Ctx[Instance].SampleRate     = AudioInit->SampleRate;
    Audio_Out_Ctx[Instance].BitsPerSample  = AudioInit->BitsPerSample;
    Audio_Out_Ctx[Instance].ChannelsNbr    = AudioInit->ChannelsNbr;
    Audio_Out_Ctx[Instance].Volume         = AudioInit->Volume;

    /* Probe the audio codec */
    if ((Audio_Out_Ctx[0].State == AUDIO_OUT_STATE_RESET) && \
      (Audio_In_Ctx[0].State == AUDIO_IN_STATE_RESET))
    {
      if (CS42L51_Probe() != BSP_ERROR_NONE)
      {
        status = BSP_ERROR_COMPONENT_FAILURE;
      }
    }

    if (status == BSP_ERROR_NONE)
    {
      /* Set SAI instance */
      haudio_out_sai.Instance = AUDIO_OUT_SAI;
      
      /* Configure the SAI PLL according to the requested audio frequency if not already done by other instances */
      if (Audio_In_Ctx[0].State == AUDIO_IN_STATE_RESET)
      {
        if (MX_SAI2_ClockConfig(&haudio_out_sai, AudioInit->SampleRate) != HAL_OK)
        {
          status = BSP_ERROR_CLOCK_FAILURE;
        }
      }
      if (status == BSP_ERROR_NONE)
      {
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 0)
        SAI_MspInit(&haudio_out_sai);
#else
        /* Register the SAI MSP Callbacks */
        if (Audio_Out_Ctx[Instance].IsMspCallbacksValid == 0U)
        {
          if (BSP_AUDIO_OUT_RegisterDefaultMspCallbacks(Instance) != BSP_ERROR_NONE)
          {
            status = BSP_ERROR_PERIPH_FAILURE;
          }
        }
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 0) */
      }

      if (status == BSP_ERROR_NONE)
      {
        /* Prepare SAI peripheral initialization */
        MX_SAI_Config_t mxSaiInit;
        mxSaiInit.AudioFrequency    = AudioInit->SampleRate;
        mxSaiInit.AudioMode         = SAI_MODEMASTER_TX;
        mxSaiInit.ClockStrobing     = SAI_CLOCKSTROBING_FALLINGEDGE;
        mxSaiInit.MonoStereoMode    = (AudioInit->ChannelsNbr == 1U) ? SAI_MONOMODE : SAI_STEREOMODE;
        if (AudioInit->BitsPerSample == AUDIO_RESOLUTION_24B)
        {
          mxSaiInit.DataSize          = SAI_DATASIZE_24;
          mxSaiInit.FrameLength       = 64;
          mxSaiInit.ActiveFrameLength = 32;
        }
        else
        {
          mxSaiInit.DataSize          = SAI_DATASIZE_16;
          mxSaiInit.FrameLength       = 32;
          mxSaiInit.ActiveFrameLength = 16;
        }
        mxSaiInit.OutputDrive       = SAI_OUTPUTDRIVE_ENABLE;
        mxSaiInit.Synchro           = SAI_ASYNCHRONOUS;
        mxSaiInit.SynchroExt        = SAI_SYNCEXT_DISABLE;
        mxSaiInit.SlotActive        = SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1;

        /* SAI peripheral initialization */
        if (MX_SAI2_Init(&haudio_out_sai, &mxSaiInit) != HAL_OK)
        {
          status = BSP_ERROR_PERIPH_FAILURE;
        }
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1)
        /* Register SAI TC, HT and Error callbacks */
        else if (HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_TX_COMPLETE_CB_ID, SAI_TxCpltCallback) != HAL_OK)
        {
          status = BSP_ERROR_PERIPH_FAILURE;
        }
        else if (HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_TX_HALFCOMPLETE_CB_ID, SAI_TxHalfCpltCallback)
                 != HAL_OK)
        {
          status = BSP_ERROR_PERIPH_FAILURE;
        }
        else if (HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_ERROR_CB_ID, SAI_ErrorCallback) != HAL_OK)
        {
          status = BSP_ERROR_PERIPH_FAILURE;
        }
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1) */
        else
        {
          /* Initialize audio codec */
          CS42L51_Init_t codec_init;
          codec_init.InputDevice  = (Audio_In_Ctx[0].State == AUDIO_IN_STATE_RESET)
                                     ? CS42L51_IN_NONE: CS42L51_IN_MIC1;
          codec_init.OutputDevice = CS42L51_OUT_HEADPHONE;
          codec_init.Frequency    = AudioInit->SampleRate;
          codec_init.Resolution   = CS42L51_RESOLUTION_16B; /* Not used */
          codec_init.Volume       = AudioInit->Volume;
          if (Audio_Drv->Init(Audio_CompObj, &codec_init) < 0)
          {
            status = BSP_ERROR_COMPONENT_FAILURE;
          }
          else
          {
            /* Update audio out context state */
            Audio_Out_Ctx[Instance].State = AUDIO_OUT_STATE_STOP;
          }
        }
      }
    }
  }
  return status;
}

/**
* @brief  De-initialize the audio out peripherals.
* @param  Instance Audio out instance.
* @retval BSP status.
*/
int32_t BSP_AUDIO_OUT_DeInit(uint32_t Instance)
{
  int32_t status = BSP_ERROR_NONE;
  
  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else if (Audio_Out_Ctx[Instance].State != AUDIO_OUT_STATE_RESET)
  {
    /* Reset audio codec if not currently used by audio in instance 0 */
    if (Audio_In_Ctx[0].State == AUDIO_IN_STATE_RESET)
    {
      (void)CS42L51_PowerDown();
    }
    
    /* SAI peripheral de-initialization */
    if (HAL_SAI_DeInit(&haudio_out_sai) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
    /* De-initialize audio codec if not currently used by audio in instance 0 */
    else
    {
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 0)
      SAI_MspDeInit(&haudio_out_sai);
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 0) */
      if (Audio_In_Ctx[0].State == AUDIO_IN_STATE_RESET)
      {
        if (Audio_Drv->DeInit(Audio_CompObj) < 0)
        {
          status = BSP_ERROR_COMPONENT_FAILURE;
        }
      }
    }
    
    if (status == BSP_ERROR_NONE)
    {
      /* Update audio out context */
      Audio_Out_Ctx[Instance].State  = AUDIO_OUT_STATE_RESET;
      Audio_Out_Ctx[Instance].IsMute = 0U;
    }
  }
  else
  {
    /* Nothing to do */
  }
  return status;
}

/**
  * @brief  Start playing audio stream from a data buffer for a determined size.
  * @param  Instance Audio out instance.
  * @param  pData Pointer on data buffer.
  * @param  NbrOfBytes Size of buffer in bytes. Maximum size is 65535 bytes.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_Play(uint32_t Instance, uint8_t *pData, uint32_t NbrOfBytes)
{
  int32_t  status = BSP_ERROR_NONE;
  uint16_t NbrOfDmaDatas;

  if ((Instance >= AUDIO_OUT_INSTANCES_NBR) || (pData == NULL))
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  /* Check audio out state */
  else if (Audio_Out_Ctx[Instance].State != AUDIO_OUT_STATE_STOP)
  {
    status = BSP_ERROR_BUSY;
  }
  else
  {
    /* Compute number of DMA data to tranfser according resolution */
    if (Audio_Out_Ctx[Instance].BitsPerSample == AUDIO_RESOLUTION_16B)
    {
      NbrOfDmaDatas = (uint16_t)(NbrOfBytes / 2U);
    }
    else /* AUDIO_RESOLUTION_24b */
    {
      NbrOfDmaDatas = (uint16_t)(NbrOfBytes / 4U);
    }
    /* Initiate a DMA transfer of audio samples towards the serial audio interface */
    if (HAL_SAI_Transmit_DMA(&haudio_out_sai, pData, NbrOfDmaDatas) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
    /* Call the audio codec play function */
    else if (Audio_Drv->Play(Audio_CompObj) < 0)
    {
      status = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Update audio out state */
      Audio_Out_Ctx[Instance].State = AUDIO_OUT_STATE_PLAYING;
    }
  }
  return status;
}

/**
  * @brief  Pause playback of audio stream.
  * @param  Instance Audio out instance.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_Pause(uint32_t Instance)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  /* Check audio out state */
  else if (Audio_Out_Ctx[Instance].State != AUDIO_OUT_STATE_PLAYING)
  {
    status = BSP_ERROR_BUSY;
  }
  /* Call the audio codec pause function */
  else if (Audio_Drv->Pause(Audio_CompObj) < 0)
  {
    status = BSP_ERROR_COMPONENT_FAILURE;
  }
  /* Pause DMA transfer of audio samples towards the serial audio interface */
  else if (HAL_SAI_DMAPause(&haudio_out_sai) != HAL_OK)
  {
    status = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    /* Update audio out state */
    Audio_Out_Ctx[Instance].State = AUDIO_OUT_STATE_PAUSE;
  }
  return status;
}

/**
  * @brief  Resume playback of audio stream.
  * @param  Instance Audio out instance.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_Resume(uint32_t Instance)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  /* Check audio out state */
  else if (Audio_Out_Ctx[Instance].State != AUDIO_OUT_STATE_PAUSE)
  {
    status = BSP_ERROR_BUSY;
  }
  /* Call the audio codec resume function */
  else if (Audio_Drv->Resume(Audio_CompObj) < 0)
  {
    status = BSP_ERROR_COMPONENT_FAILURE;
  }
  /* Resume DMA transfer of audio samples towards the serial audio interface */
  else if (HAL_SAI_DMAResume(&haudio_out_sai) != HAL_OK)
  {
    status = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    /* Update audio out state */
    Audio_Out_Ctx[Instance].State = AUDIO_OUT_STATE_PLAYING;
  }
  return status;
}

/**
  * @brief  Stop playback of audio stream.
  * @param  Instance Audio out instance.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_Stop(uint32_t Instance)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  /* Check audio out state */
  else if (Audio_Out_Ctx[Instance].State == AUDIO_OUT_STATE_STOP)
  {
    /* Nothing to do */
  }
  else if ((Audio_Out_Ctx[Instance].State != AUDIO_OUT_STATE_PLAYING) &&
           (Audio_Out_Ctx[Instance].State != AUDIO_OUT_STATE_PAUSE))
  {
    status = BSP_ERROR_BUSY;
  }
  /* Call the audio codec stop function */
  else if (Audio_Drv->Stop(Audio_CompObj, CS42L51_PDWN_SW) < 0)
  {
    status = BSP_ERROR_COMPONENT_FAILURE;
  }
  /* Stop DMA transfer of audio samples towards the serial audio interface */
  else if (HAL_SAI_DMAStop(&haudio_out_sai) != HAL_OK)
  {
    status = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    /* Update audio out state */
    Audio_Out_Ctx[Instance].State = AUDIO_OUT_STATE_STOP;
  }
  return status;
}

/**
  * @brief  Mute playback of audio stream.
  * @param  Instance Audio out instance.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_Mute(uint32_t Instance)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  /* Check audio out state */
  else if (Audio_Out_Ctx[Instance].State == AUDIO_OUT_STATE_RESET)
  {
    status = BSP_ERROR_BUSY;
  }
  /* Check audio out mute status */
  else if (Audio_Out_Ctx[Instance].IsMute == 1U)
  {
    /* Nothing to do */
  }
  /* Call the audio codec mute function */
  else if (Audio_Drv->SetMute(Audio_CompObj, CS42L51_MUTE_ON) < 0)
  {
    status = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    /* Update audio out mute status */
    Audio_Out_Ctx[Instance].IsMute = 1U;
  }
  return status;
}

/**
  * @brief  Unmute playback of audio stream.
  * @param  Instance Audio out instance.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_UnMute(uint32_t Instance)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  /* Check audio out state */
  else if (Audio_Out_Ctx[Instance].State == AUDIO_OUT_STATE_RESET)
  {
    status = BSP_ERROR_BUSY;
  }
  /* Check audio out mute status */
  else if (Audio_Out_Ctx[Instance].IsMute == 0U)
  {
    /* Nothing to do */
  }
  /* Call the audio codec mute function */
  else if (Audio_Drv->SetMute(Audio_CompObj, CS42L51_MUTE_OFF) < 0)
  {
    status = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    /* Update audio out mute status */
    Audio_Out_Ctx[Instance].IsMute = 0U;
  }
  return status;
}

/**
  * @brief  Check audio out mute status.
  * @param  Instance Audio out instance.
  * @param  IsMute Pointer to mute status. Value is 1 for mute, 0 for unmute status.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_IsMute(uint32_t Instance, uint32_t *IsMute)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  /* Check audio out state */
  else if (Audio_Out_Ctx[Instance].State == AUDIO_OUT_STATE_RESET)
  {
    status = BSP_ERROR_BUSY;
  }
  /* Get the current audio out mute status */
  else
  {
    *IsMute = Audio_Out_Ctx[Instance].IsMute;
  }
  return status;
}

/**
  * @brief  Set audio out volume.
  * @param  Instance Audio out instance.
  * @param  Volume Volume level in percentage from 0% to 100%.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_SetVolume(uint32_t Instance, uint32_t Volume)
{
  int32_t status = BSP_ERROR_NONE;

  if ((Instance >= AUDIO_OUT_INSTANCES_NBR) || (Volume > 100U))
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  /* Check audio out state */
  else if (Audio_Out_Ctx[Instance].State == AUDIO_OUT_STATE_RESET)
  {
    status = BSP_ERROR_BUSY;
  }
  else
  {
    /* Call the audio codec volume control function */
    if (Audio_Drv->SetVolume(Audio_CompObj, VOLUME_OUTPUT, (uint8_t) Volume) < 0)
    {
      status = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Store volume on audio out context */
      Audio_Out_Ctx[Instance].Volume = Volume;
    }
  }
  return status;
}

/**
  * @brief  Get audio out volume.
  * @param  Instance Audio out instance.
  * @param  Volume Pointer to volume level in percentage from 0% to 100%.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_GetVolume(uint32_t Instance, uint32_t *Volume)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  /* Check audio out state */
  else if (Audio_Out_Ctx[Instance].State == AUDIO_OUT_STATE_RESET)
  {
    status = BSP_ERROR_BUSY;
  }
  /* Get the current audio out volume */
  else
  {
    *Volume = Audio_Out_Ctx[Instance].Volume;
  }
  return status;
}

/**
  * @brief  Set audio out sample rate.
  * @param  Instance Audio out instance.
  * @param  SampleRate Sample rate of the audio out stream.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_SetSampleRate(uint32_t Instance, uint32_t SampleRate)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  /* Check audio out state */
  else if (Audio_Out_Ctx[Instance].State != AUDIO_OUT_STATE_STOP)
  {
    status = BSP_ERROR_BUSY;
  }
  /* Check if record on instance 0 is on going and corresponding sample rate */
  else if ((Audio_In_Ctx[0].State != AUDIO_IN_STATE_RESET) &&
           (Audio_In_Ctx[0].SampleRate != SampleRate))
  {
    status = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  /* Check if sample rate is modified */
  else if (Audio_Out_Ctx[Instance].SampleRate == SampleRate)
  {
    /* Nothing to do */
  }
  else
  {
    /* Update SAI1 clock config */
    haudio_out_sai.Init.AudioFrequency = SampleRate;
    if (MX_SAI2_ClockConfig(&haudio_out_sai, SampleRate) != HAL_OK)
    {
      status = BSP_ERROR_CLOCK_FAILURE;
    }
    /* Re-initialize SAI1 with new sample rate */
    else if (HAL_SAI_Init(&haudio_out_sai) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1)
    /* Register SAI TC, HT and Error callbacks */
    else if (HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_TX_COMPLETE_CB_ID, SAI_TxCpltCallback) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_TX_HALFCOMPLETE_CB_ID, SAI_TxHalfCpltCallback) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_ERROR_CB_ID, SAI_ErrorCallback) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1) */
    /* Store new sample rate on audio out context */
    else
    {
      Audio_Out_Ctx[Instance].SampleRate = SampleRate;
    }
  }
  return status;
}

/**
  * @brief  Get audio out sample rate.
  * @param  Instance Audio out instance.
  * @param  SampleRate Pointer to sample rate of the audio out stream.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_GetSampleRate(uint32_t Instance, uint32_t *SampleRate)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  /* Check audio out state */
  else if (Audio_Out_Ctx[Instance].State == AUDIO_OUT_STATE_RESET)
  {
    status = BSP_ERROR_BUSY;
  }
  /* Get the current audio out sample rate */
  else
  {
    *SampleRate = Audio_Out_Ctx[Instance].SampleRate;
  }
  return status;
}

/**
  * @brief  Set audio out device.
  * @param  Instance Audio out instance.
  * @param  Device Device of the audio out stream.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_SetDevice(uint32_t Instance, uint32_t Device)
{
  int32_t status = BSP_ERROR_NONE;

  UNUSED(Device);

  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  /* Check audio out state */
  else if (Audio_Out_Ctx[Instance].State != AUDIO_OUT_STATE_STOP)
  {
    status = BSP_ERROR_BUSY;
  }
  else
  {
    /* Nothing to do because there is only one device (AUDIO_OUT_HEADPHONE) */
  }
  return status;
}

/**
  * @brief  Get audio out device.
  * @param  Instance Audio out instance.
  * @param  Device Pointer to device of the audio out stream.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_GetDevice(uint32_t Instance, uint32_t *Device)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  /* Check audio out state */
  else if (Audio_Out_Ctx[Instance].State == AUDIO_OUT_STATE_RESET)
  {
    status = BSP_ERROR_BUSY;
  }
  /* Get the current audio out device */
  else
  {
    *Device = Audio_Out_Ctx[Instance].Device;
  }
  return status;
}

/**
  * @brief  Set bits per sample for the audio out stream.
  * @param  Instance Audio out instance.
  * @param  BitsPerSample Bits per sample of the audio out stream.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_SetBitsPerSample(uint32_t Instance, uint32_t BitsPerSample)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else if ((Instance == 0U) && ((BitsPerSample == AUDIO_RESOLUTION_32B) || (BitsPerSample == AUDIO_RESOLUTION_8B)))
  {
    status = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }
  /* Check audio out state */
  else if (Audio_Out_Ctx[Instance].State != AUDIO_OUT_STATE_STOP)
  {
    status = BSP_ERROR_BUSY;
  }
  else
  {
    /* Store new bits per sample on audio out context */
    Audio_Out_Ctx[Instance].BitsPerSample = BitsPerSample;

    /* Update data size, frame length and active frame length parameters of SAI handle */
    if (BitsPerSample == AUDIO_RESOLUTION_24B)
    {
      haudio_out_sai.Init.DataSize               = SAI_DATASIZE_24;
      haudio_out_sai.FrameInit.FrameLength       = 64;
      haudio_out_sai.FrameInit.ActiveFrameLength = 32;
    }
    else
    {
      haudio_out_sai.Init.DataSize               = SAI_DATASIZE_16;
      haudio_out_sai.FrameInit.FrameLength       = 32;
      haudio_out_sai.FrameInit.ActiveFrameLength = 16;
    }

#if (USE_HAL_SAI_REGISTER_CALLBACKS == 0)
    SAI_MspInit(&haudio_out_sai);
#else
    /* Update SAI state only to keep current MSP functions */
    haudio_out_sai.State = HAL_SAI_STATE_RESET;
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 0) */

    /* Re-initialize SAI1 with new parameters */
    if (HAL_SAI_Init(&haudio_out_sai) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1)
    /* Register SAI TC, HT and Error callbacks */
    else if (HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_TX_COMPLETE_CB_ID, SAI_TxCpltCallback) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_TX_HALFCOMPLETE_CB_ID, SAI_TxHalfCpltCallback) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_ERROR_CB_ID, SAI_ErrorCallback) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1) */
    else
    {
      /* Nothing to do */
    }
  }
  return status;
}

/**
  * @brief  Get bits per sample for the audio out stream.
  * @param  Instance Audio out instance.
  * @param  BitsPerSample Pointer to bits per sample of the audio out stream.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_GetBitsPerSample(uint32_t Instance, uint32_t *BitsPerSample)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  /* Check audio out state */
  else if (Audio_Out_Ctx[Instance].State == AUDIO_OUT_STATE_RESET)
  {
    status = BSP_ERROR_BUSY;
  }
  /* Get the current bits per sample of audio out stream */
  else
  {
    *BitsPerSample = Audio_Out_Ctx[Instance].BitsPerSample;
  }
  return status;
}

/**
  * @brief  Set channels number for the audio out stream.
  * @param  Instance Audio out instance.
  * @param  ChannelNbr Channels number of the audio out stream.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_SetChannelsNbr(uint32_t Instance, uint32_t ChannelNbr)
{
  int32_t status = BSP_ERROR_NONE;

  if ((Instance >= AUDIO_OUT_INSTANCES_NBR) || ((ChannelNbr != 1U) && (ChannelNbr != 2U)))
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  /* Check audio out state */
  else if (Audio_Out_Ctx[Instance].State != AUDIO_OUT_STATE_STOP)
  {
    status = BSP_ERROR_BUSY;
  }
  else
  {
    /* Update mono or stereo mode of SAI handle */
    haudio_out_sai.Init.MonoStereoMode = (ChannelNbr == 1U) ? SAI_MONOMODE : SAI_STEREOMODE;

    /* Re-initialize SAI1 with new parameter */
    if (HAL_SAI_Init(&haudio_out_sai) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1)
    /* Register SAI TC, HT and Error callbacks */
    else if (HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_TX_COMPLETE_CB_ID, SAI_TxCpltCallback) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_TX_HALFCOMPLETE_CB_ID, SAI_TxHalfCpltCallback) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_ERROR_CB_ID, SAI_ErrorCallback) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1) */
    else
    {
      /* Store new channels number on audio out context */
      Audio_Out_Ctx[Instance].ChannelsNbr = ChannelNbr;
    }
  }
  return status;
}

/**
  * @brief  Get channels number for the audio out stream.
  * @param  Instance Audio out instance.
  * @param  ChannelNbr Pointer to channels number of the audio out stream.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_GetChannelsNbr(uint32_t Instance, uint32_t *ChannelNbr)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  /* Check audio out state */
  else if (Audio_Out_Ctx[Instance].State == AUDIO_OUT_STATE_RESET)
  {
    status = BSP_ERROR_BUSY;
  }
  /* Get the current channels number of audio out stream */
  else
  {
    *ChannelNbr = Audio_Out_Ctx[Instance].ChannelsNbr;
  }
  return status;
}

/**
  * @brief  Get current state for the audio out stream.
  * @param  Instance Audio out instance.
  * @param  State Pointer to state of the audio out stream.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_GetState(uint32_t Instance, uint32_t *State)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  /* Get the current state of audio out stream */
  else
  {
    *State = Audio_Out_Ctx[Instance].State;
  }
  return status;
}

/**
  * @brief  BSP AUDIO OUT interrupt handler.
  * @param  Instance Audio out instance.
  * @param  Device Device of the audio out stream.
  * @retval None.
  */
void BSP_AUDIO_OUT_IRQHandler(uint32_t Instance, uint32_t Device)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);
  UNUSED(Device);

  HAL_DMA_IRQHandler(haudio_out_sai.hdmatx);
}

#if (USE_HAL_SAI_REGISTER_CALLBACKS == 0)  || !defined (USE_HAL_SAI_REGISTER_CALLBACKS)
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
     should be coded by user (its prototype is already declared in stm32h7b3i_eval_audio.h) */
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
     should be coded by user (its prototype is already declared in stm32h7b3i_eval_audio.h) */
  BSP_AUDIO_OUT_HalfTransfer_CallBack(0);
}

/**
  * @brief  SAI error callbacks.
  * @param  hsai  SAI handle
  * @retval None
  */
void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
  if(hsai->Instance == AUDIO_OUT_SAI)
  {
    BSP_AUDIO_OUT_Error_CallBack(0);
  }
  else
  {
    BSP_AUDIO_IN_Error_CallBack(0);
  }
}
#endif /* SAI Register Callbacks */

#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register default BSP AUDIO OUT msp callbacks.
  * @param  Instance AUDIO OUT Instance.
  * @retval BSP status.
  */
int32_t BSP_AUDIO_OUT_RegisterDefaultMspCallbacks(uint32_t Instance)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Register MspInit/MspDeInit callbacks */
    if (HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_MSPINIT_CB_ID, SAI_MspInit) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_MSPDEINIT_CB_ID, SAI_MspDeInit) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      Audio_Out_Ctx[Instance].IsMspCallbacksValid = 1U;
    }
  }
  /* Return BSP status */
  return status;
}

/**
  * @brief  Register BSP AUDIO OUT msp callbacks.
  * @param  Instance AUDIO OUT Instance.
  * @param  CallBacks Pointer to MspInit/MspDeInit callback functions.
  * @retval BSP status
  */
int32_t BSP_AUDIO_OUT_RegisterMspCallbacks(uint32_t Instance, BSP_AUDIO_OUT_Cb_t *CallBacks)
{
  int32_t status = BSP_ERROR_NONE;

  if (Instance >= AUDIO_OUT_INSTANCES_NBR)
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Register MspInit/MspDeInit callbacks */
    if (HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_MSPINIT_CB_ID, CallBacks->pMspSaiInitCb) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_SAI_RegisterCallback(&haudio_out_sai, HAL_SAI_MSPDEINIT_CB_ID, CallBacks->pMspSaiDeInitCb) != HAL_OK)
    {
      status = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      Audio_Out_Ctx[Instance].IsMspCallbacksValid = 1U;
    }
  }
  /* Return BSP status */
  return status;
}
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1) */

/**
  * @brief  Manage the BSP audio out transfer complete event.
  * @param  Instance Audio out instance.
  * @retval None.
  */
__weak void BSP_AUDIO_OUT_TransferComplete_CallBack(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);
}

/**
  * @brief  Manage the BSP audio out half transfer complete event.
  * @param  Instance Audio out instance.
  * @retval None.
  */
__weak void BSP_AUDIO_OUT_HalfTransfer_CallBack(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);
}

/**
  * @brief  Manages the BSP audio out error event.
  * @param  Instance Audio out instance.
  * @retval None.
  */
__weak void BSP_AUDIO_OUT_Error_CallBack(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);
}

/**
  * @brief  SAI1 clock Config.
  * @param  hsai SAI handle.
  * @param  SampleRate Audio sample rate used to play the audio stream.
  * @note   The SAI PLL configuration done within this function assumes that
  *         the SAI PLL input is HSI clock and that HSI is already enabled at 64 MHz.
  * @retval HAL status.
  */
__weak HAL_StatusTypeDef MX_SAI1_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t SampleRate)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsai);

  HAL_StatusTypeDef ret = HAL_OK;
  RCC_PeriphCLKInitTypeDef rcc_ex_clk_init_struct;

  rcc_ex_clk_init_struct.PLL3.PLL3Source = RCC_PLL3_SOURCE_HSI;
  rcc_ex_clk_init_struct.PLL3.PLL3RGE = RCC_PLL3_VCIRANGE_2;
  rcc_ex_clk_init_struct.PLL3.PLL3FRACN = 0;
  rcc_ex_clk_init_struct.PLL3.PLL3ClockOut = RCC_PLL3_DIVP;
  rcc_ex_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
  rcc_ex_clk_init_struct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL3P;
  rcc_ex_clk_init_struct.PLL3.PLL3Q = 28;
  rcc_ex_clk_init_struct.PLL3.PLL3R = 28;
  rcc_ex_clk_init_struct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;

  if ((SampleRate == AUDIO_FREQUENCY_11K) || (SampleRate == AUDIO_FREQUENCY_22K) || (SampleRate == AUDIO_FREQUENCY_44K))
  {
    /* SAI clock config:
    PLL3_VCO Input = HSI/PLL3M = 2 Mhz
    PLL3_VCO Output = PLL3_VCO Input * PLL3N = 192 Mhz
    SAI_CLK_x = PLL2_VCO Output/PLL3P = 192/17 = 11.294 Mhz */
    rcc_ex_clk_init_struct.PLL3.PLL3M = 32;
    rcc_ex_clk_init_struct.PLL3.PLL3N = 96;
    rcc_ex_clk_init_struct.PLL3.PLL3P = 17;
  }
  else  /* AUDIO_FREQUENCY_8K, AUDIO_FREQUENCY_16K, AUDIO_FREQUENCY_32K, AUDIO_FREQUENCY_48K, AUDIO_FREQUENCY_96K */
  {
    /* SAI clock config:
    PLL3_VCO Input = HSI/PLL3M = 2 Mhz
    PLL3_VCO Output = PLL3_VCO Input * PLL2N = 344 Mhz
    SAI_CLK_x = PLL3_VCO Output/PLL3P = 344/7 = 49.142 Mhz */
    rcc_ex_clk_init_struct.PLL3.PLL3M = 32;
    rcc_ex_clk_init_struct.PLL3.PLL3N = 172;
    rcc_ex_clk_init_struct.PLL3.PLL3P = 7;
  }

  if (HAL_RCCEx_PeriphCLKConfig(&rcc_ex_clk_init_struct) != HAL_OK)
  {
    ret = HAL_ERROR;
  }
  __HAL_RCC_SAI1_CLK_ENABLE();

  return ret;
}

/**
  * @brief  SAI2 clock Config.
  * @param  hsai SAI handle.
  * @param  SampleRate Audio sample rate used to play the audio stream.
  * @note   The SAI PLL configuration done within this function assumes that
  *         the SAI PLL input is HSI clock and that HSI is already enabled at 64 MHz.
  * @retval HAL status.
  */
__weak HAL_StatusTypeDef MX_SAI2_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t SampleRate)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hsai);

  HAL_StatusTypeDef ret = HAL_OK;
  RCC_PeriphCLKInitTypeDef rcc_ex_clk_init_struct;

  rcc_ex_clk_init_struct.PLL2.PLL2Source = RCC_PLL2_SOURCE_HSI;
  rcc_ex_clk_init_struct.PLL2.PLL2RGE = RCC_PLL2_VCIRANGE_2;
  rcc_ex_clk_init_struct.PLL2.PLL2FRACN = 0;
  rcc_ex_clk_init_struct.PLL2.PLL2ClockOut = RCC_PLL2_DIVP;
  rcc_ex_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_SAI2;
  rcc_ex_clk_init_struct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLL2P;
  rcc_ex_clk_init_struct.PLL2.PLL2Q = 28;
  rcc_ex_clk_init_struct.PLL2.PLL2R = 28;
  rcc_ex_clk_init_struct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;

  if ((SampleRate == AUDIO_FREQUENCY_11K) || (SampleRate == AUDIO_FREQUENCY_22K) || (SampleRate == AUDIO_FREQUENCY_44K))
  {
    /* SAI clock config:
    PLL2_VCO Input = HSI/PLL2M = 2 Mhz
    PLL2_VCO Output = PLL2_VCO Input * PLL2N = 192 Mhz
    SAI_CLK_x = PLL2_VCO Output/PLL2P = 192/17 = 11.294 Mhz */
    rcc_ex_clk_init_struct.PLL2.PLL2M = 32;
    rcc_ex_clk_init_struct.PLL2.PLL2N = 96;
    rcc_ex_clk_init_struct.PLL2.PLL2P = 17;
  }
  else  /* AUDIO_FREQUENCY_8K, AUDIO_FREQUENCY_16K, AUDIO_FREQUENCY_32K, AUDIO_FREQUENCY_48K, AUDIO_FREQUENCY_96K */
  {
    /* SAI clock config:
    PLL2_VCO Input = HSI/PLL2M = 2 Mhz
    PLL2_VCO Output = PLL2_VCO Input * PLL2N = 344 Mhz
    SAI_CLK_x = PLL2_VCO Output/PLL2P = 344/7 = 49.142 Mhz */
    rcc_ex_clk_init_struct.PLL2.PLL2M = 32;
    rcc_ex_clk_init_struct.PLL2.PLL2N = 172;
    rcc_ex_clk_init_struct.PLL2.PLL2P = 7;
  }

  if (HAL_RCCEx_PeriphCLKConfig(&rcc_ex_clk_init_struct) != HAL_OK)
  {
    ret = HAL_ERROR;
  }
  __HAL_RCC_SAI2_CLK_ENABLE();

  return ret;
}

/**
  * @brief  Initialize SAI2.
  * @param  hsai SAI handle.
  * @param  MXInit SAI configuration structure.
  * @retval HAL status.
  */
__weak HAL_StatusTypeDef MX_SAI2_Init(SAI_HandleTypeDef *hsai, MX_SAI_Config_t *MXInit)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (hsai->Instance == SAI2_Block_A)
  {
    /* Disable SAI peripheral to allow access to SAI internal registers */
    __HAL_SAI_DISABLE(hsai);

    /* Configure SAI2_Block_A */
    hsai->Init.MonoStereoMode       = MXInit->MonoStereoMode;
    hsai->Init.AudioFrequency       = MXInit->AudioFrequency;
    hsai->Init.AudioMode            = MXInit->AudioMode;
    hsai->Init.NoDivider            = SAI_MASTERDIVIDER_ENABLE;
    hsai->Init.Protocol             = SAI_FREE_PROTOCOL;
    hsai->Init.DataSize             = MXInit->DataSize;
    hsai->Init.FirstBit             = SAI_FIRSTBIT_MSB;
    hsai->Init.ClockStrobing        = MXInit->ClockStrobing;
    hsai->Init.Synchro              = MXInit->Synchro;
    hsai->Init.OutputDrive          = MXInit->OutputDrive;
    hsai->Init.FIFOThreshold        = SAI_FIFOTHRESHOLD_1QF;
    hsai->Init.SynchroExt           = MXInit->SynchroExt;
    hsai->Init.CompandingMode       = SAI_NOCOMPANDING;
    hsai->Init.TriState             = SAI_OUTPUT_NOTRELEASED;
    hsai->Init.Mckdiv               = 0U;
    hsai->Init.MckOutput            = SAI_MCK_OUTPUT_ENABLE;
    hsai->Init.MckOverSampling      = SAI_MCK_OVERSAMPLING_DISABLE;
    hsai->Init.PdmInit.Activation   = DISABLE;

    /* Configure SAI2_Block_A Frame */
    hsai->FrameInit.FrameLength       = MXInit->FrameLength;
    hsai->FrameInit.ActiveFrameLength = MXInit->ActiveFrameLength;
    hsai->FrameInit.FSDefinition      = SAI_FS_CHANNEL_IDENTIFICATION;
    hsai->FrameInit.FSPolarity        = SAI_FS_ACTIVE_LOW;
    hsai->FrameInit.FSOffset          = SAI_FS_BEFOREFIRSTBIT;

    /* Configure SAI2_Block_A Slot */
    hsai->SlotInit.FirstBitOffset     = 0;
    if (MXInit->DataSize == AUDIO_RESOLUTION_24B)
    {
      hsai->SlotInit.SlotSize         = SAI_SLOTSIZE_32B;
    }
    else
    {
      hsai->SlotInit.SlotSize         = SAI_SLOTSIZE_16B;
    }
    hsai->SlotInit.SlotNumber         = 2;
    hsai->SlotInit.SlotActive         = MXInit->SlotActive;

    if (HAL_SAI_Init(hsai) != HAL_OK)
    {
      status = HAL_ERROR;
    }
  }
  else /* (hsai->Instance == SAI2_Block_B) */
  {
    __HAL_SAI_DISABLE(hsai);

    /*****************************/
    /* SAI block used for record */
    /*****************************/
    /* Configure SAI2_Block_B used for receive */
    hsai->Init.AudioMode            = SAI_MODESLAVE_RX;
    hsai->Init.Synchro              = SAI_SYNCHRONOUS;
    hsai->Init.SynchroExt           = SAI_SYNCEXT_DISABLE;
    hsai->Init.OutputDrive          = SAI_OUTPUTDRIVE_ENABLE;
    hsai->Init.NoDivider            = SAI_MASTERDIVIDER_ENABLE;
    hsai->Init.FIFOThreshold        = SAI_FIFOTHRESHOLD_1QF;
    hsai->Init.AudioFrequency       = SAI_AUDIO_FREQUENCY_MCKDIV;
    hsai->Init.Mckdiv               = 0U;
    hsai->Init.MonoStereoMode       = SAI_MONOMODE;
    hsai->Init.CompandingMode       = SAI_NOCOMPANDING;
    hsai->Init.TriState             = SAI_OUTPUT_NOTRELEASED;
    hsai->Init.Protocol             = SAI_FREE_PROTOCOL;
    hsai->Init.DataSize             = MXInit->DataSize;
    hsai->Init.FirstBit             = SAI_FIRSTBIT_MSB;
    hsai->Init.ClockStrobing        = SAI_CLOCKSTROBING_FALLINGEDGE;
    hsai->Init.PdmInit.Activation   = DISABLE;

    /* Configure SAI2_Block_B Frame
    Frame Length: 64 or 32
    Frame active Length: 32 or 16
    FS Definition: Start frame + Channel Side identification
    FS Polarity: FS active Low
    FS Offset: FS asserted one bit before the first bit of slot 0 */
    hsai->FrameInit.FrameLength = MXInit->FrameLength;
    hsai->FrameInit.ActiveFrameLength = MXInit->ActiveFrameLength;
    hsai->FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
    hsai->FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
    hsai->FrameInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;

    /* Configure SAI SAI2_Block_B Slot
    Slot First Bit Offset: 0
    Slot Size  : 16
    Slot Number: 2
    Slot Active: Slots 0 and 1 actives */
    hsai->SlotInit.FirstBitOffset = 0;
    hsai->SlotInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
    hsai->SlotInit.SlotNumber = 2;
    hsai->SlotInit.SlotActive = SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1;

    /**********************************/
    /* Initializes the SAI peripheral */
    /**********************************/
    if (HAL_SAI_Init(hsai) != HAL_OK)
    {
      status = HAL_ERROR;
    }
  }
  return status;
}
/**
  * @}
  */

/** @defgroup STM32H573I_DK_AUDIO_OUT_Private_Functions AUDIO OUT Private Functions
  * @{
  */
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1)
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
     should be coded by user (its prototype is already declared in stm32h7b3i_eval_audio.h) */
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
     should be coded by user (its prototype is already declared in stm32h7b3i_eval_audio.h) */
  BSP_AUDIO_OUT_HalfTransfer_CallBack(0);
}

/**
  * @brief  SAI error callbacks.
  * @param  hsai  SAI handle
  * @retval None
  */
static void SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
  if(hsai->Instance == AUDIO_OUT_SAI)
  {
    BSP_AUDIO_OUT_Error_CallBack(0);
  }
  else
  {
    BSP_AUDIO_IN_Error_CallBack(0);
  }
}
#endif /* SAI Register callbacks */
/**
  * @}
  */

/** @addtogroup STM32H573I_DK_AUDIO_IN_Exported_Functions
  * @{
  */

/**
  * @brief  Initialize wave recording.
  * @param  Instance  Audio IN instance: 0 for SAI, 1 for SAI PDM
  * @param  AudioInit Init structure
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_Init(uint32_t Instance, BSP_AUDIO_Init_t* AudioInit)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
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

    /* Un-reset audio codec if not currently used by audio out instances */
    if ((Audio_Out_Ctx[0].State == AUDIO_OUT_STATE_RESET) && (Instance == 0U))
    {
      (void)CS42L51_PowerUp();

      /* Initialize the codec internal registers */
      if(CS42L51_Probe() != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }

    if (ret == BSP_ERROR_NONE)
    {
      if(Instance == 0U)
      {
        if(Audio_In_Ctx[Instance].Device != AUDIO_IN_DEVICE_ANALOG_MIC)
        {
          ret = BSP_ERROR_WRONG_PARAM;
        }
        /* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */
        else if(MX_SAI2_ClockConfig(&haudio_in_sai, AudioInit->SampleRate) != HAL_OK)
        {
          ret = BSP_ERROR_CLOCK_FAILURE;
        }
        else
        {
          haudio_in_sai.Instance    = AUDIO_IN_SAI;
          haudio_out_sai.Instance   = AUDIO_OUT_SAI;
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 0)
          SAI_MspInit(&haudio_in_sai);
#else
          /* Register the default SAI MSP callbacks */
          if(Audio_In_Ctx[Instance].IsMspCallbacksValid == 0U)
          {
            if(BSP_AUDIO_IN_RegisterDefaultMspCallbacks(Instance) != BSP_ERROR_NONE)
            {
              ret = BSP_ERROR_PERIPH_FAILURE;
            }
          }
          if (ret == BSP_ERROR_NONE)
          {
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 0) */
            MX_SAI_Config_t mx_config;

            /* Prepare haudio_in_sai handle */
            mx_config.AudioFrequency        = Audio_In_Ctx[Instance].SampleRate;
            mx_config.AudioMode             = SAI_MODESLAVE_RX;
            mx_config.ClockStrobing         = SAI_CLOCKSTROBING_FALLINGEDGE;
            mx_config.MonoStereoMode        = (AudioInit->ChannelsNbr == 1U) ? SAI_MONOMODE : SAI_STEREOMODE;
            mx_config.DataSize              = SAI_DATASIZE_16;
            mx_config.FrameLength           = 32;
            mx_config.ActiveFrameLength     = 16;
            mx_config.OutputDrive           = SAI_OUTPUTDRIVE_ENABLE;
            mx_config.Synchro               = SAI_SYNCHRONOUS;
            mx_config.SynchroExt            = SAI_SYNCEXT_DISABLE;
            mx_config.SlotActive            = SAI_SLOTACTIVE_0;

            if(MX_SAI2_Init(&haudio_in_sai, &mx_config) != HAL_OK)
            {
              /* Return BSP_ERROR_PERIPH_FAILURE when operations are not correctly done */
              ret = BSP_ERROR_PERIPH_FAILURE;
            }
            else
            {
              /* Prepare haudio_out_sai handle */
              mx_config.AudioMode         = SAI_MODEMASTER_TX;
              mx_config.Synchro           = SAI_ASYNCHRONOUS;
              mx_config.SlotActive        = SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1;

              if(MX_SAI2_Init(&haudio_out_sai, &mx_config) != HAL_OK)
              {
                /* Return BSP_ERROR_PERIPH_FAILURE when operations are not correctly done */
                ret = BSP_ERROR_PERIPH_FAILURE;
              }
            }
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1)
            if (ret == BSP_ERROR_NONE)
            {
              /* Register SAI TC, HT and Error callbacks */
              if(HAL_SAI_RegisterCallback(&haudio_in_sai, HAL_SAI_RX_COMPLETE_CB_ID, SAI_RxCpltCallback) != HAL_OK)
              {
                ret = BSP_ERROR_PERIPH_FAILURE;
              }
              else if(HAL_SAI_RegisterCallback(&haudio_in_sai, HAL_SAI_RX_HALFCOMPLETE_CB_ID, SAI_RxHalfCpltCallback) != HAL_OK)
              {
                ret = BSP_ERROR_PERIPH_FAILURE;
              }
              else
              {
                if(HAL_SAI_RegisterCallback(&haudio_in_sai, HAL_SAI_ERROR_CB_ID, SAI_ErrorCallback) != HAL_OK)
                {
                  ret = BSP_ERROR_PERIPH_FAILURE;
                }
              }
            }
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1) */
#if (USE_AUDIO_CODEC_CS42L51 == 1)
            if (ret == BSP_ERROR_NONE)
            {
              CS42L51_Init_t codec_init;

              /* Fill codec_init structure */
              codec_init.OutputDevice = (Audio_Out_Ctx[0].State == AUDIO_OUT_STATE_RESET) ? CS42L51_OUT_NONE : CS42L51_OUT_HEADPHONE;
              codec_init.Frequency    = AudioInit->SampleRate;
              codec_init.Resolution   = CS42L51_RESOLUTION_16B; /* Not used */
              codec_init.Volume       = AudioInit->Volume;
              codec_init.InputDevice  = (AudioInit->Device == AUDIO_IN_DEVICE_ANALOG_MIC) ? CS42L51_IN_MIC1 : CS42L51_IN_NONE;

              /* Initialize the codec internal registers */
              if(Audio_Drv->Init(Audio_CompObj, &codec_init) < 0)
              {
                ret = BSP_ERROR_COMPONENT_FAILURE;
              }
              else
              {
                /* Update audio in context state */
                Audio_In_Ctx[Instance].State = AUDIO_IN_STATE_STOP;
              }
            }
#endif  /*USE_AUDIO_CODEC_CS42L51 == 1)*/
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1)
          }
#endif
        }
      }
      else /* (Instance == 1U) */
      {
        /* SAI1 gets its clock from SAI1_Block_A
        PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */
        if(MX_SAI1_ClockConfig(&haudio_in_sai, AudioInit->SampleRate) != HAL_OK)
        {
          ret = BSP_ERROR_CLOCK_FAILURE;
        }
        else
        {
          haudio_in_sai.Instance = AUDIO_IN_SAI_PDM;
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
          MX_SAI_Config_t mx_config;

          /* Prepare haudio_in_sai handle */
          mx_config.MonoStereoMode    = SAI_STEREOMODE;
          mx_config.DataSize          = SAI_DATASIZE_8;
          mx_config.FrameLength       = 16;
          mx_config.ActiveFrameLength = 1;
          mx_config.OutputDrive       = SAI_OUTPUTDRIVE_DISABLE;
          mx_config.SlotActive        = (AudioInit->ChannelsNbr == 1U) ? SAI_SLOTACTIVE_0 : SAI_SLOTACTIVE_ALL;
          mx_config.AudioFrequency    = AudioInit->SampleRate;
          mx_config.AudioMode         = SAI_MODEMASTER_RX;
          mx_config.ClockStrobing     = SAI_CLOCKSTROBING_FALLINGEDGE;
          mx_config.Synchro           = SAI_ASYNCHRONOUS;
          mx_config.SynchroExt        = SAI_SYNCEXT_DISABLE;

          if(MX_SAI1_Init(&haudio_in_sai, &mx_config) != HAL_OK)
          {
            /* Return BSP_ERROR_PERIPH_FAILURE when operations are not correctly done */
            ret = BSP_ERROR_PERIPH_FAILURE;
          }
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1U)
          /* Register SAI TC, HT and Error callbacks */
          else if(HAL_SAI_RegisterCallback(&haudio_in_sai, HAL_SAI_RX_COMPLETE_CB_ID, SAI_RxCpltCallback) != HAL_OK)
          {
            ret = BSP_ERROR_PERIPH_FAILURE;
          }
          else if(HAL_SAI_RegisterCallback(&haudio_in_sai, HAL_SAI_RX_HALFCOMPLETE_CB_ID, SAI_RxHalfCpltCallback) != HAL_OK)
          {
            ret = BSP_ERROR_PERIPH_FAILURE;
          }
          else
          {
            if(HAL_SAI_RegisterCallback(&haudio_in_sai, HAL_SAI_ERROR_CB_ID, SAI_ErrorCallback) != HAL_OK)
            {
              ret = BSP_ERROR_PERIPH_FAILURE;
            }
          }
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1U) */
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
  * @brief  Deinit the audio IN peripherals.
  * @param  Instance  AUDIO IN Instance. It can be 0 when SAI is used or 1 if SAI PDM is used
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_DeInit(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

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
      haudio_in_sai.Instance = AUDIO_IN_SAI;
      /* Initialize the haudio_out_sai Instance parameter */
      haudio_out_sai.Instance = AUDIO_OUT_SAI;

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
    else /* if(Instance == 1U) */
    {
#if (USE_HAL_SAI_REGISTER_CALLBACKS == 0U)
      SAI_MspDeInit(&haudio_in_sai);
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 0U) */

      /* Initialize the haudio_in_sai Instance parameter */
      haudio_in_sai.Instance = AUDIO_IN_SAI_PDM;
      /* Initialize the haudio_out_sai Instance parameter */
      haudio_out_sai.Instance = AUDIO_OUT_SAI;

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
  * @brief  Start audio recording.
  * @param  Instance  AUDIO IN Instance. It can be 0 (SAI is used)
  * @param  pBuf     Main buffer pointer for the recorded data storing
  * @param  NbrOfBytes  Size of the record buffer
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_Record(uint32_t Instance, uint8_t* pBuf, uint32_t NbrOfBytes)
{
  int32_t ret = BSP_ERROR_NONE;
  
  if(Instance != 0U)
  {
    ret = BSP_ERROR_WRONG_PARAM;
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
      else
      {
        /* Start the process receive DMA */
        if(HAL_SAI_Receive_DMA(&haudio_in_sai, (uint8_t*)pBuf, (uint16_t)(NbrOfBytes/(Audio_In_Ctx[Instance].BitsPerSample/8U))) != HAL_OK)
        {
          ret = BSP_ERROR_PERIPH_FAILURE;
        }
        /* Update BSP AUDIO IN state */
        Audio_In_Ctx[Instance].State = AUDIO_IN_STATE_RECORDING;
      }
    }
  }
  
  /* Return BSP status */
  return ret;
}

/**
  * @brief  Stop audio recording.
  * @param  Instance  AUDIO IN Instance. It can be 0 (SAI is used) or 1 (SAI PDM used)
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

    /* Update BSP AUDIO IN state */
    Audio_In_Ctx[Instance].State = AUDIO_IN_STATE_STOP;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Pause the audio file stream.
  * @param  Instance  AUDIO IN Instance. It can be 0 (SAI is used) or 1 (SAI PDM used)
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

    /* Update BSP AUDIO IN state */
    Audio_In_Ctx[Instance].State = AUDIO_IN_STATE_PAUSE;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Resume the audio file stream.
  * @param  Instance  AUDIO IN Instance. It can be 0 (SAI is used) or 1 (SAI PDM used)
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

    /* Update BSP AUDIO IN state */
    Audio_In_Ctx[Instance].State = AUDIO_IN_STATE_RECORDING;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Start audio recording.
  * @param  Instance  AUDIO IN SAI PDM Instance. It can be only 1
  * @param  pBuf     Main buffer pointer for the recorded data storing
  * @param  NbrOfBytes  Size of the record buffer
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_RecordPDM(uint32_t Instance, uint8_t* pBuf, uint32_t NbrOfBytes)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance != 1U)
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

    /* Update BSP AUDIO IN state */
    Audio_In_Ctx[Instance].State = AUDIO_IN_STATE_RECORDING;
  }

  /* Return BSP status */
  return ret;
}


/**
  * @brief  Set Audio In device
  * @param  Instance  AUDIO IN Instance. It can be 0 for SAI, 1 for SAI PDM
  * @param  Device    The audio input device to be used
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_SetDevice(uint32_t Instance, uint32_t Device)
{
  int32_t ret = BSP_ERROR_NONE;
  BSP_AUDIO_Init_t audio_init;

  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Audio_In_Ctx[Instance].State == AUDIO_IN_STATE_STOP)
  {
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
  * @param  Instance  AUDIO IN Instance. It can be 0 for SAI, 1 for SAI PDM
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
  * @param  Instance    Audio IN instanc. It can be 0 for SAI, 1 for SAI PDM
  * @param  SampleRate  Input frequency to be set
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_SetSampleRate(uint32_t Instance, uint32_t  SampleRate)
{
  int32_t ret = BSP_ERROR_NONE;
  BSP_AUDIO_Init_t audio_init;

  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Audio_In_Ctx[Instance].State == AUDIO_IN_STATE_STOP)
  {
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
  * @param  Instance  AUDIO IN Instance. It can be 0 for SAI, 1 for SAI PDM
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
  * @param  Instance  AUDIO IN Instance. It can be 0 for SAI, 1 for SAI PDM
  * @param  BitsPerSample  Input resolution to be set
  * @retval BSP status
  */
int32_t BSP_AUDIO_IN_SetBitsPerSample(uint32_t Instance, uint32_t BitsPerSample)
{
  int32_t ret = BSP_ERROR_NONE;
  BSP_AUDIO_Init_t audio_init;

  if(Instance >= AUDIO_IN_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Audio_In_Ctx[Instance].State == AUDIO_IN_STATE_STOP)
  {
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
  * @param  Instance  AUDIO IN Instance. It can be 0 for SAI, 1 for SAI PDM
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
  * @param  Instance  AUDIO IN Instance. It can be 0 for SAI, 1 for SAI PDM
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
  * @param  Instance  AUDIO IN Instance. It can be 0 for SAI, 1 for SAI PDM
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
  * @param  Instance  AUDIO IN Instance. It can be 0 for SAI, 1 for SAI PDM
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
  else
  {
    /* Input Volume to be returned */
    Audio_In_Ctx[Instance].Volume = Volume;
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Get the current audio in volume level.
  * @param  Instance  AUDIO IN Instance. It can be 0 for SAI, 1 for SAI PDM
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
  * @param  Instance  AUDIO IN Instance. It can be 0 for SAI, 1 for SAI PDM
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
  * @param  Instance Audio IN instance: It can be 0 for SAI, 1 for SAI PDM
  * @param  InputDevice Can be:
  *         - AUDIO_IN_DEVICE_ANALOG_MIC
  *         - AUDIO_IN_DEVICE_DIGITAL_MIC
  *         - AUDIO_IN_DEVICE_DIGITAL_MIC1
  * @retval None
  */
void BSP_AUDIO_IN_IRQHandler(uint32_t Instance, uint32_t InputDevice)
{
  if(((Instance == 0U) || (Instance == 1U)) &&\
    ((InputDevice == AUDIO_IN_DEVICE_DIGITAL_MIC) || (InputDevice == AUDIO_IN_DEVICE_ANALOG_MIC)))
    {
      HAL_DMA_IRQHandler(haudio_in_sai.hdmarx);
    }
}

/**
  * @brief  Initializes the Audio Codec audio in instance (SAI).
  * @param  hsai : SAI handle
  * @param  MXConfig SAI configuration structure
  * @note   Being __weak it can be overwritten by the application
  * @retval HAL status
  */
__weak HAL_StatusTypeDef MX_SAI1_Init(SAI_HandleTypeDef* hsai, MX_SAI_Config_t *MXConfig)
{
  HAL_StatusTypeDef ret = HAL_OK;

  /* Disable SAI peripheral to allow access to SAI internal registers */
  __HAL_SAI_DISABLE(hsai);

  /* Configure SAI1_Block_A */
  hsai->Init.AudioFrequency         = SAI_AUDIO_FREQUENCY_MCKDIV;
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
  hsai->Init.TriState               = SAI_OUTPUT_NOTRELEASED;
  /* Set Mckdiv according sample rate */
  if ((MXConfig->AudioFrequency == AUDIO_FREQUENCY_8K) || (MXConfig->AudioFrequency == AUDIO_FREQUENCY_16K) ||
      (MXConfig->AudioFrequency == AUDIO_FREQUENCY_32K))
  {
    /* Set PDM clock to 1024KHz so Mckdiv to 24 (Fsai/(Fpdm*2)) */
    hsai->Init.Mckdiv = 24U;
  }
  else if ((MXConfig->AudioFrequency == AUDIO_FREQUENCY_48K) || (MXConfig->AudioFrequency == AUDIO_FREQUENCY_96K) ||
           (MXConfig->AudioFrequency == AUDIO_FREQUENCY_192K))
  {
    /* Set PDM clock to 3072KHz so Mckdiv to 8 (Fsai/(Fpdm*2)) */
    hsai->Init.Mckdiv = 8U;
  }
  else if ((MXConfig->AudioFrequency == AUDIO_FREQUENCY_11K) || (MXConfig->AudioFrequency == AUDIO_FREQUENCY_22K) ||
           (MXConfig->AudioFrequency == AUDIO_FREQUENCY_44K))
  {
    /* Set PDM clock to 1411,2KHz so Mckdiv to 4 (Fsai/(Fpdm*2)) */
    hsai->Init.Mckdiv = 4U;
  }
  else /* AUDIO_FREQUENCY_88K or AUDIO_FREQUENCY_196K */
  {
    /* Set PDM clock to 2822,4KHz so Mckdiv to 2 (Fsai/(Fpdm*2)) */
    hsai->Init.Mckdiv = 2U;
  }
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
  hsai->SlotInit.SlotNumber         = 2;
  hsai->SlotInit.SlotActive         = MXConfig->SlotActive;

  if(HAL_SAI_Init(hsai) != HAL_OK)
  {
    ret = HAL_ERROR;
  }

  return ret;
}

#if (USE_HAL_SAI_REGISTER_CALLBACKS == 0) || !defined (USE_HAL_SAI_REGISTER_CALLBACKS)
/**
  * @brief  Half reception complete callback.
  * @param  hsai   SAI handle.
  * @retval None
  */
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Call the record update function to get the first half */
  if(hsai->Instance == AUDIO_IN_SAI)
  {
    BSP_AUDIO_IN_HalfTransfer_CallBack(0);
  }
  else
  {
    BSP_AUDIO_IN_HalfTransfer_CallBack(1);
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
  if(hsai->Instance == AUDIO_IN_SAI)
  {
    BSP_AUDIO_IN_TransferComplete_CallBack(0);
  }
  else
  {
    BSP_AUDIO_IN_TransferComplete_CallBack(1);
  }
}
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 0) || !defined (USE_HAL_SAI_REGISTER_CALLBACKS) */

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

/** @defgroup STM32H573I_DK_AUDIO_IN_Private_Functions AUDIO IN Private Functions
  * @{
  */

#if (USE_HAL_SAI_REGISTER_CALLBACKS == 1)
/**
  * @brief  Half reception complete callback.
  * @param  hsai   SAI handle.
  * @retval None
  */
static void SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Call the record update function to get the first half */
  if(hsai->Instance == AUDIO_IN_SAI)
  {
    BSP_AUDIO_IN_HalfTransfer_CallBack(0);
  }
  else
  {
    BSP_AUDIO_IN_HalfTransfer_CallBack(1);
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
  if(hsai->Instance == AUDIO_IN_SAI)
  {
    BSP_AUDIO_IN_TransferComplete_CallBack(0);
  }
  else
  {
    BSP_AUDIO_IN_TransferComplete_CallBack(1);
  }
}
#endif /* (USE_HAL_SAI_REGISTER_CALLBACKS == 1) */
/**
  * @}
  */

/** @defgroup STM32H573I_DK_AUDIO_Private_Functions AUDIO Private Functions
  * @{
  */

#if (USE_AUDIO_CODEC_CS42L51 == 1)
/**
  * @brief  Register Bus IOs if component ID is OK
  * @retval error status
  */
static int32_t CS42L51_Probe(void)
{
  int32_t                   ret = BSP_ERROR_NONE;
  CS42L51_IO_t              IOCtx;
  static CS42L51_Object_t   CS42L51Obj;
  uint32_t                  cs42l51_id;

  /* Configure the audio driver */
  IOCtx.Address     = AUDIO_I2C_ADDRESS;
  IOCtx.Init        = BSP_I2C4_Init;
  IOCtx.DeInit      = BSP_I2C4_DeInit;
  IOCtx.ReadReg     = BSP_I2C4_ReadReg;
  IOCtx.WriteReg    = BSP_I2C4_WriteReg;
  IOCtx.GetTick     = BSP_GetTick;

  if(CS42L51_RegisterBusIO (&CS42L51Obj, &IOCtx) != CS42L51_OK)
  {
    ret = BSP_ERROR_BUS_FAILURE;
  }
  else if(CS42L51_ReadID(&CS42L51Obj, &cs42l51_id) != CS42L51_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else if((cs42l51_id & CS42L51_ID_MASK) != CS42L51_ID)
  {
    ret = BSP_ERROR_UNKNOWN_COMPONENT;
  }
  else
  {
    Audio_Drv = (AUDIO_Drv_t *) &CS42L51_Driver;
    Audio_CompObj = &CS42L51Obj;
  }

  return ret;
}

/**
  * @brief  Un-reset CS42L51.
  * @retval BSP status.
  */
static int32_t CS42L51_PowerUp(void)
{
  GPIO_InitTypeDef  gpio_init_structure;

  AUDIO_NRST_ENABLE();

  /* Configure the CS42L51 reset pin */
  gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_init_structure.Pull = GPIO_PULLDOWN;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio_init_structure.Pin = AUDIO_NRST_PIN;
  HAL_GPIO_Init(AUDIO_NRST_GPIO_PORT, &gpio_init_structure);
  HAL_GPIO_WritePin(AUDIO_NRST_GPIO_PORT, AUDIO_NRST_PIN, GPIO_PIN_SET);

  /* Wait 1ms according CS42L51 datasheet */
  HAL_Delay(1);

  return BSP_ERROR_NONE;
}

/**
  * @brief  Reset CS42L51.
  * @retval BSP status.
  */
static int32_t CS42L51_PowerDown(void)
{
  int32_t ret = BSP_ERROR_NONE;
  GPIO_InitTypeDef  gpio_init_structure;

  gpio_init_structure.Pin = AUDIO_NRST_PIN;
  HAL_GPIO_DeInit(AUDIO_NRST_GPIO_PORT, gpio_init_structure.Pin);

  return ret;
}
#endif /* USE_AUDIO_CODEC_CS42L51 */

/**
  * @brief  Initialize BSP_AUDIO_OUT MSP.
  * @param  hsai  SAI handle
  * @retval None
  */
static void SAI_MspInit(SAI_HandleTypeDef *hsai)
{
  GPIO_InitTypeDef  gpio_init_structure;
  static DMA_NodeTypeDef TxNode, RxNode;
  static DMA_NodeConfTypeDef dmaNodeConfig;

    /* Enable SAI clock */
  AUDIO_OUT_SAI_CLK_ENABLE();

  /* Enable GPIO clock */
  AUDIO_OUT_SAI_MCLK_ENABLE();
  AUDIO_OUT_SAI_SCK_ENABLE();
  AUDIO_OUT_SAI_SD_ENABLE();
  AUDIO_OUT_SAI_FS_ENABLE();
  /* CODEC_SAI pins configuration: FS, SCK, MCK and SD pins ------------------*/
  gpio_init_structure.Pin = AUDIO_OUT_SAI_FS_PIN;
  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = AUDIO_OUT_SAI_FS_AF;
  HAL_GPIO_Init(AUDIO_OUT_SAI_FS_GPIO_PORT, &gpio_init_structure);

  gpio_init_structure.Pin = AUDIO_OUT_SAI_SCK_PIN;
  gpio_init_structure.Alternate = AUDIO_OUT_SAI_SCK_AF;
  HAL_GPIO_Init(AUDIO_OUT_SAI_SCK_GPIO_PORT, &gpio_init_structure);

  gpio_init_structure.Pin =  AUDIO_OUT_SAI_SD_PIN;
  gpio_init_structure.Alternate = AUDIO_OUT_SAI_SD_AF;
  HAL_GPIO_Init(AUDIO_OUT_SAI_SD_GPIO_PORT, &gpio_init_structure);

  gpio_init_structure.Pin = AUDIO_OUT_SAI_MCLK_PIN;
  gpio_init_structure.Alternate = AUDIO_OUT_SAI_MCLK_AF;
  HAL_GPIO_Init(AUDIO_OUT_SAI_MCLK_GPIO_PORT, &gpio_init_structure);

  if(hsai->Instance == AUDIO_OUT_SAI)
  {
    /* Configure the hDmaSaiTx handle parameters */
    AUDIO_OUT_SAI_DMA_CLK_ENABLE();

    if (SAITxQueue.Head == NULL)
    {
      /* DMA for Tx */
      /* Set node type */
      dmaNodeConfig.NodeType                            = DMA_GPDMA_LINEAR_NODE;
      /* Set common node parameters */
      dmaNodeConfig.Init.Request                        = AUDIO_OUT_SAI_DMA_REQUEST;
      dmaNodeConfig.Init.BlkHWRequest                   = DMA_BREQ_SINGLE_BURST;
      dmaNodeConfig.Init.Direction                      = DMA_MEMORY_TO_PERIPH;
      dmaNodeConfig.Init.SrcInc                         = DMA_SINC_INCREMENTED;
      dmaNodeConfig.Init.DestInc                        = DMA_DINC_FIXED;
      if (Audio_Out_Ctx[0].BitsPerSample == AUDIO_RESOLUTION_16B)
      {
        dmaNodeConfig.Init.SrcDataWidth                 = DMA_SRC_DATAWIDTH_HALFWORD;
        dmaNodeConfig.Init.DestDataWidth                = DMA_DEST_DATAWIDTH_HALFWORD;
      }
      else /* AUDIO_RESOLUTION_24b */
      {
        dmaNodeConfig.Init.SrcDataWidth                 = DMA_SRC_DATAWIDTH_WORD;
        dmaNodeConfig.Init.DestDataWidth                = DMA_DEST_DATAWIDTH_WORD;
      }
      dmaNodeConfig.Init.SrcBurstLength                 = 1;
      dmaNodeConfig.Init.DestBurstLength                = 1;
      dmaNodeConfig.Init.Priority                       = DMA_HIGH_PRIORITY;
      dmaNodeConfig.Init.TransferEventMode              = DMA_TCEM_BLOCK_TRANSFER;
      dmaNodeConfig.Init.TransferAllocatedPort          = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT1;
      dmaNodeConfig.Init.Mode                           = DMA_NORMAL;
      /* Set node data handling parameters */
      dmaNodeConfig.DataHandlingConfig.DataExchange     = DMA_EXCHANGE_NONE;
      dmaNodeConfig.DataHandlingConfig.DataAlignment    = DMA_DATA_RIGHTALIGN_ZEROPADDED;
      /* Set node trigger parameters */
      dmaNodeConfig.TriggerConfig.TriggerPolarity       = DMA_TRIG_POLARITY_MASKED;

      /* Build NodeTx */
      if (HAL_DMAEx_List_BuildNode(&dmaNodeConfig, &TxNode) != HAL_OK)
      {
        BSP_AUDIO_IN_Error_CallBack(0);
      }

      /* Insert NodeTx to SAI queue */
      if (HAL_DMAEx_List_InsertNode_Tail(&SAITxQueue, &TxNode) != HAL_OK)
      {
        BSP_AUDIO_IN_Error_CallBack(0);
      }

      /* Set queue circular mode for sai queue */
      if (HAL_DMAEx_List_SetCircularMode(&SAITxQueue) != HAL_OK)
      {
        BSP_AUDIO_IN_Error_CallBack(0);
      }
    }

    /* DMA for Tx */
    hDmaSaiTx.Instance                         = AUDIO_OUT_SAI_DMA_CHANNEL;

    hDmaSaiTx.InitLinkedList.Priority          = DMA_HIGH_PRIORITY;
    hDmaSaiTx.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
    hDmaSaiTx.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT1;
    hDmaSaiTx.InitLinkedList.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
    hDmaSaiTx.InitLinkedList.LinkedListMode    = DMA_LINKEDLIST_CIRCULAR;

    /* DMA linked list init */
    if (HAL_DMAEx_List_Init(&hDmaSaiTx) != HAL_OK)
    {
      BSP_AUDIO_IN_Error_CallBack(0);
    }

    /* Link SAI queue to DMA channel */
    if (HAL_DMAEx_List_LinkQ(&hDmaSaiTx, &SAITxQueue) != HAL_OK)
    {
      BSP_AUDIO_IN_Error_CallBack(0);
    }

    /* Associate the DMA handle */
    __HAL_LINKDMA(hsai, hdmatx, hDmaSaiTx);

    /* SAI DMA IRQ Channel configuration */
    HAL_NVIC_SetPriority(AUDIO_OUT_SAI_DMA_IRQ, BSP_AUDIO_OUT_IT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(AUDIO_OUT_SAI_DMA_IRQ);

  }

  /* Audio In Msp initialization */
  if(hsai->Instance == AUDIO_IN_SAI)
  {
    /* Enable SAI clock */
    AUDIO_IN_SAI_CLK_ENABLE();

    /* Enable SD GPIO clock */
    AUDIO_IN_SAI_SD_ENABLE();
    /* CODEC_SAI pin configuration: SD pin */
    gpio_init_structure.Pin = AUDIO_IN_SAI_SD_PIN;
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio_init_structure.Alternate = AUDIO_IN_SAI_AF;
    HAL_GPIO_Init(AUDIO_IN_SAI_SD_GPIO_PORT, &gpio_init_structure);

    /* Enable the DMA clock */
    AUDIO_IN_SAI_DMA_CLK_ENABLE();

    if (SAIRxQueue.Head == NULL)
    {
      /* DMA for Rx */
      /* Set node type */
      dmaNodeConfig.NodeType                            = DMA_GPDMA_LINEAR_NODE;
      /* Set common node parameters */
      dmaNodeConfig.Init.Request                        = AUDIO_IN_SAI_DMA_REQUEST;
      dmaNodeConfig.Init.BlkHWRequest                   = DMA_BREQ_SINGLE_BURST;
      dmaNodeConfig.Init.Direction                      = DMA_PERIPH_TO_MEMORY;
      dmaNodeConfig.Init.SrcInc                         = DMA_SINC_FIXED;
      dmaNodeConfig.Init.DestInc                        = DMA_DINC_INCREMENTED;
      if (Audio_In_Ctx[0].BitsPerSample == AUDIO_RESOLUTION_16B)
      {
        dmaNodeConfig.Init.SrcDataWidth                 = DMA_SRC_DATAWIDTH_HALFWORD;
        dmaNodeConfig.Init.DestDataWidth                = DMA_DEST_DATAWIDTH_HALFWORD;
      }
      else /* AUDIO_RESOLUTION_24b */
      {
        dmaNodeConfig.Init.SrcDataWidth                 = DMA_SRC_DATAWIDTH_WORD;
        dmaNodeConfig.Init.DestDataWidth                = DMA_DEST_DATAWIDTH_WORD;
      }
      dmaNodeConfig.Init.SrcBurstLength                 = 1;
      dmaNodeConfig.Init.DestBurstLength                = 1;
      dmaNodeConfig.Init.Priority                       = DMA_HIGH_PRIORITY;
      dmaNodeConfig.Init.TransferEventMode              = DMA_TCEM_BLOCK_TRANSFER;
      dmaNodeConfig.Init.TransferAllocatedPort          = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT1;
      dmaNodeConfig.Init.Mode                           = DMA_NORMAL;
      /* Set node data handling parameters */
      dmaNodeConfig.DataHandlingConfig.DataExchange     = DMA_EXCHANGE_NONE;
      dmaNodeConfig.DataHandlingConfig.DataAlignment    = DMA_DATA_RIGHTALIGN_ZEROPADDED;
      /* Set node trigger parameters */
      dmaNodeConfig.TriggerConfig.TriggerPolarity       = DMA_TRIG_POLARITY_MASKED;

      /* Build NodeRx */
      if (HAL_DMAEx_List_BuildNode(&dmaNodeConfig, &RxNode) != HAL_OK)
      {
        BSP_AUDIO_IN_Error_CallBack(0);
      }

      /* Insert NodeTx to SAI queue */
      if (HAL_DMAEx_List_InsertNode_Tail(&SAIRxQueue, &RxNode) != HAL_OK)
      {
        BSP_AUDIO_IN_Error_CallBack(0);
      }

      /* Set queue circular mode for sai queue */
      if (HAL_DMAEx_List_SetCircularMode(&SAIRxQueue) != HAL_OK)
      {
        BSP_AUDIO_IN_Error_CallBack(0);
      }
    }

    /* DMA for Rx */
    hDmaSaiRx.Instance                         = AUDIO_IN_SAI_DMA_CHANNEL;

    hDmaSaiRx.InitLinkedList.Priority          = DMA_HIGH_PRIORITY;
    hDmaSaiRx.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
    hDmaSaiRx.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT1;
    hDmaSaiRx.InitLinkedList.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
    hDmaSaiRx.InitLinkedList.LinkedListMode    = DMA_LINKEDLIST_CIRCULAR;

    /* DMA linked list init */
    if (HAL_DMAEx_List_Init(&hDmaSaiRx) != HAL_OK)
    {
      BSP_AUDIO_IN_Error_CallBack(0);
    }

    /* Link SAI queue to DMA channel */
    if (HAL_DMAEx_List_LinkQ(&hDmaSaiRx, &SAIRxQueue) != HAL_OK)
    {
      BSP_AUDIO_IN_Error_CallBack(0);
    }

    /* Associate the DMA handle */
    __HAL_LINKDMA(hsai, hdmarx, hDmaSaiRx);

    /* SAI DMA IRQ Channel configuration */
    HAL_NVIC_SetPriority(AUDIO_IN_SAI_DMA_IRQ, BSP_AUDIO_IN_IT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(AUDIO_IN_SAI_DMA_IRQ);
  }

  if(hsai->Instance == AUDIO_IN_SAI_PDM)
  {
     /* Enable SAI clock */
    AUDIO_IN_SAI_PDM_CLK_ENABLE();

    AUDIO_IN_SAI_PDM_CLK_IN_ENABLE();
    AUDIO_IN_SAI_PDM_DATA_IN_ENABLE();

    gpio_init_structure.Pin = AUDIO_IN_SAI_PDM_CLK_IN_PIN;
    gpio_init_structure.Mode = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init_structure.Alternate = AUDIO_IN_SAI_PDM_DATA_CLK_AF;
    HAL_GPIO_Init(AUDIO_IN_SAI_PDM_CLK_IN_PORT, &gpio_init_structure);

    gpio_init_structure.Pin = AUDIO_IN_SAI_PDM_DATA_IN_PIN;
    HAL_GPIO_Init(AUDIO_IN_SAI_PDM_DATA_IN_PORT, &gpio_init_structure);

    /* Enable the DMA clock */
    AUDIO_IN_SAI_PDM_DMA_CLK_ENABLE();

    if (SAIRxQueue.Head == NULL)
    {
      /* DMA for Rx */
      /* Set node type */
      dmaNodeConfig.NodeType                            = DMA_GPDMA_LINEAR_NODE;
      /* Set common node parameters */
      dmaNodeConfig.Init.Request                        = AUDIO_IN_SAI_PDM_DMA_REQUEST;
      dmaNodeConfig.Init.BlkHWRequest                   = DMA_BREQ_SINGLE_BURST;
      dmaNodeConfig.Init.Direction                      = DMA_PERIPH_TO_MEMORY;
      dmaNodeConfig.Init.SrcInc                         = DMA_SINC_FIXED;
      dmaNodeConfig.Init.DestInc                        = DMA_DINC_INCREMENTED;
      dmaNodeConfig.Init.SrcDataWidth                   = DMA_SRC_DATAWIDTH_BYTE;
      dmaNodeConfig.Init.DestDataWidth                  = DMA_DEST_DATAWIDTH_BYTE;
      dmaNodeConfig.Init.SrcBurstLength                 = 1;
      dmaNodeConfig.Init.DestBurstLength                = 1;
      dmaNodeConfig.Init.Priority                       = DMA_HIGH_PRIORITY;
      dmaNodeConfig.Init.TransferEventMode              = DMA_TCEM_BLOCK_TRANSFER;
      dmaNodeConfig.Init.TransferAllocatedPort          = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT1;
      dmaNodeConfig.Init.Mode                           = DMA_NORMAL;
      /* Set node data handling parameters */
      dmaNodeConfig.DataHandlingConfig.DataExchange     = DMA_EXCHANGE_NONE;
      dmaNodeConfig.DataHandlingConfig.DataAlignment    = DMA_DATA_RIGHTALIGN_ZEROPADDED;
      /* Set node trigger parameters */
      dmaNodeConfig.TriggerConfig.TriggerPolarity       = DMA_TRIG_POLARITY_MASKED;

      /* Build NodeRx */
      if (HAL_DMAEx_List_BuildNode(&dmaNodeConfig, &RxNode) != HAL_OK)
      {
        BSP_AUDIO_IN_Error_CallBack(0);
      }

      /* Insert NodeTx to SAI queue */
      if (HAL_DMAEx_List_InsertNode_Tail(&SAIRxQueue, &RxNode) != HAL_OK)
      {
        BSP_AUDIO_IN_Error_CallBack(0);
      }

      /* Set queue circular mode for sai queue */
      if (HAL_DMAEx_List_SetCircularMode(&SAIRxQueue) != HAL_OK)
      {
        BSP_AUDIO_IN_Error_CallBack(0);
      }
    }

    /* DMA for Rx */
    hDmaSaiRx.Instance                         = AUDIO_IN_SAI_PDM_DMA_CHANNEL;

    hDmaSaiRx.InitLinkedList.Priority          = DMA_HIGH_PRIORITY;
    hDmaSaiRx.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
    hDmaSaiRx.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT1;
    hDmaSaiRx.InitLinkedList.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
    hDmaSaiRx.InitLinkedList.LinkedListMode    = DMA_LINKEDLIST_CIRCULAR;

    /* DMA linked list init */
    if (HAL_DMAEx_List_Init(&hDmaSaiRx) != HAL_OK)
    {
      BSP_AUDIO_IN_Error_CallBack(0);
    }

    /* Link SAI queue to DMA channel */
    if (HAL_DMAEx_List_LinkQ(&hDmaSaiRx, &SAIRxQueue) != HAL_OK)
    {
      BSP_AUDIO_IN_Error_CallBack(0);
    }

    /* Associate the DMA handle */
    __HAL_LINKDMA(hsai, hdmarx, hDmaSaiRx);

    /* SAI DMA IRQ Channel configuration */
    HAL_NVIC_SetPriority(AUDIO_IN_SAI_PDM_DMA_IRQ, BSP_AUDIO_IN_IT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(AUDIO_IN_SAI_PDM_DMA_IRQ);
  }
}

/**
  * @brief  Deinitializes SAI MSP.
  * @param  hsai  SAI handle
  * @retval HAL status
  */
static void SAI_MspDeInit(SAI_HandleTypeDef *hsai)
{
  if(hsai->Instance == AUDIO_OUT_SAI)
  {
    /* Disable SAI DMA Channel IRQ */
    HAL_NVIC_DisableIRQ(AUDIO_OUT_SAI_DMA_IRQ);

    /* Reset the DMA Channel configuration*/
    if (HAL_DMAEx_List_DeInit(&hDmaSaiTx) != HAL_OK)
    {
      BSP_AUDIO_IN_Error_CallBack(0);
    }

    /* Reset TxQueue */
    if (HAL_DMAEx_List_ResetQ(&SAITxQueue) != HAL_OK)
    {
      BSP_AUDIO_IN_Error_CallBack(0);
    }

    /* De-initialize FS, SCK, MCK and SD pins */
    HAL_GPIO_DeInit(AUDIO_OUT_SAI_FS_GPIO_PORT, AUDIO_OUT_SAI_FS_PIN);
    HAL_GPIO_DeInit(AUDIO_OUT_SAI_SCK_GPIO_PORT, AUDIO_OUT_SAI_SCK_PIN);
    HAL_GPIO_DeInit(AUDIO_OUT_SAI_SD_GPIO_PORT, AUDIO_OUT_SAI_SD_PIN);
    HAL_GPIO_DeInit(AUDIO_OUT_SAI_MCLK_GPIO_PORT, AUDIO_OUT_SAI_MCLK_PIN);
  }
  if(hsai->Instance == AUDIO_IN_SAI)
  {
    /* Disable SAI DMA Channel IRQ */
    HAL_NVIC_DisableIRQ(AUDIO_IN_SAI_DMA_IRQ);

    /* Reset the DMA Channel configuration*/
    if (HAL_DMAEx_List_DeInit(&hDmaSaiRx) != HAL_OK)
    {
      BSP_AUDIO_IN_Error_CallBack(0);
    }

    /* Reset RxQueue */
    if (HAL_DMAEx_List_ResetQ(&SAIRxQueue) != HAL_OK)
    {
      BSP_AUDIO_IN_Error_CallBack(0);
    }

    /* De-initialize SD pin */
    HAL_GPIO_DeInit(AUDIO_IN_SAI_SD_GPIO_PORT, AUDIO_IN_SAI_SD_PIN);

    /* Disable SAI clock */
    AUDIO_IN_SAI_CLK_DISABLE();
  }
  if(hsai->Instance == AUDIO_IN_SAI_PDM)
  {
    /* Disable SAI DMA Channel IRQ */
    HAL_NVIC_DisableIRQ(AUDIO_IN_SAI_PDM_DMA_IRQ);

    /* Reset the DMA Channel configuration*/
    if (HAL_DMAEx_List_DeInit(&hDmaSaiRx) != HAL_OK)
    {
      BSP_AUDIO_IN_Error_CallBack(0);
    }

    /* Reset RxQueue */
    if (HAL_DMAEx_List_ResetQ(&SAIRxQueue) != HAL_OK)
    {
      BSP_AUDIO_IN_Error_CallBack(0);
    }

    /* De-initialize SD pin */
    HAL_GPIO_DeInit(AUDIO_IN_SAI_PDM_DATA_IN_PORT, AUDIO_IN_SAI_PDM_DATA_IN_PIN);
    HAL_GPIO_DeInit(AUDIO_IN_SAI_PDM_CLK_IN_PORT, AUDIO_IN_SAI_PDM_CLK_IN_PIN);

    /* Disable SAI clock */
    AUDIO_IN_SAI_PDM_CLK_DISABLE();
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


