/**
  ******************************************************************************
  * @file    openbl_usb_cmd.c
  * @author  MCD Application Team
  * @brief   Contains USB protocol commands
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
#include "openbl_usb_cmd.h"
#include "openbl_mem.h"
#include "openbootloader_conf.h"
#include "usb_interface.h"
#include "common_interface.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define USB_RAM_BUFFER_SIZE             20U  /* Size of USB buffer used to store received data from the host */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Exported functions---------------------------------------------------------*/
/**
  * @brief  Erase sector.
  * @param  Address: Address of sector to be erased.
  * @retval 0 if operation is successful, MAL_FAIL else.
  */
uint16_t OPENBL_USB_EraseMemory(uint32_t Address)
{
  ErrorStatus error_value;
  uint8_t status;
  uint32_t numpage;
  uint32_t page;
  uint8_t usb_ram_buffer[USB_RAM_BUFFER_SIZE];
  uint8_t *ramaddress;

  ramaddress = (uint8_t *) usb_ram_buffer;
  numpage = 1;

  *ramaddress = (uint8_t)(numpage & 0x00FFU);
  ramaddress++;

  *ramaddress = (uint8_t)((numpage & 0xFF00U) >> 8);
  ramaddress++;

  page = OPENBL_USB_GetPage(Address);

  *ramaddress = (uint8_t)(page & 0x00FFU);
  ramaddress++;

  *ramaddress = (uint8_t)((page & 0xFF00U) >> 8);
  ramaddress++;

  error_value = OPENBL_MEM_Erase(OPENBL_DEFAULT_MEM, (uint8_t *) usb_ram_buffer, USB_RAM_BUFFER_SIZE);

  if (error_value != SUCCESS)
  {
    status = 1U;
  }
  else
  {
    status = 0U;
  }

  return status;
}

/**
  * @brief  Memory write routine.
  * @param  pSrc: Pointer to the source buffer. Address to be written to.
  * @param  pDest: Pointer to the destination buffer.
  * @param  Length: Number of data to be written (in bytes).
  * @retval USBD_OK if operation is successful, MAL_FAIL else.
  */
void OPENBL_USB_WriteMemory(uint8_t *pSrc, uint8_t *pDest, uint32_t Length)
{
  uint32_t address;

  address = (uint32_t)pDest[0] | ((uint32_t)pDest[1] << 8) |
            ((uint32_t)pDest[2] << 16) | ((uint32_t)pDest[3] << 24);

  OPENBL_MEM_Write(address, pSrc, Length);

  /* Start post processing task if needed */
  Common_StartPostProcessing();
}

/**
  * @brief  Memory read routine.
  * @param  pSrc: Pointer to the source buffer. Address to be written to.
  * @param  pDest: Pointer to the destination buffer.
  * @param  Length: Number of data to be read (in bytes).
  * @retval Pointer to the physical address where data should be read.
  */
uint8_t *OPENBL_USB_ReadMemory(uint8_t *pSrc, uint8_t *pDest, uint32_t Length)
{
  uint32_t memory_index;
  uint32_t address;
  uint32_t i;

  address = (uint32_t)pSrc[0] | ((uint32_t)pSrc[1] << 8) |
            ((uint32_t)pSrc[2] << 16) | ((uint32_t)pSrc[3] << 24);

  memory_index = OPENBL_MEM_GetMemoryIndex(address);

  for (i = 0; i < Length; i++)
  {
    pDest[i] = OPENBL_MEM_Read(address, memory_index);
    address++;
  }

  /* Return a valid address to avoid HardFault */
  return pDest;
}

/**
  * @brief  This function is used to jump to the user application.
  * @param  Address: The jump address.
  * @retval None
  */
void OPENBL_USB_Jump(uint32_t Address)
{
  uint8_t status;

  /* Check if received address is valid or not */
  status = OPENBL_MEM_CheckJumpAddress(Address);

  if (status == 1U)
  {
    OPENBL_MEM_JumpToAddress(Address);
  }
}

/**
  * @brief  Write protect.
  * @param  pBuffer: A buffer that contains the list of sectors or pages to be protected.
  * @param  Length: Contains the length of the pBuffer.
  * @retval None.
  */
void OPENBL_USB_WriteProtect(uint8_t *pBuffer, uint32_t Length)
{
  ErrorStatus error_value;

  error_value = OPENBL_MEM_SetWriteProtection(ENABLE, OPENBL_DEFAULT_MEM, pBuffer, Length);

  if (error_value == SUCCESS)
  {
    /* Start post processing task if needed */
    Common_StartPostProcessing();
  }
}

/**
  * @brief  Write unprotect.
  * @retval None.
  */
void OPENBL_USB_WriteUnprotect(void)
{
  ErrorStatus error_value;

  error_value = OPENBL_MEM_SetWriteProtection(DISABLE, OPENBL_DEFAULT_MEM, NULL, 0U);

  if (error_value == SUCCESS)
  {
    /* Start post processing task if needed */
    Common_StartPostProcessing();
  }
}

/**
  * @brief  Read protect.
  * @retval None.
  */
void OPENBL_USB_ReadProtect(void)
{
  /* Enable the read protection */
  OPENBL_MEM_SetReadOutProtection(OPENBL_DEFAULT_MEM, ENABLE);

  /* Start post processing task if needed */
  Common_StartPostProcessing();
}

/**
  * @brief  Read unprotect.
  * @retval None.
  */
void OPENBL_USB_ReadUnprotect(void)
{
  /* Disable the read protection */
  OPENBL_MEM_SetReadOutProtection(OPENBL_DEFAULT_MEM, DISABLE);

  /* Start post processing task if needed */
  Common_StartPostProcessing();
}
