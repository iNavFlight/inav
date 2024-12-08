/**
  ******************************************************************************
  * @file    openbootloader_conf.h
  * @author  MCD Application Team
  * @brief   Contains Open Bootloader configuration
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef OPENBOOTLOADER_CONF_H
#define OPENBOOTLOADER_CONF_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "platform.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* -------------------------------- Device ID ------------------------------- */
#define DEVICE_ID_MSB                     0x04U  /* MSB byte of device ID */
#define DEVICE_ID_LSB                     0x82U  /* LSB byte of device ID */

/* -------------------------- Definitions for Memories ---------------------- */
#define FLASH_MEM_SIZE                    (2048U * 1024U)                 /* Size of Flash 2 MByte */
#define FLASH_START_ADDRESS               0x08000000U                     /* Flash start address */
#define FLASH_END_ADDRESS                 (FLASH_BASE + FLASH_MEM_SIZE)   /* Flash end address */

#define RAM_SIZE                          (768U * 1024U)                  /* Size of RAM 768 kByte */
#define RAM_START_ADDRESS                 0x20000000U                     /* SRAM start address  */
#define RAM_END_ADDRESS                   (RAM_START_ADDRESS + RAM_SIZE)  /* SRAM end address */

#define OB_SIZE                           432U                            /* Size of OB 432 Byte */
#define OB_START_ADDRESS                  0x40022050U                     /* Option bytes registers address */
#define OB1_START_ADDRESS                 OB_START_ADDRESS                /* Option Bytes 1 start address */
#define OB2_START_ADDRESS                 0x400221E0U                     /* Option Bytes 2 start address */
#define OB_END_ADDRESS                    (OB_START_ADDRESS + OB_SIZE)    /* Option bytes end address */

#define OTP_SIZE                          (2U * 1024U)                    /* Size of OTP 2048 Byte */
#define OTP_START_ADDRESS                 0x08FFF000U                     /* OTP start address */
#define OTP_END_ADDRESS                   (OTP_START_ADDRESS + OTP_SIZE)  /* OTP end address */

#define ICP_SIZE                          (35U * 1024U)                   /* Size of ICP 35 kByte */
#define ICP_START_ADDRESS                 0x0BF97000U                     /* System memory start address */
#define ICP_END_ADDRESS                   (ICP_START_ADDRESS + ICP_SIZE)  /* System memory end address */

#define EB_SIZE                           156U                            /* Size of Engi bytes 156 Byte */
#define EB_START_ADDRESS                  0x40022400U                     /* Engi bytes start address */
#define EB_END_ADDRESS                    (EB_START_ADDRESS + EB_SIZE)    /* Engi bytes end address */

#define OPENBL_RAM_SIZE                   0x11800U             /* RAM used by the Open Bootloader 71680 Bytes */

#define OPENBL_DEFAULT_MEM                FLASH_START_ADDRESS  /* Used for Erase and Write protect CMDs */

#define RDP_LEVEL_0                       OB_RDP_LEVEL_0
#define RDP_LEVEL_1                       OB_RDP_LEVEL_1
#define RDP_LEVEL_2                       OB_RDP_LEVEL_2

#define AREA_ERROR                        0x0U  /* Error Address Area */
#define FLASH_AREA                        0x1U  /* Flash Address Area */
#define RAM_AREA                          0x2U  /* RAM Address area */
#define OB_AREA                           0x3U  /* Option bytes Address area */
#define OTP_AREA                          0x4U  /* OTP Address area */
#define ICP_AREA                          0x5U  /* System memory area */
#define EB_AREA                           0x6U  /* Engi bytes Address area */

#define FLASH_MASS_ERASE                  0xFFFF
#define FLASH_BANK1_ERASE                 0xFFFE
#define FLASH_BANK2_ERASE                 0xFFFD

#define INTERFACES_SUPPORTED              6U

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENBOOTLOADER_CONF_H */
