/**
  ******************************************************************************
  * @file    stm32756g_eval_ts.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the Touch 
  *          Screen on STM32756G-EVAL and STM32746G-EVAL evaluation boards.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  @verbatim
  How To use this driver:
  -----------------------
   - This driver is used to drive the touch screen module of the STM32756G-EVAL
     evaluation board on the AMPIRE 640x480 LCD mounted on MB1063 or AMPIRE 
     480x272 LCD mounted on MB1046 daughter board.
   - If the AMPIRE 640x480 LCD is used, the TS3510 or EXC7200 component driver
     must be included according to the touch screen driver present on this board.
   - If the AMPIRE 480x272 LCD is used, the STMPE811 IO expander device component 
     driver must be included in order to run the TS module commanded by the IO 
     expander device, the MFXSTM32L152 IO expander device component driver must be
     also included in case of interrupt mode use of the TS.

  Driver description:
  ------------------
  + Initialization steps:
     o Initialize the TS module using the BSP_TS_Init() function. This 
       function includes the MSP layer hardware resources initialization and the
       communication layer configuration to start the TS use. The LCD size properties
       (x and y) are passed as parameters.
     o If TS interrupt mode is desired, you must configure the TS interrupt mode
       by calling the function BSP_TS_ITConfig(). The TS interrupt mode is generated
       as an external interrupt whenever a touch is detected. 
       The interrupt mode internally uses the IO functionalities driver driven by
       the IO expander, to configure the IT line.
  
  + Touch screen use
     o The touch screen state is captured whenever the function BSP_TS_GetState() is 
       used. This function returns information about the last LCD touch occurred
       in the TS_StateTypeDef structure.
     o If TS interrupt mode is used, the function BSP_TS_ITGetStatus() is needed to get
       the interrupt status. To clear the IT pending bits, you should call the 
       function BSP_TS_ITClear().
     o The IT is handled using the corresponding external interrupt IRQ handler,
       the user IT callback treatment is implemented on the same external interrupt
       callback.
  @endverbatim
  ******************************************************************************
  */ 

/* Dependencies
- stm32756g_eval_lcd.c
- exc7200.c
- ts3510.c
- stmpe811.c
EndDependencies */

/* Includes ------------------------------------------------------------------*/
#include "stm32756g_eval_ts.h"
#include "stm32756g_eval_io.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32756G_EVAL
  * @{
  */ 
  
/** @defgroup STM32756G_EVAL_TS STM32756G_EVAL TS
  * @{
  */   

/** @defgroup STM32756G_EVAL_TS_Private_Types_Definitions TS Private Types Definitions
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup STM32756G_EVAL_TS_Private_Defines TS Private Defines
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup STM32756G_EVAL_TS_Private_Macros TS Private Macros
  * @{
  */ 
/**
  * @}
  */  

/** @defgroup STM32756G_EVAL_TS_Private_Variables TS Private Variables
  * @{
  */ 
static TS_DrvTypeDef *tsDriver;
static uint16_t tsXBoundary, tsYBoundary; 
static uint8_t  tsOrientation;
static uint8_t  I2cAddress;
/**
  * @}
  */ 

/** @defgroup STM32756G_EVAL_TS_Private_Functions_Prototypes TS Private Functions Prototypes
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup STM32756G_EVAL_TS_Private_Functions TS Private Functions
  * @{
  */ 

/**
  * @brief  Initializes and configures the touch screen functionalities and 
  *         configures all necessary hardware resources (GPIOs, clocks..).
  * @param  xSize: Maximum X size of the TS area on LCD
  * @param  ySize: Maximum Y size of the TS area on LCD  
  * @retval TS_OK if all initializations are OK. Other value if error.
  */
uint8_t BSP_TS_Init(uint16_t xSize, uint16_t ySize)
{
  uint8_t status = TS_OK;
  tsXBoundary = xSize;
  tsYBoundary = ySize;
  
  /* Read ID and verify if the IO expander is ready */
  if(stmpe811_ts_drv.ReadID(TS_I2C_ADDRESS) == STMPE811_ID) 
  { 
    /* Initialize the TS driver structure */
    tsDriver = &stmpe811_ts_drv;  
    I2cAddress = TS_I2C_ADDRESS;
    tsOrientation = TS_SWAP_Y;
  }
  else
  {
    IOE_Init();
    
    /* Check TS3510 touch screen driver presence to determine if TS3510 or
     * EXC7200 driver will be used */
    if(BSP_TS3510_IsDetected() == 0)
    {
      /* Initialize the TS driver structure */
      tsDriver = &ts3510_ts_drv; 
      I2cAddress = TS3510_I2C_ADDRESS;
    }
    else    
    {
      /* Initialize the TS driver structure */
      tsDriver = &exc7200_ts_drv; 
      I2cAddress = EXC7200_I2C_ADDRESS;
    }
    tsOrientation = TS_SWAP_NONE;   
  }
  
  /* Initialize the TS driver */
  tsDriver->Init(I2cAddress);
  tsDriver->Start(I2cAddress);
  
  return status;
}

/**
  * @brief  DeInitializes the TouchScreen.
  * @retval TS state
  */
uint8_t BSP_TS_DeInit(void)
{ 
  /* Actually ts_driver does not provide a DeInit function */
  return TS_OK;
}

/**
  * @brief  Configures and enables the touch screen interrupts.
  * @retval TS_OK if all initializations are OK. Other value if error.
  */
uint8_t BSP_TS_ITConfig(void)
{
  /* Initialize the IO */
  BSP_IO_Init();
  
  /* Configure TS IT line IO */
  BSP_IO_ConfigPin(TS_INT_PIN, IO_MODE_IT_FALLING_EDGE);
  
  /* Enable the TS ITs */
  tsDriver->EnableIT(I2cAddress);

  return TS_OK;  
}

/**
  * @brief  Gets the touch screen interrupt status.
  * @retval TS_OK if all initializations are OK. Other value if error.
  */
uint8_t BSP_TS_ITGetStatus(void)
{
  /* Return the TS IT status */
  return (tsDriver->GetITStatus(I2cAddress));
}

/**
  * @brief  Returns status and positions of the touch screen.
  * @param  TS_State: Pointer to touch screen current state structure
  * @retval TS_OK if all initializations are OK. Other value if error.
  */
uint8_t BSP_TS_GetState(TS_StateTypeDef *TS_State)
{
  static uint32_t _x = 0, _y = 0;
  uint16_t x_diff, y_diff , x , y;
  uint16_t swap;
  
  TS_State->TouchDetected = tsDriver->DetectTouch(I2cAddress);
  
  if(TS_State->TouchDetected)
  {
    tsDriver->GetXY(I2cAddress, &x, &y); 
    
    if(tsOrientation & TS_SWAP_X)
    {
      x = 4096 - x;  
    }
    
    if(tsOrientation & TS_SWAP_Y)
    {
      y = 4096 - y;
    }
    
    if(tsOrientation & TS_SWAP_XY)
    {
      swap = y; 
      y = x;      
      x = swap;      
    }
    
    x_diff = x > _x? (x - _x): (_x - x);
    y_diff = y > _y? (y - _y): (_y - y); 
    
    if (x_diff + y_diff > 5)
    {
      _x = x;
      _y = y; 
    }

      TS_State->x = (tsXBoundary * _x) >> 12;
      TS_State->y = (tsYBoundary * _y) >> 12;
  }  
  return TS_OK;
}

/**
  * @brief  Clears all touch screen interrupts.
  * @retval None
  */
void BSP_TS_ITClear(void)
{
  /* Clear all IO IT pin */
  BSP_IO_ITClear();
  
  /* Clear TS IT pending bits */
  tsDriver->ClearIT(I2cAddress); 
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

