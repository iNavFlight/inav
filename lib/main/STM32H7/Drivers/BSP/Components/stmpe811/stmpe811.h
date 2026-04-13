/**
  ******************************************************************************
  * @file    stmpe811.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the
  *          stmpe811.c IO Expander component core drivers.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2014 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STMPE811_H
#define STMPE811_H

#ifdef __cplusplus
 extern "C" {
#endif   
   
/* Includes ------------------------------------------------------------------*/
#include "stmpe811_reg.h"
#include <stddef.h>
#include "stmpe811_conf.h"

/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup Components
  * @{
  */
    
/** @addtogroup STMPE811
  * @{
  */    
   
/** @defgroup STMPE811_Exported_Constants Exported Constants
  * @{
  */ 

#define STMPE811_OK                    (0)
#define STMPE811_ERROR                 (-1)
   
/* Chip IDs */   
#define STMPE811_ID                     0x0811U

/* Max detectable simultaneous touches */
#define STMPE811_MAX_NB_TOUCH           0x01U

/* Global interrupt Enable bit */ 
#define STMPE811_GIT_EN                 0x01U   

/* IO expander functionalities */        
#define STMPE811_ADC_FCT                0x01U
#define STMPE811_TS_FCT                 0x02U
#define STMPE811_IO_FCT                 0x04U
#define STMPE811_TEMPSENS_FCT           0x08U

/* Global Interrupts definitions */ 
#define STMPE811_GIT_IO                 0x80U  /* IO interrupt                   */
#define STMPE811_GIT_ADC                0x40U  /* ADC interrupt                  */
#define STMPE811_GIT_TEMP               0x20U  /* Not implemented                */
#define STMPE811_GIT_FE                 0x10U  /* FIFO empty interrupt           */
#define STMPE811_GIT_FF                 0x08U  /* FIFO full interrupt            */
#define STMPE811_GIT_FOV                0x04U  /* FIFO overflowed interrupt      */
#define STMPE811_GIT_FTH                0x02U  /* FIFO above threshold interrupt */
#define STMPE811_GIT_TOUCH              0x01U  /* Touch is detected interrupt    */      
#define STMPE811_ALL_GIT                0x1FU  /* All global interrupts          */
#define STMPE811_TS_IT                  (STMPE811_GIT_TOUCH | STMPE811_GIT_FTH |  STMPE811_GIT_FOV | STMPE811_GIT_FF | STMPE811_GIT_FE) /* Touch screen interrupts */
    
/* Touch Screen Pins definition */ 
#define STMPE811_TOUCH_YD               STMPE811_GPIO_PIN_7 
#define STMPE811_TOUCH_XD               STMPE811_GPIO_PIN_6
#define STMPE811_TOUCH_YU               STMPE811_GPIO_PIN_5
#define STMPE811_TOUCH_XU               STMPE811_GPIO_PIN_4
#define STMPE811_TOUCH_IO_ALL           (uint32_t)(STMPE811_TOUCH_YD | STMPE811_TOUCH_XD | STMPE811_TOUCH_YU | STMPE811_TOUCH_XU)

/* IO Pins definition */ 
#define STMPE811_GPIO_PIN_0                 0x01U
#define STMPE811_GPIO_PIN_1                 0x02U
#define STMPE811_GPIO_PIN_2                 0x04U
#define STMPE811_GPIO_PIN_3                 0x08U
#define STMPE811_GPIO_PIN_4                 0x10U
#define STMPE811_GPIO_PIN_5                 0x20U
#define STMPE811_GPIO_PIN_6                 0x40U
#define STMPE811_GPIO_PIN_7                 0x80U
#define STMPE811_GPIO_PIN_ALL               0xFFU

/* IO Pins directions */ 
#define STMPE811_GPIO_DIR_IN                0x00U
#define STMPE811_GPIO_DIR_OUT               0x01U

#define STMPE811_GPIO_NOPULL                0x0U   /*!< No Pull-up or Pull-down activation  */
#define STMPE811_GPIO_PULLUP                0x1U   /*!< Pull-up activation                  */
#define STMPE811_GPIO_PULLDOWN              0x2U   /*!< Pull-down activation                */
                                                
#define STMPE811_GPIO_MODE_OFF              0x0U  /* when pin isn't used*/    
#define STMPE811_GPIO_MODE_ANALOG           0x1U  /* analog mode */
#define STMPE811_GPIO_MODE_INPUT            0x2U  /* input floating */
#define STMPE811_GPIO_MODE_OUTPUT_OD        0x3U  /* Open Drain output without internal resistor */
#define STMPE811_GPIO_MODE_OUTPUT_PP        0x4U  /* PushPull output without internal resistor */
#define STMPE811_GPIO_MODE_IT_RISING_EDGE   0x5U  /* float input - irq detect on rising edge */
#define STMPE811_GPIO_MODE_IT_FALLING_EDGE  0x6U  /* float input - irq detect on falling edge */
#define STMPE811_GPIO_MODE_IT_LOW_LEVEL     0x7U  /* float input - irq detect on low level */
#define STMPE811_GPIO_MODE_IT_HIGH_LEVEL    0x8U  /* float input - irq detect on high level */

/* TS registers masks */
#define STMPE811_TS_CTRL_ENABLE             0x01U  
#define STMPE811_TS_CTRL_STATUS             0x80U
/**
  * @}
  */ 

/** @defgroup STMPE811_Exported_Types STMPE811 Exported Types
  * @{
  */
typedef struct
{
  uint32_t Pin;       /*!< Specifies the GPIO pins to be configured.
                           This parameter can be any value of @ref GPIO_pins_define */

  uint32_t Mode;      /*!< Specifies the operating mode for the selected pins.
                           This parameter can be a value of @ref IO_ModeTypedef */

  uint32_t Pull;      /*!< Specifies the Pull-up or Pull-Down activation for the selected pins.
                           This parameter can be a value of @ref GPIO_pull_define */
}STMPE811_IO_Init_t;

typedef int32_t (*STMPE811_Init_Func)    (void);
typedef int32_t (*STMPE811_DeInit_Func)  (void);
typedef int32_t (*STMPE811_GetTick_Func) (void);
typedef int32_t (*STMPE811_Delay_Func)   (uint32_t);
typedef int32_t (*STMPE811_WriteReg_Func)(uint16_t, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*STMPE811_ReadReg_Func) (uint16_t, uint16_t, uint8_t*, uint16_t);

typedef struct
{
  STMPE811_Init_Func          Init;
  STMPE811_DeInit_Func        DeInit;
  uint16_t                    Address;  
  STMPE811_WriteReg_Func      WriteReg;
  STMPE811_ReadReg_Func       ReadReg;
  STMPE811_GetTick_Func       GetTick; 
} STMPE811_IO_t;
 
typedef struct
{
  STMPE811_IO_t         IO; 
  stmpe811_ctx_t        Ctx;  
  uint8_t               IsInitialized;
} STMPE811_Object_t;

typedef struct
{ 
  uint32_t  Radian;
  uint32_t  OffsetLeftRight;
  uint32_t  OffsetUpDown;
  uint32_t  DistanceLeftRight;
  uint32_t  DistanceUpDown;
  uint32_t  DistanceZoom;  
}STMPE811_Gesture_Init_t;

typedef struct
{
  uint32_t  TouchDetected;
  uint32_t  TouchX;
  uint32_t  TouchY;
} STMPE811_State_t;

typedef struct
{
  uint32_t  TouchDetected;
  uint32_t  TouchX[STMPE811_MAX_NB_TOUCH];
  uint32_t  TouchY[STMPE811_MAX_NB_TOUCH];
  uint32_t  TouchWeight[STMPE811_MAX_NB_TOUCH];
  uint32_t  TouchEvent[STMPE811_MAX_NB_TOUCH];
  uint32_t  TouchArea[STMPE811_MAX_NB_TOUCH];
} STMPE811_MultiTouch_State_t;

typedef struct 
{       
  uint8_t   MultiTouch;      
  uint8_t   Gesture;
  uint8_t   MaxTouch;
  uint32_t  MaxXl;
  uint32_t  MaxYl;
} STMPE811_Capabilities_t;

/* Touch screen driver structure initialization */  
typedef struct
{
  int32_t ( *Init                    )(STMPE811_Object_t *);
  int32_t ( *DeInit                  )(STMPE811_Object_t *);  
  int32_t ( *TS_GestureConfig        )(STMPE811_Object_t *, STMPE811_Gesture_Init_t*);
  int32_t ( *ReadID                  )(STMPE811_Object_t *, uint32_t *);  
  int32_t ( *TS_GetState             )(STMPE811_Object_t *, STMPE811_State_t*);
  int32_t ( *TS_GetMultiTouchState   )(STMPE811_Object_t *, STMPE811_MultiTouch_State_t*);
  int32_t ( *TS_GetGesture           )(STMPE811_Object_t *, uint8_t*);
  int32_t ( *TS_GetCapabilities      )(STMPE811_Object_t *, STMPE811_Capabilities_t*);  
  int32_t ( *TS_EnableIT             )(STMPE811_Object_t *);
  int32_t ( *TS_DisableIT            )(STMPE811_Object_t *);
  int32_t ( *TS_ClearIT              )(STMPE811_Object_t *);
  int32_t ( *TS_ITStatus             )(STMPE811_Object_t *);
}STMPE811_TS_Mode_t;

/* IO driver structure initialization */ 
typedef struct 
{
  int32_t ( *Init               )(STMPE811_Object_t *, STMPE811_IO_Init_t *);
  int32_t ( *DeInit             )(STMPE811_Object_t *);  
  int32_t ( *ReadID             )(STMPE811_Object_t *, uint32_t*);
  int32_t ( *Reset              )(STMPE811_Object_t *);
  int32_t ( *IO_Start           )(STMPE811_Object_t *, uint32_t);
  int32_t ( *IO_WritePin        )(STMPE811_Object_t *, uint32_t, uint8_t);
  int32_t ( *IO_ReadPin         )(STMPE811_Object_t *, uint32_t);
  int32_t ( *IO_EnableIT        )(STMPE811_Object_t *);
  int32_t ( *IO_DisableIT       )(STMPE811_Object_t *);
  int32_t ( *IO_ITStatus        )(STMPE811_Object_t *, uint32_t);
  int32_t ( *IO_ClearIT         )(STMPE811_Object_t *, uint32_t);
}STMPE811_IO_Mode_t;
/**
  * @}
  */ 

/** @addtogroup STMPE811_Exported_Variables
  * @{
  */ 
/* Touch screen driver structure */
extern STMPE811_TS_Mode_t STMPE811_TS_Driver;

/* IO driver structure */
extern STMPE811_IO_Mode_t STMPE811_IO_Driver;

/**
  * @}
  */ 
   
/** @addtogroup STMPE811_Exported_Functions
  * @{
  */

/** 
  * @brief STMPE811 Control functions
  */
int32_t STMPE811_RegisterBusIO (STMPE811_Object_t *pObj, STMPE811_IO_t *pIO);    
int32_t STMPE811_Init(STMPE811_Object_t *pObj);
int32_t STMPE811_DeInit(STMPE811_Object_t *pObj);
int32_t STMPE811_Reset(STMPE811_Object_t *pObj);
int32_t STMPE811_ReadID(STMPE811_Object_t *pObj, uint32_t *Id);

/** 
  * @brief STMPE811 IO functionalities functions
  */
int32_t STMPE811_IO_Start(STMPE811_Object_t *pObj, uint32_t IO_Pin);
int32_t STMPE811_IO_Init(STMPE811_Object_t *pObj, STMPE811_IO_Init_t *IoInit);
int32_t STMPE811_IO_InitPin(STMPE811_Object_t *pObj, uint32_t IO_Pin, uint8_t Direction);
int32_t STMPE811_IO_WritePin(STMPE811_Object_t *pObj, uint32_t IO_Pin, uint8_t PinState);
int32_t STMPE811_IO_ReadPin(STMPE811_Object_t *pObj, uint32_t IO_Pin);
int32_t STMPE811_IO_EnableAF(STMPE811_Object_t *pObj, uint32_t IO_Pin);
int32_t STMPE811_IO_DisableAF(STMPE811_Object_t *pObj, uint32_t IO_Pin);
int32_t STMPE811_IO_EnableIT(STMPE811_Object_t *pObj);
int32_t STMPE811_IO_DisableIT(STMPE811_Object_t *pObj);
int32_t STMPE811_IO_EnablePinIT(STMPE811_Object_t *pObj, uint32_t IO_Pin);
int32_t STMPE811_IO_DisablePinIT(STMPE811_Object_t *pObj, uint32_t IO_Pin);
int32_t STMPE811_IO_ITStatus(STMPE811_Object_t *pObj, uint32_t IO_Pin);
int32_t STMPE811_IO_ClearIT(STMPE811_Object_t *pObj, uint32_t IO_Pin);

/** 
  * @brief STMPE811 Touch screen functionalities functions
  */
int32_t STMPE811_TS_Init(STMPE811_Object_t *pObj);
int32_t STMPE811_TS_GestureConfig(STMPE811_Object_t *pObj, STMPE811_Gesture_Init_t *GestureInit);
int32_t STMPE811_TS_GetState(STMPE811_Object_t *pObj, STMPE811_State_t *State);
int32_t STMPE811_TS_GetMultiTouchState(STMPE811_Object_t *pObj, STMPE811_MultiTouch_State_t *State);
int32_t STMPE811_TS_GetGesture(STMPE811_Object_t *pObj, uint8_t *GestureId);
int32_t STMPE811_TS_EnableIT(STMPE811_Object_t *pObj);
int32_t STMPE811_TS_DisableIT(STMPE811_Object_t *pObj);
int32_t STMPE811_TS_ITStatus(STMPE811_Object_t *pObj);
int32_t STMPE811_TS_ClearIT(STMPE811_Object_t *pObj);
int32_t STMPE811_GetCapabilities(STMPE811_Object_t *pObj, STMPE811_Capabilities_t *Capabilities);

#ifdef __cplusplus
}
#endif
#endif /* STMPE811_H */

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
