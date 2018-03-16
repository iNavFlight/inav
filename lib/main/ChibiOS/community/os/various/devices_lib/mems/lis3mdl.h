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
 * @file    lis3mdl.h
 * @brief   LIS3MDL MEMS interface module header.
 *
 * @{
 */

#ifndef _LIS3MDL_H_
#define _LIS3MDL_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define  LIS3MDL_COMP_SENS_4GA           ((float)6842.0f)            /*!< compass sensitivity with 4 GA full scale [LSB  / Ga] */
#define  LIS3MDL_COMP_SENS_8GA           ((float)3421.0f)            /*!< compass sensitivity with 8 GA full scale [LSB  / Ga] */
#define  LIS3MDL_COMP_SENS_12GA          ((float)2281.0f)            /*!< compass sensitivity with 12 GA full scale [LSB  / Ga] */
#define  LIS3MDL_COMP_SENS_16GA          ((float)1711.0f)            /*!< compass sensitivity with 16 GA full scale [LSB  / Ga] */
/**
 * @name    LIS3MDL register names
 * @{
 */
/******************************************************************************/
/*                                                                            */
/*                        LIS3MDL on board MEMS                               */
/*                                                                            */
/******************************************************************************/
/*****************  Bit definition for I2C/SPI communication  *****************/
#define  LIS3MDL_SUB                             ((uint8_t)0x7F)            /*!< SUB[6:0] Sub-registers address Mask */
#define  LIS3MDL_SUB_0                           ((uint8_t)0x01)            /*!< bit 0 */
#define  LIS3MDL_SUB_1                           ((uint8_t)0x02)            /*!< bit 1 */
#define  LIS3MDL_SUB_2                           ((uint8_t)0x08)            /*!< bit 3 */
#define  LIS3MDL_SUB_4                           ((uint8_t)0x10)            /*!< bit 4 */
#define  LIS3MDL_SUB_5                           ((uint8_t)0x20)            /*!< bit 5 */
#define  LIS3MDL_SUB_6                           ((uint8_t)0x40)            /*!< bit 6 */

#define  LIS3MDL_SUB_MSB                         ((uint8_t)0x80)            /*!< Multiple data read\write bit */

/****************  Bit definition SUB-Registers Addresses  ********************/
#define  LIS3MDL_SUB_WHO_AM_I                    ((uint8_t)0x0F)            /*!< CONTROL REGISTER 1 */
#define  LIS3MDL_SUB_CTRL_REG1                   ((uint8_t)0x20)            /*!< CONTROL REGISTER 1 */
#define  LIS3MDL_SUB_CTRL_REG2                   ((uint8_t)0x21)            /*!< CONTROL REGISTER 2 */
#define  LIS3MDL_SUB_CTRL_REG3                   ((uint8_t)0x22)            /*!< CONTROL REGISTER 3 */
#define  LIS3MDL_SUB_CTRL_REG4                   ((uint8_t)0x23)            /*!< CONTROL REGISTER 4 */
#define  LIS3MDL_SUB_CTRL_REG5                   ((uint8_t)0x24)            /*!< CONTROL REGISTER 5 */
#define  LIS3MDL_SUB_STATUS_REG                  ((uint8_t)0x27)            /*!< STATUS REGISTER */
#define  LIS3MDL_SUB_OUT_X_L                     ((uint8_t)0x28)            /*!< OUTPUT X-AXIS LOW */
#define  LIS3MDL_SUB_OUT_X_H                     ((uint8_t)0x29)            /*!< OUTPUT X-AXIS HIGH */
#define  LIS3MDL_SUB_OUT_Y_L                     ((uint8_t)0x2A)            /*!< OUTPUT Y-AXIS LOW */
#define  LIS3MDL_SUB_OUT_Y_H                     ((uint8_t)0x2B)            /*!< OUTPUT Y-AXIS HIGH */
#define  LIS3MDL_SUB_OUT_Z_L                     ((uint8_t)0x2C)            /*!< OUTPUT Z-AXIS LOW */
#define  LIS3MDL_SUB_OUT_Z_H                     ((uint8_t)0x2D)            /*!< OUTPUT Z-AXIS HIGH */
#define  LIS3MDL_SUB_INT_CFG                     ((uint8_t)0x30)            /*!< INTERRUPT1 CONFIG */
#define  LIS3MDL_SUB_INT_SOURCE                  ((uint8_t)0x31)            /*!< INTERRUPT1 SOURCE */
#define  LIS3MDL_SUB_INT_THS_L                   ((uint8_t)0x32)            /*!< INTERRUPT1 THRESHOLD */
#define  LIS3MDL_SUB_INT_THS_H                   ((uint8_t)0x33)            /*!< INTERRUPT1 DURATION */

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
 * @name    Compass data structures and types
 * @{
 */

/**
 * @brief  Compass Slave Address
 */
typedef enum {
  LIS3MDL_SAD_GND = 0x1C,                             /*!< COMPASS Slave Address when SA1 is to GND */
  LIS3MDL_SAD_VCC = 0x1E                              /*!< COMPASS Slave Address when SA1 is to VCC */
}LIS3MDL_SAD_t;

/**
 * @brief  Compass Operation Mode for X and Y axes
 */
typedef enum {
  LIS3MDL_OMXY_LOW_POWER          = 0x00,             /*!< Operation Mode XY low power */
  LIS3MDL_OMXY_MEDIUM_PERFORMANCE = 0x20,             /*!< Operation Mode XY medium performance */
  LIS3MDL_OMXY_HIGH_PERFORMANCE   = 0x40,             /*!< Operation Mode XY high performance */
  LIS3MDL_OMXY_ULTRA_PERFORMANCE  = 0x60              /*!< Operation Mode XY ultra performance */
}LIS3MDL_OMXY_t;

/**
 * @brief  Compass Output Data Rate
 */
typedef enum {
  LIS3MDL_ODR_0_625Hz = 0x00,                         /*!< Output Data Rate = 0.625 Hz */
  LIS3MDL_ODR_1_25Hz  = 0x04,                         /*!< Output Data Rate = 1.25 Hz */
  LIS3MDL_ODR_2_5Hz   = 0x08,                         /*!< Output Data Rate = 2.5 Hz */
  LIS3MDL_ODR_5Hz     = 0x0C,                         /*!< Output Data Rate = 5 Hz */
  LIS3MDL_ODR_10Hz    = 0x10,                         /*!< Output Data Rate = 10 Hz */
  LIS3MDL_ODR_20Hz    = 0x14,                         /*!< Output Data Rate = 20 Hz */
  LIS3MDL_ODR_40Hz    = 0x18,                         /*!< Output Data Rate = 40 Hz */
  LIS3MDL_ODR_80Hz    = 0x1C                          /*!< Output Data Rate = 80 Hz */
}LIS3MDL_ODR_t;

/**
 * @brief  Compass Full Scale
 */
typedef enum {
  LIS3MDL_FS_4GA  = 0x00,                             /*!< ±4 Gauss */
  LIS3MDL_FS_8GA  = 0x02,                             /*!< ±8 Gauss */
  LIS3MDL_FS_12GA = 0x04,                             /*!< ±12 Gauss */
  LIS3MDL_FS_16GA = 0x0C                              /*!< ±16 Gauss */
}LIS3MDL_FS_t;

/**
 * @brief  Compass Low Mode configuration
 */
typedef enum {
  LIS3MDL_LOW_POWER_DISABLED = 0x00,                   /*!< Low Power mode disabled */
  LIS3MDL_LOW_POWER_ENABLED  = 0x20                    /*!< Low Power mode enabled */
}LIS3MDL_PM_t;

/**
 * @brief  Compass Mode
 */
typedef enum {
  LIS3MDL_MD_CONTINOUS_CONVERSION = 0x00,              /*!< Continous conversion mode */
  LIS3MDL_MD_SINGLE_CONVERSION    = 0x01,              /*!< Single conversion mode */
  LIS3MDL_MD_POWER_DOWN           = 0x02               /*!< Power down mode */
}LIS3MDL_MD_t;


/**
 * @brief  Compass Operation Mode for Z axis
 */
typedef enum {
  LIS3MDL_OMZ_LOW_POWER          = 0x00,               /*!< Operation Mode Z low power */
  LIS3MDL_OMZ_MEDIUM_PERFORMANCE = 0x04,               /*!< Operation Mode Z medium performance */
  LIS3MDL_OMZ_HIGH_PERFORMANCE   = 0x08,               /*!< Operation Mode Z high performance */
  LIS3MDL_OMZ_ULTRA_PERFORMANCE  = 0x0C                /*!< Operation Mode Z ultra performance */
}LIS3MDL_OMZ_t;

/**
 * @brief  Compass Endianness
 */
typedef enum {
  LIS3MDL_End_LITTLE = 0x00,                 /*!< Little Endian: data LSB @ lower address */
  LIS3MDL_End_BIG    = 0x02                  /*!< Big Endian: data MSB @ lower address */
}LIS3MDL_End_t;

/**
 * @brief  Compass Block Data Update
 */
typedef enum {
  LIS3MDL_BDU_CONTINOUS = 0x00,              /*!< Continuos Update */
  LIS3MDL_BDU_BLOCKED   = 0x40               /*!< Single Update: output registers not updated until MSB and LSB reading */
}LIS3MDL_BDU_t;




/**
 * @brief   Gyroscope configuration structure.
 */
typedef struct {
  /**
   * @brief  Compass Slave Address
   */
  LIS3MDL_SAD_t   slaveaddress;
  /**
   * @brief  Compass Operation Mode for X and Y axes
   */
  LIS3MDL_OMXY_t  opmodexy;
  /**
   * @brief  Compass Output Data Rate
   */
  LIS3MDL_ODR_t   outputdatarate;
  /**
   * @brief  Compass Full Scale
   */
  LIS3MDL_FS_t    fullscale;
  /**
   * @brief  Compass Low Mode configuration
   */
  LIS3MDL_PM_t    lowpowermode;
  /**
   * @brief  Compass Mode
   */
  LIS3MDL_MD_t    mode;
  /**
   * @brief  Compass Operation Mode for Z axis
   */
  LIS3MDL_OMZ_t   opmodez;
  /**
   * @brief  Compass Endianness
   */
  LIS3MDL_End_t   endianess;
  /**
   * @brief  Compass Block Data Update
   */
  LIS3MDL_BDU_t   blockdataupdate;
} LIS3MDL_Config;
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

  uint8_t lis3mdlReadRegister(I2CDriver *i2cp, uint8_t sad, uint8_t sub,
                                 msg_t* message);
  void lis3mdlWriteRegister(I2CDriver *i2cp, uint8_t sad, uint8_t sub,
                                 uint8_t value, msg_t* message);
#ifdef __cplusplus
}
#endif

#endif /* _LIS3MDL_H_ */

/** @} */
