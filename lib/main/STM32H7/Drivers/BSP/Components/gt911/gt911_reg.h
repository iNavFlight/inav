/**
  ******************************************************************************
  * @file    gt911_reg.h
  * @author  MCD Application Team
  * @brief   Header of gt911_reg.c
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef GT911_REG_H
#define GT911_REG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GT911_REG_OK                      (0)
#define GT911_REG_ERROR                   (-1)

/* Includes ------------------------------------------------------------------*/
/* Macros --------------------------------------------------------------------*/

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @addtogroup GT911
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** @defgroup GT911_Exported_Types GT911 Exported Types
  * @{
  */
/************** Generic Function  *******************/

typedef int32_t (*GT911_Write_Func)(void *, uint16_t, uint8_t *, uint16_t);
typedef int32_t (*GT911_Read_Func)(void *, uint16_t, uint8_t *, uint16_t);

/**
  * @}
  */

/** @defgroup GT911_Imported_Globals GT911 Imported Globals
  * @{
  */
typedef struct
{
  GT911_Write_Func   WriteReg;
  GT911_Read_Func    ReadReg;
  void                *handle;
} gt911_ctx_t;
/**
  * @}
  */

/** @defgroup GT911_Exported_Constants GT911 Exported Constants
  * @{
  */

/* Current mode register of the GT911 (R/W) */
#define GT911_DEV_MODE_REG         0x00U

/* Gesture ID register */
#define GT911_GEST_ID_REG          0x814BU

/* Gesture mode enabled */
#define GT911_GESTURE_EN           0x8U

/* Gesture Coordinates registers */
#define GT911_START_X_L            0x814DU
#define GT911_START_X_H            0x814EU
#define GT911_START_Y_L            0x814FU
#define GT911_START_Y_H            0x8150U
#define GT911_END_X_L              0x8151U
#define GT911_END_X_H              0x8152U
#define GT911_END_Y_L              0x8153U
#define GT911_END_Y_H              0x8154U
#define GT911_WEIGHT_L             0x8155U
#define GT911_WEIGHT_H             0x8156U
#define GT911_HEIGHT_L             0x8157U
#define GT911_HEIGHT_H             0x8158U


/* Touch Data Status register : gives number of active touch points (0..5) */
#define GT911_TD_STAT_REG          0x814EU

/* P1 X, Y coordinates, weight(Point size) and track-id registers */
#define GT911_P1_XL_REG            0x8150U
#define GT911_P1_XH_REG            0x8151U
#define GT911_P1_YL_REG            0x8152U
#define GT911_P1_YH_REG            0x8153U
#define GT911_P1_WEIGHTL_REG       0x8154U
#define GT911_P1_WEIGHTH_REG       0x8155U
#define GT911_P1_TID_REG           0x8157U

/* P2 X, Y coordinates, weight and tid registers */
#define GT911_P2_XL_REG            0x8158U
#define GT911_P2_XH_REG            0x8159U
#define GT911_P2_YL_REG            0x815AU
#define GT911_P2_YH_REG            0x815BU
#define GT911_P2_WEIGHTL_REG       0x815CU
#define GT911_P2_WEIGHTH_REG       0x815DU
#define GT911_P2_TID_REG           0x815FU

/* P3 X, Y coordinates, weight and tid registers */
#define GT911_P3_XL_REG            0x8160U
#define GT911_P3_XH_REG            0x8161U
#define GT911_P3_YL_REG            0x8162U
#define GT911_P3_YH_REG            0x8163U
#define GT911_P3_WEIGHTL_REG       0x8164U
#define GT911_P3_WEIGHTH_REG       0x8165U
#define GT911_P3_TID_REG           0x8167U

/* P4 X, Y coordinates, weight and tid registers */
#define GT911_P4_XL_REG            0x8168U
#define GT911_P4_XH_REG            0x8169U
#define GT911_P4_YL_REG            0x816AU
#define GT911_P4_YH_REG            0x816BU
#define GT911_P4_WEIGHTL_REG       0x816CU
#define GT911_P4_WEIGHTH_REG       0x816DU
#define GT911_P4_TID_REG           0x816FU

/* P5 X, Y coordinates, weight and tid registers */
#define GT911_P5_XL_REG            0x8170U
#define GT911_P5_XH_REG            0x8171U
#define GT911_P5_YL_REG            0x8172U
#define GT911_P5_YH_REG            0x8173U
#define GT911_P5_WEIGHTL_REG       0x8174U
#define GT911_P5_WEIGHTH_REG       0x8175U
#define GT911_P5_TID_REG           0x8177U

/* Threshold for touch detection */
#define GT911_TH_GROUP_REG         0x80U

/* Filter function coefficients */
#define GT911_TH_DIFF_REG          0x85U

/* Control register */
#define GT911_CTRL_REG             0x86U

/* The time period of switching from Active mode to Monitor mode when there is no touching */
#define GT911_TIMEENTERMONITOR_REG 0x87U

/* Report rate in Active mode */
#define GT911_PERIODACTIVE_REG     0x88U

/* Report rate in Monitor mode */
#define GT911_PERIODMONITOR_REG    0x89U

/* Maximum offset while Moving Left and Moving Right gesture */
#define GT911_OFFSET_LR_REG        0x92U

/* Maximum offset while Moving Up and Moving Down gesture */
#define GT911_OFFSET_UD_REG        0x93U

/* Minimum distance while moving gesture */
#define GT911_DIS_GESTURE_REG      0x8071U

/* High 8-bit of LIB Version info */
#define GT911_LIB_VER_H_REG        0xA1U

/* Low 8-bit of LIB Version info */
#define GT911_LIB_VER_L_REG        0xA2U

/* Chip Selecting */
#define GT911_CIPHER_REG           0xA3U

/* Module_Switch1 register for Interrupt */
#define GT911_MSW1_REG             0x804DU

/* Current power mode the GT911 system is in (R) */
#define GT911_PWR_MODE_REG         0xA5U

/* GT911 firmware version */
#define GT911_FIRMID_REG           0x8144U

/* GT911 Chip identification register */
#define GT911_CHIP_ID_REG          0x8140U

/* Release code version */
#define GT911_RELEASE_CODE_ID_REG  0xAFU

/* Current operating mode the GT911 system is in (R) */
#define GT911_COMMAND_REG          0x8040U

/* Coordinates report rate (= 5+N ms) */
#define GT911_REFRESH_RATE_REG     0x8056U
#define GT911_REFRESH_RATE_MSK     0x0FU

/* Version number configuration */
#define GT911_CONFIG_VERS_REG      0x8047U

/* Checksum configuration register */
#define GT911_CONFIG_CHKSUM_REG    0x80FFU

/* Configuration update flag register */
#define GT911_CONFIG_FRESH_REG     0x8100U

/* Command check register */
#define GT911_COMMAND_CHK_REG      0x8046U

/* Gesture configuration registers */
#define GT911_GESTURE_PRESS_TIME   0x8072U
#define GT911_GESTURE_TIME_ABORT   0x00U

#define GT911_GESTURE_SLOPE_ADJUST 0x8073U
#define GT911_GESTURE_ADJUST_VAL   0x00U

#define GT911_GESTURE_CTRL_REG     0x8074U
#define GT911_GESTURE_INVALID_TIM  0x0FU

#define GT911_GESTURE_SWITCH1_REG  0x8075U
#define GT911_GESTURE_SWITCH2_REG  0x8076U
#define GT911_GESTURE_SWITCH1_VAL  0x00U
#define GT911_GESTURE_SWITCH2_VAL  0x00U

#define GT911_GESTURE_REFRESH_REG  0x8077U

#define GT911_GESTURE_TH_REG       0x8078U


/**
  * @}
  */

/*******************************************************************************
  * Register      : Generic - All
  * Address       : Generic - All
  * Bit Group Name: None
  * Permission    : W
  *******************************************************************************/
int32_t gt911_write_reg(gt911_ctx_t *pCtx, uint16_t reg, uint8_t *pData, uint16_t length);
int32_t gt911_read_reg(gt911_ctx_t *pCtx, uint16_t reg, uint8_t *pData, uint16_t length);
/**************** Base Function  *******************/

/*******************************************************************************
  * Register      : DEV_MODE
  * Address       : 0X00
  * Bit Group Name: DEVICE_MODE
  * Permission    : RW
  *******************************************************************************/
#define   GT911_DEV_MODE_BIT_MASK        0x70U
#define   GT911_DEV_MODE_BIT_POSITION    4U
int32_t  gt911_dev_mode_w(gt911_ctx_t *pCtx, uint8_t value);
int32_t  gt911_dev_mode_r(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : GEST_ID
  * Address       : 0X01
  * Bit Group Name: Gesture ID
  * Permission    : R
  *******************************************************************************/
#define   GT911_GEST_ID_BIT_MASK        0xFFU
#define   GT911_GEST_ID_BIT_POSITION    0U
int32_t gt911_gest_id(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : TD_STATUS
  * Address       : 0x814E
  * Bit Group Name:
  * Permission    : R
  *******************************************************************************/
//JCC adding START
#define   GT911_TD_STATUS_BIT_BUFFER_STAT 0x80U
#define   GT911_TD_STATUS_BIT_HAVEKEY     0x10U
#define   GT911_TD_STATUS_BITS_NBTOUCHPTS 0x0FU
//JCC adding END
#define   GT911_TD_STATUS_BIT_MASK        0x07U
#define   GT911_TD_STATUS_BIT_POSITION    0U
int32_t gt911_td_status(gt911_ctx_t *pCtx, uint8_t *pValue);

int32_t  gt911_clr_int(gt911_ctx_t *pCtx);

/*******************************************************************************
  * Register      : P1_XH
  * Address       : 0X03
  * Bit Group Name: First Event Flag
  * Permission    : R
  * Default value : 0xF0U
  *******************************************************************************/
#define   GT911_P1_XH_EF_BIT_MASK        0xC0U
#define   GT911_P1_XH_EF_BIT_POSITION    6U
int32_t gt911_p1_xh_ef(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P1_XL
  * Address       : 0x8150U
  * Bit Group Name: First Touch X Position
  * Permission    : R
  * Default value : 0x00U
  *******************************************************************************/
#define   GT911_P1_XL_TP_BIT_MASK        0xFFU
#define   GT911_P1_XL_TP_BIT_POSITION    0U
int32_t gt911_p1_xl_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P1_XH
  * Address       : 0x8151U
  * Bit Group Name: First Touch X Position
  * Permission    : R
  * Default value : 0x00U
  *******************************************************************************/
#define   GT911_P1_XH_TP_BIT_MASK        0x0FU
#define   GT911_P1_XH_TP_BIT_POSITION    1U
int32_t gt911_p1_xh_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P1_YL
  * Address       : 0x8152U
  * Bit Group Name: First Touch Y Position
  * Permission    : R
  * Default value : 0x00U
  *******************************************************************************/
#define   GT911_P1_YL_TP_BIT_MASK        0xFFU
#define   GT911_P1_YL_TP_BIT_POSITION    2U
int32_t gt911_p1_yl_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P1_YH
  * Address       : 0x8153U
  * Bit Group Name: First Touch Y Position
  * Permission    : R
  * Default value : 0x00U
  *******************************************************************************/
#define   GT911_P1_YH_TP_BIT_MASK        0x0FU
#define   GT911_P1_YH_TP_BIT_POSITION    3U
int32_t gt911_p1_yh_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P1_WEIGHT
  * Address       : 0x8154U
  * Bit Group Name: First Touch Weight(pressure)
  * Permission    : R
  * Default value : 0xFFFU
  *******************************************************************************/
#define   GT911_P1_WEIGHT_BIT_MASK        0xFFFU
#define   GT911_P1_WEIGHT_BIT_POSITION    4U
int32_t gt911_p1_weight(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P1_TID
  * Address       : 0x8157U
  * Bit Group Name: First Touch track-ID
  * Permission    : R
  * Default value : 0xFFU
  *******************************************************************************/
#define   GT911_P1_TID_BIT_MASK           0xFFU
#define   GT911_P1_TID_BIT_POSITION       7U
int32_t gt911_p1_tid(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P2_XH
  * Address       : 0X09
  * Bit Group Name: Second Event Flag
  * Permission    : R
  * Default value : 0xF0U
  *******************************************************************************/
#define   GT911_P2_XH_EF_BIT_MASK        0xC0U
#define   GT911_P2_XH_EF_BIT_POSITION    6U
int32_t gt911_p2_xh_ef(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P2_XH
  * Address       : 0X09
  * Bit Group Name: Second Touch X Position
  * Permission    : R
  * Default value : 0x0FU
  *******************************************************************************/
#define   GT911_P2_XH_TP_BIT_MASK        0x0FU
#define   GT911_P2_XH_TP_BIT_POSITION    8U
int32_t gt911_p2_xh_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P2_XL
  * Address       : 0X0A
  * Bit Group Name: Second Touch X Position
  * Permission    : R
  * Default value : 0xFFU
  *******************************************************************************/
#define   GT911_P2_XL_TP_BIT_MASK        0xFFU
#define   GT911_P2_XL_TP_BIT_POSITION    9U
int32_t gt911_p2_xl_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P2_YH
  * Address       : 0X0B
  * Bit Group Name: Second Touch ID
  * Permission    : R
  * Default value : 0xF0U
  *******************************************************************************/
#define   GT911_P2_YH_TID_BIT_MASK        0xF0U
#define   GT911_P2_YH_TID_BIT_POSITION    15U
int32_t gt911_p2_yh_tid(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P2_YH
  * Address       : 0x0B
  * Bit Group Name: Second Touch Y Position
  * Permission    : R
  * Default value : 0x0FU
  *******************************************************************************/
#define   GT911_P2_YH_TP_BIT_MASK        0x0FU
#define   GT911_P2_YH_TP_BIT_POSITION    11U
int32_t gt911_p2_yh_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P2_YL
  * Address       : 0X0C
  * Bit Group Name: Second Touch Y Position
  * Permission    : R
  * Default value : 0xFFU
  *******************************************************************************/
#define   GT911_P2_YL_TP_BIT_MASK        0xFFU
#define   GT911_P2_YL_TP_BIT_POSITION    10U
int32_t gt911_p2_yl_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P2_WEIGHT
  * Address       : 0X0D
  * Bit Group Name: Second Touch Weight(pressure)
  * Permission    : R
  * Default value : 0xFFU
  *******************************************************************************/
#define   GT911_P2_WEIGHT_BIT_MASK        0xFFFU
#define   GT911_P2_WEIGHT_BIT_POSITION    12U
int32_t gt911_p2_weight(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P2_TID
  * Address       : 0X0E
  * Bit Group Name: Second Touch Area
  * Permission    : R
  * Default value : 0xFFU
  *******************************************************************************/
#define   GT911_P2_TID_BIT_MASK        0xFFU
#define   GT911_P2_TID_BIT_POSITION    15U
int32_t gt911_p2_tid(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P3_XH
  * Address       : 0X0F
  * Bit Group Name: Third Event Flag
  * Permission    : R
  * Default value : 0xF0U
  *******************************************************************************/
#define   GT911_P3_XH_EF_BIT_MASK        0xC0U
#define   GT911_P3_XH_EF_BIT_POSITION    16U
int32_t gt911_p3_xh_ef(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P3_XH
  * Address       : 0X0F
  * Bit Group Name: Third Touch X High Position
  * Permission    : R
  * Default value : 0x0FU
  *******************************************************************************/
#define   GT911_P3_XH_TP_BIT_MASK        0x0FU
#define   GT911_P3_XH_TP_BIT_POSITION    17U
int32_t gt911_p3_xh_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P3_XL
  * Address       : 0X10
  * Bit Group Name: Third Touch X Low Position
  * Permission    : R
  * Default value : 0xFFU
  *******************************************************************************/
#define   GT911_P3_XL_TP_BIT_MASK        0xFFU
#define   GT911_P3_XL_TP_BIT_POSITION    16U
int32_t gt911_p3_xl_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P3_YH
  * Address       : 0X11
  * Bit Group Name: Third Touch ID
  * Permission    : R
  * Default value : 0xF0U
  *******************************************************************************/
#define   GT911_P3_YH_TID_BIT_MASK        0xF0U
#define   GT911_P3_YH_TID_BIT_POSITION    23U
int32_t gt911_p3_yh_tid(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P3_YH
  * Address       : 0x11
  * Bit Group Name: Third Touch Y High Position
  * Permission    : R
  * Default value : 0x0FU
  *******************************************************************************/
#define   GT911_P3_YH_TP_BIT_MASK        0x0FU
#define   GT911_P3_YH_TP_BIT_POSITION    18U
int32_t gt911_p3_yh_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P3_YL
  * Address       : 0X12
  * Bit Group Name: Third Touch Y Low Position
  * Permission    : R
  * Default value : 0xFFU
  *******************************************************************************/
#define   GT911_P3_YL_TP_BIT_MASK        0xFFU
#define   GT911_P3_YL_TP_BIT_POSITION    17U
int32_t gt911_p3_yl_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P3_WEIGHT
  * Address       : 0X13
  * Bit Group Name: Third Touch Weight(pressure)
  * Permission    : R
  * Default value : 0xFFU
  *******************************************************************************/
#define   GT911_P3_WEIGHT_BIT_MASK        0xFFFU
#define   GT911_P3_WEIGHT_BIT_POSITION    19U
int32_t gt911_p3_weight(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P3_TID
  * Address       : 0X14
  * Bit Group Name: Third Touch Area
  * Permission    : R
  * Default value : 0xFFU
  *******************************************************************************/
#define   GT911_P3_TID_BIT_MASK        0xFFU
#define   GT911_P3_TID_BIT_POSITION    23U
int32_t gt911_p3_tid(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P4_XH
  * Address       : 0X15
  * Bit Group Name: Fourth Event Flag
  * Permission    : R
  * Default value : 0xF0U
  *******************************************************************************/
#define   GT911_P4_XH_EF_BIT_MASK        0xC0U
#define   GT911_P4_XH_EF_BIT_POSITION    25
int32_t gt911_p4_xh_ef(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P4_XH
  * Address       : 0X15
  * Bit Group Name: Fourth Touch X High Position
  * Permission    : R
  * Default value : 0x0FU
  *******************************************************************************/
#define   GT911_P4_XH_TP_BIT_MASK        0x0FU
#define   GT911_P4_XH_TP_BIT_POSITION    25U
int32_t gt911_p4_xh_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P4_XL
  * Address       : 0X16
  * Bit Group Name: Fourth Touch X Low Position
  * Permission    : R
  * Default value : 0xFFU
  *******************************************************************************/
#define   GT911_P4_XL_TP_BIT_MASK        0xFFU
#define   GT911_P4_XL_TP_BIT_POSITION    24U
int32_t gt911_p4_xl_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P4_YH
  * Address       : 0X17
  * Bit Group Name: Fourth Touch ID
  * Permission    : R
  * Default value : 0xF0U
  *******************************************************************************/
#define   GT911_P4_YH_TID_BIT_MASK        0xF0U
#define   GT911_P4_YH_TID_BIT_POSITION    31U
int32_t gt911_p4_yh_tid(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P4_YH
  * Address       : 0x17
  * Bit Group Name: Fourth Touch Y High Position
  * Permission    : R
  * Default value : 0x0FU
  *******************************************************************************/
#define   GT911_P4_YH_TP_BIT_MASK        0x0FU
#define   GT911_P4_YH_TP_BIT_POSITION    27U
int32_t gt911_p4_yh_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P4_YL
  * Address       : 0X18
  * Bit Group Name: Fourth Touch Y Low Position
  * Permission    : R
  * Default value : 0xFFU
  *******************************************************************************/
#define   GT911_P4_YL_TP_BIT_MASK        0xFFU
#define   GT911_P4_YL_TP_BIT_POSITION    26U
int32_t gt911_p4_yl_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P4_WEIGHT
  * Address       : 0X19
  * Bit Group Name: Fourth Touch Weight(pressure)
  * Permission    : R
  * Default value : 0xFFU
  *******************************************************************************/
#define   GT911_P4_WEIGHT_BIT_MASK        0xFFFU
#define   GT911_P4_WEIGHT_BIT_POSITION    28U
int32_t gt911_p4_weight(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P4_TID
  * Address       : 0X1A
  * Bit Group Name: Fourth Touch Area
  * Permission    : R
  * Default value : 0xFFU
  *******************************************************************************/
#define   GT911_P4_TID_BIT_MASK        0xFFU
#define   GT911_P4_TID_BIT_POSITION    31U
int32_t gt911_p4_tid(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P5_XH
  * Address       : 0X1B
  * Bit Group Name: Fifth Event Flag
  * Permission    : R
  * Default value : 0xF0U
  *******************************************************************************/
#define   GT911_P5_XH_EF_BIT_MASK        0xC0U
#define   GT911_P5_XH_EF_BIT_POSITION    6U
int32_t gt911_p5_xh_ef(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P5_XH
  * Address       : 0X1B
  * Bit Group Name: Fifth Touch X High Position
  * Permission    : R
  * Default value : 0x0FU
  *******************************************************************************/
#define   GT911_P5_XH_TP_BIT_MASK        0x0FU
#define   GT911_P5_XH_TP_BIT_POSITION    0U
int32_t gt911_p5_xh_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P5_XL
  * Address       : 0X1C
  * Bit Group Name: Fifth Touch X Low Position
  * Permission    : R
  * Default value : 0xFFU
  *******************************************************************************/
#define   GT911_P5_XL_TP_BIT_MASK        0xFFU
#define   GT911_P5_XL_TP_BIT_POSITION    0U
int32_t gt911_p5_xl_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P5_YH
  * Address       : 0X1D
  * Bit Group Name: Fifth Touch ID
  * Permission    : R
  * Default value : 0xF0U
  *******************************************************************************/
#define   GT911_P5_YH_TID_BIT_MASK        0xF0U
#define   GT911_P5_YH_TID_BIT_POSITION    4U
int32_t gt911_p5_yh_tid(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P5_YH
  * Address       : 0x1D
  * Bit Group Name: Fifth Touch Y High Position
  * Permission    : R
  * Default value : 0x0FU
  *******************************************************************************/
#define   GT911_P5_YH_TP_BIT_MASK        0x0FU
#define   GT911_P5_YH_TP_BIT_POSITION    0U
int32_t gt911_p5_yh_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P5_YL
  * Address       : 0X1E
  * Bit Group Name: Fifth Touch Y Low Position
  * Permission    : R
  * Default value : 0xFFU
  *******************************************************************************/
#define   GT911_P5_YL_TP_BIT_MASK        0xFFU
#define   GT911_P5_YL_TP_BIT_POSITION    0U
int32_t gt911_p5_yl_tp(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P5_WEIGHT
  * Address       : 0X1F
  * Bit Group Name: Fifth Touch Weight(pressure)
  * Permission    : R
  * Default value : 0xFFU
  *******************************************************************************/
#define   GT911_P5_WEIGHT_BIT_MASK        0xFFFU
#define   GT911_P5_WEIGHT_BIT_POSITION    0U
int32_t gt911_p5_weight(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : P5_TID
  * Address       : 0X20
  * Bit Group Name: Fifth Touch Area
  * Permission    : R
  * Default value : 0xFFU
  *******************************************************************************/
#define   GT911_P5_TID_BIT_MASK        0xFFU
#define   GT911_P5_TID_BIT_POSITION    4U
int32_t gt911_p5_tid(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : TH_GROUP
  * Address       : 0X80
  * Bit Group Name: Threshold for touch detection
  * Permission    : RW
  * Default value : None
  *******************************************************************************/
#define   GT911_TH_GROUP_BIT_MASK        0xFFU
#define   GT911_TH_GROUP_BIT_POSITION    0U
int32_t gt911_th_group(gt911_ctx_t *pCtx, uint8_t value);

/*******************************************************************************
  * Register      : TH_DIFF
  * Address       : 0X85
  * Bit Group Name: Filter function coefficient
  * Permission    : RW
  * Default value : None
  *******************************************************************************/
#define   GT911_TH_DIFF_BIT_MASK        0xFFU
#define   GT911_TH_DIFF_BIT_POSITION    0U
int32_t gt911_th_diff(gt911_ctx_t *pCtx, uint8_t value);

/*******************************************************************************
  * Register      : CTRL
  * Address       : 0X86
  * Bit Group Name:
  * Permission    : RW
  * Default value : 0x01
  *******************************************************************************/
#define   GT911_CTRL_BIT_MASK           0xFFU
#define   GT911_CTRL_BIT_POSITION       0U
int32_t gt911_ctrl(gt911_ctx_t *pCtx, uint8_t value);

/*******************************************************************************
  * Register      : TIMEENTERMONITOR
  * Address       : 0X87
  * Bit Group Name:
  * Permission    : RW
  * Default value : 0x0A
  *******************************************************************************/
#define   GT911_TIMEENTERMONITOR_BIT_MASK           0xFFU
#define   GT911_TIMEENTERMONITOR_BIT_POSITION       0U
int32_t gt911_time_enter_monitor(gt911_ctx_t *pCtx, uint8_t value);

/*******************************************************************************
  * Register      : PERIODACTIVE
  * Address       : 0X88
  * Bit Group Name:
  * Permission    : RW
  * Default value : None
  *******************************************************************************/
#define   GT911_PERIODACTIVE_BIT_MASK           0xFFU
#define   GT911_PERIODACTIVE_BIT_POSITION       0U
int32_t gt911_period_active(gt911_ctx_t *pCtx, uint8_t value);

/*******************************************************************************
  * Register      : PERIODMONITOR
  * Address       : 0X89
  * Bit Group Name:
  * Permission    : RW
  * Default value : 0x28
  *******************************************************************************/
#define   GT911_PERIODMONITOR_BIT_MASK           0xFFU
#define   GT911_PERIODMONITOR_BIT_POSITION       0U
int32_t gt911_period_monitor(gt911_ctx_t *pCtx, uint8_t value);

/*******************************************************************************
  * Register      : DISTANCE_LEFT_RIGHT
  * Address       : 0X94
  * Bit Group Name:
  * Permission    : RW
  * Default value : 0x19
  *******************************************************************************/
#define   GT911_DISTANCE_LR_BIT_MASK           0x0FU
#define   GT911_DISTANCE_LR_BIT_POSITION       0U
int32_t  gt911_distance_left_right(gt911_ctx_t *pCtx, uint8_t value);

/*******************************************************************************
  * Register      : DISTANCE_UP_DOWN
  * Address       : 0X95
  * Bit Group Name:
  * Permission    : RW
  * Default value : 0x19
  *******************************************************************************/
#define   GT911_DISTANCE_UD_BIT_MASK           0xF0U
#define   GT911_DISTANCE_UD_BIT_POSITION       0U
int32_t gt911_distance_up_down(gt911_ctx_t *pCtx, uint8_t value);

/*******************************************************************************
  * Register      : LIB_VER_H
  * Address       : 0XA1
  * Bit Group Name:
  * Permission    : R
  * Default value : None
  *******************************************************************************/
#define   GT911_LIB_VER_H_BIT_MASK           0xFFU
#define   GT911_LIB_VER_H_BIT_POSITION       0U
int32_t gt911_lib_ver_high(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : LIB_VER_L
  * Address       : 0XA2
  * Bit Group Name:
  * Permission    : R
  * Default value : None
  *******************************************************************************/
#define   GT911_LIB_VER_L_BIT_MASK           0xFFU
#define   GT911_LIB_VER_L_BIT_POSITION       0U
int32_t gt911_lib_ver_low(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : CIPHER
  * Address       : 0XA3
  * Bit Group Name:
  * Permission    : R
  * Default value : 0x06
  *******************************************************************************/
#define   GT911_CIPHER_BIT_MASK           0xFFU
#define   GT911_CIPHER_BIT_POSITION       0U
int32_t gt911_cipher(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : M_SW1
  * Address       : 0x804D
  * Bit Group Name:
  * Permission    : RW
  * Default value : 0x00?
  *******************************************************************************/
#define   GT911_M_SW1_BIT_MASK            0x03U
#define   GT911_M_SW1_BIT_POSITION        0U
int32_t gt911_m_sw1(gt911_ctx_t *pCtx, uint8_t value);

/*******************************************************************************
  * Register      : PWR_MODE
  * Address       : 0XA5
  * Bit Group Name:
  * Permission    : RW
  * Default value : 0x00
  *******************************************************************************/
#define   GT911_PWR_MODE_BIT_MASK           0xFFU
#define   GT911_PWR_MODE_BIT_POSITION       0U
int32_t gt911_pwr_mode(gt911_ctx_t *pCtx, uint8_t value);

/*******************************************************************************
  * Register      : FIRMID
  * Address       : 0XA6
  * Bit Group Name:
  * Permission    : R
  * Default value : None
  *******************************************************************************/
#define   GT911_FIRMID_BIT_MASK           0xFFU
#define   GT911_FIRMID_BIT_POSITION       0U
int32_t gt911_firm_id(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : FOCALTECH_ID
  * Address       : 0XA8
  * Bit Group Name:
  * Permission    : R
  * Default value : 0x11
  *******************************************************************************/
#define   GT911_CHIP_ID_BIT_MASK           0xFFU
#define   GT911_CHIP_ID_BIT_POSITION       0U
int32_t gt911_chip_id(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : RELEASE_CODE_ID
  * Address       : 0XAF
  * Bit Group Name:
  * Permission    : R
  * Default value : 0x001
  *******************************************************************************/
#define   GT911_RC_ID_BIT_MASK           0xFFU
#define   GT911_RC_ID_BIT_POSITION       0U
int32_t gt911_release_code_id(gt911_ctx_t *pCtx, uint8_t *pValue);

/*******************************************************************************
  * Register      : STATE
  * Address       : 0XBC
  * Bit Group Name:
  * Permission    : RW
  * Default value : 0x01
  *******************************************************************************/
#define   GT911_STATE_BIT_MASK           0xFFU
#define   GT911_STATE_BIT_POSITION       0U
int32_t gt911_mode(gt911_ctx_t *pCtx, uint8_t value);

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

/**
  * @}
  */

#ifdef __cplusplus
}
#endif
#endif /* GT911_REG_H */
