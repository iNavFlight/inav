/**
  ******************************************************************************
  * @file    stm32h7b3i_eval_eeprom.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage an I2C M24LR64
  *          EEPROM memory.
  @verbatim
            To be able to use this driver, the switch EE_M24LR64 must be defined
            in your toolchain compiler preprocessor

            ===================================================================
            Notes:
             - The I2C EEPROM memory (M24LR64) is available on separate daughter
               board ANT7-M24LR-A, which is not provided with the STM32H7B3I_EVAL
               board.
               To use this driver you have to connect the ANT7-M24LR-A to CN3
               connector of STM32H7B3I_EVAL board.
            ===================================================================

            It implements a high level communication layer for read and write
            from/to this memory. The needed STM32Fxxx hardware resources (I2C and
            GPIO) are defined in stm32h7b3i_eval_bus.h file, and the initialization is
            performed in BSP_EEPROM_Init().
            You can easily tailor this driver to any other development board,
            by just adapting the defines for hardware resources and
            BSP_EEPROM_Init() function.

            + Call the function BSP_EEPROM_WritePage() to write a single page of 4 bytes
            + Call the function BSP_EEPROM_ReadPage() to read a single page of 4 bytes
            + Call the function BSP_EEPROM_WriteBuffer() to write a buffer of N bytes to EEPROM
            + Call the function BSP_EEPROM_ReadBuffer() to read a buffer of N bytes from EEPROM
            + Call the function BSP_EEPROM_IsDeviceReady() to verify if device is ready. This function
              returns a BUSY error if the device is not ready after several trials.

           @note  Regarding the "Instance" parameter, needed for all functions, it is used to select
                  an EEPROM instance. On the STM32H7B3I_EVAL board, there's one instance. Then, this
                  parameter should be 0.



       +-----------------------------------------------------------------+
       |     Pin assignment for M24LR64 EEPROM          |
       +---------------------------------------+-----------+-------------+
       |  STM32Fxxx I2C Pins                   |   EEPROM  |   Pin       |
       +---------------------------------------+-----------+-------------+
       | .                                     |   E0(GND) |    1  (0V)  |
       | .                                     |   AC0     |    2        |
       | .                                     |   AC1     |    3        |
       | .                                     |   VSS     |    4  (0V)  |
       | SDA                                   |   SDA     |    5        |
       | SCL                                   |   SCL     |    6        |
       | .                                     |   E1(GND) |    7  (0V)  |
       | .                                     |   VDD     |    8 (3.3V) |
       +---------------------------------------+-----------+-------------+
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
#include "stm32h7b3i_eval_eeprom.h"
#include "stm32h7b3i_eval_bus.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H7B3I_EVAL
  * @{
  */

/** @defgroup STM32H7B3I_EVAL_EEPROM EEPROM
  * @brief This file includes the I2C EEPROM driver of STM32H7B3I_EVAL board.
  * @{
  */

/** @defgroup STM32H7B3I_EVAL_EEPROM_Exported_Variables EEPROM Exported Variables
  * @{
  */
void   *Eeprom_CompObj = NULL;
/**
  * @}
  */

/** @defgroup STM32H7B3I_EVAL_EEPROM_Private_Variables EEPROM Private Variables
  * @{
  */
static M24LR64_EEPROM_Drv_t     *Eeprom_Drv = NULL;
/**
  * @}
  */

/** @defgroup STM32H7B3I_EVAL_EEPROM_Private_Function_Prototypes EEPROM Private Functions Prototypes
  * @{
  */
static int32_t M24LR64_Probe(void);
static int32_t EEPROM_WriteBytes(uint32_t Instance, uint8_t *pBuffer, uint32_t WriteAddr, uint32_t NbrOfBytes);
/**
  * @}
  */

/** @defgroup STM32H7B3I_EVAL_EEPROM_Exported_Functions EEPROM Exported Functions
  * @{
  */
/**
  * @brief  Initialize the I2C EEPROM.
  * @param  Instance EEPROM instance. Could be only 0.
  * @retval EEPROM state
  */
int32_t BSP_EEPROM_Init(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if (Instance >= EEPROM_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (M24LR64_Probe() != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_NO_INIT;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  DeInitialize the EEPROM.
  * @param  Instance EEPROM instance. Could be only 0.
  * @retval EEPROM state
  */
int32_t BSP_EEPROM_DeInit(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if (Instance >= EEPROM_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (Eeprom_Drv->DeInit(Eeprom_CompObj) < 0)
    {
      ret = BSP_ERROR_BUS_FAILURE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Write buffer of data to an I2C EEPROM page.
  * @param  Instance EEPROM instance. Could be only 0.
  * @param  pBuffer  Pointer to the buffer  containing the data to be written
  *         to the EEPROM.
  * @param  PageNbr EEPROM's internal page number to write to.
  * @retval BSP status
  */
int32_t BSP_EEPROM_WritePage(uint32_t Instance, uint8_t *pBuffer, uint32_t PageNbr)
{
  int32_t ret = BSP_ERROR_NONE;
  uint16_t WriteAddr = (uint16_t)PageNbr * EEPROM_PAGESIZE;

  if ((Instance >= EEPROM_INSTANCES_NBR) && (WriteAddr > EEPROM_MAX_SIZE))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if (Eeprom_Drv->Write(Eeprom_CompObj, WriteAddr, pBuffer, EEPROM_PAGESIZE) != BSP_ERROR_NONE)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    if (BSP_EEPROM_IsDeviceReady(Instance) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Read buffer of data from an I2C EEPROM page.
  * @param  Instance EEPROM instance. Could be only 0.
  * @param  pBuffer  Pointer to the buffer  containing the data to be written
  *         to the EEPROM.
  * @param  PageNbr EEPROM's internal page number to read from.
  * @retval BSP status
  */
int32_t BSP_EEPROM_ReadPage(uint32_t Instance, uint8_t *pBuffer, uint32_t PageNbr)
{
  int32_t ret = BSP_ERROR_NONE;
  uint16_t ReadAddr = (uint16_t)PageNbr * EEPROM_PAGESIZE;

  if ((Instance >= EEPROM_INSTANCES_NBR) && (ReadAddr > EEPROM_MAX_SIZE))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (Eeprom_Drv->Read(Eeprom_CompObj, ReadAddr, pBuffer, EEPROM_PAGESIZE) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Read a buffer of data from the EEPROM.
  * @param  Instance EEPROM instance. Could be only 0.
  * @param  pBuffer  Pointer to the buffer that receives the data read from
  *         the EEPROM.
  * @param  ReadAddr EEPROM's internal address to start reading from.
  * @param  NbrOfBytes  Number of bytes to be read from the EEPROM.
  * @retval BSP status
  */
int32_t BSP_EEPROM_ReadBuffer(uint32_t Instance, uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NbrOfBytes)
{
  int32_t ret = BSP_ERROR_NONE;

  if (Instance >= EEPROM_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (Eeprom_Drv->Read(Eeprom_CompObj, ReadAddr, pBuffer, NbrOfBytes) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Writes buffer of data to the I2C EEPROM.
  * @param  Instance EEPROM instance. Could be only 0.
  * @param  pBuffer  Pointer to the buffer  containing the data to be written
  *         to the EEPROM.
  * @param  WriteAddr EEPROM's internal address to write to.
  * @param  NbrOfBytes  number of bytes to write to the EEPROM.
  * @retval BSP status
  */
int32_t BSP_EEPROM_WriteBuffer(uint32_t Instance, uint8_t *pBuffer, uint32_t WriteAddr, uint32_t NbrOfBytes)
{
  int32_t ret = BSP_ERROR_NONE;
  uint32_t numofpages, numofsingle, count;
  uint8_t *write_buffer;
  uint32_t write_addr, nbr_of_bytes;
  uint32_t  i;

  if (Instance >= EEPROM_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    write_buffer = pBuffer;
    write_addr   = WriteAddr;
    nbr_of_bytes = NbrOfBytes;
    count = EEPROM_PAGESIZE - (write_addr % EEPROM_PAGESIZE);
    numofpages =  nbr_of_bytes / EEPROM_PAGESIZE;
    numofsingle = nbr_of_bytes % EEPROM_PAGESIZE;

    /* If WriteAddr is EEPROM_PAGESIZE aligned */
    if ((write_addr % EEPROM_PAGESIZE) == 0U)
    {
      /* If nbr_of_bytes < EEPROM_PAGESIZE */
      if (numofpages == 0U)
      {
        /* Start writing data */
        if (EEPROM_WriteBytes(Instance, write_buffer, write_addr, numofsingle) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
      }
      /* If nbr_of_bytes > EEPROM_PAGESIZE */
      else
      {
        i = 0;
        do
        {
          if (EEPROM_WriteBytes(Instance, write_buffer, write_addr, EEPROM_PAGESIZE) != BSP_ERROR_NONE)
          {
            ret = BSP_ERROR_COMPONENT_FAILURE;
          }
          else
          {
            write_addr += EEPROM_PAGESIZE;
            write_buffer += EEPROM_PAGESIZE;
            i++;
          }
        } while ((i < numofpages) && (ret == BSP_ERROR_NONE));

        if (numofsingle != 0U)
        {
          if (EEPROM_WriteBytes(Instance, write_buffer, write_addr, numofsingle) != BSP_ERROR_NONE)
          {
            ret = BSP_ERROR_COMPONENT_FAILURE;
          }
        }
      }
    }
    /* If WriteAddr is not EEPROM_PAGESIZE aligned */
    else
    {
      /* If nbr_of_bytes < EEPROM_PAGESIZE */
      if (numofpages == 0U)
      {
        /* If the number of data to be written is more than the remaining space
        in the current page: */
        if (nbr_of_bytes > count)
        {
          /* Write the data contained in same page */
          if (EEPROM_WriteBytes(Instance, write_buffer, write_addr, count) != BSP_ERROR_NONE)
          {
            ret = BSP_ERROR_COMPONENT_FAILURE;
          }/* Write the remaining data in the following page */
          else if (EEPROM_WriteBytes(Instance, (uint8_t *)(write_buffer + count), (write_addr + count),
                                     nbr_of_bytes - count) != BSP_ERROR_NONE)
          {
            ret = BSP_ERROR_COMPONENT_FAILURE;
          }
          else
          {
            ret = BSP_ERROR_NONE;
          }
        }
        else if (EEPROM_WriteBytes(Instance, write_buffer, write_addr, numofsingle) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else
        {
          ret = BSP_ERROR_NONE;
        }
      }
      /* If nbr_of_bytes > EEPROM_PAGESIZE */
      else
      {
        i = 0;
        nbr_of_bytes -= count;
        numofpages =  nbr_of_bytes / EEPROM_PAGESIZE;
        numofsingle = nbr_of_bytes % EEPROM_PAGESIZE;

        if (EEPROM_WriteBytes(Instance, write_buffer, write_addr, count) != BSP_ERROR_NONE)
        {
          ret = BSP_ERROR_COMPONENT_FAILURE;
        }
        else
        {
          write_addr += count;
          write_buffer += count;
        }

        do
        {
          if (EEPROM_WriteBytes(Instance, write_buffer, write_addr, EEPROM_PAGESIZE) != BSP_ERROR_NONE)
          {
            ret = BSP_ERROR_COMPONENT_FAILURE;
          }
          else
          {
            write_addr += EEPROM_PAGESIZE;
            write_buffer += EEPROM_PAGESIZE;
            i++;
          }
        } while ((i < numofpages) && (ret == BSP_ERROR_NONE));

        if (numofsingle != 0U)
        {
          if (EEPROM_WriteBytes(Instance, write_buffer, write_addr, numofsingle) != BSP_ERROR_NONE)
          {
            ret = BSP_ERROR_COMPONENT_FAILURE;
          }
        }
      }
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Wait for EEPROM Standby state.
  * @param  Instance EEPROM instance. Could be only 0.
  * @note   This function allows to wait and check that EEPROM has finished the
  *         last operation. It is mostly used after Write operation: after receiving
  *         the buffer to be written, the EEPROM may need additional time to actually
  *         perform the write operation. During this time, it doesn't answer to
  *         I2C packets addressed to it. Once the write operation is complete
  *         the EEPROM responds to its address.
  * @retval BSP status
  */
int32_t BSP_EEPROM_IsDeviceReady(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if (Instance >= EEPROM_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if (Eeprom_Drv->IsReady(Eeprom_CompObj, EEPROM_MAX_TRIALS) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_BUSY;
    }
  }

  /* Return BSP status */
  return ret;
}
/**
  * @}
  */

/** @defgroup STM32H7B3I_EVAL_EEPROM_Private_Functions EEPROM Private Functions
  * @{
  */
/**
  * @brief  Writes buffer of bytes to the I2C EEPROM.
  * @param  Instance EEPROM instance. Could be only 0.
  * @param  pBuffer  Pointer to the buffer  containing the data to be written
  *         to the EEPROM.
  * @param  WriteAddr EEPROM's internal address to write to.
  * @param  NbrOfBytes  number of bytes to write to the EEPROM.
  * @retval BSP status
  */
static int32_t EEPROM_WriteBytes(uint32_t Instance, uint8_t *pBuffer, uint32_t WriteAddr, uint32_t NbrOfBytes)
{
  int32_t ret = BSP_ERROR_NONE;

  if (Eeprom_Drv->Write(Eeprom_CompObj, WriteAddr, pBuffer, NbrOfBytes) < 0)
  {
    ret = BSP_ERROR_COMPONENT_FAILURE;
  }
  else
  {
    if (BSP_EEPROM_IsDeviceReady(Instance) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_BUSY;
    }
  }

  /* Return BSP status */
  return ret;
}

/**
  * @brief  Register Bus IOs
  * @note   There are 2 different versions of M24LR64 (A01 & A02).
  *             Then try to connect on 1st one (EEPROM_I2C_ADDRESS_A01)
  *             and if problem, check the 2nd one (EEPROM_I2C_ADDRESS_A02)
  * @retval BSP status
  */
static int32_t M24LR64_Probe(void)
{
  int32_t ret = BSP_ERROR_UNKNOWN_COMPONENT;
  M24LR64_IO_t              IOCtx;
  static M24LR64_Object_t   M24LR64Obj;
  uint32_t i;
  uint8_t i2c_address[] = {EEPROM_I2C_ADDRESS_A01, EEPROM_I2C_ADDRESS_A02};

  /* Configure the audio driver */
  IOCtx.Init        = BSP_I2C2_Init;
  IOCtx.DeInit      = BSP_I2C2_DeInit;
  IOCtx.Read        = BSP_I2C2_ReadReg16;
  IOCtx.Write       = BSP_I2C2_WriteReg16;
  IOCtx.IsReady     = BSP_I2C2_IsReady;

  for (i = 0U; i < 2U; i++)
  {
    IOCtx.Address     = (uint16_t)i2c_address[i];

    if (M24LR64_RegisterBusIO(&M24LR64Obj, &IOCtx) != M24LR64_OK)
    {
      ret = BSP_ERROR_BUS_FAILURE;
    }
    else
    {
      Eeprom_Drv = &M24LR64_EEPROM_Driver;
      Eeprom_CompObj = &M24LR64Obj;
      if (Eeprom_Drv->Init(Eeprom_CompObj) != M24LR64_OK)
      {
        ret = BSP_ERROR_COMPONENT_FAILURE;
      }
      else if (Eeprom_Drv->IsReady(Eeprom_CompObj, EEPROM_MAX_TRIALS) == BSP_ERROR_NONE)
      {
        ret = BSP_ERROR_NONE;
        break;
      }
      else
      {
        ret = BSP_ERROR_BUSY;
      }
    }
  }

  return ret;
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
