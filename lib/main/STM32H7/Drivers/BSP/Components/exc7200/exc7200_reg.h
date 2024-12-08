/**
 ******************************************************************************
 * @file    exc7200_reg.h
 * @author  MCD Application Team
 * @brief   Header of exc7200_regs.c
 *          
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
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
#ifndef EXC7200_REG_H
#define EXC7200_REG_H

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

/** @addtogroup EXC7200
 * @{
 */

/** @defgroup EXC7200_Exported_Constants Exported Constants
  * @{
  */ 

/*  */   
#define EXC7200_READ_REG                             0x09U  

/**
  * @}
  */ 

/** @defgroup EXC7200_Exported_Types EXC7200 Exported Types
 * @{
 */
typedef int32_t (*EXC7200_Write_Func)(void *, uint8_t, uint8_t*, uint16_t);
typedef int32_t (*EXC7200_Read_Func) (void *, uint8_t, uint8_t*, uint16_t);

typedef struct
{
  EXC7200_Write_Func   WriteReg;
  EXC7200_Read_Func    ReadReg;
  void                *handle;
} exc7200_ctx_t;
/**
 * @}
 */

/** @addtogroup EXC7200_Exported_Functions
 * @{
 */
int32_t exc7200_write_reg(exc7200_ctx_t *ctx, uint8_t reg, uint8_t *pbuf, uint16_t length);
int32_t exc7200_read_reg(exc7200_ctx_t *ctx, uint8_t reg, uint8_t *pbuf, uint16_t length);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif
#endif /* EXC7200_REG_H */

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
