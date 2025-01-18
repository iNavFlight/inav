/**
  ******************************************************************************
  * @file    stm32h7b3i_eval.c
  * @author  MCD Application Team
  * @brief   This file provides a set of firmware functions to manage
  *          LEDs
  *          push-buttons
  *          Joystick
  *          POT
  *          COM ports
  *          available on STM32H7B3I_EVAL board(MB1331) from STMicroelectronics.
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
#include "stm32h7b3i_eval.h"
#if (USE_BSP_IO_CLASS > 0)
#include "stm32h7b3i_eval_io.h"
#endif

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H7B3I_EVAL
  * @{
  */

/** @defgroup STM32H7B3I_EVAL_LOW_LEVEL LOW LEVEL
  * @{
  */

/** @defgroup STM32H7B3I_EVAL_LOW_LEVEL_Private_TypesDefinitions LOW LEVEL Private Types Definitions
  * @{
  */
typedef void (* BSP_EXTI_LineCallback)(void);
/**
  * @}
  */

/** @defgroup STM32H7B3I_EVAL_LOW_LEVEL_Private_FunctionPrototypes  LOW LEVEL Private Function Prototypes
  * @{
  */
static void BUTTON_WAKEUP_EXTI_Callback(void);
static void BUTTON_TAMPER_EXTI_Callback(void);
static void BUTTON_USER_EXTI_Callback(void);
#if (USE_BSP_COM_FEATURE > 0)
static void USART1_MspInit(UART_HandleTypeDef *huart);
static void USART1_MspDeInit(UART_HandleTypeDef *huart);
#endif
#if (USE_BSP_POT_FEATURE > 0)
static void ADC1_MspInit(ADC_HandleTypeDef *hadc);
static void ADC1_MspDeInit(ADC_HandleTypeDef *hadc);
#endif

#if (USE_BSP_IO_CLASS > 0)
static void JOY1_UP_EXTI_Callback(void);
static void JOY1_DOWN_EXTI_Callback(void);
static void JOY1_LEFT_EXTI_Callback(void);
static void JOY1_RIGHT_EXTI_Callback(void);
static void JOY1_SEL_EXTI_Callback(void);
#endif

/**
  * @}
  */
/** @defgroup STM32H7B3I_EVAL_LOW_LEVEL_Exported_Variables LOW LEVEL Exported Variables
  * @{
  */
EXTI_HandleTypeDef hpb_exti[BUTTONn];
#if (USE_BSP_COM_FEATURE > 0)
UART_HandleTypeDef hcom_uart[COMn];
USART_TypeDef *COM_USART[COMn]   = {COM1_UART, COM1_UART};
#endif
#if (USE_BSP_POT_FEATURE > 0)
ADC_HandleTypeDef hpot_adc[POTn];
#endif
/**
  * @}
  */

/** @defgroup STM32H7B3I_EVAL_LOW_LEVEL_Private_Variables LOW LEVEL Private Variables
  * @{
  */
static GPIO_TypeDef *LED_PORT[LEDn] =
{
  LED1_GPIO_PORT,
#if (USE_BSP_IO_CLASS > 0)
  0,
  0,
  0
#endif
};

static const uint32_t LED_PIN[LEDn] = {LED1_PIN,
#if (USE_BSP_IO_CLASS > 0)
                                       LED2_PIN,
                                       LED3_PIN,
                                       LED4_PIN
#endif
                                      };

static GPIO_TypeDef *BUTTON_PORT[BUTTONn] = {BUTTON_WAKEUP_GPIO_PORT,
                                             BUTTON_TAMPER_GPIO_PORT,
                                             BUTTON_USER_GPIO_PORT
                                            };
static const uint16_t BUTTON_PIN[BUTTONn] = {BUTTON_WAKEUP_PIN,
                                             BUTTON_TAMPER_PIN,
                                             BUTTON_USER_PIN
                                            };
static const IRQn_Type BUTTON_IRQn[BUTTONn] = {BUTTON_WAKEUP_EXTI_IRQn,
                                               BUTTON_TAMPER_EXTI_IRQn,
                                               BUTTON_USER_EXTI_IRQn
                                              };


#if (USE_COM_LOG > 0)
static COM_TypeDef COM_ActiveLogPort = COM1;
#endif

#if (USE_BSP_COM_FEATURE > 0)
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
static uint32_t IsComMspCbValid[COMn] = {0};
#endif
#endif

#if (USE_BSP_POT_FEATURE > 0)
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
static uint32_t IsPotMspCbValid[POTn] = {0};
#endif
#endif
#if (USE_BSP_IO_CLASS > 0)
static uint32_t JoyPinsMask;
static GPIO_TypeDef *JOY1_PORT[JOY_KEY_NUMBER] = {JOY1_SEL_GPIO_PORT,
                                                  JOY1_DOWN_GPIO_PORT,
                                                  JOY1_LEFT_GPIO_PORT,
                                                  JOY1_RIGHT_GPIO_PORT,
                                                  JOY1_UP_GPIO_PORT
                                                 };

static const uint16_t JOY1_PIN[JOY_KEY_NUMBER] = {JOY1_SEL_PIN,
                                                  JOY1_DOWN_PIN,
                                                  JOY1_LEFT_PIN,
                                                  JOY1_RIGHT_PIN,
                                                  JOY1_UP_PIN
                                                 };
static EXTI_HandleTypeDef hExtiJoyHandle[JOY_KEY_NUMBER];
#endif
/**
  * @}
  */

/** @defgroup STM32H7B3I_EVAL_LOW_LEVEL_Private_Functions LOW LEVEL Exported Functions
  * @{
  */

/**
  * @brief  This method returns the STM32H7B3I_EVAL BSP Driver revision
  * @retval version: 0xXYZR (8bits for each decimal, R for RC)
*/
int32_t BSP_GetVersion(void)
{
  return (int32_t)STM32H7B3I_EVAL_BSP_VERSION;
}

/**
  * @brief  This method returns the board name
  * @retval pointer to the board name string
  */
const uint8_t *BSP_GetBoardName(void)
{
  return (uint8_t *)STM32H7B3I_EVAL_BSP_BOARD_NAME;
}

/**
  * @brief  This method returns the board ID
  * @retval pointer to the board name string
  */
const uint8_t *BSP_GetBoardID(void)
{
  return (uint8_t *)STM32H7B3I_EVAL_BSP_BOARD_ID;
}

/**
  * @brief  Configures LED on GPIO.
  * @param  Led LED to be configured.
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  *            @arg  LED3
  *            @arg  LED4
  * @retval BSP status
  */
int32_t BSP_LED_Init(Led_TypeDef Led)
{
  int32_t ret = BSP_ERROR_NONE;

#if (USE_BSP_IO_CLASS > 0)
  BSP_IO_Init_t io_init_structure;
#endif /* (USE_BSP_IO_CLASS > 0) */
  GPIO_InitTypeDef  gpio_init_structure;

  if (Led == LED1)
  {
    /* Enable the GPIO_LED clock */
    LED1_GPIO_CLK_ENABLE();

    /* Configure the GPIO_LED pin */
    gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Pull = GPIO_PULLUP;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio_init_structure.Pin = LED_PIN [Led];
    HAL_GPIO_Init(LED_PORT[Led], &gpio_init_structure);
    HAL_GPIO_WritePin(LED_PORT [Led], (uint16_t)LED_PIN[Led], GPIO_PIN_SET);
  }
  else
  {
#if (USE_BSP_IO_CLASS > 0)

    io_init_structure.Pin  = LED_PIN[Led];
    io_init_structure.Pull = IO_PULLUP;
    io_init_structure.Mode = IO_MODE_OUTPUT_PP;
    /* Initialize IO expander */
    if (BSP_IO_Init(0, &io_init_structure) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_NO_INIT;
    }
    else
    {
      if (BSP_IO_WritePin(0, LED_PIN[Led], IO_PIN_SET) != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_NO_INIT;
      }
    }
#endif /* (USE_BSP_IO_CLASS > 0) */
  }

  return ret;
}

/**
  * @brief  DeInit LEDs.
  * @param  Led LED to be configured.
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  *            @arg  LED3
  *            @arg  LED4
  * @retval BSP status
  */
int32_t BSP_LED_DeInit(Led_TypeDef Led)
{
  int32_t ret = BSP_ERROR_NONE;
#if (USE_BSP_IO_CLASS > 0)
  BSP_IO_Init_t io_init_structure;
#endif /* (USE_BSP_IO_CLASS > 0) */
  GPIO_InitTypeDef  gpio_init_structure;

  if (Led == LED1)
  {
    /* Configure the GPIO_LED pin */
    gpio_init_structure.Pin = LED_PIN [Led];
    HAL_GPIO_WritePin(LED_PORT[Led], (uint16_t)LED_PIN[Led], GPIO_PIN_RESET);

    HAL_GPIO_DeInit(LED_PORT[Led], gpio_init_structure.Pin);
  }
  else
  {
#if (USE_BSP_IO_CLASS > 0)
    if (BSP_IO_WritePin(0, LED_PIN[Led], IO_PIN_RESET) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_NO_INIT;
    }
    else
    {
      io_init_structure.Pin  = LED_PIN[Led];
      io_init_structure.Pull = IO_PULLUP;
      io_init_structure.Mode = IO_MODE_OFF;

      if (BSP_IO_Init(0, &io_init_structure) != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_NO_INIT;
      }
    }
#endif /* (USE_BSP_IO_CLASS > 0) */
  }
  return ret;
}

/**
  * @brief  Turns selected LED On.
  * @param  Led LED to be set on
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  *            @arg  LED3
  *            @arg  LED4
  * @retval BSP status
  */
int32_t BSP_LED_On(Led_TypeDef Led)
{
  int32_t ret;

  if (Led == LED1)
  {
    HAL_GPIO_WritePin(LED_PORT [Led], (uint16_t)LED_PIN [Led], GPIO_PIN_RESET);
    ret = BSP_ERROR_NONE;
  }
  else
  {
#if (USE_BSP_IO_CLASS > 0)
    ret = BSP_IO_WritePin(0, LED_PIN[Led], IO_PIN_RESET);
#else
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
#endif /* (USE_BSP_IO_CLASS > 0) */
  }
  return ret;
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led LED to be set off
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  *            @arg  LED3
  *            @arg  LED4
  * @retval BSP status
  */
int32_t BSP_LED_Off(Led_TypeDef Led)
{
  int32_t ret;
  if (Led == LED1)
  {
    HAL_GPIO_WritePin(LED_PORT [Led], (uint16_t)LED_PIN [Led], GPIO_PIN_SET);
    ret = BSP_ERROR_NONE;
  }
  else
  {
#if (USE_BSP_IO_CLASS > 0)
    ret = BSP_IO_WritePin(0, LED_PIN[Led], IO_PIN_SET);
#else
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
#endif /* (USE_BSP_IO_CLASS > 0) */
  }
  return ret;
}

/**
  * @brief  Toggles the selected LED.
  * @param  Led LED to be toggled
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  *            @arg  LED3
  *            @arg  LED4
  * @retval BSP status
  */
int32_t BSP_LED_Toggle(Led_TypeDef Led)
{
  int32_t ret;
  if (Led == LED1)
  {
    HAL_GPIO_TogglePin(LED_PORT[Led], (uint16_t)LED_PIN[Led]);
    ret = BSP_ERROR_NONE;
  }
  else
  {
#if (USE_BSP_IO_CLASS > 0)
    ret = BSP_IO_TogglePin(0, LED_PIN[Led]);
#else
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
#endif /* (USE_BSP_IO_CLASS > 0) */
  }
  return ret;
}

/**
  * @brief  Get the selected LED state.
  * @param  Led LED to be get its state
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  *            @arg  LED3
  *            @arg  LED4
  * @retval LED status
  */
int32_t BSP_LED_GetState(Led_TypeDef Led)
{
  int32_t ret;

  if (Led == LED1)
  {
    ret = (int32_t)HAL_GPIO_ReadPin(LED_PORT [Led], (uint16_t)LED_PIN [Led]);
  }
  else
  {
#if (USE_BSP_IO_CLASS > 0)
    ret = BSP_IO_ReadPin(0, LED_PIN[Led]);
#else
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
#endif /* (USE_BSP_IO_CLASS > 0) */
  }

  return ret;
}

/**
  * @brief  Configures button GPIO and EXTI Line.
  * @param  Button: Button to be configured
  *          This parameter can be one of the following values:
  *            @arg  BUTTON_USER: User Push Button
  *            @arg  BUTTON_TAMPER: Tamper Push Button
  *            @arg  BUTTON_WAKEUP: Wakeup Push Button
  * @param  ButtonMode Button mode
  *          This parameter can be one of the following values:
  *            @arg  BUTTON_MODE_GPIO: Button will be used as simple IO
  *            @arg  BUTTON_MODE_EXTI: Button will be connected to EXTI line
  *                                    with interrupt generation capability
  * @retval BSP status
  */
int32_t BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode)
{
  GPIO_InitTypeDef gpio_init_structure;
  static BSP_EXTI_LineCallback ButtonCallback[BUTTONn] =
  {
    BUTTON_WAKEUP_EXTI_Callback,
    BUTTON_TAMPER_EXTI_Callback,
    BUTTON_USER_EXTI_Callback
  };
  static uint32_t  BSP_BUTTON_PRIO [BUTTONn] =
  {
    BSP_BUTTON_WAKEUP_IT_PRIORITY,
    BSP_BUTTON_TAMPER_IT_PRIORITY,
    BSP_BUTTON_USER_IT_PRIORITY,
  };
  static const uint32_t BUTTON_EXTI_LINE[BUTTONn] =
  {
    BUTTON_WAKEUP_EXTI_LINE,
    BUTTON_TAMPER_EXTI_LINE,
    BUTTON_USER_EXTI_LINE,
  };
  /* Enable the BUTTON clock*/
  if (Button == BUTTON_WAKEUP)
  {
    BUTTON_WAKEUP_GPIO_CLK_ENABLE();
  }
  else if (Button == BUTTON_TAMPER)
  {
    BUTTON_TAMPER_GPIO_CLK_ENABLE();
  }
  else
  {
    BUTTON_USER_GPIO_CLK_ENABLE();
  }
  gpio_init_structure.Pin = BUTTON_PIN [Button];
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;

  if (Button == BUTTON_TAMPER)
  {
    gpio_init_structure.Pull = GPIO_PULLUP;
  }

  if (ButtonMode == BUTTON_MODE_GPIO)
  {
    /* Configure Button pin as input */
    gpio_init_structure.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(BUTTON_PORT [Button], &gpio_init_structure);
  }
  else /* (ButtonMode == BUTTON_MODE_EXTI) */
  {
    /* Configure Button pin as input with External interrupt */
    gpio_init_structure.Mode = GPIO_MODE_IT_RISING;

    HAL_GPIO_Init(BUTTON_PORT[Button], &gpio_init_structure);

    (void)HAL_EXTI_GetHandle(&hpb_exti[Button], BUTTON_EXTI_LINE[Button]);
    (void)HAL_EXTI_RegisterCallback(&hpb_exti[Button],  HAL_EXTI_COMMON_CB_ID, ButtonCallback[Button]);

    /* Enable and set Button EXTI Interrupt to the lowest priority */
    HAL_NVIC_SetPriority((BUTTON_IRQn[Button]), BSP_BUTTON_PRIO[Button], 0x00);
    HAL_NVIC_EnableIRQ((BUTTON_IRQn[Button]));
  }
  return BSP_ERROR_NONE;
}

/**
  * @brief  Push Button DeInit.
  * @param  Button Button to be configured
  *          This parameter can be one of the following values:
  *            @arg  BUTTON_USER: User Push Button
  *            @arg  BUTTON_TAMPER: Tamper Push Button
  *            @arg  BUTTON_WAKEUP: Wakeup Push Button
  * @note PB DeInit does not disable the GPIO clock
  * @retval BSP status
  */

int32_t BSP_PB_DeInit(Button_TypeDef Button)
{
  GPIO_InitTypeDef gpio_init_structure;

  gpio_init_structure.Pin = BUTTON_PIN[Button];
  HAL_NVIC_DisableIRQ((IRQn_Type)(BUTTON_IRQn[Button]));
  HAL_GPIO_DeInit(BUTTON_PORT[Button], gpio_init_structure.Pin);

  return BSP_ERROR_NONE;
}

/**
  * @brief  Returns the selected button state.
  * @param  Button Button to be checked
  *          This parameter can be one of the following values:
  *            @arg  BUTTON_USER: User Push Button
  *            @arg  BUTTON_TAMPER: Tamper Push Button
  *            @arg  BUTTON_WAKEUP: Wakeup Push Button
  * @retval The Button GPIO pin value
  */
int32_t BSP_PB_GetState(Button_TypeDef Button)
{
  return (int32_t)HAL_GPIO_ReadPin(BUTTON_PORT[Button], BUTTON_PIN[Button]);
}

/**
  * @brief  This function handles Push-Button interrupt requests.
  * @param  Button Specifies the pin connected EXTI line
  * @retval None
  */
void BSP_PB_IRQHandler(Button_TypeDef Button)
{
  HAL_EXTI_IRQHandler(&hpb_exti[Button]);
}

/**
  * @brief  BSP Push Button callback
  * @param  Button Specifies the pin connected EXTI line
  * @retval None.
  */
__weak void BSP_PB_Callback(Button_TypeDef Button)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Button);
  /* This function should be implemented by the user application.
     It is called into this driver when an event on Button is triggered. */
}

#if (USE_BSP_COM_FEATURE > 0)
/**
  * @brief  Configures COM port.
  * @param  COM COM port to be configured.
  *          This parameter can be COM1 or COM2
  * @param  COM_Init Pointer to a UART_HandleTypeDef structure that contains the
  *                configuration information for the specified USART peripheral.
  * @retval BSP status
  */
int32_t BSP_COM_Init(COM_TypeDef COM, COM_InitTypeDef *COM_Init)
{
  int32_t ret = BSP_ERROR_NONE;

  if (COM >= COMn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Init the UART Msp */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 0)
    USART1_MspInit(&hcom_uart[COM]);
#else
    if (IsComMspCbValid[COM] == 0U)
    {
      if (BSP_COM_RegisterDefaultMspCallbacks(COM) != BSP_ERROR_NONE)
      {
        return BSP_ERROR_MSP_FAILURE;
      }
    }
#endif

    if (MX_USART1_Init(&hcom_uart[COM], COM_Init) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  DeInit COM port.
  * @param  COM COM port to be configured.
  *          This parameter can be (COM1)
  * @retval BSP status
  */
int32_t BSP_COM_DeInit(COM_TypeDef COM)
{
  int32_t ret = BSP_ERROR_NONE;

  if (COM >= COMn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* USART configuration */
    hcom_uart[COM].Instance = COM_USART[COM];

#if (USE_HAL_UART_REGISTER_CALLBACKS == 0)
    USART1_MspDeInit(&hcom_uart[COM]);
#endif /* (USE_HAL_UART_REGISTER_CALLBACKS == 0) */

    if (HAL_UART_DeInit(&hcom_uart[COM]) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  Configures COM1 port.
  * @param  huart USART handle
  * @param  COM_Init Pointer to a UART_HandleTypeDef structure that contains the
  *                  configuration information for the specified USART peripheral.
  * @retval HAL error code
  */
__weak HAL_StatusTypeDef MX_USART1_Init(UART_HandleTypeDef *huart, MX_UART_InitTypeDef *COM_Init)
{
  /* USART configuration */
  huart->Instance          = COM_USART[COM1];
  huart->Init.BaudRate     = COM_Init->BaudRate;
  huart->Init.Mode         = UART_MODE_TX_RX;
  huart->Init.Parity       = (uint32_t)COM_Init->Parity;
  huart->Init.WordLength   = (uint32_t)COM_Init->WordLength;
  huart->Init.StopBits     = (uint32_t)COM_Init->StopBits;
  huart->Init.HwFlowCtl    = (uint32_t)COM_Init->HwFlowCtl;
  huart->Init.OverSampling = UART_OVERSAMPLING_8;

  return HAL_UART_Init(huart);
}

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
/**
  * @brief Register Default COM Msp Callbacks
  * @param  COM COM port to be configured.
  * @retval BSP status
  */
int32_t BSP_COM_RegisterDefaultMspCallbacks(COM_TypeDef COM)
{
  int32_t ret = BSP_ERROR_NONE;

  if (COM >= COMn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    __HAL_UART_RESET_HANDLE_STATE(&hcom_uart[COM]);

    /* Register default MspInit/MspDeInit Callback */
    if (HAL_UART_RegisterCallback(&hcom_uart[COM], HAL_UART_MSPINIT_CB_ID,) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_UART_RegisterCallback(&hcom_uart[COM], HAL_UART_MSPDEINIT_CB_ID, USART1_MspDeInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      IsComMspCbValid[COM] = 1U;
    }
  }

  /* BSP status */
  return ret;
}

/**
  * @brief  Register COM1 Msp Callback registering
  * @param  COM COM port to be configured.
  * @param  Callbacks     pointer to COM MspInit/MspDeInit callback functions
  * @retval BSP status
  */
int32_t BSP_COM_RegisterMspCallbacks(COM_TypeDef COM, BSP_COM_Cb_t *Callback)
{
  int32_t ret = BSP_ERROR_NONE;

  if (COM >= COMn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    __HAL_UART_RESET_HANDLE_STATE(&hcom_uart[COM]);

    /* Register MspInit/MspDeInit Callbacks */
    if (HAL_UART_RegisterCallback(&hcom_uart[COM], HAL_UART_MSPINIT_CB_ID, Callback->pMspInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_UART_RegisterCallback(&hcom_uart[COM], HAL_UART_MSPDEINIT_CB_ID, Callback->pMspDeInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      IsComMspCbValid[COM] = 1U;
    }
  }

  /* BSP status */
  return ret;
}
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */

#if (USE_COM_LOG > 0)
/**
  * @brief  Select the active COM port.
  * @param  COM COM port to be activated.
  *          This parameter can be COM1 or/and COM2
  * @retval BSP status
  */
int32_t BSP_COM_SelectLogPort(COM_TypeDef COM)
{
  if (COM_ActiveLogPort != COM)
  {
    COM_ActiveLogPort = COM;
  }
  return BSP_ERROR_NONE;
}

#ifdef __GNUC__
int __io_putchar(int ch)
#else
int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
{
  (void)HAL_UART_Transmit(&hcom_uart [COM_ActiveLogPort], (uint8_t *) &ch, 1, COM_POLL_TIMEOUT);
  return ch;
}
#endif /* USE_COM_LOG */
#endif /* USE_BSP_COM_FEATURE */

#if (USE_BSP_POT_FEATURE > 0)
/**
  * @brief  Init Potentiometer.
  * @param  POT  Potentiometer to be activated.
  * @retval BSP status
  */
int32_t BSP_POT_Init(POT_TypeDef POT)
{
  int32_t ret = BSP_ERROR_NONE;

  if (POT >= POTn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 0)
    /* Init the ADC Msp */
    ADC1_MspInit(&hpot_adc[POT]);
#else
    if (IsPotMspCbValid[POT] == 0U)
    {
      if (BSP_POT_RegisterDefaultMspCallbacks(POT) != BSP_ERROR_NONE)
      {
        return BSP_ERROR_MSP_FAILURE;
      }
    }
#endif
    if (MX_ADC1_Init(&hpot_adc[POT]) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  De-Initialize the Potentiometer.
  * @param  POT  Potentiometer to be activated.
  * @retval BSP status
  */
int32_t BSP_POT_DeInit(POT_TypeDef POT)
{
  int32_t ret = BSP_ERROR_NONE;

  if (POT >= POTn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* ADC configuration */
    hpot_adc[POT].Instance = POT1_ADC;

#if (USE_HAL_ADC_REGISTER_CALLBACKS == 0)
    ADC1_MspDeInit(&hpot_adc[POT]);

#endif /* (USE_HAL_ADC_REGISTER_CALLBACKS == 0) */

    if (HAL_ADC_DeInit(&hpot_adc[POT]) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  Get Potentiometer level in 12 bits.
  * @param  POT Potentiometer instance
  * @retval Potentiometer level(0..100%) / 0xFFFFFFFF : Error
  */
int32_t BSP_POT_GetLevel(POT_TypeDef POT)
{
  int32_t ret = BSP_ERROR_PERIPH_FAILURE;

  if (POT >= POTn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (HAL_ADC_Start(&hpot_adc[POT]) == HAL_OK)
    {
      if (HAL_ADC_PollForConversion(&hpot_adc[POT], POT_ADC_POLL_TIMEOUT) == HAL_OK)
      {
        if (HAL_ADC_GetValue(&hpot_adc[POT]) <= (uint32_t)0xFFF)
        {
          ret = (int32_t)POT_CONVERT2PERC((uint16_t)HAL_ADC_GetValue(&hpot_adc[POT]));
        }
      }
    }
  }

  return ret;
}

/**
  * @brief  Configures ADC peripheral
  * @param  hadc ADC handle
  * @retval HAL error code
  */
__weak HAL_StatusTypeDef MX_ADC1_Init(ADC_HandleTypeDef *hadc)
{
  HAL_StatusTypeDef ret = HAL_OK;
  ADC_ChannelConfTypeDef sConfig;

  /* ADC configuration */
  hadc->Instance                   = POT1_ADC;
  hadc->Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc->Init.Resolution            = ADC_RESOLUTION_12B;
  hadc->Init.ScanConvMode          = (uint32_t)DISABLE;
#if defined(ADC_EOC_SINGLE_CONV)
  hadc->Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
#endif
  hadc->Init.ContinuousConvMode    = DISABLE;
  hadc->Init.NbrOfConversion       = 1;
  hadc->Init.DiscontinuousConvMode = DISABLE;
  hadc->Init.NbrOfDiscConversion   = 0;
  hadc->Init.ExternalTrigConv      = ADC_EXTERNALTRIG_T1_CC1;
  hadc->Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;

  if (HAL_ADC_Init(hadc) != HAL_OK)
  {
    ret =  HAL_ERROR;
  }
  else
  {
    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
      */
    sConfig.Channel      = POT1_ADC_CHANNEL;
    sConfig.Rank         = 1;
    sConfig.SamplingTime = POT1_SAMPLING_TIME;
    sConfig.Offset       = 0;
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
  * @brief Register Default Potentiometer Msp Callbacks
  * @param POT Potentiometer instance
  * @retval BSP status
  */
int32_t BSP_POT_RegisterDefaultMspCallbacks(POT_TypeDef POT)
{
  int32_t ret = BSP_ERROR_NONE;

  if (POT >= POTn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    __HAL_ADC_RESET_HANDLE_STATE(&hpot_adc[POT]);

    /* Register default MspInit/MspDeInit Callback */
    if (HAL_ADC_RegisterCallback(&hpot_adc[POT], HAL_ADC_MSPINIT_CB_ID, ADC1_MspInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_ADC_RegisterCallback(&hpot_adc[POT], HAL_ADC_MSPDEINIT_CB_ID, ADC1_MspDeInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      IsPotMspCbValid[POT] = 1U;
    }
  }

  /* BSP status */
  return ret;
}

/**
  * @brief Register Potentiometer Msp Callback registering
  * @param POT Potentiometer instance
  * @param Callbacks     pointer to Potentiometer MspInit/MspDeInit callback functions
  * @retval BSP status
  */
int32_t BSP_POT_RegisterMspCallbacks(POT_TypeDef POT, BSP_POT_Cb_t *Callback)
{
  int32_t ret = BSP_ERROR_NONE;

  if (POT >= POTn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    __HAL_ADC_RESET_HANDLE_STATE(&hpot_adc[POT]);

    /* Register MspInit/MspDeInit Callbacks */
    if (HAL_ADC_RegisterCallback(&hpot_adc[POT], HAL_ADC_MSPINIT_CB_ID, Callback->pMspInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if (HAL_ADC_RegisterCallback(&hpot_adc[POT], HAL_ADC_MSPDEINIT_CB_ID, Callback->pMspDeInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      IsPotMspCbValid[POT] = 1U;
    }
  }

  /* BSP status */
  return ret;
}

#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */
#endif /* USE_BSP_POT_FEATURE */

#if (USE_BSP_IO_CLASS > 0)
/**
  * @brief  Configures joystick GPIO and EXTI modes.
  * @param  JOY Joystick to be initialized
  * @param  JoyMode Button mode.
  *          This parameter can be one of the following values:
  *            @arg  JOY_MODE_GPIO: Joystick pins will be used as simple IOs
  *            @arg  JOY_MODE_EXTI: Joystick pins will be connected to EXTI line
  *                                 with interrupt generation capability
  * @param  JoyPins joystick pins to be initialized
  * @retval BSP status
  */
int32_t BSP_JOY_Init(JOY_TypeDef JOY, JOYMode_TypeDef JoyMode, JOYPin_TypeDef JoyPins)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t joykey, key_pressed;
  GPIO_InitTypeDef gpio_init_structure;

  static BSP_EXTI_LineCallback JoyCallback[JOY_KEY_NUMBER] = {JOY1_SEL_EXTI_Callback,
                                                              JOY1_DOWN_EXTI_Callback,
                                                              JOY1_LEFT_EXTI_Callback,
                                                              JOY1_RIGHT_EXTI_Callback,
                                                              JOY1_UP_EXTI_Callback
                                                             };
  static const uint32_t JOY_EXTI_LINE[JOY_KEY_NUMBER]   = {JOY1_SEL_EXTI_LINE,
                                                           JOY1_DOWN_EXTI_LINE,
                                                           JOY1_LEFT_EXTI_LINE,
                                                           JOY1_RIGHT_EXTI_LINE,
                                                           JOY1_UP_EXTI_LINE
                                                          };
  static uint32_t  BSP_JOY_PRIO [JOY_KEY_NUMBER] = {BSP_JOY1_SEL_IT_PRIORITY,
                                                    BSP_JOY1_DOWN_IT_PRIORITY,
                                                    BSP_JOY1_LEFT_IT_PRIORITY,
                                                    BSP_JOY1_RIGHT_IT_PRIORITY,
                                                    BSP_JOY1_UP_IT_PRIORITY
                                                   };
  static const IRQn_Type JOY1_IRQn[JOY_KEY_NUMBER] = {JOY1_SEL_EXTI_IRQn,
                                                      JOY1_DOWN_EXTI_IRQn,
                                                      JOY1_LEFT_EXTI_IRQn,
                                                      JOY1_RIGHT_EXTI_IRQn,
                                                      JOY1_UP_EXTI_IRQn
                                                     };

  /* Store Joystick pins initialized */
  JoyPinsMask |= (uint32_t)JoyPins;

  /* Initialized the Joystick. */
  for (joykey = 0U; joykey < JOY_KEY_NUMBER ; joykey++)
  {
    key_pressed = 1UL << joykey;
    if ((key_pressed & (uint32_t)JoyPins) == key_pressed)
    {
      if (JOY == JOY1)
      {
        /* Enable the JOY clock */
        if (key_pressed == (uint32_t)JOY_SEL)
        {
          JOY1_SEL_GPIO_CLK_ENABLE();
        }
        else if (key_pressed == (uint32_t)JOY_DOWN)
        {
          JOY1_DOWN_GPIO_CLK_ENABLE();
        }
        else if (key_pressed == (uint32_t) JOY_LEFT)
        {
          JOY1_LEFT_GPIO_CLK_ENABLE();
        }
        else if (key_pressed == (uint32_t) JOY_RIGHT)
        {
          JOY1_RIGHT_GPIO_CLK_ENABLE();
        }
        else
        {
          if (key_pressed == (uint32_t) JOY_UP)
          {
            JOY1_UP_GPIO_CLK_ENABLE();
          }
        }

        gpio_init_structure.Pin = JOY1_PIN[joykey];
        gpio_init_structure.Pull = GPIO_NOPULL;
        gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

        if (JoyMode == JOY_MODE_GPIO)
        {
          /* Configure Joy pin as input */
          gpio_init_structure.Mode = GPIO_MODE_INPUT;
          HAL_GPIO_Init(JOY1_PORT[joykey], &gpio_init_structure);
        }
        else /*(JoyMode == JOY_MODE_EXTI)*/
        {
          /* Configure Joy pin as input with External interrupt */
          gpio_init_structure.Mode = GPIO_MODE_IT_RISING;
          HAL_GPIO_Init(JOY1_PORT[joykey], &gpio_init_structure);

          (void)HAL_EXTI_GetHandle(&hExtiJoyHandle[joykey], JOY_EXTI_LINE[joykey]);
          (void)HAL_EXTI_RegisterCallback(&hExtiJoyHandle[joykey],  HAL_EXTI_COMMON_CB_ID, JoyCallback[joykey]);

          /* Enable and set Joy EXTI Interrupt to the lowest priority */
          HAL_NVIC_SetPriority((IRQn_Type)(JOY1_IRQn[joykey]), BSP_JOY_PRIO[joykey], 0x00);
          HAL_NVIC_EnableIRQ((IRQn_Type)(JOY1_IRQn[joykey]));
        }
      }
    }
  }

  return ret;
}


/**
  * @brief  DeInit joystick GPIOs.
  * @param  JOY instance
  * @note   JOY DeInit does not disable the MFX, just set the MFX pins in Off mode
  * @param  JoyPins joystick pins to be de-initialized
  * @retval BSP status
  */
int32_t BSP_JOY_DeInit(JOY_TypeDef JOY, JOYPin_TypeDef JoyPins)
{
  uint32_t joykey, key_pressed;

  /* Store Joystick pins initialized */
  JoyPinsMask &= (uint32_t)JoyPins;

  /* Initialized the Joystick. */
  for (joykey = 0U; joykey < JOY_KEY_NUMBER ; joykey++)
  {
    key_pressed = 1UL << joykey;
    if ((key_pressed & (uint32_t)JoyPins) == key_pressed)
    {
      if (JOY == JOY1)
      {
        /* Disable the JOY clock */
        switch (key_pressed)
        {
          case JOY_SEL:
            JOY1_SEL_GPIO_CLK_DISABLE();
            break;

          case JOY_DOWN:
            JOY1_DOWN_GPIO_CLK_DISABLE();
            break;

          case JOY_LEFT:
            JOY1_LEFT_GPIO_CLK_DISABLE();
            break;

          case JOY_RIGHT:
            JOY1_RIGHT_GPIO_CLK_DISABLE();
            break;

          case JOY_UP:
            JOY1_UP_GPIO_CLK_DISABLE();
            break;
          default:
            break;
        }
        HAL_GPIO_DeInit(JOY1_PORT[joykey], JOY1_PIN[joykey]);
      }
    }
  }
  return BSP_ERROR_NONE;
}

/**
  * @brief  This function handles JOY keys interrupt request.
  * @param  JOY Joystick instance
  * @param  JoyPin Specifies the pin connected EXTI handler
  * @retval None
  */
void BSP_JOY_IRQHandler(JOY_TypeDef JOY, JOYPin_TypeDef JoyPin)
{
  if (JOY == JOY1)
  {
    HAL_EXTI_IRQHandler(&hExtiJoyHandle[POSITION_VAL((uint32_t)JoyPin)]);
  }
}

/**
  * @brief  BSP Joystick Callback.
  * @param  JOY Joystick instance
  * @param  JoyPin Specifies the pin connected EXTI line
  * @retval None.
  */
__weak void BSP_JOY_Callback(JOY_TypeDef JOY, uint32_t JoyPin)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(JOY);
  UNUSED(JoyPin);

  /* This function should be implemented by the user application.
     It is called into this driver when an event on JoyPin is triggered. */
}

/**
  * @brief  Returns the current joystick status.
  * @param  JOY Joystick instance
  * @retval BSP error code if value is negative or one of the following values:
  *            @arg  JOY_NONE
  *            @arg  JOY_SEL
  *            @arg  JOY_DOWN
  *            @arg  JOY_LEFT
  *            @arg  JOY_RIGHT
  *            @arg  JOY_UP
  */
int32_t BSP_JOY_GetState(JOY_TypeDef JOY)
{
  uint32_t joykey, key_pressed;
  for (joykey = 0U; joykey < JOY_KEY_NUMBER ; joykey++)
  {
    key_pressed = 1UL << joykey;
    if ((key_pressed & JoyPinsMask) == key_pressed)
    {
      if (JOY == JOY1)
      {
        if (HAL_GPIO_ReadPin(JOY1_PORT[joykey], JOY1_PIN[joykey]) != GPIO_PIN_SET)
        {
          /* Return Code Joystick key pressed */
          return (int32_t)key_pressed;
        }
      }
    }
  }
  /* No Joystick key pressed */
  return (int32_t)JOY_NONE;
}
/**
  * @}
  */

/** @defgroup STM32H7B3I_EVAL_LOW_LEVEL_Private_Functions LOW LEVEL Private Functions
  * @{
  */
/**
  * @brief  JOY1 UP EXTI line detection callbacks.
  * @retval None
  */
static void JOY1_UP_EXTI_Callback(void)
{
  BSP_JOY_Callback(JOY1, JOY1_UP_PIN);
}

/**
  * @brief  JOY1 DOWN EXTI line detection callbacks.
  * @retval None
  */
static void JOY1_DOWN_EXTI_Callback(void)
{
  BSP_JOY_Callback(JOY1, JOY1_DOWN_PIN);
}

/**
  * @brief  JOY1 SEL EXTI line detection callbacks.
  * @retval None
  */
static void JOY1_SEL_EXTI_Callback(void)
{
  BSP_JOY_Callback(JOY1, JOY1_SEL_PIN);
}

/**
  * @brief  JOY1 LEFT EXTI line detection callbacks.
  * @retval None
  */
static void JOY1_LEFT_EXTI_Callback(void)
{
  BSP_JOY_Callback(JOY1, JOY1_LEFT_PIN);
}

/**
  * @brief  JOY1 RIGHT EXTI line detection callbacks.
  * @retval None
  */
static void JOY1_RIGHT_EXTI_Callback(void)
{
  BSP_JOY_Callback(JOY1, JOY1_RIGHT_PIN);
}
#endif /* USE_BSP_IO_CLASS */


/**
  * @brief  WAKEUP EXTI line detection callbacks.
  * @retval None
  */
static void BUTTON_WAKEUP_EXTI_Callback(void)
{
  BSP_PB_Callback(BUTTON_WAKEUP);
}

/**
  * @brief  TAMPER EXTI line detection callbacks.
  * @retval None
  */
static void BUTTON_TAMPER_EXTI_Callback(void)
{
  BSP_PB_Callback(BUTTON_TAMPER);
}

/**
  * @brief  KEY EXTI line detection callbacks.
  * @retval None
  */
static void BUTTON_USER_EXTI_Callback(void)
{
  BSP_PB_Callback(BUTTON_USER);
}

#if (USE_BSP_COM_FEATURE > 0)
/**
  * @brief  Initializes UART MSP.
  * @param  huart UART handle
  * @retval None
  */
static void USART1_MspInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef gpio_init_structure;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);

  /* Enable GPIO clock */
  COM1_TX_GPIO_CLK_ENABLE();
  COM1_RX_GPIO_CLK_ENABLE();

  /* Enable USART clock */
  COM1_CLK_ENABLE();

  /* Configure USART Tx as alternate function */
  gpio_init_structure.Pin = COM1_TX_PIN;
  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio_init_structure.Pull = GPIO_PULLUP;
  gpio_init_structure.Alternate = COM1_TX_AF;
  HAL_GPIO_Init(COM1_TX_GPIO_PORT, &gpio_init_structure);

  /* Configure USART Rx as alternate function */
  gpio_init_structure.Pin = COM1_RX_PIN;
  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Alternate = COM1_RX_AF;
  HAL_GPIO_Init(COM1_RX_GPIO_PORT, &gpio_init_structure);
}

/**
  * @brief  Initialize USART6 Msp part
  * @param  huart UART handle
  * @retval None
  */
static void USART1_MspDeInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef          gpio_init_structure;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);

  /* COM GPIO pin configuration */
  gpio_init_structure.Pin  = COM1_TX_PIN;
  HAL_GPIO_DeInit(COM1_TX_GPIO_PORT, gpio_init_structure.Pin);

  gpio_init_structure.Pin  = COM1_RX_PIN;
  HAL_GPIO_DeInit(COM1_RX_GPIO_PORT, gpio_init_structure.Pin);

  /* Disable USART clock */
  COM1_CLK_DISABLE();
}
#endif /* USE_BSP_COM_FEATURE */

#if (USE_BSP_POT_FEATURE > 0)
/**
  * @brief  Initialize POT1 Msp part
  * @param  hadc ADC handle
  * @retval None
  */
static void ADC1_MspInit(ADC_HandleTypeDef *hadc)
{
  GPIO_InitTypeDef          gpio_init_structure;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);

  POT1_CLK_ENABLE();
  POT1_CHANNEL_GPIO_CLK_ENABLE();

  /* ADC Channel GPIO pin configuration */
  gpio_init_structure.Pin  = POT1_CHANNEL_GPIO_PIN;
  gpio_init_structure.Mode = GPIO_MODE_ANALOG;
  gpio_init_structure.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(POT1_CHANNEL_GPIO_PORT, &gpio_init_structure);
}

/**
  * @brief  De-Initialize POT1 Msp part
  * @param  hadc ADC handle
  * @retval None
  */
static void ADC1_MspDeInit(ADC_HandleTypeDef *hadc)
{
  GPIO_InitTypeDef          gpio_init_structure;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);

  /* POT GPIO pin configuration */
  gpio_init_structure.Pin  = POT1_CHANNEL_GPIO_PIN;
  HAL_GPIO_DeInit(POT1_CHANNEL_GPIO_PORT, gpio_init_structure.Pin);

  /* Disable ADC clock */
  POT1_CLK_DISABLE();
}
#endif /* USE_BSP_POT_FEATURE */

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
