/**
  ******************************************************************************
  * @file    usbd_dfu_media_template.c
  * @author  MCD Application Team
  * @brief   Memory management layer
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_dfu_media_template.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
uint16_t MEM_If_Init(void);
uint16_t MEM_If_Erase(uint32_t Add);
uint16_t MEM_If_Write(uint8_t *src, uint8_t *dest, uint32_t Len);
uint8_t *MEM_If_Read(uint8_t *src, uint8_t *dest, uint32_t Len);
uint16_t MEM_If_DeInit(void);
uint16_t MEM_If_GetStatus(uint32_t Add, uint8_t Cmd, uint8_t *buffer);
#if (USBD_DFU_VENDOR_CMD_ENABLED == 1U)
uint16_t MEM_If_GetVendorCMD(uint8_t *cmd, uint8_t *cmdlength);
uint16_t MEM_If_VendorDownloadCMD(uint8_t *pbuf, uint32_t BlockNumber, uint32_t wlength, uint32_t *status);
uint16_t MEM_If_VendorUploadCMD(uint32_t Add, uint32_t BlockNumber, uint32_t *status);
#endif /* USBD_DFU_VENDOR_CMD_ENABLED */
#if (USBD_DFU_VENDOR_CHECK_ENABLED == 1U)
uint16_t MEM_If_VendorCheck(uint8_t *pbuf, uint32_t ReqType, uint32_t *status);
#endif /* USBD_DFU_VENDOR_CHECK_ENABLED */
#if (USBD_DFU_VENDOR_EXIT_ENABLED == 1U)
uint16_t MEM_If_LeaveDFU(uint32_t Add);
#endif /* USBD_DFU_VENDOR_EXIT_ENABLED */

USBD_DFU_MediaTypeDef USBD_DFU_MEDIA_Template_fops =
{
  (uint8_t *)"DFU MEDIA",
  MEM_If_Init,
  MEM_If_DeInit,
  MEM_If_Erase,
  MEM_If_Write,
  MEM_If_Read,
  MEM_If_GetStatus,
#if (USBD_DFU_VENDOR_CMD_ENABLED == 1U)
  MEM_If_GetVendorCMD,
  MEM_If_VendorDownloadCMD,
  MEM_If_VendorUploadCMD,
#endif /* USBD_DFU_VENDOR_CMD_ENABLED */
#if (USBD_DFU_VENDOR_CHECK_ENABLED == 1U)
  MEM_If_VendorCheck,
#endif /* USBD_DFU_VENDOR_CHECK_ENABLED */
#if (USBD_DFU_VENDOR_EXIT_ENABLED == 1U)
  MEM_If_LeaveDFU
#endif /* USBD_DFU_VENDOR_EXIT_ENABLED */
};

/**
  * @brief  MEM_If_Init
  *         Memory initialization routine.
  * @param  None
  * @retval 0 if operation is successful, MAL_FAIL else.
  */
uint16_t MEM_If_Init(void)
{
  return 0;
}

/**
  * @brief  MEM_If_DeInit
  *         Memory deinitialization routine.
  * @param  None
  * @retval 0 if operation is successful, MAL_FAIL else.
  */
uint16_t MEM_If_DeInit(void)
{
  return 0;
}

/**
  * @brief  MEM_If_Erase
  *         Erase sector.
  * @param  Add: Address of sector to be erased.
  * @retval 0 if operation is successful, MAL_FAIL else.
  */
uint16_t MEM_If_Erase(uint32_t Add)
{
  UNUSED(Add);

  return 0;
}

/**
  * @brief  MEM_If_Write
  *         Memory write routine.
  * @param  Add: Address to be written to.
  * @param  Len: Number of data to be written (in bytes).
  * @retval 0 if operation is successful, MAL_FAIL else.
  */
uint16_t MEM_If_Write(uint8_t *src, uint8_t *dest, uint32_t Len)
{
  UNUSED(src);
  UNUSED(dest);
  UNUSED(Len);

  return 0;
}

/**
  * @brief  MEM_If_Read
  *         Memory read routine.
  * @param  Add: Address to be read from.
  * @param  Len: Number of data to be read (in bytes).
  * @retval Pointer to the physical address where data should be read.
  */
uint8_t *MEM_If_Read(uint8_t *src, uint8_t *dest, uint32_t Len)
{
  UNUSED(src);
  UNUSED(dest);
  UNUSED(Len);

  /* Return a valid address to avoid HardFault */
  return NULL;
}

/**
  * @brief  Flash_If_GetStatus
  *         Memory read routine.
  * @param  Add: Address to be read from.
  * @param  cmd: Number of data to be read (in bytes).
  * @retval Pointer to the physical address where data should be read.
  */
uint16_t MEM_If_GetStatus(uint32_t Add, uint8_t Cmd, uint8_t *buffer)
{
  UNUSED(Add);
  UNUSED(buffer);

  switch (Cmd)
  {
    case DFU_MEDIA_PROGRAM:

      break;

    case DFU_MEDIA_ERASE:
    default:

      break;
  }
  return (0);
}

#if (USBD_DFU_VENDOR_CMD_ENABLED == 1U)
/**
  * @brief  Get supported vendor specific commands
  * @param  pointer to supported vendor commands
  * @param  pointer to length of supported vendor commands
  * @retval 0 if operation is successful
  */
uint16_t MEM_If_GetVendorCMD(uint8_t *cmd, uint8_t *cmdlength)
{
  UNUSED(cmd);
  UNUSED(cmdlength);

  return 0U;
}

/**
  * @brief  Vendor specific download commands
  * @param  pbuf DFU data buffer
  * @param  BlockNumber DFU memory block number
  * @param  wLength DFU request length
  * @param  pointer to DFU status
  * @retval 0 if operation is successful
  */
uint16_t MEM_If_VendorDownloadCMD(uint8_t *pbuf, uint32_t BlockNumber, uint32_t wlength, uint32_t *status)
{
  UNUSED(pbuf);
  UNUSED(BlockNumber);
  UNUSED(wlength);
  UNUSED(status);

  return 0U;
}


/**
  * @brief  Vendor specific upload commands
  * @param  Add memory Address
  * @param  BlockNumber DFU memory block number
  * @param  pointer to DFU status
  * @retval 0 if operation is successful
  */
uint16_t MEM_If_VendorUploadCMD(uint32_t Add, uint32_t BlockNumber, uint32_t *status)
{
  UNUSED(Add);
  UNUSED(BlockNumber);
  UNUSED(status);

  return 0U;
}
#endif /* USBD_DFU_VENDOR_CMD_ENABLED */

#if (USBD_DFU_VENDOR_CHECK_ENABLED == 1U)
/**
  * @brief  Vendor memory check
  * @param  pbuf DFU data buffer
  * @param  ReqType IS_DFU_SETADDRESSPOINTER/DOWNLOAD/UPLOAD
  * @param  pointer to DFU status
  * @retval 0 if operation is successful
  */
uint16_t MEM_If_VendorCheck(uint8_t *pbuf, uint32_t ReqType, uint32_t *status)
{
  UNUSED(pbuf);
  UNUSED(ReqType);
  UNUSED(status);

  return 0U;
}
#endif /* USBD_DFU_VENDOR_CHECK_ENABLED */

#if (USBD_DFU_VENDOR_EXIT_ENABLED == 1U)
/**
  * @brief  Vendor Leave DFU
  * @param  Application address
  * @retval 0 if operation is successful
  */
uint16_t MEM_If_LeaveDFU(uint32_t Add)
{
  UNUSED(Add);

  return 0U;
}
#endif /* USBD_DFU_VENDOR_EXIT_ENABLED */
