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
 * @file    MAX7219.h
 * @brief   MAX7219 display driver module header.
 *
 * @{
 */

#ifndef _MAX7219_H_
#define _MAX7219_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    MAX7219 register names
 * @{
 */
/******************************************************************************/
/*                                                                            */
/*                        MAX7219 display driver                              */
/*                                                                            */
/******************************************************************************/
/*******************  Bit definition for SPI communication  *******************/
#define  MAX7219_DI                              ((uint16_t)0x00FF)          /*!< DI[7:0] Data input */
#define  MAX7219_DI_0                            ((uint16_t)0x0001)          /*!< bit 0 */
#define  MAX7219_DI_1                            ((uint16_t)0x0002)          /*!< bit 1 */
#define  MAX7219_DI_2                            ((uint16_t)0x0004)          /*!< bit 2 */
#define  MAX7219_DI_3                            ((uint16_t)0x0008)          /*!< bit 3 */
#define  MAX7219_DI_4                            ((uint16_t)0x0010)          /*!< bit 4 */
#define  MAX7219_DI_5                            ((uint16_t)0x0020)          /*!< bit 5 */
#define  MAX7219_DI_6                            ((uint16_t)0x0040)          /*!< bit 6 */
#define  MAX7219_DI_7                            ((uint16_t)0x0080)          /*!< bit 7 */

#define  MAX7219_AD                              ((uint16_t)0x0F00)          /*!< AD[11:8] Data input */
#define  MAX7219_AD_0                            ((uint16_t)0x0100)          /*!< bit 8 */
#define  MAX7219_AD_1                            ((uint16_t)0x0200)          /*!< bit 9 */
#define  MAX7219_AD_2                            ((uint16_t)0x0400)          /*!< bit 10 */
#define  MAX7219_AD_3                            ((uint16_t)0x0800)          /*!< bit 11 */

/******************  Bit definition for Registers Addresses *******************/
#define  MAX7219_AD_NOP                          ((uint16_t)0x0000)          /*!< No operation */
#define  MAX7219_AD_DIGIT_0                      ((uint16_t)0x0100)          /*!< Digit 0 */
#define  MAX7219_AD_DIGIT_1                      ((uint16_t)0x0200)          /*!< Digit 1 */
#define  MAX7219_AD_DIGIT_2                      ((uint16_t)0x0300)          /*!< Digit 2 */
#define  MAX7219_AD_DIGIT_3                      ((uint16_t)0x0400)          /*!< Digit 3 */
#define  MAX7219_AD_DIGIT_4                      ((uint16_t)0x0500)          /*!< Digit 4 */
#define  MAX7219_AD_DIGIT_5                      ((uint16_t)0x0600)          /*!< Digit 5 */
#define  MAX7219_AD_DIGIT_6                      ((uint16_t)0x0700)          /*!< Digit 6 */
#define  MAX7219_AD_DIGIT_7                      ((uint16_t)0x0800)          /*!< Digit 7 */
#define  MAX7219_AD_DECODE_MODE                  ((uint16_t)0x0900)          /*!< Decode mode */
#define  MAX7219_AD_INTENSITY                    ((uint16_t)0x0A00)          /*!< Intensity */
#define  MAX7219_AD_SCAN_LIMIT                   ((uint16_t)0x0B00)          /*!< Scan limit */
#define  MAX7219_AD_SHUTDOWN                     ((uint16_t)0x0C00)          /*!< Shutdown */
#define  MAX7219_AD_DISPLAY_TEST                 ((uint16_t)0x0F00)          /*!< Display test */

/***************  Bit definition for Registers Configuration  *****************/
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !HAL_USE_SPI
#error "MAX7219 requires HAL_USE_SPI"
#endif
/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @name    MAX7219 data structures and types
 * @{
 *
 */

/**
 * @brief  MAX7219 operation mode
 */
typedef enum {
  MAX7219_OM_Shutdown  = 0x00,                 /*!< Shutdown mode */
  MAX7219_OM_Normal    = 0x01                  /*!< Normal mode */
} MAX7219_OM_t;

/**
 * @brief  MAX7219 decoder mode
 */
typedef enum {
  MAX7219_DM_No_decode  = 0x00,                 /*!< No decode */
  MAX7219_DM_CodeB_0    = 0x01,                 /*!< Code B on Digit 0 */
  MAX7219_DM_CodeB_1    = 0x03,                 /*!< Code B on Digits 0-1 */
  MAX7219_DM_CodeB_2    = 0x07,                 /*!< Code B on Digits from 0 to 2 */
  MAX7219_DM_CodeB_3    = 0x0F,                 /*!< Code B on Digits from 0 to 3 */
  MAX7219_DM_CodeB_4    = 0x1F,                 /*!< Code B on Digits from 0 to 4 */
  MAX7219_DM_CodeB_5    = 0x3F,                 /*!< Code B on Digits from 0 to 5 */
  MAX7219_DM_CodeB_6    = 0x7F,                 /*!< Code B on Digits from 0 to 6 */
  MAX7219_DM_CodeB_7    = 0xFF                  /*!< Code B on every digit */
} MAX7219_DM_t;

/**
 * @brief  MAX7219 intensity mode
 */
typedef enum {
  MAX7219_IM_1_32       = 0x00,                 /*!< 1/32 intensity */
  MAX7219_IM_3_32       = 0x01,                 /*!< 3/32 intensity */
  MAX7219_IM_5_32       = 0x02,                 /*!< 5/32 intensity */
  MAX7219_IM_7_32       = 0x03,                 /*!< 7/32 intensity */
  MAX7219_IM_9_32       = 0x04,                 /*!< 9/32 intensity */
  MAX7219_IM_11_32      = 0x05,                 /*!< 11/32 intensity */
  MAX7219_IM_13_32      = 0x06,                 /*!< 13/32 intensity */
  MAX7219_IM_15_32      = 0x07,                 /*!< 15/32 intensity */
  MAX7219_IM_17_32      = 0x08,                 /*!< 17/32 intensity */
  MAX7219_IM_19_32      = 0x09,                 /*!< 19/32 intensity */
  MAX7219_IM_21_32      = 0x0A,                 /*!< 21/32 intensity */
  MAX7219_IM_23_32      = 0x0B,                 /*!< 23/32 intensity */
  MAX7219_IM_25_32      = 0x0C,                 /*!< 25/32 intensity */
  MAX7219_IM_27_32      = 0x0D,                 /*!< 27/32 intensity */
  MAX7219_IM_29_32      = 0x0E,                 /*!< 29/32 intensity */
  MAX7219_IM_31_32      = 0x0F                  /*!< 31/32 intensity */
} MAX7219_IM_t;

/**
 * @brief  MAX7219 scan line mode
 */
typedef enum {
  MAX7219_SL_0          = 0x00,                 /*!< Scanned digit 0 only */
  MAX7219_SL_1          = 0x01,                 /*!< Scanned digit 0 & 1 */
  MAX7219_SL_2          = 0x02,                 /*!< Scanned digit 0 - 2 */
  MAX7219_SL_3          = 0x03,                 /*!< Scanned digit 0 - 3 */
  MAX7219_SL_4          = 0x04,                 /*!< Scanned digit 0 - 4 */
  MAX7219_SL_5          = 0x05,                 /*!< Scanned digit 0 - 5 */
  MAX7219_SL_6          = 0x06,                 /*!< Scanned digit 0 - 6 */
  MAX7219_SL_7          = 0x07                  /*!< Scanned digit 0 - 7 */
} MAX7219_SL_t;
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

  void max7219WriteRegister(SPIDriver *spip, uint16_t adr, uint8_t data);
#ifdef __cplusplus
}
#endif
#endif /* _MAX7219_H_ */

/** @} */

