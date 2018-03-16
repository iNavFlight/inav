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
 * @file    lsm6ds0.c
 * @brief   LSM6DS0 MEMS interface module through I2C code.
 *
 * @addtogroup lsm6ds0
 * @{
 */

#include "ch.h"
#include "hal.h"

#include "lsm6ds0.h"

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
 * @brief   Reads a generic sub-register value.
 * @pre     The I2C interface must be initialized and the driver started.
 *
 * @param[in] i2cp      pointer to the I2C interface
 * @param[in] sad       slave address without R bit
 * @param[in] sub       sub-register address
 * @param[in] message   pointer to message
 * @return              register value.
 */
uint8_t lsm6ds0ReadRegister(I2CDriver *i2cp, uint8_t sad, uint8_t sub,
                                 msg_t* message) {

  uint8_t txbuf, rxbuf[2];
#if defined(STM32F103_MCUCONF)
  txbuf = LSM303DLHC_SUB_MSB | sub;
  if(message != NULL){
    *message = i2cMasterTransmitTimeout(i2cp, sad, &txbuf, 1, rxbuf, 2,
                                        TIME_INFINITE);
  }
  else{
    i2cMasterTransmitTimeout(i2cp, sad, &txbuf, 1, rxbuf, 2, TIME_INFINITE);
  }
  return rxbuf[0];
#else
  txbuf = sub;
  if(message != NULL){
    *message = i2cMasterTransmitTimeout(i2cp, sad, &txbuf, 1, rxbuf, 1,
                                        TIME_INFINITE);
  }
  else{
    i2cMasterTransmitTimeout(i2cp, sad, &txbuf, 1, rxbuf, 1, TIME_INFINITE);
  }
  return rxbuf[0];
#endif
}

/**
 * @brief   Writes a value into a register.
 * @pre     The I2C interface must be initialized and the driver started.
 *
 * @param[in] i2cp       pointer to the I2C interface
 * @param[in] sad        slave address without R bit
 * @param[in] sub        sub-register address
 * @param[in] value      the value to be written
 * @param[out] message   pointer to message
 */
void lsm6ds0WriteRegister(I2CDriver *i2cp, uint8_t sad, uint8_t sub,
                               uint8_t value, msg_t* message) {

  uint8_t txbuf[2];
  uint8_t rxbuf;
  switch (sub) {
  default:
    /* Reserved register must not be written, according to the datasheet
     * this could permanently damage the device.
     */
    chDbgAssert(FALSE, "lsm6ds0WriteRegister(), reserved register");
  case LSM6DS0_SUB_WHO_AM_I:
  case LSM6DS0_SUB_INT_GEN_SRC_G:
  case LSM6DS0_SUB_OUT_TEMP_L:
  case LSM6DS0_SUB_OUT_TEMP_H:
  case LSM6DS0_SUB_STATUS_REG1:
  case LSM6DS0_SUB_OUT_X_L_G:
  case LSM6DS0_SUB_OUT_X_H_G:
  case LSM6DS0_SUB_OUT_Y_L_G:
  case LSM6DS0_SUB_OUT_Y_H_G:
  case LSM6DS0_SUB_OUT_Z_L_G:
  case LSM6DS0_SUB_OUT_Z_H_G:
  case LSM6DS0_SUB_INT_GEN_SRC_XL:
  case LSM6DS0_SUB_STATUS_REG2:
  case LSM6DS0_SUB_OUT_X_L_XL:
  case LSM6DS0_SUB_OUT_X_H_XL:
  case LSM6DS0_SUB_OUT_Y_L_XL:
  case LSM6DS0_SUB_OUT_Y_H_XL:
  case LSM6DS0_SUB_OUT_Z_L_XL:
  case LSM6DS0_SUB_OUT_Z_H_XL:
  case LSM6DS0_SUB_FIFO_SRC:
  /* Read only registers cannot be written, the command is ignored.*/
    return;
  case LSM6DS0_SUB_ACT_THS:
  case LSM6DS0_SUB_ACT_DUR:
  case LSM6DS0_SUB_INT_GEN_CFG_XL:
  case LSM6DS0_SUB_INT_GEN_THS_X_XL:
  case LSM6DS0_SUB_INT_GEN_THS_Y_XL:
  case LSM6DS0_SUB_INT_GEN_THS_Z_XL:
  case LSM6DS0_SUB_INT_GEN_DUR_XL:
  case LSM6DS0_SUB_REFERENCE_G:
  case LSM6DS0_SUB_INT_CTRL:
  case LSM6DS0_SUB_CTRL_REG1_G:
  case LSM6DS0_SUB_CTRL_REG2_G:
  case LSM6DS0_SUB_CTRL_REG3_G:
  case LSM6DS0_SUB_ORIENT_CFG_G:
  case LSM6DS0_SUB_CTRL_REG4:
  case LSM6DS0_SUB_CTRL_REG5_XL:
  case LSM6DS0_SUB_CTRL_REG6_XL:
  case LSM6DS0_SUB_CTRL_REG7_XL:
  case LSM6DS0_SUB_CTRL_REG8:
  case LSM6DS0_SUB_CTRL_REG9:
  case LSM6DS0_SUB_CTRL_REG10:
  case LSM6DS0_SUB_FIFO_CTRL:
  case LSM6DS0_SUB_INT_GEN_CFG_G:
  case LSM6DS0_SUB_INT_GEN_THS_XH_G:
  case LSM6DS0_SUB_INT_GEN_THS_XL_G:
  case LSM6DS0_SUB_INT_GEN_THS_YH_G:
  case LSM6DS0_SUB_INT_GEN_THS_YL_G:
  case LSM6DS0_SUB_INT_GEN_THS_ZH_G:
  case LSM6DS0_SUB_INT_GEN_THS_ZL_G:
  case LSM6DS0_SUB_INT_GEN_DUR_G:
    txbuf[0] = sub;
    txbuf[1] = value;
    if(message != NULL){
      *message = i2cMasterTransmitTimeout(i2cp, sad, txbuf, 2, &rxbuf, 0,
                                          TIME_INFINITE);
    }
    else{
      i2cMasterTransmitTimeout(i2cp, sad, txbuf, 2, &rxbuf, 0, TIME_INFINITE);
    }
    break;
  }
}

/** @} */
