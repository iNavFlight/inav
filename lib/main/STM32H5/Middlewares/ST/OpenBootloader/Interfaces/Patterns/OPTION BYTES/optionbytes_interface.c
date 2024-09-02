/**
  ******************************************************************************
  * @file    optionbytes_interface.c
  * @author  MCD Application Team
  * @brief   Contains Option Bytes access functions
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
#include "optionbytes_interface.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
OPENBL_MemoryTypeDef OB_Descriptor =
{
  OB_START_ADDRESS,
  OB_END_ADDRESS,
  OB_SIZE,
  OB_AREA,
  OPENBL_OB_Read,
  OPENBL_OB_Write,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Launch the option byte loading.
  * @retval None.
  */
void OPENBL_OB_Launch(void)
{
  /* Set the option start bit */
  HAL_FLASH_OB_Launch();

  /* Set the option lock bit and Lock the flash */
  HAL_FLASH_OB_Lock();
  HAL_FLASH_Lock();
}

/**
  * @brief  This function is used to read data from a given address.
  * @param  Address The address to be read.
  * @retval Returns the read value.
  */
uint8_t OPENBL_OB_Read(uint32_t Address)
{
  return (*(uint8_t *)(Address));
}

/**
  * @brief  Write Flash OB keys to unlock the option bytes settings
  * @param  None
  * @retval None
  */
void BL_FLASH_WriteOptKeys(void)
{
  if (READ_BIT(FLASH->CR, FLASH_CR_LOCK) != RESET)
  {
    /* Authorize the FLASH Registers access */
    WRITE_REG(FLASH->KEYR, FLASH_KEY1);
    WRITE_REG(FLASH->KEYR, FLASH_KEY2);
  }

  if (READ_BIT(FLASH->CR, FLASH_CR_OPTLOCK) != RESET)
  {
    /* Authorizes the Option Byte register programming */
    WRITE_REG(FLASH->OPTKEYR, FLASH_OPTKEY1);
    WRITE_REG(FLASH->OPTKEYR, FLASH_OPTKEY2);
  }
}
/**
  * @brief  This function is used to write data in Option bytes.
  * @param  Address The address where that data will be written.
  * @param  Data The data to be written.
  * @param  DataLength The length of the data to be written.
  * @retval None.
  */
void OPENBL_OB_Write(uint32_t Address, uint8_t *Data, uint32_t DataLength)
{
  /* Unlock the FLASH & Option Bytes Registers access */
  HAL_FLASH_Unlock();
  HAL_FLASH_OB_Unlock();

  /* Clear error programming flags */
  __HAL_FLASH_CLEAR_FLAG(FLASH_SR_ERRORS);

  /* Write RDP Level */
  WRITE_REG(FLASH->OPTR, *(Data));

  /* Write OPTR */
  if (DataLength >= 4)
  {
    WRITE_REG(FLASH->OPTR, (*(Data) | (*(Data + 1) << 8) | (*(Data + 2) << 16) | (*(Data + 3) << 24)));
  }

  /* Write PCROP1ASR */
  if (DataLength >= 10)
  {
    WRITE_REG(FLASH->PCROP1ASR, (*(Data + 8) | (*(Data + 9) << 8)));
  }

  /* Write PCROP1AER */
  if (DataLength >= 20)
  {
    WRITE_REG(FLASH->PCROP1AER, (*(Data + 16) | (*(Data + 17) << 8) | (*(Data + 19) << 24)));
  }

  /* Write WRP1AR */
  if (DataLength >= 28)
  {
    WRITE_REG(FLASH->WRP1AR, (*(Data + 24) | (*(Data + 25) << 8) | (*(Data + 26) << 16) | (*(Data + 27) << 24)));
  }

  /* Write WRP1BR */
  if (DataLength >= 36)
  {
    WRITE_REG(FLASH->WRP1BR, (*(Data + 32) | (*(Data + 33) << 8) | (*(Data + 34) << 16) | (*(Data + 35) << 24)));
  }

  /* Write PCROP1BSR */
  if (DataLength >= 42)
  {
    WRITE_REG(FLASH->PCROP1BSR, (*(Data + 40) | (*(Data + 41) << 8)));
  }

  /* Write PCROP1BER */
  if (DataLength >= 50)
  {
    WRITE_REG(FLASH->PCROP1BER, (*(Data + 48) | (*(Data + 49) << 8)));
  }

  /* Write PCROP2ASR */
  if (DataLength >= 58)
  {
    WRITE_REG(FLASH->PCROP2ASR, (*(Data + 56) | (*(Data + 57) << 8)));
  }

  /* Write PCROP2AER */
  if (DataLength >= 66)
  {
    WRITE_REG(FLASH->PCROP2AER, (*(Data + 64) | (*(Data + 65) << 8)));
  }

  /* Write WRP2AR */
  if (DataLength >= 76)
  {
    WRITE_REG(FLASH->WRP2AR, (*(Data + 72) | (*(Data + 73) << 8) | (*(Data + 74) << 16) | (*(Data + 75) << 24)));
  }

  /* Write WRP2BR */
  if (DataLength >= 84)
  {
    WRITE_REG(FLASH->WRP2BR, (*(Data + 80) | (*(Data + 81) << 8) | (*(Data + 82) << 16) | (*(Data + 83) << 24)));
  }

  /* Write PCROP2BSR */
  if (DataLength >= 90)
  {
    WRITE_REG(FLASH->PCROP2BSR, (*(Data + 88) | (*(Data + 89) << 8)));
  }

  /* Write PCROP2BER */
  if (DataLength >= 98)
  {
    WRITE_REG(FLASH->PCROP2BER, (*(Data + 96) | (*(Data + 97) << 8)));
  }

  /* Write SECR */
  if (DataLength >= 116)
  {
    WRITE_REG(FLASH->SECR, (*(Data + 112) | (*(Data + 113) << 8) | (*(Data + 114) << 16) | (*(Data + 115) << 24)));
  }

  SET_BIT(FLASH->CR, FLASH_CR_OPTSTRT);

  /* Register system reset callback */
  Common_SetPostProcessingCallback(OPENBL_OB_Launch);
}
