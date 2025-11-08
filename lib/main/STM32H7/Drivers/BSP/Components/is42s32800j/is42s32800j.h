/**
  ******************************************************************************
  * @file    is42s32800j.h
  * @author  MCD Application Team
  * @brief   This file contains all the description of the IS42S32800J SDRAM
  *          memory.
  *
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef IS42S32800J_H
#define IS42S32800J_H

#ifdef __cplusplus
 extern "C" {
#endif
   
/* Includes ------------------------------------------------------------------*/
#include "is42s32800j_conf.h"
/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup IS42S32800J
  * @{
  */

/** @defgroup IS42S32800J_Exported_Types Exported Types
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
} IS42S32800J_Context_t;

/**
  * @}
  */

/** @defgroup IS42S32800J_Exported_Constants
  * @{
  */
#define IS42S32800J_OK                (0)
#define IS42S32800J_ERROR             (-1)

/* Register Mode */
#define IS42S32800J_BURST_LENGTH_1              0x00000000U
#define IS42S32800J_BURST_LENGTH_2              0x00000001U
#define IS42S32800J_BURST_LENGTH_4              0x00000002U
#define IS42S32800J_BURST_LENGTH_8              0x00000004U
#define IS42S32800J_BURST_TYPE_SEQUENTIAL       0x00000000U
#define IS42S32800J_BURST_TYPE_INTERLEAVED      0x00000008U
#define IS42S32800J_CAS_LATENCY_2               0x00000020U
#define IS42S32800J_CAS_LATENCY_3               0x00000030U
#define IS42S32800J_OPERATING_MODE_STANDARD     0x00000000U
#define IS42S32800J_WRITEBURST_MODE_PROGRAMMED  0x00000000U 
#define IS42S32800J_WRITEBURST_MODE_SINGLE      0x00000200U 

/* Command Mode */
#define IS42S32800J_NORMAL_MODE_CMD             0x00000000U
#define IS42S32800J_CLK_ENABLE_CMD              0x00000001U
#define IS42S32800J_PALL_CMD                    0x00000002U
#define IS42S32800J_AUTOREFRESH_MODE_CMD        0x00000003U
#define IS42S32800J_LOAD_MODE_CMD               0x00000004U
#define IS42S32800J_SELFREFRESH_MODE_CMD        0x00000005U
#define IS42S32800J_POWERDOWN_MODE_CMD          0x00000006U

/**
  * @}
  */ 

/** @addtogroup IS42S32800J_Private_Functions
  * @{
  */
int32_t IS42S32800J_Init(SDRAM_HandleTypeDef *Ctx, IS42S32800J_Context_t *pRegMode);
int32_t IS42S32800J_ClockEnable(SDRAM_HandleTypeDef *Ctx, uint32_t Interface);
int32_t IS42S32800J_Precharge(SDRAM_HandleTypeDef *Ctx, uint32_t Interface);
int32_t IS42S32800J_ModeRegConfig(SDRAM_HandleTypeDef *Ctx, IS42S32800J_Context_t *pRegMode);
int32_t IS42S32800J_TimingConfig(SDRAM_HandleTypeDef *Ctx, FMC_SDRAM_TimingTypeDef *pTiming);
int32_t IS42S32800J_RefreshMode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface, uint32_t RefreshMode);
int32_t IS42S32800J_RefreshRate(SDRAM_HandleTypeDef *Ctx, uint32_t RefreshCount);
int32_t IS42S32800J_EnterPowerMode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface);
int32_t IS42S32800J_ExitPowerMode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface);
int32_t IS42S32800J_Sendcmd(SDRAM_HandleTypeDef *Ctx, FMC_SDRAM_CommandTypeDef *SdramCmd);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* IS42S32800J_H */

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
