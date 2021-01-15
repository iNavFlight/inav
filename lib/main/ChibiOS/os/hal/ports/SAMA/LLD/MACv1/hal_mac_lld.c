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
 * @file    MACv1/hal_mac_lld.c
 * @brief   SAMA low level MAC driver code.
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
/*
 * @brief    Buffer size.
 */
#define BUFFER_SIZE ((((SAMA_MAC_BUFFERS_SIZE - 1) | 3) + 1) / 4)

/*
 * @brief    NO CACHE attribute
 */
#if !defined(NO_CACHE)
#define NO_CACHE                        __attribute__((section (".nocache")))
#endif

/* MII divider optimal value.*/
#if (SAMA_GMAC0CLK <= 20000000)
#define GMAC_CLK     GMAC_NCFGR_CLK_MCK_8
#elif (SAMA_GMAC0CLK <= 40000000)
#define GMAC_CLK     GMAC_NCFGR_CLK_MCK_16
#elif (SAMA_GMAC0CLK <= 80000000)
#define GMAC_CLK     GMAC_NCFGR_CLK_MCK_32
#elif (SAMA_GMAC0CLK <= 120000000)
#define GMAC_CLK     GMAC_NCFGR_CLK_MCK_48
#elif (SAMA_GMAC0CLK <= 160000000)
#define GMAC_CLK     GMAC_NCFGR_CLK_MCK_64
#elif (SAMA_GMAC0CLK <= 240000000)
#define GMAC_CLK     GMAC_NCFGR_CLK_MCK_96
#else
#error "MCK too high, cannot configure MDC clock"
#endif

/*
 * BV1000GT boards use phy address 0
 */
#if defined(BOARD_ATSAM5D28_XULT)
#define BOARD_PHY_ADDRESS    0
#endif

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/
/**
 * @brief   Ethernet driver 0.
 */
MACDriver ETHD0;

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

static const uint8_t default_mac_address[] = {0x54, 0x54, 0x08, 0x34, 0x1f, 0x3a};

/*
 * In terms of AMBA AHB operation, the descriptors are read from memory using
 * a single 32-bit AHB access. The descriptors should be aligned at 32-bit
 * boundaries and the descriptors are written to using two individual non
 * sequential accesses.
 */

/* Rx descriptor list */
NO_CACHE ALIGNED_VAR(4)
static sama_eth_rx_descriptor_t __eth_rd[SAMA_MAC_RECEIVE_BUFFERS];

/* Tx descriptor list */
NO_CACHE ALIGNED_VAR(4)
static sama_eth_tx_descriptor_t __eth_td[SAMA_MAC_TRANSMIT_BUFFERS];
NO_CACHE ALIGNED_VAR(4)
static sama_eth_tx_descriptor_t __eth_td1[1];
NO_CACHE ALIGNED_VAR(4)
static sama_eth_tx_descriptor_t __eth_td2[1];

NO_CACHE
static uint32_t __eth_rb[SAMA_MAC_RECEIVE_BUFFERS][BUFFER_SIZE];
NO_CACHE
static uint32_t __eth_tb[SAMA_MAC_TRANSMIT_BUFFERS][BUFFER_SIZE];

/*===========================================================================*/
/* Driver local macros.                                                      */
/*===========================================================================*/
/**
 * @brief   Waits for phy management logic idle.
 *
 * @notapi
 */
#define phyWaitIdle() {                                                     \
  while ((GMAC0->GMAC_NSR & GMAC_NSR_IDLE) == 0) {                          \
    ;                                                                       \
  }                                                                         \
}

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
/**
 * @brief   Writes a PHY register.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @param[in] reg_addr  register address
 * @param[in] value     new register value
 *
 * @notapi
 */
void mii_write(MACDriver *macp, uint32_t reg_addr, uint32_t value) {

  phyWaitIdle();

  /* Write maintenance register (Clause 22) */
  GMAC0->GMAC_MAN = GMAC_MAN_CLTTO | GMAC_MAN_OP(1) | GMAC_MAN_WTN(2) |
                   GMAC_MAN_PHYA(macp->phyaddr) | GMAC_MAN_REGA(reg_addr) |
                   GMAC_MAN_DATA(value);
  phyWaitIdle();
}

/**
 * @brief   Reads a PHY register.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @param[in] reg_addr  register address
 *
 * @return              The PHY register content.
 *
 * @notapi
 */
uint32_t mii_read(MACDriver *macp, uint32_t reg_addr) {

  phyWaitIdle();

  /* Read maintenance register */
  GMAC0->GMAC_MAN = GMAC_MAN_CLTTO | GMAC_MAN_OP(2) | GMAC_MAN_WTN(2) |
                   GMAC_MAN_PHYA(macp->phyaddr) | GMAC_MAN_REGA(reg_addr);
  phyWaitIdle();

  return (uint32_t) ((GMAC0->GMAC_MAN) & GMAC_MAN_DATA_Msk >> GMAC_MAN_DATA_Pos);
}

#if !defined(BOARD_PHY_ADDRESS)
/**
 * @brief   PHY address detection.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 */
static void mii_find_phy(MACDriver *macp) {
  uint32_t i;

#if SAMA_MAC_PHY_TIMEOUT > 0
  unsigned n = SAMA_MAC_PHY_TIMEOUT;
 do {
#endif
    for (i = 1U; i < 31U; i++) {
      macp->phyaddr = i;
      if ((mii_read(macp, MII_PHYSID1) == (BOARD_PHY_ID >> 16U)) &&
          ((mii_read(macp, MII_PHYSID2) & 0xFFF0U) == (BOARD_PHY_ID & 0xFFF0U))) {
        return;
      }
    }
#if SAMA_MAC_PHY_TIMEOUT > 0
    n--;
  } while (n > 0U);
#endif
  macp->phyaddr = 0;
  return;
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
static void mac_lld_set_address(const uint8_t *p) {

  /* MAC address configuration, only a single address comparator is used,
     hash table not used.*/
  GMAC0->GMAC_SA[0].GMAC_SAB = (p[3] << 24) | (p[2] << 16) |
                               (p[1] << 8) | p[0];
  GMAC0->GMAC_SA[0].GMAC_SAT = (p[5] << 8) | p[4];
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

OSAL_IRQ_HANDLER(SAMA_ETH_HANDLER) {
  uint32_t isr;
  uint32_t rsr;
  uint32_t tsr;

  OSAL_IRQ_PROLOGUE();

  isr = GMAC0->GMAC_ISR;
  rsr = GMAC0->GMAC_RSR;
  tsr = GMAC0->GMAC_TSR;

  if (isr & GMAC_ISR_RCOMP) {
    /* Data Received.*/
    osalSysLockFromISR();
    /* Clear Status Register */
    GMAC0->GMAC_RSR = rsr;
    osalThreadDequeueAllI(&ETHD0.rdqueue, MSG_RESET);
#if MAC_USE_EVENTS
    osalEventBroadcastFlagsI(&ETHD0.rdevent, 0);
#endif
    osalSysUnlockFromISR();
  }

  if (isr & GMAC_ISR_TCOMP) {
    /* Data Transmitted.*/
    osalSysLockFromISR();
    /* Clear Status Register */
    GMAC0->GMAC_TSR = tsr;
    osalThreadDequeueAllI(&ETHD0.tdqueue, MSG_RESET);
    osalSysUnlockFromISR();
  }
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level MAC initialization.
 *
 * @notapi
 */
void mac_lld_init(void) {

  unsigned i;

#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_GMAC0, SECURE_PER);
  mtxConfigPeriphSecurity(MATRIX1, ID_GMAC0_Q1, SECURE_PER);
  mtxConfigPeriphSecurity(MATRIX1, ID_GMAC0_Q2, SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */

  macObjectInit(&ETHD0);
  ETHD0.link_up = false;

  /* Descriptor tables are initialized in chained mode, note that the status
     word is not initialized here but in mac_lld_start().*/
  for (i = 0; i < SAMA_MAC_RECEIVE_BUFFERS; i++) {
    __eth_rd[i].rdes0 = ((uint32_t)__eth_rb[i]) & (SAMA_RDES0_RBAP_MASK);
  /* Status reset */
    __eth_rd[i].rdes1 = 0;
  /* For last buffer wrap is set */
    if (i == (SAMA_MAC_RECEIVE_BUFFERS - 1)){
      __eth_rd[i].rdes0 |= SAMA_RDES0_WRAP;
    }
  }

  __eth_td1[0].tdes1 = 0;
  __eth_td2[0].tdes1 = 0;

  for (i = 0; i < SAMA_MAC_TRANSMIT_BUFFERS; i++) {
    __eth_td[i].tdes0 = (uint32_t)__eth_tb[i];
    /* Status reset */
    __eth_td[i].tdes1 = 0;
      /* For last buffer wrap is set */
      if (i == (SAMA_MAC_TRANSMIT_BUFFERS - 1)){
        __eth_td[i].tdes1 |= SAMA_TDES1_WRAP;
        __eth_td1[0].tdes1 |= SAMA_TDES1_WRAP;
        __eth_td2[0].tdes1 |= SAMA_TDES1_WRAP;
      }
  }

  /* MAC clocks temporary activation.*/
  pmcEnableETH0();

  /* Configures MDIO clock */
  GMAC0->GMAC_NCFGR = (GMAC0->GMAC_NCFGR & ~GMAC_NCFGR_CLK_Msk) | GMAC_CLK;

  /* Enables management port */
  GMAC0->GMAC_NCR |= GMAC_NCR_MPE;

  /* Selection of the RMII or MII mode based on info exported by board.h.*/
#if defined(BOARD_PHY_RMII)
  GMAC0->GMAC_UR = GMAC_UR_RMII;
#else
  GMAC0->GMAC_UR &= ~GMAC_UR_RMII;
#endif

  /* PHY address setup.*/
#if defined(BOARD_PHY_ADDRESS)
  ETHD0.phyaddr = BOARD_PHY_ADDRESS;
#else
  mii_find_phy(&ETHD0);
#endif

#if defined(BOARD_PHY_RESET)
  /* PHY board-specific reset procedure.*/
  BOARD_PHY_RESET();
#else
  /* PHY soft reset procedure.*/
  mii_write(&ETHD0, MII_BMCR, BMCR_RESET);
#if defined(BOARD_PHY_RESET_DELAY)
  osalSysPolledDelayX(BOARD_PHY_RESET_DELAY);
#endif
  while (mii_read(&ETHD0, MII_BMCR) & BMCR_RESET)
    ;
#endif

#if SAMA_MAC_ETH0_CHANGE_PHY_STATE
  /* PHY in power down mode until the driver will be started.*/
  mii_write(&ETHD0, MII_BMCR, mii_read(&ETHD0, MII_BMCR) | BMCR_PDOWN);
#endif

  /* MAC clocks stopped again. */
  pmcDisableETH0();
}

/**
 * @brief   Configures and activates the MAC peripheral.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 *
 * @notapi
 */
void mac_lld_start(MACDriver *macp) {
  unsigned i;

  /* Disable interrupts */
  GMAC0->GMAC_IDR = 0xFFFFFFFF;         /* Queue 0 */
  GMAC0->GMAC_IDRPQ[0] = 0xFFFFFFFF;    /* Queue 1 */
  GMAC0->GMAC_IDRPQ[1] = 0xFFFFFFFF;    /* Queue 2 */

  /* Clear statistic */
  GMAC0->GMAC_NCR |= GMAC_NCR_CLRSTAT;

  /* Clear rx and tx status bit */
  GMAC0->GMAC_RSR = 0xF;
  GMAC0->GMAC_TSR = 0xFF;

  /* Clear interrupt status register */
  GMAC0->GMAC_ISR;                      /* Queue 0 */
  GMAC0->GMAC_ISRPQ[0];                 /* Queue 1 */
  GMAC0->GMAC_ISRPQ[1];                 /* Queue 2 */

  /* Free all descriptors.*/
  for (i = 0; i < SAMA_MAC_RECEIVE_BUFFERS; i++)
    __eth_rd[i].rdes0 &= ~SAMA_RDES0_OWN;

  /* Current receive descriptor */
  macp->rxptr = (sama_eth_rx_descriptor_t *)__eth_rd;

  for (i = 0; i < SAMA_MAC_TRANSMIT_BUFFERS; i++)
    __eth_td[i].tdes1 |= SAMA_TDES1_LAST_BUFF | SAMA_TDES1_USED;

  __eth_td1[0].tdes1 |= SAMA_TDES1_LAST_BUFF | SAMA_TDES1_USED;
  __eth_td2[0].tdes1 |= SAMA_TDES1_LAST_BUFF | SAMA_TDES1_USED;

  macp->txptr = (sama_eth_tx_descriptor_t *)__eth_td;

  /* MAC clocks activation and commanded reset procedure.*/
  pmcEnableETH0();

  /* Enable interrupt */
  aicSetSourcePriority(ID_GMAC0, SAMA_MAC_ETH0_IRQ_PRIORITY);
  aicSetSourceHandler(ID_GMAC0, SAMA_ETH_HANDLER);
  aicEnableInt(ID_GMAC0);

#if SAMA_MAC_ETH0_CHANGE_PHY_STATE
  /* PHY in power up mode.*/
  mii_write(macp, MII_BMCR, mii_read(macp, MII_BMCR) & ~BMCR_PDOWN);
#endif

  /* MAC address setup.*/
  if (macp->config->mac_address == NULL)
    mac_lld_set_address(default_mac_address);
  else
    mac_lld_set_address(macp->config->mac_address);

  /* Transmitter and receiver enabled.
     Note that the complete setup of the MAC is performed when the link
     status is detected.*/
  uint32_t ncfgr = GMAC0->GMAC_NCFGR;

#if SAMA_MAC_IP_CHECKSUM_OFFLOAD
  GMAC0->GMAC_NCFGR = GMAC_NCFGR_SPD | GMAC_NCFGR_FD | GMAC_NCFGR_RXCOEN |
                      GMAC_NCFGR_MAXFS | GMAC_NCFGR_RFCS | ncfgr;
  GMAC0->GMAC_DCFGR |= GMAC_DCFGR_TXCOEN;
#else
  GMAC0->GMAC_NCFGR = GMAC_NCFGR_SPD | GMAC_NCFGR_FD |
                      GMAC_NCFGR_MAXFS | GMAC_NCFGR_RFCS| ncfgr;
#endif

  /* DMA configuration:
   * Descriptor chains pointers.
   */
  GMAC0->GMAC_RBQB = (uint32_t)__eth_rd;
  /*
   * The queue pointers must be initialized and point to
   * USED descriptor for all queues including those not
   * intended for use.
   */
  GMAC0->GMAC_RBQBAPQ[0] = (uint32_t)__eth_rd;
  GMAC0->GMAC_RBQBAPQ[1] = (uint32_t)__eth_rd;

  GMAC0->GMAC_TBQB = (uint32_t)__eth_td;
  /*
   * The queue pointers must be initialized and point to
   * USED descriptor for alla queues including those not
   * intended for use.
   */
  GMAC0->GMAC_TBQBAPQ[0] = (uint32_t)__eth_td1;
  GMAC0->GMAC_TBQBAPQ[1] = (uint32_t)__eth_td2;

  /* Enabling required interrupt sources.*/
  GMAC0->GMAC_IER  = GMAC_IER_TCOMP | GMAC_IER_RCOMP;

  /* DMA general settings.*/
  uint32_t dcfgr =  GMAC0->GMAC_DCFGR & 0xFFFF;
  GMAC0->GMAC_DCFGR = dcfgr | GMAC_DCFGR_DRBS(24);

  /* Enable RX and TX.*/
  GMAC0->GMAC_NCR |= GMAC_NCR_RXEN | GMAC_NCR_TXEN;
}

/**
 * @brief   Deactivates the MAC peripheral.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 *
 * @notapi
 */
void mac_lld_stop(MACDriver *macp) {

  if (macp->state != MAC_STOP) {
#if SAMA_MAC_ETH0_CHANGE_PHY_STATE
    /* PHY in power down mode until the driver will be restarted.*/
    mii_write(macp, MII_BMCR, mii_read(macp, MII_BMCR) | BMCR_PDOWN);
#endif

    /* Reset Network Control Register */
    GMAC0->GMAC_NCR &= ~(GMAC_NCR_RXEN | GMAC_NCR_TXEN);

    /* Disable interrupts */
    GMAC0->GMAC_IDR = 0xFFFFFFFF;

    /* Clear statistic */
    GMAC0->GMAC_NCR |= GMAC_NCR_CLRSTAT;

    /* Clear rx and tx status bit */
    GMAC0->GMAC_RSR = 0xF;
    GMAC0->GMAC_TSR = 0xFF;

    /* Clear interrupt status register */
    GMAC0->GMAC_ISR;

    /* MAC clocks stopped.*/
    pmcDisableETH0();

    /* ISR vector disabled.*/
    aicDisableInt(ID_GMAC0);
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
 * @retval MSG_OK       the descriptor has been obtained.
 * @retval MSG_TIMEOUT  descriptor not available.
 *
 * @notapi
 */
msg_t mac_lld_get_transmit_descriptor(MACDriver *macp,
                                      MACTransmitDescriptor *tdp) {
  sama_eth_tx_descriptor_t *tdes;

  if (!macp->link_up)
    return MSG_TIMEOUT;

  osalSysLock();

  /* Get Current TX descriptor.*/
  tdes = macp->txptr;

  /* Ensure that descriptor isn't owned by the Ethernet DMA or locked by
     another thread.*/
  if ((tdes->tdes1 & SAMA_TDES1_LOCKED) | (!(tdes->tdes1 & SAMA_TDES1_USED))) {
    osalSysUnlock();
    return MSG_TIMEOUT;
  }

  /* Marks the current descriptor as locked using a reserved bit.*/
  tdes->tdes1 |= SAMA_TDES1_LOCKED;

  if (!(tdes->tdes1 & SAMA_TDES1_WRAP)) {
    macp->txptr += 1;
  }
  else {
    macp->txptr = (sama_eth_tx_descriptor_t *)__eth_td;
  }

  osalSysUnlock();

  /* Set the buffer size and configuration.*/
  tdp->offset   = 0;
  tdp->size     = SAMA_MAC_BUFFERS_SIZE;
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
void mac_lld_release_transmit_descriptor(MACTransmitDescriptor *tdp) {

  osalDbgAssert(tdp->physdesc->tdes1 & SAMA_TDES1_USED,
              "attempt to release descriptor already owned by DMA");

  osalSysLock();

  /* Unlocks the descriptor and returns it to the DMA engine.*/
  tdp->physdesc->tdes1 &= ~(SAMA_TDES1_LOCKED | SAMA_TDES1_USED | SAMA_TDES1_LENGTH_BUFF);
  /* Configure lentgh of buffer */
  tdp->physdesc->tdes1 |= (SAMA_TDES1_LENGTH_BUFF & tdp->offset);

  /* Wait for the write to tdes1 to go through before resuming the DMA.*/
  __DSB();

  if (!(GMAC0->GMAC_TSR & GMAC_TSR_TXGO)) {
    GMAC0->GMAC_NCR |= GMAC_NCR_TSTART;
  }

  osalSysUnlock();
}

/**
 * @brief   Returns a receive descriptor.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @param[out] rdp      pointer to a @p MACReceiveDescriptor structure
 * @return              The operation status.
 * @retval MSG_OK       the descriptor has been obtained.
 * @retval MSG_TIMEOUT  descriptor not available.
 *
 * @notapi
 */
msg_t mac_lld_get_receive_descriptor(MACDriver *macp,
                                     MACReceiveDescriptor *rdp) {
  sama_eth_rx_descriptor_t *rdes;

  osalSysLock();

  /* Get Current RX descriptor.*/
  rdes = macp->rxptr;

  /* Iterates through received frames until a valid one is found, invalid
     frames are discarded.*/
  while (rdes->rdes0 & SAMA_RDES0_OWN) {
    if (rdes->rdes1 & (SAMA_RDES1_EOF | SAMA_RDES1_SOF)) {
      /* Found a valid one.*/
      rdp->offset   = 0;
      /* Only with RFCS set */
      rdp->size     = (rdes->rdes1 & SAMA_RDES1_LOF);
      rdp->physdesc = rdes;
      if (!(rdes->rdes0 & SAMA_RDES0_WRAP)) {
        macp->rxptr   += 1;
      }
      else {
        macp->rxptr = (sama_eth_rx_descriptor_t *)__eth_rd;
      }
      osalSysUnlock();
      return MSG_OK;
    }
    /* Invalid frame found, purging.*/
    rdes->rdes0 &= ~SAMA_RDES0_OWN;
    if (!(rdes->rdes0 & SAMA_RDES0_WRAP)) {
      rdes += 1;
    }
    else {
      rdes = (sama_eth_rx_descriptor_t *)__eth_rd;
    }
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
void mac_lld_release_receive_descriptor(MACReceiveDescriptor *rdp) {

  osalDbgAssert(rdp->physdesc->rdes0 & SAMA_RDES0_OWN,
              "attempt to release descriptor not owned by DMA");

  osalSysLock();

  /* Give buffer back to the Ethernet DMA.*/
  rdp->physdesc->rdes0 &= ~SAMA_RDES0_OWN;

  /* Wait for the write to rdes0 to go through before resuming the DMA.*/
  __DSB();

  osalSysUnlock();
}

/**
 * @brief   Updates and returns the link status.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @return              The link status.
 * @retval true         if the link is active.
 * @retval false        if the link is down.
 *
 * @notapi
 */
bool mac_lld_poll_link_status(MACDriver *macp) {
  uint32_t maccr, bmsr, bmcr;

  maccr = GMAC0->GMAC_NCFGR;

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
      maccr |= GMAC_NCFGR_SPD;
    else
      maccr &= ~GMAC_NCFGR_SPD;

    /* Check on link mode.*/
    if (lpa & (LPA_10FULL | LPA_100FULL))
      maccr |= GMAC_NCFGR_FD;
    else
      maccr &= ~GMAC_NCFGR_FD;
  }
  else {
    /* Link must be established.*/
    if (!(bmsr & BMSR_LSTATUS))
      return macp->link_up = false;

    /* Check on link speed.*/
    if (bmcr & BMCR_SPEED100)
      maccr |= GMAC_NCFGR_SPD;
    else
      maccr &= ~GMAC_NCFGR_SPD;

    /* Check on link mode.*/
    if (bmcr & BMCR_FULLDPLX)
      maccr |= GMAC_NCFGR_FD;
    else
      maccr &= ~GMAC_NCFGR_FD;
  }

  /* Changes the mode in the MAC.*/
  GMAC0->GMAC_NCFGR = maccr;

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
                                         size_t size) {

  osalDbgAssert((tdp->physdesc->tdes1 & SAMA_TDES1_USED),
               "attempt to write descriptor not owned");

  if (size > tdp->size - tdp->offset)
    size = tdp->size - tdp->offset;

  if (size > 0) {
    memcpy((uint8_t *)(tdp->physdesc->tdes0) + tdp->offset, buf, size);
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
                                       size_t size) {

  osalDbgAssert((rdp->physdesc->rdes0 & SAMA_RDES0_OWN),
              "attempt to read descriptor not owned");

  if (size > rdp->size - rdp->offset)
    size = rdp->size - rdp->offset;

  if (size > 0) {
    memcpy(buf, (uint8_t *)(rdp->physdesc->rdes0 & SAMA_RDES0_RBAP_MASK) + rdp->offset, size);
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
                                          size_t *sizep) {

  if (tdp->offset == 0) {
    *sizep      = tdp->size;
    tdp->offset = size;
    return (uint8_t *)tdp->physdesc->tdes0;
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
                                               size_t *sizep) {

  if (rdp->size > 0) {
    *sizep      = rdp->size;
    rdp->offset = rdp->size;
    rdp->size   = 0;
    return (uint8_t *)rdp->physdesc->rdes0 & SAMA_RDES0_RBAP_MASK;
  }
  *sizep = 0;
  return NULL;
}
#endif /* MAC_USE_ZERO_COPY */

#endif /* HAL_USE_MAC */

/** @} */
