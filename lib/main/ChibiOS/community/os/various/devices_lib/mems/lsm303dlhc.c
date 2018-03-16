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
 * @file    lsm303dlhc.c
 * @brief   LSM303DLHC MEMS interface module through I2C code.
 *
 * @addtogroup lsm303dlhc
 * @{
 */

#include "ch.h"
#include "hal.h"

#include "lsm303dlhc.h"

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
uint8_t lsm303dlhcReadRegister(I2CDriver *i2cp, uint8_t sad, uint8_t sub,
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
void lsm303dlhcWriteRegister(I2CDriver *i2cp,uint8_t sad, uint8_t sub,
                             uint8_t value, msg_t* message) {

  uint8_t txbuf[2];
  uint8_t rxbuf;
  if(sad == LSM303DLHC_SAD_ACCEL){
    switch (sub) {
    default:
      /* Reserved register must not be written, according to the datasheet
       * this could permanently damage the device.
       */
      chDbgAssert(FALSE, "lsm303dlhcWriteRegister(), reserved register");
    case LSM303DLHC_SUB_ACC_STATUS_REG:
    case LSM303DLHC_SUB_ACC_OUT_X_L:
    case LSM303DLHC_SUB_ACC_OUT_X_H:
    case LSM303DLHC_SUB_ACC_OUT_Y_L:
    case LSM303DLHC_SUB_ACC_OUT_Y_H:
    case LSM303DLHC_SUB_ACC_OUT_Z_L:
    case LSM303DLHC_SUB_ACC_OUT_Z_H:
    case LSM303DLHC_SUB_ACC_FIFO_SRC_REG:
    case LSM303DLHC_SUB_ACC_INT1_SOURCE:
    case LSM303DLHC_SUB_ACC_INT2_SOURCE:
    case LSM303DLHC_SUB_ACC_CLICK_SRC:
    /* Read only registers cannot be written, the command is ignored.*/
      return;
    case LSM303DLHC_SUB_ACC_CTRL_REG1:
    case LSM303DLHC_SUB_ACC_CTRL_REG2:
    case LSM303DLHC_SUB_ACC_CTRL_REG3:
    case LSM303DLHC_SUB_ACC_CTRL_REG4:
    case LSM303DLHC_SUB_ACC_CTRL_REG5:
    case LSM303DLHC_SUB_ACC_CTRL_REG6:
    case LSM303DLHC_SUB_ACC_REFERENCE:
    case LSM303DLHC_SUB_ACC_FIFO_CTRL_REG:
    case LSM303DLHC_SUB_ACC_INT1_CFG:
    case LSM303DLHC_SUB_ACC_INT1_THS:
    case LSM303DLHC_SUB_ACC_INT1_DURATION:
    case LSM303DLHC_SUB_ACC_INT2_CFG:
    case LSM303DLHC_SUB_ACC_INT2_THS:
    case LSM303DLHC_SUB_ACC_INT2_DURATION:
    case LSM303DLHC_SUB_ACC_CLICK_CFG:
    case LSM303DLHC_SUB_ACC_CLICK_THS:
    case LSM303DLHC_SUB_ACC_TIME_LIMIT:
    case LSM303DLHC_SUB_ACC_TIME_LATENCY:
    case LSM303DLHC_SUB_ACC_TIME_WINDOW:
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
  else if(sad == LSM303DLHC_SAD_COMPASS){
    switch (sub) {
    default:
      /* Reserved register must not be written, according to the datasheet
       * this could permanently damage the device.
       */
      chDbgAssert(FALSE, "lsm303dlhcWriteRegister(), reserved register");
    case LSM303DLHC_SUB_COMP_OUT_X_H:
    case LSM303DLHC_SUB_COMP_OUT_X_L:
    case LSM303DLHC_SUB_COMP_OUT_Z_H:
    case LSM303DLHC_SUB_COMP_OUT_Z_L:
    case LSM303DLHC_SUB_COMP_OUT_Y_H:
    case LSM303DLHC_SUB_COMP_OUT_Y_L:
    case LSM303DLHC_SUB_COMP_SR_REG:
    case LSM303DLHC_SUB_COMP_IRA_REG:
    case LSM303DLHC_SUB_COMP_IRB_REG:
    case LSM303DLHC_SUB_COMP_IRC_REG:
    case LSM303DLHC_SUB_COMP_TEMP_OUT_H:
    case LSM303DLHC_SUB_COMP_TEMP_OUT_L:
      /* Read only registers cannot be written, the command is ignored.*/
      return;
    case LSM303DLHC_SUB_COMP_CRA_REG:
    case LSM303DLHC_SUB_COMP_CRB_REG:
    case LSM303DLHC_SUB_COMP_MR_REG:
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
}
/** @} */
