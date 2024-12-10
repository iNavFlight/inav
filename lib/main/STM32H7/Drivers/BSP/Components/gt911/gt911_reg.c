/**
  ******************************************************************************
  * @file    gt911_reg.c
  * @author  MCD Application Team
  * @brief   This file provides unitary register function to control the gt911 Touch
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gt911_reg.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @defgroup GT911 GT911
  * @{
  */
/**
  * @brief  Read GT911 registers.
  * @param  pCtx Pointer to component context
  * @param  reg Component reg to read from
  * @param  pData pointer to data to be read
  * @param  length Length of data to read
  * @retval Component status
  */
int32_t gt911_read_reg(gt911_ctx_t *pCtx, uint16_t reg, uint8_t *pData, uint16_t length)
{
  return pCtx->ReadReg(pCtx->handle, reg, pData, length);
}

/**
  * @brief  Write GT911 registers.
  * @param  pCtx Pointer to component context
  * @param  reg Component reg to write to
  * @param  pData pointer to data to be written
  * @param  length Length of data to write
  * @retval Component status
  */
int32_t gt911_write_reg(gt911_ctx_t *pCtx, uint16_t reg, uint8_t *pData, uint16_t length)
{
  int32_t ret = GT911_REG_OK;
  uint16_t ii;
  uint8_t calc_chksum = 0U;
  uint8_t r_data;
  uint8_t value = 1U;

  if (pCtx->WriteReg(pCtx->handle, reg, pData, length) != 0U)
  {
    ret = GT911_REG_ERROR;
  }

  if ((reg >= GT911_CONFIG_VERS_REG) && (reg < GT911_CONFIG_CHKSUM_REG))
  {
    for (ii = GT911_CONFIG_VERS_REG; ii < GT911_CONFIG_CHKSUM_REG; ii++)
    {
      if (pCtx->ReadReg(pCtx->handle, ii, &r_data, 1U) != 0U)
      {
        ret = GT911_REG_ERROR;
      }
      else
      {
        calc_chksum += r_data;
      }
    }

    if (ret == GT911_REG_OK)
    {
      calc_chksum = ((~calc_chksum) + 1U) & 0xffU;
      if (pCtx->ReadReg(pCtx->handle, GT911_CONFIG_CHKSUM_REG, &r_data, 1U) != 0U)
      {
        ret = GT911_REG_ERROR;  /* I2C reading failed */
      }
      else
      {
        if (calc_chksum != r_data)
        {
          if (pCtx->WriteReg(pCtx->handle, GT911_CONFIG_CHKSUM_REG, &calc_chksum, 1U) != 0U) /* Update checksum */
          {
            ret = GT911_REG_ERROR;
          }
          else
          {
            if (pCtx->WriteReg(pCtx->handle, GT911_CONFIG_FRESH_REG, &value, 1U) != 0U) /* Update Config Fresh */
            {
              ret = GT911_REG_ERROR;
            }
          }
        }
        else
        {
          /* Do nothing */
        }
      }
    }
  }

  return ret;
}

/**
  * @brief  Set GT911 working mode
  * @param  pCtx Pointer to component context
  * @param  value Value to write to GT911_DEV_MODE_REG register
  * @retval Component status
  */
int32_t  gt911_dev_mode_w(gt911_ctx_t *pCtx, uint8_t value)
{
  int32_t ret;
  uint8_t tmp;

  ret = gt911_read_reg(pCtx, GT911_DEV_MODE_REG, &tmp, 1U);

  if (ret == 0L)
  {
    tmp &= ~GT911_DEV_MODE_BIT_MASK;
    tmp |= value << GT911_DEV_MODE_BIT_POSITION;

    ret = gt911_write_reg(pCtx, GT911_DEV_MODE_REG, &tmp, 1U);
  }

  return ret;
}

/**
  * @brief  Get GT911 working mode
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_DEV_MODE_REG register
  * @retval Component status
  */
int32_t  gt911_dev_mode_r(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_DEV_MODE_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_DEV_MODE_BIT_MASK;
    *pValue = *pValue >> GT911_DEV_MODE_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 gesture ID
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_GEST_ID_REG register
  * @retval Component status
  */
int32_t  gt911_gest_id(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_GEST_ID_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Get GT911 Touch Data Status
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of Touch Data Status register
  * @retval Component status
  */
int32_t  gt911_td_status(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;
  do
  {
    ret = gt911_read_reg(pCtx, GT911_TD_STAT_REG, (uint8_t *)pValue, 1U);
    if (ret == 0L)
    {
      /* If the BUFFER_STATUS bit[7] is only set, write 0 on the register */
      if (*pValue == GT911_TD_STATUS_BIT_BUFFER_STAT)
      {
        *pValue = 0U;
        ret = gt911_write_reg(pCtx, GT911_TD_STAT_REG, (uint8_t *)pValue, 1U);
      }
      else
      {
        *pValue &= GT911_TD_STATUS_BITS_NBTOUCHPTS;
        break;
      }
    }
  } while (ret == 0L);
  return ret;
}

/**
  * @brief  Get GT911 Clear Interrupt/Buffer Status
  * @param  pCtx Pointer to component context
  * @retval Component status
  */
int32_t  gt911_clr_int(gt911_ctx_t *pCtx)
{
  int32_t ret;
  uint8_t tmp;

  tmp = 0U;
  ret = gt911_write_reg(pCtx, GT911_TD_STAT_REG, &tmp, 1U);

  return ret;
}

/**
  * @brief  Get GT911 First X Event Flag
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P1_XH_REG register
  * @retval Component status
  */
int32_t  gt911_p1_xh_ef(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P1_XH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P1_XH_EF_BIT_MASK;
    *pValue = *pValue >> GT911_P1_XH_EF_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 First X Low Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P1_XL_REG register
  * @retval Component status
  */
int32_t  gt911_p1_xl_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_P1_XL_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Get GT911 First X High Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P1_XH_REG register
  * @retval Component status
  */
int32_t  gt911_p1_xh_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P1_XH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P1_XH_TP_BIT_MASK;
    *pValue = *pValue >> GT911_P1_XH_TP_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 First Y Low Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P1_YL_REG register
  * @retval Component status
  */
int32_t  gt911_p1_yl_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_P1_YL_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Get GT911 First Y High Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P1_YH_REG register
  * @retval Component status
  */
int32_t  gt911_p1_yh_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P1_YH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P1_YH_TP_BIT_MASK;
    *pValue = *pValue >> GT911_P1_YH_TP_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 First Touch pressure
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P1_WEIGHTL_REG register
  * @retval Component status
  */
int32_t  gt911_p1_weight(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_P1_WEIGHTL_REG, (uint8_t *)pValue, 2U);
}

/**
  * @brief  Get GT911 First track ID
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P1_TID_REG register
  * @retval Component status
  */
int32_t  gt911_p1_tid(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P1_TID_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P1_TID_BIT_MASK;
    *pValue = *pValue >> GT911_P1_TID_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Second X Event Flag
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P2_XH_REG register
  * @retval Component status
  */
int32_t  gt911_p2_xh_ef(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P2_XH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P2_XH_EF_BIT_MASK;
    *pValue = *pValue >> GT911_P2_XH_EF_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Second X High Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P2_XH_REG register
  * @retval Component status
  */
int32_t  gt911_p2_xh_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P2_XH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P2_XH_TP_BIT_MASK;
    *pValue = *pValue >> GT911_P2_XH_TP_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Second X Low Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P2_XL_REG register
  * @retval Component status
  */
int32_t  gt911_p2_xl_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_P2_XL_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Get GT911 Second Touch ID
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P2_XH_REG register
  * @retval Component status
  */
int32_t  gt911_p2_yh_tid(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P2_YH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P2_YH_TID_BIT_MASK;
    *pValue = *pValue >> GT911_P2_YH_TID_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Second Y High Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P2_YH_REG register
  * @retval Component status
  */
int32_t  gt911_p2_yh_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P2_YH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P2_YH_TP_BIT_MASK;
    *pValue = *pValue >> GT911_P2_YH_TP_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Second Y Low Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P2_YL_REG register
  * @retval Component status
  */
int32_t  gt911_p2_yl_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_P2_YL_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Get GT911 Second Touch pressure
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P2_WEIGHT_REG register
  * @retval Component status
  */
int32_t  gt911_p2_weight(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_P2_WEIGHTL_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Get GT911 Second Touch area
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P2_TID_REG register
  * @retval Component status
  */
int32_t  gt911_p2_tid(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P2_TID_REG, (uint8_t *)pValue, 1U);
  if (ret == 0L)
  {
    *pValue &= GT911_P2_TID_BIT_MASK;
    *pValue = *pValue >> GT911_P2_TID_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Third X Event Flag
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P3_XH_REG register
  * @retval Component status
  */
int32_t  gt911_p3_xh_ef(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P3_XH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P3_XH_EF_BIT_MASK;
    *pValue = *pValue >> GT911_P3_XH_EF_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Third X High Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P3_XH_REG register
  * @retval Component status
  */
int32_t  gt911_p3_xh_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P3_XH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P3_XH_TP_BIT_MASK;
    *pValue = *pValue >> GT911_P3_XH_TP_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Third X Low Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P3_XL_REG register
  * @retval Component status
  */
int32_t  gt911_p3_xl_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_P3_XL_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Get GT911 Third Touch ID
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P3_XH_REG register
  * @retval Component status
  */
int32_t  gt911_p3_yh_tid(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P3_YH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P3_YH_TID_BIT_MASK;
    *pValue = *pValue >> GT911_P3_YH_TID_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Third Y High Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P3_YH_REG register
  * @retval Component status
  */
int32_t  gt911_p3_yh_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P3_YH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P3_YH_TP_BIT_MASK;
    *pValue = *pValue >> GT911_P3_YH_TP_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Third Y Low Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P3_YL_REG register
  * @retval Component status
  */
int32_t  gt911_p3_yl_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_P3_YL_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Get GT911 Third Touch pressure
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P3_WEIGHT_REG register
  * @retval Component status
  */
int32_t  gt911_p3_weight(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_P3_WEIGHTL_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Get GT911 Third Touch area
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P3_TID_REG register
  * @retval Component status
  */
int32_t  gt911_p3_tid(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P3_TID_REG, (uint8_t *)pValue, 1U);
  if (ret == 0L)
  {
    *pValue &= GT911_P3_TID_BIT_MASK;
    *pValue = *pValue >> GT911_P3_TID_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Fourth X Event Flag
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P4_XH_REG register
  * @retval Component status
  */
int32_t  gt911_p4_xh_ef(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P4_XH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P4_XH_EF_BIT_MASK;
    *pValue = *pValue >> GT911_P4_XH_EF_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Fourth X High Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P4_XH_REG register
  * @retval Component status
  */
int32_t  gt911_p4_xh_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P4_XH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P4_XH_TP_BIT_MASK;
    *pValue = *pValue >> GT911_P4_XH_TP_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Fourth X Low Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P4_XL_REG register
  * @retval Component status
  */
int32_t  gt911_p4_xl_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_P4_XL_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Get GT911 Fourth Touch ID
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P4_XH_REG register
  * @retval Component status
  */
int32_t  gt911_p4_yh_tid(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P4_YH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P4_YH_TID_BIT_MASK;
    *pValue = *pValue >> GT911_P4_YH_TID_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Fourth Y High Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P4_YH_REG register
  * @retval Component status
  */
int32_t  gt911_p4_yh_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P4_YH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P4_YH_TP_BIT_MASK;
    *pValue = *pValue >> GT911_P4_YH_TP_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Fourth Y Low Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P4_YL_REG register
  * @retval Component status
  */
int32_t  gt911_p4_yl_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_P4_YL_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Get GT911 Fourth Touch pressure
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P4_WEIGHT_REG register
  * @retval Component status
  */
int32_t  gt911_p4_weight(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_P4_WEIGHTL_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Get GT911 Fourth Touch area
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P4_TID_REG register
  * @retval Component status
  */
int32_t  gt911_p4_tid(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P4_TID_REG, (uint8_t *)pValue, 1U);
  if (ret == 0L)
  {
    *pValue &= GT911_P4_TID_BIT_MASK;
    *pValue = *pValue >> GT911_P4_TID_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Fifth X Event Flag
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P5_XH_REG register
  * @retval Component status
  */
int32_t  gt911_p5_xh_ef(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P5_XH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P5_XH_EF_BIT_MASK;
    *pValue = *pValue >> GT911_P5_XH_EF_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Fifth X High Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P5_XH_REG register
  * @retval Component status
  */
int32_t  gt911_p5_xh_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P5_XH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P5_XH_TP_BIT_MASK;
    *pValue = *pValue >> GT911_P5_XH_TP_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Fifth X Low Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P5_XL_REG register
  * @retval Component status
  */
int32_t  gt911_p5_xl_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_P5_XL_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Get GT911 Fifth Touch ID
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P5_XH_REG register
  * @retval Component status
  */
int32_t  gt911_p5_yh_tid(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P5_YH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P5_YH_TID_BIT_MASK;
    *pValue = *pValue >> GT911_P5_YH_TID_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Fifth Y High Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P5_YH_REG register
  * @retval Component status
  */
int32_t  gt911_p5_yh_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P5_YH_REG, (uint8_t *)pValue, 1U);

  if (ret == 0L)
  {
    *pValue &= GT911_P5_YH_TP_BIT_MASK;
    *pValue = *pValue >> GT911_P5_YH_TP_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Get GT911 Fifth Y Low Touch Position
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P5_YL_REG register
  * @retval Component status
  */
int32_t  gt911_p5_yl_tp(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_P5_YL_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Get GT911 Fifth Touch pressure
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P5_WEIGHT_REG register
  * @retval Component status
  */
int32_t  gt911_p5_weight(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_P5_WEIGHTL_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Get GT911 Fifth Touch area
  * @param  pCtx Pointer to component context
  * @param  pValue pointer to the pValue of GT911_P5_TID_REG register
  * @retval Component status
  */
int32_t  gt911_p5_tid(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  int32_t ret;

  ret = gt911_read_reg(pCtx, GT911_P5_TID_REG, (uint8_t *)pValue, 1U);
  if (ret == 0L)
  {
    *pValue &= GT911_P5_TID_BIT_MASK;
    *pValue = *pValue >> GT911_P5_TID_BIT_POSITION;
  }

  return ret;
}

/**
  * @brief  Set GT911 Threshold for touch detection
  * @param  pCtx Pointer to component context
  * @param  value Value to write to GT911_TH_GROUP_REG register
  * @retval Component status
  */
int32_t  gt911_th_group(gt911_ctx_t *pCtx, uint8_t value)
{
  return gt911_write_reg(pCtx, GT911_TH_GROUP_REG, &value, 1U);
}

/**
  * @brief  Set GT911 Filter function coefficient
  * @param  pCtx Pointer to component context
  * @param  value Value to write to GT911_TH_DIFF_REG register
  * @retval Component status
  */
int32_t  gt911_th_diff(gt911_ctx_t *pCtx, uint8_t value)
{
  return gt911_write_reg(pCtx, GT911_TH_DIFF_REG, &value, 1U);
}

/**
  * @brief  Control the Switch between Active and Monitoring Mode
  * @param  pCtx Pointer to component context
  * @param  value Value to write to GT911_CTRL_REG register
  * @retval Component status
  */
int32_t  gt911_ctrl(gt911_ctx_t *pCtx, uint8_t value)
{
  return gt911_write_reg(pCtx, GT911_CTRL_REG, &value, 1U);
}

/**
  * @brief  Set the time period of switching from Active mode to Monitor
  * @param  pCtx Pointer to component context
  * @param  value Value to write to GT911_TIMEENTERMONITOR_REG register
  * @retval Component status
  */
int32_t  gt911_time_enter_monitor(gt911_ctx_t *pCtx, uint8_t value)
{
  return gt911_write_reg(pCtx, GT911_TIMEENTERMONITOR_REG, &value, 1U);
}

/**
  * @brief  Set rate in Active mode
  * @param  pCtx Pointer to component context
  * @param  value Value to write to GT911_PERIODACTIVE_REG register
  * @retval Component status
  */
int32_t  gt911_period_active(gt911_ctx_t *pCtx, uint8_t value)
{
  return gt911_write_reg(pCtx, GT911_PERIODACTIVE_REG, &value, 1U);
}

/**
  * @brief  Set rate in Monitor mode
  * @param  pCtx Pointer to component context
  * @param  value Value to write to GT911_PERIODMONITOR_REG register
  * @retval Component status
  */
int32_t  gt911_period_monitor(gt911_ctx_t *pCtx, uint8_t value)
{
  return gt911_write_reg(pCtx, GT911_PERIODMONITOR_REG, &value, 1U);
}

/**
  * @brief  Set Minimum distance while Moving Left and Moving Right gesture
  * @param  pCtx Pointer to component context
  * @param  value Value to write to GT911_DIS_GESTURE_REG register
  * @retval Component status
  */
int32_t  gt911_distance_left_right(gt911_ctx_t *pCtx, uint8_t value)
{
  int32_t ret = GT911_REG_OK;
  uint8_t r_data;
  uint8_t tmp_value;

  if (gt911_read_reg(pCtx, GT911_DIS_GESTURE_REG, &r_data, 1U) != GT911_REG_OK)
  {
    ret = GT911_REG_ERROR;
  }
  else
  {
    tmp_value = ((r_data & GT911_DISTANCE_UD_BIT_MASK) | (value & GT911_DISTANCE_LR_BIT_MASK));

    if (gt911_write_reg(pCtx, GT911_DIS_GESTURE_REG, &tmp_value, 1U) != GT911_REG_OK)
    {
      ret = GT911_REG_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Set Minimum distance while Moving Up and Moving Down gesture
  * @param  pCtx Pointer to component context
  * @param  value Value to write to GT911_DIS_GESTURE_REG register
  * @retval Component status
  */
int32_t  gt911_distance_up_down(gt911_ctx_t *pCtx, uint8_t value)
{
  int32_t ret = GT911_REG_OK;
  uint8_t r_data;
  uint8_t tmp_value;

  if (gt911_read_reg(pCtx, GT911_DIS_GESTURE_REG, &r_data, 1U) != GT911_REG_OK)
  {
    ret = GT911_REG_ERROR;
  }
  else
  {
    tmp_value = ((value & GT911_DISTANCE_UD_BIT_MASK) | (r_data & GT911_DISTANCE_LR_BIT_MASK));

    if (gt911_write_reg(pCtx, GT911_DIS_GESTURE_REG, &tmp_value, 1U) != GT911_REG_OK)
    {
      ret = GT911_REG_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Read High 8-bit of LIB Version info
  * @param  pCtx Pointer to component context
  * @param  pValue Pointer to GT911_LIB_VER_H_REG register pValue
  * @retval Component status
  */
int32_t  gt911_lib_ver_high(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_LIB_VER_H_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Read Low 8-bit of LIB Version info
  * @param  pCtx Pointer to component context
  * @param  pValue Pointer to GT911_LIB_VER_L_REG register pValue
  * @retval Component status
  */
int32_t  gt911_lib_ver_low(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_LIB_VER_L_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Read status of cipher register
  * @param  pCtx Pointer to component context
  * @param  pValue Pointer to GT911_CIPHER_REG register pValue
  * @retval Component status
  */
int32_t  gt911_cipher(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_CIPHER_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Select Interrupt (polling or trigger) mode
  * @param  pCtx Pointer to component context
  * @param  value Value to write to GT911_MSW1_REG register
  * @retval Component status
  */
int32_t  gt911_m_sw1(gt911_ctx_t *pCtx, uint8_t value)
{
  return gt911_write_reg(pCtx, GT911_MSW1_REG, &value, 1U);
}

/**
  * @brief  Select Current power mode
  * @param  pCtx Pointer to component context
  * @param  value Value to write to GT911_PWR_MODE_REG register
  * @retval Component status
  */
int32_t  gt911_pwr_mode(gt911_ctx_t *pCtx, uint8_t value)
{
  return gt911_write_reg(pCtx, GT911_PWR_MODE_REG, &value, 1U);
}

/**
  * @brief  Read the Firmware Version
  * @param  pCtx Pointer to component context
  * @param  pValue Pointer to GT911_FIRMID_REG register pValue
  * @retval Component status
  */
int32_t  gt911_firm_id(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  /* GT911 Firmware version: low byte, high byte */
  return gt911_read_reg(pCtx, GT911_FIRMID_REG, (uint8_t *)pValue, 2U);
}

/**
  * @brief  Read the Goodix's GT911 ID (4-byte)
  * @param  pCtx Pointer to component context
  * @param  pValue Pointer to GT911_CHIP_ID_REG register pValue
  * @retval Component status
  */
int32_t  gt911_chip_id(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  /* 4-byte Product ID (ASCII, "911"0x00) */
  return gt911_read_reg(pCtx, GT911_CHIP_ID_REG, (uint8_t *)pValue, 4U);
}

/**
  * @brief  Read the Release code version
  * @param  pCtx Pointer to component context
  * @param  pValue Pointer to GT911_RELEASE_CODE_ID_REG register pValue
  * @retval Component status
  */
int32_t  gt911_release_code_id(gt911_ctx_t *pCtx, uint8_t *pValue)
{
  return gt911_read_reg(pCtx, GT911_RELEASE_CODE_ID_REG, (uint8_t *)pValue, 1U);
}

/**
  * @brief  Select Current Operating mode
  * @param  pCtx Pointer to component context
  * @param  value Value to write to GT911_COMMAND_REG register
  * @retval Component status
  */
int32_t  gt911_mode(gt911_ctx_t *pCtx, uint8_t value)
{
  return gt911_write_reg(pCtx, GT911_COMMAND_REG, &value, 1U);
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
