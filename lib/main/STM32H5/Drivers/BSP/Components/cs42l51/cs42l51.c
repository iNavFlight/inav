/**
  ******************************************************************************
  * @file    cs42l51.c
  * @author  MCD Application Team
  * @brief   This file provides the CS42L51 Audio Codec driver.
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

/* Includes ------------------------------------------------------------------*/
#include "cs42l51.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup cs42l51
  * @brief     This file provides a set of functions needed to drive the
  *            CS42L51 audio codec.
  * @{
  */

/** @defgroup CS42L51_Private_Types Private Types
  * @{
  */
/* Audio codec driver structure initialization */
CS42L51_Drv_t CS42L51_Driver =
{
  CS42L51_Init,
  CS42L51_DeInit,
  CS42L51_ReadID,
  CS42L51_Play,
  CS42L51_Pause,
  CS42L51_Resume,
  CS42L51_Stop,
  CS42L51_SetFrequency,
  CS42L51_GetFrequency,
  CS42L51_SetVolume,
  CS42L51_GetVolume,
  CS42L51_SetMute,
  CS42L51_SetOutputMode,
  CS42L51_SetResolution,
  CS42L51_GetResolution,
  CS42L51_SetProtocol,
  CS42L51_GetProtocol,
  CS42L51_Reset
};

/**
  * @}
  */

/** @defgroup CS42L51_Private_Variables Private Variables
  * @{
  */
static uint32_t CS42L51_CurrentDevices = CS42L51_OUT_NONE;

/**
  * @}
  */

/** @defgroup CS42L51_Function_Prototypes Function Prototypes
  * @{
  */
static int32_t CS42L51_ReadRegWrap(const void *handle, uint16_t Reg, uint8_t *Data, uint16_t Length);
static int32_t CS42L51_WriteRegWrap(const void *handle, uint16_t Reg, uint8_t *Data, uint16_t Length);

/**
  * @}
  */

/** @defgroup CS42L51_Private_Functions Private Functions
  * @{
  */

/**
  * @brief Initializes the audio codec and the control interface.
  * @param pObj pointer to component object
  * @param pInit pointer de component init structure
  * @retval Component status
  */
int32_t CS42L51_Init(CS42L51_Object_t *pObj, const CS42L51_Init_t *pInit)
{
  int32_t ret = 0;
  uint8_t tmp;

  if (pObj->IsInitialized == 0U)
  {
    /* Set the device in standby mode */
    ret += cs42l51_read_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);
    tmp |= 0x01U;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);

    /* Set all power down bits to 1 */
    tmp = 0x7FU;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);
    ret += cs42l51_read_reg(&pObj->Ctx, CS42L51_MIC_PWR_SPEED_CTRL, &tmp, 1);
    tmp |= 0x0EU;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_MIC_PWR_SPEED_CTRL, &tmp, 1);

    pObj->IsInitialized = 1U;
  }
  else
  {
    /* Set all power down bits to 1 except PDN to mute ADCs and DACs*/
    tmp = 0x7EU;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);
    ret += cs42l51_read_reg(&pObj->Ctx, CS42L51_MIC_PWR_SPEED_CTRL, &tmp, 1);
    tmp |= 0x0EU;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_MIC_PWR_SPEED_CTRL, &tmp, 1);

    /* Disable zero cross and soft ramp */
    ret += cs42l51_read_reg(&pObj->Ctx, CS42L51_DAC_CTRL, &tmp, 1);
    tmp &= 0xFCU;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_DAC_CTRL, &tmp, 1);

    /* Power control : Enter standby (PDN = 1) */
    ret += cs42l51_read_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);
    tmp |= 0x01U;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);
  }


  /* Mic Power and Speed Control : Auto detect on, Speed mode SSM, tri state off, MCLK divide by 2 off */
  ret += cs42l51_read_reg(&pObj->Ctx, CS42L51_MIC_PWR_SPEED_CTRL, &tmp, 1);
  tmp = ((tmp & 0x0EU) | 0xA0U);
  ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_MIC_PWR_SPEED_CTRL, &tmp, 1);

  /* Interface control : Loopback off, Slave, I2S (SDIN and SOUT), Digital mix off, Mic mix off */
  tmp = 0x0CU;
  ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_INTERFACE_CTRL, &tmp, 1);

  /* Mic control : ADC single volume off, ADCB boost off, ADCA boost off, MicBias on AIN3B/MICIN2 pin,
  MicBias level 0.8xVA, MICB boost 32db, MICA boost 32dB */
  tmp = 0x03U;
  ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_MIC_CTRL, &tmp, 1);

  /* ADC control : ADCB HPF on, ADCB HPF freeze off, ADCA HPF on, ADCA HPF freeze off, Soft ramp B on,
  Zero cross B on, Soft ramp A on, Zero cross A on */
  tmp = 0xAFU;
  ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_ADC_CTRL, &tmp, 1);

  /* DAC output control : HP Gain to 1, Single volume control off, PCM invert signals polarity off,
  DAC channels mute on */
  tmp = 0xC3U;
  ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_DAC_OUTPUT_CTRL, &tmp, 1);

  /* DAC control : Signal processing to DAC, Freeze off, De-emphasis off, Analog output auto mute off, DAC soft ramp */
  tmp = 0x42U;
  ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_DAC_CTRL, &tmp, 1);

  /* ALCA and PGAA Control : ALCA soft ramp disable on, ALCA zero cross disable on, PGA A Gain +8dB */
  tmp = 0xD0U;
  ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_ALCA_PGAA_CTRL, &tmp, 1);

  /* ALCB and PGAB Control : ALCB soft ramp disable on, ALCB zero cross disable on, PGA B Gain +8dB */
  ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_ALCB_PGAB_CTRL, &tmp, 1);

  /* ADCA Attenuator : 0dB */
  tmp = 0x00U;
  ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_ADCA_ATTENUATOR, &tmp, 1);

  /* ADCB Attenuator : 0dB */
  ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_ADCB_ATTENUATOR, &tmp, 1);

  /* ADCA mixer volume control : ADCA mixer channel mute on, ADCA mixer volume 0dB */
  tmp = 0x80U;
  ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_ADCA_MIXER_VOL_CTRL, &tmp, 1);

  /* ADCB mixer volume control : ADCB mixer channel mute on, ADCB mixer volume 0dB */
  ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_ADCB_MIXER_VOL_CTRL, &tmp, 1);

  /* PCMA mixer volume control : PCMA mixer channel mute off, PCMA mixer volume 0dB */
  tmp = 0x00U;
  ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_PCMA_MIXER_VOL_CTRL, &tmp, 1);

  /* PCMB mixer volume control : PCMB mixer channel mute off, PCMB mixer volume 0dB */
  ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_PCMB_MIXER_VOL_CTRL, &tmp, 1);

  /* PCM channel mixer : AOUTA Left, AOUTB Right */
  ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_ADC_PCM_CHANNEL_MIXER, &tmp, 1);

  if ((pInit->OutputDevice & CS42L51_OUT_HEADPHONE) == CS42L51_OUT_HEADPHONE)
  {
    tmp = VOLUME_CONVERT(pInit->Volume);
    /* AOUTA volume control : AOUTA volume */
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_AOUTA_VOL_CTRL, &tmp, 1);
    /* AOUTB volume control : AOUTB volume */
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_AOUTB_VOL_CTRL, &tmp, 1);
  }

  /* ALC enable and attack rate : ALCB and ALCA enable, fastest attack */
  tmp = 0x40U;
  ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_ALC_ENABLE_AND_ATTACK_RATE, &tmp, 1);

  /* Store current devices */
  CS42L51_CurrentDevices = (pInit->OutputDevice | pInit->InputDevice);

  if (ret != CS42L51_OK)
  {
    ret = CS42L51_ERROR;
  }

  return ret;
}

/**
  * @brief  Deinitializes the audio codec.
  * @param  pObj pointer to component object
  * @retval Component status
  */
int32_t CS42L51_DeInit(CS42L51_Object_t *pObj)
{
  if (pObj->IsInitialized == 1U)
  {
    pObj->IsInitialized = 0U;
  }

  return CS42L51_OK;
}

/**
  * @brief  Get the CS42L51 ID.
  * @param  pObj pointer to component object
  * @param  Id component ID
  * @retval Component status
  */
int32_t CS42L51_ReadID(CS42L51_Object_t *pObj, uint32_t *Id)
{
  int32_t ret;
  uint8_t cs42l51_id;

  /* Initialize the Control interface of the Audio Codec */
  pObj->IO.Init();

  /* Get ID from component */
  ret = cs42l51_read_reg(&pObj->Ctx, CS42L51_CHIP_ID, &cs42l51_id, 1);

  *Id = cs42l51_id;

  return ret;
}

/**
  * @brief Start the audio Codec play feature.
  * @note For this codec no Play options are required.
  * @param  pObj pointer to component object
  * @retval Component status
  */
int32_t CS42L51_Play(CS42L51_Object_t *pObj)
{
  int32_t ret = 0;
  uint8_t tmp;

  if ((CS42L51_CurrentDevices & CS42L51_OUT_HEADPHONE) == CS42L51_OUT_HEADPHONE)
  {
    /* Unmute the output first */
    ret += CS42L51_SetMute(pObj, CS42L51_MUTE_OFF);

    /* DAC control : Signal processing to DAC, Freeze off, De-emphasis off, Analog output auto mute off,
	DAC soft ramp */
    tmp = 0x42U;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_DAC_CTRL, &tmp, 1);

    /* Power control 1 : PDN_DACA, PDN_DACB disable. */
    ret += cs42l51_read_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);
    tmp &= 0x9FU;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);
  }

  if ((CS42L51_CurrentDevices & CS42L51_IN_LINE1) == CS42L51_IN_LINE1)
  {
    /* ADC Input Select, Invert and Mute : AIN1B to PGAB, AIN1A to PGAA, ADCB invert off,
    ADCA invert off, ADCB mute off, ADCA mute off */
    tmp = 0x00U;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_ADCX_INPUT_SELECT, &tmp, 1);

    /* Power control 1 : PDN_PGAA, PDN_PGAA, PDN_ADCB, PDN_ADCA disable. */
    ret += cs42l51_read_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);
    tmp &= 0xF9U;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);

    /* Mic Power and Speed Control : PDN_MICA, PDN_MICB, PDN_MIC_BIAS disable. */
    ret += cs42l51_read_reg(&pObj->Ctx, CS42L51_MIC_PWR_SPEED_CTRL, &tmp, 1);
    tmp &= 0xFFU;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_MIC_PWR_SPEED_CTRL, &tmp, 1);
  }

  if ((CS42L51_CurrentDevices & CS42L51_IN_MIC1) == CS42L51_IN_MIC1)
  {
    /* ADC Input Select, Invert and Mute : AIN1B to PGAB, AIN3A to PreAmp to PGAA, ADCB invert off,
    ADCA invert off, ADCB mute on, ADCA mute off */
    tmp = 0x32U;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_ADCX_INPUT_SELECT, &tmp, 1);

    /* Power control 1 : PDN_PGAA, PDN_ADCA disable. */
    ret += cs42l51_read_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);
    tmp &= 0xF5U;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);

    /* Mic Power and Speed Control : PDN_MICA, PDN_MIC_BIAS disable. */
    ret += cs42l51_read_reg(&pObj->Ctx, CS42L51_MIC_PWR_SPEED_CTRL, &tmp, 1);
    tmp &= 0xF9U;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_MIC_PWR_SPEED_CTRL, &tmp, 1);
  }

  if ((CS42L51_CurrentDevices & CS42L51_IN_MIC2) == CS42L51_IN_MIC2)
  {
    /* Power control 1 : PDN_PGAB, PDN_ADCB disable. */
    ret += cs42l51_read_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);
    tmp &= 0xEBU;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);

    /* Mic Power and Speed Control : PDN_MICB, PDN_MIC_BIAS disable. */
    ret += cs42l51_read_reg(&pObj->Ctx, CS42L51_MIC_PWR_SPEED_CTRL, &tmp, 1);
    tmp &= 0xF5U;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_MIC_PWR_SPEED_CTRL, &tmp, 1);
  }

  /* Power control : Exit standby (PDN = 0) */
  ret += cs42l51_read_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);
  tmp &= 0xFEU;
  ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);

  return ret;
}

/**
  * @brief Pauses playing on the audio codec.
  * @param  pObj pointer to component object
  * @retval Component status
  */
int32_t CS42L51_Pause(CS42L51_Object_t *pObj)
{
  /* Pause the audio file playing */
  if (CS42L51_SetMute(pObj, CS42L51_MUTE_ON) != CS42L51_OK)
  {
    return CS42L51_ERROR;
  }

  return CS42L51_OK;
}

/**
  * @brief Resumes playing on the audio codec.
  * @param  pObj pointer to component object
  * @retval Component status
  */
int32_t CS42L51_Resume(CS42L51_Object_t *pObj)
{
  /* Resumes the audio file playing */
  return CS42L51_SetMute(pObj, CS42L51_MUTE_OFF);
}

/**
  * @brief Stops audio Codec playing. It powers down the codec.
  * @param  pObj pointer to component object
  * @param CodecPdwnMode  selects the  power down mode.
  *          - CS42L51_PDWN_SW: only mutes the audio codec. When resuming from this
  *                           mode the codec keeps the previous initialization
  *                           (no need to re-Initialize the codec registers).
  *          - CS42L51_PDWN_HW: Physically power down the codec. When resuming from this
  *                           mode, the codec is set to default configuration
  *                           (user should re-Initialize the codec in order to
  *                            play again the audio stream).
  * @retval Component status
  */
int32_t CS42L51_Stop(CS42L51_Object_t *pObj, uint32_t CodecPdwnMode)
{
  int32_t ret;
  uint8_t tmp;

  /* Mute the output first */
  ret = CS42L51_SetMute(pObj, CS42L51_MUTE_ON);

  if (CodecPdwnMode == CS42L51_PDWN_SW)
  {
    /* Only output mute required*/
  }
  else /* CS42L51_PDWN_HW */
  {
    /* Set all power down bits to 1 except PDN to mute ADCs and DACs*/
    tmp = 0x7EU;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);
    ret += cs42l51_read_reg(&pObj->Ctx, CS42L51_MIC_PWR_SPEED_CTRL, &tmp, 1);
    tmp |= 0x0EU;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_MIC_PWR_SPEED_CTRL, &tmp, 1);

    /* Disable zero cross and soft ramp */
    ret += cs42l51_read_reg(&pObj->Ctx, CS42L51_DAC_CTRL, &tmp, 1);
    tmp &= 0xFCU;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_DAC_CTRL, &tmp, 1);

    /* Power control : Enter standby (PDN = 1) */
    ret += cs42l51_read_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);
    tmp |= 0x01U;
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_PWR_CTRL_1, &tmp, 1);
  }

  if (ret != CS42L51_OK)
  {
    ret = CS42L51_ERROR;
  }

  return ret;
}

/**
  * @brief Set higher or lower the codec volume level.
  * @param  pObj pointer to component object
  * @param  InputOutput Input or Output volume
  * @param  Volume  a byte value from 0 to 255
  *         (refer to codec registers description for more details).
  * @retval Component status
  */
int32_t CS42L51_SetVolume(CS42L51_Object_t *pObj, uint32_t InputOutput, uint8_t Volume)
{
  int32_t ret;
  uint8_t convertedvol;

  if (InputOutput != VOLUME_OUTPUT)
  {
    ret = CS42L51_ERROR;
  }
  else
  {
    convertedvol = VOLUME_CONVERT(Volume);

    /* AOUTA volume control : AOUTA volume */
    ret = cs42l51_write_reg(&pObj->Ctx, CS42L51_AOUTA_VOL_CTRL, &convertedvol, 1);
    /* AOUTB volume control : AOUTB volume */
    ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_AOUTB_VOL_CTRL, &convertedvol, 1);
  }

  if (ret != CS42L51_OK)
  {
    ret = CS42L51_ERROR;
  }

  return ret;
}

/**
  * @brief Get higher or lower the codec volume level.
  * @param  pObj pointer to component object
  * @param  InputOutput Input or Output volume
  * @param  Volume audio volume
  * @retval Component status
  */
int32_t CS42L51_GetVolume(CS42L51_Object_t *pObj, uint32_t InputOutput, uint8_t *Volume)
{
  int32_t ret;
  uint8_t tmp;

  if (InputOutput != VOLUME_OUTPUT)
  {
    ret = CS42L51_ERROR;
  }
  else
  {
    ret = cs42l51_read_reg(&pObj->Ctx, CS42L51_AOUTA_VOL_CTRL, &tmp, 1);
    *Volume = VOLUME_INVERT(tmp);
  }

  return ret;
}

/**
  * @brief Enables or disables the mute feature on the audio codec.
  * @param  pObj pointer to component object
  * @param Cmd  CS42L51_MUTE_ON to enable the mute or CS42L51_MUTE_OFF to disable the
  *             mute mode.
  * @retval Component status
  */
int32_t CS42L51_SetMute(CS42L51_Object_t *pObj, uint32_t Cmd)
{
  int32_t ret;
  uint8_t tmp;

  ret = cs42l51_read_reg(&pObj->Ctx, CS42L51_DAC_OUTPUT_CTRL, &tmp, 1);

  /* Set the Mute mode */
  if (Cmd == CS42L51_MUTE_ON)
  {
    tmp |= 0x03U;
  }
  else /* CS42L51_MUTE_OFF Disable the Mute */
  {
    tmp &= 0xFCU;
  }

  ret += cs42l51_write_reg(&pObj->Ctx, CS42L51_DAC_OUTPUT_CTRL, &tmp, 1);

  if (ret != CS42L51_OK)
  {
    ret = CS42L51_ERROR;
  }

  return ret;
}

/**
  * @brief Switch dynamically (while audio file is played) the output target
  *         (speaker or headphone).
  * @param  pObj pointer to component object
  * @param  Output Only CS42L51_OUT_HEADPHONE output is supported
  * @retval Component status
  */
int32_t CS42L51_SetOutputMode(const CS42L51_Object_t *pObj, uint32_t Output)
{
  (void)(pObj);
  (void)(Output);

  /* This feature is not supported */
  return CS42L51_ERROR;
}

/**
  * @brief Set Audio resolution.
  * @param pObj pointer to component object
  * @param Resolution  Audio resolution. Can be:
  *                    CS42L51_RESOLUTION_16B, CS42L51_RESOLUTION_18B,
  *                    CS42L51_RESOLUTION_20B or CS42L51_RESOLUTION_24B
  * @note This is applicable only for CS42L51_PROTOCOL_R_JUSTIFIED protocol
  * @retval Component status
  */
int32_t CS42L51_SetResolution(const CS42L51_Object_t *pObj, uint32_t Resolution)
{
  (void)(pObj);
  (void)(Resolution);

  /* This feature is not supported */
  return CS42L51_ERROR;
}

/**
  * @brief Get Audio resolution.
  * @param pObj pointer to component object
  * @retval Audio resolution
  */
int32_t CS42L51_GetResolution(const CS42L51_Object_t *pObj, const uint32_t *Resolution)
{
  (void)(pObj);
  (void)(Resolution);

  /* This feature is not supported */
  return CS42L51_ERROR;
}

/**
  * @brief Set Audio Protocol.
  * @param pObj pointer to component object
  * @param Protocol Audio Protocol. Can be:
  *                  CS42L51_PROTOCOL_R_JUSTIFIED, CS42L51_PROTOCOL_L_JUSTIFIED
  *                  or CS42L51_PROTOCOL_I2S
  * @retval Component status
  */
int32_t CS42L51_SetProtocol(const CS42L51_Object_t *pObj, uint32_t Protocol)
{
  (void)(pObj);
  (void)(Protocol);

  /* This feature is not supported */
  return CS42L51_ERROR;
}

/**
  * @brief Get Audio Protocol.
  * @param pObj pointer to component object
  * @param Protocol pointer to protocol value
  * @retval Component status
  */
int32_t CS42L51_GetProtocol(const CS42L51_Object_t *pObj, const uint32_t *Protocol)
{
  (void)(pObj);
  (void)(Protocol);

  /* This feature is not supported */
  return CS42L51_ERROR;
}

/**
  * @brief Sets new frequency.
  * @param pObj pointer to component object
  * @param AudioFreq Audio frequency
  * @retval Component status
  */
int32_t CS42L51_SetFrequency(const CS42L51_Object_t *pObj, uint32_t AudioFreq)
{
  (void)(pObj);
  (void)(AudioFreq);

  /* This feature is not supported */
  return CS42L51_ERROR;
}

/**
  * @brief Get frequency.
  * @param pObj pointer to component object
  * @param AudioFreq Audio frequency
  * @retval Component status
  */
int32_t CS42L51_GetFrequency(const CS42L51_Object_t *pObj, const uint32_t *AudioFreq)
{
  (void)(pObj);
  (void)(AudioFreq);

  /* This feature is not supported */
  return CS42L51_ERROR;
}

/**
  * @brief Resets cs42l51 registers.
  * @param pObj pointer to component object
  * @retval Component status
  */
int32_t CS42L51_Reset(const CS42L51_Object_t *pObj)
{
  /* De-Initialize Audio Codec interface */
  pObj->IO.DeInit();

  /* Initialize Audio Codec interface */
  pObj->IO.Init();

  return CS42L51_OK;
}

/******************** Static functions ****************************************/
/**
  * @brief  Function
  * @param  Component object pointer
  * @retval error status
  */
int32_t CS42L51_RegisterBusIO(CS42L51_Object_t *pObj, CS42L51_IO_t *pIO)
{
  int32_t ret;

  if (pObj == NULL)
  {
    ret = CS42L51_ERROR;
  }
  else
  {
    pObj->IO.Init      = pIO->Init;
    pObj->IO.DeInit    = pIO->DeInit;
    pObj->IO.Address   = pIO->Address;
    pObj->IO.WriteReg  = pIO->WriteReg;
    pObj->IO.ReadReg   = pIO->ReadReg;
    pObj->IO.GetTick   = pIO->GetTick;

    pObj->Ctx.ReadReg  = CS42L51_ReadRegWrap;
    pObj->Ctx.WriteReg = CS42L51_WriteRegWrap;
    pObj->Ctx.handle   = pObj;

    if (pObj->IO.Init != NULL)
    {
      ret = pObj->IO.Init();
    }
    else
    {
      ret = CS42L51_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Function
  * @param  handle  Component object handle
  * @param  Reg     The target register address to write
  * @param  pData   The target register value to be written
  * @param  Length  buffer size to be written
  * @retval error status
  */
static int32_t CS42L51_ReadRegWrap(const void *handle, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
  const CS42L51_Object_t *pObj = (const CS42L51_Object_t *)handle;

  return pObj->IO.ReadReg(pObj->IO.Address, Reg, pData, Length);
}

/**
  * @brief  Function
  * @param  handle Component object handle
  * @param  Reg    The target register address to write
  * @param  pData  The target register value to be written
  * @param  Length buffer size to be written
  * @retval error status
  */
static int32_t CS42L51_WriteRegWrap(const void *handle, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
  const CS42L51_Object_t *pObj = (const CS42L51_Object_t *)handle;

  return pObj->IO.WriteReg(pObj->IO.Address, Reg, pData, Length);
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
