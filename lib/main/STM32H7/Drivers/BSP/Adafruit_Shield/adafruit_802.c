/**
  ******************************************************************************
  * @file    adafruit_802.c
  * @author  MCD Application Team
  * @brief   This file provides set of firmware functions to manage:
  *          - Joystick available on Adafruit 1.8" TFT LCD shield (reference ID 802)
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "adafruit_802.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup ADAFRUIT_802
  * @{
  */

/** @addtogroup ADAFRUIT_802_LOW_LEVEL
  * @{
  */

/** @defgroup ADAFRUIT_802_LOW_LEVEL_Private_Variables LOW LEVEL Private Variables
  * @{
  */
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
static uint32_t IsJoyMspCbValid = 0;
#endif

static ADC_HandleTypeDef hJoyHandle;
/**
  * @}
  */

/** @defgroup ADAFRUIT_802_LOW_LEVEL_Private_FunctionPrototypes LOW LEVEL Private FunctionPrototypes
  * @{
  */
static void JOY1_MspInit(ADC_HandleTypeDef *hadc);
static void JOY1_MspDeInit(ADC_HandleTypeDef *hadc);
/**
  * @}
  */

/** @addtogroup ADAFRUIT_802_LOW_LEVEL_Exported_Functions
  * @{
  */

/**
  * @brief  Configures the joystick.
  * @param  JOY Joystick to be initialized
  * @param  JoyMode Button mode.
  *          This parameter is not used as the ADC pin should be always configured
  *          in analog mode.
  * @param  JoyPins There are no physical pins but voltage levels
  * @retval BSP status
  */
int32_t ADAFRUIT_802_JOY_Init(JOY_TypeDef JOY, JOYMode_TypeDef JoyMode, JOYPin_TypeDef JoyPins)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(JoyPins);
  UNUSED(JoyMode);

  int32_t ret = BSP_ERROR_NONE;

  if(JOY >= JOYn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 0)
    /* Init the UART Msp */
    JOY1_MspInit(&hJoyHandle);
#else
    if(IsJoyMspCbValid == 0U)
    {
      if(ADAFRUIT_802_JOY_RegisterDefaultMspCallbacks(JOY) != BSP_ERROR_NONE)
      {
        return BSP_ERROR_MSP_FAILURE;
      }
    }
#endif

    if(MX_ADAFRUIT_802_ADC_Init(&hJoyHandle) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
#if defined(ADC_CALFACT_CALFACT_S)
      /* Run ADC calibration */
#if defined(ADC_CR_ADCALLIN)
      if(HAL_ADCEx_Calibration_Start(&hJoyHandle, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
      {
        ret =  BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
        if(HAL_ADCEx_Calibration_Start(&hJoyHandle, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED) != HAL_OK)
        {
          ret =  BSP_ERROR_PERIPH_FAILURE;
        }
      }
#else
      if(HAL_ADCEx_Calibration_Start(&hJoyHandle, ADC_SINGLE_ENDED) != HAL_OK)
      {
        ret =  BSP_ERROR_PERIPH_FAILURE;
      }
#endif
#endif /* defined(ADC_CALFACT_CALFACT_S) */
    }
  }

  return ret;
}

/**
  * @brief  DeInit joystick GPIOs.
  * @param  JOY Joystick to be de-initialized
  * @param  JoyPins There are no physical pins but voltage levels
  * @retval BSP status
  */
int32_t ADAFRUIT_802_JOY_DeInit(JOY_TypeDef JOY, JOYPin_TypeDef JoyPins)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(JoyPins);

  int32_t  ret = BSP_ERROR_NONE;

  /* ADC configuration */
  hJoyHandle.Instance = ADAFRUIT_802_ADCx;

  if(JOY >= JOYn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 0)
    JOY1_MspDeInit(&hJoyHandle);
#endif /* (USE_HAL_UART_REGISTER_CALLBACKS == 0) */

    if(HAL_ADC_DeInit(&hJoyHandle) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  Returns the Joystick key pressed.
  * @param  JOY Joystick to get its pins' state
  * @retval BSP error code if value is negative or one of the following values:
  *           - JOY_NONE   : 3.3 V / 4095
  *           - JOY_SEL    : 1.055 V / 1308
  *           - JOY_DOWN   : 0.71 V / 88
  *           - JOY_LEFT   : 3.0 V / 3720
  *           - JOY_RIGHT  : 0.595 V / 737
  *           - JOY_UP     : 1.65 V / 2046
  */
int32_t ADAFRUIT_802_JOY_GetState(JOY_TypeDef JOY)
{
  int32_t ret;
  uint32_t  keyconvertedvalue;

  if(JOY >= JOYn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }/* Start the conversion process */
  else if(HAL_ADC_Start(&hJoyHandle) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }/* Wait for the end of conversion */
  else if(HAL_ADC_PollForConversion(&hJoyHandle, ADAFRUIT_802_ADCx_POLL_TIMEOUT) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }/* Check if the continuous conversion of regular channel is finished */
  else if((HAL_ADC_GetState(&hJoyHandle) & HAL_ADC_STATE_REG_EOC) != HAL_ADC_STATE_REG_EOC)
  {
    ret = BSP_ERROR_BUSY;
  }
  else
  {
    /* Get the converted value of regular channel */
    keyconvertedvalue = HAL_ADC_GetValue(&hJoyHandle);

    if((keyconvertedvalue > 2010U) && (keyconvertedvalue < 2500U))
    {
      ret = (int32_t)JOY_UP;
    }
    else if((keyconvertedvalue > 680U) && (keyconvertedvalue < 900U))
    {
      ret = (int32_t)JOY_RIGHT;
    }
    else if((keyconvertedvalue > 1270U) && (keyconvertedvalue < 1500U))
    {
      ret = (int32_t)JOY_SEL;
    }
    else if((keyconvertedvalue > 50U) && (keyconvertedvalue < 200U))
    {
      ret = (int32_t)JOY_DOWN;
    }
    else if((keyconvertedvalue > 3400U) && (keyconvertedvalue < 3760U))
    {
      ret = (int32_t)JOY_LEFT;
    }
    else
    {
      ret = (int32_t)JOY_NONE;
    }
  }

  /* Return the code of the Joystick key pressed */
  return ret;
}

/**
  * @brief  Configures ADC peripheral
  * @param  hadc ADC handle
  * @retval HAL error code
  */
__weak HAL_StatusTypeDef MX_ADAFRUIT_802_ADC_Init(ADC_HandleTypeDef *hadc)
{
  HAL_StatusTypeDef ret = HAL_OK;
  ADC_ChannelConfTypeDef sConfig;

  /* ADC configuration */
  hadc->Instance                   = ADAFRUIT_802_ADCx;
  hadc->Init.ClockPrescaler        = ADAFRUIT_802_ADCx_PRESCALER;
  hadc->Init.Resolution            = ADC_RESOLUTION_12B;
#if defined(ADC_CR2_ALIGN)
  hadc->Init.DataAlign             = ADC_DATAALIGN_RIGHT;
#endif
  hadc->Init.ScanConvMode          = (uint32_t)DISABLE;
#if defined(ADC_EOC_SINGLE_CONV)
  hadc->Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
#endif
  hadc->Init.ContinuousConvMode    = DISABLE;
  hadc->Init.NbrOfConversion       = 1;
  hadc->Init.DiscontinuousConvMode = DISABLE;
#if defined(ADC_CFGR_DISCNUM)
  hadc->Init.NbrOfDiscConversion   = 0;
#endif
  hadc->Init.ExternalTrigConv      = ADC_SOFTWARE_START;
  hadc->Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
#if defined(ADC_CR2_DMA)
  hadc->Init.DMAContinuousRequests = DISABLE;
#endif
#if defined(ADC_SMPR_SMP1) && defined(ADC_SMPR_SMP2)
  hadc->Init.SamplingTimeCommon1 = ADC_SAMPLETIME_39CYCLES_5;
  hadc->Init.SamplingTimeCommon2 = ADC_SAMPLETIME_39CYCLES_5;
#endif
#if defined(ADC_CFGR2_LFTRIG)
  hadc->Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
#endif

  if (HAL_ADC_Init(hadc) != HAL_OK)
  {
    ret =  HAL_ERROR;
  }
  else
  {
    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
    */
    sConfig.Channel      = ADAFRUIT_802_ADCx_CHANNEL;
    sConfig.Rank         = ADAFRUIT_802_ADCx_RANK;
    sConfig.SamplingTime = ADAFRUIT_802_ADCx_SAMPLETIME;
#if defined(ADC_OFR1_OFFSET1)
    sConfig.Offset       = 0;
#endif
#if defined(ADC_SINGLE_ENDED)
    sConfig.SingleDiff   = ADC_SINGLE_ENDED;
#endif
#if defined(ADC_OFFSET_NONE)
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
#endif
    if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK)
    {
      ret =  HAL_ERROR;
    }
  }

  return ret;
}

#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
/**
  * @brief Register Default ADCx Bus Msp Callbacks
  * @param  JOY Joystick to be configured.
  *          This parameter can be JOY1
  * @retval BSP status
  */
int32_t ADAFRUIT_802_JOY_RegisterDefaultMspCallbacks(JOY_TypeDef JOY)
{
  int32_t ret = BSP_ERROR_NONE;

  if(JOY >= JOYn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    __HAL_ADC_RESET_HANDLE_STATE(&hJoyHandle);

    /* Register default MspInit/MspDeInit Callback */
    if(HAL_ADC_RegisterCallback(&hJoyHandle, HAL_ADC_MSPINIT_CB_ID, JOY1_MspInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_ADC_RegisterCallback(&hJoyHandle, HAL_ADC_MSPDEINIT_CB_ID, JOY1_MspDeInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      IsJoyMspCbValid = 1U;
    }
  }

  /* BSP status */
  return ret;
}

/**
  * @brief Register ADCx Bus Msp Callback registering
  * @param  JOY Joystick to be configured.
  *          This parameter can be JOY1
  * @param Callbacks     pointer to ADCx MspInit/MspDeInit callback functions
  * @retval BSP status
  */
int32_t ADAFRUIT_802_JOY_RegisterMspCallbacks(JOY_TypeDef JOY, ADAFRUIT_802_JOY_Cb_t *Callback)
{
  int32_t ret = BSP_ERROR_NONE;

  if(JOY >= JOYn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    __HAL_ADC_RESET_HANDLE_STATE(&hJoyHandle);

    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_ADC_RegisterCallback(&hJoyHandle, HAL_ADC_MSPINIT_CB_ID, Callback->pMspInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_ADC_RegisterCallback(&hJoyHandle, HAL_ADC_MSPDEINIT_CB_ID, Callback->pMspDeInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      IsJoyMspCbValid = 1U;
    }
  }

  /* BSP status */
  return ret;
}
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */

/**
  * @}
  */

/** @defgroup ADAFRUIT_802_LOW_LEVEL_Private_Functions LOW LEVEL Private Functions
  * @{
  */
/**
  * @brief  Initializes ADC MSP.
  * @param  hadc ADC handle
  * @retval None
  */
static void JOY1_MspInit(ADC_HandleTypeDef *hadc)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
#if defined (RCC_ADCCLKSOURCE_SYSCLK) || defined (RCC_ADC12CLKSOURCE_SYSCLK)
  RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct;
#endif

  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);

  /*** Configure the GPIOs ***/
  /* Enable GPIO clock */
  ADAFRUIT_802_ADCx_GPIO_CLK_ENABLE();

  /* Configure the selected ADC Channel as analog input */
  GPIO_InitStruct.Pin  = ADAFRUIT_802_ADCx_GPIO_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
#if defined (RCC_ADCCLKSOURCE_SYSCLK) || defined (RCC_ADC12CLKSOURCE_SYSCLK)
  GPIO_InitStruct.Mode  = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
#endif
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ADAFRUIT_802_ADCx_GPIO_PORT, &GPIO_InitStruct);

  /*** Configure the ADC peripheral ***/
  /* Enable ADC clock */
  ADAFRUIT_802_ADCx_CLK_ENABLE();

#if defined (RCC_ADCCLKSOURCE_SYSCLK)
  /* Configure SYSCLK as source clock for ADC */
  RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  RCC_PeriphCLKInitStruct.AdcClockSelection    = RCC_ADCCLKSOURCE_SYSCLK;
  HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
#elif defined (RCC_ADC12CLKSOURCE_SYSCLK)
  RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
  RCC_PeriphCLKInitStruct.Adc12ClockSelection    = RCC_ADC12CLKSOURCE_SYSCLK;
  HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
#endif
}

/**
  * @brief  DeInitializes ADC MSP.
  * @param  hadc ADC handle
  * @note ADC DeInit does not disable the GPIO clock
  * @retval None
  */
static void JOY1_MspDeInit(ADC_HandleTypeDef *hadc)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);

  /*** DeInit the ADC peripheral ***/
  /* Disable ADC clock */
  ADAFRUIT_802_ADCx_CLK_DISABLE();

  /* Configure the selected ADC Channel as analog input */
  GPIO_InitStruct.Pin = ADAFRUIT_802_ADCx_GPIO_PIN ;
  HAL_GPIO_DeInit(ADAFRUIT_802_ADCx_GPIO_PORT, GPIO_InitStruct.Pin);
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
