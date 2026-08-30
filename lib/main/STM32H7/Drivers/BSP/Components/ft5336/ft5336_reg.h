/**
 ******************************************************************************
 * @file    ft5336_reg.h
 * @author  MCD Application Team
 * @brief   Header of ft5336_reg.c
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
#ifndef FT5336_REG_H
#define FT5336_REG_H

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

/** @addtogroup FT5336
 * @{
 */

/* Exported types ------------------------------------------------------------*/

/** @defgroup FT5336_Exported_Types FT5336 Exported Types
 * @{
 */
/************** Generic Function  *******************/

typedef int32_t (*FT5336_Write_Func)(void *, uint8_t, uint8_t*, uint16_t);
typedef int32_t (*FT5336_Read_Func) (void *, uint8_t, uint8_t*, uint16_t);

/**
 * @}
 */

/** @defgroup FT5336_Imported_Globals FT5336 Imported Globals
 * @{
 */
typedef struct
{
  FT5336_Write_Func   WriteReg;
  FT5336_Read_Func    ReadReg;
  void                *handle;
} ft5336_ctx_t;
/**
 * @}
 */

/** @defgroup FT5336_Exported_Constants FT5336 Exported Constants
 * @{
 */

/* Current mode register of the FT5336 (R/W) */
#define FT5336_DEV_MODE_REG         0x00

/* Gesture ID register */
#define FT5336_GEST_ID_REG          0x01

/* Touch Data Status register : gives number of active touch points (0..2) */
#define FT5336_TD_STAT_REG          0x02

/* P1 X, Y coordinates, weight and misc registers */
#define FT5336_P1_XH_REG            0x03U
#define FT5336_P1_XL_REG            0x04U
#define FT5336_P1_YH_REG            0x05U
#define FT5336_P1_YL_REG            0x06U
#define FT5336_P1_WEIGHT_REG        0x07U
#define FT5336_P1_MISC_REG          0x08U

/* P2 X, Y coordinates, weight and misc registers */
#define FT5336_P2_XH_REG            0x09U
#define FT5336_P2_XL_REG            0x0AU
#define FT5336_P2_YH_REG            0x0BU
#define FT5336_P2_YL_REG            0x0CU
#define FT5336_P2_WEIGHT_REG        0x0DU
#define FT5336_P2_MISC_REG          0x0EU

/* P3 X, Y coordinates, weight and misc registers */
#define FT5336_P3_XH_REG            0x0FU
#define FT5336_P3_XL_REG            0x10U
#define FT5336_P3_YH_REG            0x11U
#define FT5336_P3_YL_REG            0x12U
#define FT5336_P3_WEIGHT_REG        0x13U
#define FT5336_P3_MISC_REG          0x14U

/* P4 X, Y coordinates, weight and misc registers */
#define FT5336_P4_XH_REG            0x15U
#define FT5336_P4_XL_REG            0x16U
#define FT5336_P4_YH_REG            0x17U
#define FT5336_P4_YL_REG            0x18U
#define FT5336_P4_WEIGHT_REG        0x19U
#define FT5336_P4_MISC_REG          0x1AU

/* P5 X, Y coordinates, weight and misc registers */
#define FT5336_P5_XH_REG            0x1BU
#define FT5336_P5_XL_REG            0x1CU
#define FT5336_P5_YH_REG            0x1DU
#define FT5336_P5_YL_REG            0x1EU
#define FT5336_P5_WEIGHT_REG        0x1FU
#define FT5336_P5_MISC_REG          0x20U

/* P6 X, Y coordinates, weight and misc registers */
#define FT5336_P6_XH_REG            0x21U
#define FT5336_P6_XL_REG            0x22U
#define FT5336_P6_YH_REG            0x23U
#define FT5336_P6_YL_REG            0x24U
#define FT5336_P6_WEIGHT_REG        0x25U
#define FT5336_P6_MISC_REG          0x26U

/* P7 X, Y coordinates, weight and misc registers */
#define FT5336_P7_XH_REG            0x27U
#define FT5336_P7_XL_REG            0x28U
#define FT5336_P7_YH_REG            0x29U
#define FT5336_P7_YL_REG            0x2AU
#define FT5336_P7_WEIGHT_REG        0x2BU
#define FT5336_P7_MISC_REG          0x2CU

/* P8 X, Y coordinates, weight and misc registers */
#define FT5336_P8_XH_REG            0x2DU
#define FT5336_P8_XL_REG            0x2EU
#define FT5336_P8_YH_REG            0x2FU
#define FT5336_P8_YL_REG            0x30U
#define FT5336_P8_WEIGHT_REG        0x31U
#define FT5336_P8_MISC_REG          0x32U

/* P9 X, Y coordinates, weight and misc registers */
#define FT5336_P9_XH_REG            0x33U
#define FT5336_P9_XL_REG            0x34U
#define FT5336_P9_YH_REG            0x35U
#define FT5336_P9_YL_REG            0x36U
#define FT5336_P9_WEIGHT_REG        0x37U
#define FT5336_P9_MISC_REG          0x38U

/* P10 X, Y coordinates, weight and misc registers */
#define FT5336_P10_XH_REG           0x39U
#define FT5336_P10_XL_REG           0x3AU
#define FT5336_P10_YH_REG           0x3BU
#define FT5336_P10_YL_REG           0x3CU
#define FT5336_P10_WEIGHT_REG       0x3DU
#define FT5336_P10_MISC_REG         0x3EU

/* Threshold for touch detection */
#define FT5336_TH_GROUP_REG         0x80

/* Filter function coefficients */
#define FT5336_TH_DIFF_REG          0x85

/* Control register */
#define FT5336_CTRL_REG             0x86

/* The time period of switching from Active mode to Monitor mode when there is no touching */
#define FT5336_TIMEENTERMONITOR_REG 0x87

/* Report rate in Active mode */
#define FT5336_PERIODACTIVE_REG     0x88

/* Report rate in Monitor mode */
#define FT5336_PERIODMONITOR_REG    0x89

/* The value of the minimum allowed angle while Rotating gesture mode */
#define FT5336_RADIAN_VALUE_REG     0x91

/* Maximum offset while Moving Left and Moving Right gesture */
#define FT5336_OFFSET_LR_REG        0x92

/* Maximum offset while Moving Up and Moving Down gesture */
#define FT5336_OFFSET_UD_REG        0x93

/* Minimum distance while Moving Left and Moving Right gesture */
#define FT5336_DISTANCE_LR_REG      0x94

/* Minimum distance while Moving Up and Moving Down gesture */
#define FT5336_DISTANCE_UD_REG      0x95

/* Maximum distance while Zoom In and Zoom Out gesture */
#define FT5336_DISTANCE_ZOOM_REG    0x96

/* High 8-bit of LIB Version info */
#define FT5336_LIB_VER_H_REG        0xA1

/* Low 8-bit of LIB Version info */
#define FT5336_LIB_VER_L_REG        0xA2

/* Chip Selecting */
#define FT5336_CIPHER_REG           0xA3

/* Interrupt mode register (used when in interrupt mode) */
#define FT5336_GMODE_REG            0xA4

/* Current power mode the FT5336 system is in (R) */
#define FT5336_PWR_MODE_REG         0xA5

/* FT5336 firmware version */
#define FT5336_FIRMID_REG           0xA6

/* FT5336 Chip identification register */
#define FT5336_CHIP_ID_REG          0xA8

/* Release code version */
#define FT5336_RELEASE_CODE_ID_REG  0xAF

/* Current operating mode the FT5336 system is in (R) */
#define FT5336_STATE_REG            0xBC

/**
 * @}
 */

/*******************************************************************************
* Register      : Generic - All
* Address       : Generic - All
* Bit Group Name: None
* Permission    : W
*******************************************************************************/
int32_t ft5336_write_reg(ft5336_ctx_t *ctx, uint8_t reg, uint8_t *pbuf, uint16_t length);
int32_t ft5336_read_reg(ft5336_ctx_t *ctx, uint8_t reg, uint8_t *pbuf, uint16_t length);

/**************** Base Function  *******************/

/*******************************************************************************
* Register      : DEV_MODE
* Address       : 0X00
* Bit Group Name: DEVICE_MODE
* Permission    : RW
*******************************************************************************/
#define   FT5336_DEV_MODE_BIT_MASK        0x70U
#define   FT5336_DEV_MODE_BIT_POSITION    4
int32_t  ft5336_dev_mode_w(ft5336_ctx_t *ctx, uint8_t value);
int32_t  ft5336_dev_mode_r(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : GEST_ID
* Address       : 0X01
* Bit Group Name: Gesture ID
* Permission    : R
*******************************************************************************/
#define   FT5336_GEST_ID_BIT_MASK        0xFFU
#define   FT5336_GEST_ID_BIT_POSITION    0
int32_t ft5336_gest_id(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : TD_STATUS
* Address       : 0X02
* Bit Group Name: 
* Permission    : R
*******************************************************************************/
#define   FT5336_TD_STATUS_BIT_MASK        0x0FU
#define   FT5336_TD_STATUS_BIT_POSITION    0
int32_t ft5336_td_status(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P1_XH
* Address       : 0X03
* Bit Group Name: First Event Flag
* Permission    : R
* Default value : 0xF0U
*******************************************************************************/
#define   FT5336_P1_XH_EF_BIT_MASK        0xC0U
#define   FT5336_P1_XH_EF_BIT_POSITION    6
int32_t ft5336_p1_xh_ef(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P1_XH
* Address       : 0X03
* Bit Group Name: First Touch X Position
* Permission    : R
* Default value : 0x0FU
*******************************************************************************/
#define   FT5336_P1_XH_TP_BIT_MASK        0x0FU
#define   FT5336_P1_XH_TP_BIT_POSITION    0
int32_t ft5336_p1_xh_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P1_XL
* Address       : 0X04
* Bit Group Name: First Touch X Position
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P1_XL_TP_BIT_MASK        0xFFU
#define   FT5336_P1_XL_TP_BIT_POSITION    0
int32_t ft5336_p1_xl_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P1_YH
* Address       : 0X05
* Bit Group Name: First Touch ID
* Permission    : R
* Default value : 0xF0U
*******************************************************************************/
#define   FT5336_P1_YH_TID_BIT_MASK        0xF0U
#define   FT5336_P1_YH_TID_BIT_POSITION    4
int32_t ft5336_p1_yh_tid(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P1_YH
* Address       : 0x06
* Bit Group Name: First Touch Y Position
* Permission    : R
* Default value : 0x0FU
*******************************************************************************/
#define   FT5336_P1_YH_TP_BIT_MASK        0x0FU
#define   FT5336_P1_YH_TP_BIT_POSITION    0
int32_t ft5336_p1_yh_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P1_YL
* Address       : 0X06
* Bit Group Name: First Touch Y Position
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P1_YL_TP_BIT_MASK        0xFFU
#define   FT5336_P1_YL_TP_BIT_POSITION    0
int32_t ft5336_p1_yl_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P1_WEIGHT
* Address       : 0X07
* Bit Group Name: First Touch Weight(pressure)
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P1_WEIGHT_BIT_MASK        0xFFU
#define   FT5336_P1_WEIGHT_BIT_POSITION    0
int32_t ft5336_p1_weight(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P1_MISC
* Address       : 0X08
* Bit Group Name: First Touch Area
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P1_MISC_BIT_MASK        0xF0U
#define   FT5336_P1_MISC_BIT_POSITION    4
int32_t ft5336_p1_misc(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P2_XH
* Address       : 0X09
* Bit Group Name: Second Event Flag
* Permission    : R
* Default value : 0xF0U
*******************************************************************************/
#define   FT5336_P2_XH_EF_BIT_MASK        0xC0U
#define   FT5336_P2_XH_EF_BIT_POSITION    6
int32_t ft5336_p2_xh_ef(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P2_XH
* Address       : 0X09
* Bit Group Name: Second Touch X Position
* Permission    : R
* Default value : 0x0FU
*******************************************************************************/
#define   FT5336_P2_XH_TP_BIT_MASK        0x0FU
#define   FT5336_P2_XH_TP_BIT_POSITION    0
int32_t ft5336_p2_xh_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P2_XL
* Address       : 0X0A
* Bit Group Name: Second Touch X Position
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P2_XL_TP_BIT_MASK        0xFFU
#define   FT5336_P2_XL_TP_BIT_POSITION    0
int32_t ft5336_p2_xl_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P2_YH
* Address       : 0X0B
* Bit Group Name: Second Touch ID
* Permission    : R
* Default value : 0xF0U
*******************************************************************************/
#define   FT5336_P2_YH_TID_BIT_MASK        0xF0U
#define   FT5336_P2_YH_TID_BIT_POSITION    4
int32_t ft5336_p2_yh_tid(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P2_YH
* Address       : 0x0B
* Bit Group Name: Second Touch Y Position
* Permission    : R
* Default value : 0x0FU
*******************************************************************************/
#define   FT5336_P2_YH_TP_BIT_MASK        0x0FU
#define   FT5336_P2_YH_TP_BIT_POSITION    0
int32_t ft5336_p2_yh_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P2_YL
* Address       : 0X0C
* Bit Group Name: Second Touch Y Position
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P2_YL_TP_BIT_MASK        0xFFU
#define   FT5336_P2_YL_TP_BIT_POSITION    0
int32_t ft5336_p2_yl_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P2_WEIGHT
* Address       : 0X0D
* Bit Group Name: Second Touch Weight(pressure)
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P2_WEIGHT_BIT_MASK        0xFFU
#define   FT5336_P2_WEIGHT_BIT_POSITION    0
int32_t ft5336_p2_weight(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P2_MISC
* Address       : 0X0E
* Bit Group Name: Second Touch Area
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P2_MISC_BIT_MASK        0xF0U
#define   FT5336_P2_MISC_BIT_POSITION    4
int32_t ft5336_p2_misc(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P3_XH
* Address       : 0X0F
* Bit Group Name: Third Event Flag
* Permission    : R
* Default value : 0xF0U
*******************************************************************************/
#define   FT5336_P3_XH_EF_BIT_MASK        0xC0U
#define   FT5336_P3_XH_EF_BIT_POSITION    6
int32_t ft5336_p3_xh_ef(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P3_XH
* Address       : 0X0F
* Bit Group Name: Third Touch X High Position
* Permission    : R
* Default value : 0x0FU
*******************************************************************************/
#define   FT5336_P3_XH_TP_BIT_MASK        0x0FU
#define   FT5336_P3_XH_TP_BIT_POSITION    0
int32_t ft5336_p3_xh_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P3_XL
* Address       : 0X10
* Bit Group Name: Third Touch X Low Position
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P3_XL_TP_BIT_MASK        0xFFU
#define   FT5336_P3_XL_TP_BIT_POSITION    0
int32_t ft5336_p3_xl_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P3_YH
* Address       : 0X11
* Bit Group Name: Third Touch ID
* Permission    : R
* Default value : 0xF0U
*******************************************************************************/
#define   FT5336_P3_YH_TID_BIT_MASK        0xF0U
#define   FT5336_P3_YH_TID_BIT_POSITION    4
int32_t ft5336_p3_yh_tid(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P3_YH
* Address       : 0x11
* Bit Group Name: Third Touch Y High Position
* Permission    : R
* Default value : 0x0FU
*******************************************************************************/
#define   FT5336_P3_YH_TP_BIT_MASK        0x0FU
#define   FT5336_P3_YH_TP_BIT_POSITION    0
int32_t ft5336_p3_yh_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P3_YL
* Address       : 0X12
* Bit Group Name: Third Touch Y Low Position
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P3_YL_TP_BIT_MASK        0xFFU
#define   FT5336_P3_YL_TP_BIT_POSITION    0
int32_t ft5336_p3_yl_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P3_WEIGHT
* Address       : 0X13
* Bit Group Name: Third Touch Weight(pressure)
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P3_WEIGHT_BIT_MASK        0xFFU
#define   FT5336_P3_WEIGHT_BIT_POSITION    0
int32_t ft5336_p3_weight(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P3_MISC
* Address       : 0X14
* Bit Group Name: Third Touch Area
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P3_MISC_BIT_MASK        0xF0U
#define   FT5336_P3_MISC_BIT_POSITION    4
int32_t ft5336_p3_misc(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P4_XH
* Address       : 0X15
* Bit Group Name: Fourth Event Flag
* Permission    : R
* Default value : 0xF0U
*******************************************************************************/
#define   FT5336_P4_XH_EF_BIT_MASK        0xC0U
#define   FT5336_P4_XH_EF_BIT_POSITION    6
int32_t ft5336_p4_xh_ef(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P4_XH
* Address       : 0X15
* Bit Group Name: Fourth Touch X High Position
* Permission    : R
* Default value : 0x0FU
*******************************************************************************/
#define   FT5336_P4_XH_TP_BIT_MASK        0x0FU
#define   FT5336_P4_XH_TP_BIT_POSITION    0
int32_t ft5336_p4_xh_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P4_XL
* Address       : 0X16
* Bit Group Name: Fourth Touch X Low Position
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P4_XL_TP_BIT_MASK        0xFFU
#define   FT5336_P4_XL_TP_BIT_POSITION    0
int32_t ft5336_p4_xl_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P4_YH
* Address       : 0X17
* Bit Group Name: Fourth Touch ID
* Permission    : R
* Default value : 0xF0U
*******************************************************************************/
#define   FT5336_P4_YH_TID_BIT_MASK        0xF0U
#define   FT5336_P4_YH_TID_BIT_POSITION    4
int32_t ft5336_p4_yh_tid(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P4_YH
* Address       : 0x17
* Bit Group Name: Fourth Touch Y High Position
* Permission    : R
* Default value : 0x0FU
*******************************************************************************/
#define   FT5336_P4_YH_TP_BIT_MASK        0x0FU
#define   FT5336_P4_YH_TP_BIT_POSITION    0
int32_t ft5336_p4_yh_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P4_YL
* Address       : 0X18
* Bit Group Name: Fourth Touch Y Low Position
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P4_YL_TP_BIT_MASK        0xFFU
#define   FT5336_P4_YL_TP_BIT_POSITION    0
int32_t ft5336_p4_yl_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P4_WEIGHT
* Address       : 0X19
* Bit Group Name: Fourth Touch Weight(pressure)
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P4_WEIGHT_BIT_MASK        0xFFU
#define   FT5336_P4_WEIGHT_BIT_POSITION    0
int32_t ft5336_p4_weight(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P4_MISC
* Address       : 0X1A
* Bit Group Name: Fourth Touch Area
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P4_MISC_BIT_MASK        0xF0U
#define   FT5336_P4_MISC_BIT_POSITION    4
int32_t ft5336_p4_misc(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P5_XH
* Address       : 0X1B
* Bit Group Name: Fifth Event Flag
* Permission    : R
* Default value : 0xF0U
*******************************************************************************/
#define   FT5336_P5_XH_EF_BIT_MASK        0xC0U
#define   FT5336_P5_XH_EF_BIT_POSITION    6
int32_t ft5336_p5_xh_ef(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P5_XH
* Address       : 0X1B
* Bit Group Name: Fifth Touch X High Position
* Permission    : R
* Default value : 0x0FU
*******************************************************************************/
#define   FT5336_P5_XH_TP_BIT_MASK        0x0FU
#define   FT5336_P5_XH_TP_BIT_POSITION    0
int32_t ft5336_p5_xh_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P5_XL
* Address       : 0X1C
* Bit Group Name: Fifth Touch X Low Position
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P5_XL_TP_BIT_MASK        0xFFU
#define   FT5336_P5_XL_TP_BIT_POSITION    0
int32_t ft5336_p5_xl_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P5_YH
* Address       : 0X1D
* Bit Group Name: Fifth Touch ID
* Permission    : R
* Default value : 0xF0U
*******************************************************************************/
#define   FT5336_P5_YH_TID_BIT_MASK        0xF0U
#define   FT5336_P5_YH_TID_BIT_POSITION    4
int32_t ft5336_p5_yh_tid(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P5_YH
* Address       : 0x1D
* Bit Group Name: Fifth Touch Y High Position
* Permission    : R
* Default value : 0x0FU
*******************************************************************************/
#define   FT5336_P5_YH_TP_BIT_MASK        0x0FU
#define   FT5336_P5_YH_TP_BIT_POSITION    0
int32_t ft5336_p5_yh_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P5_YL
* Address       : 0X1E
* Bit Group Name: Fifth Touch Y Low Position
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P5_YL_TP_BIT_MASK        0xFFU
#define   FT5336_P5_YL_TP_BIT_POSITION    0
int32_t ft5336_p5_yl_tp(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P5_WEIGHT
* Address       : 0X1F
* Bit Group Name: Fifth Touch Weight(pressure)
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P5_WEIGHT_BIT_MASK        0xFFU
#define   FT5336_P5_WEIGHT_BIT_POSITION    0
int32_t ft5336_p5_weight(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : P5_MISC
* Address       : 0X20
* Bit Group Name: Fifth Touch Area
* Permission    : R
* Default value : 0xFFU
*******************************************************************************/
#define   FT5336_P5_MISC_BIT_MASK        0xF0U
#define   FT5336_P5_MISC_BIT_POSITION    4
int32_t ft5336_p5_misc(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : TH_GROUP
* Address       : 0X80
* Bit Group Name: Threshold for touch detection
* Permission    : RW
* Default value : None
*******************************************************************************/
#define   FT5336_TH_GROUP_BIT_MASK        0xFFU
#define   FT5336_TH_GROUP_BIT_POSITION    0
int32_t ft5336_th_group(ft5336_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : TH_DIFF
* Address       : 0X85
* Bit Group Name: Filter function coefficient
* Permission    : RW
* Default value : None
*******************************************************************************/
#define   FT5336_TH_DIFF_BIT_MASK        0xFFU
#define   FT5336_TH_DIFF_BIT_POSITION    0
int32_t ft5336_th_diff(ft5336_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : CTRL
* Address       : 0X86
* Bit Group Name: 
* Permission    : RW
* Default value : 0x01
*******************************************************************************/
#define   FT5336_CTRL_BIT_MASK           0xFFU
#define   FT5336_CTRL_BIT_POSITION       0
int32_t ft5336_ctrl(ft5336_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : TIMEENTERMONITOR
* Address       : 0X87
* Bit Group Name: 
* Permission    : RW
* Default value : 0x0A
*******************************************************************************/
#define   FT5336_TIMEENTERMONITOR_BIT_MASK           0xFFU
#define   FT5336_TIMEENTERMONITOR_BIT_POSITION       0
int32_t ft5336_time_enter_monitor(ft5336_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : PERIODACTIVE
* Address       : 0X88
* Bit Group Name: 
* Permission    : RW
* Default value : None
*******************************************************************************/
#define   FT5336_PERIODACTIVE_BIT_MASK           0xFFU
#define   FT5336_PERIODACTIVE_BIT_POSITION       0
int32_t ft5336_period_active(ft5336_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : PERIODMONITOR
* Address       : 0X89
* Bit Group Name: 
* Permission    : RW
* Default value : 0x28
*******************************************************************************/
#define   FT5336_PERIODMONITOR_BIT_MASK           0xFFU
#define   FT5336_PERIODMONITOR_BIT_POSITION       0
int32_t ft5336_period_monitor(ft5336_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : RADIAN_VALUE
* Address       : 0X91
* Bit Group Name: 
* Permission    : RW
* Default value : 0x0A
*******************************************************************************/
#define   FT5336_RADIAN_VALUE_BIT_MASK           0xFFU
#define   FT5336_RADIAN_VALUE_BIT_POSITION       0
int32_t ft5336_radian_value(ft5336_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : OFFSET_LEFT_RIGHT
* Address       : 0X92
* Bit Group Name: 
* Permission    : RW
* Default value : 0x19
*******************************************************************************/
#define   FT5336_OFFSET_LR_BIT_MASK           0xFFU
#define   FT5336_OFFSET_LR_BIT_POSITION       0
int32_t ft5336_offset_left_right(ft5336_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : OFFSET_UP_DOWN
* Address       : 0X93
* Bit Group Name: 
* Permission    : RW
* Default value : 0x19
*******************************************************************************/
#define   FT5336_OFFSET_UD_BIT_MASK           0xFFU
#define   FT5336_OFFSET_UD_BIT_POSITION       0
int32_t ft5336_offset_up_down(ft5336_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : DISTANCE_LEFT_RIGHT
* Address       : 0X94
* Bit Group Name: 
* Permission    : RW
* Default value : 0x19
*******************************************************************************/
#define   FT5336_DISTANCE_LR_BIT_MASK           0xFFU
#define   FT5336_DISTANCE_LR_BIT_POSITION       0
int32_t  ft5336_disatnce_left_right(ft5336_ctx_t *ctx, uint8_t value);
        
/*******************************************************************************
* Register      : DISTANCE_UP_DOWN
* Address       : 0X95
* Bit Group Name: 
* Permission    : RW
* Default value : 0x19
*******************************************************************************/
#define   FT5336_DISTANCE_UD_BIT_MASK           0xFFU
#define   FT5336_DISTANCE_UD_BIT_POSITION       0
int32_t ft5336_distance_up_down(ft5336_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : DISTANCE_ZOOM
* Address       : 0X96
* Bit Group Name: 
* Permission    : RW
* Default value : 0x32
*******************************************************************************/
#define   FT5336_DISTANCE_ZOOM_BIT_MASK           0xFFU
#define   FT5336_DISTANCE_ZOOM_BIT_POSITION       0
int32_t ft5336_distance_zoom(ft5336_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : LIB_VER_H
* Address       : 0XA1
* Bit Group Name: 
* Permission    : R
* Default value : None
*******************************************************************************/
#define   FT5336_LIB_VER_H_BIT_MASK           0xFFU
#define   FT5336_LIB_VER_H_BIT_POSITION       0
int32_t ft5336_lib_ver_high(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : LIB_VER_L
* Address       : 0XA2
* Bit Group Name: 
* Permission    : R
* Default value : None
*******************************************************************************/
#define   FT5336_LIB_VER_L_BIT_MASK           0xFFU
#define   FT5336_LIB_VER_L_BIT_POSITION       0
int32_t ft5336_lib_ver_low(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : CIPHER
* Address       : 0XA3
* Bit Group Name: 
* Permission    : R
* Default value : 0x06
*******************************************************************************/
#define   FT5336_CIPHER_BIT_MASK           0xFFU
#define   FT5336_CIPHER_BIT_POSITION       0
int32_t ft5336_cipher(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : G_MODE
* Address       : 0XA4
* Bit Group Name: 
* Permission    : RW
* Default value : 0x01
*******************************************************************************/
#define   FT5336_G_MODE_BIT_MASK           0xFFU
#define   FT5336_G_MODE_BIT_POSITION       0
int32_t ft5336_g_mode(ft5336_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : PWR_MODE
* Address       : 0XA5
* Bit Group Name: 
* Permission    : RW
* Default value : 0x00
*******************************************************************************/
#define   FT5336_PWR_MODE_BIT_MASK           0xFFU
#define   FT5336_PWR_MODE_BIT_POSITION       0
int32_t ft5336_pwr_mode(ft5336_ctx_t *ctx, uint8_t value);

/*******************************************************************************
* Register      : FIRMID
* Address       : 0XA6
* Bit Group Name: 
* Permission    : R
* Default value : None
*******************************************************************************/
#define   FT5336_FIRMID_BIT_MASK           0xFFU
#define   FT5336_FIRMID_BIT_POSITION       0
int32_t ft5336_firm_id(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : FOCALTECH_ID
* Address       : 0XA8
* Bit Group Name: 
* Permission    : R
* Default value : 0x11
*******************************************************************************/
#define   FT5336_CHIP_ID_BIT_MASK           0xFFU
#define   FT5336_CHIP_ID_BIT_POSITION       0
int32_t ft5336_chip_id(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : RELEASE_CODE_ID
* Address       : 0XAF
* Bit Group Name: 
* Permission    : R
* Default value : 0x001
*******************************************************************************/
#define   FT5336_RC_ID_BIT_MASK           0xFFU
#define   FT5336_RC_ID_BIT_POSITION       0
int32_t ft5336_release_code_id(ft5336_ctx_t *ctx, uint8_t *value);

/*******************************************************************************
* Register      : STATE
* Address       : 0XBC
* Bit Group Name: 
* Permission    : RW
* Default value : 0x01
*******************************************************************************/
#define   FT5336_STATE_BIT_MASK           0xFFU
#define   FT5336_STATE_BIT_POSITION       0
int32_t ft5336_state(ft5336_ctx_t *ctx, uint8_t value);

  /**
   * @}
   */

#ifdef __cplusplus
}
#endif
#endif /* FT5336_REG_H */


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
