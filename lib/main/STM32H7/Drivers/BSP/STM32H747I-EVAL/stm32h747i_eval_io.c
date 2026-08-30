/**
  ******************************************************************************
  * @file    stm32h747i_eval_io.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the IO pins
  *          on STM32H747I_EVAL board.
  @verbatim
  How To use this driver:
  -----------------------
   - This driver is used to drive the IO module of the STM32H747I_EVAL board.
   - The MFXSTM32L152 IO expander device component driver must be included with this
     driver in order to run the IO functionalities commanded by the IO expander (MFX)
     device mounted on the board.

  Driver description:
  -------------------
  + Initialization steps:
     o Initialize the IO module using the BSP_IO_Init() function. This
       function includes the MSP layer hardware resources initialization and the
       communication layer configuration to start the IO functionalities use.

  + IO functionalities use
     o The IO pin mode is configured when calling the function BSP_IO_ConfigPin(), you
       must specify the desired IO mode by choosing the "IO_ModeTypedef" parameter
       predefined value.
     o If an IO pin is used in interrupt mode, the function BSP_IO_ITGetStatus() is
       needed to get the interrupt status. To clear the IT pending bits, you should
       call the function BSP_IO_ITClear() with specifying the IO pending bit to clear.
     o The IT is handled using the corresponding external interrupt IRQ handler,
       the user IT callback treatment is implemented on the same external interrupt
       callback.
     o The IRQ_OUT pin (common for all functionalities: JOY, SD, LEDs, etc)  can be
       configured using the function BSP_IO_ConfigIrqOutPin()
     o To get/set an IO pin combination state you can use the functions
       BSP_IO_ReadPin()/BSP_IO_WritePin() or the function BSP_IO_TogglePin() to toggle the pin
       state.
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
#include "stm32h747i_eval_io.h"
#include "stm32h747i_eval_bus.h"
/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H747I_EVAL
  * @{
  */

/** @defgroup STM32H747I_EVAL_IO IO
  * @{
  */

/** @defgroup STM32H747I_EVAL_IO_Exported_Variables Exported Variables
  * @{
  */
IOEXPANDER_Ctx_t                IO_Ctx[IOEXPANDER_INSTANCES_NBR] = {0};
MFXSTM32L152_Object_t           Io_CompObj;
EXTI_HandleTypeDef              hio_exti;
/**
  * @}
  */

/** @defgroup STM32H747I_EVAL_IO_Private_Variables Private Variables
  * @{
  */
static IO_Drv_t                 *Io_Drv = NULL;
/**
  * @}
  */

/** @defgroup STM32H747I_EVAL_IO_Private_Functions_Prototypes Private Functions Prototypes
  * @{
  */
static int32_t MFXSTM32L152_Probe(uint32_t Instance);
static void IO_EXTI_Callback(void);
/**
  * @}
  */

/** @defgroup STM32H747I_EVAL_IO_Exported_Functions Exported Functions
  * @{
  */

/**
  * @brief  Initializes and start the IOExpander component
  * @param  Instance IOE instance
  * @param  Function to be initialized. Could be IOEXPANDER_IO_MODE
  * @note   IOEXPANDER_IDD_MODE and IOEXPANDER_TS_MODE are not used on STM32H747I_EVAL board
  * @retval BSP status
  */
int32_t BSP_IOEXPANDER_Init(uint32_t Instance, uint32_t Function)
{
  int32_t ret = BSP_ERROR_NONE;

  if ((Instance >= IOEXPANDER_INSTANCES_NBR) || (Function != IOEXPANDER_IO_MODE))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (IO_Ctx[Instance].IsInitialized == 0U)
    {
      if (MFXSTM32L152_Probe(Instance) != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_NO_INIT;
      }
      else
      {
        IO_Ctx[Instance].IsInitialized = 1;
      }
    }

    if (IO_Ctx[Instance].IsInitialized == 1U)
    {
      if (Function == IOEXPANDER_IO_MODE)
      {
        Io_Drv = (IO_Drv_t *) &MFXSTM32L152_IO_Driver;
      }
      else
      {
        ret = BSP_ERROR_WRONG_PARAM;
      }
    }
  }

  return ret;
}

/**
  * @brief  De-Initializes the IOExpander component
  * @param  Instance IOE instance
  * @note   The de-init is common for all IOE functions
  * @retval BSP status
  */
int32_t BSP_IOEXPANDER_DeInit(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= IOEXPANDER_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (IO_Ctx[Instance].IsInitialized == 1U)
    {
      /* DeInit MFX if no more Functions on are going */
      if (IO_Ctx[Instance].Functions == 0U)
      {
        if (Io_Drv->DeInit(&Io_CompObj) < 0)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else
        {
          IO_Ctx[Instance].IsInitialized = 0;
        }
      }
    }
  }
  return ret;
}

/**
  * @brief  Initializes the IO peripheral according to the specified parameters in the BSP_IO_Init_t.
  * @param  Instance IOE instance
  * @param  Init     pointer to a BSP_IO_Init_t structure that contains
  *         the configuration information for the specified IO pin.
  * @retval BSP status
  */
int32_t BSP_IO_Init(uint32_t Instance, BSP_IO_Init_t *Init)
{
  int32_t ret = BSP_ERROR_NONE;

  if((Instance >= IOEXPANDER_INSTANCES_NBR) || (Init == NULL))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if((IO_Ctx[Instance].Functions & IOEXPANDER_IO_MODE) != IOEXPANDER_IO_MODE)
    {
      if(BSP_IOEXPANDER_Init(Instance, IOEXPANDER_IO_MODE) != BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else if (Io_Drv->Start(&Io_CompObj, IO_PIN_ALL) < 0)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else
      {
        IO_Ctx[Instance].Functions |= IOEXPANDER_IO_MODE;
      }
    }

    if(ret == BSP_ERROR_NONE)
    {
      /* If IT mode is selected, configures MFX low level interrupt */
      if(Init->Mode >= IO_MODE_IT_RISING_EDGE)
      {
        BSP_IOEXPANDER_ITConfig();
      }

      /* Initializes IO pin */
      if(Io_Drv->Init(&Io_CompObj, Init) < 0)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
    }
  }

  return ret;
}

/**
  * @brief  DeInitializes the IO peripheral
  * @param  Instance IOE instance
  * @retval BSP status
  */
int32_t BSP_IO_DeInit(uint32_t Instance)
{
  int32_t ret;

  if (Instance >= IOEXPANDER_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if ((IO_Ctx[Instance].Functions & IOEXPANDER_IO_MODE) != IOEXPANDER_IO_MODE)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    IO_Ctx[Instance].Functions &= ~(IOEXPANDER_IO_MODE);
    ret = BSP_IOEXPANDER_DeInit(Instance);
  }

  return ret;
}

/**
  * @brief  Gets the selected pins IT status.
  * @param  Instance IOE instance
  * @param  IoPin  Selected pins to check the status.
  *          This parameter can be any combination of the IO pins.
  * @retval Pin IT status or BSP_ERROR_WRONG_PARAM
  */
int32_t BSP_IO_GetIT(uint32_t Instance, uint32_t IoPin)
{
  if((Instance >= IOEXPANDER_INSTANCES_NBR) || ((IO_Ctx[Instance].Functions & IOEXPANDER_IO_MODE) != IOEXPANDER_IO_MODE))
  {
    return BSP_ERROR_WRONG_PARAM;
  }

  /* Return the IO Pin IT status */
  return (Io_Drv->ITStatus(&Io_CompObj, IoPin));
}

/**
  * @brief  Clear only one or a selection of IO IT pending bits.
  * @param  Instance IOE instance
  * @param  IO_Pins_To_Clear   MFX IRQ status IO pin to clear (or combination of several IOs)
  * @retval BSP status
  */
int32_t BSP_IO_ClearIT(uint32_t Instance, uint32_t IO_Pins_To_Clear)
{
  int32_t ret = BSP_ERROR_NONE;

  if((Instance >= IOEXPANDER_INSTANCES_NBR) || ((IO_Ctx[Instance].Functions & IOEXPANDER_IO_MODE) != IOEXPANDER_IO_MODE))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Clear only the selected list of IO IT pending bits */
    if(Io_Drv->ClearIT(&Io_CompObj, IO_Pins_To_Clear) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  Sets the selected pins state.
  * @param  Instance IOE instance
  * @param  IoPin  Selected pins to write.
  *          This parameter can be any combination of the IO pins.
  * @param  PinState  New pins state to write
  * @retval BSP status
  */
int32_t BSP_IO_WritePin(uint32_t Instance, uint32_t IoPin, uint32_t PinState)
{
  int32_t ret = BSP_ERROR_NONE;

  if((Instance >= IOEXPANDER_INSTANCES_NBR) || ((IO_Ctx[Instance].Functions & IOEXPANDER_IO_MODE) != IOEXPANDER_IO_MODE))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Set the Pin state */
    if(Io_Drv->WritePin(&Io_CompObj, IoPin, PinState) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  Gets the selected pins current state.
  * @param  Instance IOE instance
  * @param  IoPin  Selected pin to read.
  * @retval The current pins state or BSP_ERROR_WRONG_PARAM
  */
int32_t BSP_IO_ReadPin(uint32_t Instance, uint32_t IoPin)
{
  if((Instance >= IOEXPANDER_INSTANCES_NBR) || ((IO_Ctx[Instance].Functions & IOEXPANDER_IO_MODE) != IOEXPANDER_IO_MODE))
  {
    return BSP_ERROR_WRONG_PARAM;
  }

  return Io_Drv->ReadPin(&Io_CompObj, IoPin);
}

/**
  * @brief  Toggles the selected pins state.
  * @param  Instance IOE instance
  * @param  IoPin  Selected pins to toggle.
  *          This parameter can be any combination of the IO pins.
  * @note   This function is only used to toggle one pin in the same time
  * @retval None
  */
int32_t BSP_IO_TogglePin(uint32_t Instance, uint32_t IoPin)
{
  int32_t ret = BSP_ERROR_NONE;
  int32_t pinState;

  if((Instance >= IOEXPANDER_INSTANCES_NBR) || ((IO_Ctx[Instance].Functions & IOEXPANDER_IO_MODE) != IOEXPANDER_IO_MODE))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Toggle the current pin state */
    pinState = Io_Drv->ReadPin(&Io_CompObj, IoPin);
    if (pinState < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      if (pinState == 0) /* Reset state */
      {
        if (Io_Drv->WritePin(&Io_CompObj, IoPin, IO_PIN_SET) < 0)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
      }
      else /* Set state */
      {
        if (Io_Drv->WritePin(&Io_CompObj, IoPin, IO_PIN_RESET) < 0)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
      }
    }
  }

  return ret;
}

/**
  * @brief  Configures MFX low level interrupt.
  * @retval None
  */
void BSP_IOEXPANDER_ITConfig(void)
{
  static uint32_t mfx_io_it_enabled = 0U;
  GPIO_InitTypeDef  gpio_init_structure;

  if(mfx_io_it_enabled == 0U)
  {
    mfx_io_it_enabled = 1U;
    /* Enable the GPIO EXTI clock */
    MFX_IRQOUT_GPIO_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    gpio_init_structure.Pin   = MFX_IRQOUT_PIN;
    gpio_init_structure.Pull  = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;
    gpio_init_structure.Mode  = GPIO_MODE_IT_RISING;
    HAL_GPIO_Init(MFX_IRQOUT_GPIO_PORT, &gpio_init_structure);
    (void)HAL_EXTI_GetHandle(&hio_exti, IO_EXTI_LINE);
    (void)HAL_EXTI_RegisterCallback(&hio_exti,  HAL_EXTI_COMMON_CB_ID, IO_EXTI_Callback);

    /* Enable and set GPIO EXTI Interrupt to the lowest priority */
    HAL_NVIC_SetPriority((IRQn_Type)(MFX_IRQOUT_EXTI_IRQn), BSP_IOEXPANDER_IT_PRIORITY, 0x0F);
    HAL_NVIC_EnableIRQ((IRQn_Type)(MFX_IRQOUT_EXTI_IRQn));
  }
}

/**
  * @brief  This function handles IO interrupt request.
  * @param  Instance IO instance
  * @retval None
  */
void BSP_IO_IRQHandler(uint32_t Instance)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);

  HAL_EXTI_IRQHandler(&hio_exti);
}

/**
  * @brief  BSP TS Callback.
  * @param  Instance IO instance
  * @retval None.
  */
__weak void BSP_IO_Callback(uint32_t Instance)
{
  /* This function should be implemented by the user application.
     It is called into this driver when an event on TS touch detection */
}

/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_IO_Private_Functions Private Functions
  * @{
  */

/**
  * @brief  Register Bus IOs if component ID is OK
  * @param  Instance IO instance
  * @retval error status
  */
static int32_t MFXSTM32L152_Probe(uint32_t Instance)
{
  int32_t              ret = BSP_ERROR_NONE;
  MFXSTM32L152_IO_t    IOCtx;
  uint32_t             mfxstm32l152_id, i;
  uint8_t i2c_address[] = {IO_I2C_ADDRESS, IO_I2C_ADDRESS_2};

  /* Configure the audio driver */
  IOCtx.Init        = BSP_I2C1_Init;
  IOCtx.DeInit      = BSP_I2C1_DeInit;
  IOCtx.ReadReg     = BSP_I2C1_ReadReg;
  IOCtx.WriteReg    = BSP_I2C1_WriteReg;
  IOCtx.GetTick     = BSP_GetTick;

  for(i = 0U; i < 2U; i++)
  {
    IOCtx.Address     = (uint16_t)i2c_address[i];
    if(MFXSTM32L152_RegisterBusIO (&Io_CompObj, &IOCtx) != MFXSTM32L152_OK)
    {
      ret = BSP_ERROR_BUS_FAILURE;
    }
    else if(MFXSTM32L152_ReadID(&Io_CompObj, &mfxstm32l152_id) != MFXSTM32L152_OK)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }
    else
    {
      if((mfxstm32l152_id == MFXSTM32L152_ID) || (mfxstm32l152_id == MFXSTM32L152_ID_2))
      {
        if(MFXSTM32L152_Init(&Io_CompObj) != MFXSTM32L152_OK)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        break;
      }
      else
      {
        ret = BSP_ERROR_UNKNOWN_COMPONENT;
      }
    }
  }

  return ret;
}

/**
  * @brief  IO EXTI touch detection callbacks.
  * @retval None
  */
static void IO_EXTI_Callback(void)
{
  BSP_IO_Callback(0);
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
