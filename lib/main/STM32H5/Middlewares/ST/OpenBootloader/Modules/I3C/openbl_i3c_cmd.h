/**
  ******************************************************************************
  * @file    openbl_i3c_cmd.h
  * @author  MCD Application Team
  * @brief   Header for openbl_i3c_cmd.c module
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
#ifndef OPENBL_I3C_CMD_H
#define OPENBL_I3C_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "openbl_core.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define OPENBL_I3C_VERSION                 0x10U               /* Open Bootloader I3C protocol V1.0 */

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
OPENBL_CommandsTypeDef *OPENBL_I3C_GetCommandsList(void);
void OPENBL_I3C_SetCommandsList(OPENBL_CommandsTypeDef *pI3cCmd);
void OPENBL_I3C_GetCommand(void);
void OPENBL_I3C_GetVersion(void);
void OPENBL_I3C_GetID(void);
void OPENBL_I3C_ReadMemory(void);
void OPENBL_I3C_WriteMemory(void);
void OPENBL_I3C_Go(void);
void OPENBL_I3C_EraseMemory(void);
void OPENBL_I3C_WriteProtect(void);
void OPENBL_I3C_WriteUnprotect(void);
void OPENBL_I3C_SpecialCommand(void);
void OPENBL_I3C_ExtendedSpecialCommand(void);

#ifdef __cplusplus
}
#endif

#endif /* OPENBL_I3C_CMD_H */
