/**
 ******************************************************************************
 * @file    exc80w32_reg.h
 * @author  MCD Application Team
 * @brief   Header of exc80w32_reg.c
 *          
 ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef EXC80W32_REG_H
#define EXC80W32_REG_H

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

/** @addtogroup EXC80W32
 * @{
 */

/** @defgroup EXC80W32_Exported_Constants Exported Constants
  * @{
  */ 

/*  */   
#define EXC80W32_READ_REG                             0x0FU  

/**
  * @}
  */ 

/** @defgroup EXC80W32_Exported_Types EXC80W32 Exported Types
 * @{
 */
typedef int32_t (*EXC80W32_Write_Func)(void *, uint8_t, uint8_t*, uint16_t);
typedef int32_t (*EXC80W32_Read_Func) (void *, uint8_t, uint8_t*, uint16_t);

typedef struct
{
  EXC80W32_Write_Func   WriteReg;
  EXC80W32_Read_Func    ReadReg;
  void                *handle;
} exc80w32_ctx_t;
/**
 * @}
 */

/** @addtogroup EXC80W32_Exported_Functions
 * @{
 */
int32_t exc80w32_write_reg(exc80w32_ctx_t *ctx, uint8_t reg, uint8_t *pbuf, uint16_t length);
int32_t exc80w32_read_reg(exc80w32_ctx_t *ctx, uint8_t reg, uint8_t *pbuf, uint16_t length);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif
#endif /* EXC80W32_REG_H */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

