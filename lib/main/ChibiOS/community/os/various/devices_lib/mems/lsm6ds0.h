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
 * @file    lsm6ds0.h
 * @brief   LSM6DS0 MEMS interface module header.
 *
 * @{
 */

#ifndef _LSM6DS0_H_
#define _LSM6DS0_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define  LSM6DS0_ACC_SENS_2G                     ((float)1671.095f)         /*!< Accelerometer sensitivity with 2 G full scale [LSB * s^2 / m] */
#define  LSM6DS0_ACC_SENS_4G                     ((float)835.547f)          /*!< Accelerometer sensitivity with 4 G full scale [LSB * s^2 / m] */
#define  LSM6DS0_ACC_SENS_8G                     ((float)417.774)           /*!< Accelerometer sensitivity with 8 G full scale [LSB * s^2 / m] */
#define  LSM6DS0_ACC_SENS_16G                    ((float)139.258f)          /*!< Accelerometer sensitivity with 16 G full scale [LSB * s^2 / m] */

#define  LSM6DS0_GYRO_SENS_245DPS                ((float)114.286f)          /*!< Gyroscope sensitivity with 245 dps full scale [LSB * s / °] */
#define  LSM6DS0_GYRO_SENS_500DPS                ((float)57.143f)           /*!< Gyroscope sensitivity with 500 dps full scale [LSB * s / °] */
#define  LSM6DS0_GYRO_SENS_2000DPS               ((float)14.286f)           /*!< Gyroscope sensitivity with 2000 dps full scale [LSB * s / °] */
/**
 * @name    LSM6DS0 register names
 * @{
 */
/******************************************************************************/
/*                                                                            */
/*                        LSM6DS0 on board MEMS                               */
/*                                                                            */
/******************************************************************************/
/*****************  Bit definition for I2C/SPI communication  *****************/
#define  LSM6DS0_SUB                             ((uint8_t)0x7F)            /*!< SUB[6:0] Sub-registers address Mask */
#define  LSM6DS0_SUB_0                           ((uint8_t)0x01)            /*!< bit 0 */
#define  LSM6DS0_SUB_1                           ((uint8_t)0x02)            /*!< bit 1 */
#define  LSM6DS0_SUB_2                           ((uint8_t)0x08)            /*!< bit 3 */
#define  LSM6DS0_SUB_4                           ((uint8_t)0x10)            /*!< bit 4 */
#define  LSM6DS0_SUB_5                           ((uint8_t)0x20)            /*!< bit 5 */
#define  LSM6DS0_SUB_6                           ((uint8_t)0x40)            /*!< bit 6 */

#define  LSM6DS0_SUB_MSB                         ((uint8_t)0x80)            /*!< Multiple data read\write bit */

/*****************  Bit definition for Registers Addresses  *******************/
#define  LSM6DS0_SUB_ACT_THS                     ((uint8_t)0x04)            /*!< Activity threshold register */
#define  LSM6DS0_SUB_ACT_DUR                     ((uint8_t)0x05)            /*!< Inactivity duration register */
#define  LSM6DS0_SUB_INT_GEN_CFG_XL              ((uint8_t)0x06)            /*!< Accelerometer interrupt generator configuration register */
#define  LSM6DS0_SUB_INT_GEN_THS_X_XL            ((uint8_t)0x07)            /*!< Accelerometer X-axis interrupt threshold register */
#define  LSM6DS0_SUB_INT_GEN_THS_Y_XL            ((uint8_t)0x08)            /*!< Accelerometer Y-axis interrupt threshold register */
#define  LSM6DS0_SUB_INT_GEN_THS_Z_XL            ((uint8_t)0x09)            /*!< Accelerometer Z-axis interrupt threshold register */
#define  LSM6DS0_SUB_INT_GEN_DUR_XL              ((uint8_t)0x0A)            /*!< Accelerometer interrupt duration register */
#define  LSM6DS0_SUB_REFERENCE_G                 ((uint8_t)0x0B)            /*!< Gyroscope reference value register for digital high-pass filter */
#define  LSM6DS0_SUB_INT_CTRL                    ((uint8_t)0x0C)            /*!< INT pin control register */
#define  LSM6DS0_SUB_WHO_AM_I                    ((uint8_t)0x0F)            /*!< Who_AM_I register */
#define  LSM6DS0_SUB_CTRL_REG1_G                 ((uint8_t)0x10)            /*!< Gyroscope control register 1 */
#define  LSM6DS0_SUB_CTRL_REG2_G                 ((uint8_t)0x11)            /*!< Gyroscope control register 2 */
#define  LSM6DS0_SUB_CTRL_REG3_G                 ((uint8_t)0x12)            /*!< Gyroscope control register 3 */
#define  LSM6DS0_SUB_ORIENT_CFG_G                ((uint8_t)0x13)            /*!< Gyroscope sign and orientation register */
#define  LSM6DS0_SUB_INT_GEN_SRC_G               ((uint8_t)0x14)            /*!< Gyroscope interrupt source register */
#define  LSM6DS0_SUB_OUT_TEMP_L                  ((uint8_t)0x15)            /*!< Temperature data output low register */
#define  LSM6DS0_SUB_OUT_TEMP_H                  ((uint8_t)0x16)            /*!< Temperature data output high register */
#define  LSM6DS0_SUB_STATUS_REG1                 ((uint8_t)0x17)            /*!< Status register 1 */
#define  LSM6DS0_SUB_OUT_X_L_G                   ((uint8_t)0x18)            /*!< Gyroscope X-axis low output register */
#define  LSM6DS0_SUB_OUT_X_H_G                   ((uint8_t)0x19)            /*!< Gyroscope X-axis high output register */
#define  LSM6DS0_SUB_OUT_Y_L_G                   ((uint8_t)0x1A)            /*!< Gyroscope Y-axis low output register */
#define  LSM6DS0_SUB_OUT_Y_H_G                   ((uint8_t)0x1B)            /*!< Gyroscope Y-axis high output register */
#define  LSM6DS0_SUB_OUT_Z_L_G                   ((uint8_t)0x1C)            /*!< Gyroscope Z-axis low output register */
#define  LSM6DS0_SUB_OUT_Z_H_G                   ((uint8_t)0x1D)            /*!< Gyroscope Z-axis high output register */
#define  LSM6DS0_SUB_CTRL_REG4                   ((uint8_t)0x1E)            /*!< Control register 4 */
#define  LSM6DS0_SUB_CTRL_REG5_XL                ((uint8_t)0x1F)            /*!< Accelerometer Control Register 5 */
#define  LSM6DS0_SUB_CTRL_REG6_XL                ((uint8_t)0x20)            /*!< Accelerometer Control Register 6 */
#define  LSM6DS0_SUB_CTRL_REG7_XL                ((uint8_t)0x21)            /*!< Accelerometer Control Register 7 */
#define  LSM6DS0_SUB_CTRL_REG8                   ((uint8_t)0x22)            /*!< Control register 8 */
#define  LSM6DS0_SUB_CTRL_REG9                   ((uint8_t)0x23)            /*!< Control register 9 */
#define  LSM6DS0_SUB_CTRL_REG10                  ((uint8_t)0x24)            /*!< Control register 10 */
#define  LSM6DS0_SUB_INT_GEN_SRC_XL              ((uint8_t)0x26)            /*!< Accelerometer interrupt source register */
#define  LSM6DS0_SUB_STATUS_REG2                 ((uint8_t)0x27)            /*!< Status register */
#define  LSM6DS0_SUB_OUT_X_L_XL                  ((uint8_t)0x28)            /*!< Accelerometer X-axis low output register */
#define  LSM6DS0_SUB_OUT_X_H_XL                  ((uint8_t)0x29)            /*!< Accelerometer X-axis high output register */
#define  LSM6DS0_SUB_OUT_Y_L_XL                  ((uint8_t)0x2A)            /*!< Accelerometer Y-axis low output register */
#define  LSM6DS0_SUB_OUT_Y_H_XL                  ((uint8_t)0x2B)            /*!< Accelerometer Y-axis high output register */
#define  LSM6DS0_SUB_OUT_Z_L_XL                  ((uint8_t)0x2C)            /*!< Accelerometer Z-axis low output register */
#define  LSM6DS0_SUB_OUT_Z_H_XL                  ((uint8_t)0x2D)            /*!< Accelerometer Z-axis high output register */
#define  LSM6DS0_SUB_FIFO_CTRL                   ((uint8_t)0x2E)            /*!< FIFO control register */
#define  LSM6DS0_SUB_FIFO_SRC                    ((uint8_t)0x2F)            /*!< FIFO status control register */
#define  LSM6DS0_SUB_INT_GEN_CFG_G               ((uint8_t)0x30)            /*!< Gyroscope interrupt generator configuration register */
#define  LSM6DS0_SUB_INT_GEN_THS_XH_G            ((uint8_t)0x31)            /*!< Gyroscope X-axis low interrupt generator threshold registers */
#define  LSM6DS0_SUB_INT_GEN_THS_XL_G            ((uint8_t)0x32)            /*!< Gyroscope X-axis high interrupt generator threshold registers  */
#define  LSM6DS0_SUB_INT_GEN_THS_YH_G            ((uint8_t)0x33)            /*!< Gyroscope Y-axis low interrupt generator threshold registers */
#define  LSM6DS0_SUB_INT_GEN_THS_YL_G            ((uint8_t)0x34)            /*!< Gyroscope Y-axis high interrupt generator threshold registers */
#define  LSM6DS0_SUB_INT_GEN_THS_ZH_G            ((uint8_t)0x35)            /*!< Gyroscope Z-axis low interrupt generator threshold registers */
#define  LSM6DS0_SUB_INT_GEN_THS_ZL_G            ((uint8_t)0x36)            /*!< Gyroscope Z-axis high interrupt generator threshold registers */
#define  LSM6DS0_SUB_INT_GEN_DUR_G               ((uint8_t)0x37)            /*!< Gyroscope interrupt generator duration register */

/** @} */

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
 * @name    Generic LSM6DS0 data structures and types
 * @{
 */

/**
 * @brief  Accelerometer and Gyroscope Slave Address
 */
typedef enum {
  LSM6DS0_SAD_GND = 0x6A,                             /*!< LSM6DS0 Slave Address when SA1 is to GND */
  LSM6DS0_SAD_VCC = 0x6B                              /*!< LSM6DS0 Slave Address when SA1 is to VCC */
}LSM6DS0_SAD_t;

/**
 * @brief  Accelerometer and Gyroscope Block Data Update
 */
typedef enum
{
  LSM6DS0_BDU_CONTINOUS = 0x00,                       /*!< Continuos Update */
  LSM6DS0_BDU_BLOCKED   = 0x40                        /*!< Single Update: output registers not updated until MSB and LSB reading */
}LSM6DS0_BDU_t;

/**
 * @brief  Accelerometer and Gyroscope Endianness
 */
typedef enum
{
  LSM6DS0_END_LITTLE = 0x00,                          /*!< Little Endian: data LSB @ lower address */
  LSM6DS0_END_BIG    = 0x20                           /*!< Big Endian: data MSB @ lower address */
}LSM6DS0_END_t;
/** @}  */

/**
 * @name    Accelerometer data structures and types
 * @{
 */

/**
 * @brief  Accelerometer Decimation Mode
 */
typedef enum {
  LSM6DS0_ACC_DEC_DISABLED = 0x00,                    /*!< NO decimation */
  LSM6DS0_ACC_DEC_X2       = 0x40,                    /*!< Decimation update every 2 sample */
  LSM6DS0_ACC_DEC_X4       = 0x80,                    /*!< Decimation update every 4 sample */
  LSM6DS0_ACC_DEC_X8       = 0xC0                     /*!< Decimation update every 8 sample */
}LSM6DS0_ACC_DEC_t;

/**
 * @brief   Accelerometer Axes Enabling
 */
typedef enum{
  LSM6DS0_ACC_AE_DISABLED = 0x00,                     /*!< Axes all disabled */
  LSM6DS0_ACC_AE_X        = 0x08,                     /*!< Only X-axis enabled */
  LSM6DS0_ACC_AE_Y        = 0x10,                     /*!< Only Y-axis enabled */
  LSM6DS0_ACC_AE_XY       = 0x18,                     /*!< X & Y axes enabled */
  LSM6DS0_ACC_AE_Z        = 0x20,                     /*!< Only Z-axis enabled */
  LSM6DS0_ACC_AE_XZ       = 0x28,                     /*!< X & Z axes enabled  */
  LSM6DS0_ACC_AE_YZ       = 0x30,                     /*!< Y & Z axes enabled  */
  LSM6DS0_ACC_AE_XYZ      = 0x38                      /*!< All axes enabled */
}LSM6DS0_ACC_AE_t;

/**
 * @brief  Accelerometer Output Data Rate
 */
typedef enum {
  LSM6DS0_ACC_ODR_PD    = 0x00,                       /*!< Power down */
  LSM6DS0_ACC_ODR_10Hz  = 0x20,                       /*!< Output Data Rate = 10 Hz */
  LSM6DS0_ACC_ODR_50Hz  = 0x40,                       /*!< Output Data Rate = 50 Hz */
  LSM6DS0_ACC_ODR_119Hz = 0x60,                       /*!< Output Data Rate = 119 Hz */
  LSM6DS0_ACC_ODR_238Hz = 0x80,                       /*!< Output Data Rate = 238 Hz */
  LSM6DS0_ACC_ODR_476Hz = 0xA0,                       /*!< Output Data Rate = 476 Hz */
  LSM6DS0_ACC_ODR_952Hz = 0xC0                        /*!< Output Data Rate = 952 Hz */
}LSM6DS0_ACC_ODR_t;

/**
 * @brief  Accelerometer Full Scale
 */
typedef enum {
  LSM6DS0_ACC_FS_2G  = 0x00,                          /*!< ±2 g m/s^2 */
  LSM6DS0_ACC_FS_4G  = 0x10,                          /*!< ±4 g m/s^2 */
  LSM6DS0_ACC_FS_8G  = 0x18,                          /*!< ±8 g m/s^2 */
  LSM6DS0_ACC_FS_16G = 0x08                           /*!< ±16 g m/s^2 */
}LSM6DS0_ACC_FS_t;

/**
 * @brief  Accelerometer Antialiasing filter Bandwidth Selection
 */
typedef enum {
  LSM6DS0_ACC_BW_408Hz    = 0x00,                     /*!< AA filter bandwidth = 408 Hz  */
  LSM6DS0_ACC_BW_211Hz    = 0x01,                     /*!< AA filter bandwidth = 211 Hz */
  LSM6DS0_ACC_BW_105Hz    = 0x02,                     /*!< AA filter bandwidth = 105 Hz */
  LSM6DS0_ACC_BW_50Hz     = 0x03,                     /*!< AA filter bandwidth = 50 Hz */
  LSM6DS0_ACC_BW_ACCORDED = 0x04,                     /*!< AA filter bandwidth chosen by ODR selection */
}LSM6DS0_ACC_BW_t;

/**
 * @brief  Accelerometer High Resolution mode
 */
typedef enum
{
  LSM6DS0_ACC_HR_Disabled = 0x00,                     /*!< High resolution output mode disabled, FDS bypassed */
  LSM6DS0_ACC_HR_EN_9     = 0xC4,                     /*!< High resolution output mode enabled, LP cutoff = ODR/9, FDS enabled */
  LSM6DS0_ACC_HR_EN_50    = 0x84,                     /*!< High resolution output mode enabled, LP cutoff = ODR/50, FDS enabled */
  LSM6DS0_ACC_HR_EN_100   = 0xA4,                     /*!< High resolution output mode enabled, LP cutoff = ODR/100, FDS enabled */
  LSM6DS0_ACC_HR_EN_400   = 0xE4,                     /*!< High resolution output mode enabled, LP cutoff = ODR/400, FDS enabled */
}LSM6DS0_ACC_HR_t;

/**
 * @brief  HP filter for interrupt
 */
typedef enum
{
  LSM6DS0_ACC_HPIS1_BYPASSED = 0x00,                  /*!< High-pass filter bypassed */
  LSM6DS0_ACC_HPIS1_ENABLED  = 0x01                   /*!< High-pass filter enabled for accelerometer interrupt function on interrupt */
}LSM6DS0_ACC_HPIS1_t;

/**
 * @brief   Accelerometer configuration structure.
 */
typedef struct {

  /**
   * @brief  LSM6DS0 Slave Address
   */
  LSM6DS0_SAD_t       slaveaddress;
  /**
   * @brief  Accelerometer Decimation Mode
   */
  LSM6DS0_ACC_DEC_t   decimation;
  /**
   * @brief  Accelerometer Output Data Rate
   */
  LSM6DS0_ACC_ODR_t   outputdatarate;
  /**
   * @brief  Accelerometer Antialiasing filter Bandwidth Selection
   */
  LSM6DS0_ACC_BW_t    bandwidth;
  /**
   * @brief  Accelerometer Full Scale
   */
  LSM6DS0_ACC_FS_t    fullscale;
  /**
   * @brief   Accelerometer Axes Enabling
   */
  LSM6DS0_ACC_AE_t    axesenabling;
  /**
   * @brief  Accelerometer High Resolution mode
   */
  LSM6DS0_ACC_HR_t    highresmode;
  /**
   * @brief  HP filter for interrupt
   */
  LSM6DS0_ACC_HPIS1_t hpfirq;
  /**
   * @brief  LSM6DS0 Endianness
   */
  LSM6DS0_END_t       endianess;
  /**
   * @brief  LSM6DS0 Block Data Update
   */
  LSM6DS0_BDU_t       blockdataupdate;
} LSM6DS0_ACC_Config;
/** @}  */

/**
 * @name    Gyroscope data structures and types
 * @{
 */

/**
 * @brief  Gyroscope Output Data Rate
 */
typedef enum {
  LSM6DS0_GYRO_ODR_PD             = 0x00,             /*!< Power down */
  LSM6DS0_GYRO_ODR_14_9Hz_CO_5Hz  = 0x20,             /*!< Output Data Rate = 14.9 Hz, CutOff = 5Hz */
  LSM6DS0_GYRO_ODR_59_5Hz_CO_16Hz = 0x40,             /*!< Output Data Rate = 59.5 Hz, CutOff = 16Hz */
  LSM6DS0_GYRO_ODR_119Hz_CO_14Hz  = 0x60,             /*!< Output Data Rate = 119 Hz, CutOff = 14Hz */
  LSM6DS0_GYRO_ODR_119Hz_CO_31Hz  = 0x61,             /*!< Output Data Rate = 119 Hz, CutOff = 31Hz */
  LSM6DS0_GYRO_ODR_238Hz_CO_14Hz  = 0x80,             /*!< Output Data Rate = 238 Hz, CutOff = 14Hz */
  LSM6DS0_GYRO_ODR_238Hz_CO_29Hz  = 0x81,             /*!< Output Data Rate = 328 Hz, CutOff = 29Hz */
  LSM6DS0_GYRO_ODR_238Hz_CO_63Hz  = 0x82,             /*!< Output Data Rate = 238 Hz, CutOff = 63Hz */
  LSM6DS0_GYRO_ODR_238Hz_CO_78Hz  = 0x83,             /*!< Output Data Rate = 476 Hz, CutOff = 78Hz */
  LSM6DS0_GYRO_ODR_476Hz_CO_21Hz  = 0xA0,             /*!< Output Data Rate = 476 Hz, CutOff = 21Hz */
  LSM6DS0_GYRO_ODR_476Hz_CO_28Hz  = 0xA1,             /*!< Output Data Rate = 238 Hz, CutOff = 28Hz */
  LSM6DS0_GYRO_ODR_476Hz_CO_57Hz  = 0xA2,             /*!< Output Data Rate = 476 Hz, CutOff = 57Hz */
  LSM6DS0_GYRO_ODR_476Hz_CO_100Hz = 0xA3,             /*!< Output Data Rate = 476 Hz, CutOff = 100Hz */
  LSM6DS0_GYRO_ODR_952Hz_CO_33Hz  = 0xC0,             /*!< Output Data Rate = 952 Hz, CutOff = 33Hz */
  LSM6DS0_GYRO_ODR_952Hz_CO_40Hz  = 0xC1,             /*!< Output Data Rate = 952 Hz, CutOff = 40Hz */
  LSM6DS0_GYRO_ODR_952Hz_CO_58Hz  = 0xC2,             /*!< Output Data Rate = 952 Hz, CutOff = 58Hz */
  LSM6DS0_GYRO_ODR_952Hz_CO_100Hz = 0xC3              /*!< Output Data Rate = 952 Hz, CutOff = 100Hz */
}LSM6DS0_GYRO_ODR_t;

/**
 * @brief  Gyroscope Full Scale
 */
typedef enum {
  LSM6DS0_GYRO_FS_245DSP  = 0x00,                     /*!< ±245 degrees per second */
  LSM6DS0_GYRO_FS_500DSP  = 0x08,                     /*!< ±500 degrees per second */
  LSM6DS0_GYRO_FS_2000DSP = 0x18                      /*!< ±2000 degrees per second */
}LSM6DS0_GYRO_FS_t;

/**
 * @brief  Gyroscope Output Selection
 */
typedef enum {
  LSM6DS0_GYRO_OUT_SEL_BYPASS    = 0x00,              /*!< Output not filtered */
  LSM6DS0_GYRO_OUT_SEL_FILTERED  = 0x01,              /*!< Output filtered */
}LSM6DS0_GYRO_OUT_SEL_t;

/**
 * @brief  Gyroscope Interrupt Selection
 */
typedef enum {
  LSM6DS0_GYRO_INT_SEL_BYPASS    = 0x00,              /*!< Interrupt generator signal not filtered */
  LSM6DS0_GYRO_INT_SEL_FILTERED  = 0x08,              /*!< Interrupt generator signal filtered */
}LSM6DS0_GYRO_INT_SEL_t;

/**
 * @brief  Gyroscope Low Power Mode
 */
typedef enum {
  LSM6DS0_GYRO_LP_MODE_HIGH_PERFORMANCE = 0x00,       /*!< High performance */
  LSM6DS0_GYRO_LP_MODE_LOW_POWER        = 0x80,       /*!< Low power */
}LSM6DS0_GYRO_LP_MODE_t;

/**
 * @brief  Gyroscope High Pass Filter Cutoff Selection
 */
typedef enum {
  LSM6DS0_GYRO_HPCF_DISABLED = 0x00,                  /*!< HP filter disabled  */
  LSM6DS0_GYRO_HPCF_0        = 0x40,                  /*!< Config 0 refer to table 48 of DOcID025604 Rev 3  */
  LSM6DS0_GYRO_HPCF_1        = 0x41,                  /*!< Config 1 refer to table 48 of DOcID025604 Rev 3  */
  LSM6DS0_GYRO_HPCF_2        = 0x42,                  /*!< Config 2 refer to table 48 of DOcID025604 Rev 3  */
  LSM6DS0_GYRO_HPCF_3        = 0x43,                  /*!< Config 3 refer to table 48 of DOcID025604 Rev 3  */
  LSM6DS0_GYRO_HPCF_4        = 0x44,                  /*!< Config 4 refer to table 48 of DOcID025604 Rev 3  */
  LSM6DS0_GYRO_HPCF_5        = 0x45,                  /*!< Config 5 refer to table 48 of DOcID025604 Rev 3  */
  LSM6DS0_GYRO_HPCF_6        = 0x46,                  /*!< Config 6 refer to table 48 of DOcID025604 Rev 3  */
  LSM6DS0_GYRO_HPCF_7        = 0x47,                  /*!< Config 7 refer to table 48 of DOcID025604 Rev 3  */
  LSM6DS0_GYRO_HPCF_8        = 0x48,                  /*!< Config 8 refer to table 48 of DOcID025604 Rev 3  */
  LSM6DS0_GYRO_HPCF_9        = 0x49,                  /*!< Config 9 refer to table 48 of DOcID025604 Rev 3  */
  LSM6DS0_GYRO_HPCF_10       = 0x4A                   /*!< Config 10 refer to table 48 of DOcID025604 Rev 3  */
}LSM6DS0_GYRO_HPCF_t;

/**
 * @brief   Gyroscope Axes Enabling
 */
typedef enum{
  LSM6DS0_GYRO_AE_DISABLED = 0x00,                    /*!< Axes all disabled */
  LSM6DS0_GYRO_AE_X        = 0x08,                    /*!< Only X-axis enabled */
  LSM6DS0_GYRO_AE_Y        = 0x10,                    /*!< Only Y-axis enabled */
  LSM6DS0_GYRO_AE_XY       = 0x18,                    /*!< X & Y axes enabled */
  LSM6DS0_GYRO_AE_Z        = 0x20,                    /*!< Only Z-axis enabled */
  LSM6DS0_GYRO_AE_XZ       = 0x28,                    /*!< X & Z axes enabled  */
  LSM6DS0_GYRO_AE_YZ       = 0x30,                    /*!< Y & Z axes enabled  */
  LSM6DS0_GYRO_AE_XYZ      = 0x38                     /*!< All axes enabled */
}LSM6DS0_GYRO_AE_t;

/**
 * @brief  Gyroscope Decimation Mode
 */
typedef enum {
  LSM6DS0_GYRO_DEC_DISABLED = 0x00,                   /*!< NO decimation */
  LSM6DS0_GYRO_DEC_X2       = 0x40,                   /*!< Decimation update every 2 sample */
  LSM6DS0_GYRO_DEC_X4       = 0x80,                   /*!< Decimation update every 4 sample */
  LSM6DS0_GYRO_DEC_X8       = 0xC0                    /*!< Decimation update every 8 sample */
}LSM6DS0_GYRO_DEC_t;

/**
 * @brief  Gyroscope Sleep Mode
 */
typedef enum {
  LSM6DS0_GYRO_SLP_DISABLED = 0x00,                   /*!< Gyroscope sleep mode disabled */
  LSM6DS0_GYRO_SLP_ENABLED  = 0x40                    /*!< Gyroscope sleep mode enabled */
}LSM6DS0_GYRO_SLP_t;
/**
 * @brief   Gyroscope configuration structure.
 */
typedef struct {
  /**
   * @brief  LSM6DS0 Slave Address
   */
  LSM6DS0_SAD_t   slaveaddress;
  /**
   * @brief  Gyroscope Output Data Rate
   */
  LSM6DS0_GYRO_ODR_t   outputdatarate;
  /**
   * @brief  Gyroscope Full Scale
   */
  LSM6DS0_GYRO_FS_t    fullscale;
  /**
   * @brief  Gyroscope Output Selection
   */
  LSM6DS0_GYRO_OUT_SEL_t outputselect;
  /**
   * @brief  Gyroscope Interrupt Selection
   */
  LSM6DS0_GYRO_INT_SEL_t irqselect;
  /**
   * @brief  Gyroscope Low Power Mode
   */
  LSM6DS0_GYRO_LP_MODE_t lowpowermode;
  /**
   * @brief  Gyroscope High Pass Filter Cutoff Selection
   */
  LSM6DS0_GYRO_HPCF_t HPCfrequency;
  /**
   * @brief   Gyroscope Axes Enabling
   */
  LSM6DS0_GYRO_AE_t axesenabling;
  /**
   * @brief  Gyroscope Decimation Mode
   */
  LSM6DS0_GYRO_DEC_t decimation;
  /**
   * @brief  LSM6DS0 Endianness
   */
  LSM6DS0_END_t   endianess;
  /**
   * @brief  LSM6DS0 Block Data Update
   */
  LSM6DS0_BDU_t   blockdataupdate;
} LSM6DS0_GYRO_Config;
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

  uint8_t lsm6ds0ReadRegister(I2CDriver *i2cp, uint8_t sad, uint8_t sub,
                                 msg_t* message);
  void lsm6ds0WriteRegister(I2CDriver *i2cp, uint8_t sad, uint8_t sub,
                                 uint8_t value, msg_t* message);
#ifdef __cplusplus
}
#endif

#endif /* _LSM6DS0_H_ */

/** @} */
