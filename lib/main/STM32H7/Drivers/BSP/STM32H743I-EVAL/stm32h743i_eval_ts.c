/**
  ******************************************************************************
  * @file    stm32h743i_eval_ts.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the Touch
  *          Screen on STM32H743I-EVAL evaluation boards.
  @verbatim
  How To use this driver:
  -----------------------
   - This driver is used to drive the touch screen module of the STM32H743I-EVAL
     evaluation board on the AMPIRE 640x480 LCD mounted on MB1063 or AMPIRE
     480x272 LCD mounted on MB1046 daughter board.
   - If the AMPIRE 640x480 LCD is used, the TS3510 or EXC7200 component driver
     must be included according to the touch screen driver present on this board.
   - If the AMPIRE 480x272 LCD is used, the EXC7200 IO expander device component
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
#include "stm32h743i_eval_ts.h"
#include "stm32h743i_eval_io.h"
#include "stm32h743i_eval_bus.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H743I_EVAL
  * @{
  */

/** @addtogroup STM32H743I_EVAL_TS
  * @{
  */

/** @defgroup STM32H743I_EVAL_TS_Private_Types_Definitions TS Private Types Definitions
  * @{
  */
typedef void (* BSP_EXTI_LineCallback) (void);
/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_TS_Private_Defines TS Private Defines
  * @{
  */
/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_TS_Private_Macros TS Private Macros
  * @{
  */
#define TS_MIN(a,b) ((a > b) ? b : a)
/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_TS_Private_Variables TS Private Variables
  * @{
  */
static TS_Drv_t           *Ts_Drv = NULL;
/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_TS_Exported_Variables TS Exported Variables
  * @{
  */
EXTI_HandleTypeDef hts_exti[TS_INSTANCES_NBR] = {0};
void     *Ts_CompObj[TS_INSTANCES_NBR] = {0};
TS_Ctx_t  Ts_Ctx[TS_INSTANCES_NBR]     = {0};
/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_TS_Private_Function_Prototypes TS Private Function Prototypes
  * @{
  */
#if (USE_EXC7200_TS_CTRL == 1U)
static int32_t EXC7200_Probe(uint32_t Instance);
#endif
#if (USE_TS3510_TS_CTRL == 1U)
static int32_t TS3510_Probe(uint32_t Instance);
#endif
#if (USE_EXC80W32_TS_CTRL == 1U)
static int32_t EXC80W32_Probe(uint32_t Instance);
#endif
static void TS_EXTI_Callback(void);
/**
  * @}
  */

/** addtogroup STM32H743I_EVAL_TS_Exported_Functions
  * @{
  */

/**
  * @brief  Initializes and configures the touch screen functionalities and
  *         configures all necessary hardware resources (GPIOs, clocks..).
  * @param  Instance TS instance. Could be only 0.
  * @param  TS_Init  TS Init structure
  * @retval BSP status
  */
int32_t BSP_TS_Init(uint32_t Instance, TS_Init_t *TS_Init)
{
  int32_t ret = BSP_ERROR_NONE;

  if((Instance >=TS_INSTANCES_NBR) || (TS_Init->Width == 0U) ||( TS_Init->Width > TS_MAX_WIDTH) ||\
                         (TS_Init->Height == 0U) ||( TS_Init->Height > TS_MAX_HEIGHT) ||\
                         (TS_Init->Accuracy > TS_MIN((TS_Init->Width), (TS_Init->Height))))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_EXC7200_TS_CTRL == 1U)
    ret = EXC7200_Probe(Instance);
#endif
#if (USE_TS3510_TS_CTRL == 1U)
    ret = TS3510_Probe(Instance);
#endif
#if (USE_EXC80W32_TS_CTRL == 1U)
    ret = EXC80W32_Probe(Instance);
#endif
    if(ret != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_UNKNOWN_COMPONENT;
    }
    else
    {
      TS_Capabilities_t Capabilities;
      /* Store parameters on TS context */
      Ts_Ctx[Instance].Width             = TS_Init->Width;
      Ts_Ctx[Instance].Height            = TS_Init->Height;
      Ts_Ctx[Instance].Orientation       = TS_Init->Orientation;
      Ts_Ctx[Instance].Accuracy          = TS_Init->Accuracy;
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
        Ts_Ctx[Instance].PreviousX[0] = TS_Init->Width + TS_Init->Accuracy + 1U;
        Ts_Ctx[Instance].PreviousY[0] = TS_Init->Height + TS_Init->Accuracy + 1U;
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

  if(Instance >=TS_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(Ts_Drv->DeInit(Ts_CompObj[Instance]) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  Get Touch Screen instance capabilities
  * @param  Instance Touch Screen instance
  * @param  Capabilities pointer to Touch Screen capabilities
  * @retval BSP status
  */
int32_t BSP_TS_GetCapabilities(uint32_t Instance, TS_Capabilities_t *Capabilities)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >=TS_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    (void)Ts_Drv->GetCapabilities(Ts_CompObj[Instance], Capabilities);
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
  static const uint32_t TS_EXTI_LINE[TS_INSTANCES_NBR] = {TS_INT_LINE};
  static BSP_EXTI_LineCallback TsCallback[TS_INSTANCES_NBR] = {TS_EXTI_Callback};

  if(Instance >= TS_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_BSP_IO_CLASS > 0U)
    BSP_IO_Init_t    io_init_structure;

    /* Configure Interrupt mode for TS_INT pin falling edge : when a new touch is available */
    /* TS_INT pin is active on low level on new touch available */
    io_init_structure.Pin= TS_INT_PIN;
    io_init_structure.Mode = IO_MODE_IT_LOW_LEVEL;
    io_init_structure.Pull = IO_PULLUP;
    ret = BSP_IO_Init(0, &io_init_structure);
#endif
    if(ret == BSP_ERROR_NONE)
    {
      if(HAL_EXTI_GetHandle(&hts_exti[Instance], TS_EXTI_LINE[Instance]) != HAL_OK)
      {
        ret = BSP_ERROR_PERIPH_FAILURE;
      }
      else if(HAL_EXTI_RegisterCallback(&hts_exti[Instance],  HAL_EXTI_COMMON_CB_ID, TsCallback[Instance]) != HAL_OK)
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

  if(Instance >=TS_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
#if (USE_BSP_IO_CLASS > 0U)
    BSP_IO_Init_t    io_init_structure;

    /* Configure TS_INT_PIN low level to generate MFX_IRQ_OUT in EXTI on MCU            */
    io_init_structure.Pin  = TS_INT_PIN;
    io_init_structure.Pull = IO_PULLUP;
    io_init_structure.Mode = IO_MODE_OFF;
    if(BSP_IO_Init(0, &io_init_structure) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_NO_INIT;
    }
#endif
  }

  return ret;
}

/**
  * @brief  This function handles TS interrupt request.
  * @param  Instance TS instance
  * @retval None
  */
void BSP_TS_IRQHandler(uint32_t Instance)
{
  HAL_EXTI_IRQHandler(&hts_exti[Instance]);
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
    EXC7200_State_t state;

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
        x_oriented = Ts_Ctx[Instance].MaxX - state.TouchX - 1UL;
      }

      if((Ts_Ctx[Instance].Orientation & TS_SWAP_Y) == TS_SWAP_Y)
      {
        y_oriented = Ts_Ctx[Instance].MaxY - state.TouchY;
      }

      /* Apply boundary */
      TS_State->TouchX = (x_oriented * Ts_Ctx[Instance].Width) / Ts_Ctx[Instance].MaxX;
      TS_State->TouchY = (y_oriented * Ts_Ctx[Instance].Height) / Ts_Ctx[Instance].MaxY;
      /* Store Current TS state */
      TS_State->TouchDetected = state.TouchDetected;

      /* Check accuracy */
      x_diff = (TS_State->TouchX > Ts_Ctx[Instance].PreviousX[0])?
               (TS_State->TouchX - Ts_Ctx[Instance].PreviousX[0]):
               (Ts_Ctx[Instance].PreviousX[0] - TS_State->TouchX);

      y_diff = (TS_State->TouchY > Ts_Ctx[Instance].PreviousY[0])?
               (TS_State->TouchY - Ts_Ctx[Instance].PreviousY[0]):
               (Ts_Ctx[Instance].PreviousY[0] - TS_State->TouchY);


          if ((x_diff > Ts_Ctx[Instance].Accuracy) || (y_diff > Ts_Ctx[Instance].Accuracy))
      {
        /* New touch detected */
        Ts_Ctx[Instance].PreviousX[0] = TS_State->TouchX;
        Ts_Ctx[Instance].PreviousY[0] = TS_State->TouchY;
      }
      else
      {
            TS_State->TouchX = Ts_Ctx[Instance].PreviousX[0];
            TS_State->TouchY = Ts_Ctx[Instance].PreviousY[0];
      }
    }
    else
    {
      TS_State->TouchDetected = 0U;
      TS_State->TouchX = Ts_Ctx[Instance].PreviousX[0];
      TS_State->TouchY = Ts_Ctx[Instance].PreviousY[0];
    }
  }

  return ret;
}

/**
  * @brief  Set TS orientation
  * @param  Instance TS instance. Could be only 0.
  * @param  Orientation Orientation to be set
  * @retval BSP status
  */
int32_t BSP_TS_Set_Orientation(uint32_t Instance, uint32_t Orientation)
{
  Ts_Ctx[Instance].Orientation = Orientation;
  return BSP_ERROR_NONE;
}

/**
  * @brief  Get TS orientation
  * @param  Instance TS instance. Could be only 0.
  * @param  Orientation Current Orientation to be returned
  * @retval BSP status
  */
int32_t BSP_TS_Get_Orientation(uint32_t Instance, uint32_t *Orientation)
{
  *Orientation = Ts_Ctx[Instance].Orientation;
  return BSP_ERROR_NONE;
}
/**
  * @}
  */

/** @defgroup STM32H7B3I_Discovery_TS_Private_Functions TS Private Functions
  * @{
  */
#if (USE_EXC7200_TS_CTRL == 1U)
/**
  * @brief  Register Bus IOs if component ID is OK
  * @param  Instance TS instance. Could be only 0.
  * @retval BSP status
  */
static int32_t EXC7200_Probe(uint32_t Instance)
{
  int32_t ret               = BSP_ERROR_NONE;
  EXC7200_IO_t              IOCtx;
  static EXC7200_Object_t   EXC7200Obj;
  uint32_t exc7200_id       = 0U;

  /* Configure the touch screen driver */
  IOCtx.Address     = TS_EXC7200_I2C_ADDRESS;
  IOCtx.Init        = BSP_I2C1_Init;
  IOCtx.DeInit      = BSP_I2C1_DeInit;
  IOCtx.ReadReg     = BSP_I2C1_ReadReg;
  IOCtx.WriteReg    = BSP_I2C1_WriteReg;
  IOCtx.GetTick     = BSP_GetTick;

  if(EXC7200_RegisterBusIO(&EXC7200Obj, &IOCtx) != EXC7200_OK)
  {
    ret = BSP_ERROR_BUS_FAILURE;
  }
  else if(EXC7200_ReadID(&EXC7200Obj, &exc7200_id) != EXC7200_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else if(exc7200_id != EXC7200_ID)
  {
    ret = BSP_ERROR_UNKNOWN_COMPONENT;
  }
  else
  {
    Ts_CompObj[Instance] = &EXC7200Obj;
    Ts_Drv = (TS_Drv_t *) &EXC7200_TS_Driver;

    if(Ts_Drv->Init(Ts_CompObj[Instance]) != EXC7200_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  return ret;
}
#endif

#if (USE_TS3510_TS_CTRL == 1U)
/**
  * @brief  Register Bus IOs if component ID is OK
  * @param  Instance TS instance. Could be only 0.
  * @retval BSP status
  */
static int32_t TS3510_Probe(uint32_t Instance)
{
  int32_t ret              = BSP_ERROR_NONE;
  TS3510_IO_t              IOCtx;
  static TS3510_Object_t   TS3510Obj;
  uint32_t ts3510_id       = 0U;

  /* Configure the touch screen driver */
  IOCtx.Address     = TS_TS3510_I2C_ADDRESS;
  IOCtx.Init        = BSP_I2C1_Init;
  IOCtx.DeInit      = BSP_I2C1_DeInit;
  IOCtx.ReadReg     = BSP_I2C1_ReadReg;
  IOCtx.WriteReg    = BSP_I2C1_WriteReg;
  IOCtx.GetTick     = BSP_GetTick;

  if(TS3510_RegisterBusIO(&TS3510Obj, &IOCtx) != TS3510_OK)
  {
    ret = BSP_ERROR_BUS_FAILURE;
  }
  else if(TS3510_ReadID(&TS3510Obj, &ts3510_id) != TS3510_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else if(ts3510_id != TS3510_ID)
  {
    ret = BSP_ERROR_UNKNOWN_COMPONENT;
  }
  else
  {
    Ts_CompObj[Instance] = &TS3510Obj;
    Ts_Drv = (TS_Drv_t *) &TS3510_TS_Driver;

    if(Ts_Drv->Init(Ts_CompObj[Instance]) != TS3510_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  return ret;
}
#endif
#if (USE_EXC80W32_TS_CTRL == 1U)
/**
  * @brief  Register Bus IOs if component ID is OK
  * @param  Instance TS instance. Could be only 0.
  * @retval BSP status
  */
static int32_t EXC80W32_Probe(uint32_t Instance)
{
  int32_t ret                = BSP_ERROR_NONE;
  EXC80W32_IO_t              IOCtx;
  static EXC80W32_Object_t   EXC80W32Obj;
  uint32_t exc80w32_id       = 0U;

  /* Configure the touch screen driver */
  IOCtx.Address     = TS_EXC80W32_I2C_ADDRESS;
  IOCtx.Init        = BSP_I2C1_Init;
  IOCtx.DeInit      = BSP_I2C1_DeInit;
  IOCtx.ReadReg     = BSP_I2C1_ReadReg;
  IOCtx.WriteReg    = BSP_I2C1_WriteReg;
  IOCtx.GetTick     = BSP_GetTick;

  if(EXC80W32_RegisterBusIO(&EXC80W32Obj, &IOCtx) != EXC80W32_OK)
  {
    ret = BSP_ERROR_BUS_FAILURE;
  }
  else if(EXC80W32_ReadID(&EXC80W32Obj, &exc80w32_id) != EXC80W32_OK)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else if(exc80w32_id != EXC80W32_ID)
  {
    ret = BSP_ERROR_UNKNOWN_COMPONENT;
  }
  else
  {
    Ts_CompObj[Instance] = &EXC80W32Obj;
    Ts_Drv = (TS_Drv_t *) &EXC80W32_TS_Driver;

    if(Ts_Drv->Init(Ts_CompObj[Instance]) != TS3510_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  return ret;
}
#endif

/**
  * @brief  TS EXTI touch detection callbacks.
  * @retval None
  */
static void TS_EXTI_Callback(void)
{
  BSP_TS_Callback(0);

#if (USE_BSP_IO_CLASS > 0)
  (void)BSP_IO_ClearIT(0, TS_INT_PIN);
#endif /* ( USE_BSP_IO_CLASS > 0) */
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
