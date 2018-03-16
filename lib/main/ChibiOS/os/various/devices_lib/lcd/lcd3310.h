/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    lcd3310.h
 * @brief   Nokia 3310 LCD interface module through SPI code.
 *
 * @addtogroup lcd3310
 * @{
 */

#ifndef _LCD3310_H_
#define _LCD3310_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define LCD3310_X_RES                  84
#define LCD3310_Y_RES                  48

#define LCD3310_FONT_X_SIZE             5
#define LCD3310_FONT_Y_SIZE             8

#define LCD3310_SEND_CMD                0
#define LCD3310_SEND_DATA               1

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !defined(LCD3310_RES_PIN)
#error "LCD3310_RES_PIN not defined!!!"
#endif

#if !defined(LCD3310_RES_PORT)
#error "LCD3310_RES_PORT not defined!!!"
#endif

#if !defined(LCD3310_DC_PIN)
#error "LCD3310_DC_PIN not defined!!!"
#endif

#if!defined(LCD3310_DC_PORT)
#error "LCD3310_DC_PORT not defined!!!"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void lcd3310Init(SPIDriver *spip);
  void lcd3310WriteByte(SPIDriver *spip, uint8_t data, uint8_t cd);
  void lcd3310Contrast(SPIDriver *spip, uint8_t contrast);
  void lcd3310Clear(SPIDriver *spip);
  void lcd3310SetPosXY(SPIDriver *spip, uint8_t x, uint8_t y);
  void lcd3310WriteChar (SPIDriver *spip, uint8_t ch);
  void lcd3310WriteText(SPIDriver *spip, const uint8_t * strp);
  void lcd3310RotateText(SPIDriver *spip, const uint8_t * strp, uint8_t offset);
#ifdef __cplusplus
}
#endif

#endif /* _LCD3310_H_ */

/** @} */
