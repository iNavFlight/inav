/**
  ******************************************************************************
  * @file    openbl_i2c_cmd.h
  * @author  MCD Application Team
  * @brief   Header for openbl_i2c_cmd.c module
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
#ifndef OPENBL_I2C_CMD_H
#define OPENBL_I2C_CMD_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "openbl_core.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define OPENBL_I2C_VERSION                 0x12U               /* Open Bootloader I2C protocol V1.2 */

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
OPENBL_CommandsTypeDef *OPENBL_I2C_GetCommandsList(void);
void OPENBL_I2C_SetCommandsList(OPENBL_CommandsTypeDef *pI2cCmd);
void OPENBL_I2C_GetCommand(void);
void OPENBL_I2C_GetVersion(void);
void OPENBL_I2C_GetID(void);
void OPENBL_I2C_ReadMemory(void);
void OPENBL_I2C_WriteMemory(void);
void OPENBL_I2C_Go(void);
void OPENBL_I2C_ReadoutProtect(void);
void OPENBL_I2C_ReadoutUnprotect(void);
void OPENBL_I2C_EraseMemory(void);
void OPENBL_I2C_WriteProtect(void);
void OPENBL_I2C_WriteUnprotect(void);
void OPENBL_I2C_NonStretchWriteMemory(void);
void OPENBL_I2C_NonStretchEraseMemory(void);
void OPENBL_I2C_NonStretchWriteProtect(void);
void OPENBL_I2C_NonStretchWriteUnprotect(void);
void OPENBL_I2C_NonStretchReadoutProtect(void);
void OPENBL_I2C_NonStretchReadoutUnprotect(void);
void OPENBL_I2C_SpecialCommand(void);
void OPENBL_I2C_ExtendedSpecialCommand(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENBL_I2C_CMD_H */
