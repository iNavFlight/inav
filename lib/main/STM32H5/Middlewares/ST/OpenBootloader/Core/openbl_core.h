/**
  ******************************************************************************
  * @file    openbl_core.h
  * @author  MCD Application Team
  * @brief   Header for openbl_core.c module
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
#ifndef OPENBL_CORE_H
#define OPENBL_CORE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

#include "openbootloader_conf.h"

/* Exported constants --------------------------------------------------------*/
#define ERROR_COMMAND                     0xECU             /* Error command */
#define ACK_BYTE                          0x79U             /* Acknowledge Byte ID */
#define NACK_BYTE                         0x1FU             /* No Acknowledge Byte ID */
#define BUSY_BYTE                         0x76U             /* Busy Byte */
#define SYNC_BYTE                         0xA5U             /* Synchronization byte */
#define SPECIAL_CMD_SIZE_BUFFER1          128U              /* Special command received data buffer size */
#define SPECIAL_CMD_SIZE_BUFFER2          1024U             /* Special command write data buffer size */

/* ---------------------- Open Bootloader Commands ---------------------------*/
#define CMD_GET_COMMAND                   0x00U             /* Get commands command */
#define CMD_GET_VERSION                   0x01U             /* Get Version command */
#define CMD_GET_ID                        0x02U             /* Get ID command */
#define CMD_SPEED                         0x03U             /* Speed command */
#define CMD_READ_MEMORY                   0x11U             /* Read Memory command */
#define CMD_WRITE_MEMORY                  0x31U             /* Write Memory command */
#define CMD_GO                            0x21U             /* GO command */
#define CMD_READ_PROTECT                  0x82U             /* Readout Protect command */
#define CMD_READ_UNPROTECT                0x92U             /* Readout Unprotect command */
#define CMD_LEG_ERASE_MEMORY              0x43U             /* Erase Memory command */
#define CMD_EXT_ERASE_MEMORY              0x44U             /* Erase Memory command */
#define CMD_WRITE_PROTECT                 0x63U             /* Write Protect command */
#define CMD_WRITE_UNPROTECT               0x73U             /* Write Unprotect command */
#define CMD_NS_WRITE_MEMORY               0x32U             /* No Stretch Write Memory command */
#define CMD_NS_ERASE_MEMORY               0x45U             /* No Stretch Erase Memory command */
#define CMD_NS_WRITE_PROTECT              0x64U             /* No Stretch Write Protect command */
#define CMD_NS_WRITE_UNPROTECT            0x74U             /* No Stretch Write Unprotect command */
#define CMD_NS_READ_PROTECT               0x83U             /* No Stretch Read Protect command */
#define CMD_NS_READ_UNPROTECT             0x93U             /* No Stretch Read Unprotect command */
#define CMD_SPECIAL_COMMAND               0x50U             /* Special Command command */
#define CMD_EXTENDED_SPECIAL_COMMAND      0x51U             /* Extended Special Command command */
#define CMD_CHECKSUM                      0xA1U             /* Checksum command */

/* Exported types ------------------------------------------------------------*/
typedef struct
{
  void (*Init)(void);
  void (*DeInit)(void);
  uint8_t (*Detection)(void);
  uint8_t (*GetCommandOpcode)(void);
  void (*SendByte)(uint8_t Byte);
} OPENBL_OpsTypeDef;

typedef struct
{
  void (*GetCommand)(void);
  void (*GetVersion)(void);
  void (*GetID)(void);
  void (*ReadMemory)(void);
  void (*WriteMemory)(void);
  void (*Go)(void);
  void (*ReadoutProtect)(void);
  void (*ReadoutUnprotect)(void);
  void (*EraseMemory)(void);
  void (*WriteProtect)(void);
  void (*WriteUnprotect)(void);
  void (*NsWriteMemory)(void);
  void (*NsEraseMemory)(void);
  void (*NsWriteProtect)(void);
  void (*NsWriteUnprotect)(void);
  void (*NsReadoutProtect)(void);
  void (*NsReadoutUnprotect)(void);
  void (*Speed)(void);
  void (*SpecialCommand)(void);
  void (*ExtendedSpecialCommand)(void);
} OPENBL_CommandsTypeDef;

typedef struct
{
  OPENBL_OpsTypeDef *p_Ops;
  OPENBL_CommandsTypeDef *p_Cmd;
} OPENBL_HandleTypeDef;

typedef enum
{
  OPENBL_SPECIAL_CMD          = 0x1U,
  OPENBL_EXTENDED_SPECIAL_CMD = 0x2U
} OPENBL_SpecialCmdTypeTypeDef;

typedef struct
{
  OPENBL_SpecialCmdTypeTypeDef CmdType;
  uint16_t OpCode;
  uint16_t SizeBuffer1;
  uint8_t Buffer1[SPECIAL_CMD_SIZE_BUFFER1];
  uint16_t SizeBuffer2;
  uint8_t Buffer2[SPECIAL_CMD_SIZE_BUFFER2];
} OPENBL_SpecialCmdTypeDef;

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void OPENBL_Init(void);
void OPENBL_DeInit(void);
void OPENBL_InterfacesDeInit(void);
uint32_t OPENBL_InterfaceDetection(void);
void OPENBL_CommandProcess(void);
ErrorStatus OPENBL_RegisterInterface(OPENBL_HandleTypeDef *Interface);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPENBL_CORE_H */
