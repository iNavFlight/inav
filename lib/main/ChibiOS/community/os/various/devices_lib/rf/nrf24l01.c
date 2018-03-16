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
 * @file    nrf24l01.c
 * @brief   NRF24L01 interface module code.
 *
 * @addtogroup nrf24l01
 * @{
 */

#include "ch.h"
#include "hal.h"

#include "nrf24l01.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define ACTIVATE                  0x73
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
 * @brief   Gets the status register value.
 * @pre     The SPI interface must be initialized and the driver started.
 *
 * @param[in] spip      pointer to the SPI interface
 *
 * @return              the status register value
 */
NRF24L01_status_t nrf24l01GetStatus(SPIDriver *spip) {
  uint8_t txbuf = NRF24L01_CMD_NOP;
  uint8_t status;
  spiSelect(spip);
  spiExchange(spip, 1, &txbuf, &status);
  spiUnselect(spip);
  return status;
}

/**
 * @brief   Reads a generic register value.
 *
 * @note    Cannot be used to set addresses
 * @pre     The SPI interface must be initialized and the driver started.
 *
 * @param[in] spip      pointer to the SPI interface
 * @param[in] reg       register number
 * @param[out] pvalue   pointer to a data buffer
 *
 * @return              the status register value
 */
NRF24L01_status_t nrf24l01ReadRegister(SPIDriver *spip, uint8_t reg,
                                       uint8_t* pvalue) {
  uint8_t txbuf = (NRF24L01_CMD_READ | reg);
  uint8_t status = 0xFF;
  spiSelect(spip);
  spiExchange(spip, 1, &txbuf, &status);
  spiReceive(spip, 1, pvalue);
  spiUnselect(spip);
  return status;
}

/**
 * @brief   Writes a generic register value.
 *
 * @note    Cannot be used to set addresses
 * @pre     The SPI interface must be initialized and the driver started.
 *
 * @param[in] spip      pointer to the SPI interface
 * @param[in] reg       register number
 * @param[in] value     data value
 *
 * @return              the status register value
 */
NRF24L01_status_t nrf24l01WriteRegister(SPIDriver *spip, uint8_t reg,
                                        uint8_t value) {

  uint8_t txbuf[2] = {(NRF24L01_CMD_WRITE | reg), value};
  uint8_t rxbuf[2] = {0xFF, 0xFF};
  switch (reg) {

    default:
      /* Reserved register must not be written, according to the datasheet
       * this could permanently damage the device.
       */
      chDbgAssert(FALSE, "lg3d20WriteRegister(), reserved register");
    case NRF24L01_AD_OBSERVE_TX:
    case NRF24L01_AD_CD:
    case NRF24L01_AD_RX_ADDR_P0:
    case NRF24L01_AD_RX_ADDR_P1:
    case NRF24L01_AD_RX_ADDR_P2:
    case NRF24L01_AD_RX_ADDR_P3:
    case NRF24L01_AD_RX_ADDR_P4:
    case NRF24L01_AD_RX_ADDR_P5:
    case NRF24L01_AD_TX_ADDR:
    /* Read only or addresses registers cannot be written,
     * the command is ignored.
     */
      return 0;
    case NRF24L01_AD_CONFIG:
    case NRF24L01_AD_EN_AA:
    case NRF24L01_AD_EN_RXADDR:
    case NRF24L01_AD_SETUP_AW:
    case NRF24L01_AD_SETUP_RETR:
    case NRF24L01_AD_RF_CH:
    case NRF24L01_AD_RF_SETUP:
    case NRF24L01_AD_STATUS:
    case NRF24L01_AD_RX_PW_P0:
    case NRF24L01_AD_RX_PW_P1:
    case NRF24L01_AD_RX_PW_P2:
    case NRF24L01_AD_RX_PW_P3:
    case NRF24L01_AD_RX_PW_P4:
    case NRF24L01_AD_RX_PW_P5:
    case NRF24L01_AD_FIFO_STATUS:
    case NRF24L01_AD_DYNPD:
    case NRF24L01_AD_FEATURE:
      spiSelect(spip);
      spiExchange(spip, 2, txbuf, rxbuf);
      spiUnselect(spip);
      return rxbuf[0];
  }
}


/**
 * @brief   Writes an address.
 *
 * @pre     The SPI interface must be initialized and the driver started.
 *
 * @param[in] spip      pointer to the SPI interface
 * @param[in] reg       register number
 * @param[in] pvalue    pointer to address value
 * @param[in] addlen    address len
 *
 * @return              the status register value
 */
NRF24L01_status_t nrf24l01WriteAddress(SPIDriver *spip, uint8_t reg,
                                       uint8_t *pvalue, uint8_t addlen) {

  uint8_t txbuf[NRF24L01_MAX_ADD_LENGHT + 1];
  uint8_t rxbuf[NRF24L01_MAX_ADD_LENGHT + 1];
  unsigned i;

  if(addlen > NRF24L01_MAX_ADD_LENGHT) {
    chDbgAssert(FALSE, "nrf24l01WriteAddress(), wrong address length");
    return 0;
  }
  txbuf[0] = (NRF24L01_CMD_WRITE | reg);
  rxbuf[0] = 0xFF;
  for(i = 1; i <= addlen; i++) {
    txbuf[i] = *(pvalue + (i - 1));
    rxbuf[i] = 0xFF;
  }
  switch (reg) {

    default:
      /* Reserved register must not be written, according to the datasheet
       * this could permanently damage the device.
       */
      chDbgAssert(FALSE, "nrf24l01WriteAddress(), reserved register");
    case NRF24L01_AD_OBSERVE_TX:
    case NRF24L01_AD_CD:
    case NRF24L01_AD_CONFIG:
    case NRF24L01_AD_EN_AA:
    case NRF24L01_AD_EN_RXADDR:
    case NRF24L01_AD_SETUP_AW:
    case NRF24L01_AD_SETUP_RETR:
    case NRF24L01_AD_RF_CH:
    case NRF24L01_AD_RF_SETUP:
    case NRF24L01_AD_STATUS:
    case NRF24L01_AD_RX_PW_P0:
    case NRF24L01_AD_RX_PW_P1:
    case NRF24L01_AD_RX_PW_P2:
    case NRF24L01_AD_RX_PW_P3:
    case NRF24L01_AD_RX_PW_P4:
    case NRF24L01_AD_RX_PW_P5:
    case NRF24L01_AD_FIFO_STATUS:
    case NRF24L01_AD_DYNPD:
    case NRF24L01_AD_FEATURE:
    /* Not address registers cannot be written, the command is ignored.*/
      return 0;
    case NRF24L01_AD_RX_ADDR_P0:
    case NRF24L01_AD_RX_ADDR_P1:
    case NRF24L01_AD_RX_ADDR_P2:
    case NRF24L01_AD_RX_ADDR_P3:
    case NRF24L01_AD_RX_ADDR_P4:
    case NRF24L01_AD_RX_ADDR_P5:
    case NRF24L01_AD_TX_ADDR:
      spiSelect(spip);
      spiExchange(spip, addlen + 1, txbuf, rxbuf);
      spiUnselect(spip);
      return rxbuf[0];
  }
}
/**
 * @brief   Reads RX payload from FIFO.
 *
 * @note    Payload is deleted from FIFO after it is read. Used in RX mode.
 * @pre     The SPI interface must be initialized and the driver started.
 *
 * @param[in] spip      pointer to the SPI interface
 * @param[in] paylen    payload length
 * @param[in] rxbuf     pointer to a buffer
 *
 * @return              the status register value
 */
NRF24L01_status_t nrf24l01GetRxPl(SPIDriver *spip, uint8_t paylen,
                                  uint8_t* rxbuf) {

  uint8_t txbuf = NRF24L01_CMD_R_RX_PAYLOAD;
  uint8_t status;
  if(paylen > NRF24L01_MAX_PL_LENGHT) {
    return 0;
  }
  spiSelect(spip);
  spiExchange(spip, 1, &txbuf, &status);
  spiReceive(spip, paylen, rxbuf);
  spiUnselect(spip);
  return status;
}

/**
 * @brief   Writes TX payload on FIFO.
 *
 * @note    Used in TX mode.
 * @pre     The SPI interface must be initialized and the driver started.
 *
 * @param[in] spip      pointer to the SPI interface
 * @param[in] paylen    payload length
 * @param[in] rxbuf     pointer to a buffer
 *
 * @return              the status register value
 */
NRF24L01_status_t nrf24l01WriteTxPl(SPIDriver *spip, uint8_t paylen,
                                    uint8_t* txbuf) {

  uint8_t cmd = NRF24L01_CMD_W_TX_PAYLOAD;
  uint8_t status;
  if(paylen > NRF24L01_MAX_PL_LENGHT) {
    return 0;
  }
  spiSelect(spip);
  spiExchange(spip, 1, &cmd, &status);
  spiSend(spip, paylen, txbuf);
  spiUnselect(spip);
  return status;
}

/**
 * @brief   Flush TX FIFO.
 *
 * @note    Used in TX mode.
 * @pre     The SPI interface must be initialized and the driver started.
 *
 * @param[in] spip      pointer to the SPI interface
 *
 * @return              the status register value
 */
NRF24L01_status_t nrf24l01FlushTx(SPIDriver *spip) {

  uint8_t txbuf = NRF24L01_CMD_FLUSH_TX;
  uint8_t status;
  spiSelect(spip);
  spiExchange(spip, 1, &txbuf, &status);
  spiUnselect(spip);
  return status;
}

/**
 * @brief   Flush RX FIFO.
 *
 * @note    Used in RX mode. Should not be executed during transmission of
            acknowledge, that is, acknowledge package will not be completed.
 * @pre     The SPI interface must be initialized and the driver started.
 *
 * @param[in] spip      pointer to the SPI interface
 *
 * @return              the status register value
 */
NRF24L01_status_t nrf24l01FlushRx(SPIDriver *spip) {

  uint8_t txbuf = NRF24L01_CMD_FLUSH_RX;
  uint8_t status;
  spiSelect(spip);
  spiExchange(spip, 1, &txbuf, &status);
  spiUnselect(spip);
  return status;
}

#if NRF24L01_USE_FEATURE || defined(__DOXYGEN__)
/**
 * @brief   Activates the following features:
 *          R_RX_PL_WID        -> (In order to enable DPL the EN_DPL bit in the
 *                                 FEATURE register must be set)
 *          W_ACK_PAYLOAD      -> (In order to enable PL with ACK the EN_ACK_PAY
 *                                 bit in the FEATURE register must be set)
 *          W_TX_PAYLOAD_NOACK -> (In order to send a PL without ACK
 *                                 the EN_DYN_ACK it in the FEATURE register
 *                                 must be set)
 *
 * @note    A new ACTIVATE command with the same data deactivates them again.
 *          This is executable in power down or stand by modes only.
 * @pre     The SPI interface must be initialized and the driver started.
 *
 * @param[in] spip      pointer to the SPI interface
 *
 * @return              the status register value
 */
NRF24L01_status_t nrf24l01Activate(SPIDriver *spip) {

  uint8_t txbuf[2] = {NRF24L01_CMD_FLUSH_RX, ACTIVATE};
  uint8_t rxbuf[2];
  spiSelect(spip);
  spiExchange(spip, 2, txbuf, rxbuf);
  spiUnselect(spip);
  return rxbuf[0];
}

/**
 * @brief   Reads RX payload lenght for the top R_RX_PAYLOAD
 *          in the RX FIFO when Dynamic Payload Length is activated.
 *
 * @note    R_RX_PL_WID must be set and activated.
 * @pre     The SPI interface must be initialized and the driver started.
 *
 * @param[in] spip      pointer to the SPI interface
 * @param[in] ppaylen   pointer to the payload length variable
 *
 * @return              the status register value
 */
NRF24L01_status_t nrf24l01ReadRxPlWid(SPIDriver *spip, uint8_t *ppaylen) {

  uint8_t txbuf[2] = {NRF24L01_CMD_R_RX_PL_WID, 0xFF};
  uint8_t rxbuf[2];
  spiSelect(spip);
  spiExchange(spip, 2, txbuf, rxbuf);
  spiUnselect(spip);
  *ppaylen = rxbuf[1];
  return rxbuf[0];
}

/**
 * @brief   Writes TX payload associateted to ACK.
 *
 * @note    Used in RX mode. Write Payload to be transmitted together with
 *          ACK packet on PIPE PPP. (PPP valid in the range from 000 to 101).
 * @note    EN_ACK_PAY must be set and activated.
 * @pre     The SPI interface must be initialized and the driver started.
 *
 * @param[in] spip      pointer to the SPI interface
 * @param[in] paylen    payload length
 * @param[in] rxbuf     pointer to a buffer
 *
 * @return              the status register value
 */
NRF24L01_status_t nrf24l01WriteAckPl(SPIDriver *spip, uint8_t ppp, uint8_t paylen,
                                       uint8_t* payload){

  payload[0] = NRF24L01_CMD_W_ACK_PAYLOAD | NRF24L01_MAX_PPP;
  uint8_t status;
  if((paylen > NRF24L01_MAX_PL_LENGHT) || (ppp > NRF24L01_MAX_PPP)) {
    return 0;
  }
  spiSelect(spip);
  spiExchange(spip, 1, payload, &status);
  spiSend(spip, paylen, payload);
  spiUnselect(spip);
  return status;
}

/**
 * @brief   Writes next TX payload without ACK.
 *
 * @note    Used in TX mode.
 * @note    EN_DYN_ACK must be set and activated.
 * @pre     The SPI interface must be initialized and the driver started.
 *
 * @param[in] spip      pointer to the SPI interface
 * @param[in] paylen    payload length
 * @param[in] rxbuf     pointer to a buffer
 *
 * @return              the status register value
 */
NRF24L01_status_t nrf24l01WriteTxPlNoAck(SPIDriver *spip, uint8_t paylen,
                                    uint8_t* txbuf) {

  txbuf[0] = NRF24L01_CMD_W_TX_PAYLOAD_NOACK;
  uint8_t status;
  if(paylen > NRF24L01_MAX_PL_LENGHT) {
    return 0;
  }
  spiSelect(spip);
  spiExchange(spip, 1, txbuf, &status);
  spiSend(spip, paylen, txbuf);
  spiUnselect(spip);
  return status;
}
#endif /* NRF24L01_USE_FEATURE */

/** @} */
