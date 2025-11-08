/**
  ******************************************************************************
  * @file    stmpe811_reg.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the
  *          stmpe811.c IO expander registers driver.
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
#ifndef STMPE811_REG_H
#define STMPE811_REG_H

#ifdef __cplusplus
 extern "C" {
#endif   

#include <cmsis_compiler.h>

/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup Components
  * @{
  */
    
/** @addtogroup STMPE811
  * @{
  */    
   
/** @addtogroup STMPE811_Exported_Constants
  * @{
  */ 
    
/* Identification registers & System Control */ 
#define STMPE811_CHP_ID_LSB_REG         0x00U
#define STMPE811_CHP_ID_MSB_REG         0x01U
#define STMPE811_ID_VER_REG             0x02U
   
/* General Control Registers */ 
#define STMPE811_SYS_CTRL1_REG          0x03U
#define STMPE811_SYS_CTRL2_REG          0x04U
#define STMPE811_SPI_CFG_REG            0x08U 

/* Interrupt system Registers */ 
#define STMPE811_INT_CTRL_REG           0x09U
#define STMPE811_INT_EN_REG             0x0AU
#define STMPE811_INT_STA_REG            0x0BU
#define STMPE811_IO_INT_EN_REG          0x0CU
#define STMPE811_IO_INT_STA_REG         0x0DU

/* IO Registers */ 
#define STMPE811_IO_SET_PIN_REG         0x10U
#define STMPE811_IO_CLR_PIN_REG         0x11U
#define STMPE811_IO_MP_STA_REG          0x12U
#define STMPE811_IO_DIR_REG             0x13U
#define STMPE811_IO_ED_REG              0x14U
#define STMPE811_IO_RE_REG              0x15U
#define STMPE811_IO_FE_REG              0x16U
#define STMPE811_IO_AF_REG              0x17U

/* ADC Registers */ 
#define STMPE811_ADC_INT_EN_REG         0x0EU
#define STMPE811_ADC_INT_STA_REG        0x0FU
#define STMPE811_ADC_CTRL1_REG          0x20U
#define STMPE811_ADC_CTRL2_REG          0x21U
#define STMPE811_ADC_CAPT_REG           0x22U
#define STMPE811_ADC_DATA_CH0_REG       0x30U 
#define STMPE811_ADC_DATA_CH1_REG       0x32U 
#define STMPE811_ADC_DATA_CH2_REG       0x34U 
#define STMPE811_ADC_DATA_CH3_REG       0x36U 
#define STMPE811_ADC_DATA_CH4_REG       0x38U 
#define STMPE811_ADC_DATA_CH5_REG       0x3AU 
#define STMPE811_ADC_DATA_CH6_REG       0x3BU 
#define STMPE811_ADC_DATA_CH7_REG       0x3CU 

/* Touch Screen Registers */ 
#define STMPE811_TSC_CTRL_REG           0x40U
#define STMPE811_TSC_CFG_REG            0x41U
#define STMPE811_WDW_TR_X_REG           0x42U 
#define STMPE811_WDW_TR_Y_REG           0x44U
#define STMPE811_WDW_BL_X_REG           0x46U
#define STMPE811_WDW_BL_Y_REG           0x48U
#define STMPE811_FIFO_TH_REG            0x4AU
#define STMPE811_FIFO_STA_REG           0x4BU
#define STMPE811_FIFO_SIZE_REG          0x4CU
#define STMPE811_TSC_DATA_X_REG         0x4DU 
#define STMPE811_TSC_DATA_Y_REG         0x4FU
#define STMPE811_TSC_DATA_Z_REG         0x51U
#define STMPE811_TSC_DATA_XYZ_REG       0x52U 
#define STMPE811_TSC_FRACT_XYZ_REG      0x56U
#define STMPE811_TSC_DATA_INC_REG       0x57U
#define STMPE811_TSC_DATA_NON_INC_REG   0xD7U
#define STMPE811_TSC_I_DRIVE_REG        0x58U
#define STMPE811_TSC_SHIELD_REG         0x59U

/**
  * @}
  */ 
   
/** @addtogroup STMPE811_Exported_Types
  * @{
  */

typedef int32_t (*STMPE811_Write_Func)(void *, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*STMPE811_Read_Func) (void *, uint16_t, uint8_t*, uint16_t);

typedef struct
{
  STMPE811_Write_Func   WriteReg;
  STMPE811_Read_Func    ReadReg;
  void                  *handle;
  /* Internal resources */
} stmpe811_ctx_t;

/**
  * @}
  */ 

/** @addtgroup STMPE811_Exported_Functions
  * @{
  */
int32_t stmpe811_write_reg(stmpe811_ctx_t *ctx, uint16_t reg, uint8_t *data, uint16_t length);
int32_t stmpe811_read_reg(stmpe811_ctx_t *ctx, uint16_t reg, uint8_t *data, uint16_t length);

/**
  * @}
  */ 
#ifdef __cplusplus
}
#endif
#endif /* STMPE811_REG_H */

/**
  * @}
  */ 

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
