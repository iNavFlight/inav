/*
    ChibiOS - Copyright (C) 2006..2017 Giovanni Di Sirio
              Copyright (C) 2015..2017 Diego Ismirlian, (dismirlian (at) google's mail)

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

#include "hal.h"

#if HAL_USE_USBH
#include "usbh/internal.h"
#include <string.h>

#if STM32_USBH_USE_OTG1
#if !defined(STM32_OTG1_CHANNELS_NUMBER)
#error "STM32_OTG1_CHANNELS_NUMBER must be defined"
#endif
#if !defined(STM32_OTG1_RXFIFO_SIZE)
#define STM32_OTG1_RXFIFO_SIZE		1024
#endif
#if !defined(STM32_OTG1_PTXFIFO_SIZE)
#define STM32_OTG1_PTXFIFO_SIZE		128
#endif
#if !defined(STM32_OTG1_NPTXFIFO_SIZE)
#define STM32_OTG1_NPTXFIFO_SIZE	128
#endif
#if (STM32_OTG1_RXFIFO_SIZE + STM32_OTG1_PTXFIFO_SIZE + STM32_OTG1_NPTXFIFO_SIZE) > (STM32_OTG1_FIFO_MEM_SIZE * 4)
#error "Not enough memory in OTG1 implementation"
#elif (STM32_OTG1_RXFIFO_SIZE + STM32_OTG1_PTXFIFO_SIZE + STM32_OTG1_NPTXFIFO_SIZE) < (STM32_OTG1_FIFO_MEM_SIZE * 4)
#warning "Spare memory in OTG1; could enlarge RX, PTX or NPTX FIFO sizes"
#endif
#if (STM32_OTG1_RXFIFO_SIZE % 4) || (STM32_OTG1_PTXFIFO_SIZE % 4) || (STM32_OTG1_NPTXFIFO_SIZE % 4)
#error "FIFO sizes must be a multiple of 32-bit words"
#endif
#endif

#if STM32_USBH_USE_OTG2
#if !defined(STM32_OTG2_CHANNELS_NUMBER)
#error "STM32_OTG2_CHANNELS_NUMBER must be defined"
#endif
#if !defined(STM32_OTG2_RXFIFO_SIZE)
#define STM32_OTG2_RXFIFO_SIZE		2048
#endif
#if !defined(STM32_OTG2_PTXFIFO_SIZE)
#define STM32_OTG2_PTXFIFO_SIZE		1024
#endif
#if !defined(STM32_OTG2_NPTXFIFO_SIZE)
#define STM32_OTG2_NPTXFIFO_SIZE	1024
#endif
#if (STM32_OTG2_RXFIFO_SIZE + STM32_OTG2_PTXFIFO_SIZE + STM32_OTG2_NPTXFIFO_SIZE) > (STM32_OTG2_FIFO_MEM_SIZE * 4)
#error "Not enough memory in OTG2 implementation"
#elif (STM32_OTG2_RXFIFO_SIZE + STM32_OTG2_PTXFIFO_SIZE + STM32_OTG2_NPTXFIFO_SIZE) < (STM32_OTG2_FIFO_MEM_SIZE * 4)
#warning "Spare memory in OTG2; could enlarge RX, PTX or NPTX FIFO sizes"
#endif
#if (STM32_OTG2_RXFIFO_SIZE % 4) || (STM32_OTG2_PTXFIFO_SIZE % 4) || (STM32_OTG2_NPTXFIFO_SIZE % 4)
#error "FIFO sizes must be a multiple of 32-bit words"
#endif
#endif


#if USBH_LLD_DEBUG_ENABLE_TRACE
#define udbgf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define udbg(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define udbgf(f, ...)  do {} while(0)
#define udbg(f, ...)   do {} while(0)
#endif

#if USBH_LLD_DEBUG_ENABLE_INFO
#define uinfof(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uinfo(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uinfof(f, ...)  do {} while(0)
#define uinfo(f, ...)   do {} while(0)
#endif

#if USBH_LLD_DEBUG_ENABLE_WARNINGS
#define uwarnf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uwarn(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uwarnf(f, ...)  do {} while(0)
#define uwarn(f, ...)   do {} while(0)
#endif

#if USBH_LLD_DEBUG_ENABLE_ERRORS
#define uerrf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uerr(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uerrf(f, ...)  do {} while(0)
#define uerr(f, ...)   do {} while(0)
#endif

static void _transfer_completedI(usbh_ep_t *ep, usbh_urb_t *urb, usbh_urbstatus_t status);
static void _try_commit_np(USBHDriver *host);
static void otg_rxfifo_flush(USBHDriver *usbp);
static void otg_txfifo_flush(USBHDriver *usbp, uint32_t fifo);

#if STM32_USBH_USE_OTG1
USBHDriver USBHD1;
#endif
#if STM32_USBH_USE_OTG2
USBHDriver USBHD2;
#endif

/*===========================================================================*/
/* Little helper functions.                                                  */
/*===========================================================================*/
static inline void _move_to_pending_queue(usbh_ep_t *ep) {
	list_move_tail(&ep->node, ep->pending_list);
}

static inline usbh_urb_t *_active_urb(usbh_ep_t *ep) {
	return list_first_entry(&ep->urb_list, usbh_urb_t, node);
}

static inline void _save_dt_mask(usbh_ep_t *ep, uint32_t hctsiz) {
	ep->dt_mask = hctsiz & HCTSIZ_DPID_MASK;
}

/*===========================================================================*/
/* Functions called from many places.                                        */
/*===========================================================================*/
static void _transfer_completedI(usbh_ep_t *ep, usbh_urb_t *urb, usbh_urbstatus_t status) {
	osalDbgCheckClassI();

	urb->queued = FALSE;

	/* remove URB from EP's queue */
	list_del_init(&urb->node);

	/* Call the callback function now, so that if it calls usbhURBSubmitI,
	 * the list_empty check below will be false. Also, note that the
	 *   if (list_empty(&ep->node)) {
	 *     ...
	 *   }
	 * in usbh_lld_urb_submit will be false, since the endpoint is
	 * still in the active queue.
	 */
	_usbh_urb_completeI(urb, status);

	if (list_empty(&ep->urb_list)) {
		/* no more URBs to process in this EP, remove EP from the host's queue */
		list_del_init(&ep->node);
	} else {
		/* more URBs to process */
		_move_to_pending_queue(ep);
	}
}

static void _halt_channel(USBHDriver *host, stm32_hc_management_t *hcm, usbh_lld_halt_reason_t reason) {
	(void)host;

	if (hcm->halt_reason != USBH_LLD_HALTREASON_NONE) {
		uwarnf("\t%s: Repeated halt (original=%d, new=%d)", hcm->ep->name, hcm->halt_reason, reason);
		return;
	}

#if CH_DBG_ENABLE_CHECKS
	if (usbhEPIsPeriodic(hcm->ep)) {
		osalDbgCheck(host->otg->HPTXSTS & HPTXSTS_PTXQSAV_MASK);
	} else {
		osalDbgCheck(host->otg->HNPTXSTS & HPTXSTS_PTXQSAV_MASK);
	}
#endif

	hcm->halt_reason = reason;
	hcm->hc->HCCHAR |= HCCHAR_CHENA | HCCHAR_CHDIS;
}

static void _release_channel(USBHDriver *host, stm32_hc_management_t *hcm) {
//	static const char *reason[] =  {"XFRC",	"XFRC",	"NAK", "STALL",	"ERROR", "ABORT"};
//	udbgf("\t%s: release (%s)", hcm->ep->name, reason[hcm->halt_reason]);
	hcm->hc->HCINTMSK = 0;
	host->otg->HAINTMSK &= ~hcm->haintmsk;
	hcm->halt_reason = USBH_LLD_HALTREASON_NONE;
	if (usbhEPIsPeriodic(hcm->ep)) {
		list_add(&hcm->node, &host->ch_free[0]);
	} else {
		list_add(&hcm->node, &host->ch_free[1]);
	}
	hcm->ep->xfer.hcm = 0;
	hcm->ep = 0;
}

static bool _activate_ep(USBHDriver *host, usbh_ep_t *ep) {
	struct list_head *list;
	uint16_t spc;

	osalDbgCheck(ep->xfer.hcm == NULL);

	if (usbhEPIsPeriodic(ep)) {
		list = &host->ch_free[0];
		spc = (host->otg->HPTXSTS >> 16) & 0xff;
	} else {
		list = &host->ch_free[1];
		spc = (host->otg->HNPTXSTS >> 16) & 0xff;
	}

	if (list_empty(list)) {
		uwarnf("\t%s: No free %s channels", ep->name, usbhEPIsPeriodic(ep) ? "P" : "NP");
		return FALSE;
	}

	if (spc <= STM32_USBH_MIN_QSPACE) {
		uwarnf("\t%s: No space in %s Queue (spc=%d)", ep->name, usbhEPIsPeriodic(ep) ? "P" : "NP", spc);
		return FALSE;
	}

	/* get the first channel */
	stm32_hc_management_t *hcm = list_first_entry(list, stm32_hc_management_t, node);
	osalDbgCheck((hcm->halt_reason == USBH_LLD_HALTREASON_NONE) && (hcm->ep == NULL));

	usbh_urb_t *const urb = _active_urb(ep);
	uint32_t hcintmsk = ep->hcintmsk;
	uint32_t hcchar = ep->hcchar;
	uint16_t mps = ep->wMaxPacketSize;

	uint32_t xfer_packets;
	uint32_t xfer_len = 0;	//Initialize just to shut up a compiler warning

	osalDbgCheck(urb->status == USBH_URBSTATUS_PENDING);

	/* check if the URB is a new one, or we must continue a previously started URB */
	if (urb->queued == FALSE) {
		/* prepare EP for a new URB */
		if (ep->type == USBH_EPTYPE_CTRL) {
			xfer_len = 8;
			ep->xfer.buf = (uint8_t *)urb->setup_buff;
			ep->dt_mask = HCTSIZ_DPID_SETUP;
			ep->in = FALSE;
			ep->xfer.u.ctrl_phase = USBH_LLD_CTRLPHASE_SETUP;
		} else {
			xfer_len = urb->requestedLength;
			ep->xfer.buf = urb->buff;
		}
		ep->xfer.error_count = 0;
	} else {
		osalDbgCheck(urb->requestedLength >= urb->actualLength);

		if (ep->type == USBH_EPTYPE_CTRL) {
			switch (ep->xfer.u.ctrl_phase) {
			case USBH_LLD_CTRLPHASE_SETUP:
				xfer_len = 8;
				ep->xfer.buf = (uint8_t *)urb->setup_buff;
				ep->dt_mask = HCTSIZ_DPID_SETUP;
				break;
			case USBH_LLD_CTRLPHASE_DATA:
				xfer_len = urb->requestedLength - urb->actualLength;
				ep->xfer.buf = (uint8_t *) urb->buff + urb->actualLength;
				break;
			case USBH_LLD_CTRLPHASE_STATUS:
				xfer_len = 0;
				ep->dt_mask = HCTSIZ_DPID_DATA1;
				ep->xfer.error_count = 0;
				break;
			default:
				osalDbgCheck(0);
			}
			if (ep->in) {
				hcintmsk |= HCINTMSK_DTERRM | HCINTMSK_BBERRM;
				hcchar |= HCCHAR_EPDIR;
			}
		} else {
			xfer_len = urb->requestedLength - urb->actualLength;
			ep->xfer.buf = (uint8_t *) urb->buff + urb->actualLength;
		}

		if (ep->xfer.error_count)
			hcintmsk |= HCINTMSK_ACKM;

	}
	ep->xfer.partial = 0;

	if (ep->type == USBH_EPTYPE_ISO) {
		ep->dt_mask = HCTSIZ_DPID_DATA0;

		/* [USB 2.0 spec, 5.6.4]: A host must not issue more than 1
		 * transaction in a (micro)frame for an isochronous endpoint
		 * unless the endpoint is high-speed, high-bandwidth.
		 */
		if (xfer_len > mps)
			xfer_len = mps;
	} else if (xfer_len > 0x7FFFF) {
		xfer_len = 0x7FFFF - mps + 1;
	}

	/* calculate required packets */
	if (xfer_len) {
		xfer_packets = (xfer_len + mps - 1) / mps;

		if (xfer_packets > 0x3FF) {
			xfer_packets = 0x3FF;
			xfer_len = xfer_packets * mps;
		}
	} else {
		xfer_packets = 1;	/* Need 1 packet for transfer length of 0 */
	}

	if (ep->in)
		xfer_len = xfer_packets * mps;

	/* Clear old interrupt conditions,
	 * configure transfer size,
	 * enable required interrupts */
	stm32_otg_host_chn_t *const hc = hcm->hc;
	hc->HCINT = 0xffffffff;
	hc->HCTSIZ = ep->dt_mask
					| HCTSIZ_PKTCNT(xfer_packets)
					| HCTSIZ_XFRSIZ(xfer_len);
	hc->HCINTMSK = hcintmsk;

	/* Queue the transfer for the next frame (no effect for non-periodic transfers) */
	if (!(host->otg->HFNUM & 1))
		hcchar |= HCCHAR_ODDFRM;

	/* configure channel characteristics and queue a request */
	hc->HCCHAR = hcchar;
	if (ep->in && (xfer_packets > 1)) {
		/* For IN transfers, try to queue two back-to-back packets.
		 * This results in a 1% performance gain for Full Speed transfers
		 */
		if (--spc > STM32_USBH_MIN_QSPACE) {
			hc->HCCHAR |= HCCHAR_CHENA;
		} else {
			uwarnf("\t%s: Could not queue back-to-back packets", ep->name);
		}
	}

	if (urb->queued == FALSE) {
		urb->queued = TRUE;
		udbgf("\t%s: Start (%dB)", ep->name, xfer_len);
	} else {
		udbgf("\t%s: Restart (%dB)", ep->name, xfer_len);
	}

	ep->xfer.len = xfer_len;
	ep->xfer.packets = (uint16_t)xfer_packets;

	/* remove the channel from the free list, link endpoint <-> channel and move to the active queue*/
	list_del(&hcm->node);
	ep->xfer.hcm = hcm;
	hcm->ep = ep;
	list_move_tail(&ep->node, ep->active_list);


	stm32_otg_t *const otg = host->otg;

	/* enable this channel's interrupt and global channel interrupt */
	otg->HAINTMSK |= hcm->haintmsk;
	if (ep->in) {
		otg->GINTMSK |= GINTMSK_HCM;
	} else if (usbhEPIsPeriodic(ep)) {
		otg->GINTMSK |= GINTMSK_HCM | GINTMSK_PTXFEM;
	} else {
		//TODO: write to the FIFO now
		otg->GINTMSK |= GINTMSK_HCM | GINTMSK_NPTXFEM;
	}

	return TRUE;
}

static bool _update_urb(usbh_ep_t *ep, uint32_t hctsiz, usbh_urb_t *urb, bool completed) {
	uint32_t len;

	if (!completed) {
		len = ep->wMaxPacketSize * (ep->xfer.packets - ((hctsiz & HCTSIZ_PKTCNT_MASK) >> 19));
	} else {
		if (ep->in) {
			len = ep->xfer.len - ((hctsiz & HCTSIZ_XFRSIZ_MASK) >> 0);
		} else {
			len = ep->xfer.len;
		}
		osalDbgCheck(len == ep->xfer.partial);	//TODO: if len == ep->xfer.partial, use this instead of the above code
	}

#if 0
	osalDbgAssert(urb->actualLength + len <= urb->requestedLength, "what happened?");
#else
	if (urb->actualLength + len > urb->requestedLength) {
		uerrf("\t%s: Trimming actualLength %u -> %u", ep->name, urb->actualLength + len, urb->requestedLength);
		urb->actualLength = urb->requestedLength;
		return TRUE;
	}
#endif

	urb->actualLength += len;
	if ((urb->actualLength == urb->requestedLength)
			|| (ep->in && completed && (hctsiz & HCTSIZ_XFRSIZ_MASK)))
		return TRUE;

	return FALSE;
}

static void _try_commit_np(USBHDriver *host) {
	usbh_ep_t *item, *tmp;

	list_for_each_entry_safe(item, usbh_ep_t, tmp, &host->ep_pending_lists[USBH_EPTYPE_CTRL], node) {
		if (!_activate_ep(host, item))
			return;
	}

	list_for_each_entry_safe(item, usbh_ep_t, tmp, &host->ep_pending_lists[USBH_EPTYPE_BULK], node) {
		if (!_activate_ep(host, item))
			return;
	}
}

static void _try_commit_p(USBHDriver *host, bool sof) {
	usbh_ep_t *item, *tmp;

	list_for_each_entry_safe(item, usbh_ep_t, tmp, &host->ep_pending_lists[USBH_EPTYPE_ISO], node) {
		if (!_activate_ep(host, item))
			return;
	}

	list_for_each_entry_safe(item, usbh_ep_t, tmp, &host->ep_pending_lists[USBH_EPTYPE_INT], node) {
		osalDbgCheck(item);
		/* TODO: improve this */
		if (sof && item->xfer.u.frame_counter)
			--item->xfer.u.frame_counter;

		if (item->xfer.u.frame_counter == 0) {
			if (!_activate_ep(host, item))
				return;
			item->xfer.u.frame_counter = item->bInterval;
		}
	}

	if (list_empty(&host->ep_pending_lists[USBH_EPTYPE_ISO])
		&& list_empty(&host->ep_pending_lists[USBH_EPTYPE_INT])) {
		host->otg->GINTMSK &= ~GINTMSK_SOFM;
	} else {
		host->otg->GINTMSK |= GINTMSK_SOFM;
	}
}

static void _purge_queue(USBHDriver *host, struct list_head *list) {
	usbh_ep_t *ep, *tmp;
	list_for_each_entry_safe(ep, usbh_ep_t, tmp, list, node) {
		usbh_urb_t *const urb = _active_urb(ep);
		stm32_hc_management_t *const hcm = ep->xfer.hcm;
		uwarnf("\t%s: Abort URB, USBH_URBSTATUS_DISCONNECTED", ep->name);
		if (hcm) {
			uwarnf("\t%s: URB had channel %d assigned, halt_reason = %d", ep->name, hcm - host->channels, hcm->halt_reason);
			_release_channel(host, hcm);
			_update_urb(ep, hcm->hc->HCTSIZ, urb, FALSE);
		}
		_transfer_completedI(ep, urb, USBH_URBSTATUS_DISCONNECTED);
	}
}

static void _purge_active(USBHDriver *host) {
	_purge_queue(host, &host->ep_active_lists[0]);
	_purge_queue(host, &host->ep_active_lists[1]);
	_purge_queue(host, &host->ep_active_lists[2]);
	_purge_queue(host, &host->ep_active_lists[3]);
}

static void _purge_pending(USBHDriver *host) {
	_purge_queue(host, &host->ep_pending_lists[0]);
	_purge_queue(host, &host->ep_pending_lists[1]);
	_purge_queue(host, &host->ep_pending_lists[2]);
	_purge_queue(host, &host->ep_pending_lists[3]);
}

static uint32_t _write_packet(struct list_head *list, uint32_t space_available) {
	usbh_ep_t *ep;

	uint32_t remaining = 0;

	list_for_each_entry(ep, usbh_ep_t, list, node) {
		if (ep->in || (ep->xfer.hcm->halt_reason != USBH_LLD_HALTREASON_NONE))
			continue;

		int32_t rem = ep->xfer.len - ep->xfer.partial;
		osalDbgCheck(rem >= 0);
		if (rem <= 0)
			continue;

		remaining += rem;

		if (!space_available) {
			if (remaining)
				break;

			continue;
		}

		/* write one packet only */
		if (rem > ep->wMaxPacketSize)
			rem = ep->wMaxPacketSize;

		/* round up to dwords */
		uint32_t words = (rem + 3) / 4;

		if (words > space_available)
			words = space_available;

		space_available -= words;

		uint32_t written = words * 4;
		if ((int32_t)written > rem)
			written = rem;

		volatile uint32_t *dest = ep->xfer.hcm->fifo;
		uint32_t *src = (uint32_t *)ep->xfer.buf;
		udbgf("\t%s: write %d words (%dB), partial=%d", ep->name, words, written, ep->xfer.partial);
		while (words--) {
			*dest = *src++;
		}

		ep->xfer.buf += written;
		ep->xfer.partial += written;

		remaining -= written;
	}

	return remaining;
}


/*===========================================================================*/
/* API.                                                                      */
/*===========================================================================*/

void usbh_lld_ep_object_init(usbh_ep_t *ep) {
/*			CTRL(IN)	CTRL(OUT)	INT(IN)		INT(OUT)	BULK(IN)	BULK(OUT)	ISO(IN)		ISO(OUT)
 * STALL		si		solo DAT/STAT	si			si			si			si			no			no		ep->type != ISO && (ep->type != CTRL || ctrlphase != SETUP)
 * ACK			si			si			si			si			si			si			no			no		ep->type != ISO
 * NAK			si			si			si			si			si			si			no			no		ep->type != ISO
 * BBERR		si			no			si			no			si			no			si			no		ep->in
 * TRERR		si			si			si			si			si			si			si			no		ep->type != ISO || ep->in
 * DTERR		si			no			si			no			si			no			no			no		ep->type != ISO && ep->in
 * FRMOR		no			no			si			si			no			no			si			si		ep->type = PERIODIC
 */
	USBHDriver *host = ep->device->host;
	uint32_t hcintmsk = HCINTMSK_CHHM | HCINTMSK_XFRCM | HCINTMSK_AHBERRM;

	switch (ep->type) {
	case USBH_EPTYPE_ISO:
		hcintmsk |= HCINTMSK_FRMORM;
		if (ep->in) {
			hcintmsk |= HCINTMSK_TRERRM | HCINTMSK_BBERRM;
		}
		break;
	case USBH_EPTYPE_INT:
		hcintmsk |= HCINTMSK_TRERRM | HCINTMSK_FRMORM | HCINTMSK_STALLM | HCINTMSK_NAKM;
		if (ep->in) {
			hcintmsk |= HCINTMSK_DTERRM | HCINTMSK_BBERRM;
		}
		ep->xfer.u.frame_counter = 1;
		break;
	case USBH_EPTYPE_CTRL:
		hcintmsk |= HCINTMSK_TRERRM | HCINTMSK_STALLM | HCINTMSK_NAKM;
		break;
	case USBH_EPTYPE_BULK:
		hcintmsk |= HCINTMSK_TRERRM | HCINTMSK_STALLM | HCINTMSK_NAKM;
		if (ep->in) {
			hcintmsk |= HCINTMSK_DTERRM | HCINTMSK_BBERRM;
		}
		break;
	default:
		chDbgCheck(0);
	}
	ep->active_list = &host->ep_active_lists[ep->type];
	ep->pending_list = &host->ep_pending_lists[ep->type];
	INIT_LIST_HEAD(&ep->urb_list);
	INIT_LIST_HEAD(&ep->node);

	ep->hcintmsk = hcintmsk;
	ep->hcchar = HCCHAR_CHENA
			| HCCHAR_DAD(ep->device->address)
			| HCCHAR_MCNT(1)
			| HCCHAR_EPTYP(ep->type)
			| ((ep->device->speed == USBH_DEVSPEED_LOW) ? HCCHAR_LSDEV : 0)
			| (ep->in ? HCCHAR_EPDIR : 0)
			| HCCHAR_EPNUM(ep->address)
			| HCCHAR_MPS(ep->wMaxPacketSize);
}

void usbh_lld_ep_open(usbh_ep_t *ep) {
	uinfof("\t%s: Open EP", ep->name);
	ep->status = USBH_EPSTATUS_OPEN;
}

void usbh_lld_ep_close(usbh_ep_t *ep) {
	usbh_urb_t *urb;
	uinfof("\t%s: Closing EP...", ep->name);
	while (!list_empty(&ep->urb_list)) {
		urb = list_first_entry(&ep->urb_list, usbh_urb_t, node);
		uinfof("\t%s: Abort URB, USBH_URBSTATUS_DISCONNECTED", ep->name);
		_usbh_urb_abort_and_waitS(urb, USBH_URBSTATUS_DISCONNECTED);
	}
	uinfof("\t%s: Closed", ep->name);
	ep->status = USBH_EPSTATUS_CLOSED;
}

bool usbh_lld_ep_reset(usbh_ep_t *ep) {
	ep->dt_mask = HCTSIZ_DPID_DATA0;
	return TRUE;
}

void usbh_lld_urb_submit(usbh_urb_t *urb) {
	usbh_ep_t *const ep = urb->ep;
	USBHDriver *const host = ep->device->host;

	if (!(host->otg->HPRT & HPRT_PENA)) {
		uwarnf("\t%s: Can't submit URB, port disabled", ep->name);
		_usbh_urb_completeI(urb, USBH_URBSTATUS_DISCONNECTED);
		return;
	}

	/* add the URB to the EP's queue */
	list_add_tail(&urb->node, &ep->urb_list);

	/* check if the EP wasn't in any queue (pending nor active) */
	if (list_empty(&ep->node)) {

		/* add the EP to the pending queue */
		_move_to_pending_queue(ep);

		if (usbhEPIsPeriodic(ep)) {
			host->otg->GINTMSK |= GINTMSK_SOFM;
		} else {
			/* try to queue non-periodic transfers */
			_try_commit_np(ep->device->host);
		}
	}
}

/* usbh_lld_urb_abort may require a reschedule if called from a S-locked state */
bool usbh_lld_urb_abort(usbh_urb_t *urb, usbh_urbstatus_t status) {
	osalDbgCheck(usbhURBIsBusy(urb));

	usbh_ep_t *const ep = urb->ep;
	osalDbgCheck(ep);
	stm32_hc_management_t *const hcm = ep->xfer.hcm;

	if ((hcm != NULL) && (urb == _active_urb(ep))) {
		/* This URB is active (channel assigned, top of the EP's URB list) */

		if (hcm->halt_reason == USBH_LLD_HALTREASON_NONE) {
			/* The channel is not being halted */
			uinfof("\t%s: usbh_lld_urb_abort: channel is not being halted", hcm->ep->name);
			urb->status = status;
			_halt_channel(ep->device->host, hcm, USBH_LLD_HALTREASON_ABORT);
		} else {
			/* The channel is being halted, so we can't re-halt it. The CHH interrupt will
			 * be in charge of completing the transfer, but the URB will not have the specified status.
			 */
			uinfof("\t%s: usbh_lld_urb_abort: channel is being halted", hcm->ep->name);
		}
		return FALSE;
	}

	/* This URB is inactive, we can cancel it now */
	uinfof("\t%s: usbh_lld_urb_abort: URB is not active", ep->name);
	_transfer_completedI(ep, urb, status);

	return TRUE;
}


/*===========================================================================*/
/* Channel Interrupts.                                                       */
/*===========================================================================*/

//CTRL(IN)	CTRL(OUT)	INT(IN)		INT(OUT)	BULK(IN)	BULK(OUT)	ISO(IN)		ISO(OUT)
//	si			si			si			si			si			si			no			no		ep->type != ISO && !ep->in
static inline void _ack_int(USBHDriver *host, stm32_hc_management_t *hcm, stm32_otg_host_chn_t *hc) {
	(void)host;
	osalDbgAssert(hcm->ep->type != USBH_EPTYPE_ISO, "ACK should not happen in ISO endpoints");
	hcm->ep->xfer.error_count = 0;
	hc->HCINTMSK &= ~HCINTMSK_ACKM;
	udbgf("\t%s: ACK", hcm->ep->name);
}

//CTRL(IN)	CTRL(OUT)	INT(IN)		INT(OUT)	BULK(IN)	BULK(OUT)	ISO(IN)		ISO(OUT)
//	si			no			si			no			si			no			no			no		ep->type != ISO && ep->in
static inline void _dterr_int(USBHDriver *host, stm32_hc_management_t *hcm, stm32_otg_host_chn_t *hc) {
	(void)host;
	osalDbgAssert(hcm->ep->in && (hcm->ep->type != USBH_EPTYPE_ISO), "DTERR should not happen in OUT or ISO endpoints");
#if 0
	hc->HCINTMSK &= ~(HCINTMSK_DTERRM | HCINTMSK_ACKM);
	hcm->ep->xfer.error_count = 0;
	_halt_channel(host, hcm, USBH_LLD_HALTREASON_ERROR);
#else
	/* restart directly, no need to halt it in this case */
	hcm->ep->xfer.error_count = 0;
	hc->HCINTMSK &= ~HCINTMSK_ACKM;
	hc->HCCHAR |= HCCHAR_CHENA;
#endif
	uerrf("\t%s: DTERR", hcm->ep->name);
}

//CTRL(IN)	CTRL(OUT)	INT(IN)		INT(OUT)	BULK(IN)	BULK(OUT)	ISO(IN)		ISO(OUT)
//	si			no			si			no			si			no			si			no		ep->in
static inline void _bberr_int(USBHDriver *host, stm32_hc_management_t *hcm, stm32_otg_host_chn_t *hc) {
	osalDbgAssert(hcm->ep->in, "BBERR should not happen in OUT endpoints");
	hc->HCINTMSK &= ~HCINTMSK_BBERRM;
	hcm->ep->xfer.error_count = 3;
	_halt_channel(host, hcm, USBH_LLD_HALTREASON_ERROR);
	uerrf("\t%s: BBERR", hcm->ep->name);
}

///CTRL(IN)	CTRL(OUT)	INT(IN)		INT(OUT)	BULK(IN)	BULK(OUT)	ISO(IN)		ISO(OUT)
//	si			si			si			si			si			si			si			no		ep->type != ISO || ep->in
static inline void _trerr_int(USBHDriver *host, stm32_hc_management_t *hcm, stm32_otg_host_chn_t *hc) {
	osalDbgAssert(hcm->ep->in || (hcm->ep->type != USBH_EPTYPE_ISO), "TRERR should not happen in ISO OUT endpoints");
	hc->HCINTMSK &= ~HCINTMSK_TRERRM;
	++hcm->ep->xfer.error_count;
	_halt_channel(host, hcm, USBH_LLD_HALTREASON_ERROR);
	uerrf("\t%s: TRERR", hcm->ep->name);
}

//CTRL(IN)	CTRL(OUT)	INT(IN)		INT(OUT)	BULK(IN)	BULK(OUT)	ISO(IN)		ISO(OUT)
//	no			no			si			si			no			no			si			si		ep->type = PERIODIC
static inline void _frmor_int(USBHDriver *host, stm32_hc_management_t *hcm, stm32_otg_host_chn_t *hc) {
	osalDbgAssert(usbhEPIsPeriodic(hcm->ep), "FRMOR should not happen in non-periodic endpoints");
	hc->HCINTMSK &= ~HCINTMSK_FRMORM;
	hcm->ep->xfer.error_count = 3;
	_halt_channel(host, hcm, USBH_LLD_HALTREASON_ERROR);
	uerrf("\t%s: FRMOR", hcm->ep->name);
}

//CTRL(IN)	CTRL(OUT)	INT(IN)		INT(OUT)	BULK(IN)	BULK(OUT)	ISO(IN)		ISO(OUT)
//	si			si			si			si			si			si			no			no		ep->type != ISO
static inline void _nak_int(USBHDriver *host, stm32_hc_management_t *hcm, stm32_otg_host_chn_t *hc) {
	osalDbgAssert(hcm->ep->type != USBH_EPTYPE_ISO, "NAK should not happen in ISO endpoints");
	if (!hcm->ep->in || (hcm->ep->type == USBH_EPTYPE_INT)) {
		hc->HCINTMSK &= ~HCINTMSK_NAKM;
		_halt_channel(host, hcm, USBH_LLD_HALTREASON_NAK);
	} else {
		/* restart directly, no need to halt it in this case */
		hcm->ep->xfer.error_count = 0;
		hc->HCINTMSK &= ~HCINTMSK_ACKM;
		hc->HCCHAR |= HCCHAR_CHENA;
	}
	udbgf("\t%s: NAK", hcm->ep->name);
}

//CTRL(IN)	CTRL(OUT)	INT(IN)		INT(OUT)	BULK(IN)	BULK(OUT)	ISO(IN)		ISO(OUT)
//	si		sólo DAT/STAT	si			si			si			si			no			no		ep->type != ISO && (ep->type != CTRL || ctrlphase != SETUP)
static inline void _stall_int(USBHDriver *host, stm32_hc_management_t *hcm, stm32_otg_host_chn_t *hc) {
	osalDbgAssert(hcm->ep->type != USBH_EPTYPE_ISO, "STALL should not happen in ISO endpoints");
	hc->HCINTMSK &= ~HCINTMSK_STALLM;
	_halt_channel(host, hcm, USBH_LLD_HALTREASON_STALL);
	uwarnf("\t%s: STALL", hcm->ep->name);
}

static void _complete_bulk_int(USBHDriver *host, stm32_hc_management_t *hcm, usbh_ep_t *ep, usbh_urb_t *urb, uint32_t hctsiz) {
	_release_channel(host, hcm);
	_save_dt_mask(ep, hctsiz);
	if (_update_urb(ep, hctsiz, urb, TRUE)) {
		udbgf("\t%s: done", ep->name);
		_transfer_completedI(ep, urb, USBH_URBSTATUS_OK);
	} else {
		osalDbgCheck(urb->requestedLength > 0x7FFFF);
		uwarnf("\t%s: incomplete", ep->name);
		_move_to_pending_queue(ep);
	}
	if (usbhEPIsPeriodic(ep)) {
		_try_commit_p(host, FALSE);
	} else {
		_try_commit_np(host);
	}
}

static void _complete_control(USBHDriver *host, stm32_hc_management_t *hcm, usbh_ep_t *ep, usbh_urb_t *urb, uint32_t hctsiz) {
	osalDbgCheck(ep->xfer.u.ctrl_phase != USBH_LLD_CTRLPHASE_SETUP);

	_release_channel(host, hcm);
	if (ep->xfer.u.ctrl_phase == USBH_LLD_CTRLPHASE_DATA) {
		if (_update_urb(ep, hctsiz, urb, TRUE)) {
			udbgf("\t%s: DATA done", ep->name);
			ep->xfer.u.ctrl_phase = USBH_LLD_CTRLPHASE_STATUS;
			ep->in = !ep->in;
		} else {
			osalDbgCheck(urb->requestedLength > 0x7FFFF);
			uwarnf("\t%s: DATA incomplete", ep->name);
			_save_dt_mask(ep, hctsiz);
		}
		_move_to_pending_queue(ep);
	} else {
		osalDbgCheck(ep->xfer.u.ctrl_phase == USBH_LLD_CTRLPHASE_STATUS);
		udbgf("\t%s: STATUS done", ep->name);
		_transfer_completedI(ep, urb, USBH_URBSTATUS_OK);
	}
	_try_commit_np(host);
}

static void _complete_control_setup(USBHDriver *host, stm32_hc_management_t *hcm, usbh_ep_t *ep, usbh_urb_t *urb) {
	_release_channel(host, hcm);
	if (urb->requestedLength) {
		udbgf("\t%s: SETUP done -> DATA", ep->name);
		ep->xfer.u.ctrl_phase = USBH_LLD_CTRLPHASE_DATA;
		ep->in = *((uint8_t *)urb->setup_buff) & 0x80 ? TRUE : FALSE;
		ep->dt_mask = HCTSIZ_DPID_DATA1;
		ep->xfer.error_count = 0;
	} else {
		udbgf("\t%s: SETUP done -> STATUS", ep->name);
		ep->in = TRUE;
		ep->xfer.u.ctrl_phase = USBH_LLD_CTRLPHASE_STATUS;
	}
	_move_to_pending_queue(ep);
	_try_commit_np(host);
}

static void _complete_iso(USBHDriver *host, stm32_hc_management_t *hcm, usbh_ep_t *ep, usbh_urb_t *urb, uint32_t hctsiz) {
	udbgf("\t%s: done", hcm->ep->name);
	_release_channel(host, hcm);
	_update_urb(ep, hctsiz, urb, TRUE);
	_transfer_completedI(ep, urb, USBH_URBSTATUS_OK);
	_try_commit_p(host, FALSE);
}

static inline void _xfrc_int(USBHDriver *host, stm32_hc_management_t *hcm, stm32_otg_host_chn_t *hc) {
	usbh_ep_t *const ep = hcm->ep;
	usbh_urb_t *const urb = _active_urb(ep);
	osalDbgCheck(urb);
	uint32_t hctsiz = hc->HCTSIZ;

	hc->HCINTMSK &= ~HCINTMSK_XFRCM;

	switch (ep->type) {
	case USBH_EPTYPE_CTRL:
		if (ep->xfer.u.ctrl_phase == USBH_LLD_CTRLPHASE_SETUP) {
			_complete_control_setup(host, hcm, ep, urb);
		} else if (ep->in) {
			_halt_channel(host, hcm, USBH_LLD_HALTREASON_XFRC);
		} else {
			_complete_control(host, hcm, ep, urb, hctsiz);
		}
		break;

	case USBH_EPTYPE_BULK:
		if (ep->in) {
			_halt_channel(host, hcm, USBH_LLD_HALTREASON_XFRC);
		} else {
			_complete_bulk_int(host, hcm, ep, urb, hctsiz);
		}
		break;

	case USBH_EPTYPE_INT:
		if (ep->in && (hctsiz & HCTSIZ_PKTCNT_MASK)) {
			_halt_channel(host, hcm, USBH_LLD_HALTREASON_XFRC);
		} else {
			_complete_bulk_int(host, hcm, ep, urb, hctsiz);
		}
		break;

	case USBH_EPTYPE_ISO:
		if (ep->in && (hctsiz & HCTSIZ_PKTCNT_MASK)) {
			_halt_channel(host, hcm, USBH_LLD_HALTREASON_XFRC);
		} else {
			_complete_iso(host, hcm, ep, urb, hctsiz);
		}
		break;
	}
}

static inline void _chh_int(USBHDriver *host, stm32_hc_management_t *hcm, stm32_otg_host_chn_t *hc) {

	usbh_ep_t *const ep = hcm->ep;
	usbh_urb_t *const urb = _active_urb(ep);
	osalDbgCheck(urb);
	uint32_t hctsiz = hc->HCTSIZ;
	usbh_lld_halt_reason_t reason = hcm->halt_reason;

	//osalDbgCheck(reason != USBH_LLD_HALTREASON_NONE);
	if (reason == USBH_LLD_HALTREASON_NONE) {
		uwarnf("\tCHH: ch=%d, USBH_LLD_HALTREASON_NONE", hcm - host->channels);
		return;
	}

	if (reason == USBH_LLD_HALTREASON_XFRC) {
		osalDbgCheck(ep->in);
		switch (ep->type) {
		case USBH_EPTYPE_CTRL:
			_complete_control(host, hcm, ep, urb, hctsiz);
			break;
		case USBH_EPTYPE_BULK:
		case USBH_EPTYPE_INT:
			_complete_bulk_int(host, hcm, ep, urb, hctsiz);
			break;
		case USBH_EPTYPE_ISO:
			_complete_iso(host, hcm, ep, urb, hctsiz);
			break;
		}
	} else {
		_release_channel(host, hcm);
		_save_dt_mask(ep, hctsiz);
		bool done = _update_urb(ep, hctsiz, urb, FALSE);

		switch (reason) {
		case USBH_LLD_HALTREASON_NAK:
			if ((ep->type == USBH_EPTYPE_INT) && ep->in) {
				_transfer_completedI(ep, urb, USBH_URBSTATUS_TIMEOUT);
			} else {
				ep->xfer.error_count = 0;
				_move_to_pending_queue(ep);
			}
			break;

		case USBH_LLD_HALTREASON_STALL:
			if (ep->type == USBH_EPTYPE_CTRL) {
				if (ep->xfer.u.ctrl_phase == USBH_LLD_CTRLPHASE_SETUP) {
					uerrf("\t%s: Faulty device: STALLed SETUP phase", ep->name);
				}
			} else {
				ep->status = USBH_EPSTATUS_HALTED;
			}
			_transfer_completedI(ep, urb, USBH_URBSTATUS_STALL);
			break;

		case USBH_LLD_HALTREASON_ERROR:
			if ((ep->type == USBH_EPTYPE_ISO) || done || (ep->xfer.error_count >= 3)) {
				_transfer_completedI(ep, urb, USBH_URBSTATUS_ERROR);
			} else {
				uerrf("\t%s: err=%d, done=%d, retry", ep->name, ep->xfer.error_count, done);
				_move_to_pending_queue(ep);
			}
			break;

		case USBH_LLD_HALTREASON_ABORT:
			uwarnf("\t%s: Abort", ep->name);
			_transfer_completedI(ep, urb, urb->status);
			break;

		default:
			osalDbgCheck(0);
			break;
		}

		if (usbhEPIsPeriodic(ep)) {
			_try_commit_p(host, FALSE);
		} else {
			_try_commit_np(host);
		}
	}
}

static void _hcint_n_int(USBHDriver *host, uint8_t chn) {

	stm32_hc_management_t *const hcm = &host->channels[chn];
	stm32_otg_host_chn_t *const hc = hcm->hc;

	uint32_t hcint = hc->HCINT;
	hcint &= hc->HCINTMSK;
	hc->HCINT = hcint;

	osalDbgCheck((hcint & HCINTMSK_AHBERRM) == 0);
	osalDbgCheck(hcm->ep);

	if (hcint & HCINTMSK_STALLM)
		_stall_int(host, hcm, hc);
	if (hcint & HCINTMSK_NAKM)
		_nak_int(host, hcm, hc);
	if (hcint & HCINTMSK_ACKM)
		_ack_int(host, hcm, hc);
	if (hcint & HCINTMSK_TRERRM)
		_trerr_int(host, hcm, hc);
	if (hcint & HCINTMSK_BBERRM)
		_bberr_int(host, hcm, hc);
	if (hcint & HCINTMSK_FRMORM)
		_frmor_int(host, hcm, hc);
	if (hcint & HCINTMSK_DTERRM)
		_dterr_int(host, hcm, hc);
	if (hcint & HCINTMSK_XFRCM)
		_xfrc_int(host, hcm, hc);
	if (hcint & HCINTMSK_CHHM)
		_chh_int(host, hcm, hc);
}

static inline void _hcint_int(USBHDriver *host) {
	uint32_t haint;

	haint = host->otg->HAINT;
	haint &= host->otg->HAINTMSK;

#if USBH_DEBUG_ENABLE && USBH_LLD_DEBUG_ENABLE_ERRORS
	if (!haint) {
		uint32_t a, b;
		a = host->otg->HAINT;
		b = host->otg->HAINTMSK;
		uerrf("HAINT=%08x, HAINTMSK=%08x", a, b);
		return;
	}
#endif

#if 1	//channel lookup loop
	uint8_t i;
	for (i = 0; haint && (i < host->channels_number); i++) {
		if (haint & (1 << i)) {
			_hcint_n_int(host, i);
			haint &= ~(1 << i);
		}
	}
#else	//faster calculation, with __CLZ (count leading zeroes)
	while (haint) {
		uint8_t chn = (uint8_t)(31 - __CLZ(haint));
		osalDbgAssert(chn < host->channels_number, "what?");
		haint &= ~host->channels[chn].haintmsk;
		_hcint_n_int(host, chn);
	}
#endif
}


/*===========================================================================*/
/* Host interrupts.                                                          */
/*===========================================================================*/
static inline void _sof_int(USBHDriver *host) {

	/* this is part of the workaround to the LS bug in the OTG core */
#undef HPRT_PLSTS_MASK
#define HPRT_PLSTS_MASK (3U<<10)
	if (host->check_ls_activity) {
		stm32_otg_t *const otg = host->otg;
		uint16_t remaining = otg->HFNUM >> 16;
		if (remaining < 5975) {
			uwarnf("LS: ISR called too late (time=%d)", 6000 - remaining);
			return;
		}
		/* 15us loop during which we check if the core generates an actual keep-alive
		 * (or activity other than idle) on the DP/DM lines. After 15us, we abort
		 * the loop and wait for the next SOF. If no activity is detected, the upper
		 * layer will time-out waiting for the reset to happen, and the port will remain
		 * enabled (though in a dumb state). This will be detected on the next port reset
		 * request and the OTG core will be reset. */
		for (;;) {
			uint32_t line_status = otg->HPRT & HPRT_PLSTS_MASK;
			remaining = otg->HFNUM >> 16;
			if (!(otg->HPRT & HPRT_PENA)) {
				uwarn("LS: Port disabled");
				return;
			}
			if (line_status != HPRT_PLSTS_DM) {
				/* success; report that the port is enabled */
				uinfof("LS: activity detected, line=%d, time=%d", line_status >> 10,  6000 - remaining);
				host->check_ls_activity = FALSE;
				otg->GINTMSK = (otg->GINTMSK & ~GINTMSK_SOFM) | (GINTMSK_HCM | GINTMSK_RXFLVLM);
				host->rootport.lld_status |= USBH_PORTSTATUS_ENABLE;
				host->rootport.lld_c_status |= USBH_PORTSTATUS_C_ENABLE;
				return;
			}
			if (remaining < 5910) {
				udbg("LS: No activity detected");
				return;
			}
		}
	}

	/* real SOF interrupt */
	udbg("SOF");
	_try_commit_p(host, TRUE);
}

static inline void _rxflvl_int(USBHDriver *host) {

	stm32_otg_t *const otg = host->otg;

	otg->GINTMSK &= ~GINTMSK_RXFLVLM;
	while (otg->GINTSTS & GINTSTS_RXFLVL) {
		uint32_t grxstsp = otg->GRXSTSP;
		osalDbgCheck((grxstsp & GRXSTSP_CHNUM_MASK) < host->channels_number);
		stm32_hc_management_t *const hcm = &host->channels[grxstsp & GRXSTSP_CHNUM_MASK];
		uint32_t hctsiz = hcm->hc->HCTSIZ;

		if ((grxstsp & GRXSTSP_PKTSTS_MASK) == GRXSTSP_PKTSTS(2)) {
			/* 0010: IN data packet received */
			usbh_ep_t *const ep = hcm->ep;
			osalDbgCheck(ep);

			/* restart the channel ASAP */
			if (hctsiz & HCTSIZ_PKTCNT_MASK) {
#if CH_DBG_ENABLE_CHECKS
				if (usbhEPIsPeriodic(ep)) {
					osalDbgCheck(host->otg->HPTXSTS & HPTXSTS_PTXQSAV_MASK);
				} else {
					osalDbgCheck(host->otg->HNPTXSTS & HPTXSTS_PTXQSAV_MASK);
				}
#endif
				hcm->hc->HCCHAR |= HCCHAR_CHENA;
			}

			udbgf("\t%s: RXFLVL rx=%dB, rem=%dB (%dpkts)",
					ep->name,
					(grxstsp & GRXSTSP_BCNT_MASK) >> 4,
					(hctsiz & HCTSIZ_XFRSIZ_MASK),
					(hctsiz & HCTSIZ_PKTCNT_MASK) >> 19);

			/* Read */
			uint32_t *dest = (uint32_t *)ep->xfer.buf;
			volatile uint32_t *const src = hcm->fifo;

			uint32_t bcnt = (grxstsp & GRXSTSP_BCNT_MASK) >> GRXSTSP_BCNT_OFF;
			osalDbgCheck(bcnt + ep->xfer.partial <= ep->xfer.len);

			//TODO: optimize this
			uint32_t words = bcnt / 4;
			uint8_t bytes = bcnt & 3;
			while (words--) {
				*dest++ = *src;
			}
			if (bytes) {
				uint32_t r = *src;
				uint8_t *bsrc = (uint8_t *)&r;
				uint8_t *bdest = (uint8_t *)dest;
				do {
					*bdest++ = *bsrc++;
				} while (--bytes);
			}

			ep->xfer.buf += bcnt;
			ep->xfer.partial += bcnt;

#if 0 //STM32_USBH_CHANNELS_NP > 1
			/* check bug */
			if (hctsiz & HCTSIZ_PKTCNT_MASK) {
				uint32_t pkt = (hctsiz & HCTSIZ_PKTCNT_MASK) >> 19;
				uint32_t siz = (hctsiz & HCTSIZ_XFRSIZ_MASK);
				if (pkt * ep->wMaxPacketSize != siz) {
					uerrf("\t%s: whatttt???", ep->name);
				}
			}
#endif

#if USBH_DEBUG_ENABLE && USBH_LLD_DEBUG_ENABLE_ERRORS
		} else {
			/* 0011: IN transfer completed (triggers an interrupt)
			 * 0101: Data toggle error (triggers an interrupt)
			 * 0111: Channel halted (triggers an interrupt)
			 */
			switch (grxstsp & GRXSTSP_PKTSTS_MASK) {
			case GRXSTSP_PKTSTS(3):
			case GRXSTSP_PKTSTS(5):
			case GRXSTSP_PKTSTS(7):
				break;
			default:
				uerrf("\tRXFLVL: ch=%d, UNK=%d", grxstsp & GRXSTSP_CHNUM_MASK, (grxstsp & GRXSTSP_PKTSTS_MASK) >> 17);
				break;
			}
#endif
		}
	}
	otg->GINTMSK |= GINTMSK_RXFLVLM;
}

static inline void _nptxfe_int(USBHDriver *host) {
	uint32_t rem;
	stm32_otg_t *const otg = host->otg;

	rem = _write_packet(&host->ep_active_lists[USBH_EPTYPE_CTRL],
			otg->HNPTXSTS & HPTXSTS_PTXFSAVL_MASK);

	rem += _write_packet(&host->ep_active_lists[USBH_EPTYPE_BULK],
			otg->HNPTXSTS & HPTXSTS_PTXFSAVL_MASK);

	if (!rem)
		otg->GINTMSK &= ~GINTMSK_NPTXFEM;

}

static inline void _ptxfe_int(USBHDriver *host) {
	uint32_t rem;
	stm32_otg_t *const otg = host->otg;

	rem = _write_packet(&host->ep_active_lists[USBH_EPTYPE_ISO],
			otg->HPTXSTS & HPTXSTS_PTXFSAVL_MASK);

	rem += _write_packet(&host->ep_active_lists[USBH_EPTYPE_INT],
			otg->HPTXSTS & HPTXSTS_PTXFSAVL_MASK);

	if (!rem)
		otg->GINTMSK &= ~GINTMSK_PTXFEM;
}

static void _disable(USBHDriver *host) {
	host->rootport.lld_status &= ~(USBH_PORTSTATUS_CONNECTION | USBH_PORTSTATUS_ENABLE);
	host->rootport.lld_c_status |= USBH_PORTSTATUS_C_CONNECTION | USBH_PORTSTATUS_C_ENABLE;

	_purge_active(host);
	_purge_pending(host);

	host->otg->GINTMSK &= ~(GINTMSK_HCM | GINTMSK_RXFLVLM);
}

static inline void _discint_int(USBHDriver *host) {
	uinfo("DISCINT: Port disconnection detected");
	_disable(host);
}

static inline void _hprtint_int(USBHDriver *host) {
	stm32_otg_t *const otg = host->otg;
	uint32_t hprt = otg->HPRT;

	/* note: writing PENA = 1 actually disables the port */
	uint32_t hprt_clr = hprt & ~(HPRT_PENA | HPRT_PCDET | HPRT_PENCHNG | HPRT_POCCHNG);

	if (hprt & HPRT_PCDET) {
		hprt_clr |= HPRT_PCDET;
		if (hprt & HPRT_PCSTS) {
			uinfo("\tHPRT: Port connection detected");
			host->rootport.lld_status |= USBH_PORTSTATUS_CONNECTION;
			host->rootport.lld_c_status |= USBH_PORTSTATUS_C_CONNECTION;
		}
	}

	if (hprt & HPRT_PENCHNG) {
		hprt_clr |= HPRT_PENCHNG;
		if (hprt & HPRT_PENA) {
			uinfo("\tHPRT: Port enabled");
			host->rootport.lld_status &= ~(USBH_PORTSTATUS_HIGH_SPEED | USBH_PORTSTATUS_LOW_SPEED);

			/* configure FIFOs */
#define HNPTXFSIZ						DIEPTXF0
#if STM32_USBH_USE_OTG1
#if STM32_USBH_USE_OTG2
			if (&USBHD1 == host)
#endif
			{
				otg->GRXFSIZ = GRXFSIZ_RXFD(STM32_OTG1_RXFIFO_SIZE / 4);
				otg->HNPTXFSIZ = HPTXFSIZ_PTXSA(STM32_OTG1_RXFIFO_SIZE / 4) | HPTXFSIZ_PTXFD(STM32_OTG1_NPTXFIFO_SIZE / 4);
				otg->HPTXFSIZ = HPTXFSIZ_PTXSA((STM32_OTG1_RXFIFO_SIZE / 4) + (STM32_OTG1_NPTXFIFO_SIZE / 4)) | HPTXFSIZ_PTXFD(STM32_OTG1_PTXFIFO_SIZE / 4);
			}
#endif
#if STM32_USBH_USE_OTG2
#if STM32_USBH_USE_OTG1
			if (&USBHD2 == host)
#endif
			{
				otg->GRXFSIZ = GRXFSIZ_RXFD(STM32_OTG2_RXFIFO_SIZE / 4);
				otg->HNPTXFSIZ = HPTXFSIZ_PTXSA(STM32_OTG2_RXFIFO_SIZE / 4) | HPTXFSIZ_PTXFD(STM32_OTG2_NPTXFIFO_SIZE / 4);
				otg->HPTXFSIZ = HPTXFSIZ_PTXSA((STM32_OTG2_RXFIFO_SIZE / 4) + (STM32_OTG2_NPTXFIFO_SIZE / 4)) | HPTXFSIZ_PTXFD(STM32_OTG2_PTXFIFO_SIZE / 4);
			}
#endif
#undef HNPTXFSIZ

			/* Make sure the FIFOs are flushed. */
		    otg_txfifo_flush(host, 0x10);
		    otg_rxfifo_flush(host);

		    /* Clear all pending HC Interrupts */
			uint8_t i;
			for (i = 0; i < host->channels_number; i++) {
				otg->hc[i].HCINTMSK = 0;
				otg->hc[i].HCINT = 0xFFFFFFFF;
			}

			/* configure speed */
			if ((hprt & HPRT_PSPD_MASK) == HPRT_PSPD_LS) {
				host->rootport.lld_status |= USBH_PORTSTATUS_LOW_SPEED;
				otg->HFIR = 6000;
				otg->HCFG = (otg->HCFG & ~HCFG_FSLSPCS_MASK) | HCFG_FSLSPCS_6;

				/* Low speed devices connected to the STM32's internal transceiver sometimes
				 * don't behave correctly. Although HPRT reports a port enable, really
				 * no traffic is generated, and the core is non-functional. To avoid
				 * this we won't report the port enable until we are sure that the
				 * port is working. */
				host->check_ls_activity = TRUE;
				otg->GINTMSK |= GINTMSK_SOFM;
			} else {
				otg->HFIR = 48000;
				otg->HCFG = (otg->HCFG & ~HCFG_FSLSPCS_MASK) | HCFG_FSLSPCS_48;
				host->check_ls_activity = FALSE;

				/* enable channel and rx interrupts */
				otg->GINTMSK |= GINTMSK_HCM | GINTMSK_RXFLVLM;
				host->rootport.lld_status |= USBH_PORTSTATUS_ENABLE;
				host->rootport.lld_c_status |= USBH_PORTSTATUS_C_ENABLE;
			}
		} else {
			if (hprt & HPRT_PCSTS) {
				if (hprt & HPRT_POCA) {
					uerr("\tHPRT: Port disabled due to overcurrent");
				} else {
					uerr("\tHPRT: Port disabled due to port babble");
				}
			} else {
				uerr("\tHPRT: Port disabled due to disconnect");
			}
			_disable(host);
		}
	}

	if (hprt & HPRT_POCCHNG) {
		hprt_clr |= HPRT_POCCHNG;
		if (hprt & HPRT_POCA) {
			uerr("\tHPRT: Overcurrent");
			host->rootport.lld_status |= USBH_PORTSTATUS_OVERCURRENT;
		} else {
			udbg("\tHPRT: Clear overcurrent");
			host->rootport.lld_status &= ~USBH_PORTSTATUS_OVERCURRENT;
		}
		host->rootport.lld_c_status |= USBH_PORTSTATUS_C_OVERCURRENT;
	}

	otg->HPRT = hprt_clr;
}

static void usb_lld_serve_interrupt(USBHDriver *host) {
	osalDbgCheck(host && (host->status != USBH_STATUS_STOPPED));

	stm32_otg_t *const otg = host->otg;
	uint32_t gintsts = otg->GINTSTS;

	/* check host mode */
	if (!(gintsts & GINTSTS_CMOD)) {
		uerr("Device mode");
		otg->GINTSTS = gintsts;
		return;
	}

	/* check mismatch */
	osalDbgAssert((gintsts & GINTSTS_MMIS) == 0, "mode mismatch");

	gintsts &= otg->GINTMSK;
	if (!gintsts) {
#if USBH_DEBUG_ENABLE && USBH_DEBUG_ENABLE_WARNINGS
		uint32_t a, b;
		a = otg->GINTSTS;
		b = otg->GINTMSK;
		uwarnf("Masked bits caused an ISR: GINTSTS=%08x, GINTMSK=%08x (unhandled bits=%08x)", a, b, a & ~b);
#endif
		return;
	}

	otg->GINTSTS = gintsts;

	if (gintsts & GINTSTS_SOF)
		_sof_int(host);
	if (gintsts & GINTSTS_RXFLVL)
		_rxflvl_int(host);
	if (gintsts & GINTSTS_HPRTINT)
		_hprtint_int(host);
	if (gintsts & GINTSTS_DISCINT)
		_discint_int(host);
	if (gintsts & GINTSTS_HCINT)
		_hcint_int(host);
	if (gintsts & GINTSTS_NPTXFE)
		_nptxfe_int(host);
	if (gintsts & GINTSTS_PTXFE)
		_ptxfe_int(host);
	if (gintsts & GINTSTS_IPXFR) {
		uerr("IPXFRM");
	}
}


/*===========================================================================*/
/* Interrupt handlers.                                                       */
/*===========================================================================*/

#if STM32_USBH_USE_OTG1
OSAL_IRQ_HANDLER(STM32_OTG1_HANDLER) {
	OSAL_IRQ_PROLOGUE();
	osalSysLockFromISR();
	usb_lld_serve_interrupt(&USBHD1);
	osalSysUnlockFromISR();
	OSAL_IRQ_EPILOGUE();
}
#endif

#if STM32_USBH_USE_OTG2
OSAL_IRQ_HANDLER(STM32_OTG2_HANDLER) {
	OSAL_IRQ_PROLOGUE();
	osalSysLockFromISR();
	usb_lld_serve_interrupt(&USBHD2);
	osalSysUnlockFromISR();
	OSAL_IRQ_EPILOGUE();
}
#endif


/*===========================================================================*/
/* Initialization functions.                                                 */
/*===========================================================================*/
static void otg_core_reset(USBHDriver *usbp) {
  stm32_otg_t *const otgp = usbp->otg;

  /* Wait AHB idle condition.*/
  while ((otgp->GRSTCTL & GRSTCTL_AHBIDL) == 0)
	;

  osalSysPolledDelayX(64);

  /* Core reset and delay of at least 3 PHY cycles.*/
  otgp->GRSTCTL = GRSTCTL_CSRST;
  while ((otgp->GRSTCTL & GRSTCTL_CSRST) != 0)
	;

  osalSysPolledDelayX(24);

  /* Wait AHB idle condition.*/
  while ((otgp->GRSTCTL & GRSTCTL_AHBIDL) == 0)
	;
}

static void otg_rxfifo_flush(USBHDriver *usbp) {
  stm32_otg_t *const otgp = usbp->otg;

  otgp->GRSTCTL = GRSTCTL_RXFFLSH;
  while ((otgp->GRSTCTL & GRSTCTL_RXFFLSH) != 0)
	;
  /* Wait for 3 PHY Clocks.*/
  osalSysPolledDelayX(24);
}

static void otg_txfifo_flush(USBHDriver *usbp, uint32_t fifo) {
  stm32_otg_t *const otgp = usbp->otg;

  otgp->GRSTCTL = GRSTCTL_TXFNUM(fifo) | GRSTCTL_TXFFLSH;
  while ((otgp->GRSTCTL & GRSTCTL_TXFFLSH) != 0)
	;
  /* Wait for 3 PHY Clocks.*/
  osalSysPolledDelayX(24);
}

static void _init(USBHDriver *host) {
	int i;

	usbhObjectInit(host);

#if STM32_USBH_USE_OTG1
#if STM32_USBH_USE_OTG2
	if (&USBHD1 == host)
#endif
	{
		host->otg = OTG_FS;
		host->channels_number = STM32_OTG1_CHANNELS_NUMBER;
	}
#endif

#if STM32_USBH_USE_OTG2
#if STM32_USBH_USE_OTG1
	if (&USBHD2 == host)
#endif
	{
		host->otg = OTG_HS;
		host->channels_number = STM32_OTG2_CHANNELS_NUMBER;
	}
#endif
	INIT_LIST_HEAD(&host->ch_free[0]);
	INIT_LIST_HEAD(&host->ch_free[1]);
	for (i = 0; i < host->channels_number; i++) {
		host->channels[i].haintmsk = 1 << i;
		host->channels[i].hc = &host->otg->hc[i];
		host->channels[i].fifo = host->otg->FIFO[i];
		if (i < STM32_USBH_CHANNELS_NP) {
			list_add_tail(&host->channels[i].node, &host->ch_free[1]);
		} else {
			list_add_tail(&host->channels[i].node, &host->ch_free[0]);
		}
	}
	for (i = 0; i < 4; i++) {
		INIT_LIST_HEAD(&host->ep_active_lists[i]);
		INIT_LIST_HEAD(&host->ep_pending_lists[i]);
	}
}

void usbh_lld_init(void) {
#if STM32_USBH_USE_OTG1
	_init(&USBHD1);
#endif
#if STM32_USBH_USE_OTG2
	_init(&USBHD2);
#endif
}

static void _usbh_start(USBHDriver *usbh) {
	stm32_otg_t *const otgp = usbh->otg;

	/* Clock activation.*/
#if STM32_USBH_USE_OTG1
#if STM32_USBH_USE_OTG2
	if (&USBHD1 == usbh)
#endif
	{
		/* OTG FS clock enable and reset.*/
		rccEnableOTG_FS(FALSE);
		rccResetOTG_FS();

		otgp->GINTMSK = 0;

		/* Enables IRQ vector.*/
		nvicEnableVector(STM32_OTG1_NUMBER, STM32_USB_OTG1_IRQ_PRIORITY);
	}
#endif

#if STM32_USBH_USE_OTG2
#if STM32_USBH_USE_OTG1
	if (&USBHD2 == usbh)
#endif
	{
		/* OTG HS clock enable and reset.*/
		rccEnableOTG_HS(FALSE); // Disable HS clock when cpu is in sleep mode
		rccDisableOTG_HSULPI();
		rccResetOTG_HS();

		otgp->GINTMSK = 0;

		/* Enables IRQ vector.*/
		nvicEnableVector(STM32_OTG2_NUMBER, STM32_USB_OTG2_IRQ_PRIORITY);
	}
#endif

	otgp->GUSBCFG = GUSBCFG_PHYSEL | GUSBCFG_TRDT(5);

	otg_core_reset(usbh);

	otgp->GCCFG = GCCFG_PWRDWN;

	/* Forced host mode. */
	otgp->GUSBCFG = GUSBCFG_FHMOD | GUSBCFG_PHYSEL | GUSBCFG_TRDT(5);

	/* PHY enabled.*/
	otgp->PCGCCTL = 0;

	/* Internal FS PHY activation.*/
#if STM32_OTG_STEPPING == 1
#if defined(BOARD_OTG_NOVBUSSENS)
	otgp->GCCFG = GCCFG_NOVBUSSENS | GCCFG_PWRDWN;
#else
	otgp->GCCFG = GCCFG_PWRDWN;
#endif
#elif STM32_OTG_STEPPING == 2
#if defined(BOARD_OTG_NOVBUSSENS)
	otgp->GCCFG = GCCFG_PWRDWN;
#else
	otgp->GCCFG = (GCCFG_VBDEN | GCCFG_PWRDWN);
#endif

#endif
	/* 48MHz 1.1 PHY.*/
	otgp->HCFG = HCFG_FSLSS | HCFG_FSLSPCS_48;

	/* Interrupts on FIFOs half empty.*/
	otgp->GAHBCFG = 0;

	otgp->GOTGINT = 0xFFFFFFFF;

	otgp->HPRT |= HPRT_PPWR;

	otg_txfifo_flush(usbh, 0x10);
	otg_rxfifo_flush(usbh);

	otgp->GINTSTS = 0xffffffff;
	otgp->GINTMSK = GINTMSK_DISCM | GINTMSK_HPRTM | GINTMSK_MMISM;

	usbh->rootport.lld_status = USBH_PORTSTATUS_POWER;
	usbh->rootport.lld_c_status = 0;

	/* Global interrupts enable.*/
	otgp->GAHBCFG |= GAHBCFG_GINTMSK;
}

void usbh_lld_start(USBHDriver *usbh) {
	if (usbh->status != USBH_STATUS_STOPPED) return;
	_usbh_start(usbh);
}

/*===========================================================================*/
/* Root Hub request handler.                                                 */
/*===========================================================================*/
usbh_urbstatus_t usbh_lld_root_hub_request(USBHDriver *usbh, uint8_t bmRequestType, uint8_t bRequest,
		uint16_t wvalue, uint16_t windex, uint16_t wlength, uint8_t *buf) {

	uint16_t typereq = (bmRequestType << 8) | bRequest;

	switch (typereq) {
	case ClearHubFeature:
		switch (wvalue) {
		case USBH_HUB_FEAT_C_HUB_LOCAL_POWER:
		case USBH_HUB_FEAT_C_HUB_OVER_CURRENT:
			break;
		default:
			osalDbgAssert(0, "invalid wvalue");
		}
		break;

	case ClearPortFeature:
		osalDbgAssert(windex == 1, "invalid windex");

		osalSysLock();
		switch (wvalue) {
		case USBH_PORT_FEAT_ENABLE:
		case USBH_PORT_FEAT_SUSPEND:
		case USBH_PORT_FEAT_POWER:
			osalDbgAssert(0, "unimplemented");	/* TODO */
			break;

		case USBH_PORT_FEAT_INDICATOR:
			osalDbgAssert(0, "unsupported");
			break;

		case USBH_PORT_FEAT_C_CONNECTION:
			usbh->rootport.lld_c_status &= ~USBH_PORTSTATUS_C_CONNECTION;
			break;

		case USBH_PORT_FEAT_C_RESET:
			usbh->rootport.lld_c_status &= ~USBH_PORTSTATUS_C_RESET;
			break;

		case USBH_PORT_FEAT_C_ENABLE:
			usbh->rootport.lld_c_status &= ~USBH_PORTSTATUS_C_ENABLE;
			break;

		case USBH_PORT_FEAT_C_SUSPEND:
			usbh->rootport.lld_c_status &= ~USBH_PORTSTATUS_C_SUSPEND;
			break;

		case USBH_PORT_FEAT_C_OVERCURRENT:
			usbh->rootport.lld_c_status &= ~USBH_PORTSTATUS_C_OVERCURRENT;
			break;

		default:
			osalDbgAssert(0, "invalid wvalue");
			break;
		}
		osalSysUnlock();
		break;

	case GetHubDescriptor:
		osalDbgAssert(0, "unsupported");
		break;

	case GetHubStatus:
		osalDbgCheck(wlength >= 4);
		*(uint32_t *)buf = 0;
		break;

	case GetPortStatus:
		osalDbgAssert(windex == 1, "invalid windex");
		osalDbgCheck(wlength >= 4);
		osalSysLock();
		*(uint32_t *)buf = usbh->rootport.lld_status | (usbh->rootport.lld_c_status << 16);
		osalSysUnlock();
		break;

	case SetHubFeature:
		osalDbgAssert(0, "unsupported");
		break;

	case SetPortFeature:
		osalDbgAssert(windex == 1, "invalid windex");

		switch (wvalue) {
		case USBH_PORT_FEAT_TEST:
		case USBH_PORT_FEAT_SUSPEND:
		case USBH_PORT_FEAT_POWER:
			osalDbgAssert(0, "unimplemented");	/* TODO */
			break;

		case USBH_PORT_FEAT_RESET: {
			osalSysLock();
			stm32_otg_t *const otg = usbh->otg;
			uint32_t hprt;
			otg->PCGCCTL = 0;
			hprt = otg->HPRT;
			if (hprt & HPRT_PENA) {
				/* This can occur when the OTG core doesn't generate traffic
				 * despite reporting a successful por enable. */
				uerr("Detected enabled port; resetting OTG core");
				otg->GAHBCFG = 0;
				osalThreadSleepS(OSAL_MS2I(20));
				_usbh_start(usbh);				/* this effectively resets the core */
				osalThreadSleepS(OSAL_MS2I(100));	/* during this delay, the core generates connect ISR */
				uinfo("OTG reset ended");
				if (otg->HPRT & HPRT_PCSTS) {
					/* if the device is still connected, don't report a C_CONNECTION flag, which would cause
					 * the upper layer to abort enumeration */
					uinfo("Clear connection change flag");
					usbh->rootport.lld_c_status &= ~USBH_PORTSTATUS_C_CONNECTION;
				}
			}
			/* note: writing PENA = 1 actually disables the port */
			hprt &= ~(HPRT_PSUSP | HPRT_PENA | HPRT_PCDET | HPRT_PENCHNG | HPRT_POCCHNG);
			while ((otg->GRSTCTL & GRSTCTL_AHBIDL) == 0);
			otg->HPRT = hprt | HPRT_PRST;
			osalThreadSleepS(OSAL_MS2I(15));
			otg->HPRT = hprt;
			osalThreadSleepS(OSAL_MS2I(10));
			usbh->rootport.lld_c_status |= USBH_PORTSTATUS_C_RESET;
			osalSysUnlock();
		} 	break;

		case USBH_PORT_FEAT_INDICATOR:
			osalDbgAssert(0, "unsupported");
			break;

		default:
			osalDbgAssert(0, "invalid wvalue");
			break;
		}
		break;

	default:
		osalDbgAssert(0, "invalid typereq");
		break;
	}

	return USBH_URBSTATUS_OK;
}

uint8_t usbh_lld_roothub_get_statuschange_bitmap(USBHDriver *usbh) {
	return usbh->rootport.lld_c_status ? (1 << 1) : 0;
}

#endif
