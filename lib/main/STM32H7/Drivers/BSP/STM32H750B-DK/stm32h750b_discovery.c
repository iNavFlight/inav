/**
  ******************************************************************************
  * @file    stm32h750b_discovery.c
  * @author  MCD Application Team
  * @brief   This file provides a set of firmware functions to manage
  *          LEDs
  *          push-buttons
  *          COM ports
  *          available on STM32H750B-DK board(MB1381) from STMicroelectronics.
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

/* Includes ------------------------------------------------------------------*/
#include "stm32h750b_discovery.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H750B_DK
  * @{
  */

/** @defgroup STM32H750B_DK_LOW_LEVEL LOW LEVEL
  * @{
  */

/** @defgroup STM32H750B_DK_LOW_LEVEL_Private_TypesDefinitions Private Types Definitions
  * @{
  */
typedef void (* BSP_EXTI_LineCallback) (void);
/**
  * @}
  */
/** @defgroup STM32H750B_DK_LOW_LEVEL_Private_FunctionPrototypes Private Function Prototypes
  * @{
  */
static void BUTTON_USER_EXTI_Callback(void);
#if (USE_BSP_COM_FEATURE > 0)
static void USART3_MspInit(UART_HandleTypeDef *huart);
static void USART3_MspDeInit(UART_HandleTypeDef *huart);
#endif
/**
  * @}
  */

/** @defgroup STM32H750B_DK_LOW_LEVEL_Exported_Variables Exported Variables
  * @{
  */
EXTI_HandleTypeDef hpb_exti[BUTTONn];
#if (USE_BSP_COM_FEATURE > 0)
UART_HandleTypeDef hcom_uart[COMn];
USART_TypeDef* COM_USART[COMn]   = {COM1_UART};
#endif
/**
  * @}
  */

/** @defgroup STM32H750B_DK_LOW_LEVEL_Private_Variables Private Variables
  * @{
  */

static GPIO_TypeDef* LED_PORT[LEDn] = {LED1_GPIO_PORT,
                                       LED2_GPIO_PORT};

static const  uint32_t LED_PIN[LEDn] = {LED1_PIN,
                                        LED2_PIN};

static GPIO_TypeDef* BUTTON_PORT[BUTTONn] = {BUTTON_USER_GPIO_PORT };

static const uint16_t BUTTON_PIN[BUTTONn] = {BUTTON_USER_PIN };

static const IRQn_Type BUTTON_IRQn[BUTTONn] = {BUTTON_USER_EXTI_IRQn };

#if (USE_COM_LOG == 1)
static COM_TypeDef COM_ActiveLogPort = COM1;
#endif
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
static uint32_t IsComMspCbValid[COMn] = {0};
#endif

/**
  * @}
  */

/** @defgroup STM32H750B_DK_LOW_LEVEL_Exported_Functions Exported Functions
  * @{
  */

/**
  * @brief  This method returns the STM32H750B Discovery BSP Driver revision
  * @retval version: 0xXYZR (8bits for each decimal, R for RC)
  */
int32_t BSP_GetVersion(void)
{
  return (int32_t)STM32H750B_DK_BSP_VERSION;
}

/**
  * @brief  This method returns the board name
  * @retval pointer to the board name string
  */
const uint8_t* BSP_GetBoardName(void)
{
  return (uint8_t *)STM32H750B_DK_BSP_BOARD_NAME;
}

/**
  * @brief  This method returns the board ID
  * @retval pointer to the board name string
  */
const uint8_t* BSP_GetBoardID(void)
{
  return (uint8_t *)STM32H750B_DK_BSP_BOARD_ID;
}

/**
  * @brief  Configures LED on GPIO.
  * @param  Led LED to be configured.
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  * @retval BSP status
  */
int32_t  BSP_LED_Init(Led_TypeDef Led)
{
  int32_t ret = BSP_ERROR_NONE;
  GPIO_InitTypeDef  gpio_init_structure;

  /* Enable the GPIO_LED clock */
  if (Led == LED1)
  {
    LED1_GPIO_CLK_ENABLE() ;
  }
  else if (Led == LED2)
  {
    LED2_GPIO_CLK_ENABLE() ;
  }
  else
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }

  /* configure the GPIO_LED pin */
  gpio_init_structure.Pin = LED_PIN [Led];
  gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_init_structure.Pull = GPIO_PULLUP;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init (LED_PORT [Led], &gpio_init_structure);

  /* By default, turn off LED */
  HAL_GPIO_WritePin (LED_PORT [Led], (uint16_t)LED_PIN [Led], GPIO_PIN_SET);
  return ret;
  }


/**
  * @brief  DeInit LEDs.
  * @param  Led: LED to be configured.
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  * @note Led DeInit does not disable the GPIO clock
  * @retval BSP status
  */
int32_t BSP_LED_DeInit(Led_TypeDef Led)
{
  int32_t ret = BSP_ERROR_NONE;
  GPIO_InitTypeDef  gpio_init_structure;

  /* DeInit the GPIO_LED pin */
  gpio_init_structure.Pin = LED_PIN [Led];
  /* Turn off LED */
  HAL_GPIO_WritePin (LED_PORT [Led], (uint16_t)LED_PIN[Led], GPIO_PIN_SET);
  HAL_GPIO_DeInit (LED_PORT [Led], gpio_init_structure.Pin);
  return ret;
}

/**
  * @brief  Turns selected LED On.
  * @param  Led  LED to be set on
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  * @retval BSP status
  */
int32_t BSP_LED_On(Led_TypeDef Led)
{
  int32_t ret = BSP_ERROR_NONE;

  HAL_GPIO_WritePin (LED_PORT [Led], (uint16_t)LED_PIN [Led], GPIO_PIN_RESET);
  return ret;
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led  LED to be set off
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  * @retval BSP status
  */
int32_t BSP_LED_Off(Led_TypeDef Led)
{
  int32_t ret = BSP_ERROR_NONE;
  HAL_GPIO_WritePin (LED_PORT [Led], (uint16_t)LED_PIN [Led], GPIO_PIN_SET);
  return ret;
}

/**
  * @brief  Toggles the selected LED.
  * @param  Led  LED to be toggled
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  * @retval BSP status
  */
int32_t BSP_LED_Toggle(Led_TypeDef Led)
{
  int32_t ret = BSP_ERROR_NONE;
  HAL_GPIO_TogglePin(LED_PORT[Led], (uint16_t)LED_PIN[Led]);
  return ret;
}
/**
  * @brief  Get the selected LED state.
  * @param  Led LED to be get its state
  *          This parameter can be one of the following values:
  *            @arg  LED1
  *            @arg  LED2
  * @retval LED status
  */
 int32_t BSP_LED_GetState (Led_TypeDef Led)
  {
      int32_t ret;
      ret = (int32_t)HAL_GPIO_ReadPin (LED_PORT [Led], (uint16_t)LED_PIN [Led]);
      return ret;
  }
/**
  * @brief  Configures button GPIO and EXTI Line.
  * @param  Button Button to be configured
  *          This parameter can be one of the following values:
  *            @arg  BUTTON_USER: User Push Button
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
  static BSP_EXTI_LineCallback ButtonCallback[BUTTONn] ={BUTTON_USER_EXTI_Callback};
  static uint32_t  BSP_BUTTON_PRIO [BUTTONn] = {BSP_BUTTON_USER_IT_PRIORITY};
  static const uint32_t BUTTON_EXTI_LINE[BUTTONn] ={BUTTON_USER_EXTI_LINE};
  /* Enable the BUTTON clock*/
  BUTTON_USER_GPIO_CLK_ENABLE();
  gpio_init_structure.Pin = BUTTON_PIN [Button];
  gpio_init_structure.Pull = GPIO_PULLDOWN;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;

  if(ButtonMode == BUTTON_MODE_GPIO)
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
  * @brief  KEY EXTI line detection callbacks.
  * @retval None
  */
static void BUTTON_USER_EXTI_Callback(void)
{
  BSP_PB_Callback(BUTTON_USER);
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
  *          This parameter can be COM1
  * @param  COM_Init Pointer to a UART_HandleTypeDef structure that contains the
  *                configuration information for the specified USART peripheral.
  * @retval BSP status
  */
int32_t BSP_COM_Init(COM_TypeDef COM, COM_InitTypeDef *COM_Init)
{
  int32_t ret = BSP_ERROR_NONE;

  if(COM >= COMn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Init the UART Msp */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 0)
    USART3_MspInit(&hcom_uart[COM]);
#else
    if(IsComMspCbValid[COM] == 0U)
    {
      if(BSP_COM_RegisterDefaultMspCallbacks(COM) != BSP_ERROR_NONE)
      {
        return BSP_ERROR_MSP_FAILURE;
      }
    }
#endif

    if(MX_USART3_Init(&hcom_uart[COM], COM_Init) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  DeInit COM port.
  * @param  COM COM port to be configured.
  *          This parameter can be COM1
  * @retval BSP status
  */
int32_t BSP_COM_DeInit(COM_TypeDef COM)
{
  int32_t ret = BSP_ERROR_NONE;

  if(COM >= COMn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* USART configuration */
    hcom_uart[COM].Instance = COM_USART[COM];

#if (USE_HAL_UART_REGISTER_CALLBACKS == 0)
    USART3_MspDeInit(&hcom_uart[COM]);
#endif /* (USE_HAL_UART_REGISTER_CALLBACKS == 0) */

    if(HAL_UART_DeInit(&hcom_uart[COM]) != HAL_OK)
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
__weak HAL_StatusTypeDef MX_USART3_Init(UART_HandleTypeDef *huart, MX_UART_InitTypeDef *COM_Init)
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

  if(COM >= COMn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    __HAL_UART_RESET_HANDLE_STATE(&hcom_uart[COM]);

    /* Register default MspInit/MspDeInit Callback */
    if(HAL_UART_RegisterCallback(&hcom_uart[COM], HAL_UART_MSPINIT_CB_ID, USART3_MspInit) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_UART_RegisterCallback(&hcom_uart[COM], HAL_UART_MSPDEINIT_CB_ID, USART3_MspDeInit) != HAL_OK)
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

  if(COM >= COMn)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    __HAL_UART_RESET_HANDLE_STATE(&hcom_uart[COM]);

    /* Register MspInit/MspDeInit Callbacks */
    if(HAL_UART_RegisterCallback(&hcom_uart[COM], HAL_UART_MSPINIT_CB_ID, Callback->pMspInitCb) != HAL_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else if(HAL_UART_RegisterCallback(&hcom_uart[COM], HAL_UART_MSPDEINIT_CB_ID, Callback->pMspDeInitCb) != HAL_OK)
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

#if (USE_COM_LOG == 1)
/**
  * @brief  Select the active COM port.
  * @param  COM COM port to be activated.
  *          This parameter can be COM1
  * @retval BSP status
  */
int32_t BSP_COM_SelectLogPort(COM_TypeDef COM)
{
  if(COM_ActiveLogPort != COM)
  {
    COM_ActiveLogPort = COM;
  }
  return BSP_ERROR_NONE;
}

 #ifdef __GNUC__
 int __io_putchar (int ch)
 #else
 int fputc (int ch, FILE *f)
 #endif /* __GNUC__ */
{
  HAL_UART_Transmit (&hcom_uart [COM_ActiveLogPort], (uint8_t *) &ch, 1, COM_POLL_TIMEOUT);
  return ch;
}
#endif /* USE_COM_LOG */
#endif
/**
  * @}
  */

/** @defgroup STM32H7B3I_DISCO_LOW_LEVEL_Private_Functions Private Functions
  * @{
  */
#if (USE_BSP_COM_FEATURE > 0)
/**
  * @brief  Initializes UART MSP.
  * @param  huart UART handle
  * @retval None
  */
static void USART3_MspInit(UART_HandleTypeDef *huart)
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
  * @brief  Initialize USART3 Msp part
  * @param  huart UART handle
  * @retval None
  */
static void USART3_MspDeInit(UART_HandleTypeDef *huart)
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
#endif
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
