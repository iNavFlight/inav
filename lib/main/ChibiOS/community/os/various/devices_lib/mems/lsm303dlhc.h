/*
    Pretty LAYer for ChibiOS/RT - Copyright (C) 2015 Rocco Marco Guglielmi
	
    This file is part of PLAY for ChibiOS/RT.

    PLAY is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    PLAY is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
    Special thanks to Giovanni Di Sirio for teachings, his moral support and
    friendship. Note that some or every piece of this file could be part of
    the ChibiOS project that is intellectual property of Giovanni Di Sirio.
    Please refer to ChibiOS/RT license before use this file.
	
	For suggestion or Bug report - roccomarco.guglielmi@playembedded.org
 */

/**
 * @file    lsm303dlhc.h
 * @brief   LSM303DLHC MEMS interface module through I2C header.
 *
 * @addtogroup lsm303dlhc
 * @{
 */

#ifndef _LSM303DLHC_H_
#define _LSM303DLHC_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define  LSM303DLHC_ACC_SENS_2G                  ((float)1671.836f)          /*!< Accelerometer sensitivity with 2 G full scale [LSB * s^2 / m] */
#define  LSM303DLHC_ACC_SENS_4G                  ((float)835.918f)           /*!< Accelerometer sensitivity with 4 G full scale [LSB * s^2 / m] */
#define  LSM303DLHC_ACC_SENS_8G                  ((float)417.959f)           /*!< Accelerometer sensitivity with 8 G full scale [LSB * s^2 / m] */
#define  LSM303DLHC_ACC_SENS_16G                 ((float)208.979f)           /*!< Accelerometer sensitivity with 16 G full scale [LSB * s^2 / m] */

#define  LSM303DLHC_COMP_SENS_XY_1_3GA           ((float)1100.0f)            /*!< Compass sensitivity with 1.3 GA full scale [LSB  / Ga] */
#define  LSM303DLHC_COMP_SENS_XY_1_9GA           ((float)855.0f)             /*!< Compass sensitivity with 1.9 GA full scale [LSB  / Ga] */
#define  LSM303DLHC_COMP_SENS_XY_2_5GA           ((float)670.0f)             /*!< Compass sensitivity with 2.5 GA full scale [LSB  / Ga] */
#define  LSM303DLHC_COMP_SENS_XY_4_0GA           ((float)450.0f)             /*!< Compass sensitivity with 4.0 GA full scale [LSB  / Ga] */
#define  LSM303DLHC_COMP_SENS_XY_4_7GA           ((float)400.0f)             /*!< Compass sensitivity with 4.7 GA full scale [LSB  / Ga] */
#define  LSM303DLHC_COMP_SENS_XY_5_6GA           ((float)330.0f)             /*!< Compass sensitivity with 5.6 GA full scale [LSB  / Ga] */
#define  LSM303DLHC_COMP_SENS_XY_8_1GA           ((float)230.0f)             /*!< Compass sensitivity with 8.1 GA full scale [LSB  / Ga] */

#define  LSM303DLHC_COMP_SENS_Z_1_3GA            ((float)980.0f)             /*!< Compass sensitivity with 1.3 GA full scale [LSB  / Ga] */
#define  LSM303DLHC_COMP_SENS_Z_1_9GA            ((float)765.0f)             /*!< Compass sensitivity with 1.9 GA full scale [LSB  / Ga] */
#define  LSM303DLHC_COMP_SENS_Z_2_5GA            ((float)600.0f)             /*!< Compass sensitivity with 2.5 GA full scale [LSB  / Ga] */
#define  LSM303DLHC_COMP_SENS_Z_4_0GA            ((float)400.0f)             /*!< Compass sensitivity with 4.0 GA full scale [LSB  / Ga] */
#define  LSM303DLHC_COMP_SENS_Z_4_7GA            ((float)355.0f)             /*!< Compass sensitivity with 4.7 GA full scale [LSB  / Ga] */
#define  LSM303DLHC_COMP_SENS_Z_5_6GA            ((float)295.0f)             /*!< Compass sensitivity with 5.6 GA full scale [LSB  / Ga] */
#define  LSM303DLHC_COMP_SENS_Z_8_1GA            ((float)205.0f)             /*!< Compass sensitivity with 8.1 GA full scale [LSB  / Ga] */
/**
 * @name    LSM303DLHC register names
 * @{
 */
/******************************************************************************/
/*                                                                            */
/*                       LSM303DLHC on board MEMS                             */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for I2C communication  *******************/
#define  LSM303DLHC_SAD                          ((uint8_t)0x7F)            /*!< SAD[6:0] Slave Address Mask */
#define  LSM303DLHC_SAD_ACCEL                    ((uint8_t)0x19)            /*!< ACCELEROMETER Slave Address */
#define  LSM303DLHC_SAD_COMPASS                  ((uint8_t)0x1E)            /*!< MAGNETOMETER  Slave Address */

#define  LSM303DLHC_SUB                          ((uint8_t)0x7F)            /*!< SUB[6:0] Sub-registers address Mask */
#define  LSM303DLHC_SUB_0                        ((uint8_t)0x01)            /*!< bit 0 */
#define  LSM303DLHC_SUB_1                        ((uint8_t)0x02)            /*!< bit 1 */
#define  LSM303DLHC_SUB_2                        ((uint8_t)0x08)            /*!< bit 3 */
#define  LSM303DLHC_SUB_4                        ((uint8_t)0x10)            /*!< bit 4 */
#define  LSM303DLHC_SUB_5                        ((uint8_t)0x20)            /*!< bit 5 */
#define  LSM303DLHC_SUB_6                        ((uint8_t)0x40)            /*!< bit 6 */

#define  LSM303DLHC_SUB_MSB                      ((uint8_t)0x80)            /*!< Multiple data read\write bit */

/********  Bit definition for Accelerometer SUB-Registers Addresses  **********/
#define  LSM303DLHC_SUB_ACC_CTRL_REG1            ((uint8_t)0x20)            /*!< CONTROL REGISTER 1 FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_CTRL_REG2            ((uint8_t)0x21)            /*!< CONTROL REGISTER 2 FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_CTRL_REG3            ((uint8_t)0x22)            /*!< CONTROL REGISTER 3 FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_CTRL_REG4            ((uint8_t)0x23)            /*!< CONTROL REGISTER 4 FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_CTRL_REG5            ((uint8_t)0x24)            /*!< CONTROL REGISTER 5 FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_CTRL_REG6            ((uint8_t)0x25)            /*!< CONTROL REGISTER 6 FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_REFERENCE            ((uint8_t)0x26)            /*!< REFERENCE/DATACAPTURE FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_STATUS_REG           ((uint8_t)0x27)            /*!< STATUS REGISTER FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_OUT_X_L              ((uint8_t)0x28)            /*!< OUTPUT X-AXIS LOW FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_OUT_X_H              ((uint8_t)0x29)            /*!< OUTPUT X-AXIS HIGH FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_OUT_Y_L              ((uint8_t)0x2A)            /*!< OUTPUT Y-AXIS LOW FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_OUT_Y_H              ((uint8_t)0x2B)            /*!< OUTPUT Y-AXIS HIGH FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_OUT_Z_L              ((uint8_t)0x2C)            /*!< OUTPUT Z-AXIS LOW FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_OUT_Z_H              ((uint8_t)0x2D)            /*!< OUTPUT Z-AXIS HIGH FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_FIFO_CTRL_REG        ((uint8_t)0x2E)            /*!< FIFO CONTROL REGISTER FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_FIFO_SRC_REG         ((uint8_t)0x2F)            /*!< FIFO SOURCE REGISTER FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_INT1_CFG             ((uint8_t)0x30)            /*!< INTERRUPT1 CONFIG FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_INT1_SOURCE          ((uint8_t)0x31)            /*!< INTERRUPT1 SOURCE FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_INT1_THS             ((uint8_t)0x32)            /*!< INTERRUPT1 THRESHOLD FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_INT1_DURATION        ((uint8_t)0x33)            /*!< INTERRUPT1 DURATION FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_INT2_CFG             ((uint8_t)0x34)            /*!< INTERRUPT2 CONFIG FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_INT2_SOURCE          ((uint8_t)0x35)            /*!< INTERRUPT2 SOURCE FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_INT2_THS             ((uint8_t)0x36)            /*!< INTERRUPT2 THRESHOLD FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_INT2_DURATION        ((uint8_t)0x37)            /*!< INTERRUPT2 DURATION FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_CLICK_CFG            ((uint8_t)0x38)            /*!< CLICK CONFIG FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_CLICK_SRC            ((uint8_t)0x39)            /*!< CLICK SOURCE FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_CLICK_THS            ((uint8_t)0x3A)            /*!< CLICK THRESHOLD FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_TIME_LIMIT           ((uint8_t)0x3B)            /*!< TIME LIMIT FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_TIME_LATENCY         ((uint8_t)0x3C)            /*!< TIME LATENCY FOR ACCELEROMETER */
#define  LSM303DLHC_SUB_ACC_TIME_WINDOW          ((uint8_t)0x3D)            /*!< TIME WINDOW FOR ACCELEROMETER */

/*********  Bit definition for Compass SUB-Registers Addresses  **********/
#define  LSM303DLHC_SUB_COMP_CRA_REG             ((uint8_t)0x00)            /*!< CONTROL REGISTER A FOR MAGNETOMETER */
#define  LSM303DLHC_SUB_COMP_CRB_REG             ((uint8_t)0x01)            /*!< CONTROL REGISTER B FOR MAGNETOMETER */
#define  LSM303DLHC_SUB_COMP_MR_REG              ((uint8_t)0x02)            /*!< STATUS REGISTER FOR MAGNETOMETER */
#define  LSM303DLHC_SUB_COMP_OUT_X_H             ((uint8_t)0x03)            /*!< OUTPUT X-AXIS HIGH FOR MAGNETOMETER */
#define  LSM303DLHC_SUB_COMP_OUT_X_L             ((uint8_t)0x04)            /*!< OUTPUT X-AXIS LOW FOR MAGNETOMETER */
#define  LSM303DLHC_SUB_COMP_OUT_Z_H             ((uint8_t)0x05)            /*!< OUTPUT Z-AXIS HIGH FOR MAGNETOMETER */
#define  LSM303DLHC_SUB_COMP_OUT_Z_L             ((uint8_t)0x06)            /*!< OUTPUT Z-AXIS LOW FOR MAGNETOMETER */
#define  LSM303DLHC_SUB_COMP_OUT_Y_H             ((uint8_t)0x07)            /*!< OUTPUT Y-AXIS HIGH FOR MAGNETOMETER */
#define  LSM303DLHC_SUB_COMP_OUT_Y_L             ((uint8_t)0x08)            /*!< OUTPUT Y-AXIS LOW FOR MAGNETOMETER */
#define  LSM303DLHC_SUB_COMP_SR_REG              ((uint8_t)0x09)            /*!< SR REGISTER FOR MAGNETOMETER */
#define  LSM303DLHC_SUB_COMP_IRA_REG             ((uint8_t)0x0A)            /*!< IR A REGISTER FOR MAGNETOMETER */
#define  LSM303DLHC_SUB_COMP_IRB_REG             ((uint8_t)0x0B)            /*!< IR B REGISTER FOR MAGNETOMETER */
#define  LSM303DLHC_SUB_COMP_IRC_REG             ((uint8_t)0x0C)            /*!< IR C REGISTER FOR MAGNETOMETER */
#define  LSM303DLHC_SUB_COMP_TEMP_OUT_H          ((uint8_t)0x31)            /*!< OUTPUT TEMP HIGH FOR MAGNETOMETER */
#define  LSM303DLHC_SUB_COMP_TEMP_OUT_L          ((uint8_t)0x32)            /*!< OUTPUT TEMP LOW FOR MAGNETOMETER */

/** @}  */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @name    Accelerometer data structures and types
 * @{
 */

/**
 * @brief  Accelerometer Output Data Rate
 */
typedef enum
{
  LSM303DLHC_ACC_ODR_PD        = 0x00,         /*!< Power down */
  LSM303DLHC_ACC_ODR_1Hz       = 0x10,         /*!< Output Data Rate = 1 Hz */
  LSM303DLHC_ACC_ODR_10Hz      = 0x20,         /*!< Output Data Rate = 10 Hz */
  LSM303DLHC_ACC_ODR_25Hz      = 0x30,         /*!< Output Data Rate = 25 Hz */
  LSM303DLHC_ACC_ODR_50Hz      = 0x40,         /*!< Output Data Rate = 50 Hz */
  LSM303DLHC_ACC_ODR_100Hz     = 0x50,         /*!< Output Data Rate = 100 Hz */
  LSM303DLHC_ACC_ODR_200Hz     = 0x60,         /*!< Output Data Rate = 200 Hz */
  LSM303DLHC_ACC_ODR_400Hz     = 0x70,         /*!< Output Data Rate = 400 Hz */
  LSM303DLHC_ACC_ODR_1620Hz    = 0x80,         /*!< Output Data Rate = 1620 Hz Low Power mode only */
  LSM303DLHC_ACC_ODR_1344Hz    = 0x90          /*!< Output Data Rate = 1344 Hz in Normal mode and 5376 Hz in Low Power Mode */
}LSM303DLHC_ACC_ODR_t;

/**
 * @brief  Accelerometer Power Mode
 */
typedef enum
{
  LSM303DLHC_ACC_PM_NORMAL    = 0x00,         /*!< Normal mode enabled */
  LSM303DLHC_ACC_PM_LOW_POWER = 0x08          /*!< Low Power mode enabled */
}LSM303DLHC_ACC_PM_t;

/**
 * @brief  Accelerometer Full Scale
 */
typedef enum
{
  LSM303DLHC_ACC_FS_2G   = 0x00,               /*!< ±2 g m/s^2 */
  LSM303DLHC_ACC_FS_4G   = 0x10,               /*!< ±4 g m/s^2 */
  LSM303DLHC_ACC_FS_8G   = 0x20,               /*!< ±8 g m/s^2 */
  LSM303DLHC_ACC_FS_16G  = 0x30                /*!< ±16 g m/s^2 */
}LSM303DLHC_ACC_FS_t;

/**
 * @brief   Accelerometer Axes Enabling
 */
typedef enum{
  LSM303DLHC_ACC_AE_DISABLED = 0x00,           /*!< Axes all disabled */
  LSM303DLHC_ACC_AE_X        = 0x01,           /*!< Only X-axis enabled */
  LSM303DLHC_ACC_AE_Y        = 0x02,           /*!< Only Y-axis enabled */
  LSM303DLHC_ACC_AE_XY       = 0x03,           /*!< X & Y axes enabled */
  LSM303DLHC_ACC_AE_Z        = 0x04,           /*!< Only Z-axis enabled */
  LSM303DLHC_ACC_AE_XZ       = 0x05,           /*!< X & Z axes enabled  */
  LSM303DLHC_ACC_AE_YZ       = 0x06,           /*!< Y & Z axes enabled  */
  LSM303DLHC_ACC_AE_XYZ      = 0x07            /*!< All axes enabled */
}LSM303DLHC_ACC_AE_t;

/**
 * @brief  Accelerometer Block Data Update
 */
typedef enum
{
  LSM303DLHC_ACC_BDU_CONTINOUS = 0x00,          /*!< Continuos Update */
  LSM303DLHC_ACC_BDU_BLOCKED   = 0x80           /*!< Single Update: output registers not updated until MSB and LSB reading */
}LSM303DLHC_ACC_BDU_t;

/**
 * @brief  Accelerometer Endianness
 */
typedef enum
{
  LSM303DLHC_ACC_End_LITTLE = 0x00,            /*!< Little Endian: data LSB @ lower address */
  LSM303DLHC_ACC_End_BIG    = 0x40             /*!< Big Endian: data MSB @ lower address */
}LSM303DLHC_ACC_End_t;

/**
 * @brief  Accelerometer High Resolution mode
 */
typedef enum
{
  LSM303DLHC_ACC_HR_Enabled  = 0x08,            /*!< High resolution output mode enabled */
  LSM303DLHC_ACC_HR_Disabled = 0x00             /*!< High resolution output mode disabled */
}LSM303DLHC_ACC_HR_t;

/**
 * @brief   Accelerometer configuration structure.
 */
typedef struct {
  /**
   * @brief Accelerometer fullscale value.
   */
  LSM303DLHC_ACC_FS_t    fullscale;
  /**
   * @brief Accelerometer power mode selection.
   */
  LSM303DLHC_ACC_PM_t    powermode;
  /**
   * @brief Accelerometer output data rate selection.
   */
  LSM303DLHC_ACC_ODR_t   outputdatarate;
  /**
   * @brief Accelerometer axes enabling.
   */
  LSM303DLHC_ACC_AE_t    axesenabling;
  /**
   * @brief Accelerometer block data update.
   */
  LSM303DLHC_ACC_BDU_t   blockdataupdate;
  /**
   * @brief Accelerometer block data update.
   */
  LSM303DLHC_ACC_HR_t    highresmode;
} LSM303DLHC_ACC_Config;
/** @}  */


/**
 * @name    Compass data types
 * @{
 */

/**
 * @brief  Compass Output Data Rate
 */
typedef enum
{
  LSM303DLHC_COMP_ODR_0_75_Hz = 0x00,                  /*!< Output Data Rate = 0.75 Hz */
  LSM303DLHC_COMP_ODR_1_5_Hz  = 0x04,                  /*!< Output Data Rate = 1.5 Hz */
  LSM303DLHC_COMP_ODR_3_0_Hz  = 0x08,                  /*!< Output Data Rate = 3 Hz */
  LSM303DLHC_COMP_ODR_7_5_Hz  = 0x0C,                  /*!< Output Data Rate = 7.5 Hz */
  LSM303DLHC_COMP_ODR_15_Hz   = 0x10,                  /*!< Output Data Rate = 15 Hz */
  LSM303DLHC_COMP_ODR_30_Hz   = 0x14,                  /*!< Output Data Rate = 30 Hz */
  LSM303DLHC_COMP_ODR_75_Hz   = 0x18,                  /*!< Output Data Rate = 75 Hz */
  LSM303DLHC_COMP_ODR_220_Hz  = 0x1C                   /*!< Output Data Rate = 220 Hz */
}LSM303DLHC_COMP_ODR_t;


/**
 * @brief  Compass Full Scale
 */
typedef enum
{
  LSM303DLHC_COMP_FS_1_3_GA  = 0x20,                   /*!< Full scale = ±1.3 Gauss */
  LSM303DLHC_COMP_FS_1_9_GA  = 0x40,                   /*!< Full scale = ±1.9 Gauss */
  LSM303DLHC_COMP_FS_2_5_GA  = 0x60,                   /*!< Full scale = ±2.5 Gauss */
  LSM303DLHC_COMP_FS_4_0_GA  = 0x80,                   /*!< Full scale = ±4.0 Gauss */
  LSM303DLHC_COMP_FS_4_7_GA  = 0xA0,                   /*!< Full scale = ±4.7 Gauss */
  LSM303DLHC_COMP_FS_5_6_GA  = 0xC0,                   /*!< Full scale = ±5.6 Gauss */
  LSM303DLHC_COMP_FS_8_1_GA  = 0xE0                    /*!< Full scale = ±8.1 Gauss */
}LSM303DLHC_COMP_FS_t;


/**
 * @brief  Compass Working Mode
 */
typedef enum
{
  LSM303DLHC_COMP_WM_CONTINUOS = 0x00,                 /*!< Continuous-Conversion Mode */
  LSM303DLHC_COMP_WM_BLOCKED   = 0x01,                 /*!< Single-Conversion Mode */
  LSM303DLHC_COMP_WM_SLEEP     = 0x02                  /*!< Sleep Mode */
}LSM303DLHC_COMP_WM_t;

/**
 * @brief   Compass configuration structure.
 */
typedef struct {
  /**
   * @brief Compass fullscale value.
   */
  LSM303DLHC_COMP_FS_t    fullscale;
  /**
   * @brief Compass output data rate selection.
   */
  LSM303DLHC_COMP_ODR_t   outputdatarate;
  /**
   * @brief Compass working mode.
   */
  LSM303DLHC_COMP_WM_t    workingmode;
} LSM303DLHC_COMP_Config;
/** @}  */
/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

  uint8_t lsm303dlhcReadRegister(I2CDriver *i2cp, uint8_t sad, uint8_t sub,
                                 msg_t* message);
  void lsm303dlhcWriteRegister(I2CDriver *i2cp,uint8_t sad, uint8_t sub,
                                 uint8_t value, msg_t* message);

#ifdef __cplusplus
}
#endif
#endif /* _LSM303DLHC_H_ */
/** @} */

