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
 * @file    nrf24l01.h
 * @brief   NRF24L01 Radio frequency module interface module header.
 *
 * @{
 */

#ifndef _NRF24L01_H_
#define _NRF24L01_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define  NRF24L01_MAX_ADD_LENGHT                 ((uint8_t)  5)
#define  NRF24L01_MAX_PL_LENGHT                  ((uint8_t) 32)
#define  NRF24L01_MAX_PPP                        ((uint8_t)  5)

/**
 * @brief   Enables Advanced Features.
 */
#if !defined(NRF24L01_USE_FEATURE) || defined(__DOXYGEN__)
#define NRF24L01_USE_FEATURE                     TRUE
#endif

/**
 * @name    NRF24L01 register names
 * @{
 */
/******************************************************************************/
/*                                                                            */
/*                        NRF24L01 RF Transceiver                             */
/*                                                                            */
/******************************************************************************/
/******************  Bit definition for SPI communication  ********************/
#define  NRF24L01_DI                             ((uint8_t)0xFF)             /*!< DI[7:0] Data input */
#define  NRF24L01_DI_0                           ((uint8_t)0x01)             /*!< bit 0 */
#define  NRF24L01_DI_1                           ((uint8_t)0x02)             /*!< bit 1 */
#define  NRF24L01_DI_2                           ((uint8_t)0x04)             /*!< bit 2 */
#define  NRF24L01_DI_3                           ((uint8_t)0x08)             /*!< bit 3 */
#define  NRF24L01_DI_4                           ((uint8_t)0x10)             /*!< bit 4 */
#define  NRF24L01_DI_5                           ((uint8_t)0x20)             /*!< bit 5 */
#define  NRF24L01_DI_6                           ((uint8_t)0x40)             /*!< bit 6 */
#define  NRF24L01_DI_7                           ((uint8_t)0x80)             /*!< bit 7 */

#define  NRF24L01_AD                             ((uint8_t)0x1F)             /*!< AD[4:0] Address Data */
#define  NRF24L01_AD_0                           ((uint8_t)0x01)             /*!< bit 0 */
#define  NRF24L01_AD_1                           ((uint8_t)0x02)             /*!< bit 1 */
#define  NRF24L01_AD_2                           ((uint8_t)0x04)             /*!< bit 2 */
#define  NRF24L01_AD_3                           ((uint8_t)0x08)             /*!< bit 3 */
#define  NRF24L01_AD_4                           ((uint8_t)0x10)             /*!< bit 4 */

#define  NRF24L01_CMD_READ                       ((uint8_t)0x00)             /*!< Read command */
#define  NRF24L01_CMD_WRITE                      ((uint8_t)0x20)             /*!< Write command */
#define  NRF24L01_CMD_R_RX_PAYLOAD               ((uint8_t)0x61)             /*!< Read RX-payload*/
#define  NRF24L01_CMD_W_TX_PAYLOAD               ((uint8_t)0xA0)             /*!< Write TX-payload */
#define  NRF24L01_CMD_FLUSH_TX                   ((uint8_t)0xE1)             /*!< Flush TX FIFO */
#define  NRF24L01_CMD_FLUSH_RX                   ((uint8_t)0xE2)             /*!< Flush RX FIFO */
#define  NRF24L01_CMD_REUSE_TX_PL                ((uint8_t)0xE3)             /*!< Used for a PTX device */
#define  NRF24L01_CMD_ACTIVATE                   ((uint8_t)0x50)             /*!< Activate command */
#define  NRF24L01_CMD_R_RX_PL_WID                ((uint8_t)0x60)             /*!< Read RX-payload width */
#define  NRF24L01_CMD_W_ACK_PAYLOAD              ((uint8_t)0xA8)             /*!< Write Payload for ACK */
#define  NRF24L01_CMD_W_TX_PAYLOAD_NOACK         ((uint8_t)0xB0)             /*!< Disables AUTOACK*/
#define  NRF24L01_CMD_NOP                        ((uint8_t)0xFF)             /*!< No Operation */

/******************  Bit definition for Registers Addresses *******************/
#define  NRF24L01_AD_CONFIG                      ((uint8_t)0x00)             /*!< Configuration Register */
#define  NRF24L01_AD_EN_AA                       ((uint8_t)0x01)             /*!< Enable ‘Auto Acknowledgment’ */
#define  NRF24L01_AD_EN_RXADDR                   ((uint8_t)0x02)             /*!< Enabled RX Addresses */
#define  NRF24L01_AD_SETUP_AW                    ((uint8_t)0x03)             /*!< Setup of Address Widths */
#define  NRF24L01_AD_SETUP_RETR                  ((uint8_t)0x04)             /*!< Setup of Automatic Retransmission */
#define  NRF24L01_AD_RF_CH                       ((uint8_t)0x05)             /*!< RF Channel */
#define  NRF24L01_AD_RF_SETUP                    ((uint8_t)0x06)             /*!< RF Setup Register */
#define  NRF24L01_AD_STATUS                      ((uint8_t)0x07)             /*!< Status Register */
#define  NRF24L01_AD_OBSERVE_TX                  ((uint8_t)0x08)             /*!< Transmit observe register */
#define  NRF24L01_AD_CD                          ((uint8_t)0x09)             /*!< CD */
#define  NRF24L01_AD_RX_ADDR_P0                  ((uint8_t)0x0A)             /*!< Receive address data pipe 0 */
#define  NRF24L01_AD_RX_ADDR_P1                  ((uint8_t)0x0B)             /*!< Receive address data pipe 1 */
#define  NRF24L01_AD_RX_ADDR_P2                  ((uint8_t)0x0C)             /*!< Receive address data pipe 2 */
#define  NRF24L01_AD_RX_ADDR_P3                  ((uint8_t)0x0D)             /*!< Receive address data pipe 3 */
#define  NRF24L01_AD_RX_ADDR_P4                  ((uint8_t)0x0E)             /*!< Receive address data pipe 4 */
#define  NRF24L01_AD_RX_ADDR_P5                  ((uint8_t)0x0F)             /*!< Receive address data pipe 5 */
#define  NRF24L01_AD_TX_ADDR                     ((uint8_t)0x10)             /*!< Transmit address */
#define  NRF24L01_AD_RX_PW_P0                    ((uint8_t)0x11)             /*!< Number of bytes in RX payload in data pipe 0 */
#define  NRF24L01_AD_RX_PW_P1                    ((uint8_t)0x12)             /*!< Number of bytes in RX payload in data pipe 1 */
#define  NRF24L01_AD_RX_PW_P2                    ((uint8_t)0x13)             /*!< Number of bytes in RX payload in data pipe 2 */
#define  NRF24L01_AD_RX_PW_P3                    ((uint8_t)0x14)             /*!< Number of bytes in RX payload in data pipe 3 */
#define  NRF24L01_AD_RX_PW_P4                    ((uint8_t)0x15)             /*!< Number of bytes in RX payload in data pipe 4 */
#define  NRF24L01_AD_RX_PW_P5                    ((uint8_t)0x16)             /*!< Number of bytes in RX payload in data pipe 5 */
#define  NRF24L01_AD_FIFO_STATUS                 ((uint8_t)0x17)             /*!< FIFO Status Register */
#define  NRF24L01_AD_DYNPD                       ((uint8_t)0x1C)             /*!< Enable dynamic payload length */
#define  NRF24L01_AD_FEATURE                     ((uint8_t)0x1D)             /*!< Feature Register */

/***************  Bit definition for Registers Configuration  *****************/
#define  NRF24L01_DI_CONFIG                      ((uint8_t)0x7F)             /*!< CONTROL REGISTER BIT MASK*/
#define  NRF24L01_DI_CONFIG_PRIM_RX              ((uint8_t)0x01)             /*!< RX/TX control - 1: PRX, 0: PTX */
#define  NRF24L01_DI_CONFIG_PWR_UP               ((uint8_t)0x02)             /*!< 1: POWER UP, 0:POWER DOWN */
#define  NRF24L01_DI_CONFIG_CRCO                 ((uint8_t)0x04)             /*!< CRC encoding scheme - 1:two bytes, 0:one byte */
#define  NRF24L01_DI_CONFIG_EN_CRC               ((uint8_t)0x08)             /*!< Enable CRC. Forced high if one of the bits in the EN_AA is high */
#define  NRF24L01_DI_CONFIG_MASK_MAX_RT          ((uint8_t)0x10)             /*!< Mask interrupt caused by MAX_RT - 1: Interrupt disabled, 0: Interrupt reflected on IRQ pin */
#define  NRF24L01_DI_CONFIG_MASK_TX_DS           ((uint8_t)0x20)             /*!< Mask interrupt caused by TX_DS - 1: Interrupt disabled, 0: Interrupt reflected on IRQ pin */
#define  NRF24L01_DI_CONFIG_MASK_RX_DR           ((uint8_t)0x40)             /*!< Mask interrupt caused by RX_DR - 1: Interrupt disabled, 0: Interrupt reflected on IRQ pin */

#define  NRF24L01_DI_EN_AA                       ((uint8_t)0x3F)             /*!< ENABLE AUTO ACKNOLEDGMENT REGISTER BIT MASK */
#define  NRF24L01_DI_EN_AA_P0                    ((uint8_t)0x01)             /*!< Enable auto acknowledgement data pipe 0 */
#define  NRF24L01_DI_EN_AA_P1                    ((uint8_t)0x02)             /*!< Enable auto acknowledgement data pipe 1 */
#define  NRF24L01_DI_EN_AA_P2                    ((uint8_t)0x04)             /*!< Enable auto acknowledgement data pipe 2 */
#define  NRF24L01_DI_EN_AA_P3                    ((uint8_t)0x08)             /*!< Enable auto acknowledgement data pipe 3 */
#define  NRF24L01_DI_EN_AA_P4                    ((uint8_t)0x10)             /*!< Enable auto acknowledgement data pipe 4 */
#define  NRF24L01_DI_EN_AA_P5                    ((uint8_t)0x20)             /*!< Enable auto acknowledgement data pipe 5 */

#define  NRF24L01_DI_EN_RXADDR                   ((uint8_t)0x3F)             /*!< ENABLE RX ADDRESSES REGISTER BIT MASK */
#define  NRF24L01_DI_EN_RXADDR_P0                ((uint8_t)0x01)             /*!< Enable data pipe 0 */
#define  NRF24L01_DI_EN_RXADDR_P1                ((uint8_t)0x02)             /*!< Enable data pipe 1 */
#define  NRF24L01_DI_EN_RXADDR_P2                ((uint8_t)0x04)             /*!< Enable data pipe 2 */
#define  NRF24L01_DI_EN_RXADDR_P3                ((uint8_t)0x08)             /*!< Enable data pipe 3 */
#define  NRF24L01_DI_EN_RXADDR_P4                ((uint8_t)0x10)             /*!< Enable data pipe 4 */
#define  NRF24L01_DI_EN_RXADDR_P5                ((uint8_t)0x20)             /*!< Enable data pipe 5 */

#define  NRF24L01_DI_SETUP_AW                    ((uint8_t)0x03)             /*!< SETUP OF ADDRESSES WIDTHS REGISTER BIT MASK */
#define  NRF24L01_DI_SETUP_AW_0                  ((uint8_t)0x01)             /*!< Addressed widths bit 0 */
#define  NRF24L01_DI_SETUP_AW_1                  ((uint8_t)0x02)             /*!< Addressed widths bit 1 */

#define  NRF24L01_DI_SETUP_RETR                  ((uint8_t)0xFF)             /*!< SETUP OF AUTOMATIC RETRANSMISSION REGISTER BIT MASK */
#define  NRF24L01_DI_SETUP_RETR_ARC_0            ((uint8_t)0x01)             /*!< Auto Retransmit Count bit 0 */
#define  NRF24L01_DI_SETUP_RETR_ARC_1            ((uint8_t)0x02)             /*!< Auto Retransmit Count bit 1 */
#define  NRF24L01_DI_SETUP_RETR_ARC_2            ((uint8_t)0x04)             /*!< Auto Retransmit Count bit 2 */
#define  NRF24L01_DI_SETUP_RETR_ARC_3            ((uint8_t)0x08)             /*!< Auto Retransmit Count bit 3 */
#define  NRF24L01_DI_SETUP_RETR_ARD_0            ((uint8_t)0x10)             /*!< Auto Retransmit Delay bit 0 */
#define  NRF24L01_DI_SETUP_RETR_ARD_1            ((uint8_t)0x20)             /*!< Auto Retransmit Delay bit 1 */
#define  NRF24L01_DI_SETUP_RETR_ARD_2            ((uint8_t)0x40)             /*!< Auto Retransmit Delay bit 2 */
#define  NRF24L01_DI_SETUP_RETR_ARD_3            ((uint8_t)0x80)             /*!< Auto Retransmit Delay bit 3 */


#define  NRF24L01_DI_RF_CH                       ((uint8_t)0x7F)             /*!< RF CHANNEL REGISTER BIT MASK */
#define  NRF24L01_DI_RF_CH_0                     ((uint8_t)0x01)             /*!< RF channel bit 0 */
#define  NRF24L01_DI_RF_CH_1                     ((uint8_t)0x02)             /*!< RF channel bit 1 */
#define  NRF24L01_DI_RF_CH_2                     ((uint8_t)0x04)             /*!< RF channel bit 2 */
#define  NRF24L01_DI_RF_CH_3                     ((uint8_t)0x08)             /*!< RF channel bit 3 */
#define  NRF24L01_DI_RF_CH_4                     ((uint8_t)0x10)             /*!< RF channel bit 4 */
#define  NRF24L01_DI_RF_CH_5                     ((uint8_t)0x20)             /*!< RF channel bit 5 */
#define  NRF24L01_DI_RF_CH_6                     ((uint8_t)0x40)             /*!< RF channel bit 6 */


#define  NRF24L01_DI_RF_SETUP                    ((uint8_t)0x1F)             /*!< RF SETUP REGISTER BIT MASK */
#define  NRF24L01_DI_RF_SETUP_LNA_HCURR          ((uint8_t)0x01)             /*!< Setup LNA gain */
#define  NRF24L01_DI_RF_SETUP_RF_PWR_0           ((uint8_t)0x02)             /*!< RF output power bit 0 */
#define  NRF24L01_DI_RF_SETUP_RF_PWR_1           ((uint8_t)0x04)             /*!< RF output power bit 1 */
#define  NRF24L01_DI_RF_SETUP_RF_DR              ((uint8_t)0x08)             /*!< Air Data rate - 0: 1Mbps, 1: 2Mbps */
#define  NRF24L01_DI_RF_SETUP_PLL_LOCK           ((uint8_t)0x10)             /*!< Force PLL lock signal */

#define  NRF24L01_DI_STATUS                      ((uint8_t)0x7F)             /*!< STATUS REGISTER BIT MASK */
#define  NRF24L01_DI_STATUS_TX_FULL              ((uint8_t)0x01)             /*!< TX FIFO full flag - 0: Available locations, 1: Full */
#define  NRF24L01_DI_STATUS_RX_P_NO_0            ((uint8_t)0x02)             /*!< RX payload number bit 0 */
#define  NRF24L01_DI_STATUS_RX_P_NO_1            ((uint8_t)0x04)             /*!< RX payload number bit 1 */
#define  NRF24L01_DI_STATUS_RX_P_NO_2            ((uint8_t)0x08)             /*!< RX payload number bit 2 */
#define  NRF24L01_DI_STATUS_MAX_RT               ((uint8_t)0x10)             /*!< Maximum number of TX retransmits interrupt */
#define  NRF24L01_DI_STATUS_TX_DS                ((uint8_t)0x20)             /*!< Data Sent TX FIFO interrupt */
#define  NRF24L01_DI_STATUS_RX_DR                ((uint8_t)0x40)             /*!< Data Ready RX FIFO interrupt */

#define  NRF24L01_DI_OBSERVE_TX                  ((uint8_t)0xFF)             /*!< TRANSMIT OBSERVE REGISTER BIT MASK */
#define  NRF24L01_DI_ARC_CNT_0                   ((uint8_t)0x01)             /*!< Count retransmitted packets bit 0 */
#define  NRF24L01_DI_ARC_CNT_1                   ((uint8_t)0x02)             /*!< Count retransmitted packets bit 1 */
#define  NRF24L01_DI_ARC_CNT_2                   ((uint8_t)0x04)             /*!< Count retransmitted packets bit 2 */
#define  NRF24L01_DI_ARC_CNT_3                   ((uint8_t)0x08)             /*!< Count retransmitted packets bit 3 */
#define  NRF24L01_DI_PLOS_CNT_0                  ((uint8_t)0x10)             /*!< Count lost packets bit 0 */
#define  NRF24L01_DI_PLOS_CNT_1                  ((uint8_t)0x20)             /*!< Count lost packets bit 1 */
#define  NRF24L01_DI_PLOS_CNT_2                  ((uint8_t)0x40)             /*!< Count lost packets bit 2 */
#define  NRF24L01_DI_PLOS_CNT_3                  ((uint8_t)0x80)             /*!< Count lost packets bit 3 */

#define  NRF24L01_DI_CD                          ((uint8_t)0x01)             /*!< REGISTER BIT MASK */
#define  NRF24L01_DI_CARRIER_DETECT              ((uint8_t)0x01)             /*!< Carrier detect */

#define  NRF24L01_DI_RX_PW_P0                    ((uint8_t)0x3F)             /*!< RX PAYLOAD WIDTH FOR PIPE 0 REGISTER BIT MASK */
#define  NRF24L01_DI_RX_PW_P0_0                  ((uint8_t)0x01)             /*!< Bit 0 */
#define  NRF24L01_DI_RX_PW_P0_1                  ((uint8_t)0x02)             /*!< Bit 1 */
#define  NRF24L01_DI_RX_PW_P0_2                  ((uint8_t)0x04)             /*!< Bit 2 */
#define  NRF24L01_DI_RX_PW_P0_3                  ((uint8_t)0x08)             /*!< Bit 3 */
#define  NRF24L01_DI_RX_PW_P0_4                  ((uint8_t)0x10)             /*!< Bit 4 */
#define  NRF24L01_DI_RX_PW_P0_5                  ((uint8_t)0x20)             /*!< Bit 5 */

#define  NRF24L01_DI_RX_PW_P1                    ((uint8_t)0x3F)             /*!< RX PAYLOAD WIDTH FOR PIPE 1 REGISTER BIT MASK */
#define  NRF24L01_DI_RX_PW_P1_0                  ((uint8_t)0x01)             /*!< Bit 0 */
#define  NRF24L01_DI_RX_PW_P1_1                  ((uint8_t)0x02)             /*!< Bit 1 */
#define  NRF24L01_DI_RX_PW_P1_2                  ((uint8_t)0x04)             /*!< Bit 2 */
#define  NRF24L01_DI_RX_PW_P1_3                  ((uint8_t)0x08)             /*!< Bit 3 */
#define  NRF24L01_DI_RX_PW_P1_4                  ((uint8_t)0x10)             /*!< Bit 4 */
#define  NRF24L01_DI_RX_PW_P1_5                  ((uint8_t)0x20)             /*!< Bit 5 */

#define  NRF24L01_DI_RX_PW_P2                    ((uint8_t)0x3F)             /*!< RX PAYLOAD WIDTH FOR PIPE 2 REGISTER BIT MASK */
#define  NRF24L01_DI_RX_PW_P2_0                  ((uint8_t)0x01)             /*!< Bit 0 */
#define  NRF24L01_DI_RX_PW_P2_1                  ((uint8_t)0x02)             /*!< Bit 1 */
#define  NRF24L01_DI_RX_PW_P2_2                  ((uint8_t)0x04)             /*!< Bit 2 */
#define  NRF24L01_DI_RX_PW_P2_3                  ((uint8_t)0x08)             /*!< Bit 3 */
#define  NRF24L01_DI_RX_PW_P2_4                  ((uint8_t)0x10)             /*!< Bit 4 */
#define  NRF24L01_DI_RX_PW_P2_5                  ((uint8_t)0x20)             /*!< Bit 5 */

#define  NRF24L01_DI_RX_PW_P3                    ((uint8_t)0x3F)             /*!< RX PAYLOAD WIDTH FOR PIPE 3 REGISTER BIT MASK */
#define  NRF24L01_DI_RX_PW_P3_0                  ((uint8_t)0x01)             /*!< Bit 0 */
#define  NRF24L01_DI_RX_PW_P3_1                  ((uint8_t)0x02)             /*!< Bit 1 */
#define  NRF24L01_DI_RX_PW_P3_2                  ((uint8_t)0x04)             /*!< Bit 2 */
#define  NRF24L01_DI_RX_PW_P3_3                  ((uint8_t)0x08)             /*!< Bit 3 */
#define  NRF24L01_DI_RX_PW_P3_4                  ((uint8_t)0x10)             /*!< Bit 4 */
#define  NRF24L01_DI_RX_PW_P3_5                  ((uint8_t)0x20)             /*!< Bit 5 */

#define  NRF24L01_DI_RX_PW_P4                    ((uint8_t)0x3F)             /*!< RX PAYLOAD WIDTH FOR PIPE 4 REGISTER BIT MASK */
#define  NRF24L01_DI_RX_PW_P4_0                  ((uint8_t)0x01)             /*!< Bit 0 */
#define  NRF24L01_DI_RX_PW_P4_1                  ((uint8_t)0x02)             /*!< Bit 1 */
#define  NRF24L01_DI_RX_PW_P4_2                  ((uint8_t)0x04)             /*!< Bit 2 */
#define  NRF24L01_DI_RX_PW_P4_3                  ((uint8_t)0x08)             /*!< Bit 3 */
#define  NRF24L01_DI_RX_PW_P4_4                  ((uint8_t)0x10)             /*!< Bit 4 */
#define  NRF24L01_DI_RX_PW_P4_5                  ((uint8_t)0x20)             /*!< Bit 5 */

#define  NRF24L01_DI_RX_PW_P5                    ((uint8_t)0x3F)             /*!< RX PAYLOAD WIDTH FOR PIPE 5 REGISTER BIT MASK */
#define  NRF24L01_DI_RX_PW_P5_0                  ((uint8_t)0x01)             /*!< Bit 0 */
#define  NRF24L01_DI_RX_PW_P5_1                  ((uint8_t)0x02)             /*!< Bit 1 */
#define  NRF24L01_DI_RX_PW_P5_2                  ((uint8_t)0x04)             /*!< Bit 2 */
#define  NRF24L01_DI_RX_PW_P5_3                  ((uint8_t)0x08)             /*!< Bit 3 */
#define  NRF24L01_DI_RX_PW_P5_4                  ((uint8_t)0x10)             /*!< Bit 4 */
#define  NRF24L01_DI_RX_PW_P5_5                  ((uint8_t)0x20)             /*!< Bit 5 */

#define  NRF24L01_DI_FIFO_STATUS                 ((uint8_t)0x73)             /*!< FIFO STATUS REGISTER BIT MASK*/
#define  NRF24L01_DI_FIFO_STATUS_RX_EMPTY        ((uint8_t)0x01)             /*!< RX FIFO empty flag - 0:Data in RX FIFO, 1:RX FIFO empty */
#define  NRF24L01_DI_FIFO_STATUS_RX_FULL         ((uint8_t)0x02)             /*!< RX FIFO full flag - 0:Available locations in RX FIFO, 1:RX FIFO empty */
#define  NRF24L01_DI_FIFO_STATUS_TX_EMPTY        ((uint8_t)0x10)             /*!< TX FIFO empty flag - 0:Data in TX FIFO, 1:TX FIFO empty */
#define  NRF24L01_DI_FIFO_STATUS_TX_FULL         ((uint8_t)0x20)             /*!< TX FIFO full flag - 0:Available locations in TX FIFO, 1:TX FIFO empty */
#define  NRF24L01_DI_FIFO_STATUS_TX_REUSE        ((uint8_t)0x40)             /*!< Reuse last transmitted data packet if set high */

#define  NRF24L01_DI_DYNPD                       ((uint8_t)0x3F)             /*!< ENABLE DYNAMIC PAYLOAD LENGHT REGISTER BIT MASK */
#define  NRF24L01_DI_DYNPD_DPL_P0                ((uint8_t)0x01)             /*!< Enable dyn. payload length data pipe 0 */
#define  NRF24L01_DI_DYNPD_DPL_P1                ((uint8_t)0x02)             /*!< Enable dyn. payload length data pipe 1 */
#define  NRF24L01_DI_DYNPD_DPL_P2                ((uint8_t)0x04)             /*!< Enable dyn. payload length data pipe 2 */
#define  NRF24L01_DI_DYNPD_DPL_P3                ((uint8_t)0x08)             /*!< Enable dyn. payload length data pipe 3 */
#define  NRF24L01_DI_DYNPD_DPL_P4                ((uint8_t)0x10)             /*!< Enable dyn. payload length data pipe 4 */
#define  NRF24L01_DI_DYNPD_DPL_P5                ((uint8_t)0x20)             /*!< Enable dyn. payload length data pipe 5 */

#define  NRF24L01_DI_FEATURE                     ((uint8_t)0x07)             /*!< FEATURE REGISTER REGISTER BIT MASK */
#define  NRF24L01_DI_FEATURE_EN_DYN_ACK          ((uint8_t)0x01)             /*!< Enables the W_TX_PAYLOAD_NOACK command */
#define  NRF24L01_DI_FEATURE_EN_ACK_PAY          ((uint8_t)0x02)             /*!< Enables Payload with ACK */
#define  NRF24L01_DI_FEATURE_EN_DPL              ((uint8_t)0x04)             /*!< Enables Dynamic Payload Length */
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !(HAL_USE_SPI)
#error "RF_NRF24L01 requires HAL_USE_SPI."
#endif

#if !(HAL_USE_EXT)
#error "RF_NRF24L01 requires HAL_USE_EXT."
#endif
/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @name    RF Transceiver data structures and types
 * @{
 */

/**
 * @brief   RF Transceiver RX/TX Address field width
 */
typedef enum {

  NRF24L01_AW_3_bytes = 0x01,               /*!< 3 bytes width */
  NRF24L01_AW_4_bytes = 0x02,               /*!< 4 bytes width */
  NRF24L01_AW_5_bytes = 0x03                /*!< 5 bytes width */
} NRF24L01_AW_t;

/**
 * @brief   RF Transceiver Auto Retransmit Delay
 */
typedef enum {

  NRF24L01_ARD_250us =  0x00,               /*!< Wait 250us */
  NRF24L01_ARD_500us =  0x10,               /*!< Wait 500us */
  NRF24L01_ARD_750us =  0x20,               /*!< Wait 750us */
  NRF24L01_ARD_1000us = 0x30,               /*!< Wait 1000us */
  NRF24L01_ARD_1250us = 0x40,               /*!< Wait 1250us */
  NRF24L01_ARD_1500us = 0x50,               /*!< Wait 1500us */
  NRF24L01_ARD_1750us = 0x60,               /*!< Wait 1750us */
  NRF24L01_ARD_2000us = 0x70,               /*!< Wait 2000us */
  NRF24L01_ARD_2250us = 0x80,               /*!< Wait 2250us */
  NRF24L01_ARD_2500us = 0x90,               /*!< Wait 2500us */
  NRF24L01_ARD_2750us = 0xA0,               /*!< Wait 2750us */
  NRF24L01_ARD_3000us = 0xB0,               /*!< Wait 3000us */
  NRF24L01_ARD_3250us = 0xC0,               /*!< Wait 3250us */
  NRF24L01_ARD_3500us = 0xD0,               /*!< Wait 3500us */
  NRF24L01_ARD_3750us = 0xE0,               /*!< Wait 3750us */
  NRF24L01_ARD_4000us = 0xF0                /*!< Wait 4000us */
} NRF24L01_ARD_t;

/**
 * @brief   RF Transceiver Auto Retransmit Count
 */
typedef enum {

  NRF24L01_ARC_disabled = 0x00,             /*!< Re-Transmit disabled */
  NRF24L01_ARC_1_time   = 0x01,             /*!< Up to 1 Re-Transmit on fail of AA */
  NRF24L01_ARC_2_times  = 0x02,             /*!< Up to 2 Re-Transmit on fail of AA */
  NRF24L01_ARC_3_times  = 0x03,             /*!< Up to 3 Re-Transmit on fail of AA */
  NRF24L01_ARC_4_times  = 0x04,             /*!< Up to 4 Re-Transmit on fail of AA */
  NRF24L01_ARC_5_times  = 0x05,             /*!< Up to 5 Re-Transmit on fail of AAs */
  NRF24L01_ARC_6_times  = 0x06,             /*!< Up to 6 Re-Transmit on fail of AA */
  NRF24L01_ARC_7_times  = 0x07,             /*!< Up to 7 Re-Transmit on fail of AA */
  NRF24L01_ARC_8_times  = 0x08,             /*!< Up to 8 Re-Transmit on fail of AA */
  NRF24L01_ARC_9_times  = 0x09,             /*!< Up to 9 Re-Transmit on fail of AA */
  NRF24L01_ARC_10_times = 0x0A,             /*!< Up to 10 Re-Transmit on fail of AA */
  NRF24L01_ARC_11_times = 0x0B,             /*!< Up to 11 Re-Transmit on fail of AA */
  NRF24L01_ARC_12_times = 0x0C,             /*!< Up to 12 Re-Transmit on fail of AA */
  NRF24L01_ARC_13_times = 0x0D,             /*!< Up to 13 Re-Transmit on fail of AA */
  NRF24L01_ARC_14_times = 0x0E,             /*!< Up to 14 Re-Transmit on fail of AA */
  NRF24L01_ARC_15_times = 0x0F              /*!< Up to 15 Re-Transmit on fail of AA */
} NRF24L01_ARC_t;


/**
 * @brief   RF Transceiver configuration typedef.
 *
 * @detail  This will select frequency channel beetween 2,4 GHz and 2,525 GHz
 * @detail  according to formula 2,4GHz + RF_CH[MHz]. This value must be included
 * @detail  between 0 and 125.
 */
typedef uint8_t NRF24L01_RF_CH_t;

/**
 * @brief   RF Transceiver Air Data Rate
 */
typedef enum {

  NRF24L01_ADR_1Mbps = 0x00,             /*!< Air data rate 1 Mbps */
  NRF24L01_ADR_2Mbps = 0x08              /*!< Air data rate 2 Mbps */
} NRF24L01_ADR_t;

/**
 * @brief   RF Transceiver Output Power
 */
typedef enum {

  NRF24L01_PWR_0dBm     = 0x06,          /*!< RF output power 0 dBm */
  NRF24L01_PWR_neg6dBm  = 0x04,          /*!< RF output power -6 dBm */
  NRF24L01_PWR_neg12dBm = 0x02,          /*!< RF output power -12 dBm */
  NRF24L01_PWR_neg18dBm = 0x00           /*!< RF output power -18 dBm */
} NRF24L01_PWR_t;

/**
 * @brief   RF Transceiver Low Noise Amplifier
 *
 * @details Reduce current consumption in RX mode with 0.8 mA at cost of 1.5dB
 *          reduction in receiver sensitivity.
 */
typedef  enum {
  NRF24L01_LNA_enabled =  0x01,          /*!< LNA_CURR enabled */
  NRF24L01_LNA_disabled = 0x00           /*!< LNA_CURR disabled */
} NRF24L01_LNA_t;

/**
 * @brief   RF Transceiver Backward Compatibility
 *
 * @details This type specifies if trasmission must be compatible to receive
 *          from an nRF2401/nRF2402/nRF24E1/nRF24E.
 */
typedef  bool_t                   NRF24L01_bckwrdcmp_t;

#if NRF24L01_USE_FEATURE || defined(__doxigen__)
/**
 * @brief   RF Transceiver Dynamic Payload enabler
 *
 * @details Enables Dynamic Payload Length
 */
typedef  enum {
  NRF24L01_DPL_enabled =  0x04,          /*!< EN_DPL enabled */
  NRF24L01_DPL_disabled = 0x00           /*!< EN_DPL disabled */
} NRF24L01_DPL_t;

/**
 * @brief   RF Transceiver Dynamic Acknowledge with Payload enabler
 *
 * @details Enables Payload with ACK
 */
typedef  enum {
  NRF24L01_ACK_PAY_enabled =  0x02,          /*!< EN_ACK_PAY enabled */
  NRF24L01_ACK_PAY_disabled = 0x00           /*!< EN_ACK_PAY disabled */
} NRF24L01_ACK_PAY_t;

/**
 * @brief   RF Transceiver Dynamic Acknowledge enabler
 *
 * @details Enables the W_TX_PAYLOAD_NOACK command
 */
typedef  enum {
  NRF24L01_DYN_ACK_enabled =  0x01,          /*!< EN_DYN_ACK enabled */
  NRF24L01_DYN_ACK_disabled = 0x00           /*!< EN_DYN_ACK disabled */
} NRF24L01_DYN_ACK_t;
#endif /* NRF24L01_USE_FEATURE */

/**
 * @brief   RF Transceiver configuration structure.
 */
typedef struct {

  /**
   * @brief The chip enable line port.
   */
  ioportid_t                ceport;
  /**
   * @brief The chip enable line pad number.
   */
  uint16_t                  cepad;
  /**
   * @brief The interrupt line port.
   */
  ioportid_t                irqport;
  /**
   * @brief The interrupt line pad number.
   */
  uint16_t                  irqpad;
  /**
   * @brief Pointer to the SPI driver associated to this RF.
   */
  SPIDriver                 *spip;
  /**
   * @brief Pointer to the SPI configuration .
   */
  const SPIConfig           *spicfg;
  /**
   * @brief Pointer to the EXT driver associated to this RF.
   */
  EXTDriver                 *extp;
  /**
   * @brief EXT configuration.
   */
  EXTConfig                 *extcfg;
  /**
   * @brief RF Transceiver auto retransmit count.
   */
  NRF24L01_ARC_t            auto_retr_count;
  /**
   * @brief RF Transceiver auto retransmit delay.
   */
  NRF24L01_ARD_t            auto_retr_delay;
  /**
   * @brief RF Transceiver address width.
   */
  NRF24L01_AW_t             address_width;
  /**
   * @brief RF Transceiver channel frequency.
   */
  NRF24L01_RF_CH_t          channel_freq;
  /**
   * @brief RF Transceiver air data rate.
   */
  NRF24L01_ADR_t            data_rate;
  /**
   * @brief RF Transceiver output power.
   */
  NRF24L01_PWR_t            out_pwr;
  /**
   * @brief   RF Transceiver Low Noise Amplifier
   */
  NRF24L01_LNA_t            lna;
#if NRF24L01_USE_FEATURE || defined(__doxigen__)
  /**
  * @brief    RF Transceiver Dynamic Payload enabler
  */
  NRF24L01_DPL_t            en_dpl;

  /**
   * @brief   RF Transceiver Dynamic Acknowledge with Payload enabler
   */
  NRF24L01_ACK_PAY_t        en_ack_pay;

  /**
   * @brief   RF Transceiver Dynamic Acknowledge enabler
   */
  NRF24L01_DYN_ACK_t        en_dyn_ack;
#endif /* NRF24L01_USE_FEATURE */
} NRF24L01_Config;

/**
 * @brief   RF Transceiver status register value.
 */
typedef  uint8_t             NRF24L01_status_t;
/** @}  */
/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

/**
 * @brief   Flushes FIFOs and resets all Status flags.
 *
 * @pre     The SPI interface must be initialized and the driver started.
 *
 * @param[in] spip      pointer to the SPI interface
 *
 * @return              the status register value
 */
#define  nrf24l01Reset(spip) {                                               \
                                                                             \
  nrf24l01WriteRegister(spip, NRF24L01_AD_STATUS,                            \
                                NRF24L01_DI_STATUS_MAX_RT |                  \
                                NRF24L01_DI_STATUS_RX_DR |                   \
                                NRF24L01_DI_STATUS_TX_DS);                   \
}

#ifdef __cplusplus
extern "C" {
#endif
NRF24L01_status_t nrf24l01GetStatus(SPIDriver *spip);
NRF24L01_status_t nrf24l01ReadRegister(SPIDriver *spip, uint8_t reg,
                                       uint8_t* pvalue);
NRF24L01_status_t nrf24l01WriteRegister(SPIDriver *spip, uint8_t reg,
                                        uint8_t value);
NRF24L01_status_t nrf24l01WriteAddress(SPIDriver *spip, uint8_t reg,
                                       uint8_t *pvalue, uint8_t addlen);
NRF24L01_status_t nrf24l01GetRxPl(SPIDriver *spip, uint8_t paylen,
                                  uint8_t* rxbuf);
NRF24L01_status_t nrf24l01WriteTxPl(SPIDriver *spip, uint8_t paylen,
                                    uint8_t* txbuf);
NRF24L01_status_t nrf24l01FlushTx(SPIDriver *spip);
NRF24L01_status_t nrf24l01FlushRx(SPIDriver *spip);
#if NRF24L01_USE_FEATURE || defined(__DOXYGEN__)
NRF24L01_status_t nrf24l01Activate(SPIDriver *spip);
NRF24L01_status_t nrf24l01ReadRxPlWid(SPIDriver *spip, uint8_t* ppaylen);
NRF24L01_status_t nrf24l01WriteAckPl(SPIDriver *spip, uint8_t ppp, uint8_t paylen,
                                     uint8_t* payload);
NRF24L01_status_t nrf24l01WriteTxPlNoAck(SPIDriver *spip, uint8_t paylen,
                                         uint8_t* txbuf);
#endif /* NRF24L01_USE_FEATURE */
#ifdef __cplusplus
}
#endif

#endif /* _NRF24L01_H_ */

/** @} */
