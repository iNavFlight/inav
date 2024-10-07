/**
  ******************************************************************************
  * @file    stm32h7xx_nucleo.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for:
  *          - LEDs and push-button available on STM32H7xx-Nucleo Kit
  *            from STMicroelectronics
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32H7XX_NUCLEO_H
#define STM32H7XX_NUCLEO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_nucleo_conf.h"
#include "stm32h7xx_nucleo_errno.h"

#if (USE_BSP_COM_FEATURE > 0)
  #if (USE_COM_LOG > 0)
    #ifndef __GNUC__
      #include <stdio.h>
    #endif
  #endif
#endif

/** @addtogroup BSP
  * @{
  */

/** @defgroup STM32H7XX_NUCLEO STM32H7XX_NUCLEO
  * @{
  */

/** @defgroup STM32H7XX_NUCLEO_LOW_LEVEL LOW LEVEL
  * @{
  */

/**
  * @brief Define for STM32H7XX_NUCLEO board
  */
#if !defined (USE_STM32H7XX_NUCLEO)
 #define USE_STM32H7XX_NUCLEO
#endif

#if !defined (USE_NUCLEO_144)
 #error "Board Pin number not defined!! Add USE_NUCLEO_144 define within stm32h7xx_nucleo_conf.h file"
#endif

#if !defined (USE_NUCLEO_H745ZI_Q) && !defined (USE_NUCLEO_H743ZI) && !defined (USE_NUCLEO_H743ZI2) &&\
    !defined (USE_NUCLEO_H7A3ZI_Q) && !defined (USE_NUCLEO_H723ZG)
 #error "Board Part number not defined!! Add one of the following define within stm32h7xx_nucleo_conf.h file:\
         USE_NUCLEO_H745ZI_Q, USE_NUCLEO_H743ZI, USE_NUCLEO_H743ZI2, USE_NUCLEO_H7A3ZI_Q, USE_NUCLEO_H723ZG"
#endif

/** @defgroup STM32H7XX_NUCLEO_LOW_LEVEL_Exported_Types LOW LEVEL Exported Types
  * @{
  */
#if defined (USE_NUCLEO_H745ZI_Q) || defined (USE_NUCLEO_H743ZI2) || defined (USE_NUCLEO_H7A3ZI_Q) || defined (USE_NUCLEO_H723ZG)
typedef enum
{
  LED1 = 0,
  LED_GREEN = LED1,
  LED2 = 1,
  LED_YELLOW = LED2,
  LED3 = 2,
  LED_RED = LED3,
  LEDn
}Led_TypeDef;
#else /* USE_NUCLEO_H743ZI */
typedef enum
{
  LED1 = 0,
  LED_GREEN = LED1,
  LED2 = 1,
  LED_BLUE = LED2,
  LED3 = 2,
  LED_RED = LED3,
  LEDn
}Led_TypeDef;
#endif

typedef enum
{
  BUTTON_USER = 0U,
  BUTTONn
}Button_TypeDef;

typedef enum
{
  BUTTON_MODE_GPIO = 0,
  BUTTON_MODE_EXTI = 1
}ButtonMode_TypeDef;

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
  uint32_t               BaudRate;
  COM_WordLengthTypeDef  WordLength;
  COM_StopBitsTypeDef    StopBits;
  COM_ParityTypeDef      Parity;
  COM_HwFlowCtlTypeDef   HwFlowCtl;
}COM_InitTypeDef;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
typedef struct
{
  void (* pMspInitCb)(UART_HandleTypeDef *);
  void (* pMspDeInitCb)(UART_HandleTypeDef *);
}BSP_COM_Cb_t;
#endif /* (USE_HAL_UART_REGISTER_CALLBACKS == 1) */

#define MX_UART_InitTypeDef COM_InitTypeDef
#endif

/**
  * @}
  */

/** @defgroup STM32H7XX_NUCLEO_LOW_LEVEL_Exported_Constants LOW LEVEL Exported Constants
  * @{
  */

/**
  * @brief STM32H7XX NUCLEO BSP Driver version number V1.3.2
  */
#define STM32H7XX_NUCLEO_BSP_VERSION_MAIN   (0x01U) /*!< [31:24] main version */
#define STM32H7XX_NUCLEO_BSP_VERSION_SUB1   (0x03U) /*!< [23:16] sub1 version */
#define STM32H7XX_NUCLEO_BSP_VERSION_SUB2   (0x02U) /*!< [15:8]  sub2 version */
#define STM32H7XX_NUCLEO_BSP_VERSION_RC     (0x00U) /*!< [7:0]  release candidate */
#define STM32H7XX_NUCLEO_BSP_VERSION        ((STM32H7XX_NUCLEO_BSP_VERSION_MAIN << 24)\
                                            |(STM32H7XX_NUCLEO_BSP_VERSION_SUB1 << 16)\
                                            |(STM32H7XX_NUCLEO_BSP_VERSION_SUB2 << 8 )\
                                            |(STM32H7XX_NUCLEO_BSP_VERSION_RC))

/** @defgroup STM32H7XX_NUCLEO_LOW_LEVEL_LED LOW LEVEL LED
  * @{
  */
#define LEDn                                    3U

#define LED1_PIN                                GPIO_PIN_0
#define LED1_GPIO_PORT                          GPIOB
#define LED1_GPIO_CLK_ENABLE()                  __HAL_RCC_GPIOB_CLK_ENABLE()
#define LED1_GPIO_CLK_DISABLE()                 __HAL_RCC_GPIOB_CLK_DISABLE()

#if defined (USE_NUCLEO_H745ZI_Q) || defined (USE_NUCLEO_H743ZI2) || defined (USE_NUCLEO_H7A3ZI_Q) || defined (USE_NUCLEO_H723ZG)
#define LED2_PIN                                GPIO_PIN_1
#define LED2_GPIO_PORT                          GPIOE
#define LED2_GPIO_CLK_ENABLE()                  __HAL_RCC_GPIOE_CLK_ENABLE()
#define LED2_GPIO_CLK_DISABLE()                 __HAL_RCC_GPIOE_CLK_DISABLE()
#else /* USE_NUCLEO_H743ZI */
#define LED2_PIN                                GPIO_PIN_7
#define LED2_GPIO_PORT                          GPIOB
#define LED2_GPIO_CLK_ENABLE()                  __HAL_RCC_GPIOB_CLK_ENABLE()
#define LED2_GPIO_CLK_DISABLE()                 __HAL_RCC_GPIOB_CLK_DISABLE()
#endif

#define LED3_PIN                                GPIO_PIN_14
#define LED3_GPIO_PORT                          GPIOB
#define LED3_GPIO_CLK_ENABLE()                  __HAL_RCC_GPIOB_CLK_ENABLE()
#define LED3_GPIO_CLK_DISABLE()                 __HAL_RCC_GPIOB_CLK_DISABLE()

/**
  * @}
  */

/** @defgroup STM32H7XX_NUCLEO_LOW_LEVEL_BUTTON LOW LEVEL BUTTON
  * @{
  */
/* Button state */
#define BUTTON_RELEASED                    0U
#define BUTTON_PRESSED                     1U
#define BUTTONn                            1U

/**
 * @brief Key push-button
 */
#define BUTTON_USER_PIN                       GPIO_PIN_13
#define BUTTON_USER_GPIO_PORT                 GPIOC
#define BUTTON_USER_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOC_CLK_ENABLE()
#define BUTTON_USER_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOC_CLK_DISABLE()
#define BUTTON_USER_EXTI_IRQn                 EXTI15_10_IRQn
#define BUTTON_USER_EXTI_LINE                 EXTI_LINE_13

/**
  * @}
  */

/** @defgroup STM32H7XX_NUCLEO_LOW_LEVEL_COM LOW LEVEL COM
  * @{
  */
/**
 * @brief Definition for COM port1, connected to USART3
 */
#if (USE_BSP_COM_FEATURE > 0)

#define COMn                          1U
#define COM1_UART                     USART3
#define COM1_CLK_ENABLE()             __HAL_RCC_USART3_CLK_ENABLE()
#define COM1_CLK_DISABLE()            __HAL_RCC_USART3_CLK_DISABLE()

#define COM1_TX_PIN                   GPIO_PIN_8
#define COM1_TX_GPIO_PORT             GPIOD
#define COM1_TX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOD_CLK_ENABLE()
#define COM1_TX_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOD_CLK_DISABLE()
#define COM1_TX_AF                    GPIO_AF7_USART3

#define COM1_RX_PIN                   GPIO_PIN_9
#define COM1_RX_GPIO_PORT             GPIOD
#define COM1_RX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOD_CLK_ENABLE()
#define COM1_RX_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOD_CLK_DISABLE()
#define COM1_RX_AF                    GPIO_AF7_USART3
#define COM_POLL_TIMEOUT             1000
#endif

/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup STM32H7XX_NUCLEO_LOW_LEVEL_Exported_Variables
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

/** @defgroup STM32H7XX_NUCLEO_LOW_LEVEL_Exported_FunctionsPrototypes LOW LEVEL Exported Functions Prototypes
  * @{
  */
int32_t  BSP_GetVersion(void);
int32_t  BSP_LED_Init(Led_TypeDef Led);
int32_t  BSP_LED_DeInit(Led_TypeDef Led);
int32_t  BSP_LED_On(Led_TypeDef Led);
int32_t  BSP_LED_Off(Led_TypeDef Led);
int32_t  BSP_LED_Toggle(Led_TypeDef Led);
int32_t  BSP_LED_GetState (Led_TypeDef Led);
int32_t  BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode);
int32_t  BSP_PB_DeInit(Button_TypeDef Button);
int32_t  BSP_PB_GetState(Button_TypeDef Button);
void     BSP_PB_IRQHandler(Button_TypeDef Button);
void     BSP_PB_Callback(Button_TypeDef Button);

#if (USE_BSP_COM_FEATURE > 0)
int32_t  BSP_COM_Init(COM_TypeDef COM, COM_InitTypeDef *COM_Init);
int32_t  BSP_COM_DeInit(COM_TypeDef COM);
#if (USE_COM_LOG > 0)
int32_t  BSP_COM_SelectLogPort (COM_TypeDef COM);
#endif

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
int32_t BSP_COM_RegisterDefaultMspCallbacks(COM_TypeDef COM);
int32_t BSP_COM_RegisterMspCallbacks(COM_TypeDef COM , BSP_COM_Cb_t *Callback);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
HAL_StatusTypeDef MX_USART3_Init(UART_HandleTypeDef *huart, MX_UART_InitTypeDef *COM_Init);
#endif /* USE_BSP_COM_FEATURE */

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

#endif /* STM32H7XX_NUCLEO_H */
