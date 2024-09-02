/**
  ******************************************************************************
  * @file    ram_interface.c
  * @author  MCD Application Team
  * @brief   Contains RAM access functions
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "openbl_mem.h"
#include "app_openbootloader.h"
#include "common_interface.h"
#include "openbl_core.h"
#include "ram_interface.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
OPENBL_MemoryTypeDef RAM_Descriptor =
{
  RAM_START_ADDRESS + OPENBL_RAM_SIZE, /* OPENBL_RAM_SIZE is added to protect OpenBootloader RAM area */
  RAM_END_ADDRESS,
  RAM_SIZE,
  RAM_AREA,
  OPENBL_RAM_Read,
  OPENBL_RAM_Write,
  NULL,
  NULL,
  OPENBL_RAM_JumpToAddress,
  NULL,
  NULL
};

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  This function is used to read data from a given address.
  * @param  Address The address to be read.
  * @retval Returns the read value.
  */
uint8_t OPENBL_RAM_Read(uint32_t Address)
{
  return (*(uint8_t *)(Address));
}

/**
  * @brief  This function is used to write data in RAM memory.
  * @param  Address The address where that data will be written.
  * @param  pData The data to be written.
  * @param  DataLength The length of the data to be written.
  * @retval None.
  */
void OPENBL_RAM_Write(uint32_t Address, uint8_t *pData, uint32_t DataLength)
{
}

/**
  * @brief  This function is used to jump to a given address.
  * @param  Address The address where the function will jump.
  * @retval None.
  */
void OPENBL_RAM_JumpToAddress(uint32_t Address)
{
}
