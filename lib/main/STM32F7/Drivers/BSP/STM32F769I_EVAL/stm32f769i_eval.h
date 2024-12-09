/**
  ******************************************************************************
  * @file    stm32f769i_eval.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for STM32F769I_EVAL LEDs,
  *          push-buttons and COM ports hardware resources.
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
#ifndef __STM32F769I_EVAL_H
#define __STM32F769I_EVAL_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
   
/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32F769I_EVAL
  * @{
  */
      
/** @addtogroup STM32F769I_EVAL_LOW_LEVEL
  * @{
  */ 

/** @defgroup STM32F769I_EVAL_LOW_LEVEL_Exported_Types LOW LEVEL Exported Types
  * @{
  */
typedef enum 
{
LED1 = 0,
LED_GREEN = LED1,
LED2 = 1,
LED_ORANGE = LED2,
LED3 = 2,
LED_RED = LED3,
LED4 = 3,
LED_BLUE = LED4
}Led_TypeDef;

typedef enum 
{  
  BUTTON_WAKEUP = 0,
  BUTTON_TAMPER = 1,
  BUTTON_KEY = 2
}Button_TypeDef;

typedef enum 
{  
  BUTTON_MODE_GPIO = 0,
  BUTTON_MODE_EXTI = 1
}ButtonMode_TypeDef;

#if defined(USE_IOEXPANDER)
typedef enum 
{  
  JOY_MODE_GPIO = 0,
  JOY_MODE_EXTI = 1
}JOYMode_TypeDef;

typedef enum 
{ 
  JOY_NONE  = 0,
  JOY_SEL   = 1,
  JOY_DOWN  = 2,
  JOY_LEFT  = 3,
  JOY_RIGHT = 4,
  JOY_UP    = 5
}JOYState_TypeDef;
#endif /* USE_IOEXPANDER */

typedef enum 
{
  COM1 = 0,
  COM2 = 1
}COM_TypeDef;
/**
  * @}
  */ 

/** @defgroup STM32F769I_EVAL_LOW_LEVEL_Exported_Constants LOW LEVEL Exported Constants
  * @{
  */ 

/** 
  * @brief  Define for STM32F769I_EVAL board
  */ 
#if !defined (USE_STM32F769I_EVAL)
 #define USE_STM32F769I_EVAL
#endif

/** @addtogroup STM32F769I_EVAL_LOW_LEVEL_LED EVAL LOW LEVEL LED
  * @{
  */
#define LEDn                             ((uint32_t)4)

#define LED1_GPIO_PORT                   GPIOI
#define LED1_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOI_CLK_ENABLE()
#define LED1_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOI_CLK_DISABLE()
#define LED1_PIN                         GPIO_PIN_15	

#define LED2_GPIO_PORT                   GPIOJ
#define LED2_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOJ_CLK_ENABLE()
#define LED2_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOJ_CLK_DISABLE()
#define LED2_PIN                         GPIO_PIN_0

#define LED3_GPIO_PORT                   GPIOJ
#define LED3_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOJ_CLK_ENABLE()
#define LED3_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOJ_CLK_DISABLE()
#define LED3_PIN                         GPIO_PIN_1		

#define LED4_GPIO_PORT                   GPIOJ
#define LED4_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOJ_CLK_ENABLE()
#define LED4_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOJ_CLK_DISABLE()
#define LED4_PIN                         GPIO_PIN_3

#define LEDx_GPIO_CLK_ENABLE(__INDEX__)  do{if((__INDEX__) == 0) LED1_GPIO_CLK_ENABLE(); else \
                                            if((__INDEX__) == 1) LED2_GPIO_CLK_ENABLE(); else \
                                            if((__INDEX__) == 2) LED3_GPIO_CLK_ENABLE(); else \
                                            if((__INDEX__) == 3) LED4_GPIO_CLK_ENABLE(); \
                                            }while(0)

#define LEDx_GPIO_CLK_DISABLE(__INDEX__)  do{if((__INDEX__) == 0) LED1_GPIO_CLK_DISABLE(); else \
                                             if((__INDEX__) == 1) LED2_GPIO_CLK_DISABLE(); else \
                                             if((__INDEX__) == 2) LED3_GPIO_CLK_DISABLE(); else \
                                             if((__INDEX__) == 3) LED4_GPIO_CLK_DISABLE(); \
                                             }while(0)

/**
  * @}
  */

/**
  * @brief MFX_IRQOUt pin
  */
#define MFX_IRQOUT_PIN                    GPIO_PIN_8
#define MFX_IRQOUT_GPIO_PORT              GPIOI
#define MFX_IRQOUT_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOI_CLK_ENABLE()
#define MFX_IRQOUT_GPIO_CLK_DISABLE()     __HAL_RCC_GPIOI_CLK_DISABLE()
#define MFX_IRQOUT_EXTI_IRQn              EXTI9_5_IRQn

/** @addtogroup STM32F769I_EVAL_LOW_LEVEL_BUTTON LOW LEVEL BUTTON
  * @{
  */ 
/* Joystick pins are connected to IO Expander (accessible through I2C1 interface) */ 
#define BUTTONn                             ((uint8_t)3) 

/**
  * @brief Wakeup push-button
  */
#define WAKEUP_BUTTON_PIN                   GPIO_PIN_13
#define WAKEUP_BUTTON_GPIO_PORT             GPIOC
#define WAKEUP_BUTTON_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOC_CLK_ENABLE()
#define WAKEUP_BUTTON_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOC_CLK_DISABLE()
#define WAKEUP_BUTTON_EXTI_IRQn             EXTI15_10_IRQn 

/**
  * @brief Tamper push-button
  */
#define TAMPER_BUTTON_PIN                    GPIO_PIN_13
#define TAMPER_BUTTON_GPIO_PORT              GPIOC
#define TAMPER_BUTTON_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define TAMPER_BUTTON_GPIO_CLK_DISABLE()     __HAL_RCC_GPIOC_CLK_DISABLE()
#define TAMPER_BUTTON_EXTI_IRQn              EXTI15_10_IRQn

/**
  * @brief Key push-button
  */
#define KEY_BUTTON_PIN                       GPIO_PIN_13
#define KEY_BUTTON_GPIO_PORT                 GPIOC
#define KEY_BUTTON_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOC_CLK_ENABLE()
#define KEY_BUTTON_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOC_CLK_DISABLE()
#define KEY_BUTTON_EXTI_IRQn                 EXTI15_10_IRQn

#define BUTTONx_GPIO_CLK_ENABLE(__INDEX__)    do { if((__INDEX__) == 0) WAKEUP_BUTTON_GPIO_CLK_ENABLE(); else\
                                                   if((__INDEX__) == 1) TAMPER_BUTTON_GPIO_CLK_ENABLE(); else\
												                        KEY_BUTTON_GPIO_CLK_ENABLE(); } while(0)											   

#define BUTTONx_GPIO_CLK_DISABLE(__INDEX__)    (((__INDEX__) == 0) ? WAKEUP_BUTTON_GPIO_CLK_DISABLE() :\
                                                ((__INDEX__) == 1) ? TAMPER_BUTTON_GPIO_CLK_DISABLE() : KEY_BUTTON_GPIO_CLK_DISABLE())
/**
  * @}
  */ 

/** @addtogroup STM32F769I_EVAL_LOW_LEVEL_COM LOW LEVEL COM
  * @{
  */
#define COMn                             ((uint8_t)1)

/**
 * @brief Definition for COM port1, connected to USART1
 */ 
#define EVAL_COM1                          USART1
#define EVAL_COM1_CLK_ENABLE()             __HAL_RCC_USART1_CLK_ENABLE()
#define EVAL_COM1_CLK_DISABLE()            __HAL_RCC_USART1_CLK_DISABLE()

#define EVAL_COM1_TX_PIN                   GPIO_PIN_9
#define EVAL_COM1_TX_GPIO_PORT             GPIOA
#define EVAL_COM1_TX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()
#define EVAL_COM1_TX_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOA_CLK_DISABLE()
#define EVAL_COM1_TX_AF                    GPIO_AF7_USART1

#define EVAL_COM1_RX_PIN                   GPIO_PIN_10
#define EVAL_COM1_RX_GPIO_PORT             GPIOA
#define EVAL_COM1_RX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()
#define EVAL_COM1_RX_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOA_CLK_DISABLE()
#define EVAL_COM1_RX_AF                    GPIO_AF7_USART1

#define EVAL_COM1_IRQn                     USART1_IRQn

#define EVAL_COMx_CLK_ENABLE(__INDEX__)            do { if((__INDEX__) == COM1) EVAL_COM1_CLK_ENABLE(); } while(0)
#define EVAL_COMx_CLK_DISABLE(__INDEX__)           (((__INDEX__) == 0) ? EVAL_COM1_CLK_DISABLE() : 0)

#define EVAL_COMx_TX_GPIO_CLK_ENABLE(__INDEX__)    do { if((__INDEX__) == COM1) EVAL_COM1_TX_GPIO_CLK_ENABLE(); } while(0)
#define EVAL_COMx_TX_GPIO_CLK_DISABLE(__INDEX__)   (((__INDEX__) == 0) ? EVAL_COM1_TX_GPIO_CLK_DISABLE() : 0)

#define EVAL_COMx_RX_GPIO_CLK_ENABLE(__INDEX__)    do { if((__INDEX__) == COM1) EVAL_COM1_RX_GPIO_CLK_ENABLE(); } while(0)
#define EVAL_COMx_RX_GPIO_CLK_DISABLE(__INDEX__)   (((__INDEX__) == 0) ? EVAL_COM1_RX_GPIO_CLK_DISABLE() : 0)

/**
 * @brief Definition for Potentiometer, connected to ADC3
 */ 
#define ADCx                            ADC3
#define ADCx_CLK_ENABLE()               __HAL_RCC_ADC3_CLK_ENABLE()
#define ADCx_CHANNEL_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOF_CLK_ENABLE()

#define ADCx_FORCE_RESET()              __HAL_RCC_ADC_FORCE_RESET()
#define ADCx_RELEASE_RESET()            __HAL_RCC_ADC_RELEASE_RESET()

/* Definition for ADCx Channel Pin */
#define ADCx_CHANNEL_PIN                GPIO_PIN_10
#define ADCx_CHANNEL_GPIO_PORT          GPIOF

/* Definition for ADCx's Channel */
#define ADCx_CHANNEL                    ADC_CHANNEL_8
#define SAMPLINGTIME                    ADC_SAMPLETIME_3CYCLES
#define ADCx_POLL_TIMEOUT               10
    
/**
  * @brief Joystick Pins definition 
  */ 
#if defined(USE_IOEXPANDER)
#define JOY_SEL_PIN                    IO_PIN_0
#define JOY_DOWN_PIN                   IO_PIN_1
#define JOY_LEFT_PIN                   IO_PIN_2
#define JOY_RIGHT_PIN                  IO_PIN_3
#define JOY_UP_PIN                     IO_PIN_4
#define JOY_NONE_PIN                   JOY_ALL_PINS
#define JOY_ALL_PINS                   (IO_PIN_0 | IO_PIN_1 | IO_PIN_2 | IO_PIN_3 | IO_PIN_4)
#endif /* USE_IOEXPANDER */
/**
  * @brief Eval Pins definition connected to MFX
  */

#if defined(USE_IOEXPANDER)
#define XSDN_PIN                       IO_PIN_16    
#define MII_INT_PIN                    IO_PIN_13
#define RSTI_PIN                       IO_PIN_11    
#define CAM_PLUG_PIN                   IO_PIN_12    
#define TS_INT_PIN                     IO_PIN_14
#define AUDIO_INT_PIN                  IO_PIN_5     
#define OTG_FS1_OVER_CURRENT_PIN       IO_PIN_6      
#define OTG_FS1_POWER_SWITCH_PIN       IO_PIN_7     
#define OTG_FS2_OVER_CURRENT_PIN       IO_PIN_8     
#define OTG_FS2_POWER_SWITCH_PIN       IO_PIN_9
#define SD1_DETECT_PIN                 IO_PIN_15    
#define SD2_DETECT_PIN                 IO_PIN_10
#define SD_DETECT_PIN                  SD1_DETECT_PIN
#endif /* USE_IOEXPANDER */


/* Exported constant IO ------------------------------------------------------*/

/*  The MFX_I2C_ADDR input pin selects the MFX I2C device address 
        MFX_I2C_ADDR input pin     MFX I2C device address
            0                           b: 1000 010x    (0x84)
            1                           b: 1000 011x    (0x86)
   This input is sampled during the MFX firmware startup.  */
#define IO_I2C_ADDRESS                   ((uint16_t)0x84)  /*mfx MFX_I2C_ADDR 0*/
#define IO_I2C_ADDRESS_2                 ((uint16_t)0x86)  /*mfx MFX_I2C_ADDR 1*/
/**
  * @brief TouchScreen FT6206 Slave I2C address 1
  */
#define TS_I2C_ADDRESS                   ((uint16_t)0x54)

/**
  * @brief TouchScreen FT6336G Slave I2C address 2
  */
#define TS_I2C_ADDRESS_A02               ((uint16_t)0x70)

/**
  * @brief LCD DSI Slave I2C address 1
  */
#define LCD_DSI_ADDRESS                  TS_I2C_ADDRESS

/**
  * @brief LCD DSI Slave I2C address 2
  */
#define LCD_DSI_ADDRESS_A02              TS_I2C_ADDRESS_A02

#define CAMERA_I2C_ADDRESS               ((uint16_t)0x5A)
#define CAMERA_I2C_ADDRESS_2             ((uint16_t)0x78)
#define AUDIO_I2C_ADDRESS                ((uint16_t)0x34)
#define EEPROM_I2C_ADDRESS_A01           ((uint16_t)0xA0)
#define EEPROM_I2C_ADDRESS_A02           ((uint16_t)0xA6)  
/* I2C clock speed configuration (in Hz) 
   WARNING: 
   Make sure that this define is not already declared in other files (ie. 
   stm32f769i_eval.h file). It can be used in parallel by other modules. */
#ifndef I2C_SPEED
 #define I2C_SPEED                        ((uint32_t)100000)
#endif /* I2C_SPEED */

/* User can use this section to tailor I2Cx/I2Cx instance used and associated 
   resources */
/* Definition for I2Cx clock resources */
#define EVAL_I2Cx                             I2C1
#define EVAL_I2Cx_CLK_ENABLE()                __HAL_RCC_I2C1_CLK_ENABLE()
#define EVAL_DMAx_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define EVAL_I2Cx_SCL_SDA_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOB_CLK_ENABLE()

#define EVAL_I2Cx_FORCE_RESET()               __HAL_RCC_I2C1_FORCE_RESET()
#define EVAL_I2Cx_RELEASE_RESET()             __HAL_RCC_I2C1_RELEASE_RESET()
   
/* Definition for I2Cx Pins */
#define EVAL_I2Cx_SCL_PIN                     GPIO_PIN_8
#define EVAL_I2Cx_SCL_SDA_GPIO_PORT           GPIOB
#define EVAL_I2Cx_SCL_SDA_AF                  GPIO_AF4_I2C1
#define EVAL_I2Cx_SDA_PIN                     GPIO_PIN_9

/* I2C interrupt requests */
#define EVAL_I2Cx_EV_IRQn                     I2C1_EV_IRQn
#define EVAL_I2Cx_ER_IRQn                     I2C1_ER_IRQn

/* I2C TIMING Register define when I2C clock source is SYSCLK */
/* I2C TIMING is calculated from APB1 source clock = 50 MHz */
/* Due to the big MOFSET capacity for adapting the camera level the rising time is very large (>1us) */
/* 0x40912732 takes in account the big rising and aims a clock of 100khz */
#ifndef EVAL_I2Cx_TIMING  
#define EVAL_I2Cx_TIMING                      ((uint32_t)0x40912732)  
#endif /* EVAL_I2Cx_TIMING */

/**
  * @}
  */ 

/**
  * @}
  */ 
  
/** @defgroup STM32F769I_EVAL_LOW_LEVEL_Exported_Macros LOW LEVEL Exported Macros
  * @{
  */  
/**
  * @}
  */ 

/** @defgroup STM32F769I_EVAL_LOW_LEVEL_Exported_Functions LOW LEVEL Exported Functions
  * @{
  */
uint32_t         BSP_GetVersion(void);  
void             BSP_LED_Init(Led_TypeDef Led);
void             BSP_LED_DeInit(Led_TypeDef Led);
void             BSP_LED_On(Led_TypeDef Led);
void             BSP_LED_Off(Led_TypeDef Led);
void             BSP_LED_Toggle(Led_TypeDef Led);
void             BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode);
void             BSP_PB_DeInit(Button_TypeDef Button);
uint32_t         BSP_PB_GetState(Button_TypeDef Button);
void             BSP_COM_Init(COM_TypeDef COM, UART_HandleTypeDef *husart);
void             BSP_COM_DeInit(COM_TypeDef COM, UART_HandleTypeDef *huart);
void             BSP_POTENTIOMETER_Init(void);
uint32_t         BSP_POTENTIOMETER_GetLevel(void);
#if defined(USE_IOEXPANDER)
uint8_t          BSP_JOY_Init(JOYMode_TypeDef JoyMode);
void             BSP_JOY_DeInit(void);
JOYState_TypeDef BSP_JOY_GetState(void);
#endif /* USE_IOEXPANDER */

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

#endif /* __STM32F769I_EVAL_H */

