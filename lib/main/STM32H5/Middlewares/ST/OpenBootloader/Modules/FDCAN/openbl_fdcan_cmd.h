/**
  ******************************************************************************
  * @file    openbl_fdcan_cmd.h
  * @author  MCD Application Team
  * @brief   Header for openbl_fdcan_cmd.c module
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
#ifndef OPENBL_FDCAN_CMD_H
#define OPENBL_FDCAN_CMD_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define OPENBL_FDCAN_VERSION             0x10U      /* Open Bootloader FDCAN protocol V1.0 */
#define FDCAN_RAM_BUFFER_SIZE            1164U      /* Size of FDCAN buffer used to store received data from the host */

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern uint8_t TxData[FDCAN_RAM_BUFFER_SIZE];
extern uint8_t RxData[FDCAN_RAM_BUFFER_SIZE];

/* Exported functions ------------------------------------------------------- */
OPENBL_CommandsTypeDef *OPENBL_FDCAN_GetCommandsList(void);
void OPENBL_FDCAN_SetCommandsList(OPENBL_CommandsTypeDef *pFdcanCmd);
void OPENBL_FDCAN_GetCommand(void);
void OPENBL_FDCAN_GetVersion(void);
void OPENBL_FDCAN_GetID(void);
void OPENBL_FDCAN_ReadMemory(void);
void OPENBL_FDCAN_WriteMemory(void);
void OPENBL_FDCAN_Go(void);
void OPENBL_FDCAN_ReadoutProtect(void);
void OPENBL_FDCAN_ReadoutUnprotect(void);
void OPENBL_FDCAN_EraseMemory(void);
void OPENBL_FDCAN_WriteProtect(void);
void OPENBL_FDCAN_WriteUnprotect(void);
void OPENBL_FDCAN_SpecialCommand(void);
void OPENBL_FDCAN_ExtendedSpecialCommand(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENBL_FDCAN_CMD_H */
