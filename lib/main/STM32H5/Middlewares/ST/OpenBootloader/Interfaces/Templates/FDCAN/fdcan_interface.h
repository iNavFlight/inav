/**
  ******************************************************************************
  * @file    fdcan_interface.h
  * @author  MCD Application Team
  * @brief   Header for fdcan_interface.c module
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
#ifndef FDCAN_INTERFACE_H
#define FDCAN_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "openbl_core.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void OPENBL_FDCAN_Configuration(void);
void OPENBL_FDCAN_DeInit(void);
uint8_t OPENBL_FDCAN_ProtocolDetection(void);

uint8_t OPENBL_FDCAN_GetCommandOpcode(void);
uint8_t OPENBL_FDCAN_ReadByte(void);
void OPENBL_FDCAN_ReadBytes(uint8_t *Buffer, uint32_t BufferSize);
void OPENBL_FDCAN_SendByte(uint8_t Byte);
void OPENBL_FDCAN_SendBytes(uint8_t *Buffer, uint32_t BufferSize);
void OPENBL_FDCAN_SpecialCommandProcess(OPENBL_SpecialCmdTypeDef *Frame);

#ifdef __cplusplus
}
#endif

#endif /* FDCAN_INTERFACE_H */
