/**
  ******************************************************************************
  * @file    s70kl1281.h
  * @author  MCD Application Team
  * @brief   This file contains all the description of the S70KL1281 Octal memory.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef S70KL1281_H
#define S70KL1281_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "s70kl1281_conf.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup S70KL1281
  * @{
  */

/** @defgroup S70KL1281_Exported_Types
  * @{
  */

/**
  * @}
  */

/** @defgroup S70KL1281_Exported_Constants
  * @{
  */

/**
  * @brief  S70KL1281 Configuration
  */
#define S70KL1281_RAM_SIZE          0x1000000       /* 128 MBits => 16 MBytes */

/**
  * @brief  MX25LM51245G Error codes
  */
#define S70KL1281_OK                           (0)
#define S70KL1281_ERROR                        (-1)

/******************************************************************************
  * @brief  S70KL1281 Registers
  ****************************************************************************/
/* Device Identification Register 0 */
#define S70KL1281_DIR0_ADDRESS        0x00000000U

#define S70KL1281_DIR0_MANUFACTURER   0x000FU     /*!< Manufacturer */
#define S70KL1281_DIR0_CADC           0x00F0U     /*!< Column Address Bit Count */
#define S70KL1281_DIR0_RABC           0x1F00U     /*!< Row Address Bit Count */
#define S70KL1281_DIR0_DADDR          0xC000U     /*!< Die Address */
#define S70KL1281_DIR0_DADDR_DIE_0    0x0000U     /*!< Die 0 Address */
#define S70KL1281_DIR0_DADDR_DIE_1    0x4000U     /*!< Die 1 Address */
#define S70KL1281_DIR0_DADDR_DIE_2    0x8000U     /*!< Die 2 Address */
#define S70KL1281_DIR0_DADDR_DIE_3    0xC000U     /*!< Die 3 Address */

/* Device Identification Register 1 */
#define S70KL1281_DIR1_ADDRESS        0x00000002U

#define S70KL1281_DIR1_DTYPE          0x000FU     /*!< Device Type */
#define S70KL1281_DIR1_DTYPE_HYPERRAM 0x0000U     /*!< HyperRAM Device Type */

/* Configuration Register 0 */
#define S70KL1281_CR0_ADDRESS         0x00001000U

#define S70KL1281_CR0_BLENGTH         0x0003U     /*!< Burst length */
#define S70KL1281_CR0_BLENGTH_128     0x0000U     /*!< 128 bytes burst length */
#define S70KL1281_CR0_BLENGTH_64      0x0001U     /*!< 64 bytes burst length */
#define S70KL1281_CR0_BLENGTH_16      0x0002U     /*!< 16 bytes burst length */
#define S70KL1281_CR0_BLENGTH_32      0x0003U     /*!< 32 bytes burst length */
#define S70KL1281_CR0_HBE             0x0004U     /*!< Hybrid burst enable */
#define S70KL1281_CR0_FLE             0x0008U     /*!< Fixed latency enable */
#define S70KL1281_CR0_IL              0x00F0U     /*!< Initial latency */
#define S70KL1281_CR0_IL_5_CLOCK      0x0000U     /*!< 5 clock latency */
#define S70KL1281_CR0_IL_6_CLOCK      0x0010U     /*!< 6 clock latency */
#define S70KL1281_CR0_IL_3_CLOCK      0x00E0U     /*!< 3 clock latency */
#define S70KL1281_CR0_IL_4_CLOCK      0x00F0U     /*!< 4 clock latency */
#define S70KL1281_CR0_DS              0x7000U     /*!< Drive strength */
#define S70KL1281_CR0_DS_34           0x0000U     /*!< 34 ohms drive strength */
#define S70KL1281_CR0_DS_115          0x1000U     /*!< 115 ohms drive strength */
#define S70KL1281_CR0_DS_67           0x2000U     /*!< 67 ohms drive strength */
#define S70KL1281_CR0_DS_46           0x3000U     /*!< 46 ohms drive strength */
#define S70KL1281_CR0_DS_34_2         0x4000U     /*!< 34 ohms drive strength */
#define S70KL1281_CR0_DS_27           0x5000U     /*!< 27 ohms drive strength */
#define S70KL1281_CR0_DS_22           0x6000U     /*!< 22 ohms drive strength */
#define S70KL1281_CR0_DS_19           0x7000U     /*!< 19 ohms drive strength */
#define S70KL1281_CR0_DPDE            0x8000U     /*!< Deep Power down enable */

/* Configuration Register 1 */
#define S70KL1281_CR1_ADDRESS         0x00001002U

#define S70KL1281_CR1_DRI             0x0003U     /*!< Distributed refresh interval */
#define S70KL1281_CR1_DRI_2_TIMES     0x0000U     /*!< 2 times default distributed refresh interval */
#define S70KL1281_CR1_DRI_4_TIMES     0x0001U     /*!< 4 times default distributed refresh interval */
#define S70KL1281_CR1_DRI_DEFAULT     0x0002U     /*!< Default distributed refresh interval */
#define S70KL1281_CR1_DRI_1_5_TIMES   0x0003U     /*!< 1.5 times default distributed refresh interval */

/**
  * @}
  */
/** @defgroup S70KL1281_Exported_Types S70KL1281Exported Types
  * @{
  */
typedef enum {
  S70KL1281_BURST_16_BYTES  = S70KL1281_CR0_BLENGTH_16,
  S70KL1281_BURST_32_BYTES  = S70KL1281_CR0_BLENGTH_32,
  S70KL1281_BURST_64_BYTES  = S70KL1281_CR0_BLENGTH_64,
  S70KL1281_BURST_128_BYTES = S70KL1281_CR0_BLENGTH_128
} S70KL1281_BurstLength_t;
/**
  * @}
  */

/** @defgroup S70KL1281_Exported_Functions
  * @{
  */
/* Read/Write Array Commands **************************************************/
int32_t S70KL1281_Read(OSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t ReadAddr, uint32_t Size);
int32_t S70KL1281_Read_DMA(OSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t ReadAddr, uint32_t Size);
int32_t S70KL1281_Write(OSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t WriteAddr, uint32_t Size);
int32_t S70KL1281_Write_DMA(OSPI_HandleTypeDef *Ctx, uint8_t *pData, uint32_t WriteAddr, uint32_t Size);
int32_t S70KL1281_EnableMemoryMappedMode(OSPI_HandleTypeDef *Ctx);

/* Register/Setting Commands **************************************************/
int32_t S70KL1281_ReadCfgReg0(OSPI_HandleTypeDef *Ctx, uint16_t *Value);
int32_t S70KL1281_WriteCfgReg0(OSPI_HandleTypeDef *Ctx, uint16_t Value, uint32_t LatencyMode, uint32_t InitialLatency);
int32_t S70KL1281_ReadCfgReg1(OSPI_HandleTypeDef *Ctx, uint16_t *Value);
int32_t S70KL1281_WriteCfgReg1(OSPI_HandleTypeDef *Ctx, uint16_t Value, uint32_t LatencyMode, uint32_t InitialLatency);

/* ID Commands ****************************************************************/
int32_t S70KL1281_ReadID(OSPI_HandleTypeDef *Ctx, uint16_t *ID);

/* Power down Commands ********************************************************/
int32_t S70KL1281_EnterPowerDown(OSPI_HandleTypeDef *Ctx, uint32_t LatencyMode, uint32_t InitialLatency);
int32_t S70KL1281_LeavePowerDown(OSPI_HandleTypeDef *Ctx);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* S70KL1281_H */

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
