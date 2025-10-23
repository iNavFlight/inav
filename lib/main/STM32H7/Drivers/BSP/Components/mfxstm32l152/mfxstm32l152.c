/**
  ******************************************************************************
  * @file    mfxstm32l152.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the MFXSTM32L152
  *          IO Expander devices.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "mfxstm32l152.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @defgroup MFXSTM32L152 MFXSTM32L152
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/** @defgroup MFXSTM32L152_Private_Types_Definitions MFXSTM32L152 Private Types Definitions
  * @{
  */

/* Touch screen driver structure initialization */
MFXSTM32L152_TS_Mode_t MFXSTM32L152_TS_Driver =
{
  MFXSTM32L152_Init,
  MFXSTM32L152_ReadID,
  MFXSTM32L152_Reset,
  MFXSTM32L152_TS_Start,
  MFXSTM32L152_TS_DetectTouch,
  MFXSTM32L152_TS_GetXY,
  MFXSTM32L152_TS_EnableIT,
  MFXSTM32L152_TS_ClearIT,
  MFXSTM32L152_TS_ITStatus,
  MFXSTM32L152_TS_DisableIT,
};

/* IO driver structure initialization */
MFXSTM32L152_IO_Mode_t MFXSTM32L152_IO_Driver =
{
  MFXSTM32L152_IO_Init,
  MFXSTM32L152_DeInit,
  MFXSTM32L152_ReadID,
  MFXSTM32L152_Reset,
  MFXSTM32L152_IO_Start,
  MFXSTM32L152_IO_WritePin,
  MFXSTM32L152_IO_ReadPin,
  MFXSTM32L152_IO_EnableIT,
  MFXSTM32L152_IO_DisableIT,
  MFXSTM32L152_IO_ITStatus,
  MFXSTM32L152_IO_ClearIT,
};

/* IDD driver structure initialization */
MFXSTM32L152_IDD_Mode_t MFXSTM32L152_IDD_Driver =
{
  MFXSTM32L152_Init,
  MFXSTM32L152_DeInit,
  MFXSTM32L152_ReadID,
  MFXSTM32L152_Reset,
  MFXSTM32L152_LowPower,
  MFXSTM32L152_WakeUp,
  MFXSTM32L152_IDD_Start,
  MFXSTM32L152_IDD_Config,
  MFXSTM32L152_IDD_GetValue,
  MFXSTM32L152_IDD_EnableIT,
  MFXSTM32L152_IDD_DisableIT,
  MFXSTM32L152_IDD_GetITStatus,
  MFXSTM32L152_IDD_ClearIT,
  MFXSTM32L152_Error_EnableIT,
  MFXSTM32L152_Error_ClearIT,
  MFXSTM32L152_Error_GetITStatus,
  MFXSTM32L152_Error_DisableIT,
  MFXSTM32L152_Error_ReadSrc,
  MFXSTM32L152_Error_ReadMsg
};

/**
  * @}
  */

/* Private function prototypes -----------------------------------------------*/

/** @defgroup MFXSTM32L152_Private_Function_Prototypes MFXSTM32L152 Private Function Prototypes
  * @{
  */
static int32_t MFXSTM32L152_reg24_setPinValue(MFXSTM32L152_Object_t *pObj, uint8_t RegisterAddr, uint32_t PinPosition, uint8_t PinValue );
static int32_t MFXSTM32L152_ReadRegWrap(void *handle, uint16_t Reg, uint8_t* Data, uint16_t Length);
static int32_t MFXSTM32L152_WriteRegWrap(void *handle, uint16_t Reg, uint8_t* Data, uint16_t Length);
/**
  * @}
  */

/* Private functions ---------------------------------------------------------*/

/** @defgroup MFXSTM32L152_Exported_Functions MFXSTM32L152 Exported Functions
  * @{
  */

/**
  * @brief  Initialize the mfxstm32l152 and configure the needed hardware resources
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_Init(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;

  if(pObj->IsInitialized == 0U)
  {
    /* Initialize IO BUS layer */
    pObj->IO.Init();

    if(MFXSTM32L152_SetIrqOutPinPolarity(pObj, MFXSTM32L152_OUT_PIN_POLARITY_HIGH) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
    else if(MFXSTM32L152_SetIrqOutPinType(pObj, MFXSTM32L152_OUT_PIN_TYPE_PUSHPULL) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
    else
    {
      pObj->IsInitialized = 1U;
    }
  }

  return ret;
}

/**
  * @brief  DeInitialize the mfxstm32l152 and unconfigure the needed hardware resources
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_DeInit(MFXSTM32L152_Object_t *pObj)
{
  if(pObj->IsInitialized == 1U)
  {
    pObj->IsInitialized = 0U;
  }
  return MFXSTM32L152_OK;
}

/**
  * @brief  Reset the mfxstm32l152 by Software.
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_Reset(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t tmp = MFXSTM32L152_SWRST;

  /* Soft Reset */
  if(mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &tmp, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Put mfxstm32l152 Device in Low Power standby mode
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_LowPower(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t tmp = MFXSTM32L152_STANDBY;

  /* Enter standby mode */
  if(mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &tmp, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  WakeUp mfxstm32l152 from standby mode
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_WakeUp(MFXSTM32L152_Object_t *pObj)
{
  /* Prevent unused argument(s) compilation warning */
  (void)(pObj);

  return MFXSTM32L152_OK;
}

/**
  * @brief  Read the MFXSTM32L152 IO Expander device ID.
  * @param  pObj   Pointer to component object.
  * @retval The Device ID (two bytes).
  */
int32_t MFXSTM32L152_ReadID(MFXSTM32L152_Object_t *pObj, uint32_t *Id)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t id;

  /* Initialize IO BUS layer */
  pObj->IO.Init();

  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_ID, &id, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
  /* Store the device ID value */
  *Id = id;
  }

  return ret;
}

/**
  * @brief  Read the MFXSTM32L152 device firmware version.
  * @param  pObj   Pointer to component object.
  * @retval The Device FW version (two bytes).
  */
int32_t MFXSTM32L152_ReadFwVersion(MFXSTM32L152_Object_t *pObj)
{
  uint8_t  data[2];
  uint32_t ret;

  pObj->IO.ReadReg(pObj->IO.Address, MFXSTM32L152_REG_ADR_FW_VERSION_MSB, data, sizeof(data));

  /* Recompose MFX firmware value */
  ret = (((uint32_t)data[0] << 8) | (uint32_t)data[1]);

  return (int32_t)ret;
}

/**
  * @brief  Enable the interrupt mode for the selected IT source
  * @param  pObj   Pointer to component object.
  * @param Source: The interrupt source to be configured, could be:
  *   @arg  MFXSTM32L152_IRQ_GPIO: IO interrupt
  *   @arg  MFXSTM32L152_IRQ_IDD : IDD interrupt
  *   @arg  MFXSTM32L152_IRQ_ERROR : Error interrupt
  *   @arg  MFXSTM32L152_IRQ_TS_DET : Touch Screen Controller Touch Detected interrupt
  *   @arg  MFXSTM32L152_IRQ_TS_NE : Touch Screen FIFO Not Empty
  *   @arg  MFXSTM32L152_IRQ_TS_TH : Touch Screen FIFO threshold triggered
  *   @arg  MFXSTM32L152_IRQ_TS_FULL : Touch Screen FIFO Full
  *   @arg  MFXSTM32L152_IRQ_TS_OVF : Touch Screen FIFO Overflow
  * @retval Component status
  */
int32_t MFXSTM32L152_EnableITSource(MFXSTM32L152_Object_t *pObj, uint8_t Source)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t tmp;

  /* Get the current value of the INT_EN register */
  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_SRC_EN, &tmp, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    /* Set the interrupts to be Enabled */
    tmp |= Source;

    /* Set the register */
    if(mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_SRC_EN, &tmp, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Disable the interrupt mode for the selected IT source
  * @param  pObj   Pointer to component object.
  * @param  Source: The interrupt source to be configured, could be:
  *   @arg  MFXSTM32L152_IRQ_GPIO: IO interrupt
  *   @arg  MFXSTM32L152_IRQ_IDD : IDD interrupt
  *   @arg  MFXSTM32L152_IRQ_ERROR : Error interrupt
  *   @arg  MFXSTM32L152_IRQ_TS_DET : Touch Screen Controller Touch Detected interrupt
  *   @arg  MFXSTM32L152_IRQ_TS_NE : Touch Screen FIFO Not Empty
  *   @arg  MFXSTM32L152_IRQ_TS_TH : Touch Screen FIFO threshold triggered
  *   @arg  MFXSTM32L152_IRQ_TS_FULL : Touch Screen FIFO Full
  *   @arg  MFXSTM32L152_IRQ_TS_OVF : Touch Screen FIFO Overflow
  * @retval Component status
  */
int32_t MFXSTM32L152_DisableITSource(MFXSTM32L152_Object_t *pObj, uint8_t Source)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t tmp;

  /* Get the current value of the INT_EN register */
  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_SRC_EN, &tmp, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    /* Set the interrupts to be Enabled */
    tmp &= ~Source;

    /* Set the register */
    if(mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_SRC_EN, &tmp, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }

  return ret;
}


/**
  * @brief  Returns the selected Global interrupt source pending bit value
  * @param  pObj   Pointer to component object.
  * @param  Source: the Global interrupt source to be checked, could be:
  *   @arg  MFXSTM32L152_IRQ_GPIO: IO interrupt
  *   @arg  MFXSTM32L152_IRQ_IDD : IDD interrupt
  *   @arg  MFXSTM32L152_IRQ_ERROR : Error interrupt
  *   @arg  MFXSTM32L152_IRQ_TS_DET : Touch Screen Controller Touch Detected interrupt
  *   @arg  MFXSTM32L152_IRQ_TS_NE : Touch Screen FIFO Not Empty
  *   @arg  MFXSTM32L152_IRQ_TS_TH : Touch Screen FIFO threshold triggered
  *   @arg  MFXSTM32L152_IRQ_TS_FULL : Touch Screen FIFO Full
  *   @arg  MFXSTM32L152_IRQ_TS_OVF : Touch Screen FIFO Overflow
  * @retval The value of the checked Global interrupt source status.
  */
int32_t MFXSTM32L152_GlobalITStatus(MFXSTM32L152_Object_t *pObj, uint8_t Source)
{
  int32_t ret;
  uint8_t tmp, tmp1;

  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_PENDING, &tmp, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    tmp1 = tmp & Source;
    ret = (int32_t)tmp1;
  }

  /* Return the global IT source status (pending or not) if no error */
  return ret;
}

/**
  * @brief  Clear the selected Global interrupt pending bit(s)
  * @param  pObj   Pointer to component object.
  * @param  Source: the Global interrupt source to be cleared, could be any combination
  *         of the below values. The acknowledge signal for MFXSTM32L152_GPIOs configured in input
  *         with interrupt is not on this register but in IRQ_GPI_ACK1, IRQ_GPI_ACK2 registers.
  *   @arg  MFXSTM32L152_IRQ_IDD : IDD interrupt
  *   @arg  MFXSTM32L152_IRQ_ERROR : Error interrupt
  *   @arg  MFXSTM32L152_IRQ_TS_DET : Touch Screen Controller Touch Detected interrupt
  *   @arg  MFXSTM32L152_IRQ_TS_NE : Touch Screen FIFO Not Empty
  *   @arg  MFXSTM32L152_IRQ_TS_TH : Touch Screen FIFO threshold triggered
  *   @arg  MFXSTM32L152_IRQ_TS_FULL : Touch Screen FIFO Full
  *   @arg  MFXSTM32L152_IRQ_TS_OVF : Touch Screen FIFO Overflow
  *  /\/\ IMPORTANT NOTE /\/\ must not use MFXSTM32L152_IRQ_GPIO as argument, see IRQ_GPI_ACK1 and IRQ_GPI_ACK2 registers
  * @retval Component status
  */
int32_t MFXSTM32L152_ClearGlobalIT(MFXSTM32L152_Object_t *pObj, uint8_t Source)
{
  int32_t ret = MFXSTM32L152_OK;

  /* Write 1 to the bits that have to be cleared */
  if(mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_ACK, &Source, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Set the global interrupt Polarity of IRQ_OUT_PIN.
  * @param  pObj   Pointer to component object.
  * @param  Polarity: the IT mode polarity, could be one of the following values:
  *   @arg  MFXSTM32L152_OUT_PIN_POLARITY_LOW: Interrupt output line is active Low edge
  *   @arg  MFXSTM32L152_OUT_PIN_POLARITY_HIGH: Interrupt line output is active High edge
  * @retval Component status
  */
int32_t MFXSTM32L152_SetIrqOutPinPolarity(MFXSTM32L152_Object_t *pObj, uint8_t Polarity)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t tmp;

  /* Get the current register value */
  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_MFX_IRQ_OUT, &tmp, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
  /* Mask the polarity bits */
  tmp &= ~(uint8_t)0x02;

  /* Modify the Interrupt Output line configuration */
  tmp |= Polarity;

  /* Set the new register value */
  if(mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_MFX_IRQ_OUT, &tmp, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  }

  return ret;
}

/**
  * @brief  Set the global interrupt Type of IRQ_OUT_PIN.
  * @param  pObj   Pointer to component object.
  * @param  Type: Interrupt line activity type, could be one of the following values:
  *   @arg  MFXSTM32L152_OUT_PIN_TYPE_OPENDRAIN: Open Drain output Interrupt line
  *   @arg  MFXSTM32L152_OUT_PIN_TYPE_PUSHPULL: Push Pull output Interrupt line
  * @retval Component status
  */
int32_t MFXSTM32L152_SetIrqOutPinType(MFXSTM32L152_Object_t *pObj, uint8_t Type)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t tmp;

  /* Get the current register value */
  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_MFX_IRQ_OUT, &tmp, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    /* Mask the type bits */
    tmp &= ~(uint8_t)0x01;

    /* Modify the Interrupt Output line configuration */
    tmp |= Type;

    /* Set the new register value */
    if(mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_MFX_IRQ_OUT, &tmp, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }

  return ret;
}


/* ------------------------------------------------------------------ */
/* ----------------------- GPIO ------------------------------------- */
/* ------------------------------------------------------------------ */


/**
  * @brief  Start the IO functionality used and enable the AF for selected IO pin(s).
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin IO pin
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_Start(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t mode;

  /* Get the current register value */
  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &mode, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    /* Set the IO Functionalities to be Enabled */
    mode |= MFXSTM32L152_GPIO_EN;

    /* Enable ALTERNATE functions */
    /* AGPIO[0..3] can be either IDD or GPIO */
    /* AGPIO[4..7] can be either TS or GPIO */
    /* if IDD or TS are enabled no matter the value this bit GPIO are not available for those pins */
    /*  however the MFX will waste some cycles to to handle these potential GPIO (pooling, etc) */
    /* so if IDD and TS are both active it is better to let ALTERNATE off (0) */
    /* if however IDD or TS are not connected then set it on gives more GPIOs availability */
    /* remind that AGPIO are less efficient then normal GPIO (They use pooling rather then EXTI */
    if (IO_Pin > 0xFFFFU)
    {
      mode |= MFXSTM32L152_ALTERNATE_GPIO_EN;
    }
    else
    {
      mode &= ~MFXSTM32L152_ALTERNATE_GPIO_EN;
    }

    /* Write the new register value */
    if(mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &mode, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Initializes the IO peripheral according to the specified parameters in the MFXSTM32L152_IO_Init_t.
  * @param  pObj   Pointer to component object.
  * @param  IoInit Pointer to a BSP_IO_Init_t structure that contains
  *         the configuration information for the specified IO pin.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_Init(MFXSTM32L152_Object_t *pObj, MFXSTM32L152_IO_Init_t *IoInit)
{
  int32_t ret = MFXSTM32L152_OK;

  /* IT enable/disable */
  switch(IoInit->Mode)
  {
  case MFXSTM32L152_GPIO_MODE_OFF:
  case MFXSTM32L152_GPIO_MODE_ANALOG:
  case MFXSTM32L152_GPIO_MODE_INPUT:
  case MFXSTM32L152_GPIO_MODE_OUTPUT_OD:
  case MFXSTM32L152_GPIO_MODE_OUTPUT_PP:
    ret += MFXSTM32L152_IO_DisablePinIT(pObj, IoInit->Pin); /* first disable IT */
    break;

  case MFXSTM32L152_GPIO_MODE_IT_RISING_EDGE:
  case MFXSTM32L152_GPIO_MODE_IT_FALLING_EDGE:
  case MFXSTM32L152_GPIO_MODE_IT_LOW_LEVEL:
  case MFXSTM32L152_GPIO_MODE_IT_HIGH_LEVEL:
    ret += MFXSTM32L152_IO_EnableIT(pObj); /* first enable IT */
    break;
  default:
    break;
  }

  /* Set direction IN/OUT */
  if((IoInit->Mode == MFXSTM32L152_GPIO_MODE_OUTPUT_PP) || (IoInit->Mode == MFXSTM32L152_GPIO_MODE_OUTPUT_OD))
  {
    ret += MFXSTM32L152_IO_InitPin(pObj, IoInit->Pin, MFXSTM32L152_GPIO_DIR_OUT);
  }
  else
  {
    ret += MFXSTM32L152_IO_InitPin(pObj, IoInit->Pin, MFXSTM32L152_GPIO_DIR_IN);
  }

  /* Set Push-Pull type */
  switch(IoInit->Pull)
  {
  case MFXSTM32L152_GPIO_NOPULL:
    ret += MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPIO_TYPE1, IoInit->Pin, MFXSTM32L152_GPI_WITHOUT_PULL_RESISTOR);
    break;
  case MFXSTM32L152_GPIO_PULLUP:
  case MFXSTM32L152_GPIO_PULLDOWN:
    ret += MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPIO_TYPE1, IoInit->Pin, MFXSTM32L152_GPI_WITH_PULL_RESISTOR);
    break;
  default:
    break;
  }

  if(IoInit->Mode == MFXSTM32L152_GPIO_MODE_OUTPUT_PP)
  {
    ret += MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPIO_TYPE1, IoInit->Pin, MFXSTM32L152_GPO_PUSH_PULL);
  }

  if(IoInit->Mode == MFXSTM32L152_GPIO_MODE_OUTPUT_OD)
  {
    ret += MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPIO_TYPE1, IoInit->Pin, MFXSTM32L152_GPO_OPEN_DRAIN);
  }

  /* Set Pullup-Pulldown */
  switch(IoInit->Pull)
  {
  case MFXSTM32L152_GPIO_NOPULL:
    if((IoInit->Mode == MFXSTM32L152_GPIO_MODE_INPUT) || (IoInit->Mode == MFXSTM32L152_GPIO_MODE_ANALOG))
    {
      ret += MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPIO_PUPD1, IoInit->Pin, MFXSTM32L152_GPIO_PULL_DOWN);
    }
    else
    {
      ret += MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPIO_PUPD1, IoInit->Pin, MFXSTM32L152_GPIO_PULL_UP);
    }
    break;
  case MFXSTM32L152_GPIO_PULLUP:
    ret += MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPIO_PUPD1, IoInit->Pin, MFXSTM32L152_GPIO_PULL_UP);
    break;
  case MFXSTM32L152_GPIO_PULLDOWN:
    ret += MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPIO_PUPD1, IoInit->Pin, MFXSTM32L152_GPIO_PULL_DOWN);
    break;
  default:
    break;
  }

  /* Set Irq event and type mode */
  switch(IoInit->Mode)
  {
  case MFXSTM32L152_GPIO_MODE_IT_RISING_EDGE:
    ret += MFXSTM32L152_IO_SetIrqEvtMode(pObj, IoInit->Pin, MFXSTM32L152_IRQ_GPI_EVT_EDGE);
    ret += MFXSTM32L152_IO_SetIrqTypeMode(pObj, IoInit->Pin, MFXSTM32L152_IRQ_GPI_TYPE_HLRE);
    ret += MFXSTM32L152_IO_EnablePinIT(pObj, IoInit->Pin);  /* last to do: enable IT */
    break;
  case MFXSTM32L152_GPIO_MODE_IT_FALLING_EDGE:
    ret += MFXSTM32L152_IO_SetIrqEvtMode(pObj, IoInit->Pin, MFXSTM32L152_IRQ_GPI_EVT_EDGE);
    ret += MFXSTM32L152_IO_SetIrqTypeMode(pObj, IoInit->Pin, MFXSTM32L152_IRQ_GPI_TYPE_LLFE);
    ret += MFXSTM32L152_IO_EnablePinIT(pObj, IoInit->Pin);  /* last to do: enable IT */
    break;
  case MFXSTM32L152_GPIO_MODE_IT_HIGH_LEVEL:
    ret += MFXSTM32L152_IO_SetIrqEvtMode(pObj, IoInit->Pin, MFXSTM32L152_IRQ_GPI_EVT_LEVEL);
    ret += MFXSTM32L152_IO_SetIrqTypeMode(pObj, IoInit->Pin, MFXSTM32L152_IRQ_GPI_TYPE_HLRE);
    ret += MFXSTM32L152_IO_EnablePinIT(pObj, IoInit->Pin);  /* last to do: enable IT */
    break;
  case MFXSTM32L152_GPIO_MODE_IT_LOW_LEVEL:
    ret += MFXSTM32L152_IO_SetIrqEvtMode(pObj, IoInit->Pin, MFXSTM32L152_IRQ_GPI_EVT_LEVEL);
    ret += MFXSTM32L152_IO_SetIrqTypeMode(pObj, IoInit->Pin, MFXSTM32L152_IRQ_GPI_TYPE_LLFE);
    ret += MFXSTM32L152_IO_EnablePinIT(pObj, IoInit->Pin);  /* last to do: enable IT */
    break;
  default:
    break;
  }

  if(ret != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Initialize the selected IO pin direction.
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin The IO pin to be configured. This parameter could be any
  *         combination of the following values:
  *   @arg  MFXSTM32L152_GPIO_PIN_x: Where x can be from 0 to 23.
  * @param  Direction could be MFXSTM32L152_GPIO_DIR_IN or MFXSTM32L152_GPIO_DIR_OUT.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_InitPin(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin, uint8_t Direction)
{
  int32_t ret = MFXSTM32L152_OK;

  if(MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPIO_DIR1, IO_Pin, Direction) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Set the global interrupt Type.
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin The IO pin to be configured. This parameter could be any
  *         combination of the following values:
  *   @arg  MFXSTM32L152_GPIO_PIN_x: Where x can be from 0 to 23.
  * @param  Evt: Interrupt line activity type, could be one of the following values:
  *   @arg  MFXSTM32L152_IRQ_GPI_EVT_LEVEL: Interrupt line is active in level model
  *   @arg  MFXSTM32L152_IRQ_GPI_EVT_EDGE: Interrupt line is active in edge model
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_SetIrqEvtMode(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin, uint8_t Evt)
{
  int32_t ret = MFXSTM32L152_OK;

  if(MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_IRQ_GPI_EVT1, IO_Pin, Evt) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Configure the Edge for which a transition is detectable for the
  *         selected pin.
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin The IO pin to be configured. This parameter could be any
  *         combination of the following values:
  *   @arg  MFXSTM32L152_GPIO_PIN_x: Where x can be from 0 to 23.
  * @param  Evt: Interrupt line activity type, could be one of the following values:
  *   @arg  MFXSTM32L152_IRQ_GPI_TYPE_LLFE: Interrupt line is active in Low Level or Falling Edge
  *   @arg  MFXSTM32L152_IRQ_GPI_TYPE_HLRE: Interrupt line is active in High Level or Rising Edge
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_SetIrqTypeMode(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin, uint8_t Type)
{
  int32_t ret = MFXSTM32L152_OK;

  if(MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_IRQ_GPI_TYPE1, IO_Pin, Type) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  When GPIO is in output mode, puts the corresponding GPO in High (1) or Low (0) level.
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin The output pin to be set or reset. This parameter can be one
  *         of the following values:
  *   @arg  MFXSTM32L152_GPIO_PIN_x: where x can be from 0 to 23.
  * @param PinState: The new IO pin state.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_WritePin(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin, uint8_t PinState)
{
  int32_t ret = MFXSTM32L152_OK;

  /* Apply the bit value to the selected pin */
  if (PinState != 0U)
  {
    /* Set the SET register */
    if(MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPO_SET1, IO_Pin, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }
  else
  {
    /* Set the CLEAR register */
    if(MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_GPO_CLR1, IO_Pin, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Return the state of the selected IO pin(s).
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin The output pin to read its state. This parameter can be one
  *         of the following values:
  *   @arg  MFXSTM32L152_GPIO_PIN_x: where x can be from 0 to 23.
  * @retval IO pin(s) state.
  */
int32_t MFXSTM32L152_IO_ReadPin(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin)
{
  uint8_t tmpreg[3];
  uint32_t tmp;

  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_GPIO_STATE1, &tmpreg[0], 1) != MFXSTM32L152_OK)
  {
    return MFXSTM32L152_ERROR;
  }
  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_GPIO_STATE2, &tmpreg[1], 1) != MFXSTM32L152_OK)
  {
    return MFXSTM32L152_ERROR;
  }
  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_GPIO_STATE3, &tmpreg[2], 1) != MFXSTM32L152_OK)
  {
    return MFXSTM32L152_ERROR;
  }
  tmp = ((uint32_t)tmpreg[0] + ((uint32_t)tmpreg[1] << 8) + ((uint32_t)tmpreg[2] << 16));
  tmp &= IO_Pin;

  return(int32_t)(tmp);
}

/**
  * @brief  Enable the global IO interrupt source.
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_EnableIT(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;

  /* Enable global IO IT source */
  if(MFXSTM32L152_EnableITSource(pObj, MFXSTM32L152_IRQ_GPIO) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Disable the global IO interrupt source.
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_DisableIT(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;

  /* Disable global IO IT source */
  if(MFXSTM32L152_DisableITSource(pObj, MFXSTM32L152_IRQ_GPIO) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Enable interrupt mode for the selected IO pin(s).
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin The IO interrupt to be enabled. This parameter could be any
  *         combination of the following values:
  *   @arg  MFXSTM32L152_GPIO_PIN_x: where x can be from 0 to 23.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_EnablePinIT(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin)
{
  int32_t ret = MFXSTM32L152_OK;

  if(MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_IRQ_GPI_SRC1, IO_Pin, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Disable interrupt mode for the selected IO pin(s).
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin The IO interrupt to be disabled. This parameter could be any
  *         combination of the following values:
  *   @arg  MFXSTM32L152_GPIO_PIN_x: where x can be from 0 to 23.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_DisablePinIT(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin)
{
  int32_t ret = MFXSTM32L152_OK;

  if(MFXSTM32L152_reg24_setPinValue(pObj, MFXSTM32L152_REG_ADR_IRQ_GPI_SRC1, IO_Pin, 0) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}


/**
  * @brief  Check the status of the selected IO interrupt pending bit
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin The IO interrupt to be checked could be:
  *   @arg  MFXSTM32L152_GPIO_PIN_x Where x can be from 0 to 23.
  * @retval Status of the checked IO pin(s).
  */
int32_t MFXSTM32L152_IO_ITStatus(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin)
{
  /* Get the Interrupt status */
  uint8_t tmpreg[3];
  uint32_t tmp;

  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_GPI_PENDING1, &tmpreg[0], 1) != MFXSTM32L152_OK)
  {
    return MFXSTM32L152_ERROR;
  }

  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_GPI_PENDING2, &tmpreg[1], 1) != MFXSTM32L152_OK)
  {
    return MFXSTM32L152_ERROR;
  }

  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_GPI_PENDING3, &tmpreg[2], 1) != MFXSTM32L152_OK)
  {
    return MFXSTM32L152_ERROR;
  }

  tmp = (uint32_t)tmpreg[0] + ((uint32_t)tmpreg[1] << 8) + ((uint32_t)tmpreg[2] << 16);
  tmp &= IO_Pin;

  return(int32_t)tmp;
}

/**
  * @brief  Clear the selected IO interrupt pending bit(s). It clear automatically also the general MFXSTM32L152_REG_ADR_IRQ_PENDING
  * @param  pObj   Pointer to component object.
  * @param  IO_Pin the IO interrupt to be cleared, could be:
  *   @arg  MFXSTM32L152_GPIO_PIN_x: Where x can be from 0 to 23.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_ClearIT(MFXSTM32L152_Object_t *pObj, uint32_t IO_Pin)
{
  /* Clear the IO IT pending bit(s) by acknowledging */
  /* it cleans automatically also the Global IRQ_GPIO */
  /* normally this function is called under interrupt */
  uint8_t pin_0_7, pin_8_15, pin_16_23;

  pin_0_7   = (uint8_t)(IO_Pin & 0x0000ffU);
  pin_8_15  = (uint8_t)(IO_Pin >> 8);
  pin_8_15  = (uint8_t)(pin_8_15 & 0x00ffU);
  pin_16_23 = (uint8_t)(IO_Pin >> 16);

  if (pin_0_7 != 0U)
  {
    if(mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_GPI_ACK1, &pin_0_7, 1) != MFXSTM32L152_OK)
    {
      return MFXSTM32L152_ERROR;
    }
  }
  if (pin_8_15 != 0U)
  {
    if(mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_GPI_ACK2, &pin_8_15, 1) != MFXSTM32L152_OK)
    {
      return MFXSTM32L152_ERROR;
    }
  }
  if (pin_16_23 != 0U)
  {
    if(mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IRQ_GPI_ACK3, &pin_16_23, 1) != MFXSTM32L152_OK)
    {
      return MFXSTM32L152_ERROR;
    }
  }

  return MFXSTM32L152_OK;
}


/**
  * @brief  Enable the AF for aGPIO.
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_EnableAF(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t mode;

  /* Get the current register value */
  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &mode, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    /* Enable ALTERNATE functions */
    /* AGPIO[0..3] can be either IDD or GPIO */
    /* AGPIO[4..7] can be either TS or GPIO */
    /* if IDD or TS are enabled no matter the value this bit GPIO are not available for those pins */
    /*  however the MFX will waste some cycles to to handle these potential GPIO (pooling, etc) */
    /* so if IDD and TS are both active it is better to let ALTERNATE disabled (0) */
    /* if however IDD or TS are not connected then set it on gives more GPIOs availability */
    /* remind that AGPIO are less efficient then normal GPIO (they use pooling rather then EXTI) */
    mode |= MFXSTM32L152_ALTERNATE_GPIO_EN;

    /* Write the new register value */
    if(mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &mode, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Disable the AF for aGPIO.
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_IO_DisableAF(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t mode;

  /* Get the current register value */
  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &mode, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    /* Enable ALTERNATE functions */
    /* AGPIO[0..3] can be either IDD or GPIO */
    /* AGPIO[4..7] can be either TS or GPIO */
    /* if IDD or TS are enabled no matter the value this bit GPIO are not available for those pins */
    /*  however the MFX will waste some cycles to to handle these potential GPIO (pooling, etc) */
    /* so if IDD and TS are both active it is better to let ALTERNATE disabled (0) */
    /* if however IDD or TS are not connected then set it on gives more GPIOs availability */
    /* remind that AGPIO are less efficient then normal GPIO (they use pooling rather then EXTI) */
    mode &= ~MFXSTM32L152_ALTERNATE_GPIO_EN;

    /* Write the new register value */
    if(mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &mode, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }

  return ret;
}


/* ------------------------------------------------------------------ */
/* --------------------- TOUCH SCREEN ------------------------------- */
/* ------------------------------------------------------------------ */

/**
  * @brief  Configures the touch Screen Controller (Single point detection)
  * @param  pObj   Pointer to component object.
  * @retval Component status.
  */
int32_t MFXSTM32L152_TS_Start(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret;
  uint8_t mode, tmp;

  /* Get the current register value */
  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &mode, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    /* Set the Functionalities to be Enabled */
    mode |= MFXSTM32L152_TS_EN;

    /* Set the new register value */
    ret = mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &mode, 1);

    /* Select 2 nF filter capacitor */
    /* Configuration:
    - Touch average control    : 4 samples
    - Touch delay time         : 500 uS
    - Panel driver setting time: 500 uS
    */
    tmp = 0x32;
    ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_TS_SETTLING, &tmp, 1);
    tmp = 0x05;
    ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_TS_TOUCH_DET_DELAY, &tmp, 1);
    tmp = 0x04;
    ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_TS_AVE, &tmp, 1);

    /* Configure the Touch FIFO threshold: single point reading */
    tmp = 0x01;
    ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_TS_FIFO_TH, &tmp, 1);

    /* Clear the FIFO memory content. */
    tmp = MFXSTM32L152_TS_CLEAR_FIFO;
    ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_TS_FIFO_TH, &tmp, 1);

    /* Touch screen control configuration :
    - No window tracking index */
    tmp = 0x00;
    ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_TS_TRACK, &tmp, 1);


    /*  Clear all the IT status pending bits if any */
    ret += MFXSTM32L152_IO_ClearIT(pObj, 0xFFFFFF);
  }

  if(ret != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Return if there is touch detected or not.
  * @param  pObj   Pointer to component object.
  * @retval Touch detected state.
  */
int32_t MFXSTM32L152_TS_DetectTouch(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t state, fifo_level;

  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_TS_FIFO_STA, &state, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    if(((state & MFXSTM32L152_TS_CTRL_STATUS) == MFXSTM32L152_TS_CTRL_STATUS) != 0U)
    {
      if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_TS_FIFO_LEVEL, &fifo_level, 1) != MFXSTM32L152_OK)
      {
        ret = MFXSTM32L152_ERROR;
      }
      else
      {
        if(fifo_level > 0U)
        {
          ret = 1;
        }
      }
    }
  }

  return ret;
}

/**
  * @brief  Get the touch screen X and Y positions values
  * @param  pObj   Pointer to component object.
  * @param  X: Pointer to X position value
  * @param  Y: Pointer to Y position value
  * @retval Component status.
  */
int32_t MFXSTM32L152_TS_GetXY(MFXSTM32L152_Object_t *pObj, uint16_t *X, uint16_t *Y)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t  data_xy[3];
  uint8_t  tmp;

  pObj->IO.ReadReg(pObj->IO.Address, MFXSTM32L152_TS_XY_DATA, data_xy, sizeof(data_xy));

  /* Calculate positions values */
  *X = ((uint16_t)data_xy[1]<<4U) + ((uint16_t)data_xy[0]>>4U);
  *Y = ((uint16_t)data_xy[2]<<4U) + ((uint16_t)data_xy[0]& 4U);

  /* Reset the FIFO memory content. */
  tmp = MFXSTM32L152_TS_CLEAR_FIFO;
  if(mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_TS_FIFO_TH, &tmp, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Configure the selected source to generate a global interrupt or not
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_TS_EnableIT(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;

  /* Enable global TS IT source */
  if(MFXSTM32L152_EnableITSource(pObj, MFXSTM32L152_IRQ_TS_DET) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Configure the selected source to generate a global interrupt or not
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_TS_DisableIT(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;

  /* Disable global TS IT source */
  if(MFXSTM32L152_DisableITSource(pObj, MFXSTM32L152_IRQ_TS_DET) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Configure the selected source to generate a global interrupt or not
  * @param  pObj   Pointer to component object.
  * @retval TS interrupts status
  */
int32_t MFXSTM32L152_TS_ITStatus(MFXSTM32L152_Object_t *pObj)
{
  /* Return TS interrupts status */
  return(MFXSTM32L152_GlobalITStatus(pObj, MFXSTM32L152_IRQ_TS));
}

/**
  * @brief  Configure the selected source to generate a global interrupt or not
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_TS_ClearIT(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;

  /* Clear the global TS IT source */
  if(MFXSTM32L152_ClearGlobalIT(pObj, MFXSTM32L152_IRQ_TS) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/* ------------------------------------------------------------------ */
/* --------------------- IDD MEASUREMENT ---------------------------- */
/* ------------------------------------------------------------------ */

/**
  * @brief  Launch IDD current measurement
  * @param  DeviceAddr: Device address on communication Bus
  * @retval Component status.
  */
int32_t MFXSTM32L152_IDD_Start(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t mode = 0;

  /* Get the current register value */
  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_CTRL, &mode, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    /* Set the Functionalities to be enabled */
    mode |= MFXSTM32L152_IDD_CTRL_REQ;

    /* Start measurement campaign */
    if(mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_CTRL, &mode, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Configures the IDD current measurement
  * @param  pObj   Pointer to component object.
  * @param  MfxIddConfig: Parameters depending on hardware config.
  * @retval Component status
  */
int32_t MFXSTM32L152_IDD_Config(MFXSTM32L152_Object_t *pObj, MFXSTM32L152_IDD_Config_t * MfxIddConfig)
{
  int32_t ret;
  uint8_t value;
  uint8_t mode = 0;

  /* Get the current register value */
  ret = mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &mode, 1);

  if((mode & MFXSTM32L152_IDD_EN) != MFXSTM32L152_IDD_EN)
  {
    /* Set the Functionalities to be enabled */
    mode |= MFXSTM32L152_IDD_EN;

    /* Set the new register value */
    ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_SYS_CTRL, &mode, 1);
  }

  /* Control register setting: number of shunts */
  value =  ((MfxIddConfig->ShuntNbUsed << 1) & MFXSTM32L152_IDD_CTRL_SHUNT_NB);
  value |= (MfxIddConfig->VrefMeasurement & MFXSTM32L152_IDD_CTRL_VREF_DIS);
  value |= (MfxIddConfig->Calibration & MFXSTM32L152_IDD_CTRL_CAL_DIS);
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_CTRL, &value, 1);

  /* Idd pre delay configuration: unit and value*/
  value = (MfxIddConfig->PreDelayUnit & MFXSTM32L152_IDD_PREDELAY_UNIT) |
          (MfxIddConfig->PreDelayValue & MFXSTM32L152_IDD_PREDELAY_VALUE);
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_PRE_DELAY, &value, 1);

  /* Shunt 0 register value: MSB then LSB */
  value = (uint8_t) (MfxIddConfig->Shunt0Value >> 8);
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_SHUNT0_MSB, &value, 1);
  value = (uint8_t) (MfxIddConfig->Shunt0Value);
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_SHUNT0_LSB, &value, 1);

  /* Shunt 1 register value: MSB then LSB */
  value = (uint8_t) (MfxIddConfig->Shunt1Value >> 8);
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_SHUNT1_MSB, &value, 1);
  value = (uint8_t) (MfxIddConfig->Shunt1Value);
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_SHUNT1_LSB, &value, 1);

  /* Shunt 2 register value: MSB then LSB */
  value = (uint8_t) (MfxIddConfig->Shunt2Value >> 8);
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_SHUNT2_MSB, &value, 1);
  value = (uint8_t) (MfxIddConfig->Shunt2Value);
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_SHUNT2_LSB, &value, 1);

  /* Shunt 3 register value: MSB then LSB */
  value = (uint8_t) (MfxIddConfig->Shunt3Value >> 8);
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_SHUNT3_MSB, &value, 1);
  value = (uint8_t) (MfxIddConfig->Shunt3Value);
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_SHUNT3_LSB, &value, 1);

  /* Shunt 4 register value: MSB then LSB */
  value = (uint8_t) (MfxIddConfig->Shunt4Value >> 8);
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_SHUNT4_MSB, &value, 1);
  value = (uint8_t) (MfxIddConfig->Shunt4Value);
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_SHUNT4_LSB, &value, 1);

  /* Shunt 0 stabilization delay */
  value = (uint8_t)MfxIddConfig->Shunt0StabDelay;
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_SH0_STABILIZATION, &value, 1);

  /* Shunt 1 stabilization delay */
  value = (uint8_t)MfxIddConfig->Shunt1StabDelay;
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_SH1_STABILIZATION, &value, 1);

  /* Shunt 2 stabilization delay */
  value = (uint8_t)MfxIddConfig->Shunt2StabDelay;
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_SH2_STABILIZATION, &value, 1);

  /* Shunt 3 stabilization delay */
  value = (uint8_t)MfxIddConfig->Shunt3StabDelay;
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_SH3_STABILIZATION, &value, 1);

  /* Shunt 4 stabilization delay */
  value = (uint8_t)MfxIddConfig->Shunt4StabDelay;
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_SH4_STABILIZATION, &value, 1);

  /* Idd ampli gain value: MSB then LSB */
  value = (uint8_t) (MfxIddConfig->AmpliGain >> 8);
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_GAIN_MSB, &value, 1);
  value = (uint8_t) (MfxIddConfig->AmpliGain);
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_GAIN_LSB, &value, 1);

  /* Idd VDD min value: MSB then LSB */
  value = (uint8_t) (MfxIddConfig->VddMin >> 8);
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_VDD_MIN_MSB, &value, 1);
  value = (uint8_t) (MfxIddConfig->VddMin);
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_VDD_MIN_LSB, &value, 1);

  /* Idd number of measurements */
  value = MfxIddConfig->MeasureNb;
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_NBR_OF_MEAS, &value, 1);

  /* Idd delta delay configuration: unit and value */
  value = (MfxIddConfig->DeltaDelayUnit & MFXSTM32L152_IDD_DELTADELAY_UNIT) |
          (MfxIddConfig->DeltaDelayValue & MFXSTM32L152_IDD_DELTADELAY_VALUE);
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_MEAS_DELTA_DELAY, &value, 1);

  /* Idd number of shut on board */
  value = MfxIddConfig->ShuntNbOnBoard;
  ret += mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_SHUNTS_ON_BOARD, &value, 1);

  if(ret != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  This function allows to modify number of shunt used for a measurement
  * @param  DeviceAddr: Device address on communication Bus
  * @retval Component status.
  */
int32_t MFXSTM32L152_IDD_ConfigShuntNbLimit(MFXSTM32L152_Object_t *pObj, uint8_t ShuntNbLimit)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t mode = 0;

  /* Get the current register value */
  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_CTRL, &mode, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    /* Clear number of shunt limit */
    mode &= ~(MFXSTM32L152_IDD_CTRL_SHUNT_NB);

    /* Clear number of shunt limit */
    mode |= ((ShuntNbLimit << 1) & MFXSTM32L152_IDD_CTRL_SHUNT_NB);

    /* Write noewx desired limit */
    if(mfxstm32l152_write_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_CTRL, &mode, 1) != MFXSTM32L152_OK)
    {
      ret = MFXSTM32L152_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Get Idd current value
  * @param  DeviceAddr: Device address on communication Bus
  * @param  ReadValue: Pointer on value to be read
  * @retval Idd value in 10 nA.
  */
int32_t MFXSTM32L152_IDD_GetValue(MFXSTM32L152_Object_t *pObj, uint32_t *ReadValue)
{
  uint8_t  data[3];

  pObj->IO.ReadReg(pObj->IO.Address, MFXSTM32L152_REG_ADR_IDD_VALUE_MSB, data, sizeof(data));

  /* Recompose Idd current value */
  *ReadValue = ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | (uint32_t)data[2];

  return MFXSTM32L152_OK;
}

/**
  * @brief  Get Last shunt used for measurement
  * @param  DeviceAddr: Device address on communication Bus
  * @retval Last shunt used
  */
int32_t MFXSTM32L152_IDD_GetShuntUsed(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t tmp;

  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_IDD_SHUNT_USED, &tmp, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Configure mfx to enable Idd interrupt
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_IDD_EnableIT(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;

  /* Enable global IDD interrupt source */
  if(MFXSTM32L152_EnableITSource(pObj, MFXSTM32L152_IRQ_IDD) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Clear Idd global interrupt
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_IDD_ClearIT(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;

  /* Clear the global IDD interrupt source */
  if(MFXSTM32L152_ClearGlobalIT(pObj, MFXSTM32L152_IRQ_IDD) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  get Idd interrupt status
  * @param  pObj   Pointer to component object.
  * @retval IDD interrupts status
  */
int32_t MFXSTM32L152_IDD_GetITStatus(MFXSTM32L152_Object_t *pObj)
{
  /* Return IDD interrupt status */
  return(MFXSTM32L152_GlobalITStatus(pObj, MFXSTM32L152_IRQ_IDD));
}

/**
  * @brief  disable Idd interrupt
  * @param  pObj   Pointer to component object.
  * @retval Component status.
  */
int32_t MFXSTM32L152_IDD_DisableIT(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;

  /* Disable global IDD interrupt source */
  if(MFXSTM32L152_DisableITSource(pObj, MFXSTM32L152_IRQ_IDD) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}


/* ------------------------------------------------------------------ */
/* --------------------- ERROR MANAGEMENT --------------------------- */
/* ------------------------------------------------------------------ */

/**
  * @brief  Read Error Source.
  * @param  pObj   Pointer to component object.
  * @retval Error message code with error source
  */
int32_t MFXSTM32L152_Error_ReadSrc(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret;
  uint8_t tmp;

  /* Get the current source register value */
  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_ERROR_SRC, &tmp, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    ret = (int32_t)tmp;
  }

  return ret;
}

/**
  * @brief  Read Error Message
  * @param  pObj   Pointer to component object.
  * @retval Error message code with error source
  */
int32_t MFXSTM32L152_Error_ReadMsg(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret;
  uint8_t tmp;

  /* Get the current message register value */
  if(mfxstm32l152_read_reg(&pObj->Ctx, MFXSTM32L152_REG_ADR_ERROR_MSG, &tmp, 1) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    ret = (int32_t)tmp;
  }

  return ret;
}

/**
  * @brief  Enable Error global interrupt
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */

int32_t MFXSTM32L152_Error_EnableIT(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;

  /* Enable global Error interrupt source */
  if(MFXSTM32L152_EnableITSource(pObj, MFXSTM32L152_IRQ_ERROR) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Clear Error global interrupt
  * @param  pObj   Pointer to component object.
  * @retval Component status
  */
int32_t MFXSTM32L152_Error_ClearIT(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;

  /* Clear the global Error interrupt source */
  if(MFXSTM32L152_ClearGlobalIT(pObj, MFXSTM32L152_IRQ_ERROR) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  get Error interrupt status
  * @param  pObj   Pointer to component object.
  * @retval Error interrupts status
  */
int32_t MFXSTM32L152_Error_GetITStatus(MFXSTM32L152_Object_t *pObj)
{
  /* Return Error interrupt status */
  return(MFXSTM32L152_GlobalITStatus(pObj, MFXSTM32L152_IRQ_ERROR));
}

/**
  * @brief  disable Error interrupt
  * @param  pObj   Pointer to component object.
  * @retval Component status.
  */
int32_t MFXSTM32L152_Error_DisableIT(MFXSTM32L152_Object_t *pObj)
{
  int32_t ret = MFXSTM32L152_OK;

  /* Disable global Error interrupt source */
  if(MFXSTM32L152_DisableITSource(pObj, MFXSTM32L152_IRQ_ERROR) != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Register Bus Io to component
  * @param  Component object pointer
  * @retval Component status
  */
int32_t MFXSTM32L152_RegisterBusIO (MFXSTM32L152_Object_t *pObj, MFXSTM32L152_IO_t *pIO)
{
  int32_t ret;

  if (pObj == NULL)
  {
    ret = MFXSTM32L152_ERROR;
  }
  else
  {
    pObj->IO.Init      = pIO->Init;
    pObj->IO.DeInit    = pIO->DeInit;
    pObj->IO.Address   = pIO->Address;
    pObj->IO.WriteReg  = pIO->WriteReg;
    pObj->IO.ReadReg   = pIO->ReadReg;
    pObj->IO.GetTick   = pIO->GetTick;

    pObj->Ctx.ReadReg  = MFXSTM32L152_ReadRegWrap;
    pObj->Ctx.WriteReg = MFXSTM32L152_WriteRegWrap;
    pObj->Ctx.handle   = pObj;

    if(pObj->IO.Init != NULL)
    {
      ret = pObj->IO.Init();
    }
    else
    {
      ret = MFXSTM32L152_ERROR;
    }
  }
  return ret;
}
/**
  * @}
  */

/** @defgroup MFXSTM32L152_Private_Functions MFXSTM32L152 Private Functions
  * @{
  */
/**
  * @brief  Internal routine
  * @param  pObj   Pointer to component object.
  * @param  RegisterAddr: Register Address
  * @param  PinPosition: Pin [0:23]
  * @param  PinValue: 0/1
  * @retval Component status
  */
static int32_t MFXSTM32L152_reg24_setPinValue(MFXSTM32L152_Object_t *pObj, uint8_t RegisterAddr, uint32_t PinPosition, uint8_t PinValue)
{
  int32_t ret = MFXSTM32L152_OK;
  uint8_t tmp;
  uint8_t pin_0_7, pin_8_15, pin_16_23;

  pin_0_7   = (uint8_t)(PinPosition & 0x0000ffU);
  pin_8_15  = (uint8_t)(PinPosition >> 8);
  pin_8_15  = (uint8_t)(pin_8_15 & 0x00ffU);
  pin_16_23 = (uint8_t)(PinPosition >> 16);

  if (pin_0_7 != 0U)
  {
    /* Get the current register value */
    ret += mfxstm32l152_read_reg(&pObj->Ctx, RegisterAddr, &tmp, 1);

    /* Set the selected pin direction */
    if (PinValue != 0U)
    {
      tmp |= (uint8_t)pin_0_7;
    }
    else
    {
      tmp &= ~(uint8_t)pin_0_7;
    }

    /* Set the new register value */
    ret += mfxstm32l152_write_reg(&pObj->Ctx, RegisterAddr, &tmp, 1);
  }

  if (pin_8_15 != 0U)
  {
    /* Get the current register value */
    ret += mfxstm32l152_read_reg(&pObj->Ctx, ((uint16_t)RegisterAddr+1U), &tmp, 1);

    /* Set the selected pin direction */
    if (PinValue != 0U)
    {
      tmp |= (uint8_t)pin_8_15;
    }
    else
    {
      tmp &= ~(uint8_t)pin_8_15;
    }

    /* Set the new register value */
    ret += mfxstm32l152_write_reg(&pObj->Ctx, ((uint16_t)RegisterAddr+1U), &tmp, 1);
  }

  if (pin_16_23 != 0U)
  {
    /* Get the current register value */
    ret += mfxstm32l152_read_reg(&pObj->Ctx, ((uint16_t)RegisterAddr+2U), &tmp, 1);

    /* Set the selected pin direction */
    if (PinValue != 0U)
    {
      tmp |= (uint8_t)pin_16_23;
    }
    else
    {
      tmp &= ~(uint8_t)pin_16_23;
    }

    /* Set the new register value */
    ret += mfxstm32l152_write_reg(&pObj->Ctx, ((uint16_t)RegisterAddr+2U), &tmp, 1);
  }

  if(ret != MFXSTM32L152_OK)
  {
    ret = MFXSTM32L152_ERROR;
  }

  return ret;
}

/**
  * @brief  Wrap MFXSTM32L152 read function to Bus IO function
  * @param  handle Component object handle
  * @param  Reg    The target register address to write
  * @param  pData  The target register value to be written
  * @param  Length buffer size to be written
  * @retval error status
  */
static int32_t MFXSTM32L152_ReadRegWrap(void *handle, uint16_t Reg, uint8_t* pData, uint16_t Length)
{
  MFXSTM32L152_Object_t *pObj = (MFXSTM32L152_Object_t *)handle;

  return pObj->IO.ReadReg(pObj->IO.Address, Reg, pData, Length);
}

/**
  * @brief  Wrap MFXSTM32L152 write function to Bus IO function
  * @param  handle  Component object handle
  * @param  Reg     The target register address to write
  * @param  pData  The target register value to be written
  * @param  Length buffer size to be written
  * @retval error status
  */
static int32_t MFXSTM32L152_WriteRegWrap(void *handle, uint16_t Reg, uint8_t* pData, uint16_t Length)
{
  MFXSTM32L152_Object_t *pObj = (MFXSTM32L152_Object_t *)handle;

  return pObj->IO.WriteReg(pObj->IO.Address, Reg, pData, Length);
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
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
