/**
  ******************************************************************************
  * @file    stm32h745i_discovery.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for STM32H745I-DISCO:
  *          LEDs
  *          push-buttons
  *          COM ports
  *          hardware resources.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32H745I_DISCO_H
#define STM32H745I_DISCO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h745i_discovery_conf.h"
#include "stm32h745i_discovery_errno.h"

#if (USE_BSP_COM_FEATURE > 0)
  #if (USE_COM_LOG > 0)
    #ifndef __GNUC__
      #include "stdio.h"
    #endif
  #endif
#endif
/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H745I_DISCO
  * @{
  */

/** @addtogroup STM32H745I_DISCO_LOW_LEVEL
  * @{
  */

/** @defgroup STM32H745I_DISCO_LOW_LEVEL_Exported_Types Exported Types
 * @{
 */

typedef enum
{
  LED1 = 0,
  LED_GREEN = LED1,
  LED2 = 1,
  LED_RED = LED2,
  LEDn
} Led_TypeDef;

typedef enum
{
  BUTTON_USER = 0U,
  BUTTONn
} Button_TypeDef;

typedef enum
{
  BUTTON_MODE_GPIO = 0U,
  BUTTON_MODE_EXTI = 1U
} ButtonMode_TypeDef;

#if (USE_BSP_COM_FEATURE > 0)
typedef enum
{
  COM1 = 0U,
  COMn
}COM_TypeDef;

typedef enum
{
  COM_STOPBITS_1     =   UART_STOPBITS_1,
  COM_STOPBITS_2     =   UART_STOPBITS_2,
}COM_StopBitsTypeDef;

typedef enum
{
  COM_PARITY_NONE     =  UART_PARITY_NONE,
  COM_PARITY_EVEN     =  UART_PARITY_EVEN,
  COM_PARITY_ODD      =  UART_PARITY_ODD,
}COM_ParityTypeDef;

typedef enum
{
  COM_HWCONTROL_NONE    =  UART_HWCONTROL_NONE,
  COM_HWCONTROL_RTS     =  UART_HWCONTROL_RTS,
  COM_HWCONTROL_CTS     =  UART_HWCONTROL_CTS,
  COM_HWCONTROL_RTS_CTS =  UART_HWCONTROL_RTS_CTS,
}COM_HwFlowCtlTypeDef;

typedef enum
{
  COM_WORDLENGTH_7B = UART_WORDLENGTH_7B,
  COM_WORDLENGTH_8B = UART_WORDLENGTH_8B,
  COM_WORDLENGTH_9B = UART_WORDLENGTH_9B,
}COM_WordLengthTypeDef;

typedef struct
{
  uint32_t              BaudRate;
  COM_WordLengthTypeDef WordLength;
  COM_StopBitsTypeDef   StopBits;
  COM_ParityTypeDef     Parity;
  COM_HwFlowCtlTypeDef  HwFlowCtl;
}COM_InitTypeDef;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
typedef struct
{
  void (* pMspInitCb)(UART_HandleTypeDef *);
  void (* pMspDeInitCb)(UART_HandleTypeDef *);
}BSP_COM_Cb_t;
#endif /* (USE_HAL_UART_REGISTER_CALLBACKS == 1) */
#endif

/**
  * @}
  */

/** @defgroup STM32H745I_DISCO_LOW_LEVEL_Exported_Constants Exported Constants
  * @{
  */

/**
  * @brief  Define for STM32H745I_DISCO board
  */

#if !defined (USE_STM32H745I_DISCO)
 #define USE_STM32H745I_DISCO
#endif
/**
  * @brief STM32H745I Discovery BSP Driver version number V3.3.2
  */
#define STM32H745I_DISCO_BSP_VERSION_MAIN   (0x03) /*!< [31:24] main version */
#define STM32H745I_DISCO_BSP_VERSION_SUB1   (0x03) /*!< [23:16] sub1 version */
#define STM32H745I_DISCO_BSP_VERSION_SUB2   (0x02) /*!< [15:8]  sub2 version */
#define STM32H745I_DISCO_BSP_VERSION_RC     (0x00) /*!< [7:0]  release candidate */
#define STM32H745I_DISCO_BSP_VERSION        ((STM32H745I_DISCO_BSP_VERSION_MAIN << 24)\
                                            |(STM32H745I_DISCO_BSP_VERSION_SUB1 << 16)\
                                            |(STM32H745I_DISCO_BSP_VERSION_SUB2 << 8 )\
                                            |(STM32H745I_DISCO_BSP_VERSION_RC))
#define STM32H745I_DISCO_BSP_BOARD_NAME  "STM32H745I-DISCO";
#define STM32H745I_DISCO_BSP_BOARD_ID    "MB1381B";

/** @defgroup STM32H745I_DISCO_LOW_LEVEL_LED EVAL LOW LEVEL LED
  * @{
  */
#define LED1_GPIO_PORT                   GPIOJ
#define LED1_PIN                         GPIO_PIN_2

#define LED2_GPIO_PORT                   GPIOI
#define LED2_PIN                         GPIO_PIN_13


#define LED1_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOJ_CLK_ENABLE()
#define LED1_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOJ_CLK_DISABLE()

#define LED2_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOI_CLK_ENABLE()
#define LED2_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOI_CLK_DISABLE()
/**
  * @}
  */

/** @defgroup STM32H745I_DISCO_LOW_LEVEL_BUTTON LOW LEVEL BUTTON
  * @{
  */
/* Button state */
#define BUTTON_RELEASED                    0U
#define BUTTON_PRESSED                     1U

/**
  * @brief USER push-button
  */
#define BUTTON_USER_PIN                   GPIO_PIN_13
#define BUTTON_USER_GPIO_PORT             GPIOC
#define BUTTON_USER_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOC_CLK_ENABLE()
#define BUTTON_USER_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOC_CLK_DISABLE()
#define BUTTON_USER_EXTI_IRQn             EXTI15_10_IRQn
#define BUTTON_USER_EXTI_LINE             EXTI_LINE_13

/**
  * @}
  */

/** @defgroup STM32H745I_DISCO_LOW_LEVEL_COM LOW LEVEL COM
  * @{
  */
#if (USE_BSP_COM_FEATURE > 0)
/**
 * @brief Definition for COM port1, connected to USART3
 */
#define COM1_UART                      USART3
#define COM1_CLK_ENABLE()             __HAL_RCC_USART3_CLK_ENABLE()
#define COM1_CLK_DISABLE()            __HAL_RCC_USART3_CLK_DISABLE()

#define COM1_TX_PIN                   GPIO_PIN_10
#define COM1_TX_GPIO_PORT             GPIOB
#define COM1_TX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOB_CLK_ENABLE()
#define COM1_TX_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOB_CLK_DISABLE()
#define COM1_TX_AF                    GPIO_AF7_USART3

#define COM1_RX_PIN                   GPIO_PIN_11
#define COM1_RX_GPIO_PORT             GPIOB
#define COM1_RX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOB_CLK_ENABLE()
#define COM1_RX_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOB_CLK_DISABLE()
#define COM1_RX_AF                    GPIO_AF7_USART3
#define COM_POLL_TIMEOUT              1000

#define MX_UART_InitTypeDef COM_InitTypeDef
#endif
/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup STM32H745I_DISCO_LOW_LEVEL_Exported_Variables
  * @{
  */
extern EXTI_HandleTypeDef hpb_exti[];
#if (USE_BSP_COM_FEATURE > 0)
extern UART_HandleTypeDef hcom_uart[];
extern USART_TypeDef* COM_USART[];
#endif
/**
  * @}
  */

/** @defgroup STM32H745I_DISCO_LOW_LEVEL_Exported_FunctionsPrototypes Exported Functions Prototypes
  * @{
  */
int32_t BSP_GetVersion(void);
const uint8_t* BSP_GetBoardName(void);
const uint8_t* BSP_GetBoardID(void);

int32_t BSP_LED_Init(Led_TypeDef Led);
int32_t BSP_LED_DeInit(Led_TypeDef Led);
int32_t BSP_LED_On(Led_TypeDef Led);
int32_t BSP_LED_Off(Led_TypeDef Led);
int32_t BSP_LED_Toggle(Led_TypeDef Led);
int32_t BSP_LED_GetState (Led_TypeDef Led) ;
int32_t BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode);
int32_t BSP_PB_DeInit(Button_TypeDef Button);
int32_t  BSP_PB_GetState(Button_TypeDef Button);
void     BSP_PB_Callback(Button_TypeDef Button);
#if (USE_BSP_COM_FEATURE > 0)
int32_t  BSP_COM_Init(COM_TypeDef COM, COM_InitTypeDef *COM_Init);
int32_t  BSP_COM_DeInit(COM_TypeDef COM);
#if (USE_COM_LOG > 0)
int32_t  BSP_COM_SelectLogPort (COM_TypeDef COM);
#endif

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
int32_t BSP_COM_RegisterDefaultMspCallbacks(COM_TypeDef COM);
int32_t BSP_COM_RegisterMspCallbacks(COM_TypeDef COM, BSP_COM_Cb_t *Callback);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS   */
HAL_StatusTypeDef MX_USART3_Init(UART_HandleTypeDef *huart, MX_UART_InitTypeDef *COM_Init);
#endif /* (USE_BSP_COM_FEATURE > 0)  */
void BSP_PB_IRQHandler(Button_TypeDef Button);


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

#endif /* STM32H745I_DISCO_H */
