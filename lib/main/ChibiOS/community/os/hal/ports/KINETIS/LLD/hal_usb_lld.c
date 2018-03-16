/*
    ChibiOS - Copyright (C) 2015 RedoX https://github.com/RedoXyde/
                        (C) 2015-2016 flabbergast <s3+flabbergast@sdfeu.org>

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
 * @file    KINETIS/LLD/usb_lld.c
 * @brief   KINETIS USB subsystem low level driver source.
 *
 * @addtogroup USB
 * @{
 */

#include <string.h>

#include "hal.h"

#if HAL_USE_USB || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief USB0 driver identifier.*/
#if KINETIS_USB_USE_USB0 || defined(__DOXYGEN__)
USBDriver USBD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   IN EP0 state.
 */
USBInEndpointState ep0in;

/**
 * @brief   OUT EP0 state.
 */
USBOutEndpointState ep0out;

/**
 * @brief   Buffer for the EP0 setup packets.
 */
static uint8_t ep0setup_buffer[8];

/**
 * @brief   EP0 initialization structure.
 */
static const USBEndpointConfig ep0config = {
  USB_EP_MODE_TYPE_CTRL,
  _usb_ep0setup,
  _usb_ep0in,
  _usb_ep0out,
  64,
  64,
  &ep0in,
  &ep0out,
  1,
  ep0setup_buffer
};

/*
 * Buffer Descriptor Table (BDT)
 */

/*
 * Buffer Descriptor (BD)
 * */
typedef struct {
	uint32_t desc;
	uint8_t* addr;
} bd_t;

/*
 * Buffer Descriptor fields - p.889
 */
#define BDT_OWN		0x80
#define BDT_DATA  0x40
#define BDT_KEEP  0x20
#define BDT_NINC  0x10
#define BDT_DTS		0x08
#define BDT_STALL	0x04

#define BDT_DESC(bc, data)	(BDT_OWN | BDT_DTS | ((data&0x1)<<6) | ((bc) << 16))

/*
 * BDT PID - p.891
 */
#define BDT_PID_OUT   0x01
#define BDT_PID_IN    0x09
#define BDT_PID_SETUP 0x0D
#define BDT_TOK_PID(n)	(((n)>>2)&0xF)

/*
 * BDT index fields
 */
#define DATA0 0
#define DATA1 1

#define RX   0
#define TX   1

#define EVEN 0
#define ODD  1

#define BDT_INDEX(endpoint, tx, odd) (((endpoint)<<2) | ((tx)<<1) | (odd))
/*
 * Get RX-ed/TX-ed bytes count from BDT entry
 */
#define BDT_BC(n) (((n)>>16)&0x3FF)

/* The USB-FS needs 2 BDT entry per endpoint direction
 *    that adds to: 2*2*16 BDT entries for 16 bi-directional EP
 */
static volatile bd_t _bdt[(KINETIS_USB_ENDPOINTS)*2*2] __attribute__((aligned(512)));

/* FIXME later with dyn alloc
 * 16 EP
 *  2 directions per EP
 *  2 buffer per direction
 * => 64 buffers
 */
static uint8_t _usbb[KINETIS_USB_ENDPOINTS*4][64] __attribute__((aligned(4)));
static volatile uint8_t _usbbn=0;
uint8_t* usb_alloc(uint8_t size)
{
  (void)size;
  if(_usbbn < (KINETIS_USB_ENDPOINTS)*4)
    return _usbb[_usbbn++];
  while(1); /* Should not happen, ever */
}
/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/* Called from locked ISR. */
void usb_packet_transmit(USBDriver *usbp, usbep_t ep, size_t n)
{
  const USBEndpointConfig *epc = usbp->epc[ep];
  USBInEndpointState *isp = epc->in_state;

  bd_t *bd = (bd_t *)&_bdt[BDT_INDEX(ep, TX, isp->odd_even)];
  
  if (n > (size_t)epc->in_maxsize)
    n = (size_t)epc->in_maxsize;

  /* Copy from buf to _usbb[] */
  size_t i=0;
  for(i=0;i<n;i++)
    bd->addr[i] = isp->txbuf[i];

  /* Update the Buffer status */
  bd->desc = BDT_DESC(n, isp->data_bank);
  /* Toggle the odd and data bits for next TX */
  isp->data_bank ^= DATA1;
  isp->odd_even ^= ODD;
}

/* Called from locked ISR. */
void usb_packet_receive(USBDriver *usbp, usbep_t ep, size_t n)
{
  const USBEndpointConfig *epc = usbp->epc[ep];
  USBOutEndpointState *osp = epc->out_state;

  bd_t *bd = (bd_t *)&_bdt[BDT_INDEX(ep, RX, osp->odd_even)];

  if (n > (size_t)epc->out_maxsize)
    n = (size_t)epc->out_maxsize;

  /* Copy from _usbb[] to buf  */
  size_t i=0;
  for(i=0;i<n;i++)
    osp->rxbuf[i] = bd->addr[i];

  /* Update the Buffer status
   * Set current buffer to same DATA bank and then toggle.
   * Since even/odd buffers are ping-pong and setup re-initialized them
   * this should work correctly */
  bd->desc = BDT_DESC(epc->out_maxsize, osp->data_bank);
  osp->data_bank ^= DATA1;
  usb_lld_start_out(usbp, ep);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*============================================================================*/

#if KINETIS_USB_USE_USB0 || defined(__DOXYGEN__)
/**
 * @brief   USB interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(KINETIS_USB_IRQ_VECTOR) {
  USBDriver *usbp = &USBD1;
  uint8_t istat = USB0->ISTAT;

  OSAL_IRQ_PROLOGUE();

  /* 04 - Bit2 - Start Of Frame token received */
  if(istat & USBx_ISTAT_SOFTOK) {
    _usb_isr_invoke_sof_cb(usbp);
    USB0->ISTAT = USBx_ISTAT_SOFTOK;
  }

  /* 08 - Bit3 - Token processing completed */
  while(istat & USBx_ISTAT_TOKDNE) {
    uint8_t stat = USB0->STAT;
    uint8_t ep = stat >> 4;
    if(ep > KINETIS_USB_ENDPOINTS) {
      OSAL_IRQ_EPILOGUE();
      return;
    }
    const USBEndpointConfig *epc = usbp->epc[ep];

    /* Get the correct BDT entry */
    uint8_t odd_even = (stat & USBx_STAT_ODD_MASK) >> USBx_STAT_ODD_SHIFT;
    uint8_t tx_rx    = (stat & USBx_STAT_TX_MASK) >> USBx_STAT_TX_SHIFT;
    bd_t *bd = (bd_t*)&_bdt[BDT_INDEX(ep,tx_rx,odd_even)];

    /* Update the ODD/EVEN state for RX */
    if(tx_rx == RX && epc->out_state != NULL)
      epc->out_state->odd_even = odd_even;

    switch(BDT_TOK_PID(bd->desc))
    {
      case BDT_PID_SETUP:                                              // SETUP
      {
        /* Clear any pending IN stuff */
        _bdt[BDT_INDEX(ep, TX, EVEN)].desc = 0;
        _bdt[BDT_INDEX(ep, TX,  ODD)].desc = 0;
        /* Also in the chibios state machine */
        (usbp)->receiving &= ~1;
        /* After a SETUP, IN is always DATA1 */
        usbp->epc[ep]->in_state->data_bank = DATA1;

        /* Call SETUP function (ChibiOS core), which sends back stuff */
        _usb_isr_invoke_setup_cb(usbp, ep);
        /* Buffer is released by the above callback. */
        /* from Paul: "unfreeze the USB, now that we're ready" */
        USB0->CTL = USBx_CTL_USBENSOFEN;
      } break;
      case BDT_PID_IN:                                                 // IN
      {
        if(epc->in_state == NULL)
          break;
        /* Special case for SetAddress for EP0 */
        if(ep == 0 && (((uint16_t)usbp->setup[0]<<8)|usbp->setup[1]) == 0x0500)
        {
          usbp->address = usbp->setup[2];
          usb_lld_set_address(usbp);
          _usb_isr_invoke_event_cb(usbp, USB_EVENT_ADDRESS);
          usbp->state = USB_SELECTED;
        }
        uint16_t txed = BDT_BC(bd->desc);
        epc->in_state->txcnt += txed;
        if(epc->in_state->txcnt < epc->in_state->txsize)
        {
          epc->in_state->txbuf += txed;
          osalSysLockFromISR();
          usb_packet_transmit(usbp,ep,epc->in_state->txsize - epc->in_state->txcnt);
          osalSysUnlockFromISR();
        }
        else
        {
          if(epc->in_cb != NULL)
            _usb_isr_invoke_in_cb(usbp,ep);
        }
      } break;
      case BDT_PID_OUT:                                                // OUT
      {
        if(epc->out_state == NULL)
          break;
        uint16_t rxed = BDT_BC(bd->desc);

        osalSysLockFromISR();
        usb_packet_receive(usbp,ep,rxed);
        osalSysUnlockFromISR();
        if(rxed)
        {
          epc->out_state->rxbuf += rxed;

          /* Update transaction data */
          epc->out_state->rxcnt              += rxed;
          epc->out_state->rxsize             -= rxed;
          epc->out_state->rxpkts             -= 1;

          /* The transaction is completed if the specified number of packets
             has been received or the current packet is a short packet.*/
          if ((rxed < epc->out_maxsize) || (epc->out_state->rxpkts == 0))
          {
            if(epc->out_cb != NULL)
              _usb_isr_invoke_out_cb(usbp, ep);
          }
        }
      } break;
      default:
        break;
    }
    USB0->ISTAT = USBx_ISTAT_TOKDNE;
    istat = USB0->ISTAT;
  }

  /* 01 - Bit0 - Valid USB Reset received */
  if(istat & USBx_ISTAT_USBRST) {
    _usb_reset(usbp);
    USB0->ISTAT = USBx_ISTAT_USBRST;
    OSAL_IRQ_EPILOGUE();
    return;
  }

  /* 80 - Bit7 - STALL handshake received */
  if(istat & USBx_ISTAT_STALL) {
    USB0->ISTAT = USBx_ISTAT_STALL;
  }

  /* 02 - Bit1 - ERRSTAT condition triggered */
  if(istat & USBx_ISTAT_ERROR) {
    uint8_t err = USB0->ERRSTAT;
    USB0->ERRSTAT = err;
    USB0->ISTAT = USBx_ISTAT_ERROR;
  }

  /* 10 - Bit4 - Constant IDLE on USB bus detected */
  if(istat & USBx_ISTAT_SLEEP) {
    /* This seems to fire a few times before the device is
     * configured - need to ignore those occurences somehow. */
    /* The other option would be to only activate INTEN_SLEEPEN
     * on CONFIGURED event, but that would need to be done in
     * user firmware. */
    if(usbp->state == USB_ACTIVE) {
      _usb_suspend(usbp);
      /* Enable interrupt on resume */
      USB0->INTEN |= USBx_INTEN_RESUMEEN;
    }

    // low-power version (check!):
    // enable wakeup interrupt on resume USB signaling
    //  (check that it was a wakeup int with USBx_USBTRC0_USB_RESUME_INT)
    //? USB0->USBTRC0 |= USBx_USBTRC0_USBRESMEN
    // suspend the USB module
    //? USB0->USBCTRL |= USBx_USBCTRL_SUSP;

    USB0->ISTAT = USBx_ISTAT_SLEEP;
  }

  /* 20 - Bit5 - Resume - Only allowed in sleep=suspend mode */
  if(istat & USBx_ISTAT_RESUME) {
    /* Disable interrupt on resume (should be disabled
     * during normal operation according to datasheet). */
    USB0->INTEN &= ~USBx_INTEN_RESUMEEN;

    // low power version (check!):
    // desuspend the USB module
    //? USB0->USBCTRL &= ~USBx_USBCTRL_SUSP;
    // maybe also
    //? USB0->CTL = USBx_CTL_USBENSOFEN;
    _usb_wakeup(usbp);
    USB0->ISTAT = USBx_ISTAT_RESUME;
  }

  /* 40 - Bit6 - ATTACH - used */

  OSAL_IRQ_EPILOGUE();
}
#endif /* KINETIS_USB_USE_USB0 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level USB driver initialization.
 *
 * @notapi
 */
void usb_lld_init(void) {
  /* Driver initialization.*/
  usbObjectInit(&USBD1);

#if KINETIS_USB_USE_USB0

  SIM->SOPT2 |= SIM_SOPT2_USBSRC;

#if defined(K20x5) || defined(K20x7)

#if KINETIS_MCG_MODE == KINETIS_MCG_MODE_FEI

  /* MCGOUTCLK is the SYSCLK frequency, so don't divide for USB clock */
  SIM->CLKDIV2 = SIM_CLKDIV2_USBDIV(0);

#elif KINETIS_MCG_MODE == KINETIS_MCG_MODE_PEE

  #define KINETIS_USBCLK_FREQUENCY 48000000UL
  uint32_t i,j;
  for(i=0; i < 2; i++) {
    for(j=0; j < 8; j++) {
      if((KINETIS_PLLCLK_FREQUENCY * (i+1)) == (KINETIS_USBCLK_FREQUENCY*(j+1))) {
        SIM->CLKDIV2 = i | SIM_CLKDIV2_USBDIV(j);
        goto usbfrac_match_found;
      }
    }
  }
  usbfrac_match_found:
  osalDbgAssert(i<2 && j <8,"USB Init error");

#else /* KINETIS_MCG_MODE == KINETIS_MCG_MODE_PEE */
#error USB clock setting not implemented for this KINETIS_MCG_MODE
#endif /* KINETIS_MCG_MODE == ... */

#elif defined(KL25) || defined (KL26) || defined(KL27)

  /* No extra clock dividers for USB clock */

#else /* defined(KL25) || defined (KL26) || defined(KL27) */
#error USB driver not implemented for your MCU type
#endif

#endif /* KINETIS_USB_USE_USB0 */
}

/**
 * @brief   Configures and activates the USB peripheral.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_start(USBDriver *usbp) {
  if (usbp->state == USB_STOP) {
#if KINETIS_USB_USE_USB0
    if (&USBD1 == usbp) {
      /* Clear BDT */
      uint8_t i;
      for(i=0;i<KINETIS_USB_ENDPOINTS;i++) {
        _bdt[i].desc=0;
        _bdt[i].addr=0;
      }

      /* Enable Clock */
#if KINETIS_USB0_IS_USBOTG
      SIM->SCGC4 |= SIM_SCGC4_USBOTG;
#else /* KINETIS_USB0_IS_USBOTG */
      SIM->SCGC4 |= SIM_SCGC4_USBFS;
#endif /* KINETIS_USB0_IS_USBOTG */

#if KINETIS_HAS_USB_CLOCK_RECOVERY
      USB0->CLK_RECOVER_IRC_EN |= USBx_CLK_RECOVER_IRC_EN_IRC_EN;
      USB0->CLK_RECOVER_CTRL |= USBx_CLK_RECOVER_CTRL_CLOCK_RECOVER_EN;
#endif /* KINETIS_HAS_USB_CLOCK_RECOVERY */

      /* Reset USB module, wait for completion */
      USB0->USBTRC0 |= USBx_USBTRC0_USBRESET;
      while ((USB0->USBTRC0 & USBx_USBTRC0_USBRESET));

      /* Set BDT Address */
      USB0->BDTPAGE1 = ((uint32_t)_bdt) >> 8;
      USB0->BDTPAGE2 = ((uint32_t)_bdt) >> 16;
      USB0->BDTPAGE3 = ((uint32_t)_bdt) >> 24;

      /* Clear all ISR flags */
      USB0->ISTAT = 0xFF;
      USB0->ERRSTAT = 0xFF;
#if KINETIS_USB0_IS_USBOTG
      USB0->OTGISTAT = 0xFF;
#endif /* KINETIS_USB0_IS_USBOTG */
      USB0->USBTRC0 |= 0x40; //a hint was given that this is an undocumented interrupt bit

      /* Enable USB */
      USB0->CTL = USBx_CTL_ODDRST | USBx_CTL_USBENSOFEN;
      USB0->USBCTRL = 0;

      /* Enable reset interrupt */
      USB0->INTEN |= USBx_INTEN_USBRSTEN;

      /* Enable interrupt in NVIC */
#if KINETIS_USB0_IS_USBOTG
      nvicEnableVector(USB_OTG_IRQn, KINETIS_USB_USB0_IRQ_PRIORITY);
#else /* KINETIS_USB0_IS_USBOTG */
      nvicEnableVector(USB_IRQn, KINETIS_USB_USB0_IRQ_PRIORITY);
#endif /* KINETIS_USB0_IS_USBOTG */
    }
#endif /* KINETIS_USB_USE_USB0 */
  }
}

/**
 * @brief   Deactivates the USB peripheral.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_stop(USBDriver *usbp) {
  /* TODO: If in ready state then disables the USB clock.*/
  if (usbp->state == USB_STOP) {
#if KINETIS_USB_USE_USB0
    if (&USBD1 == usbp) {
#if KINETIS_USB0_IS_USBOTG
      nvicDisableVector(USB_OTG_IRQn);
#else /* KINETIS_USB0_IS_USBOTG */
      nvicDisableVector(USB_IRQn);
#endif /* KINETIS_USB0_IS_USBOTG */
    }
#endif /* KINETIS_USB_USE_USB0 */
  }
}

/**
 * @brief   USB low level reset routine.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_reset(USBDriver *usbp) {
  // FIXME, dyn alloc
  _usbbn = 0;

#if KINETIS_USB_USE_USB0

  /* Reset BDT ODD/EVEN bits */
  USB0->CTL = USBx_CTL_ODDRST;

  /* EP0 initialization.*/
  usbp->epc[0] = &ep0config;
  usb_lld_init_endpoint(usbp, 0);

  /* Clear all pending interrupts */
  USB0->ERRSTAT = 0xFF;
  USB0->ISTAT = 0xFF;

  /* Set the address to zero during enumeration */
  usbp->address = 0;
  USB0->ADDR = 0;

  /* Enable other interrupts */
  USB0->ERREN = 0xFF;
  USB0->INTEN = USBx_INTEN_TOKDNEEN |
    USBx_INTEN_SOFTOKEN |
    USBx_INTEN_STALLEN |
    USBx_INTEN_ERROREN |
    USBx_INTEN_USBRSTEN |
    USBx_INTEN_SLEEPEN;

  /* "is this necessary?", Paul from PJRC */
  USB0->CTL = USBx_CTL_USBENSOFEN;
#endif /* KINETIS_USB_USE_USB0 */
}

/**
 * @brief   Sets the USB address.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_set_address(USBDriver *usbp) {

#if KINETIS_USB_USE_USB0
  USB0->ADDR = usbp->address&0x7F;
#endif /* KINETIS_USB_USE_USB0 */
}

/**
 * @brief   Enables an endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_init_endpoint(USBDriver *usbp, usbep_t ep) {

  if(ep > KINETIS_USB_ENDPOINTS)
    return;

  const USBEndpointConfig *epc = usbp->epc[ep];
  uint8_t mask=0;

  if(epc->out_state != NULL)
  {
    /* OUT Endpoint */
    epc->out_state->odd_even = EVEN;
    epc->out_state->data_bank = DATA0;
    /* RXe */
    _bdt[BDT_INDEX(ep, RX, EVEN)].desc = BDT_DESC(epc->out_maxsize, DATA0);
    _bdt[BDT_INDEX(ep, RX, EVEN)].addr = usb_alloc(epc->out_maxsize);
    /* RXo */
    _bdt[BDT_INDEX(ep, RX,  ODD)].desc = BDT_DESC(epc->out_maxsize, DATA1);
    _bdt[BDT_INDEX(ep, RX,  ODD)].addr = usb_alloc(epc->out_maxsize);
    /* Enable OUT direction */
    mask |= USBx_ENDPTn_EPRXEN;
  }
  if(epc->in_state != NULL)
  {
    /* IN Endpoint */
    epc->in_state->odd_even = EVEN;
    epc->in_state->data_bank = DATA0;
    /* TXe, not used yet */
    _bdt[BDT_INDEX(ep, TX, EVEN)].desc = 0;
    _bdt[BDT_INDEX(ep, TX, EVEN)].addr = usb_alloc(epc->in_maxsize);
    /* TXo, not used yet */
    _bdt[BDT_INDEX(ep, TX,  ODD)].desc = 0;
    _bdt[BDT_INDEX(ep, TX,  ODD)].addr = usb_alloc(epc->in_maxsize);
    /* Enable IN direction */
    mask |= USBx_ENDPTn_EPTXEN;
  }

  /* EPHSHK should be set for CTRL, BULK, INTR not for ISOC*/
  if((epc->ep_mode & USB_EP_MODE_TYPE) != USB_EP_MODE_TYPE_ISOC)
    mask |= USBx_ENDPTn_EPHSHK;
  /* Endpoint is not a CTRL endpoint, disable SETUP transfers */
  if((epc->ep_mode & USB_EP_MODE_TYPE) != USB_EP_MODE_TYPE_CTRL)
    mask |= USBx_ENDPTn_EPCTLDIS;

#if KINETIS_USB_USE_USB0
  USB0->ENDPT[ep].V = mask;
#endif /* KINETIS_USB_USE_USB0 */
}

/**
 * @brief   Disables all the active endpoints except the endpoint zero.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 *
 * @notapi
 */
void usb_lld_disable_endpoints(USBDriver *usbp) {
  (void)usbp;
  uint8_t i;
#if KINETIS_USB_USE_USB0
  for(i=1;i<KINETIS_USB_ENDPOINTS;i++)
    USB0->ENDPT[i].V = 0;
#endif /* KINETIS_USB_USE_USB0 */
}

/**
 * @brief   Returns the status of an OUT endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 * @return              The endpoint status.
 * @retval EP_STATUS_DISABLED The endpoint is not active.
 * @retval EP_STATUS_STALLED  The endpoint is stalled.
 * @retval EP_STATUS_ACTIVE   The endpoint is active.
 *
 * @notapi
 */
usbepstatus_t usb_lld_get_status_out(USBDriver *usbp, usbep_t ep) {
  (void)usbp;
#if KINETIS_USB_USE_USB0
  if(ep > USB_MAX_ENDPOINTS)
    return EP_STATUS_DISABLED;
  if(!(USB0->ENDPT[ep].V & (USBx_ENDPTn_EPRXEN)))
    return EP_STATUS_DISABLED;
  else if(USB0->ENDPT[ep].V & USBx_ENDPTn_EPSTALL)
    return EP_STATUS_STALLED;
  return EP_STATUS_ACTIVE;
#endif /* KINETIS_USB_USE_USB0 */
}

/**
 * @brief   Returns the status of an IN endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 * @return              The endpoint status.
 * @retval EP_STATUS_DISABLED The endpoint is not active.
 * @retval EP_STATUS_STALLED  The endpoint is stalled.
 * @retval EP_STATUS_ACTIVE   The endpoint is active.
 *
 * @notapi
 */
usbepstatus_t usb_lld_get_status_in(USBDriver *usbp, usbep_t ep) {
  (void)usbp;
  if(ep > USB_MAX_ENDPOINTS)
    return EP_STATUS_DISABLED;
#if KINETIS_USB_USE_USB0
  if(!(USB0->ENDPT[ep].V & (USBx_ENDPTn_EPTXEN)))
    return EP_STATUS_DISABLED;
  else if(USB0->ENDPT[ep].V & USBx_ENDPTn_EPSTALL)
    return EP_STATUS_STALLED;
  return EP_STATUS_ACTIVE;
#endif /* KINETIS_USB_USE_USB0 */
}

/**
 * @brief   Reads a setup packet from the dedicated packet buffer.
 * @details This function must be invoked in the context of the @p setup_cb
 *          callback in order to read the received setup packet.
 * @pre     In order to use this function the endpoint must have been
 *          initialized as a control endpoint.
 * @post    The endpoint is ready to accept another packet.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 * @param[out] buf      buffer where to copy the packet data
 *
 * @notapi
 */
void usb_lld_read_setup(USBDriver *usbp, usbep_t ep, uint8_t *buf) {
  /* Get the BDT entry */
  USBOutEndpointState *os = usbp->epc[ep]->out_state;
  bd_t *bd = (bd_t*)&_bdt[BDT_INDEX(ep, RX, os->odd_even)];
  /* Copy the 8 bytes of data */
  uint8_t n;
  for (n = 0; n < 8; n++) {
    buf[n] = bd->addr[n];
  }
  /* Release the buffer
   * Setup packet is always DATA0
   * Initialize buffers so current expects DATA0 & opposite DATA1 */
  bd->desc = BDT_DESC(usbp->epc[ep]->out_maxsize,DATA0);
  _bdt[BDT_INDEX(ep, RX, os->odd_even^ODD)].desc = BDT_DESC(usbp->epc[ep]->out_maxsize,DATA1);
  os->data_bank = DATA1;
}

/**
 * @brief   Starts a receive operation on an OUT endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_start_out(USBDriver *usbp, usbep_t ep) {
  USBOutEndpointState *osp = usbp->epc[ep]->out_state;
  /* Transfer initialization.*/
  if (osp->rxsize == 0)         /* Special case for zero sized packets.*/
    osp->rxpkts = 1;
  else
    osp->rxpkts = (uint16_t)((osp->rxsize + usbp->epc[ep]->out_maxsize - 1) /
                             usbp->epc[ep]->out_maxsize);
}

/**
 * @brief   Starts a transmit operation on an IN endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @note      Called from ISR and locked zone.
 * @notapi
 */
void usb_lld_start_in(USBDriver *usbp, usbep_t ep) {
  (void)usbp;
  (void)ep;
  usb_packet_transmit(usbp,ep,usbp->epc[ep]->in_state->txsize);
}

/**
 * @brief   Brings an OUT endpoint in the stalled state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_stall_out(USBDriver *usbp, usbep_t ep) {
  (void)usbp;
#if KINETIS_USB_USE_USB0
  USB0->ENDPT[ep].V |= USBx_ENDPTn_EPSTALL;
#endif /* KINETIS_USB_USE_USB0 */
}

/**
 * @brief   Brings an IN endpoint in the stalled state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_stall_in(USBDriver *usbp, usbep_t ep) {
  (void)usbp;
#if KINETIS_USB_USE_USB0
  USB0->ENDPT[ep].V |= USBx_ENDPTn_EPSTALL;
#endif /* KINETIS_USB_USE_USB0 */
}

/**
 * @brief   Brings an OUT endpoint in the active state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_clear_out(USBDriver *usbp, usbep_t ep) {
  (void)usbp;
#if KINETIS_USB_USE_USB0
  USB0->ENDPT[ep].V &= ~USBx_ENDPTn_EPSTALL;
#endif /* KINETIS_USB_USE_USB0 */
}

/**
 * @brief   Brings an IN endpoint in the active state.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 *
 * @notapi
 */
void usb_lld_clear_in(USBDriver *usbp, usbep_t ep) {
  (void)usbp;
#if KINETIS_USB_USE_USB0
  USB0->ENDPT[ep].V &= ~USBx_ENDPTn_EPSTALL;
#endif /* KINETIS_USB_USE_USB0 */
}

#endif /* HAL_USE_USB */

/** @} */
