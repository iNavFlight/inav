/*
    Copyright (C) 2014..2017 Marco Veeneman

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
 * @file    MAC/hal_mac_lld.h
 * @brief   MAC Driver subsystem low level driver header.
 *
 * @addtogroup MAC
 * @{
 */

#ifndef HAL_MAC_LLD_H
#define HAL_MAC_LLD_H

#if HAL_USE_MAC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   This implementation supports the zero-copy mode API.
 */
#define MAC_SUPPORTS_ZERO_COPY      TRUE

/**
 * @name    RDES0 constants
 * @{
 */
#define TIVA_RDES0_OWN              0x80000000
#define TIVA_RDES0_AFM              0x40000000

#define TIVA_RDES0_FL_MASK          0x3FFF0000
#define TIVA_RDES0_FL(n)            ((n) << 16)

#define TIVA_RDES0_ES               0x00008000
#define TIVA_RDES0_DESERR           0x00004000
#define TIVA_RDES0_SAF              0x00002000
#define TIVA_RDES0_LE               0x00001000
#define TIVA_RDES0_OE               0x00000800
#define TIVA_RDES0_VLAN             0x00000400
#define TIVA_RDES0_FS               0x00000200
#define TIVA_RDES0_LS               0x00000100
#define TIVA_RDES0_TAGF             0x00000080
#define TIVA_RDES0_LC               0x00000040
#define TIVA_RDES0_FT               0x00000020
#define TIVA_RDES0_RWT              0x00000010
#define TIVA_RDES0_RE               0x00000008
#define TIVA_RDES0_DE               0x00000004
#define TIVA_RDES0_CE               0x00000002
#define TIVA_RDES0_ESA              0x00000001
/** @} */

/**
 * @name    RDES1 constants
 * @{
 */
#define TIVA_RDES1_DIC              0x80000000

#define TIVA_RDES1_RBS2_MASK        0x1FFF0000
#define TIVA_RDES1_RBS2(n)          ((n) << 16)

#define TIVA_RDES1_RER              0x00008000
#define TIVA_RDES1_RCH              0x00004000

#define TIVA_RDES1_RBS1_MASK        0x00001FFF
#define TIVA_RDES1_RBS1(n)          ((n) << 0)

/** @} */

/**
 * @name    TDES0 constants
 * @{
 */
#define TIVA_TDES0_OWN              0x80000000
#define TIVA_TDES0_IC               0x40000000
#define TIVA_TDES0_LS               0x20000000
#define TIVA_TDES0_FS               0x10000000
#define TIVA_TDES0_DC               0x08000000
#define TIVA_TDES0_DP               0x04000000
#define TIVA_TDES0_TTSE             0x02000000
#define TIVA_TDES0_CRCR             0x01000000

#define TIVA_TDES0_CIC_MASK         0x00C00000
#define TIVA_TDES0_CIC(n)           ((n) << 22)

#define TIVA_TDES0_TER              0x00200000
#define TIVA_TDES0_TCH              0x00100000
#define TIVA_TDES0_VLIC             0x000C0000
#define TIVA_TDES0_TTSS             0x00020000
#define TIVA_TDES0_IHE              0x00010000
#define TIVA_TDES0_ES               0x00008000
#define TIVA_TDES0_JT               0x00004000
#define TIVA_TDES0_FF               0x00002000
#define TIVA_TDES0_IPE              0x00001000
#define TIVA_TDES0_LC               0x00000800
#define TIVA_TDES0_NC               0x00000400
#define TIVA_TDES0_LCO              0x00000200
#define TIVA_TDES0_EC               0x00000100
#define TIVA_TDES0_VF               0x00000080

#define TIVA_TDES0_CC_MASK          0x00000078
#define TIVA_TDES0_CC(n)            ((n) << 3)

#define TIVA_TDES0_ED               0x00000004
#define TIVA_TDES0_UF               0x00000002
#define TIVA_TDES0_DB               0x00000001
/** @} */

/**
 * @name    TDES1 constants
 * @{
 */
#define TIVA_TDES1_SAIC_MASK        0xE0000000
#define TIVA_TDES1_SAIC(n)          ((n) << 29)

#define TIVA_TDES1_TBS2_MASK        0x1FFF0000
#define TIVA_TDES1_TBS2(n)          ((n) << 16)

#define TIVA_TDES1_TBS1_MASK        0x00001FFF
#define TIVA_TDES1_TBS1(n)          ((n) << 0)
/** @} */




/**
 * @name    Ethernet PHY registers
 */
#define TIVA_BMCR                   0x00000000  /* MR0  - Basic Mode Control     */
#define TIVA_BMSR                   0x00000001  /* MR1  - Basic Mode Status      */
#define TIVA_ID1                    0x00000002  /* MR2  - Identifier Register 1  */
#define TIVA_ID2                    0x00000003  /* MR3  - Identifier Register 2  */
#define TIVA_ANA                    0x00000004  /* MR4  - Auto-Negotiation Advertisement */
#define TIVA_ANLPA                  0x00000005  /* MR5  - Auto-Negotiation Link Partner Ability */
#define TIVA_ANER                   0x00000006  /* MR6  - Auto-Negotiation Expansion */
#define TIVA_ANNPTR                 0x00000007  /* MR7  - Auto-Negotiation Next Page TX */
#define TIVA_ANLNPTR                0x00000008  /* MR8  - Auto-Negotiation Link Partner Ability Next Page */
#define TIVA_CFG1                   0x00000009  /* MR9  - Configuration 1        */
#define TIVA_CFG2                   0x0000000A  /* MR10 - Configuration 2        */
#define TIVA_CFG3                   0x0000000B  /* MR11 - Configuration 3        */
#define TIVA_REGCTL                 0x0000000D  /* MR13 - Register Control       */
#define TIVA_ADDAR                  0x0000000E  /* MR14 - Address or Data        */
#define TIVA_STS                    0x00000010  /* MR16 - Status                 */
#define TIVA_SCR                    0x00000011  /* MR17 - Specific Control       */
#define TIVA_MISR1                  0x00000012  /* MR18 - MII Interrupt Status 1 */
#define TIVA_MISR2                  0x00000013  /* MR19 - MII Interrupt Status 2 */
#define TIVA_FCSCR                  0x00000014  /* MR20 - False Carrier Sense Counter */
#define TIVA_RXERCNT                0x00000015  /* MR21 - Receive Error Count    */
#define TIVA_BISTCR                 0x00000016  /* MR22 - BIST Control           */
#define TIVA_LEDCR                  0x00000018  /* MR24 - LED Control            */
#define TIVA_CTL                    0x00000019  /* MR25 - Control                */
#define TIVA_10BTSC                 0x0000001A  /* MR26 - 10Base-T Status/Control - MR26 */
#define TIVA_BICSR1                 0x0000001B  /* MR27 - BIST Control and Status 1 */
#define TIVA_BICSR2                 0x0000001C  /* MR28 - BIST Control and Status 2 */
#define TIVA_CDCR                   0x0000001E  /* MR30 - Cable Diagnostic Control */
#define TIVA_RCR                    0x0000001F  /* MR31 - Reset Control          */
#define TIVA_LEDCFG                 0x00000025  /* MR37 - LED Configuration      */
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   Number of available transmit buffers.
 */
#if !defined(TIVA_MAC_TRANSMIT_BUFFERS) || defined(__DOXYGEN__)
#define TIVA_MAC_TRANSMIT_BUFFERS           2
#endif

/**
 * @brief   Number of available receive buffers.
 */
#if !defined(TIVA_MAC_RECEIVE_BUFFERS) || defined(__DOXYGEN__)
#define TIVA_MAC_RECEIVE_BUFFERS            4
#endif

/**
 * @brief   Maximum supported frame size.
 */
#if !defined(TIVA_MAC_BUFFERS_SIZE) || defined(__DOXYGEN__)
#define TIVA_MAC_BUFFERS_SIZE               1522
#endif

/**
 * @brief   PHY detection timeout.
 * @details Timeout, in milliseconds, for PHY address detection, if a PHY
 *          is not detected within the timeout then the driver halts during
 *          initialization. This setting applies only if the PHY address is
 *          not explicitly set in the board header file using
 *          @p BOARD_PHY_ADDRESS. A zero value disables the timeout and a
 *          single search path is performed.
 */
#if !defined(TIVA_MAC_PHY_TIMEOUT) || defined(__DOXYGEN__)
#define TIVA_MAC_PHY_TIMEOUT                0
#endif

/**
 * @brief   Change the PHY power state inside the driver.
 */
#if !defined(TIVA_MAC_CHANGE_PHY_STATE) || defined(__DOXYGEN__)
#define TIVA_MAC_CHANGE_PHY_STATE           TRUE
#endif

/**
 * @brief   ETHD1 interrupt priority level setting.
 */
#if !defined(TIVA_MAC_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_MAC_IRQ_PRIORITY               5
#endif

/**
 * @brief   IP checksum offload.
 * @details The following modes are available:
 *          - 0 Function disabled.
 *          - 1 Only IP header checksum calculation and insertion are enabled.
 *          - 2 IP header checksum and payload checksum calculation and
 *              insertion are enabled, but pseudo-header checksum is not
 *              calculated in hardware.
 *          - 3 IP Header checksum and payload checksum calculation and
 *              insertion are enabled, and pseudo-header checksum is
 *              calculated in hardware.
 *          .
 */
#if !defined(TIVA_MAC_IP_CHECKSUM_OFFLOAD) || defined(__DOXYGEN__)
#define TIVA_MAC_IP_CHECKSUM_OFFLOAD        0
#endif
/** @} */

#ifndef EMAC_PHY_CONFIG
#define EMAC_PHY_CONFIG         ((0 << 31) | \
                                 (1 << 23) | \
                                 (1 << 10) | \
                                 (1 << 3)  | \
                                 (3 << 1)  | \
                                 (1 << 0))
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if (TIVA_MAC_PHY_TIMEOUT > 0) && !HAL_IMPLEMENTS_COUNTERS
#error "TIVA_MAC_PHY_TIMEOUT requires the realtime counter service"
#endif

#if !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_MAC_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to MAC"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of an Tiva Ethernet receive descriptor.
 */
typedef struct
{
  volatile uint32_t     rdes0;
  volatile uint32_t     rdes1;
  volatile uint32_t     rdes2;
  volatile uint32_t     rdes3;
} tiva_eth_rx_descriptor_t;

/**
 * @brief   Type of an Tiva Ethernet transmit descriptor.
 */
typedef struct
{
  volatile uint32_t     tdes0;
  volatile uint32_t     tdes1;
  volatile uint32_t     tdes2;
  volatile uint32_t     tdes3;
  volatile uint32_t     locked;
} tiva_eth_tx_descriptor_t;

/**
 * @brief   Driver configuration structure.
 */
typedef struct
{
  /**
   * @brief MAC address.
   */
  uint8_t               *mac_address;
  /* End of the mandatory fields.*/
} MACConfig;

/**
 * @brief   Structure representing a MAC driver.
 */
struct MACDriver
{
  /**
   * @brief Driver state.
   */
  macstate_t            state;
  /**
   * @brief Current configuration data.
   */
  const MACConfig       *config;
  /**
   * @brief Transmit semaphore.
   */
  threads_queue_t       tdqueue;
  /**
   * @brief Receive semaphore.
   */
  threads_queue_t       rdqueue;
#if MAC_USE_EVENTS || defined(__DOXYGEN__)
  /**
   * @brief Receive event.
   */
  event_source_t        rdevent;
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief Link status flag.
   */
  bool                  link_up;
  /**
   * @brief PHY address (pre shifted).
   */
  uint32_t              phyaddr;
  /**
   * @brief Receive next frame pointer.
   */
  tiva_eth_rx_descriptor_t *rxptr;
  /**
   * @brief Transmit next frame pointer.
   */
  tiva_eth_tx_descriptor_t *txptr;
};

/**
 * @brief   Structure representing a transmit descriptor.
 */
typedef struct
{
  /**
   * @brief Current write offset.
   */
  size_t                    offset;
  /**
   * @brief Available space size.
   */
  size_t                    size;
  /* End of the mandatory fields.*/
  /**
   * @brief Pointer to the physical descriptor.
   */
  tiva_eth_tx_descriptor_t *physdesc;
} MACTransmitDescriptor;

/**
 * @brief   Structure representing a receive descriptor.
 */
typedef struct
{
  /**
   * @brief Current read offset.
   */
  size_t                offset;
  /**
   * @brief Available data size.
   */
  size_t                size;
  /* End of the mandatory fields.*/
  /**
   * @brief Pointer to the physical descriptor.
   */
  tiva_eth_rx_descriptor_t *physdesc;
} MACReceiveDescriptor;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern MACDriver ETHD1;

#ifdef __cplusplus
extern "C" {
#endif
  void mac_lld_init(void);
  void mac_lld_start(MACDriver *macp);
  void mac_lld_stop(MACDriver *macp);
  msg_t mac_lld_get_transmit_descriptor(MACDriver *macp,
                                        MACTransmitDescriptor *tdp);
  void mac_lld_release_transmit_descriptor(MACTransmitDescriptor *tdp);
  msg_t mac_lld_get_receive_descriptor(MACDriver *macp,
                                       MACReceiveDescriptor *rdp);
  void mac_lld_release_receive_descriptor(MACReceiveDescriptor *rdp);
  bool mac_lld_poll_link_status(MACDriver *macp);
  size_t mac_lld_write_transmit_descriptor(MACTransmitDescriptor *tdp,
                                           uint8_t *buf,
                                           size_t size);
  size_t mac_lld_read_receive_descriptor(MACReceiveDescriptor *rdp,
                                         uint8_t *buf,
                                         size_t size);
#if MAC_USE_ZERO_COPY
  uint8_t *mac_lld_get_next_transmit_buffer(MACTransmitDescriptor *tdp,
                                            size_t size,
                                            size_t *sizep);
  const uint8_t *mac_lld_get_next_receive_buffer(MACReceiveDescriptor *rdp,
                                                 size_t *sizep);
#endif /* MAC_USE_ZERO_COPY */
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_MAC */

#endif /* HAL_MAC_LLD_H */

/** @} */
