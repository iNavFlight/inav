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
 * @file    l3gd20.h
 * @brief   L3GD20 MEMS interface module header.
 *
 * @{
 */

#ifndef _L3GD20_H_
#define _L3GD20_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define  L3GD20_SENS_250DPS                      ((float)131.072f)          /*!< gyroscope sensitivity with 250 dps full scale [LSB/dps] */
#define  L3GD20_SENS_500DPS                      ((float)65.536f)           /*!< gyroscope sensitivity with 500 dps full scale [LSB/dps] */
#define  L3GD20_SENS_2000DPS                     ((float)16.384f)           /*!< gyroscope sensitivity with 2000 dps full scale [LSB/dps] */
/**
 * @name    L3GD20 register names
 * @{
 */
/******************************************************************************/
/*                                                                            */
/*                        L3GD20 on board MEMS                                */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for SPI communication  *******************/
#define  L3GD20_DI                               ((uint8_t)0xFF)            /*!< DI[7:0] Data input */
#define  L3GD20_DI_0                             ((uint8_t)0x01)            /*!< bit 0 */
#define  L3GD20_DI_1                             ((uint8_t)0x02)            /*!< bit 1 */
#define  L3GD20_DI_2                             ((uint8_t)0x04)            /*!< bit 2 */
#define  L3GD20_DI_3                             ((uint8_t)0x08)            /*!< bit 3 */
#define  L3GD20_DI_4                             ((uint8_t)0x10)            /*!< bit 4 */
#define  L3GD20_DI_5                             ((uint8_t)0x20)            /*!< bit 5 */
#define  L3GD20_DI_6                             ((uint8_t)0x40)            /*!< bit 6 */
#define  L3GD20_DI_7                             ((uint8_t)0x80)            /*!< bit 7 */

#define  L3GD20_AD                               ((uint8_t)0x3F)            /*!< AD[5:0] Address Data */
#define  L3GD20_AD_0                             ((uint8_t)0x01)            /*!< bit 0 */
#define  L3GD20_AD_1                             ((uint8_t)0x02)            /*!< bit 1 */
#define  L3GD20_AD_2                             ((uint8_t)0x04)            /*!< bit 2 */
#define  L3GD20_AD_3                             ((uint8_t)0x08)            /*!< bit 3 */
#define  L3GD20_AD_4                             ((uint8_t)0x10)            /*!< bit 4 */
#define  L3GD20_AD_5                             ((uint8_t)0x20)            /*!< bit 5 */

#define  L3GD20_MS                               ((uint8_t)0x40)            /*!< Multiple read write */
#define  L3GD20_RW                               ((uint8_t)0x80)            /*!< Read Write, 1 0 */

/******************  Bit definition for Registers Addresses *******************/
#define  L3GD20_AD_WHO_AM_I                      ((uint8_t)0x0F)            /*!< WHO I AM */
#define  L3GD20_AD_CTRL_REG1                     ((uint8_t)0x20)            /*!< CONTROL REGISTER 1 */
#define  L3GD20_AD_CTRL_REG2                     ((uint8_t)0x21)            /*!< CONTROL REGISTER 2 */
#define  L3GD20_AD_CTRL_REG3                     ((uint8_t)0x22)            /*!< CONTROL REGISTER 3 */
#define  L3GD20_AD_CTRL_REG4                     ((uint8_t)0x23)            /*!< CONTROL REGISTER 4 */
#define  L3GD20_AD_CTRL_REG5                     ((uint8_t)0x24)            /*!< CONTROL REGISTER 5 */
#define  L3GD20_AD_REFERENCE                     ((uint8_t)0x25)            /*!< REFERENCE/DATACAPTURE */
#define  L3GD20_AD_OUT_TEMP                      ((uint8_t)0x26)            /*!< MEMS ONBOARD TEMP SENSOR */
#define  L3GD20_AD_STATUS_REG                    ((uint8_t)0x27)            /*!< STATUS REGISTER */
#define  L3GD20_AD_OUT_X_L                       ((uint8_t)0x28)            /*!< OUTPUT X-AXIS LOW */
#define  L3GD20_AD_OUT_X_H                       ((uint8_t)0x29)            /*!< OUTPUT X-AXIS HIGH */
#define  L3GD20_AD_OUT_Y_L                       ((uint8_t)0x2A)            /*!< OUTPUT Y-AXIS LOW */
#define  L3GD20_AD_OUT_Y_H                       ((uint8_t)0x2B)            /*!< OUTPUT Y-AXIS HIGH */
#define  L3GD20_AD_OUT_Z_L                       ((uint8_t)0x2C)            /*!< OUTPUT Z-AXIS LOW */
#define  L3GD20_AD_OUT_Z_H                       ((uint8_t)0x2D)            /*!< OUTPUT Z-AXIS HIGH */
#define  L3GD20_AD_FIFO_CTRL_REG                 ((uint8_t)0x2E)            /*!< FIFO CONTROL REGISTER */
#define  L3GD20_AD_FIFO_SRC_REG                  ((uint8_t)0x2F)            /*!< FIFO SOURCE REGISTER */
#define  L3GD20_AD_INT1_CFG                      ((uint8_t)0x30)            /*!< INTERRUPT1 CONFIG REGISTER */
#define  L3GD20_AD_INT1_SRC                      ((uint8_t)0x31)            /*!< INTERRUPT1 SOURCE REGISTER */
#define  L3GD20_AD_INT1_TSH_XH                   ((uint8_t)0x32)            /*!< INTERRUPT1 THRESHOLD X-AXIS HIGH */
#define  L3GD20_AD_INT1_TSH_XL                   ((uint8_t)0x33)            /*!< INTERRUPT1 THRESHOLD X-AXIS LOW */
#define  L3GD20_AD_INT1_TSH_YH                   ((uint8_t)0x34)            /*!< INTERRUPT1 THRESHOLD Y-AXIS HIGH */
#define  L3GD20_AD_INT1_TSH_YL                   ((uint8_t)0x35)            /*!< INTERRUPT1 THRESHOLD Y-AXIS LOW */
#define  L3GD20_AD_INT1_TSH_ZH                   ((uint8_t)0x36)            /*!< INTERRUPT1 THRESHOLD Z-AXIS HIGH */
#define  L3GD20_AD_INT1_TSH_ZL                   ((uint8_t)0x37)            /*!< INTERRUPT1 THRESHOLD Z-AXIS LOW */
#define  L3GD20_AD_INT1_DURATION                 ((uint8_t)0x38)            /*!< INTERRUPT1 DURATION */

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
 * @name    Gyroscope data structures and types
 * @{
 */

/**
 * @brief  Gyroscope Output Data Rate
 */
typedef enum {
  L3GD20_ODR_95Hz_Fc_12_5  = 0x00,          /*!< Output Data Rate = 95 Hz -  LPF Cut-Off = 12.5 Hz */
  L3GD20_ODR_95Hz_Fc_25    = 0x10,          /*!< Output Data Rate = 95 Hz -  LPF Cut-Off = 25 Hz */
  L3GD20_ODR_190Hz_Fc_12_5 = 0x40,          /*!< Output Data Rate = 190 Hz -  LPF Cut-Off = 12.5 Hz */
  L3GD20_ODR_190Hz_Fc_25   = 0x50,          /*!< Output Data Rate = 190 Hz -  LPF Cut-Off = 25 Hz */
  L3GD20_ODR_190Hz_Fc_50   = 0x60,          /*!< Output Data Rate = 190 Hz -  LPF Cut-Off = 50 Hz */
  L3GD20_ODR_190Hz_Fc_70   = 0x70,          /*!< Output Data Rate = 190 Hz -  LPF Cut-Off = 70 Hz */
  L3GD20_ODR_380Hz_Fc_20   = 0x80,          /*!< Output Data Rate = 380 Hz -  LPF Cut-Off = 20 Hz */
  L3GD20_ODR_380Hz_Fc_25   = 0x90,          /*!< Output Data Rate = 380 Hz -  LPF Cut-Off = 25 Hz */
  L3GD20_ODR_380Hz_Fc_50   = 0xA0,          /*!< Output Data Rate = 380 Hz -  LPF Cut-Off = 50 Hz */
  L3GD20_ODR_380Hz_Fc_100  = 0xB0,          /*!< Output Data Rate = 380 Hz -  LPF Cut-Off = 100 Hz */
  L3GD20_ODR_760Hz_Fc_30   = 0xC0,          /*!< Output Data Rate = 760 Hz -  LPF Cut-Off = 30 Hz */
  L3GD20_ODR_760Hz_Fc_35   = 0xD0,          /*!< Output Data Rate = 760 Hz -  LPF Cut-Off = 35 Hz */
  L3GD20_ODR_760Hz_Fc_50   = 0xE0,          /*!< Output Data Rate = 760 Hz -  LPF Cut-Off = 50 Hz */
  L3GD20_ODR_760Hz_Fc_100  = 0xF0           /*!< Output Data Rate = 760 Hz -  LPF Cut-Off = 100 Hz */
}L3GD20_ODR_t;

/**
 * @brief  Gyroscope Power Mode
 */
typedef enum {
  L3GD20_PM_POWER_DOWN   = 0x00,            /*!< Normal mode enabled */
  L3GD20_PM_SLEEP_NORMAL = 0x08             /*!< Low Power mode enabled */
}L3GD20_PM_t;

/**
 * @brief  Gyroscope Full Scale
 */
typedef enum {
  L3GD20_FS_250DPS  = 0x00,                 /*!< ±250 dps */
  L3GD20_FS_500DPS  = 0x10,                 /*!< ±500 dps */
  L3GD20_FS_2000DPS = 0x20                  /*!< ±200 dps */
}L3GD20_FS_t;

/**
 * @brief   Gyroscope Axes Enabling
 */
typedef enum {
  L3GD20_AE_DISABLED = 0x00,                /*!< All disabled */
  L3GD20_AE_X        = 0x01,                /*!< Only X */
  L3GD20_AE_Y        = 0x02,                /*!< Only Y */
  L3GD20_AE_XY       = 0x03,                /*!< X & Y */
  L3GD20_AE_Z        = 0x04,                /*!< Only Z */
  L3GD20_AE_XZ       = 0x05,                /*!< X & Z */
  L3GD20_AE_YZ       = 0x06,                /*!< Y & Z */
  L3GD20_AE_XYZ      = 0x07                 /*!< All enabled */
}L3GD20_AE_t;

/**
 * @brief  Gyroscope Block Data Update
 */
typedef enum {
  L3GD20_BDU_CONTINOUS = 0x00,              /*!< Continuos Update */
  L3GD20_BDU_BLOCKED   = 0x80               /*!< Single Update: output registers not updated until MSB and LSB reading */
}L3GD20_BDU_t;

/**
 * @brief  Gyroscope Endianness
 */
typedef enum {
  L3GD20_End_LITTLE = 0x00,                 /*!< Little Endian: data LSB @ lower address */
  L3GD20_End_BIG    = 0x40                  /*!< Big Endian: data MSB @ lower address */
}L3GD20_End_t;


/**
 * @brief   Gyroscope configuration structure.
 */
typedef struct {
  /**
   * @brief Gyroscope fullscale value.
   */
  L3GD20_FS_t    fullscale;
  /**
   * @brief Gyroscope power mode selection.
   */
  L3GD20_PM_t    powermode;
  /**
   * @brief Gyroscope output data rate selection.
   */
  L3GD20_ODR_t   outputdatarate;
  /**
   * @brief Gyroscope axes enabling.
   */
  L3GD20_AE_t    axesenabling;
  /**
   * @brief Gyroscope endianess.
   */
  L3GD20_End_t   endianess;
  /**
   * @brief Gyroscope block data update.
   */
  L3GD20_BDU_t   blockdataupdate;
} L3GD20_Config;
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

  uint8_t l3gd20ReadRegister(SPIDriver *spip, uint8_t reg);
  void l3gd20WriteRegister(SPIDriver *spip, uint8_t reg, uint8_t value);
#ifdef __cplusplus
}
#endif

#endif /* _L3GD20_H_ */

/** @} */

