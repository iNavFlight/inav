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
 * @file    lcd3310.c
 * @brief   Nokia 3310 LCD interface module through SPI code.
 *
 * @addtogroup lcd3310
 * @{
 */

#include "ch.h"
#include "hal.h"
#include "lcd3310.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

const uint8_t  Fonts8x5 [][LCD3310_FONT_X_SIZE] =
{
    { 0x00, 0x00, 0x00, 0x00, 0x00 },   /* space */
    { 0x00, 0x00, 0x2f, 0x00, 0x00 },   /* ! */
    { 0x00, 0x07, 0x00, 0x07, 0x00 },   /* " */
    { 0x14, 0x7f, 0x14, 0x7f, 0x14 },   /* # */
    { 0x24, 0x2a, 0x7f, 0x2a, 0x12 },   /* $ */
    { 0xc4, 0xc8, 0x10, 0x26, 0x46 },   /* % */
    { 0x36, 0x49, 0x55, 0x22, 0x50 },   /* & */
    { 0x00, 0x05, 0x03, 0x00, 0x00 },   /* ' */
    { 0x00, 0x1c, 0x22, 0x41, 0x00 },   /* ( */
    { 0x00, 0x41, 0x22, 0x1c, 0x00 },   /* ) */
    { 0x14, 0x08, 0x3E, 0x08, 0x14 },   /* * */
    { 0x08, 0x08, 0x3E, 0x08, 0x08 },   /* + */
    { 0x00, 0x00, 0x50, 0x30, 0x00 },   /* , */
    { 0x10, 0x10, 0x10, 0x10, 0x10 },   /* - */
    { 0x00, 0x60, 0x60, 0x00, 0x00 },   /* . */
    { 0x20, 0x10, 0x08, 0x04, 0x02 },   /* / */
    { 0x3E, 0x51, 0x49, 0x45, 0x3E },   /* 0 */
    { 0x00, 0x42, 0x7F, 0x40, 0x00 },   /* 1 */
    { 0x42, 0x61, 0x51, 0x49, 0x46 },   /* 2 */
    { 0x21, 0x41, 0x45, 0x4B, 0x31 },   /* 3 */
    { 0x18, 0x14, 0x12, 0x7F, 0x10 },   /* 4 */
    { 0x27, 0x45, 0x45, 0x45, 0x39 },   /* 5 */
    { 0x3C, 0x4A, 0x49, 0x49, 0x30 },   /* 6 */
    { 0x01, 0x71, 0x09, 0x05, 0x03 },   /* 7 */
    { 0x36, 0x49, 0x49, 0x49, 0x36 },   /* 8 */
    { 0x06, 0x49, 0x49, 0x29, 0x1E },   /* 9 */
    { 0x00, 0x36, 0x36, 0x00, 0x00 },   /* : */
    { 0x00, 0x56, 0x36, 0x00, 0x00 },   /* ; */
    { 0x08, 0x14, 0x22, 0x41, 0x00 },   /* < */
    { 0x14, 0x14, 0x14, 0x14, 0x14 },   /* = */
    { 0x00, 0x41, 0x22, 0x14, 0x08 },   /* > */
    { 0x02, 0x01, 0x51, 0x09, 0x06 },   /* ? */
    { 0x32, 0x49, 0x59, 0x51, 0x3E },   /* @ */
    { 0x7E, 0x11, 0x11, 0x11, 0x7E },   /* A */
    { 0x7F, 0x49, 0x49, 0x49, 0x36 },   /* B */
    { 0x3E, 0x41, 0x41, 0x41, 0x22 },   /* C */
    { 0x7F, 0x41, 0x41, 0x22, 0x1C },   /* D */
    { 0x7F, 0x49, 0x49, 0x49, 0x41 },   /* E */
    { 0x7F, 0x09, 0x09, 0x09, 0x01 },   /* F */
    { 0x3E, 0x41, 0x49, 0x49, 0x7A },   /* G */
    { 0x7F, 0x08, 0x08, 0x08, 0x7F },   /* H */
    { 0x00, 0x41, 0x7F, 0x41, 0x00 },   /* I */
    { 0x20, 0x40, 0x41, 0x3F, 0x01 },   /* J */
    { 0x7F, 0x08, 0x14, 0x22, 0x41 },   /* K */
    { 0x7F, 0x40, 0x40, 0x40, 0x40 },   /* L */
    { 0x7F, 0x02, 0x0C, 0x02, 0x7F },   /* M */
    { 0x7F, 0x04, 0x08, 0x10, 0x7F },   /* N */
    { 0x3E, 0x41, 0x41, 0x41, 0x3E },   /* O */
    { 0x7F, 0x09, 0x09, 0x09, 0x06 },   /* P */
    { 0x3E, 0x41, 0x51, 0x21, 0x5E },   /* Q */
    { 0x7F, 0x09, 0x19, 0x29, 0x46 },   /* R */
    { 0x46, 0x49, 0x49, 0x49, 0x31 },   /* S */
    { 0x01, 0x01, 0x7F, 0x01, 0x01 },   /* T */
    { 0x3F, 0x40, 0x40, 0x40, 0x3F },   /* U */
    { 0x1F, 0x20, 0x40, 0x20, 0x1F },   /* V */
    { 0x3F, 0x40, 0x38, 0x40, 0x3F },   /* W */
    { 0x63, 0x14, 0x08, 0x14, 0x63 },   /* X */
    { 0x07, 0x08, 0x70, 0x08, 0x07 },   /* Y */
    { 0x61, 0x51, 0x49, 0x45, 0x43 },   /* Z */
    { 0x00, 0x7F, 0x41, 0x41, 0x00 },   /* [ */
    { 0x55, 0x2A, 0x55, 0x2A, 0x55 },   /* \ */
    { 0x00, 0x41, 0x41, 0x7F, 0x00 },   /* ] */
    { 0x04, 0x02, 0x01, 0x02, 0x04 },   /* ^ */
    { 0x40, 0x40, 0x40, 0x40, 0x40 },   /* _ */
    { 0x00, 0x01, 0x02, 0x04, 0x00 },   /* ' */
    { 0x20, 0x54, 0x54, 0x54, 0x78 },   /* a */
    { 0x7F, 0x48, 0x44, 0x44, 0x38 },   /* b */
    { 0x38, 0x44, 0x44, 0x44, 0x20 },   /* c */
    { 0x38, 0x44, 0x44, 0x48, 0x7F },   /* d */
    { 0x38, 0x54, 0x54, 0x54, 0x18 },   /* e */
    { 0x08, 0x7E, 0x09, 0x01, 0x02 },   /* f */
    { 0x0C, 0x52, 0x52, 0x52, 0x3E },   /* g */
    { 0x7F, 0x08, 0x04, 0x04, 0x78 },   /* h */
    { 0x00, 0x44, 0x7D, 0x40, 0x00 },   /* i */
    { 0x20, 0x40, 0x44, 0x3D, 0x00 },   /* j */
    { 0x7F, 0x10, 0x28, 0x44, 0x00 },   /* k */
    { 0x00, 0x41, 0x7F, 0x40, 0x00 },   /* l */
    { 0x7C, 0x04, 0x18, 0x04, 0x78 },   /* m */
    { 0x7C, 0x08, 0x04, 0x04, 0x78 },   /* n */
    { 0x38, 0x44, 0x44, 0x44, 0x38 },   /* o */
    { 0x7C, 0x14, 0x14, 0x14, 0x08 },   /* p */
    { 0x08, 0x14, 0x14, 0x18, 0x7C },   /* q */
    { 0x7C, 0x08, 0x04, 0x04, 0x08 },   /* r */
    { 0x48, 0x54, 0x54, 0x54, 0x20 },   /* s */
    { 0x04, 0x3F, 0x44, 0x40, 0x20 },   /* t */
    { 0x3C, 0x40, 0x40, 0x20, 0x7C },   /* u */
    { 0x1C, 0x20, 0x40, 0x20, 0x1C },   /* v */
    { 0x3C, 0x40, 0x30, 0x40, 0x3C },   /* w */
    { 0x44, 0x28, 0x10, 0x28, 0x44 },   /* x */
    { 0x0C, 0x50, 0x50, 0x50, 0x3C },   /* y */
    { 0x44, 0x64, 0x54, 0x4C, 0x44 },   /* z */
    { 0x00, 0x08, 0x36, 0x41, 0x00 },   /* { */
    { 0x00, 0x00, 0x7F, 0x00, 0x00 },   /* | */
    { 0x00, 0x41, 0x36, 0x08, 0x00 },   /* } */
};


/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   LCD driver initialization.
 * @pre     The SPI interface must be initialized and the driver started.
 *
 * @param[in] spip      pointer to the SPI interface
 *
 */
void lcd3310Init(SPIDriver *spip) {

  /* Reset LCD */
  palClearPad(LCD3310_RES_PORT, LCD3310_RES_PIN);
  chThdSleepMilliseconds(15);
  palSetPad(LCD3310_RES_PORT, LCD3310_RES_PIN);
  chThdSleepMilliseconds(15);

  /* Send configuration commands to LCD */
  lcd3310WriteByte(spip, 0x21, LCD3310_SEND_CMD);  /* LCD extended commands */
  lcd3310WriteByte(spip, 0xC8, LCD3310_SEND_CMD);  /* Set LCD Vop (Contrast) */
  lcd3310WriteByte(spip, 0x05, LCD3310_SEND_CMD);  /* Set start line S6 to 1 TLS8204 */
  lcd3310WriteByte(spip, 0x40, LCD3310_SEND_CMD);  /* Set start line S[5:0] to 0x00 TLS8204 */
  lcd3310WriteByte(spip, 0x12, LCD3310_SEND_CMD);  /* LCD bias mode 1:68. */
  lcd3310WriteByte(spip, 0x20, LCD3310_SEND_CMD);  /* LCD standard Commands, horizontal addressing mode. */
  lcd3310WriteByte(spip, 0x08, LCD3310_SEND_CMD);  /* LCD blank */
  lcd3310WriteByte(spip, 0x0C, LCD3310_SEND_CMD);  /* LCD in normal mode. */

  lcd3310Clear(spip); /* Clear LCD */
}

/**
 * @brief   Write byte to LCD driver.
 * @pre     The LCD driver must be initialized.
 *
 * @param[in] spip    pointer to the SPI interface
 * @param[in] data    data to write
 * @param[in] cd      select between command or data
 */
void lcd3310WriteByte(SPIDriver *spip, uint8_t data, uint8_t cd) {

  spiSelect(spip);

  if(cd == LCD3310_SEND_DATA) {
    palSetPad(LCD3310_DC_PORT, LCD3310_DC_PIN);
  }
  else {
    palClearPad(LCD3310_DC_PORT, LCD3310_DC_PIN);
  }

  spiSend(spip, 1, &data);   // change to normal spi send
  spiUnselect(spip);
}

/**
 * @brief   Clear LCD
 * @pre     The LCD driver must be initialized.
 *
 * @param[in] spip    pointer to the SPI interface
 */
void lcd3310Clear(SPIDriver *spip) { // ok

  uint32_t i, j;

  for (i = 0; i < LCD3310_Y_RES/LCD3310_FONT_Y_SIZE; i++) {
    lcd3310SetPosXY(spip, 0, i);
    for (j = 0; j < LCD3310_X_RES; j++)
      lcd3310WriteByte(spip, 0x00, LCD3310_SEND_DATA);
  }

}

/**
 * @brief   Set position
 * @pre     The LCD driver must be initialized.
 *
 * @param[in] spip    pointer to the SPI interface
 * @param[in] x    column address in LCD DDRAM, 0 to 83
 * @param[in] y    page address in LCD DDRAM, 0 to 5
 */
void lcd3310SetPosXY(SPIDriver *spip, uint8_t x, uint8_t y) {

    if (y > LCD3310_Y_RES/LCD3310_FONT_Y_SIZE) return;
    if (x > LCD3310_X_RES) return;

    lcd3310WriteByte(spip, 0x80 | x, LCD3310_SEND_CMD);   /* Set x position */
    lcd3310WriteByte(spip, 0x40 | y, LCD3310_SEND_CMD);   /* Set y position */

}

/**
 * @brief   Write char
 * @pre     The LCD driver must be initialized.
 *
 * @param[in] spip    pointer to the SPI interface
 * @param[in] ch      char
 */
void lcd3310WriteChar(SPIDriver *spip, uint8_t ch) {

  uint8_t i;

  for ( i = 0; i < LCD3310_FONT_X_SIZE; i++ ){
    lcd3310WriteByte(spip, Fonts8x5[ch - 32][i], LCD3310_SEND_DATA);
  }

}

/**
 * @brief   Set LCD contrast.
 * @pre     The LCD driver must be initialized.
 *
 * @param[in] spip      pointer to the SPI interface
 * @param[in] contrast  LCD contrast value
 */
void lcd3310Contrast (SPIDriver *spip, uint8_t contrast) {

  lcd3310WriteByte(spip, 0x21, LCD3310_SEND_CMD);              /* LCD Extended Commands */
  lcd3310WriteByte(spip, 0x80 | contrast, LCD3310_SEND_CMD);   /* Set LCD Vop (Contrast) */
  lcd3310WriteByte(spip, 0x20, LCD3310_SEND_CMD);              /* LCD Standard Commands, horizontal addressing mode */
}


/**
 * @brief   Write text
 * @pre     The LCD driver must be initialized.
 *
 * @param[in] spip      pointer to the SPI interface
 * @param[in] strp      pointer to text
 */
void lcd3310WriteText(SPIDriver *spip, const uint8_t * strp) {

  while ( *strp ) {
    lcd3310WriteChar(spip, *strp);
    strp++;
  }
}

/**
 * @brief   Rotate text
 * @pre     The LCD driver must be initialized.
 *
 * @param[in] spip      pointer to the SPI interface
 * @param[in] strp      pointer to text
 * @param[in] offset    text offset
 */
void lcd3310RotateText(SPIDriver *spip, const uint8_t * strp, uint8_t offset) {

  uint8_t i;
  uint8_t n;
  uint8_t m;

  for(n = 0; strp[n] != '\0'; n++); /* Count number of char */

  if (offset >= n)
    return;

  for (i = 0; i < LCD3310_X_RES/LCD3310_FONT_X_SIZE; i++) {
      m = i + offset;
      if ( m < n)
        lcd3310WriteChar(spip, strp[m]);
      else
        lcd3310WriteChar(spip, strp[m - n]);
  }
}

/** @} */
