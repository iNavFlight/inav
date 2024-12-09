/**
  ******************************************************************************
  * @file    openbl_usart_cmd.h
  * @author  MCD Application Team
  * @brief   Header for openbl_usart_cmd.c module
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
#ifndef OPENBL_USART_CMD_H
#define OPENBL_USART_CMD_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "openbl_core.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define OPENBL_USART_VERSION                 0x31U               /* Open Bootloader USART protocol V3.1 */

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
OPENBL_CommandsTypeDef *OPENBL_USART_GetCommandsList(void);
void OPENBL_USART_SetCommandsList(OPENBL_CommandsTypeDef *pUsartCmd);
void OPENBL_USART_GetCommand(void);
void OPENBL_USART_GetVersion(void);
void OPENBL_USART_GetID(void);
void OPENBL_USART_ReadMemory(void);
void OPENBL_USART_WriteMemory(void);
void OPENBL_USART_Go(void);
void OPENBL_USART_ReadoutProtect(void);
void OPENBL_USART_ReadoutUnprotect(void);
void OPENBL_USART_EraseMemory(void);
void OPENBL_USART_WriteProtect(void);
void OPENBL_USART_WriteUnprotect(void);
void OPENBL_USART_SpecialCommand(void);
void OPENBL_USART_ExtendedSpecialCommand(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENBL_USART_CMD_H */
