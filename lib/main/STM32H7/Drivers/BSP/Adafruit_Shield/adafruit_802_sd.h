/**
  ******************************************************************************
  * @file    adafruit_802_sd.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the adafruit_802_sd.c driver.
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
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef ADAFRUIT_802_SD_H
#define ADAFRUIT_802_SD_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "adafruit_802_conf.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup ADAFRUIT_802
  * @{
  */

/** @defgroup ADAFRUIT_802_SD SD
  * @{
  */

/** @defgroup ADAFRUIT_802_SD_Exported_Types SD Exported Types
  * @{
  */
typedef struct
{
  uint32_t Reserved1:2U;               /* Reserved */
  uint32_t DeviceSize:12U;             /* Device Size */
  uint32_t MaxRdCurrentVDDMin:3U;      /* Max. read current @ VDD min */
  uint32_t MaxRdCurrentVDDMax:3U;      /* Max. read current @ VDD max */
  uint32_t MaxWrCurrentVDDMin:3U;      /* Max. write current @ VDD min */
  uint32_t MaxWrCurrentVDDMax:3U;      /* Max. write current @ VDD max */
  uint32_t DeviceSizeMul:3U;           /* Device size multiplier */
} SD_Version_1_t;


typedef struct
{
  uint32_t Reserved1:6U;               /* Reserved */
  uint32_t DeviceSize:22U;             /* Device Size */
  uint32_t Reserved2:1U;               /* Reserved */
} SD_Version_2_t;

/**
  * @brief  Card Specific Data: CSD Register
  */
typedef struct
{
  /* Header part */
  uint32_t  CSDStruct:2U;            /* CSD structure */
  uint32_t  Reserved1:6U;            /* Reserved */
  uint32_t  TAAC:8U;                 /* Data read access-time 1 */
  uint32_t  NSAC:8U;                 /* Data read access-time 2 in CLK cycles */
  uint32_t  MaxBusClkFrec:8U;        /* Max. bus clock frequency */
  uint32_t  CardComdClasses:12U;     /* Card command classes */
  uint32_t  RdBlockLen:4U;           /* Max. read data block length */
  uint32_t  PartBlockRead:1U;        /* Partial blocks for read allowed */
  uint32_t  WrBlockMisalign:1U;      /* Write block misalignment */
  uint32_t  RdBlockMisalign:1U;      /* Read block misalignment */
  uint32_t  DSRImpl:1U;              /* DSR implemented */

  /* v1 or v2 struct */
  union csd_version
  {
    SD_Version_1_t v1;
    SD_Version_2_t v2;
  } version;

  uint32_t  EraseSingleBlockEnable:1U;  /* Erase single block enable */
  uint32_t  EraseSectorSize:7U;         /* Erase group size multiplier */
  uint32_t  WrProtectGrSize:7U;         /* Write protect group size */
  uint32_t  WrProtectGrEnable:1U;       /* Write protect group enable */
  uint32_t  Reserved2:2U;               /* Reserved */
  uint32_t  WrSpeedFact:3U;             /* Write speed factor */
  uint32_t  MaxWrBlockLen:4U;           /* Max. write data block length */
  uint32_t  WriteBlockPartial:1U;       /* Partial blocks for write allowed */
  uint32_t  Reserved3:5U;               /* Reserved */
  uint32_t  FileFormatGrouop:1U;        /* File format group */
  uint32_t  CopyFlag:1U;                /* Copy flag (OTP) */
  uint32_t  PermWrProtect:1U;           /* Permanent write protection */
  uint32_t  TempWrProtect:1U;           /* Temporary write protection */
  uint32_t  FileFormat:2U;              /* File Format */
  uint32_t  Reserved4:2U;               /* Reserved */
  uint32_t  crc:7U;                     /* Reserved */
  uint32_t  Reserved5:1U;               /* always 1*/

} SD_CardSpecificData_t;

/**
  * @brief  Card Identification Data: CID Register
  */
typedef struct
{
  uint32_t  ManufacturerID;       /* ManufacturerID */
  uint32_t  OEM_AppliID;          /* OEM/Application ID */
  uint32_t  ProdName1;            /* Product Name part1 */
  uint32_t  ProdName2;            /* Product Name part2*/
  uint32_t  ProdRev;              /* Product Revision */
  uint32_t  ProdSN;               /* Product Serial Number */
  uint32_t  Reserved1;            /* Reserved1 */
  uint32_t  ManufactDate;         /* Manufacturing Date */
  uint32_t  CID_CRC;              /* CID CRC */
  uint32_t  Reserved2;            /* always 1 */
} SD_CardIdData_t;

/**
  * @brief SD Card information
  */
typedef struct
{
  SD_CardSpecificData_t Csd;
  SD_CardIdData_t Cid;
  uint32_t CardCapacity;              /*!< Card Capacity */
  uint32_t CardBlockSize;             /*!< Card Block Size */
  uint32_t LogBlockNbr;               /*!< Specifies the Card logical Capacity in blocks   */
  uint32_t LogBlockSize;              /*!< Specifies logical block size in bytes           */
} SD_CardInfo_t;

/**
  * @}
  */

/** @defgroup ADAFRUIT_802_SPI_SD_Exported_Constants SD Exported Constants
  * @{
  */

/**
  * @brief  Block Size
  */
#define ADAFRUIT_SD_BLOCK_SIZE      512U

/**
  * @brief  SD instance number
  */
#define SD_INSTANCES_NBR           1UL

/**
  * @brief  SD transfer state definition
  */
#define SD_TRANSFER_OK             0U
#define SD_TRANSFER_BUSY           1U

/**
  * @brief  SD detection on its memory slot
  */
#define SD_PRESENT                 1UL
#define SD_NOT_PRESENT             0UL

/**
  * @brief  SD capacity type
  */
#define ADAFRUIT_802_CARD_SDSC     0UL /* SD Standard Capacity */
#define ADAFRUIT_802_CARD_SDHC     1UL /* SD High Capacity     */

/**
  * @brief SD Card information structure
  */
#define ADAFRUIT_802_SD_CardInfo_t SD_CardInfo_t

/**
  * @}
  */

/** @defgroup ADAFRUIT_802_SD_Exported_Functions SD Exported Functions
  * @{
  */
int32_t ADAFRUIT_802_SD_Init(uint32_t Instance);
int32_t ADAFRUIT_802_SD_DeInit(uint32_t Instance);
int32_t ADAFRUIT_802_SD_DetectITConfig(uint32_t Instance);
int32_t ADAFRUIT_802_SD_ReadBlocks(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr);
int32_t ADAFRUIT_802_SD_WriteBlocks(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr);
int32_t ADAFRUIT_802_SD_ReadBlocks_DMA(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr);
int32_t ADAFRUIT_802_SD_WriteBlocks_DMA(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr);
int32_t ADAFRUIT_802_SD_ReadBlocks_IT(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr);
int32_t ADAFRUIT_802_SD_WriteBlocks_IT(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr);
int32_t ADAFRUIT_802_SD_Erase(uint32_t Instance, uint32_t BlockIdx, uint32_t BlocksNbr);
int32_t ADAFRUIT_802_SD_GetCardState(uint32_t Instance);
int32_t ADAFRUIT_802_SD_GetCardInfo(uint32_t Instance, ADAFRUIT_802_SD_CardInfo_t *CardInfo);
int32_t ADAFRUIT_802_SD_IsDetected(uint32_t Instance);
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* ADAFRUIT_802_SD_H */

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
