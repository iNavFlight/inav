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
 * @file    MAC/hal_mac_lld.c
 * @brief   MAC Driver subsystem low level driver source.
 *
 * @addtogroup MAC
 * @{
 */

#include <string.h>

#include "hal.h"

#if HAL_USE_MAC || defined(__DOXYGEN__)

#include "hal_mii.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define BUFFER_SIZE ((((TIVA_MAC_BUFFERS_SIZE - 1) | 3) + 1) / 4)

/* MII divider optimal value.*/
#if (TIVA_SYSCLK >= 100000000)
#define MACMIIADDR_CR   (0x01 << 2)
#elif (TIVA_SYSCLK >= 60000000)
#define MACMIIADDR_CR   (0x00 << 2)
#elif (TIVA_SYSCLK >= 35000000)
#define MACMIIADDR_CR   (0x03 << 2)
#elif (TIVA_SYSCLK >= 20000000)
#define MACMIIADDR_CR   (0x02 << 2)
#else
#error "TIVA_SYSCLK below minimum frequency for ETH operations (20MHz)"
#endif

#define EMAC_MIIADDR_MIIW       0x00000002  /* MII Write */
#define EMAC_MIIADDR_MIIB       0x00000001  /* MII Busy */

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief Ethernet driver 1.
 */
MACDriver ETHD1;

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

static const uint8_t default_mac_address[] = {0xAA, 0x55, 0x13,
                                              0x37, 0x01, 0x10};

static tiva_eth_rx_descriptor_t rd[TIVA_MAC_RECEIVE_BUFFERS];
static tiva_eth_tx_descriptor_t td[TIVA_MAC_TRANSMIT_BUFFERS];

static uint32_t rb[TIVA_MAC_RECEIVE_BUFFERS][BUFFER_SIZE];
static uint32_t tb[TIVA_MAC_TRANSMIT_BUFFERS][BUFFER_SIZE];

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Writes a PHY register.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @param[in] reg       register number
 * @param[in] value     new register value
 *
 * @notapi
 */
static void mii_write(MACDriver *macp, uint32_t reg, uint32_t value)
{
  HWREG(EMAC0_BASE + EMAC_O_MIIDATA) = value;
  HWREG(EMAC0_BASE + EMAC_O_MIIADDR) = macp->phyaddr | (reg << 6) | MACMIIADDR_CR | EMAC_MIIADDR_MIIW | EMAC_MIIADDR_MIIB;

  while ((HWREG(EMAC0_BASE + EMAC_O_MIIADDR) & EMAC_MIIADDR_MIIB) != 0)
    ;
}

/**
 * @brief   Writes an extended PHY register.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @param[in] reg       register number
 * @param[in] value     new register value
 *
 * @notapi
 */
static void mii_write_extended(MACDriver *macp, uint32_t reg, uint32_t value)
{
  mii_write(macp, TIVA_REGCTL, 0x001F);
  mii_write(macp, TIVA_ADDAR, reg);

  mii_write(macp, TIVA_REGCTL, 0x401F);
  mii_write(macp, TIVA_ADDAR, value);
}

/**
 * @brief   Reads a PHY register.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @param[in] reg       register number
 *
 * @return              The PHY register content.
 *
 * @notapi
 */
static uint32_t mii_read(MACDriver *macp, uint32_t reg)
{
  HWREG(EMAC0_BASE + EMAC_O_MIIADDR) = macp->phyaddr | (reg << 6) | MACMIIADDR_CR | EMAC_MIIADDR_MIIB;

  while ((HWREG(EMAC0_BASE + EMAC_O_MIIADDR) & EMAC_MIIADDR_MIIB) != 0)
    ;

  return HWREG(EMAC0_BASE + EMAC_O_MIIDATA);
}

/**
 * @brief   Reads an extended PHY register.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @param[in] reg       register number
 *
 * @return              The extended PHY register content.
 *
 * @notapi
 */
static uint32_t mii_read_extended(MACDriver *macp, uint32_t reg)
{
  mii_write(macp, TIVA_REGCTL, 0x001F);
  mii_write(macp, TIVA_ADDAR, reg);

  mii_write(macp, TIVA_REGCTL, 0x401F);
  return mii_read(macp, TIVA_ADDAR);
}

#if !defined(BOARD_PHY_ADDRESS)
/**
 * @brief   PHY address detection.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 */
static void mii_find_phy(MACDriver *macp)
{
  uint32_t i;

#if TIVA_MAC_PHY_TIMEOUT > 0
  rtcnt_t start = chSysGetRealtimeCounterX();
  rtcnt_t timeout  = start + MS2RTC(STM32_HCLK,STM32_MAC_PHY_TIMEOUT);
  rtcnt_t time = start;
  while (chSysIsCounterWithinX(time, start, timeout)) {
#endif
    for (i = 0; i < 31; i++) {
      macp->phyaddr = i << 11;
      HWREG(EMAC0_BASE + EMAC_O_MIIDATA) = (i << 6) | MACMIIADDR_CR;
      if ((mii_read(macp, TIVA_ID1) == (BOARD_PHY_ID >> 16)) &&
          ((mii_read(macp, TIVA_ID2) & 0xFFF0) == (BOARD_PHY_ID & 0xFFF0))) {
        return;
      }
    }
#if TIVA_MAC_PHY_TIMEOUT > 0
    time = chSysGetRealtimeCounterX();
  }
#endif
  /* Wrong or defective board.*/
  osalSysHalt("MAC failure");
}
#endif

/**
 * @brief   MAC address setup.
 *
 * @param[in] p         pointer to a six bytes buffer containing the MAC
 *                      address
 */
static void mac_lld_set_address(const uint8_t *p)
{
  /* MAC address configuration, only a single address comparator is used,
     hash table not used.*/
  HWREG(EMAC0_BASE + EMAC_O_ADDR0H)   = ((uint32_t)p[5] << 8) |
                  ((uint32_t)p[4] << 0);
  HWREG(EMAC0_BASE + EMAC_O_ADDR0L)   = ((uint32_t)p[3] << 24) |
                  ((uint32_t)p[2] << 16) |
                  ((uint32_t)p[1] << 8) |
                  ((uint32_t)p[0] << 0);
  HWREG(EMAC0_BASE + EMAC_O_ADDR1H)   = 0x0000FFFF;
  HWREG(EMAC0_BASE + EMAC_O_ADDR1L)   = 0xFFFFFFFF;
  HWREG(EMAC0_BASE + EMAC_O_ADDR2H)   = 0x0000FFFF;
  HWREG(EMAC0_BASE + EMAC_O_ADDR2L)   = 0xFFFFFFFF;
  HWREG(EMAC0_BASE + EMAC_O_ADDR3H)   = 0x0000FFFF;
  HWREG(EMAC0_BASE + EMAC_O_ADDR3L)   = 0xFFFFFFFF;
  HWREG(EMAC0_BASE + EMAC_O_HASHTBLH) = 0;
  HWREG(EMAC0_BASE + EMAC_O_HASHTBLL) = 0;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

CH_IRQ_HANDLER(TIVA_MAC_HANDLER)
{
  uint32_t dmaris;

  CH_IRQ_PROLOGUE();

  dmaris = HWREG(EMAC0_BASE + EMAC_O_DMARIS);
  HWREG(EMAC0_BASE + EMAC_O_DMARIS) = dmaris & 0x0001FFFF; /* Clear status bits.*/

  if (dmaris & (1 << 6)) {
    /* Data Received.*/
    osalSysLockFromISR();
    osalThreadDequeueAllI(&ETHD1.rdqueue, MSG_RESET);
#if MAC_USE_EVENTS
    osalEventBroadcastFlagsI(&ETHD1.rdevent, 0);
#endif
    osalSysUnlockFromISR();
  }

  if (dmaris & (1 << 0)) {
    /* Data Transmitted.*/
    osalSysLockFromISR();
    osalThreadDequeueAllI(&ETHD1.tdqueue, MSG_RESET);
    osalSysUnlockFromISR();
  }

  CH_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level MAC initialization.
 *
 * @notapi
 */
void mac_lld_init(void)
{
  uint8_t i;

  macObjectInit(&ETHD1);
  ETHD1.link_up = false;

  /* Descriptor tables are initialized in chained mode, note that the first
     word is not initialized here but in mac_lld_start().*/
  for (i = 0; i < TIVA_MAC_RECEIVE_BUFFERS; i++) {
    rd[i].rdes1 = TIVA_RDES1_RCH | TIVA_RDES1_RBS1(TIVA_MAC_BUFFERS_SIZE);
    rd[i].rdes2 = (uint32_t)rb[i];
    rd[i].rdes3 = (uint32_t)&rd[(i + 1) % TIVA_MAC_RECEIVE_BUFFERS];
  }
  for (i = 0; i < TIVA_MAC_TRANSMIT_BUFFERS; i++) {
    td[i].tdes1 = 0;
    td[i].tdes2 = (uint32_t)tb[i];
    td[i].tdes3 = (uint32_t)&td[(i + 1) % TIVA_MAC_TRANSMIT_BUFFERS];
  }

  /* Enable MAC clock */
  HWREG(SYSCTL_RCGCEMAC) = 1;
  while (HWREG(SYSCTL_PREMAC) != 0x01)
    ;

  /* Set PHYHOLD bit */
  HWREG(EMAC0_BASE + EMAC_O_PC) |= 1;

  /* Enable PHY clock */
  HWREG(SYSCTL_RCGCEPHY) = 1;
  while (HWREG(SYSCTL_PREPHY) != 0x01)
    ;

  /* Enable power to PHY */
  HWREG(SYSCTL_PCEPHY) |= 1;
  while (HWREG(SYSCTL_PREPHY) != 0x01)
    ;
#if BOARD_PHY_RMII
  HWREG(EMAC0_BASE + EMAC_O_PC) = EMAC_PHY_CONFIG | (0x04 << 28);
#else
  HWREG(EMAC0_BASE + EMAC_O_PC) = EMAC_PHY_CONFIG;
#endif

  /*
   * Write OHY led configuration.
   * 0: link ok
   * 1: tx activity
   * 2: link ok
   * blink rate: 20Hz
   */
  mii_write_extended(&ETHD1, TIVA_LEDCFG, (0 << 8) | (2 << 4) | (0 << 0));
  mii_write(&ETHD1, TIVA_LEDCR, (0 << 9));

  /* Set done bit after writing EMACPC register */
  mii_write(&ETHD1, TIVA_CFG1, (1 << 15) | mii_read(&ETHD1, TIVA_CFG1));

  while(HWREG(EMAC0_BASE + EMAC_O_DMABUSMOD) & 1)
    ;

  /* Reset MAC */
  HWREG(EMAC0_BASE + EMAC_O_DMABUSMOD) |= 1;
  while (HWREG(EMAC0_BASE + EMAC_O_DMABUSMOD) & 1)
    ;

  /* PHY address setup.*/
#if defined(BOARD_PHY_ADDRESS)
  ETHD1.phyaddr = BOARD_PHY_ADDRESS << 11;
#else
  mii_find_phy(&ETHD1);
#endif

#if defined(BOARD_PHY_RESET)
  /* PHY board-specific reset procedure.*/
  BOARD_PHY_RESET();
#else
  /* PHY soft reset procedure.*/
  mii_write(&ETHD1, MII_BMCR, BMCR_RESET);
#if defined(BOARD_PHY_RESET_DELAY)
  chSysPolledDelayX(BOARD_PHY_RESET_DELAY);
#endif
  while (mii_read(&ETHD1, MII_BMCR) & BMCR_RESET)
    ;
#endif

#if TIVA_MAC_CHANGE_PHY_STATE
  /* PHY in power down mode until the driver will be started.*/
  mii_write(&ETHD1, MII_BMCR, mii_read(&ETHD1, MII_BMCR) | BMCR_PDOWN);
#endif

  /* Disable MAC clock */
  HWREG(SYSCTL_RCGCEMAC) = 0;

  /* Disable PHY clock */
  HWREG(SYSCTL_RCGCEPHY) = 0;
}

/**
 * @brief   Configures and activates the MAC peripheral.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 *
 * @notapi
 */
void mac_lld_start(MACDriver *macp)
{
  uint8_t i;

  /* Resets the state of all descriptors.*/
  for (i = 0; i < TIVA_MAC_RECEIVE_BUFFERS; i++) {
    rd[i].rdes0 = TIVA_RDES0_OWN;
  }
  macp->rxptr = (tiva_eth_rx_descriptor_t *)rd;

  for (i = 0; i < TIVA_MAC_TRANSMIT_BUFFERS; i++) {
    td[i].tdes0 = TIVA_TDES0_TCH;
    td[i].locked = 0;
  }
  macp->txptr = (tiva_eth_tx_descriptor_t *)td;

  /* Enable MAC clock */
  HWREG(SYSCTL_RCGCEMAC) = 1;
  while (HWREG(SYSCTL_PREMAC) != 0x01)
    ;

  /* Enable PHY clock */
  HWREG(SYSCTL_RCGCEPHY) = 1;
  while (!HWREG(SYSCTL_PREPHY))
    ;

  /* ISR vector enabled.*/
  nvicEnableVector(TIVA_MAC_NUMBER, TIVA_MAC_IRQ_PRIORITY);

#if TIVA_MAC_CHANGE_PHY_STATE
  /* PHY in power up mode.*/
  mii_write(macp, MII_BMCR, mii_read(macp, MII_BMCR) & ~BMCR_PDOWN);
#endif

  /* MAC configuration.*/
  HWREG(EMAC0_BASE + EMAC_O_FRAMEFLTR) = 0;
  HWREG(EMAC0_BASE + EMAC_O_FLOWCTL) = 0;
  HWREG(EMAC0_BASE + EMAC_O_VLANTG) = 0;

  /* MAC address setup.*/
  if (macp->config->mac_address == NULL)
    mac_lld_set_address(default_mac_address);
  else
    mac_lld_set_address(macp->config->mac_address);

  /* Transmitter and receiver enabled.
     Note that the complete setup of the MAC is performed when the link
     status is detected.*/
#if TIVA_MAC_IP_CHECKSUM_OFFLOAD
  HWREG(EMAC0_BASE + EMAC_O_CFG) = (1 << 10) | (1 << 3) | (1 << 2);
#else
  HWREG(EMAC0_BASE + EMAC_O_CFG) =             (1 << 3) | (1 << 2);
#endif

  /* DMA configuration:
     Descriptor chains pointers.*/
  HWREG(EMAC0_BASE + EMAC_O_RXDLADDR) = (uint32_t)rd;
  HWREG(EMAC0_BASE + EMAC_O_TXDLADDR) = (uint32_t)td;

  /* Enabling required interrupt sources.*/
  HWREG(EMAC0_BASE + EMAC_O_DMARIS) &= 0xFFFF;
  HWREG(EMAC0_BASE + EMAC_O_DMAIM) = (1 << 16) | (1 << 6) | (1 << 0);

  /* DMA general settings.*/
  HWREG(EMAC0_BASE + EMAC_O_DMABUSMOD) = (1 << 25) | (1 << 17) | (1 << 8);

  /* Transmit FIFO flush.*/
  HWREG(EMAC0_BASE + EMAC_O_DMAOPMODE) = (1 << 20);
  while (HWREG(EMAC0_BASE + EMAC_O_DMAOPMODE) & (1 << 20))
    ;

  /* DMA final configuration and start.*/
  HWREG(EMAC0_BASE + EMAC_O_DMAOPMODE) = (1 << 26) | (1 << 25) | (1 << 21) |
                        (1 << 13) | (1 << 1);
}

/**
 * @brief   Deactivates the MAC peripheral.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 *
 * @notapi
 */
void mac_lld_stop(MACDriver *macp)
{
  if (macp->state != MAC_STOP) {
#if TIVA_MAC_CHANGE_PHY_STATE
    /* PHY in power down mode until the driver will be restarted.*/
    mii_write(macp, MII_BMCR, mii_read(macp, MII_BMCR) | BMCR_PDOWN);
#endif

    /* MAC and DMA stopped.*/
    HWREG(EMAC0_BASE + EMAC_O_CFG) = 0;
    HWREG(EMAC0_BASE + EMAC_O_DMAOPMODE) = 0;
    HWREG(EMAC0_BASE + EMAC_O_DMAIM) = 0;
    HWREG(EMAC0_BASE + EMAC_O_DMARIS) &= 0xFFFF;

    /* MAC clocks stopped.*/
    HWREG(SYSCTL_RCGCEMAC) = 0;

    /* PHY clock stopped.*/
    HWREG(SYSCTL_RCGCEPHY) = 0;

    /* ISR vector disabled.*/
    nvicDisableVector(TIVA_MAC_NUMBER);
  }
}

/**
 * @brief   Returns a transmission descriptor.
 * @details One of the available transmission descriptors is locked and
 *          returned.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @param[out] tdp      pointer to a @p MACTransmitDescriptor structure
 * @return              The operation status.
 * @retval RDY_OK       the descriptor has been obtained.
 * @retval RDY_TIMEOUT  descriptor not available.
 *
 * @notapi
 */
msg_t mac_lld_get_transmit_descriptor(MACDriver *macp,
                                      MACTransmitDescriptor *tdp)
{
  tiva_eth_tx_descriptor_t *tdes;

  if (!macp->link_up)
    return MSG_TIMEOUT;

  osalSysLock();

  /* Get Current TX descriptor.*/
  tdes = macp->txptr;

  /* Ensure that descriptor isn't owned by the Ethernet DMA or locked by
     another thread.*/
  if (tdes->tdes0 & (TIVA_TDES0_OWN) || (tdes->locked)) {
    osalSysUnlock();
    return MSG_TIMEOUT;
  }

  /* Marks the current descriptor as locked.*/
  tdes->locked = 1;

  /* Next TX descriptor to use.*/
  macp->txptr = (tiva_eth_tx_descriptor_t *)tdes->tdes3;

  osalSysUnlock();

  /* Set the buffer size and configuration.*/
  tdp->offset   = 0;
  tdp->size     = TIVA_MAC_BUFFERS_SIZE;
  tdp->physdesc = tdes;

  return MSG_OK;
}

/**
 * @brief   Releases a transmit descriptor and starts the transmission of the
 *          enqueued data as a single frame.
 *
 * @param[in] tdp       the pointer to the @p MACTransmitDescriptor structure
 *
 * @notapi
 */
void mac_lld_release_transmit_descriptor(MACTransmitDescriptor *tdp)
{
  osalDbgAssert(!(tdp->physdesc->tdes0 & TIVA_TDES0_OWN),
              "attempt to release descriptor already owned by DMA");

  osalSysLock();

  /* Unlocks the descriptor and returns it to the DMA engine.*/
  tdp->physdesc->tdes1 = tdp->offset;
  tdp->physdesc->tdes0 = TIVA_TDES0_CIC(TIVA_MAC_IP_CHECKSUM_OFFLOAD) |
                         TIVA_TDES0_IC | TIVA_TDES0_LS | TIVA_TDES0_FS |
                         TIVA_TDES0_TCH | TIVA_TDES0_OWN;
  tdp->physdesc->locked = 0;

  /* If the DMA engine is stalled then a restart request is issued.*/
  if ((HWREG(EMAC0_BASE + EMAC_O_DMARIS) & (0x7 << 20)) == (6 << 20)) {
    HWREG(EMAC0_BASE + EMAC_O_DMARIS)   = (1 << 2);
    HWREG(EMAC0_BASE + EMAC_O_TXPOLLD) = 1; /* Any value is OK.*/
  }

  osalSysUnlock();
}

/**
 * @brief   Returns a receive descriptor.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @param[out] rdp      pointer to a @p MACReceiveDescriptor structure
 * @return              The operation status.
 * @retval RDY_OK       the descriptor has been obtained.
 * @retval RDY_TIMEOUT  descriptor not available.
 *
 * @notapi
 */
msg_t mac_lld_get_receive_descriptor(MACDriver *macp,
                                     MACReceiveDescriptor *rdp)
{
  tiva_eth_rx_descriptor_t *rdes;

  osalSysLock();

  /* Get Current RX descriptor.*/
  rdes = macp->rxptr;

  /* Iterates through received frames until a valid one is found, invalid
     frames are discarded.*/
  while (!(rdes->rdes0 & TIVA_RDES0_OWN)) {
    if (!(rdes->rdes0 & (TIVA_RDES0_AFM | TIVA_RDES0_ES))
#if TIVA_MAC_IP_CHECKSUM_OFFLOAD
        && (rdes->rdes0 & TIVA_RDES0_FT)
        && !(rdes->rdes0 & (TIVA_RDES0_IPHCE | TIVA_RDES0_PCE))
#endif
        && (rdes->rdes0 & TIVA_RDES0_FS) && (rdes->rdes0 & TIVA_RDES0_LS)) {
      /* Found a valid one.*/
      rdp->offset   = 0;
      rdp->size     = ((rdes->rdes0 & TIVA_RDES0_FL_MASK) >> 16) - 4;
      rdp->physdesc = rdes;
      macp->rxptr   = (tiva_eth_rx_descriptor_t *)rdes->rdes3;

      osalSysUnlock();
      return MSG_OK;
    }
    /* Invalid frame found, purging.*/
    rdes->rdes0 = TIVA_RDES0_OWN;
    rdes = (tiva_eth_rx_descriptor_t *)rdes->rdes3;
  }

  /* Next descriptor to check.*/
  macp->rxptr = rdes;

  osalSysUnlock();
  return MSG_TIMEOUT;
}

/**
 * @brief   Releases a receive descriptor.
 * @details The descriptor and its buffer are made available for more incoming
 *          frames.
 *
 * @param[in] rdp       the pointer to the @p MACReceiveDescriptor structure
 *
 * @notapi
 */
void mac_lld_release_receive_descriptor(MACReceiveDescriptor *rdp)
{
  osalDbgAssert(!(rdp->physdesc->rdes0 & TIVA_RDES0_OWN),
              "attempt to release descriptor already owned by DMA");

  osalSysLock();

  /* Give buffer back to the Ethernet DMA.*/
  rdp->physdesc->rdes0 = TIVA_RDES0_OWN;

  /* If the DMA engine is stalled then a restart request is issued.*/
  if ((HWREG(EMAC0_BASE + EMAC_O_STATUS) & (0xf << 17)) == (4 << 17)) {
    HWREG(EMAC0_BASE + EMAC_O_DMARIS)   = (1 << 7);
    HWREG(EMAC0_BASE + EMAC_O_TXPOLLD) = 1; /* Any value is OK.*/
  }

  osalSysUnlock();
}

/**
 * @brief   Updates and returns the link status.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @return              The link status.
 * @retval TRUE         if the link is active.
 * @retval FALSE        if the link is down.
 *
 * @notapi
 */
bool mac_lld_poll_link_status(MACDriver *macp)
{
  uint32_t maccfg, bmsr, bmcr;

  maccfg = HWREG(EMAC0_BASE + EMAC_O_CFG);

  /* PHY CR and SR registers read.*/
  (void)mii_read(macp, MII_BMSR);
  bmsr = mii_read(macp, MII_BMSR);
  bmcr = mii_read(macp, MII_BMCR);

  /* Check on auto-negotiation mode.*/
  if (bmcr & BMCR_ANENABLE) {
    uint32_t lpa;

    /* Auto-negotiation must be finished without faults and link established.*/
    if ((bmsr & (BMSR_LSTATUS | BMSR_RFAULT | BMSR_ANEGCOMPLETE)) !=
        (BMSR_LSTATUS | BMSR_ANEGCOMPLETE))
      return macp->link_up = false;

    /* Auto-negotiation enabled, checks the LPA register.*/
    lpa = mii_read(macp, MII_LPA);

    /* Check on link speed.*/
    if (lpa & (LPA_100HALF | LPA_100FULL | LPA_100BASE4))
      maccfg |= (1 << 14);
    else
      maccfg &= ~(1 << 14);

    /* Check on link mode.*/
    if (lpa & (LPA_10FULL | LPA_100FULL))
      maccfg |= (1 << 11);
    else
      maccfg &= ~(1 << 11);
  }
  else {
    /* Link must be established.*/
    if (!(bmsr & BMSR_LSTATUS))
      return macp->link_up = false;

    /* Check on link speed.*/
    if (bmcr & BMCR_SPEED100)
      maccfg |= (1 << 14);
    else
      maccfg &= ~(1 << 14);

    /* Check on link mode.*/
    if (bmcr & BMCR_FULLDPLX)
      maccfg |= (1 << 11);
    else
      maccfg &= ~(1 << 11);
  }

  /* Changes the mode in the MAC.*/
  HWREG(EMAC0_BASE + EMAC_O_CFG) = maccfg;

  /* Returns the link status.*/
  return macp->link_up = true;
}

/**
 * @brief   Writes to a transmit descriptor's stream.
 *
 * @param[in] tdp       pointer to a @p MACTransmitDescriptor structure
 * @param[in] buf       pointer to the buffer containing the data to be
 *                      written
 * @param[in] size      number of bytes to be written
 * @return              The number of bytes written into the descriptor's
 *                      stream, this value can be less than the amount
 *                      specified in the parameter @p size if the maximum
 *                      frame size is reached.
 *
 * @notapi
 */
size_t mac_lld_write_transmit_descriptor(MACTransmitDescriptor *tdp,
                                         uint8_t *buf,
                                         size_t size)
{
  osalDbgAssert(!(tdp->physdesc->tdes0 & TIVA_TDES0_OWN),
              "attempt to write descriptor already owned by DMA");

  if (size > tdp->size - tdp->offset)
    size = tdp->size - tdp->offset;

  if (size > 0) {
    memcpy((uint8_t *)(tdp->physdesc->tdes2) + tdp->offset, buf, size);
    tdp->offset += size;
  }
  return size;
}

/**
 * @brief   Reads from a receive descriptor's stream.
 *
 * @param[in] rdp       pointer to a @p MACReceiveDescriptor structure
 * @param[in] buf       pointer to the buffer that will receive the read data
 * @param[in] size      number of bytes to be read
 * @return              The number of bytes read from the descriptor's
 *                      stream, this value can be less than the amount
 *                      specified in the parameter @p size if there are
 *                      no more bytes to read.
 *
 * @notapi
 */
size_t mac_lld_read_receive_descriptor(MACReceiveDescriptor *rdp,
                                       uint8_t *buf,
                                       size_t size)
{
  osalDbgAssert(!(rdp->physdesc->rdes0 & TIVA_RDES0_OWN),
              "attempt to read descriptor already owned by DMA");

  if (size > rdp->size - rdp->offset)
    size = rdp->size - rdp->offset;

  if (size > 0) {
    memcpy(buf, (uint8_t *)(rdp->physdesc->rdes2) + rdp->offset, size);
    rdp->offset += size;
  }
  return size;
}

#if MAC_USE_ZERO_COPY || defined(__DOXYGEN__)
/**
 * @brief   Returns a pointer to the next transmit buffer in the descriptor
 *          chain.
 * @note    The API guarantees that enough buffers can be requested to fill
 *          a whole frame.
 *
 * @param[in] tdp       pointer to a @p MACTransmitDescriptor structure
 * @param[in] size      size of the requested buffer. Specify the frame size
 *                      on the first call then scale the value down subtracting
 *                      the amount of data already copied into the previous
 *                      buffers.
 * @param[out] sizep    pointer to variable receiving the buffer size, it is
 *                      zero when the last buffer has already been returned.
 *                      Note that a returned size lower than the amount
 *                      requested means that more buffers must be requested
 *                      in order to fill the frame data entirely.
 * @return              Pointer to the returned buffer.
 * @retval NULL         if the buffer chain has been entirely scanned.
 *
 * @notapi
 */
uint8_t *mac_lld_get_next_transmit_buffer(MACTransmitDescriptor *tdp,
                                          size_t size,
                                          size_t *sizep)
{
  if (tdp->offset == 0) {
    *sizep      = tdp->size;
    tdp->offset = size;
    return (uint8_t *)tdp->physdesc->tdes2;
  }
  *sizep = 0;
  return NULL;
}

/**
 * @brief   Returns a pointer to the next receive buffer in the descriptor
 *          chain.
 * @note    The API guarantees that the descriptor chain contains a whole
 *          frame.
 *
 * @param[in] rdp       pointer to a @p MACReceiveDescriptor structure
 * @param[out] sizep    pointer to variable receiving the buffer size, it is
 *                      zero when the last buffer has already been returned.
 * @return              Pointer to the returned buffer.
 * @retval NULL         if the buffer chain has been entirely scanned.
 *
 * @notapi
 */
const uint8_t *mac_lld_get_next_receive_buffer(MACReceiveDescriptor *rdp,
                                               size_t *sizep)
{
  if (rdp->size > 0) {
    *sizep      = rdp->size;
    rdp->offset = rdp->size;
    rdp->size   = 0;
    return (uint8_t *)rdp->physdesc->rdes2;
  }
  *sizep = 0;
  return NULL;
}
#endif /* MAC_USE_ZERO_COPY */

#endif /* HAL_USE_MAC */

/** @} */
