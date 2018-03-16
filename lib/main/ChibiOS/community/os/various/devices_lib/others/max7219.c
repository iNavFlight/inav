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
 * @file    max7219.c
 * @brief   MAX7219 display driver module code.
 *
 * @addtogroup max7219
 * @{
 */

#include "ch.h"
#include "hal.h"

#include "max7219.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Reads a generic register value.
 * @pre     The SPI interface must be initialized and the driver started.
 *
 * @param[in] spip      pointer to the SPI interface
 * @param[in] adr       address number
 * @param[in] data      data value.
 */
void max7219WriteRegister(SPIDriver *spip, uint16_t adr, uint8_t data) {

  switch (adr) {
    default:
      return;
    case MAX7219_AD_DIGIT_0:
    case MAX7219_AD_DIGIT_1:
    case MAX7219_AD_DIGIT_2:
    case MAX7219_AD_DIGIT_3:
    case MAX7219_AD_DIGIT_4:
    case MAX7219_AD_DIGIT_5:
    case MAX7219_AD_DIGIT_6:
    case MAX7219_AD_DIGIT_7:
    case MAX7219_AD_DECODE_MODE:
    case MAX7219_AD_INTENSITY:
    case MAX7219_AD_SCAN_LIMIT:
    case MAX7219_AD_SHUTDOWN:
    case MAX7219_AD_DISPLAY_TEST:
      spiSelect(spip);
      uint16_t txbuf = {adr | data};
      spiSend(spip, 1, &txbuf);
      spiUnselect(spip);
  }
}
/** @} */
