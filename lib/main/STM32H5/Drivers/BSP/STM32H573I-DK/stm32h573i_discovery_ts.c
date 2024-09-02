/**
  ******************************************************************************
  * @file    stm32h573i_discovery_ts.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the Touch
  *          Screen on STM32H573I-DK board.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  @verbatim
  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================
   - This driver is used to drive the touch screen module of the STM32H573I-DK
     discovery board on the LCD mounted on MB989 daughter board.
     The touch screen driver IC is a FT6236U.

  Driver description:
  ---------------------
    + Initialization steps:
       o Initialize the TS using the BSP_TS_Init() function. You can select
         display orientation with "Orientation" parameter of TS_Init_t structure
         (portrait, landscape, portrait with 180 degrees rotation or landscape
         with 180 degrees rotation). The LCD size properties (width and height)
         are also parameters of TS_Init_t and depend on the orientation selected.

    + Touch screen use
       o Call BSP_TS_EnableIT() (BSP_TS_DisableIT()) to enable (disable) touch
         screen interrupt. BSP_TS_Callback() is called when TS interrupt occurs.
       o Call BSP_TS_GetState() to get the current touch status (detection and
         coordinates).
       o Call BSP_TS_Set_Orientation() to change the current orientation.
         Call BSP_TS_Get_Orientation() to get the current orientation.
       o Call BSP_TS_GetCapabilities() to get the FT6236U capabilities.
       o FT6236U doesn't support multi touch and gesture features.
         BSP_TS_Get_MultiTouchState(), BSP_TS_GestureConfig() and
         BSP_TS_GetGestureId() functions will return BSP_ERROR_FEATURE_NOT_SUPPORTED.

    + De-initialization steps:
       o De-initialize the touch screen using the BSP_TS_DeInit() function.

  @endverbatim
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32h573i_discovery_ts.h"
#include "stm32h573i_discovery_bus.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H573I_DK
  * @{
  */

/** @defgroup STM32H573I_DK_TS TS
  * @{
  */

/** @defgroup STM32H573I_DK_TS_Private_Defines TS Private Defines
  * @{
  */

/**
  * @}
  */

/** @defgroup STM32H573I_DK_TS_Private_Macros TS Private Macros
  * @{
  */
#define TS_MIN(a,b) ((a > b) ? b : a)
/**
  * @}
  */

/** @defgroup STM32H573I_DK_TS_Private_TypesDefinitions TS Private TypesDefinitions
  * @{
  */
typedef void (* BSP_EXTI_LineCallback)(void);
/**
  * @}
  */

/** @defgroup STM32H573I_DK_TS_Privates_Variables TS Privates Variables
  * @{
  */
static TS_Drv_t           *Ts_Drv = NULL;
/**
  * @}
  */

/** @defgroup STM32H573I_DK_TS_Exported_Variables TS Exported Variables
  * @{
  */
EXTI_HandleTypeDef hts_exti[TS_INSTANCES_NBR] = {0};
void     *Ts_CompObj[TS_INSTANCES_NBR] = {0};
TS_Ctx_t  Ts_Ctx[TS_INSTANCES_NBR]     = {0};
/**
  * @}
  */

/** @defgroup STM32H573I_DK_TS_Private_FunctionPrototypes TS Private Function Prototypes
  * @{
  */
static int32_t FT6X06_Probe(uint32_t Instance);
static void    TS_EXTI_Callback(void);
static void TS_RESET_MspInit(void);
/**
  * @}
  */

/** @addtogroup STM32H573I_DK_TS_Exported_Functions
  * @{
  */

/**
  * @brief  Initializes and configures the touch screen functionalities and
  *         configures all necessary hardware resources (GPIOs, I2C, clocks..).
  * @param  Instance TS instance. Could be only 0.
  * @param  TS_Init  TS Init structure
  * @retval BSP status
  */
int32_t BSP_TS_Init(uint32_t Instance, TS_Init_t *TS_Init)
{
  int32_t ret = BSP_ERROR_NONE;

  if((Instance >= TS_INSTANCES_NBR) || (TS_Init->Width == 0U) ||( TS_Init->Width > TS_MAX_WIDTH) ||\
                         (TS_Init->Height == 0U) ||( TS_Init->Height > TS_MAX_HEIGHT) ||\
                         (TS_Init->Accuracy > TS_MIN((TS_Init->Width), (TS_Init->Height))))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    TS_RESET_MspInit();

    if(FT6X06_Probe(Instance) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_NO_INIT;
    }
    else
    {
      TS_Capabilities_t Capabilities;
      uint32_t i;
      /* Store parameters on TS context */
      Ts_Ctx[Instance].Width       = TS_Init->Width;
      Ts_Ctx[Instance].Height      = TS_Init->Height;
      Ts_Ctx[Instance].Accuracy    = TS_Init->Accuracy;
      if(TS_Init->Orientation == TS_ORIENTATION_LANDSCAPE_ROT180)
      {
        Ts_Ctx[Instance].Orientation = TS_SWAP_XY | TS_SWAP_Y;
      }
      else if(TS_Init->Orientation == TS_ORIENTATION_LANDSCAPE)
      {
        Ts_Ctx[Instance].Orientation = TS_SWAP_XY | TS_SWAP_X;
      }
      else if(TS_Init->Orientation == TS_ORIENTATION_PORTRAIT_ROT180)
      {
        Ts_Ctx[Instance].Orientation = TS_SWAP_NONE;
      }
      else /* (Orientation == TS_ORIENTATION_PORTRAIT) */
      {
        Ts_Ctx[Instance].Orientation = TS_SWAP_Y | TS_SWAP_X;
      }

      /* Get capabilities to retrieve maximum values of X and Y */
      if (Ts_Drv->GetCapabilities(Ts_CompObj[Instance], &Capabilities) < 0)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else
      {
        /* Store maximum X and Y on context */
        Ts_Ctx[Instance].MaxX = Capabilities.MaxXl;
        Ts_Ctx[Instance].MaxY = Capabilities.MaxYl;
        /* Initialize previous position in order to always detect first touch */
        for(i = 0; i < TS_TOUCH_NBR; i++)
        {
          Ts_Ctx[Instance].PrevX[i] = TS_Init->Width + TS_Init->Accuracy + 1U;
          Ts_Ctx[Instance].PrevY[i] = TS_Init->Height + TS_Init->Accuracy + 1U;
        }
      }
    }
  }

  return ret;
}

/**
  * @brief  De-Initializes the touch screen functionalities
  * @param  Instance TS instance. Could be only 0.
  * @retval BSP status
  */
int32_t BSP_TS_DeInit(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= TS_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(Ts_Drv->DeInit(Ts_CompObj[Instance]) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  return ret;
}


/**
  * @brief  Get the TS capabilities.
  * @param  Instance TS Instance.
  * @param  Capabilities Pointer to TS capabilities structure.
  * @retval BSP status.
  */
int32_t BSP_TS_GetCapabilities(uint32_t Instance, TS_Capabilities_t *Capabilities)
{
  int32_t ret = BSP_ERROR_NONE;

  if ((Instance >= TS_INSTANCES_NBR) || (Capabilities == NULL))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Get the TS driver capabilities */
    if (Ts_Drv->GetCapabilities(Ts_CompObj[Instance], Capabilities) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Update maximum X and Y according orientation */
      if ((Ts_Ctx[Instance].Orientation == TS_ORIENTATION_LANDSCAPE)
          || (Ts_Ctx[Instance].Orientation == TS_ORIENTATION_LANDSCAPE_ROT180))
      {
        uint32_t tmp;
        tmp = Capabilities->MaxXl;
        Capabilities->MaxXl = Capabilities->MaxYl;
        Capabilities->MaxYl = tmp;
      }
    }
  }

  return ret;
}

/**
  * @brief  Configures and enables the touch screen interrupts.
  * @param  Instance TS instance. Could be only 0.
  * @retval BSP status
  */
int32_t BSP_TS_EnableIT(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;
  GPIO_InitTypeDef gpio_init_structure;

  if(Instance >= TS_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Configure Interrupt mode for TS_INT pin falling edge : when a new touch is available */
    /* TS_INT pin is active on low level on new touch available */
    gpio_init_structure.Pin   = TS_INT_PIN;
    gpio_init_structure.Pull  = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio_init_structure.Mode  = GPIO_MODE_IT_FALLING;
    HAL_GPIO_Init(TS_INT_GPIO_PORT, &gpio_init_structure);

    if(Ts_Drv->EnableIT(Ts_CompObj[Instance]) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      if(HAL_EXTI_GetHandle(&hts_exti[Instance], TS_EXTI_LINE) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else if(HAL_EXTI_RegisterCallback(&hts_exti[Instance],  HAL_EXTI_COMMON_CB_ID, TS_EXTI_Callback) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else
      {
        /* Enable and set the TS_INT EXTI Interrupt to an intermediate priority */
        HAL_NVIC_SetPriority((IRQn_Type)(TS_INT_EXTI_IRQn), BSP_TS_IT_PRIORITY, 0x00);
        HAL_NVIC_EnableIRQ((IRQn_Type)(TS_INT_EXTI_IRQn));
      }
    }
  }

  return ret;
}

/**
  * @brief  Disables the touch screen interrupts.
  * @param  Instance TS instance. Could be only 0.
  * @retval BSP status
  */
int32_t BSP_TS_DisableIT(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;
  GPIO_InitTypeDef gpio_init_structure;

  if(Instance >= TS_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Configure TS_INT_PIN low level to generate MFX_IRQ_OUT in EXTI on MCU            */
    gpio_init_structure.Pin  = TS_INT_PIN;
    HAL_GPIO_DeInit(TS_INT_GPIO_PORT, gpio_init_structure.Pin);

    /* Disable the TS in interrupt mode */
    /* In that case the INT output of FT6X06 when new touch is available */
    /* is active on low level and directed on EXTI */
    if(Ts_Drv->DisableIT(Ts_CompObj[Instance]) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  BSP TS Callback.
  * @param  Instance  TS instance. Could be only 0.
  * @retval None.
  */
__weak void BSP_TS_Callback(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);

  /* This function should be implemented by the user application.
     It is called into this driver when an event on TS touch detection */
}

/**
  * @brief  BSP TS interrupt handler.
  * @param  Instance TS Instance.
  * @retval None.
  */
void BSP_TS_IRQHandler(uint32_t Instance)
{
  HAL_EXTI_IRQHandler(&hts_exti[Instance]);
}

/**
  * @brief  Set the TS orientation.
  * @param  Instance TS Instance.
  * @param  Orientation TS orientation.
  * @retval BSP status.
  */
int32_t BSP_TS_Set_Orientation(uint32_t Instance, uint32_t Orientation)
{
  int32_t  status = BSP_ERROR_NONE;
  uint32_t tmp;
  uint32_t i;

  if ((Instance >= TS_INSTANCES_NBR) || (Orientation > TS_ORIENTATION_LANDSCAPE))
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Update TS context if orientation is changed from/to portrait to/from landscape */
    if ((((Ts_Ctx[Instance].Orientation == TS_ORIENTATION_LANDSCAPE)
          || (Ts_Ctx[Instance].Orientation == TS_ORIENTATION_LANDSCAPE_ROT180)) &&
         ((Orientation == TS_ORIENTATION_PORTRAIT) || (Orientation == TS_ORIENTATION_PORTRAIT_ROT180))) ||
        (((Ts_Ctx[Instance].Orientation == TS_ORIENTATION_PORTRAIT)
          || (Ts_Ctx[Instance].Orientation == TS_ORIENTATION_PORTRAIT_ROT180)) &&
         ((Orientation == TS_ORIENTATION_LANDSCAPE) || (Orientation == TS_ORIENTATION_LANDSCAPE_ROT180))))
    {
      /* Invert width and height */
      tmp = Ts_Ctx[Instance].Width;
      Ts_Ctx[Instance].Width  = Ts_Ctx[Instance].Height;
      Ts_Ctx[Instance].Height = tmp;
      /* Invert MaxX and MaxY */
      tmp = Ts_Ctx[Instance].MaxX;
      Ts_Ctx[Instance].MaxX = Ts_Ctx[Instance].MaxY;
      Ts_Ctx[Instance].MaxY = tmp;
    }

    /* Store orientation on TS context */
    if(Orientation == TS_ORIENTATION_LANDSCAPE_ROT180)
    {
      Ts_Ctx[Instance].Orientation = TS_SWAP_XY | TS_SWAP_Y;
    }
    else if(Orientation == TS_ORIENTATION_LANDSCAPE)
    {
      Ts_Ctx[Instance].Orientation = TS_SWAP_XY | TS_SWAP_X;
    }
    else if(Orientation == TS_ORIENTATION_PORTRAIT_ROT180)
    {
      Ts_Ctx[Instance].Orientation = TS_SWAP_NONE;
    }
    else /* (Orientation == TS_ORIENTATION_PORTRAIT) */
    {
      Ts_Ctx[Instance].Orientation = TS_SWAP_Y | TS_SWAP_X;
    }

    /* Reset previous position */
    for (i = 0; i < TS_TOUCH_NBR; i++)
    {
      Ts_Ctx[Instance].PrevX[i] = Ts_Ctx[Instance].Width + Ts_Ctx[Instance].Accuracy + 1U;
      Ts_Ctx[Instance].PrevY[i] = Ts_Ctx[Instance].Height + Ts_Ctx[Instance].Accuracy + 1U;
    }
  }

  return status;
}

/**
  * @brief  Get the TS orientation.
  * @param  Instance TS Instance.
  * @param  Orientation Pointer to TS orientation.
  * @retval BSP status.
  */
int32_t BSP_TS_Get_Orientation(uint32_t Instance, uint32_t *Orientation)
{
  int32_t status = BSP_ERROR_NONE;
  
  if ((Instance >= TS_INSTANCES_NBR) || (Orientation == NULL))
  {
    status = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Get orientation from TS context */    
    if(Ts_Ctx[Instance].Orientation == (TS_SWAP_XY | TS_SWAP_Y))
    {
      *Orientation = TS_ORIENTATION_LANDSCAPE_ROT180;
    }
    else if(Ts_Ctx[Instance].Orientation == (TS_SWAP_XY | TS_SWAP_X))
    {
      *Orientation = TS_ORIENTATION_LANDSCAPE;
    }
    else if(Ts_Ctx[Instance].Orientation == TS_SWAP_NONE)
    {
      *Orientation = TS_ORIENTATION_PORTRAIT_ROT180;
    }
    else
    {
      if(Ts_Ctx[Instance].Orientation == (TS_SWAP_Y | TS_SWAP_X))
      {
        *Orientation = TS_ORIENTATION_PORTRAIT;
      }
    }
  }

  return status;
}

/**
  * @brief  Returns positions of a single touch screen.
  * @param  Instance  TS instance. Could be only 0.
  * @param  TS_State  Pointer to touch screen current state structure
  * @retval BSP status
  */
int32_t BSP_TS_GetState(uint32_t Instance, TS_State_t *TS_State)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t x_oriented, y_oriented;
  uint32_t x_diff, y_diff;

  if(Instance >= TS_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    FT6X06_State_t state;

    /* Get each touch coordinates */
    if(Ts_Drv->GetState(Ts_CompObj[Instance], &state) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }/* Check and update the number of touches active detected */
    else if(state.TouchDetected != 0U)
    {
      x_oriented = state.TouchX;
      y_oriented = state.TouchY;

      if((Ts_Ctx[Instance].Orientation & TS_SWAP_XY) == TS_SWAP_XY)
      {
        x_oriented = state.TouchY;
        y_oriented = state.TouchX;
      }

      if((Ts_Ctx[Instance].Orientation & TS_SWAP_X) == TS_SWAP_X)
      {
        x_oriented = Ts_Ctx[Instance].MaxX - x_oriented - 1UL;
      }

      if((Ts_Ctx[Instance].Orientation & TS_SWAP_Y) == TS_SWAP_Y)
      {
        y_oriented = Ts_Ctx[Instance].MaxY - y_oriented;
      }

      /* Apply boundary */
      TS_State->TouchX = (x_oriented * Ts_Ctx[Instance].Width) / Ts_Ctx[Instance].MaxX;
      TS_State->TouchY = (y_oriented * Ts_Ctx[Instance].Height) / Ts_Ctx[Instance].MaxY;
      /* Store Current TS state */
      TS_State->TouchDetected = state.TouchDetected;

      /* Check accuracy */
      x_diff = (TS_State->TouchX > Ts_Ctx[Instance].PrevX[0])?
        (TS_State->TouchX - Ts_Ctx[Instance].PrevX[0]):
        (Ts_Ctx[Instance].PrevX[0] - TS_State->TouchX);

      y_diff = (TS_State->TouchY > Ts_Ctx[Instance].PrevY[0])?
        (TS_State->TouchY - Ts_Ctx[Instance].PrevY[0]):
        (Ts_Ctx[Instance].PrevY[0] - TS_State->TouchY);


      if ((x_diff > Ts_Ctx[Instance].Accuracy) || (y_diff > Ts_Ctx[Instance].Accuracy))
      {
        /* New touch detected */
        Ts_Ctx[Instance].PrevX[0] = TS_State->TouchX;
        Ts_Ctx[Instance].PrevY[0] = TS_State->TouchY;
      }
      else
      {
        TS_State->TouchX = Ts_Ctx[Instance].PrevX[0];
        TS_State->TouchY = Ts_Ctx[Instance].PrevY[0];
      }
    }
    else
    {
      TS_State->TouchDetected = 0U;
      TS_State->TouchX = Ts_Ctx[Instance].PrevX[0];
      TS_State->TouchY = Ts_Ctx[Instance].PrevY[0];
    }
  }

  return ret;
}

#if (USE_TS_MULTI_TOUCH > 0)
/**
  * @brief  Returns positions of multi touch screen.
  * @param  Instance  TS instance. Could be only 0.
  * @param  TS_State  Pointer to touch screen current state structure
  * @retval BSP status
  */
int32_t BSP_TS_Get_MultiTouchState(uint32_t Instance, TS_MultiTouch_State_t *TS_State)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t index;
  uint32_t x_oriented, y_oriented;
  uint32_t x_diff, y_diff;

  if(Instance >= TS_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    FT6X06_MultiTouch_State_t state;

    /* Get each touch coordinates */
    if(Ts_Drv->GetMultiTouchState(Ts_CompObj[Instance], &state) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Check and update the number of touches active detected */
      if(state.TouchDetected != 0U)
      {
        for(index = 0; index < state.TouchDetected; index++)
        {
          x_oriented = state.TouchX[index];
          y_oriented = state.TouchY[index];

          if((Ts_Ctx[Instance].Orientation & TS_SWAP_XY) == TS_SWAP_XY)
          {
            x_oriented = state.TouchY[index];
            y_oriented = state.TouchX[index];
          }

          if((Ts_Ctx[Instance].Orientation & TS_SWAP_X) == TS_SWAP_X)
          {
            x_oriented = Ts_Ctx[Instance].MaxX - state.TouchX[index] - 1UL;
          }

          if((Ts_Ctx[Instance].Orientation & TS_SWAP_Y) == TS_SWAP_Y)
          {
            y_oriented = Ts_Ctx[Instance].MaxY - state.TouchY[index];
          }

          /* Apply boundary */
          TS_State->TouchX[index] = (x_oriented * Ts_Ctx[Instance].Width) / Ts_Ctx[Instance].MaxX;
          TS_State->TouchY[index] = (y_oriented * Ts_Ctx[Instance].Height) / Ts_Ctx[Instance].MaxY;
          /* Store Current TS state */
          TS_State->TouchDetected = state.TouchDetected;

          /* Check accuracy */
          x_diff = (TS_State->TouchX[index] > Ts_Ctx[Instance].PrevX[index])?
                   (TS_State->TouchX[index] - Ts_Ctx[Instance].PrevX[index]):
                   (Ts_Ctx[Instance].PrevX[index] - TS_State->TouchX[index]);

          y_diff = (TS_State->TouchY[index] > Ts_Ctx[Instance].PrevY[index])?
                   (TS_State->TouchY[index] - Ts_Ctx[Instance].PrevY[index]):
                   (Ts_Ctx[Instance].PrevY[index] - TS_State->TouchY[index]);

          if ((x_diff > Ts_Ctx[Instance].Accuracy) || (y_diff > Ts_Ctx[Instance].Accuracy))
          {
            /* New touch detected */
            Ts_Ctx[Instance].PrevX[index] = TS_State->TouchX[index];
            Ts_Ctx[Instance].PrevY[index] = TS_State->TouchY[index];
          }
          else
          {
            TS_State->TouchX[index] = Ts_Ctx[Instance].PrevX[index];
            TS_State->TouchY[index] = Ts_Ctx[Instance].PrevY[index];
          }
        }
      }
      else
      {
        TS_State->TouchDetected = 0U;
        for(index = 0; index < TS_TOUCH_NBR; index++)
        {
          TS_State->TouchX[index] = Ts_Ctx[Instance].PrevX[index];
          TS_State->TouchY[index] = Ts_Ctx[Instance].PrevY[index];
        }
      }
    }
  }

  return ret;
}
#endif /* USE_TS_MULTI_TOUCH > 0 */

#if (USE_TS_GESTURE > 0)
/**
  * @brief  Configure gesture on TS.
  * @param  Instance TS Instance.
  * @param  GestureConfig Pointer to gesture configuration structure.
  * @retval BSP status.
  */
int32_t BSP_TS_GestureConfig(uint32_t Instance, const TS_Gesture_Config_t *GestureConfig)
{
  UNUSED(Instance);   
  UNUSED(GestureConfig);

    /* Feature not supported */
  return BSP_ERROR_FEATURE_NOT_SUPPORTED;
}

/**
  * @brief  Get gesture.
  * @param  Instance TS Instance.
  * @param  GestureId Pointer to gesture.
  * @retval BSP status.
  */
int32_t BSP_TS_GetGestureId(uint32_t Instance, const uint32_t *GestureId)
{
  UNUSED(Instance);    
  UNUSED(GestureId);

    /* Feature not supported */
  return BSP_ERROR_FEATURE_NOT_SUPPORTED;
}
#endif /* (USE_TS_GESTURE > 0) */

/**
  * @}
  */

/** @defgroup STM32H573I_DK_TS_Private_Functions TS Private Functions
  * @{
  */
/**
  * @brief  Probe the FT6X06 TS driver.
  * @param  Instance TS Instance.
  * @retval BSP status.
  */
static int32_t FT6X06_Probe(uint32_t Instance)
{
  int32_t ret              = BSP_ERROR_NONE;
  FT6X06_IO_t              IOCtx;
  static FT6X06_Object_t   FT6X06Obj;
  uint32_t ft6x06_id       = 0;

  /* Configure the TS driver */
  IOCtx.Address     = TS_I2C_ADDRESS;
  IOCtx.Init        = BSP_I2C4_Init;
  IOCtx.DeInit      = BSP_I2C4_DeInit;
  IOCtx.ReadReg     = BSP_I2C4_ReadReg;
  IOCtx.WriteReg    = BSP_I2C4_WriteReg;
  IOCtx.GetTick     = BSP_GetTick;

  if (FT6X06_RegisterBusIO(&FT6X06Obj, &IOCtx) != FT6X06_OK)
  {
    ret = BSP_ERROR_BUS_FAILURE;
  }
  else if (FT6X06_ReadID(&FT6X06Obj, &ft6x06_id) != FT6X06_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else if ((uint8_t)ft6x06_id != FT6X06_ID)
  {
    ret = BSP_ERROR_UNKNOWN_COMPONENT;
  }
  else
  {
    Ts_CompObj[Instance] = &FT6X06Obj;
    Ts_Drv = (TS_Drv_t *) &FT6X06_TS_Driver;
    if (Ts_Drv->Init(Ts_CompObj[Instance]) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  TS EXTI callback.
  * @retval None.
  */
static void TS_EXTI_Callback(void)
{
  BSP_TS_Callback(0);

  /* Clear interrupt on TS driver */
  if (Ts_Drv->ClearIT(Ts_CompObj[0]) < 0)
  {
    /* Nothing to do */
  }
}

/**
  * @brief  Initializes the TS_RESET pin MSP.
  * @retval None
  */
static void TS_RESET_MspInit(void)
{
  GPIO_InitTypeDef  gpio_init_structure;

  TS_RESET_GPIO_CLK_ENABLE();

  /* GPIO configuration in output for TouchScreen reset signal on TS_RESET pin */
  gpio_init_structure.Pin = TS_RESET_GPIO_PIN;
  gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TS_RESET_GPIO_PORT, &gpio_init_structure);

  HAL_GPIO_WritePin(TS_RESET_GPIO_PORT, TS_RESET_GPIO_PIN, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(TS_RESET_GPIO_PORT, TS_RESET_GPIO_PIN, GPIO_PIN_SET);
  HAL_Delay(500);
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
