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
 * @file    l3gd20.c
 * @brief   L3GD20 MEMS interface module code.
 *
 * @addtogroup l3gd20
 * @{
 */

#include "ch.h"
#include "hal.h"

#include "l3gd20.h"

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
 * @param[in] reg       register number
 * @return              register value.
 */
uint8_t l3gd20ReadRegister(SPIDriver *spip, uint8_t reg) {
  uint8_t txbuf[2] = {L3GD20_RW | reg, 0xFF};
  uint8_t rxbuf[2] = {0x00, 0x00};
  spiSelect(spip);
  spiExchange(spip, 2, txbuf, rxbuf);
  spiUnselect(spip);
  return rxbuf[1];
}


void l3gd20WriteRegister(SPIDriver *spip, uint8_t reg, uint8_t value) {

  switch (reg) {

    default:
      /* Reserved register must not be written, according to the datasheet
       * this could permanently damage the device.
       */
      chDbgAssert(FALSE, "lg3d20WriteRegister(), reserved register");
    case L3GD20_AD_WHO_AM_I:
    case L3GD20_AD_OUT_TEMP :
    case L3GD20_AD_STATUS_REG:
    case L3GD20_AD_OUT_X_L:
    case L3GD20_AD_OUT_X_H:
    case L3GD20_AD_OUT_Y_L:
    case L3GD20_AD_OUT_Y_H:
    case L3GD20_AD_OUT_Z_L:
    case L3GD20_AD_OUT_Z_H:
    case L3GD20_AD_FIFO_SRC_REG:
    case L3GD20_AD_INT1_SRC:
    /* Read only registers cannot be written, the command is ignored.*/
      return;
    case L3GD20_AD_CTRL_REG1:
    case L3GD20_AD_CTRL_REG2:
    case L3GD20_AD_CTRL_REG3:
    case L3GD20_AD_CTRL_REG4:
    case L3GD20_AD_CTRL_REG5:
    case L3GD20_AD_REFERENCE:
    case L3GD20_AD_FIFO_CTRL_REG:
    case L3GD20_AD_INT1_CFG:
    case L3GD20_AD_INT1_TSH_XH:
    case L3GD20_AD_INT1_TSH_XL:
    case L3GD20_AD_INT1_TSH_YH:
    case L3GD20_AD_INT1_TSH_YL:
    case L3GD20_AD_INT1_TSH_ZH:
    case L3GD20_AD_INT1_TSH_ZL:
    case L3GD20_AD_INT1_DURATION:
      spiSelect(spip);
      uint8_t txbuf[2] = {reg, value};
      spiSend(spip, 2, txbuf);
      spiUnselect(spip);
  }
}
/** @} */
