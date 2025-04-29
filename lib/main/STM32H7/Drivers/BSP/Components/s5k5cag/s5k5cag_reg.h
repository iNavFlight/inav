/**
  ******************************************************************************
  * @file    s5k5cag_reg.h
  * @author  MCD Application Team
  * @brief   Header of s5k5cag_reg.c
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
#ifndef S5K5CAG_REG_H
#define S5K5CAG_REG_H

#include <cmsis_compiler.h>

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/   
/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup Components
  * @{
  */ 
  
/** @addtogroup S5K5CAG
  * @{
  */

/** @defgroup S5K5CAG_Exported_Constants S5K5CAG Exported Constants
  * @{
  */

/** 
  * @brief  S5K5CAG ID
  */  
#define  S5K5CAG_ID                       0x05CAU
/** 
  * @brief  S5K5CAG Registers
  */
#define S5K5CAG_INFO_CHIPID1              0x0040U
#define S5K5CAG_INFO_CHIPID2              0x0042U
#define S5K5CAG_INFO_SVNVERSION           0x0048U
#define S5K5CAG_INFO_DATE                 0x004EU

/**
  * @}
  */

/** @addtogroup S5K5CAG_Exported_Types
  * @{
  */
typedef int32_t (*S5K5CAG_Write_Func)(void *, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*S5K5CAG_Read_Func) (void *, uint16_t, uint8_t*, uint16_t);

typedef struct
{
  S5K5CAG_Write_Func   WriteReg;
  S5K5CAG_Read_Func    ReadReg;
  void                *handle;
} s5k5cag_ctx_t;
/**
  * @}
  */

/** @addtogroup S5K5CAG_Exported_Functions
  * @{
  */
int32_t s5k5cag_write_reg(s5k5cag_ctx_t *ctx, uint16_t reg, uint16_t *data, uint16_t length);
int32_t s5k5cag_read_reg(s5k5cag_ctx_t *ctx, uint16_t reg, uint16_t* data, uint16_t length);

/**
  * @}
  */    
#ifdef __cplusplus
}
#endif

#endif /* S5K5CAG_REG_H */
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
