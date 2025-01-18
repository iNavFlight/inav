/**
  ******************************************************************************
  * @file    stm32h743i_eval.h
  * @author  MCD Application Team
    * @brief   This file contains definitions for STM32H743I_EVAL:
  *          LEDs
  *          push-buttons
  *          Joystick
  *          POT
  *          COM ports
  *          hardware resources.
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

/* IMPORTANT: The following flag must be enabled in stm32h743i_eval_conf.h file */
/* options in order to use STM32H753I EVAL2 board MB1246 Rev E03 : !!!!!!!!!! */
/* #define USE_TS3510_TS_CTRL                  0U */
/* #define USE_EXC7200_TS_CTRL                 0U */
/* #define USE_EXC80W32_TS_CTRL                1U */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32H743I_EVAL_H
#define STM32H743I_EVAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h743i_eval_conf.h"
#include "stm32h743i_eval_errno.h"

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

/** @addtogroup STM32H743I_EVAL
  * @{
  */

/** @defgroup STM32H743I_EVAL_LOW_LEVEL LOW LEVEL
  * @{
  */

/** @defgroup STM32H743I_EVAL_LOW_LEVEL_Exported_Types LOW LEVEL Exported Types
  * @{
  */
typedef enum
{
  LED1 = 0U,
  LED_GREEN = LED1,
  LED2 = 1U,
  LED_ORANGE = LED2,
  LED3 = 2U,
  LED_RED = LED3,
  LED4 = 3U,
  LED_BLUE = LED4,
  LEDn
}Led_TypeDef;

typedef enum
{
  BUTTON_WAKEUP = 0U,
  BUTTON_TAMPER = 1U,
  BUTTON_USER = 2U,
  BUTTONn
}Button_TypeDef;

typedef enum
{
  BUTTON_MODE_GPIO = 0U,
  BUTTON_MODE_EXTI = 1U
}ButtonMode_TypeDef;

#if (USE_BSP_POT_FEATURE > 0)
typedef enum
{
  POT1 = 0U,
  POTn
}POT_TypeDef;

#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
typedef struct
{
  void (* pMspInitCb)(ADC_HandleTypeDef *);
  void (* pMspDeInitCb)(ADC_HandleTypeDef *);
}BSP_POT_Cb_t;
#endif /* (USE_HAL_ADC_REGISTER_CALLBACKS == 1)   */
#endif /* USE_BSP_POT_FEATURE > 0 */

#if (USE_BSP_JOY_FEATURE > 0)
typedef enum
{
  JOY_MODE_GPIO = 0U,
  JOY_MODE_EXTI = 1U
}JOYMode_TypeDef;

typedef enum
{
  JOY1 = 0U,
  JOYn
}JOY_TypeDef;

typedef enum
{
  JOY_NONE  = 0x00U,
  JOY_SEL   = 0x01U,
  JOY_DOWN  = 0x02U,
  JOY_LEFT  = 0x04U,
  JOY_RIGHT = 0x08U,
  JOY_UP    = 0x10U,
  JOY_ALL   = 0x1FU
}JOYPin_TypeDef;
#endif /* (USE_BSP_JOY_FEATURE > 0) */

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
#endif /* (USE_BSP_COM_FEATURE > 0) */

/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_LOW_LEVEL_Exported_Constants LOW LEVEL Exported Constants
  * @{
  */
/**
  * @brief  Define for STM32H743I_EVAL board
  */
#if !defined (USE_STM32H743I_EVAL)
#define USE_STM32H743I_EVAL
#endif
/**
  * @brief STM32H743I EVAL BSP Driver version number
  */
#define STM32H743I_EVAL_BSP_VERSION_MAIN   (0x03) /*!< [31:24] main version   */
#define STM32H743I_EVAL_BSP_VERSION_SUB1   (0x03) /*!< [23:16] sub1 version   */
#define STM32H743I_EVAL_BSP_VERSION_SUB2   (0x03) /*!< [15:8]  sub2 version   */
#define STM32H743I_EVAL_BSP_VERSION_RC     (0x00) /*!< [7:0]  release candidate   */
#define STM32H743I_EVAL_BSP_VERSION        ((STM32H743I_EVAL_BSP_VERSION_MAIN << 24)\
                                            |(STM32H743I_EVAL_BSP_VERSION_SUB1 << 16)\
                                            |(STM32H743I_EVAL_BSP_VERSION_SUB2 << 8 )\
                                            |(STM32H743I_EVAL_BSP_VERSION_RC))

#define STM32H743I_EVAL_BSP_BOARD_NAME  "STM32H743I-EVAL";
#define STM32H743I_EVAL_BSP_BOARD_ID    "MB1246E";

/** @defgroup STM32H743I_EVAL_LOW_LEVEL_LED LOW LEVEL LED
  * @{
  */
#define LED1_GPIO_PORT                   GPIOF
#define LED1_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOF_CLK_ENABLE()
#define LED1_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOF_CLK_DISABLE()
#define LED1_PIN                         GPIO_PIN_10

#define LED3_GPIO_PORT                   GPIOA
#define LED3_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOA_CLK_ENABLE()
#define LED3_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOA_CLK_DISABLE()
#define LED3_PIN                         GPIO_PIN_4

#if (USE_BSP_IO_CLASS > 0)
#define LED2_PIN                         IO_PIN_10
#define LED4_PIN                         IO_PIN_11
/**
  * @}
  */

/** @defgroup STM32H7B3I_EVAL_LOW_LEVEL_JOYSTICK LOW LEVEL JOYSTICK
  * @{
  */
/* Joystick Pins definition   */
#define JOY1_SEL_PIN                   IO_PIN_0
#define JOY1_DOWN_PIN                  IO_PIN_1
#define JOY1_LEFT_PIN                  IO_PIN_2
#define JOY1_RIGHT_PIN                 IO_PIN_3
#define JOY1_UP_PIN                    IO_PIN_4
#define JOY1_ALL_PIN                   (IO_PIN_0 | IO_PIN_1 |  IO_PIN_2 | IO_PIN_3 | IO_PIN_4)
#define JOY1_EXTI_LINE                 EXTI_LINE_8
#define JOY1_EXTI_IRQn                 EXTI15_9_IRQn
#endif /* (USE_BSP_IO_CLASS > 0) */

/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_LOW_LEVEL_BUTTON LOW LEVEL BUTTON
  * @{
  */
/**
  * @brief Wakeup push-button
  */
#define BUTTON_WAKEUP_PIN                   GPIO_PIN_0
#define BUTTON_WAKEUP_GPIO_PORT             GPIOA
#define BUTTON_WAKEUP_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()
#define BUTTON_WAKEUP_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOA_CLK_DISABLE()
#define BUTTON_WAKEUP_EXTI_IRQn             EXTI0_IRQn
#define BUTTON_WAKEUP_EXTI_LINE             EXTI_LINE_0

/**
  * @brief Tamper push-button
  */
#define BUTTON_TAMPER_PIN                    GPIO_PIN_13
#define BUTTON_TAMPER_GPIO_PORT              GPIOC
#define BUTTON_TAMPER_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define BUTTON_TAMPER_GPIO_CLK_DISABLE()     __HAL_RCC_GPIOC_CLK_DISABLE()
#define BUTTON_TAMPER_EXTI_IRQn              EXTI15_10_IRQn
#define BUTTON_TAMPER_EXTI_LINE              EXTI_LINE_13
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

#if (USE_BSP_COM_FEATURE > 0)
/** @defgroup STM32H743I_EVAL_LOW_LEVEL_COM LOW LEVEL COM
  * @{
  */
/**
  * @brief Definition for COM port1, connected to USART1
  */
#define COM1_UART                     USART1
#define COM1_CLK_ENABLE()             __HAL_RCC_USART1_CLK_ENABLE()
#define COM1_CLK_DISABLE()            __HAL_RCC_USART1_CLK_DISABLE()

#define COM1_TX_PIN                   GPIO_PIN_14
#define COM1_TX_GPIO_PORT             GPIOB
#define COM1_TX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOB_CLK_ENABLE()
#define COM1_TX_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOB_CLK_DISABLE()
#define COM1_TX_AF                    GPIO_AF4_USART1

#define COM1_RX_PIN                   GPIO_PIN_15
#define COM1_RX_GPIO_PORT             GPIOB
#define COM1_RX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOB_CLK_ENABLE()
#define COM1_RX_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOB_CLK_DISABLE()
#define COM1_RX_AF                    GPIO_AF4_USART1
#define COM_POLL_TIMEOUT              1000

#define MX_UART_InitTypeDef COM_InitTypeDef
/**
  * @}
  */
#endif /* (USE_BSP_COM_FEATURE > 0) */

#if (USE_BSP_POT_FEATURE > 0)
/** @defgroup STM32H743I_EVAL_LOW_LEVEL_POT LOW LEVEL POT
  * @{
  */
/**
  * @brief Definition for Potentiometer, connected to ADC1
  */
#define POT1_ADC                        ADC1
#define POT1_CLK_ENABLE()               __HAL_RCC_ADC12_CLK_ENABLE()
#define POT1_CLK_DISABLE()              __HAL_RCC_ADC12_CLK_DISABLE()
#define POT1_CHANNEL_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOA_CLK_ENABLE()
#define POT1_FORCE_RESET()              __HAL_RCC_ADC12_FORCE_RESET()
#define POT1_RELEASE_RESET()            __HAL_RCC_ADC12_RELEASE_RESET()

/* Definition for ADCx Channel Pin */
#define POT1_CHANNEL_GPIO_PIN           GPIO_PIN_0
#define POT1_CHANNEL_GPIO_PORT          GPIOA

/* Definition for ADCx's Channel */
#define POT1_ADC_CHANNEL                ADC_CHANNEL_0
#define POT1_SAMPLING_TIME              ADC_SAMPLETIME_2CYCLES_5
#define POT1_PRESCALER                  ADC_CLOCKPRESCALER_PCLK_DIV4
#define POT_ADC_POLL_TIMEOUT            1000U
#define POT_CONVERT2PERC(x)             ((((int32_t)x) * 100)/(0xFFF))
/**
  * @}
  */
#endif /* (USE_BSP_POT_FEATURE > 0) */
/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_LOW_LEVEL_Exported_Macros LOW LEVEL Exported Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_LOW_LEVEL_Exported_Functions LOW LEVEL Exported Functions
  * @{
  */
int32_t BSP_GetVersion(void);
const uint8_t* BSP_GetBoardName(void);
const uint8_t* BSP_GetBoardID(void);

int32_t BSP_LED_Init(Led_TypeDef Led);
int32_t BSP_LED_DeInit(Led_TypeDef Led);
int32_t BSP_LED_On(Led_TypeDef Led);
int32_t BSP_LED_Off(Led_TypeDef Led);
int32_t BSP_LED_GetState (Led_TypeDef Led);
int32_t BSP_LED_Toggle(Led_TypeDef Led);
int32_t BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode);
int32_t BSP_PB_DeInit(Button_TypeDef Button);
int32_t BSP_PB_GetState(Button_TypeDef Button);
void    BSP_PB_Callback(Button_TypeDef Button);
void    BSP_PB_IRQHandler(Button_TypeDef Button);

#if (USE_BSP_JOY_FEATURE > 0)
int32_t BSP_JOY_Init(JOY_TypeDef JOY, JOYMode_TypeDef JoyMode, JOYPin_TypeDef JoyPins);
int32_t BSP_JOY_DeInit(JOY_TypeDef JOY, JOYPin_TypeDef JoyPins);
int32_t BSP_JOY_GetState(JOY_TypeDef JOY);
void    BSP_JOY_Callback(JOY_TypeDef JOY, JOYPin_TypeDef JoyPin);
void    BSP_JOY_IRQHandler(JOY_TypeDef JOY, JOYPin_TypeDef JoyPin);
#endif /* (USE_BSP_JOY_FEATURE > 0)  */

#if (USE_BSP_POT_FEATURE > 0)
int32_t BSP_POT_Init(POT_TypeDef POT);
int32_t BSP_POT_DeInit(POT_TypeDef POT);
int32_t BSP_POT_GetLevel(POT_TypeDef POT);
HAL_StatusTypeDef MX_ADC1_Init(ADC_HandleTypeDef *hadc);
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
int32_t BSP_POT_RegisterDefaultMspCallbacks(POT_TypeDef POT);
int32_t BSP_POT_RegisterMspCallbacks(POT_TypeDef POT, BSP_POT_Cb_t *Callback);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS   */
#endif /* (USE_BSP_POT_FEATURE > 0)  */

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
HAL_StatusTypeDef MX_USART1_Init(UART_HandleTypeDef *huart, MX_UART_InitTypeDef *COM_Init);
#endif /* (USE_BSP_COM_FEATURE > 0)  */

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

#endif /* STM32H743I_EVAL_H   */
