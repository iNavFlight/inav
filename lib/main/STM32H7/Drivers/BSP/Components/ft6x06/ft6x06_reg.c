/**
  ******************************************************************************
  * @file    ft6x06_reg.c
  * @author  MCD Application Team
  * @brief   This file provides unitary register function to control the ft6x06 Touch
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

/* Includes ------------------------------------------------------------------*/
#include "ft6x06_reg.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @defgroup FT6X06
  * @{
  */

/*******************************************************************************
* Function Name : ft6x06_read_reg
* Description   : Generic Reading function. It must be full-filled with either
*                 I2C or SPI reading functions
* Input         : Register Address, length of buffer
* Output        : pdata Read
*******************************************************************************/
int32_t ft6x06_read_reg(ft6x06_ctx_t *ctx, uint8_t reg, uint8_t* pdata, uint16_t length)
{
  return ctx->ReadReg(ctx->handle, reg, pdata, length);
}

/*******************************************************************************
* Function Name : ft6x06_write_reg
* Description   : Generic Writing function. It must be full-filled with either
*                 I2C or SPI writing function
* Input         : Register Address, pdata to be written, length of buffer
* Output        : None
*******************************************************************************/
int32_t ft6x06_write_reg(ft6x06_ctx_t *ctx, uint8_t reg, uint8_t *pdata, uint16_t length)
{
  return ctx->WriteReg(ctx->handle, reg, pdata, length);
}

/**************** Base Function  *******************/

/*******************************************************************************
* Function Name  : ft6x06_dev_mode_w
* Description    : Write Device Mode
* Input          : uint8_t
* Output         : None
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_dev_mode_w(ft6x06_ctx_t *ctx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;
  
  ret = ft6x06_read_reg(ctx, FT6X06_DEV_MODE_REG, &tmp, 1);
  
  if(ret == 0)
  {
    tmp &= ~FT6X06_DEV_MODE_BIT_MASK;
    tmp |= value << FT6X06_DEV_MODE_BIT_POSITION;
    
    ret = ft6x06_write_reg(ctx, FT6X06_DEV_MODE_REG, &tmp, 1);
  }
  
  return ret;
}

/*******************************************************************************
* Function Name  : ft6x06_dev_mode_r
* Description    : Read Device Mode
* Input          : Pointer to uint8_t
* Output         : None
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_dev_mode_r(ft6x06_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;
  
  ret = ft6x06_read_reg(ctx, FT6X06_DEV_MODE_REG, (uint8_t *)value, 1);
  
  if(ret == 0)
  {
    *value &= FT6X06_DEV_MODE_BIT_MASK; 
    *value = *value >> FT6X06_DEV_MODE_BIT_POSITION;
  }
  
  return ret;
}

/*******************************************************************************
* Function Name  : ft6x06_gest_id
* Description    : Read Gesture ID
* Input          : Pointer to uint8_t
* Output         : Status of GEST_ID register
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_gest_id(ft6x06_ctx_t *ctx, uint8_t *value)
{
  return ft6x06_read_reg(ctx, FT6X06_GEST_ID_REG, (uint8_t *)value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_td_status
* Description    : Read Number of Touch Points
* Input          : Pointer to uint8_t
* Output         : Status of TD_STATUS register
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_td_status(ft6x06_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;
  
  ret = ft6x06_read_reg(ctx, FT6X06_TD_STAT_REG, (uint8_t *)value, 1);
  
  if(ret == 0)
  {
    *value &= FT6X06_TD_STATUS_BIT_MASK; 
    *value = *value >> FT6X06_TD_STATUS_BIT_POSITION; 
  }
  
  return ret;
}

/*******************************************************************************
* Function Name  : ft6x06_p1_xh_ef
* Description    : Read First X Event Flag
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_P1_XH[7:6] bits
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_p1_xh_ef(ft6x06_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;
  
  ret = ft6x06_read_reg(ctx, FT6X06_P1_XH_REG, (uint8_t *)value, 1);
  
  if(ret == 0)
  {
    *value &= FT6X06_P1_XH_EF_BIT_MASK; 
    *value = *value >> FT6X06_P1_XH_EF_BIT_POSITION; 
  }
  
  return ret;
}

/*******************************************************************************
* Function Name  : ft6x06_p1_xh_tp
* Description    : Read First X High Touch Position
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_P1_XH[3:0] bits
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_p1_xh_tp(ft6x06_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;
  
  ret = ft6x06_read_reg(ctx, FT6X06_P1_XH_REG, (uint8_t *)value, 1);
  
  if(ret == 0)
  {
    *value &= FT6X06_P1_XH_TP_BIT_MASK; 
    *value = *value >> FT6X06_P1_XH_TP_BIT_POSITION; 
  }
  
  return ret;
}

/*******************************************************************************
* Function Name  : ft6x06_p1_xl_tp
* Description    : Read First X Low Touch Position
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_P1_XL register
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_p1_xl_tp(ft6x06_ctx_t *ctx, uint8_t *value)
{
  return ft6x06_read_reg(ctx, FT6X06_P1_XL_REG, (uint8_t *)value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_p1_yh_tid
* Description    : Read First Touch ID
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_P1_YH[7:4] bits
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_p1_yh_tid(ft6x06_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;
  
  ret = ft6x06_read_reg(ctx, FT6X06_P1_YH_REG, (uint8_t *)value, 1);
  
  if(ret == 0)
  {
    *value &= FT6X06_P1_YH_TID_BIT_MASK; 
    *value = *value >> FT6X06_P1_YH_TID_BIT_POSITION; 
  }
  
  return ret;
}

/*******************************************************************************
* Function Name  : ft6x06_p1_yh_tp
* Description    : Read First Y High Touch Position
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_P1_YH[3:0] bits
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_p1_yh_tp(ft6x06_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;
  
  ret = ft6x06_read_reg(ctx, FT6X06_P1_YH_REG, (uint8_t *)value, 1);
  
  if(ret == 0)
  {
    *value &= FT6X06_P1_YH_TP_BIT_MASK; 
    *value = *value >> FT6X06_P1_YH_TP_BIT_POSITION; 
  }
  
  return ret;
}

/*******************************************************************************
* Function Name  : ft6x06_p1_yl_tp
* Description    : Read First Y Low Touch Position
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_P1_YL register
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_p1_yl_tp(ft6x06_ctx_t *ctx, uint8_t *value)
{
  return ft6x06_read_reg(ctx, FT6X06_P1_YL_REG, (uint8_t *)value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_p1_weight
* Description    : Read First Touch Weight
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_P1_WEIGHT register
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_p1_weight(ft6x06_ctx_t *ctx, uint8_t *value)
{
  return ft6x06_read_reg(ctx, FT6X06_P1_WEIGHT_REG, (uint8_t *)value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_p1_misc
* Description    : Read First Touch Area
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_P1_MISC register
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_p1_misc(ft6x06_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;
  
  ret = ft6x06_read_reg(ctx, FT6X06_P1_MISC_REG, (uint8_t *)value, 1);  
  if(ret == 0)
  {
    *value &= FT6X06_P1_MISC_BIT_MASK; 
    *value = *value >> FT6X06_P1_MISC_BIT_POSITION; 
  }
  
  return ret;
}

/*******************************************************************************
* Function Name  : ft6x06_p2_xh_ef
* Description    : Read Second X Event Flag
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_P2_XH[7:6] bits
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_p2_xh_ef(ft6x06_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;
  
  ret = ft6x06_read_reg(ctx, FT6X06_P2_XH_REG, (uint8_t *)value, 1);
  
  if(ret == 0)
  {
    *value &= FT6X06_P2_XH_EF_BIT_MASK; 
    *value = *value >> FT6X06_P2_XH_EF_BIT_POSITION; 
  }
  
  return ret;
}

/*******************************************************************************
* Function Name  : ft6x06_p2_xh_tp
* Description    : Read Second X High Touch Position
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_P2_XH[3:0] bits
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_p2_xh_tp(ft6x06_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;
  
  ret = ft6x06_read_reg(ctx, FT6X06_P2_XH_REG, (uint8_t *)value, 1);
  
  if(ret == 0)
  {
    *value &= FT6X06_P2_XH_TP_BIT_MASK; 
    *value = *value >> FT6X06_P2_XH_TP_BIT_POSITION; 
  }
  
  return ret;
}

/*******************************************************************************
* Function Name  : ft6x06_p2_xl_tp
* Description    : Read Second X Low Touch Position
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_P2_XL register
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_p2_xl_tp(ft6x06_ctx_t *ctx, uint8_t *value)
{
  return ft6x06_read_reg(ctx, FT6X06_P2_XL_REG, (uint8_t *)value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_p2_yh_tid
* Description    : Read Second Touch ID
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_P2_YH[7:4] bits
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_p2_yh_tid(ft6x06_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;
  
  ret = ft6x06_read_reg(ctx, FT6X06_P2_YH_REG, (uint8_t *)value, 1);
  
  if(ret == 0)
  {
    *value &= FT6X06_P2_YH_TID_BIT_MASK; 
    *value = *value >> FT6X06_P2_YH_TID_BIT_POSITION; 
  }
  
  return ret;
}

/*******************************************************************************
* Function Name  : ft6x06_p2_yh_tp
* Description    : Read Second Y High Touch Position
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_P2_YH[3:0] bits
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_p2_yh_tp(ft6x06_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;
  
  ret = ft6x06_read_reg(ctx, FT6X06_P2_YH_REG, (uint8_t *)value, 1);
  
  if(ret == 0)
  {
    *value &= FT6X06_P2_YH_TP_BIT_MASK; 
    *value = *value >> FT6X06_P2_YH_TP_BIT_POSITION; 
  }
  
  return ret;
}

/*******************************************************************************
* Function Name  : ft6x06_p2_yl_tp
* Description    : Read Second Y Low Touch Position
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_P2_YL register
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_p2_yl_tp(ft6x06_ctx_t *ctx, uint8_t *value)
{
  return ft6x06_read_reg(ctx, FT6X06_P2_YL_REG, (uint8_t *)value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_p2_weight
* Description    : Read Second Touch Weight
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_P2_WEIGHT register
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_p2_weight(ft6x06_ctx_t *ctx, uint8_t *value)
{
  return ft6x06_read_reg(ctx, FT6X06_P2_WEIGHT_REG, (uint8_t *)value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_p2_misc
* Description    : Read Second Touch Area
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_P2_MISC register
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_p2_misc(ft6x06_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;
  
  ret = ft6x06_read_reg(ctx, FT6X06_P2_MISC_REG, (uint8_t *)value, 1);
  
  if(ret == 0)
  {
    *value &= FT6X06_P2_MISC_BIT_MASK; 
    *value = *value >> FT6X06_P2_MISC_BIT_POSITION; 
  }
  
  return ret;
}

/*******************************************************************************
* Function Name  : ft6x06_th_group
* Description    : Set Threshold for touch detection
* Input          : uint8_t
* Output         : None
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_th_group(ft6x06_ctx_t *ctx, uint8_t value)
{
  return ft6x06_write_reg(ctx, FT6X06_TH_GROUP_REG, &value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_th_diff
* Description    : Set Filter function coefficient
* Input          : uint8_t
* Output         : None
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_th_diff(ft6x06_ctx_t *ctx, uint8_t value)
{
  return ft6x06_write_reg(ctx, FT6X06_TH_DIFF_REG, &value, 1);
}

/*******************************************************************************
* Function Name  : FT6X06_CTRL
* Description    : Control the Switch between Active and Monitoring Mode 
* Input          : uint8_t
* Output         : None
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_ctrl(ft6x06_ctx_t *ctx, uint8_t value)
{
  return ft6x06_write_reg(ctx, FT6X06_CTRL_REG, &value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_time_enter_monitor
* Description    : Set the time period of switching from Active mode to Monitor 
*                  mode when there is no touching.
* Input          : uint8_t
* Output         : None
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_time_enter_monitor(ft6x06_ctx_t *ctx, uint8_t value)
{
  return ft6x06_write_reg(ctx, FT6X06_TIMEENTERMONITOR_REG, &value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_period_active
* Description    : Report rate in Active mode
* Input          : uint8_t
* Output         : None
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_period_active(ft6x06_ctx_t *ctx, uint8_t value)
{
  return ft6x06_write_reg(ctx, FT6X06_PERIODACTIVE_REG, &value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_period_monitor
* Description    : Report rate in Monitor mode
* Input          : uint8_t
* Output         : None
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_period_monitor(ft6x06_ctx_t *ctx, uint8_t value)
{
  return ft6x06_write_reg(ctx, FT6X06_PERIODMONITOR_REG, &value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_radian_value
* Description    : Set the value of the minimum allowed angle while Rotating 
*                  gesture mode
* Input          : uint8_t
* Output         : None
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_radian_value(ft6x06_ctx_t *ctx, uint8_t value)
{
  return ft6x06_write_reg(ctx, FT6X06_RADIAN_VALUE_REG, &value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_offset_left_right
* Description    : Set Maximum offset while Moving Left and Moving Right gesture
* Input          : uint8_t
* Output         : None
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_offset_left_right(ft6x06_ctx_t *ctx, uint8_t value)
{
  return ft6x06_write_reg(ctx, FT6X06_OFFSET_LR_REG, &value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_offset_up_down
* Description    : Set Maximum offset while Moving Up and Moving Down gesture
* Input          : uint8_t
* Output         : None
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_offset_up_down(ft6x06_ctx_t *ctx, uint8_t value)
{
  return ft6x06_write_reg(ctx, FT6X06_OFFSET_UD_REG, &value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_disatnce_left_right
* Description    : Set Minimum distance while Moving Left and Moving Right gesture
* Input          : uint8_t
* Output         : None
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_disatnce_left_right(ft6x06_ctx_t *ctx, uint8_t value)
{
  return ft6x06_write_reg(ctx, FT6X06_DISTANCE_LR_REG, &value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_offset_up_down
* Description    : Set Minimum distance while Moving Up and Moving Down gesture
* Input          : uint8_t
* Output         : None
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_distance_up_down(ft6x06_ctx_t *ctx, uint8_t value)
{
  return ft6x06_write_reg(ctx, FT6X06_DISTANCE_UD_REG, &value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_distance_zoom
* Description    : Set Maximum distance while Zoom In and Zoom Out gesture
* Input          : uint8_t
* Output         : None
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_distance_zoom(ft6x06_ctx_t *ctx, uint8_t value)
{
  return ft6x06_write_reg(ctx, FT6X06_DISTANCE_ZOOM_REG, &value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_lib_ver_high
* Description    : Read High 8-bit of LIB Version info
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_LIB_VER_H register
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_lib_ver_high(ft6x06_ctx_t *ctx, uint8_t *value)
{
  return ft6x06_read_reg(ctx, FT6X06_LIB_VER_H_REG, (uint8_t *)value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_lib_ver_low
* Description    : Read Low 8-bit of LIB Version info
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_LIB_VER_L register
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_lib_ver_low(ft6x06_ctx_t *ctx, uint8_t *value)
{
  return ft6x06_read_reg(ctx, FT6X06_LIB_VER_L_REG, (uint8_t *)value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_cipher
* Description    : Chip Selecting
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_CIPHER register
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_cipher(ft6x06_ctx_t *ctx, uint8_t *value)
{
  return ft6x06_read_reg(ctx, FT6X06_CIPHER_REG, (uint8_t *)value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_g_mode
* Description    : Select Interrupt (polling or trigger) mode
* Input          : uint8_t
* Output         : None
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_g_mode(ft6x06_ctx_t *ctx, uint8_t value)
{
  return ft6x06_write_reg(ctx, FT6X06_GMODE_REG, &value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_pwd_mode
* Description    : Select Interrupt (polling or trigger) mode
* Input          : uint8_t
* Output         : None
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_pwr_mode(ft6x06_ctx_t *ctx, uint8_t value)
{
  return ft6x06_write_reg(ctx, FT6X06_PWR_MODE_REG, &value, 1); 
}

/*******************************************************************************
* Function Name  : ft6x06_firm_id
* Description    : Firmware Version
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_FIRMID register
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_firm_id(ft6x06_ctx_t *ctx, uint8_t *value)
{
  return ft6x06_read_reg(ctx, FT6X06_FIRMID_REG, (uint8_t *)value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_chip_id
* Description    : FocalTech's Panel ID
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_CHIP_ID register
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_chip_id(ft6x06_ctx_t *ctx, uint8_t *value)
{
  int32_t ret;
  
  ret = ft6x06_read_reg(ctx, FT6X06_CHIP_ID_REG, (uint8_t *)value, 1);
  
  if(ret == 0)
  {
    *value &= FT6X06_CHIP_ID_BIT_MASK; 
    *value = *value >> FT6X06_CHIP_ID_BIT_POSITION; 
  }
  
  return ret;
}

/*******************************************************************************
* Function Name  : release_code_id
* Description    : Release code version
* Input          : Pointer to uint8_t
* Output         : Status of FT6X06_RELEASE_CODE_ID register
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_release_code_id(ft6x06_ctx_t *ctx, uint8_t *value)
{
  return ft6x06_read_reg(ctx, FT6X06_RELEASE_CODE_ID_REG, (uint8_t *)value, 1);
}

/*******************************************************************************
* Function Name  : ft6x06_state
* Description    : Select Current Operating mode
* Input          : uint8_t
* Output         : None
* Return         : Status [FT6X06_ERROR, FT6X06_OK]
*******************************************************************************/
int32_t  ft6x06_state(ft6x06_ctx_t *ctx, uint8_t value)
{
  return ft6x06_write_reg(ctx, FT6X06_STATE_REG, &value, 1);
}

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
