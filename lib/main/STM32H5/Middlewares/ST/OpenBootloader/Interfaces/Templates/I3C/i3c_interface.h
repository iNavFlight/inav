/**
  ******************************************************************************
  * @file    i3c_interface.h
  * @author  MCD Application Team
  * @brief   Header for i3c_interface.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef I3C_INTERFACE_H
#define I3C_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "common_interface.h"
#include "openbl_core.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void OPENBL_I3C_Configuration(void);
void OPENBL_I3C_DeInit(void);

uint8_t OPENBL_I3C_ProtocolDetection(void);
uint8_t OPENBL_I3C_GetCommandOpcode(void);
uint8_t OPENBL_I3C_ReadByte(void);
void OPENBL_I3C_SendByte(uint8_t Byte);
void OPENBL_I3C_SendAcknowledgeByte(uint8_t Acknowledge);
void OPENBL_I3C_SendBytes(uint8_t *pBuffer, uint32_t BufferSize);
void OPENBL_I3C_ReadBytes(uint8_t *pBuffer, uint32_t BufferSize);

void OPENBL_I3C_SpecialCommandProcess(OPENBL_SpecialCmdTypeDef *SpecialCmd);

void OPENBL_I3C_IRQHandler(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* I3C_INTERFACE_H */
