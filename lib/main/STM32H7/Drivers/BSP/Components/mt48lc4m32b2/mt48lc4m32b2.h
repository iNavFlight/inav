/**
  ******************************************************************************
  * @file    mt48lc4m32b2.h
  * @author  MCD Application Team
  * @brief   This file contains all the description of the MT48LC4M32B2 SDRAM
  *          memory.
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#ifndef MT48LC4M32B2_H
#define MT48LC4M32B2_H

#ifdef __cplusplus
 extern "C" {
#endif
   
/* Includes ------------------------------------------------------------------*/
#include "mt48lc4m32b2_conf.h"
/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup MT48LC4M32B2
  * @{
  */

/** @defgroup MT48LC4M32B2_Exported_Types MT48LC4M32B2 Exported Types
  * @{
  */
typedef struct 
{
  uint32_t TargetBank;           /*!< Target Bank                             */
  uint32_t RefreshMode;          /*!< Refresh Mode                            */
  uint32_t RefreshRate;          /*!< Refresh Rate                            */
  uint32_t BurstLength;          /*!< Burst Length                            */
  uint32_t BurstType;            /*!< Burst Type                              */
  uint32_t CASLatency;           /*!< CAS Latency                             */
  uint32_t OperationMode;        /*!< Operation Mode                          */
  uint32_t WriteBurstMode;       /*!< Write Burst Mode                        */
} MT48LC4M32B2_Context_t;

/**
  * @}
  */

/** @defgroup MT48LC4M32B2_Exported_Constants MT48LC4M32B2 Exported Constants
  * @{
  */
#define MT48LC4M32B2_OK                (0)
#define MT48LC4M32B2_ERROR             (-1)

/* Register Mode */
#define MT48LC4M32B2_BURST_LENGTH_1              0x00000000U
#define MT48LC4M32B2_BURST_LENGTH_2              0x00000001U
#define MT48LC4M32B2_BURST_LENGTH_4              0x00000002U
#define MT48LC4M32B2_BURST_LENGTH_8              0x00000004U
#define MT48LC4M32B2_BURST_TYPE_SEQUENTIAL       0x00000000U
#define MT48LC4M32B2_BURST_TYPE_INTERLEAVED      0x00000008U
#define MT48LC4M32B2_CAS_LATENCY_2               0x00000020U
#define MT48LC4M32B2_CAS_LATENCY_3               0x00000030U
#define MT48LC4M32B2_OPERATING_MODE_STANDARD     0x00000000U
#define MT48LC4M32B2_WRITEBURST_MODE_PROGRAMMED  0x00000000U 
#define MT48LC4M32B2_WRITEBURST_MODE_SINGLE      0x00000200U 

/* Command Mode */
#define MT48LC4M32B2_NORMAL_MODE_CMD             0x00000000U
#define MT48LC4M32B2_CLK_ENABLE_CMD              0x00000001U
#define MT48LC4M32B2_PALL_CMD                    0x00000002U
#define MT48LC4M32B2_AUTOREFRESH_MODE_CMD        0x00000003U
#define MT48LC4M32B2_LOAD_MODE_CMD               0x00000004U
#define MT48LC4M32B2_SELFREFRESH_MODE_CMD        0x00000005U
#define MT48LC4M32B2_POWERDOWN_MODE_CMD          0x00000006U

/**
  * @}
  */ 

/** @addtogroup MT48LC4M32B2_Exported_Functions
  * @{
  */
int32_t MT48LC4M32B2_Init(SDRAM_HandleTypeDef *Ctx, MT48LC4M32B2_Context_t *pRegMode);
int32_t MT48LC4M32B2_ClockEnable(SDRAM_HandleTypeDef *Ctx, uint32_t Interface);
int32_t MT48LC4M32B2_Precharge(SDRAM_HandleTypeDef *Ctx, uint32_t Interface);
int32_t MT48LC4M32B2_ModeRegConfig(SDRAM_HandleTypeDef *Ctx, MT48LC4M32B2_Context_t *pRegMode);
int32_t MT48LC4M32B2_TimingConfig(SDRAM_HandleTypeDef *Ctx, FMC_SDRAM_TimingTypeDef *pTiming);
int32_t MT48LC4M32B2_RefreshMode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface, uint32_t RefreshMode);
int32_t MT48LC4M32B2_RefreshRate(SDRAM_HandleTypeDef *Ctx, uint32_t RefreshCount);
int32_t MT48LC4M32B2_EnterPowerMode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface);
int32_t MT48LC4M32B2_ExitPowerMode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface);
int32_t MT48LC4M32B2_Sendcmd(SDRAM_HandleTypeDef *Ctx, FMC_SDRAM_CommandTypeDef *SdramCmd);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* MT48LC4M32B2_H */

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
