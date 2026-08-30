/**
 ******************************************************************************
 * @file    ts3510_reg.h
 * @author  MCD Application Team
 * @brief   Header of ts3510_reg.c
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
#ifndef TS3510_REG_H
#define TS3510_REG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Macros --------------------------------------------------------------------*/

/** @addtogroup BSP
 * @{
 */

/** @addtogroup Component
 * @{
 */

/** @addtogroup TS3510
 * @{
 */

/** @defgroup TS3510_Exported_Constants Exported Constants
  * @{
  */ 

#define TS3510_READ_BLOCK_REG                     0x8AU
#define TS3510_SEND_CMD_REG                       0x00U

/**
  * @}
  */ 

/** @defgroup TS3510_Exported_Types TS3510 Exported Types
 * @{
 */
typedef int32_t (*TS3510_Write_Func)(void *, uint8_t, uint8_t*, uint16_t);
typedef int32_t (*TS3510_Read_Func) (void *, uint8_t, uint8_t*, uint16_t);

typedef struct
{
  TS3510_Write_Func   WriteReg;
  TS3510_Read_Func    ReadReg;
  void                *handle;
} ts3510_ctx_t;
/**
 * @}
 */

/** @addtogroup TS3510_Exported_Functions
 * @{
 */
int32_t ts3510_write_reg(ts3510_ctx_t *ctx, uint8_t reg, uint8_t *pbuf, uint16_t length);
int32_t ts3510_read_reg(ts3510_ctx_t *ctx, uint8_t reg, uint8_t *pbuf, uint16_t length);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif
#endif /* TS3510_REG_H */

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
