/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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
 * @file    MACv1/hal_mac_lld.h
 * @brief   SAMA low level MAC driver header.
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
#define MAC_SUPPORTS_ZERO_COPY      FALSE

/**
 * @name    RDES0 constants
 * @{
 */
#define SAMA_RDES0_RBAP_MASK        0xFFFFFFFC
#define SAMA_RDES0_WRAP             0x00000002
#define SAMA_RDES0_OWN              0x00000001
/** @} */

/**
 * @name    RDES1 constants
 * @{
 */
#define SAMA_RDES1_BROADCAST        0x80000000
#define SAMA_RDES1_MULTICAST_HASH   0x40000000
#define SAMA_RDES1_UNICAST_HASH     0x20000000
#define SAMA_RDES1_ADDR_REG_MATCH   0x08000000
#define SAMA_RDES1_ADDR_REG         0x06000000
#define SAMA_RDES1_ID_REG_MATCH     0x01000000
#define SAMA_RDES1_SNAP_ENCODED     0x01000000
#define SAMA_RDES1_ID_REG           0x00C00000
#define SAMA_RDES1_CHECKSUM_IP_TCP  0x00800000
#define SAMA_RDES1_CHECKSUM_IP_UDP  0x00C00000
#define SAMA_RDES1_CHECKSUM_IP      0x00400000
#define SAMA_RDES1_VLAN_TAG_DET     0x00200000
#define SAMA_RDES1_PRIOR_TAG_DET    0x00100000
#define SAMA_RDES1_VLAN_PRIOR       0x000E0000
#define SAMA_RDES1_CFI              0x00010000
#define SAMA_RDES1_EOF              0x00008000
#define SAMA_RDES1_SOF              0x00004000
#define SAMA_RDES1_ADD_BIT_LOF      0x00002000
#define SAMA_RDES1_BAD_FCS          0x00002000
#define SAMA_RDES1_LOF              0x00001FFF
/** @} */

/**
 * @name    TDES0 constants
 * @{
 */
#define SAMA_TDES0_BYTE_ADDR        0xFFFFFFFF
/** @} */

/**
 * @name    TDES1 constants
 * @{
 */
#define SAMA_TDES1_USED             0x80000000
#define SAMA_TDES1_WRAP             0x40000000
#define SAMA_TDES1_RLE_TX_ERR       0x20000000
#define SAMA_TDES1_LOCKED           0x10000000 /* NOTE: Pseudo flag.        */
#define SAMA_TDES1_AHB_TX_ERR       0x08000000
#define SAMA_TDES1_LC_TX_ERR        0x04000000
#define SAMA_TDES1_CHECKSUM_ERR     0x00700000
#define SAMA_TDES1_CRC              0x00010000
#define SAMA_TDES1_LAST_BUFF        0x00008000
#define SAMA_TDES1_LENGTH_BUFF      0x00003FFF
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
#if !defined(SAMA_MAC_TRANSMIT_BUFFERS) || defined(__DOXYGEN__)
#define SAMA_MAC_TRANSMIT_BUFFERS           2
#endif

/**
 * @brief   Number of available receive buffers.
 */
#if !defined(SAMA_MAC_RECEIVE_BUFFERS) || defined(__DOXYGEN__)
#define SAMA_MAC_RECEIVE_BUFFERS            4
#endif

/**
 * @brief   Maximum supported frame size.
 */
#if !defined(SAMA_MAC_BUFFERS_SIZE) || defined(__DOXYGEN__)
#define SAMA_MAC_BUFFERS_SIZE               1536
#endif

/**
 * @brief   PHY detection timeout.
 * @details Timeout for PHY address detection, the scan for a PHY is performed
 *          the specified number of times before invoking the failure handler.
 *          This setting applies only if the PHY address is not explicitly
 *          set in the board header file using @p BOARD_PHY_ADDRESS. A zero
 *          value disables the timeout and a single search is performed.
 */
#if !defined(SAMA_MAC_PHY_TIMEOUT) || defined(__DOXYGEN__)
#define SAMA_MAC_PHY_TIMEOUT                100
#endif

/**
 * @brief   Change the PHY power state inside the driver.
 */
#if !defined(SAMA_MAC_ETH0_CHANGE_PHY_STATE) || defined(__DOXYGEN__)
#define SAMA_MAC_ETH0_CHANGE_PHY_STATE      TRUE
#endif

/**
 * @brief   ETHD0 interrupt priority level setting.
 */
#if !defined(SAMA_MAC_ETH0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_MAC_ETH0_IRQ_PRIORITY          7
#endif

/**
 * @brief   IP checksum offload.
 * @details The following modes are available:
 *          - 0 Function disabled.
 *          - 1 IP/TCP/UDP header checksum calculation and insertion are enabled.
 */
#if !defined(SAMA_MAC_IP_CHECKSUM_OFFLOAD) || defined(__DOXYGEN__)
#define SAMA_MAC_IP_CHECKSUM_OFFLOAD        1
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of an SAMA Ethernet receive descriptor.
 */
typedef struct {
  volatile uint32_t     rdes0;
  volatile uint32_t     rdes1;
} sama_eth_rx_descriptor_t;

/**
 * @brief   Type of an SAMA Ethernet transmit descriptor.
 */
typedef struct {
  volatile uint32_t     tdes0;
  volatile uint32_t     tdes1;
} sama_eth_tx_descriptor_t;

/**
 * @brief   Driver configuration structure.
 */
typedef struct {
  /**
   * @brief MAC address.
   */
  uint8_t               *mac_address;
  /* End of the mandatory fields.*/
} MACConfig;

/**
 * @brief   Structure representing a MAC driver.
 */
struct MACDriver {
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
  uint32_t phyaddr;
  /**
   * @brief Receive next frame pointer.
   */
  sama_eth_rx_descriptor_t *rxptr;
  /**
   * @brief Transmit next frame pointer.
   */
  sama_eth_tx_descriptor_t *txptr;
};

/**
 * @brief   Structure representing a transmit descriptor.
 */
typedef struct {
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
  sama_eth_tx_descriptor_t *physdesc;
} MACTransmitDescriptor;

/**
 * @brief   Structure representing a receive descriptor.
 */
typedef struct {
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
  sama_eth_rx_descriptor_t *physdesc;
} MACReceiveDescriptor;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(__DOXYGEN__)
extern MACDriver ETHD0;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void mii_write(MACDriver *macp, uint32_t reg, uint32_t value);
  uint32_t mii_read(MACDriver *macp, uint32_t reg);
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
