/**
  ******************************************************************************
  * @file    ft6x06_reg.h
  * @author  MCD Application Team
  * @brief   Header of ft6x06_reg.c
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
#ifndef FT6X06_REG_H
#define FT6X06_REG_H

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

/** @addtogroup FT6X06
 * @{
 */

/* Exported types ------------------------------------------------------------*/

/** @defgroup FT6X06_Exported_Types FT6X06 Exported Types
 * @{
 */
/************** Generic Function  *******************/

typedef int32_t (*FT6X06_Write_Func)(void *, uint8_t, uint8_t*, uint16_t);
typedef int32_t (*FT6X06_Read_Func) (void *, uint8_t, uint8_t*, uint16_t);

/**
 * @}
 */

/** @defgroup FT6X06_Imported_Globals FT6X06 Imported Globals
 * @{
 */
typedef struct
{
  FT6X06_Write_Func   WriteReg;
  FT6X06_Read_Func    ReadReg;
  void                 *handle;
} ft6x06_ctx_t;
/**
 * @}
 */

/** @defgroup FT6X06_Exported_Constants FT6X06 Exported Constants
 * @{
 */

/* Current mode register of the FT6X06 (R/W) */
#define FT6X06_DEV_MODE_REG         0x00U

/* Gesture ID register */
#define FT6X06_GEST_ID_REG          0x01U

/* Touch Data Status register : gives number of active touch points (0..2) */
#define FT6X06_TD_STAT_REG          0x02U

/* P1 X, Y coordinates, weight and misc registers */
#define FT6X06_P1_XH_REG            0x03U
#define FT6X06_P1_XL_REG            0x04U
#define FT6X06_P1_YH_REG            0x05U
#define FT6X06_P1_YL_REG            0x06U
#define FT6X06_P1_WEIGHT_REG        0x07U
#define FT6X06_P1_MISC_REG          0x08U

/* P2 X, Y coordinates, weight and misc registers */
#define FT6X06_P2_XH_REG            0x09U
#define FT6X06_P2_XL_REG            0x0AU
#define FT6X06_P2_YH_REG            0x0BU
#define FT6X06_P2_YL_REG            0x0CU
#define FT6X06_P2_WEIGHT_REG        0x0DU
#define FT6X06_P2_MISC_REG          0x0EU

/* Threshold for touch detection */
#define FT6X06_TH_GROUP_REG         0x80U

/* Filter function coefficients */
#define FT6X06_TH_DIFF_REG          0x85U

/* Control register */
#define FT6X06_CTRL_REG             0x86U

/* The time period of switching from Active mode to Monitor mode when there is no touching */
#define FT6X06_TIMEENTERMONITOR_REG 0x87U

/* Report rate in Active mode */
#define FT6X06_PERIODACTIVE_REG     0x88U

/* Report rate in Monitor mode */
#define FT6X06_PERIODMONITOR_REG    0x89U

/* The value of the minimum allowed angle while Rotating gesture mode */
#define FT6X06_RADIAN_VALUE_REG     0x91U

/* Maximum offset while Moving Left and Moving Right gesture */
#define FT6X06_OFFSET_LR_REG        0x92U

/* Maximum offset while Moving Up and Moving Down gesture */
#define FT6X06_OFFSET_UD_REG        0x93U

/* Minimum distance while Moving Left and Moving Right gesture */
#define FT6X06_DISTANCE_LR_REG      0x94U

/* Minimum distance while Moving Up and Moving Down gesture */
#define FT6X06_DISTANCE_UD_REG      0x95U

/* Maximum distance while Zoom In and Zoom Out gesture */
#define FT6X06_DISTANCE_ZOOM_REG    0x96U

/* High 8-bit of LIB Version info */
#define FT6X06_LIB_VER_H_REG        0xA1U

/* Low 8-bit of LIB Version info */
#define FT6X06_LIB_VER_L_REG        0xA2U

/* Chip Selecting */
#define FT6X06_CIPHER_REG           0xA3U

/* Interrupt mode register (used when in interrupt mode) */
#define FT6X06_GMODE_REG            0xA4U

/* Current power mode the FT6X06 system is in (R) */
#define FT6X06_PWR_MODE_REG         0xA5U

/* FT6X06 firmware version */
#define FT6X06_FIRMID_REG           0xA6U

/* FT6X06 Chip identification register */
#define FT6X06_CHIP_ID_REG          0xA8U

/* Release code version */
#define FT6X06_RELEASE_CODE_ID_REG  0xAFU

/* Current operating mode the FT6X06 system is in (R) */
#define FT6X06_STATE_REG            0xBCU

/**
 * @}
 */

/*******************************************************************************
* Register      : Generic - All
* Address       : Generic - All
* Bit Group Name: None
* Permission    : W
*******************************************************************************/
int32_t ft6x06_write_reg(ft6x06_ctx_t *ctx, uint8_t reg, uint8_t *pbuf, uint16_t length);
int32_t ft6x06_read_reg(ft6x06_ctx_t *ctx, uint8_t reg, uint8_t *pbuf, uint16_t length);

/**************** Base Function  *******************/

/*******************************************************************************
* Register      : DEV_MODE
* Address       : 0X00
* Bit Group Name: DEVICE_MODE
* Permission    : RW
*******************************************************************************/
#define   FT6X06_DEV_MODE_BIT_MASK        0x70U
#define   FT6X06_DEV_MODE_BIT_POSITION    4U
int32_t  ft6x06_dev_mode_w(ft6x06_ctx_t *ctx, uint8_t value);
int32_t  ft6x06_dev_mode_r(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : GEST_ID
* Address       : 0X01
* Bit Group Name: Gesture ID
* Permission    : R
*******************************************************************************/
#define   FT6X06_GEST_ID_BIT_MASK        0xFFU
#define   FT6X06_GEST_ID_BIT_POSITION    0U
int32_t ft6x06_gest_id(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : TD_STATUS
* Address       : 0X02
* Bit Group Name: 
* Permission    : R
*******************************************************************************/
#define   FT6X06_TD_STATUS_BIT_MASK        0x0FU
#define   FT6X06_TD_STATUS_BIT_POSITION    0U
int32_t ft6x06_td_status(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P1_XH
* Address       : 0X03
* Bit Group Name: First Event Flag
* Permission    : R
* Default value : 0xF0U
*******************************************************************************/
#define   FT6X06_P1_XH_EF_BIT_MASK        0xC0U
#define   FT6X06_P1_XH_EF_BIT_POSITION    6U
int32_t ft6x06_p1_xh_ef(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P1_XH
* Address       : 0X03
* Bit Group Name: First Touch X Position
* Permission    : R
* Default value : 0x0FU
*******************************************************************************/
#define   FT6X06_P1_XH_TP_BIT_MASK        0x0FU
#define   FT6X06_P1_XH_TP_BIT_POSITION    0U
int32_t ft6x06_p1_xh_tp(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P1_XL
* Address       : 0X04
* Bit Group Name: First Touch X Position
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT6X06_P1_XL_TP_BIT_MASK        0xFFU
#define   FT6X06_P1_XL_TP_BIT_POSITION    0U
int32_t ft6x06_p1_xl_tp(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P1_YH
* Address       : 0X05
* Bit Group Name: First Touch ID
* Permission    : R
* Default value : 0xF0U
*******************************************************************************/
#define   FT6X06_P1_YH_TID_BIT_MASK        0xF0U
#define   FT6X06_P1_YH_TID_BIT_POSITION    4U
int32_t ft6x06_p1_yh_tid(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P1_YH
* Address       : 0x06
* Bit Group Name: First Touch Y Position
* Permission    : R
* Default value : 0x0FU
*******************************************************************************/
#define   FT6X06_P1_YH_TP_BIT_MASK        0x0FU
#define   FT6X06_P1_YH_TP_BIT_POSITION    0U
int32_t ft6x06_p1_yh_tp(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P1_YL
* Address       : 0X06
* Bit Group Name: First Touch Y Position
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT6X06_P1_YL_TP_BIT_MASK        0xFFU
#define   FT6X06_P1_YL_TP_BIT_POSITION    0U
int32_t ft6x06_p1_yl_tp(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P1_WEIGHT
* Address       : 0X07
* Bit Group Name: First Touch Weight(pressure)
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT6X06_P1_WEIGHT_BIT_MASK        0xFFU
#define   FT6X06_P1_WEIGHT_BIT_POSITION    0U
int32_t ft6x06_p1_weight(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P1_MISC
* Address       : 0X08
* Bit Group Name: First Touch Area
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT6X06_P1_MISC_BIT_MASK        0xF0U
#define   FT6X06_P1_MISC_BIT_POSITION    4U
int32_t ft6x06_p1_misc(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P2_XH
* Address       : 0X09
* Bit Group Name: Second Event Flag
* Permission    : R
* Default value : 0xF0U
*******************************************************************************/
#define   FT6X06_P2_XH_EF_BIT_MASK        0xC0U
#define   FT6X06_P2_XH_EF_BIT_POSITION    6U
int32_t ft6x06_p2_xh_ef(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P2_XH
* Address       : 0X09
* Bit Group Name: Second Touch X Position
* Permission    : R
* Default value : 0x0FU
*******************************************************************************/
#define   FT6X06_P2_XH_TP_BIT_MASK        0x0FU
#define   FT6X06_P2_XH_TP_BIT_POSITION    0U
int32_t ft6x06_p2_xh_tp(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P2_XL
* Address       : 0X0A
* Bit Group Name: Second Touch X Position
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT6X06_P2_XL_TP_BIT_MASK        0xFFU
#define   FT6X06_P2_XL_TP_BIT_POSITION    0U
int32_t ft6x06_p2_xl_tp(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P2_YH
* Address       : 0X0B
* Bit Group Name: Second Touch ID
* Permission    : R
* Default value : 0xF0U
*******************************************************************************/
#define   FT6X06_P2_YH_TID_BIT_MASK        0xF0U
#define   FT6X06_P2_YH_TID_BIT_POSITION    4U
int32_t ft6x06_p2_yh_tid(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P2_YH
* Address       : 0x0B
* Bit Group Name: Second Touch Y Position
* Permission    : R
* Default value : 0x0FU
*******************************************************************************/
#define   FT6X06_P2_YH_TP_BIT_MASK        0x0FU
#define   FT6X06_P2_YH_TP_BIT_POSITION    0U
int32_t ft6x06_p2_yh_tp(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P2_YL
* Address       : 0X0C
* Bit Group Name: Second Touch Y Position
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT6X06_P2_YL_TP_BIT_MASK        0xFFU
#define   FT6X06_P2_YL_TP_BIT_POSITION    0U
int32_t ft6x06_p2_yl_tp(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P2_WEIGHT
* Address       : 0X0D
* Bit Group Name: Second Touch Weight(pressure)
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT6X06_P2_WEIGHT_BIT_MASK        0xFFU
#define   FT6X06_P2_WEIGHT_BIT_POSITION    0U
int32_t ft6x06_p2_weight(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P2_MISC
* Address       : 0X0E
* Bit Group Name: Second Touch Area
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT6X06_P2_MISC_BIT_MASK        0xF0U
#define   FT6X06_P2_MISC_BIT_POSITION    4U
int32_t ft6x06_p2_misc(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : TH_GROUP
* Address       : 0X80
* Bit Group Name: Threshold for touch detection
* Permission    : RW
* Default value : None
*******************************************************************************/
#define   FT6X06_TH_GROUP_BIT_MASK        0xFFU
#define   FT6X06_TH_GROUP_BIT_POSITION    0U
int32_t ft6x06_th_group(ft6x06_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : TH_DIFF
* Address       : 0X85
* Bit Group Name: Filter function coefficient
* Permission    : RW
* Default value : None
*******************************************************************************/
#define   FT6X06_TH_DIFF_BIT_MASK        0xFFU
#define   FT6X06_TH_DIFF_BIT_POSITION    0U
int32_t ft6x06_th_diff(ft6x06_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : CTRL
* Address       : 0X86
* Bit Group Name: 
* Permission    : RW
* Default value : 0x01
*******************************************************************************/
#define   FT6X06_CTRL_BIT_MASK           0xFFU
#define   FT6X06_CTRL_BIT_POSITION       0U
int32_t ft6x06_ctrl(ft6x06_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : TIMEENTERMONITOR
* Address       : 0X87
* Bit Group Name: 
* Permission    : RW
* Default value : 0x0A
*******************************************************************************/
#define   FT6X06_TIMEENTERMONITOR_BIT_MASK           0xFFU
#define   FT6X06_TIMEENTERMONITOR_BIT_POSITION       0U
int32_t ft6x06_time_enter_monitor(ft6x06_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : PERIODACTIVE
* Address       : 0X88
* Bit Group Name: 
* Permission    : RW
* Default value : None
*******************************************************************************/
#define   FT6X06_PERIODACTIVE_BIT_MASK           0xFFU
#define   FT6X06_PERIODACTIVE_BIT_POSITION       0U
int32_t ft6x06_period_active(ft6x06_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : PERIODMONITOR
* Address       : 0X89
* Bit Group Name: 
* Permission    : RW
* Default value : 0x28
*******************************************************************************/
#define   FT6X06_PERIODMONITOR_BIT_MASK           0xFFU
#define   FT6X06_PERIODMONITOR_BIT_POSITION       0U
int32_t ft6x06_period_monitor(ft6x06_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : RADIAN_VALUE
* Address       : 0X91
* Bit Group Name: 
* Permission    : RW
* Default value : 0x0A
*******************************************************************************/
#define   FT6X06_RADIAN_VALUE_BIT_MASK           0xFFU
#define   FT6X06_RADIAN_VALUE_BIT_POSITION       0U
int32_t ft6x06_radian_value(ft6x06_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : OFFSET_LEFT_RIGHT
* Address       : 0X92
* Bit Group Name: 
* Permission    : RW
* Default value : 0x19
*******************************************************************************/
#define   FT6X06_OFFSET_LR_BIT_MASK           0xFFU
#define   FT6X06_OFFSET_LR_BIT_POSITION       0U
int32_t ft6x06_offset_left_right(ft6x06_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : OFFSET_UP_DOWN
* Address       : 0X93
* Bit Group Name: 
* Permission    : RW
* Default value : 0x19
*******************************************************************************/
#define   FT6X06_OFFSET_UD_BIT_MASK           0xFFU
#define   FT6X06_OFFSET_UD_BIT_POSITION       0U
int32_t ft6x06_offset_up_down(ft6x06_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : DISTANCE_LEFT_RIGHT
* Address       : 0X94
* Bit Group Name: 
* Permission    : RW
* Default value : 0x19
*******************************************************************************/
#define   FT6X06_DISTANCE_LR_BIT_MASK           0xFFU
#define   FT6X06_DISTANCE_LR_BIT_POSITION       0U
int32_t  ft6x06_disatnce_left_right(ft6x06_ctx_t *ctx, uint8_t value);
        
/*******************************************************************************
* Register      : DISTANCE_UP_DOWN
* Address       : 0X95
* Bit Group Name: 
* Permission    : RW
* Default value : 0x19
*******************************************************************************/
#define   FT6X06_DISTANCE_UD_BIT_MASK           0xFFU
#define   FT6X06_DISTANCE_UD_BIT_POSITION       0U
int32_t ft6x06_distance_up_down(ft6x06_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : DISTANCE_ZOOM
* Address       : 0X96
* Bit Group Name: 
* Permission    : RW
* Default value : 0x32
*******************************************************************************/
#define   FT6X06_DISTANCE_ZOOM_BIT_MASK           0xFFU
#define   FT6X06_DISTANCE_ZOOM_BIT_POSITION       0U
int32_t ft6x06_distance_zoom(ft6x06_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : LIB_VER_H
* Address       : 0XA1
* Bit Group Name: 
* Permission    : R
* Default value : None
*******************************************************************************/
#define   FT6X06_LIB_VER_H_BIT_MASK           0xFFU
#define   FT6X06_LIB_VER_H_BIT_POSITION       0U
int32_t ft6x06_lib_ver_high(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : LIB_VER_L
* Address       : 0XA2
* Bit Group Name: 
* Permission    : R
* Default value : None
*******************************************************************************/
#define   FT6X06_LIB_VER_L_BIT_MASK           0xFFU
#define   FT6X06_LIB_VER_L_BIT_POSITION       0U
int32_t ft6x06_lib_ver_low(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : CIPHER
* Address       : 0XA3
* Bit Group Name: 
* Permission    : R
* Default value : 0x06
*******************************************************************************/
#define   FT6X06_CIPHER_BIT_MASK           0xFFU
#define   FT6X06_CIPHER_BIT_POSITION       0U
int32_t ft6x06_cipher(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : G_MODE
* Address       : 0XA4
* Bit Group Name: 
* Permission    : RW
* Default value : 0x01
*******************************************************************************/
#define   FT6X06_G_MODE_BIT_MASK           0xFFU
#define   FT6X06_G_MODE_BIT_POSITION       0U
int32_t ft6x06_g_mode(ft6x06_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : PWR_MODE
* Address       : 0XA5
* Bit Group Name: 
* Permission    : RW
* Default value : 0x00
*******************************************************************************/
#define   FT6X06_PWR_MODE_BIT_MASK           0xFFU
#define   FT6X06_PWR_MODE_BIT_POSITION       0U
int32_t ft6x06_pwr_mode(ft6x06_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : FIRMID
* Address       : 0XA6
* Bit Group Name: 
* Permission    : R
* Default value : None
*******************************************************************************/
#define   FT6X06_FIRMID_BIT_MASK           0xFFU
#define   FT6X06_FIRMID_BIT_POSITION       0U
int32_t ft6x06_firm_id(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : FOCALTECH_ID
* Address       : 0XA8
* Bit Group Name: 
* Permission    : R
* Default value : 0x11
*******************************************************************************/
#define   FT6X06_CHIP_ID_BIT_MASK           0xFFU
#define   FT6X06_CHIP_ID_BIT_POSITION       0U
int32_t ft6x06_chip_id(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : RELEASE_CODE_ID
* Address       : 0XAF
* Bit Group Name: 
* Permission    : R
* Default value : 0x001
*******************************************************************************/
#define   FT6X06_RC_ID_BIT_MASK           0xFFU
#define   FT6X06_RC_ID_BIT_POSITION       0U
int32_t ft6x06_release_code_id(ft6x06_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : STATE
* Address       : 0XBC
* Bit Group Name: 
* Permission    : RW
* Default value : 0x01
*******************************************************************************/
#define   FT6X06_STATE_BIT_MASK           0xFFU
#define   FT6X06_STATE_BIT_POSITION       0U
int32_t ft6x06_state(ft6x06_ctx_t *ctx, uint8_t value);

  /**
   * @}
   */

#ifdef __cplusplus
}
#endif
#endif /* FT6X06_REG_H */


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
