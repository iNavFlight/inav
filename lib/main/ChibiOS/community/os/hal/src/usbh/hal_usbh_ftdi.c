/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio
              Copyright (C) 2015 Diego Ismirlian, TISA, (dismirlian (at) google's mail)

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
#include "hal_usbh.h"

#if HAL_USBH_USE_FTDI

#if !HAL_USE_USBH
#error "USBHFTDI needs USBH"
#endif

#include <string.h>
#include "usbh/dev/ftdi.h"
#include "usbh/internal.h"

//#pragma GCC optimize("Og")


#if USBHFTDI_DEBUG_ENABLE_TRACE
#define udbgf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define udbg(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define udbgf(f, ...)  do {} while(0)
#define udbg(f, ...)   do {} while(0)
#endif

#if USBHFTDI_DEBUG_ENABLE_INFO
#define uinfof(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uinfo(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uinfof(f, ...)  do {} while(0)
#define uinfo(f, ...)   do {} while(0)
#endif

#if USBHFTDI_DEBUG_ENABLE_WARNINGS
#define uwarnf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uwarn(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uwarnf(f, ...)  do {} while(0)
#define uwarn(f, ...)   do {} while(0)
#endif

#if USBHFTDI_DEBUG_ENABLE_ERRORS
#define uerrf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uerr(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uerrf(f, ...)  do {} while(0)
#define uerr(f, ...)   do {} while(0)
#endif


/*===========================================================================*/
/* USB Class driver loader for FTDI								 		 	 */
/*===========================================================================*/
USBHFTDIDriver USBHFTDID[HAL_USBHFTDI_MAX_INSTANCES];

static usbh_baseclassdriver_t *_ftdi_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem);
static void _ftdi_unload(usbh_baseclassdriver_t *drv);

static const usbh_classdriver_vmt_t class_driver_vmt = {
	_ftdi_load,
	_ftdi_unload
};

const usbh_classdriverinfo_t usbhftdiClassDriverInfo = {
	0xff, 0xff, 0xff, "FTDI", &class_driver_vmt
};

static USBHFTDIPortDriver *_find_port(void) {
	uint8_t i;
	for (i = 0; i < HAL_USBHFTDI_MAX_PORTS; i++) {
		if (FTDIPD[i].ftdip == NULL)
			return &FTDIPD[i];
	}
	return NULL;
}

static usbh_baseclassdriver_t *_ftdi_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem) {
	int i;
	USBHFTDIDriver *ftdip;

	if (dev->devDesc.idVendor != 0x0403) {
		uerr("FTDI: Unrecognized VID");
		return NULL;
	}

	switch (dev->devDesc.idProduct) {
	case 0x6001:
	case 0x6010:
	case 0x6011:
	case 0x6014:
	case 0x6015:
		break;
	default:
		uerr("FTDI: Unrecognized PID");
		return NULL;
	}

	if ((rem < descriptor[0]) || (descriptor[1] != USBH_DT_INTERFACE))
		return NULL;

	const usbh_interface_descriptor_t * const ifdesc = (const usbh_interface_descriptor_t * const)descriptor;
	if (ifdesc->bInterfaceNumber != 0) {
		uwarn("FTDI: Will allocate driver along with IF #0");
	}

	/* alloc driver */
	for (i = 0; i < HAL_USBHFTDI_MAX_INSTANCES; i++) {
		if (USBHFTDID[i].dev == NULL) {
			ftdip = &USBHFTDID[i];
			goto alloc_ok;
		}
	}

	uwarn("FTDI: Can't alloc driver");

	/* can't alloc */
	return NULL;

alloc_ok:
	/* initialize the driver's variables */
	ftdip->ports = 0;
	switch (dev->devDesc.bcdDevice) {
	case 0x200:		//AM
		uinfo("FTDI: Type A chip");
		ftdip->type = USBHFTDI_TYPE_A;
		break;
	case 0x400:		//BM
	case 0x500:		//2232C
	case 0x600:		//R
	case 0x1000:	//230X
		uinfo("FTDI: Type B chip");
		ftdip->type = USBHFTDI_TYPE_B;
		break;
	case 0x700:		//2232H;
	case 0x800:		//4232H;
	case 0x900:		//232H;
		uinfo("FTDI: Type H chip");
		ftdip->type = USBHFTDI_TYPE_H;
	default:
		uerr("FTDI: Unrecognized chip type");
		return NULL;
	}
	usbhEPSetName(&dev->ctrl, "FTD[CTRL]");

	/* parse the configuration descriptor */
	generic_iterator_t iep, icfg;
	if_iterator_t iif;
	cfg_iter_init(&icfg, dev->fullConfigurationDescriptor, dev->basicConfigDesc.wTotalLength);
	for (if_iter_init(&iif, &icfg); iif.valid; if_iter_next(&iif)) {
		const usbh_interface_descriptor_t *const ifdesc = if_get(&iif);
		uinfof("FTDI: Interface #%d", ifdesc->bInterfaceNumber);

		USBHFTDIPortDriver *const prt = _find_port();
		if (prt == NULL) {
			uwarn("\tCan't alloc port for this interface");
			break;
		}

		prt->ifnum = ifdesc->bInterfaceNumber;
		prt->epin.status = USBH_EPSTATUS_UNINITIALIZED;
		prt->epout.status = USBH_EPSTATUS_UNINITIALIZED;

		for (ep_iter_init(&iep, &iif); iep.valid; ep_iter_next(&iep)) {
			const usbh_endpoint_descriptor_t *const epdesc = ep_get(&iep);
			if ((epdesc->bEndpointAddress & 0x80) && (epdesc->bmAttributes == USBH_EPTYPE_BULK)) {
				uinfof("BULK IN endpoint found: bEndpointAddress=%02x", epdesc->bEndpointAddress);
				usbhEPObjectInit(&prt->epin, dev, epdesc);
				usbhEPSetName(&prt->epin, "FTD[BIN ]");
			} else if (((epdesc->bEndpointAddress & 0x80) == 0)
					&& (epdesc->bmAttributes == USBH_EPTYPE_BULK)) {
				uinfof("BULK OUT endpoint found: bEndpointAddress=%02x", epdesc->bEndpointAddress);
				usbhEPObjectInit(&prt->epout, dev, epdesc);
				usbhEPSetName(&prt->epout, "FTD[BOUT]");
			} else {
				uinfof("unsupported endpoint found: bEndpointAddress=%02x, bmAttributes=%02x",
						epdesc->bEndpointAddress, epdesc->bmAttributes);
			}
		}

		if ((prt->epin.status != USBH_EPSTATUS_CLOSED)
				|| (prt->epout.status != USBH_EPSTATUS_CLOSED)) {
			uwarn("\tCouldn't find endpoints; can't alloc port for this interface");
			continue;
		}

		/* link the new block driver to the list */
		prt->next = ftdip->ports;
		ftdip->ports = prt;
		prt->ftdip = ftdip;

		prt->state = USBHFTDIP_STATE_ACTIVE;
	}

	return (usbh_baseclassdriver_t *)ftdip;

}

static void _stop(USBHFTDIPortDriver *ftdipp);
static void _ftdi_unload(usbh_baseclassdriver_t *drv) {
	osalDbgCheck(drv != NULL);
	USBHFTDIDriver *const ftdip = (USBHFTDIDriver *)drv;
	USBHFTDIPortDriver *ftdipp = ftdip->ports;

	osalMutexLock(&ftdip->mtx);
	while (ftdipp) {
		_stop(ftdipp);
		ftdipp = ftdipp->next;
	}

	ftdipp = ftdip->ports;
	osalSysLock();
	while (ftdipp) {
		USBHFTDIPortDriver *next = ftdipp->next;
		usbhftdipObjectInit(ftdipp);
		ftdipp = next;
	}
	osalSysUnlock();
	osalMutexUnlock(&ftdip->mtx);
}


USBHFTDIPortDriver FTDIPD[HAL_USBHFTDI_MAX_PORTS];


#define FTDI_COMMAND_RESET		0
#define FTDI_RESET_ALL 			0
#define FTDI_RESET_PURGE_RX 	1
#define FTDI_RESET_PURGE_TX 	2

#define FTDI_COMMAND_SETFLOW    2

#define FTDI_COMMAND_SETBAUD    3

#define FTDI_COMMAND_SETDATA    4
#define FTDI_SETDATA_BREAK      (0x1 << 14)

#if 0
#define FTDI_COMMAND_MODEMCTRL  	1
#define FTDI_COMMAND_GETMODEMSTATUS       5 /* Retrieve current value of modem status register */
#define FTDI_COMMAND_SETEVENTCHAR         6 /* Set the event character */
#define FTDI_COMMAND_SETERRORCHAR         7 /* Set the error character */
#define FTDI_COMMAND_SETLATENCYTIMER      9 /* Set the latency timer */
#define FTDI_COMMAND_GETLATENCYTIMER      10 /* Get the latency timer */
#endif

/*
 * DATA FORMAT
 *
 * IN Endpoint
 *
 * The device reserves the first two bytes of data on this endpoint to contain
 * the current values of the modem and line status registers. In the absence of
 * data, the device generates a message consisting of these two status bytes
 * every 40 ms
 *
 * Byte 0: Modem Status
 *
 * Offset       Description
 * B0   Reserved - must be 1
 * B1   Reserved - must be 0
 * B2   Reserved - must be 0
 * B3   Reserved - must be 0
 * B4   Clear to Send (CTS)
 * B5   Data Set Ready (DSR)
 * B6   Ring Indicator (RI)
 * B7   Receive Line Signal Detect (RLSD)
 *
 * Byte 1: Line Status
 *
 * Offset       Description
 * B0   Data Ready (DR)
 * B1   Overrun Error (OE)
 * B2   Parity Error (PE)
 * B3   Framing Error (FE)
 * B4   Break Interrupt (BI)
 * B5   Transmitter Holding Register (THRE)
 * B6   Transmitter Empty (TEMT)
 * B7   Error in RCVR FIFO
 *
 */
#define FTDI_RS0_CTS    (1 << 4)
#define FTDI_RS0_DSR    (1 << 5)
#define FTDI_RS0_RI     (1 << 6)
#define FTDI_RS0_RLSD   (1 << 7)

#define FTDI_RS_DR      1
#define FTDI_RS_OE      (1<<1)
#define FTDI_RS_PE      (1<<2)
#define FTDI_RS_FE      (1<<3)
#define FTDI_RS_BI      (1<<4)
#define FTDI_RS_THRE    (1<<5)
#define FTDI_RS_TEMT    (1<<6)
#define FTDI_RS_FIFO    (1<<7)


static usbh_urbstatus_t _ftdi_port_control(USBHFTDIPortDriver *ftdipp,
		uint8_t bRequest, uint8_t wValue, uint8_t bHIndex, uint16_t wLength,
		uint8_t *buff) {

	static const uint8_t bmRequestType[] = {
		USBH_REQTYPE_VENDOR | USBH_REQTYPE_OUT | USBH_REQTYPE_DEVICE, //0 FTDI_COMMAND_RESET
		USBH_REQTYPE_VENDOR | USBH_REQTYPE_OUT | USBH_REQTYPE_DEVICE, //1 FTDI_COMMAND_MODEMCTRL
		USBH_REQTYPE_VENDOR | USBH_REQTYPE_OUT | USBH_REQTYPE_DEVICE, //2 FTDI_COMMAND_SETFLOW
		USBH_REQTYPE_VENDOR | USBH_REQTYPE_OUT | USBH_REQTYPE_DEVICE, //3 FTDI_COMMAND_SETBAUD
		USBH_REQTYPE_VENDOR | USBH_REQTYPE_OUT | USBH_REQTYPE_DEVICE, //4 FTDI_COMMAND_SETDATA
	};

	osalDbgCheck(bRequest < sizeof_array(bmRequestType));
	osalDbgCheck(bRequest != 1);

	const USBH_DEFINE_BUFFER(usbh_control_request_t, req) = {
			bmRequestType[bRequest],
			bRequest,
			wValue,
			(bHIndex << 8) | (ftdipp->ifnum + 1),
			wLength
	};

	return usbhControlRequestExtended(ftdipp->ftdip->dev, &req, buff, NULL, MS2ST(1000));
}

static uint32_t _get_divisor(uint32_t baud, usbhftdi_type_t type) {
	static const uint8_t divfrac[8] = {0, 3, 2, 4, 1, 5, 6, 7};
	uint32_t divisor;

	if (type == USBHFTDI_TYPE_A) {
		uint32_t divisor3 = ((48000000UL / 2) + baud / 2) / baud;
		uinfof("FTDI: desired=%dbps, real=%dbps", baud, (48000000UL / 2) / divisor3);
		if ((divisor3 & 0x7) == 7)
			divisor3++; /* round x.7/8 up to x+1 */

		divisor = divisor3 >> 3;
	    divisor3 &= 0x7;
		if (divisor3 == 1)
			divisor |= 0xc000;
		else if (divisor3 >= 4)
			divisor |= 0x4000;
		else if (divisor3 != 0)
			divisor |= 0x8000;
		else if (divisor == 1)
			divisor = 0;    /* special case for maximum baud rate */
	} else {
		if (type == USBHFTDI_TYPE_B) {
			divisor = ((48000000UL / 2) + baud / 2) / baud;
			uinfof("FTDI: desired=%dbps, real=%dbps", baud, (48000000UL / 2) / divisor);
		} else {
			/* hi-speed baud rate is 10-bit sampling instead of 16-bit */
			if (baud < 1200)
				baud = 1200;
			divisor = (120000000UL * 8 + baud * 5) / (baud * 10);
			uinfof("FTDI: desired=%dbps, real=%dbps", baud, (120000000UL * 8) / divisor / 10);
		}
		divisor = (divisor >> 3) | (divfrac[divisor & 0x7] << 14);

		/* Deal with special cases for highest baud rates. */
		if (divisor == 1)
			divisor = 0;
		else if (divisor == 0x4001)
			divisor = 1;

		if (type == USBHFTDI_TYPE_H)
			divisor |= 0x00020000;
	}
	return divisor;
}

static usbh_urbstatus_t _set_baudrate(USBHFTDIPortDriver *ftdipp, uint32_t baudrate) {
	uint32_t divisor = _get_divisor(baudrate, ftdipp->ftdip->type);
	uint16_t wValue = (uint16_t)divisor;
	uint16_t wIndex = (uint16_t)(divisor >> 16);
	if (ftdipp->ftdip->dev->basicConfigDesc.bNumInterfaces > 1)
		wIndex = (wIndex << 8) | (ftdipp->ifnum + 1);

	const USBH_DEFINE_BUFFER(usbh_control_request_t, req) = {
		USBH_REQTYPE_VENDOR | USBH_REQTYPE_OUT | USBH_REQTYPE_DEVICE,
		FTDI_COMMAND_SETBAUD,
		wValue,
		wIndex,
		0
	};
	return usbhControlRequestExtended(ftdipp->ftdip->dev, &req, NULL, NULL, MS2ST(1000));
}


static void _submitOutI(USBHFTDIPortDriver *ftdipp, uint32_t len) {
	udbgf("FTDI: Submit OUT %d", len);
	ftdipp->oq_urb.requestedLength = len;
	usbhURBObjectResetI(&ftdipp->oq_urb);
	usbhURBSubmitI(&ftdipp->oq_urb);
}

static void _out_cb(usbh_urb_t *urb) {
	USBHFTDIPortDriver *const ftdipp = (USBHFTDIPortDriver *)urb->userData;
	switch (urb->status) {
	case USBH_URBSTATUS_OK:
		ftdipp->oq_ptr = ftdipp->oq_buff;
		ftdipp->oq_counter = 64;
		chThdDequeueNextI(&ftdipp->oq_waiting, Q_OK);
		return;
	case USBH_URBSTATUS_DISCONNECTED:
		uwarn("FTDI: URB OUT disconnected");
		chThdDequeueNextI(&ftdipp->oq_waiting, Q_RESET);
		return;
	default:
		uerrf("FTDI: URB OUT status unexpected = %d", urb->status);
		break;
	}
	usbhURBObjectResetI(&ftdipp->oq_urb);
	usbhURBSubmitI(&ftdipp->oq_urb);
}

static size_t _write_timeout(USBHFTDIPortDriver *ftdipp, const uint8_t *bp,
		size_t n, systime_t timeout) {
	chDbgCheck(n > 0U);

	size_t w = 0;
	chSysLock();
	while (true) {
		if (ftdipp->state != USBHFTDIP_STATE_READY) {
			chSysUnlock();
			return w;
		}
		while (usbhURBIsBusy(&ftdipp->oq_urb)) {
			if (chThdEnqueueTimeoutS(&ftdipp->oq_waiting, timeout) != Q_OK) {
				chSysUnlock();
				return w;
			}
		}

		*ftdipp->oq_ptr++ = *bp++;
		if (--ftdipp->oq_counter == 0) {
			_submitOutI(ftdipp, 64);
			chSchRescheduleS();
		}
		chSysUnlock(); /* Gives a preemption chance in a controlled point.*/

		w++;
		if (--n == 0U)
			return w;

		chSysLock();
	}
}

static msg_t _put_timeout(USBHFTDIPortDriver *ftdipp, uint8_t b, systime_t timeout) {

	chSysLock();
	if (ftdipp->state != USBHFTDIP_STATE_READY) {
		chSysUnlock();
		return Q_RESET;
	}

	while (usbhURBIsBusy(&ftdipp->oq_urb)) {
		msg_t msg = chThdEnqueueTimeoutS(&ftdipp->oq_waiting, timeout);
		if (msg < Q_OK) {
			chSysUnlock();
			return msg;
		}
	}

	*ftdipp->oq_ptr++ = b;
	if (--ftdipp->oq_counter == 0) {
		_submitOutI(ftdipp, 64);
		chSchRescheduleS();
	}
	chSysUnlock();
	return Q_OK;
}

static size_t _write(USBHFTDIPortDriver *ftdipp, const uint8_t *bp, size_t n) {
	return _write_timeout(ftdipp, bp, n, TIME_INFINITE);
}

static msg_t _put(USBHFTDIPortDriver *ftdipp, uint8_t b) {
	return _put_timeout(ftdipp, b, TIME_INFINITE);
}

static void _submitInI(USBHFTDIPortDriver *ftdipp) {
	udbg("FTDI: Submit IN");
	usbhURBObjectResetI(&ftdipp->iq_urb);
	usbhURBSubmitI(&ftdipp->iq_urb);
}

static void _in_cb(usbh_urb_t *urb) {
	USBHFTDIPortDriver *const ftdipp = (USBHFTDIPortDriver *)urb->userData;
	switch (urb->status) {
	case USBH_URBSTATUS_OK:
		if (urb->actualLength < 2) {
			uwarnf("FTDI: URB IN actualLength = %d, < 2", urb->actualLength);
		} else if (urb->actualLength > 2) {
			udbgf("FTDI: URB IN data len=%d, status=%02x %02x",
					urb->actualLength - 2,
					((uint8_t *)urb->buff)[0],
					((uint8_t *)urb->buff)[1]);
			ftdipp->iq_ptr = ftdipp->iq_buff + 2;
			ftdipp->iq_counter = urb->actualLength - 2;
			chThdDequeueNextI(&ftdipp->iq_waiting, Q_OK);
			return;
		} else {
			udbgf("FTDI: URB IN no data, status=%02x %02x",
					((uint8_t *)urb->buff)[0],
					((uint8_t *)urb->buff)[1]);
			return;
		}
		break;
	case USBH_URBSTATUS_DISCONNECTED:
		uwarn("FTDI: URB IN disconnected");
		chThdDequeueNextI(&ftdipp->iq_waiting, Q_RESET);
		return;
	default:
		uerrf("FTDI: URB IN status unexpected = %d", urb->status);
		break;
	}
	_submitInI(ftdipp);
}

static size_t _read_timeout(USBHFTDIPortDriver *ftdipp, uint8_t *bp,
		size_t n, systime_t timeout) {
	size_t r = 0;

	chDbgCheck(n > 0U);

	chSysLock();
	while (true) {
		if (ftdipp->state != USBHFTDIP_STATE_READY) {
			chSysUnlock();
			return r;
		}
		while (ftdipp->iq_counter == 0) {
			if (!usbhURBIsBusy(&ftdipp->iq_urb))
				_submitInI(ftdipp);
			if (chThdEnqueueTimeoutS(&ftdipp->iq_waiting, timeout) != Q_OK) {
				chSysUnlock();
				return r;
			}
		}
		*bp++ = *ftdipp->iq_ptr++;
		if (--ftdipp->iq_counter == 0) {
			_submitInI(ftdipp);
			chSchRescheduleS();
		}
		chSysUnlock();

		r++;
		if (--n == 0U)
			return r;

		chSysLock();
	}
}

static msg_t _get_timeout(USBHFTDIPortDriver *ftdipp, systime_t timeout) {
	uint8_t b;

	chSysLock();
	if (ftdipp->state != USBHFTDIP_STATE_READY) {
		chSysUnlock();
		return Q_RESET;
	}
	while (ftdipp->iq_counter == 0) {
		if (!usbhURBIsBusy(&ftdipp->iq_urb))
			_submitInI(ftdipp);
		msg_t msg = chThdEnqueueTimeoutS(&ftdipp->iq_waiting, timeout);
		if (msg < Q_OK) {
			chSysUnlock();
			return msg;
		}
	}
	b = *ftdipp->iq_ptr++;
	if (--ftdipp->iq_counter == 0) {
		_submitInI(ftdipp);
		chSchRescheduleS();
	}
	chSysUnlock();

	return (msg_t)b;
}

static msg_t _get(USBHFTDIPortDriver *ftdipp) {
	return _get_timeout(ftdipp, TIME_INFINITE);
}

static size_t _read(USBHFTDIPortDriver *ftdipp, uint8_t *bp, size_t n) {
	return _read_timeout(ftdipp, bp, n, TIME_INFINITE);
}

static void _vt(void *p) {
	USBHFTDIPortDriver *const ftdipp = (USBHFTDIPortDriver *)p;
	chSysLockFromISR();
	uint32_t len = ftdipp->oq_ptr - ftdipp->oq_buff;
	if (len && !usbhURBIsBusy(&ftdipp->oq_urb)) {
		_submitOutI(ftdipp, len);
	}
	if ((ftdipp->iq_counter == 0) && !usbhURBIsBusy(&ftdipp->iq_urb)) {
		_submitInI(ftdipp);
	}
	chVTSetI(&ftdipp->vt, MS2ST(16), _vt, ftdipp);
	chSysUnlockFromISR();
}

static const struct FTDIPortDriverVMT async_channel_vmt = {
	(size_t (*)(void *, const uint8_t *, size_t))_write,
	(size_t (*)(void *, uint8_t *, size_t))_read,
	(msg_t (*)(void *, uint8_t))_put,
	(msg_t (*)(void *))_get,
	(msg_t (*)(void *, uint8_t, systime_t))_put_timeout,
	(msg_t (*)(void *, systime_t))_get_timeout,
	(size_t (*)(void *, const uint8_t *, size_t, systime_t))_write_timeout,
	(size_t (*)(void *, uint8_t *, size_t, systime_t))_read_timeout
};


static void _stop(USBHFTDIPortDriver *ftdipp) {
	osalSysLock();
	chVTResetI(&ftdipp->vt);
	usbhEPCloseS(&ftdipp->epin);
	usbhEPCloseS(&ftdipp->epout);
	chThdDequeueAllI(&ftdipp->iq_waiting, Q_RESET);
	chThdDequeueAllI(&ftdipp->oq_waiting, Q_RESET);
	osalOsRescheduleS();
	ftdipp->state = USBHFTDIP_STATE_ACTIVE;
	osalSysUnlock();
}

void usbhftdipStop(USBHFTDIPortDriver *ftdipp) {
	osalDbgCheck((ftdipp->state == USBHFTDIP_STATE_ACTIVE)
			|| (ftdipp->state == USBHFTDIP_STATE_READY));

	if (ftdipp->state == USBHFTDIP_STATE_ACTIVE) {
		return;
	}

	osalMutexLock(&ftdipp->ftdip->mtx);
	_stop(ftdipp);
	osalMutexUnlock(&ftdipp->ftdip->mtx);
}

void usbhftdipStart(USBHFTDIPortDriver *ftdipp, const USBHFTDIPortConfig *config) {
	static const USBHFTDIPortConfig default_config = {
		HAL_USBHFTDI_DEFAULT_SPEED,
		HAL_USBHFTDI_DEFAULT_FRAMING,
		HAL_USBHFTDI_DEFAULT_HANDSHAKE,
		HAL_USBHFTDI_DEFAULT_XON,
		HAL_USBHFTDI_DEFAULT_XOFF
	};

	osalDbgCheck((ftdipp->state == USBHFTDIP_STATE_ACTIVE)
			|| (ftdipp->state == USBHFTDIP_STATE_READY));

	if (ftdipp->state == USBHFTDIP_STATE_READY)
		return;

	osalMutexLock(&ftdipp->ftdip->mtx);
	if (config == NULL)
		config = &default_config;

	uint16_t wValue = 0;
	_ftdi_port_control(ftdipp, FTDI_COMMAND_RESET, FTDI_RESET_ALL, 0, 0, NULL);
	_set_baudrate(ftdipp, config->speed);
	_ftdi_port_control(ftdipp, FTDI_COMMAND_SETDATA, config->framing, 0, 0, NULL);
	if (config->handshake & USBHFTDI_HANDSHAKE_XON_XOFF)
		wValue = (config->xoff_character << 8) | config->xon_character;
	_ftdi_port_control(ftdipp, FTDI_COMMAND_SETFLOW, wValue, config->handshake, 0, NULL);

	usbhURBObjectInit(&ftdipp->oq_urb, &ftdipp->epout, _out_cb, ftdipp, ftdipp->oq_buff, 0);
	chThdQueueObjectInit(&ftdipp->oq_waiting);
	ftdipp->oq_counter = 64;
	ftdipp->oq_ptr = ftdipp->oq_buff;
	usbhEPOpen(&ftdipp->epout);

	usbhURBObjectInit(&ftdipp->iq_urb, &ftdipp->epin, _in_cb, ftdipp, ftdipp->iq_buff, 64);
	chThdQueueObjectInit(&ftdipp->iq_waiting);
	ftdipp->iq_counter = 0;
	ftdipp->iq_ptr = ftdipp->iq_buff;
	usbhEPOpen(&ftdipp->epin);
	osalSysLock();
	usbhURBSubmitI(&ftdipp->iq_urb);
	osalSysUnlock();

	chVTObjectInit(&ftdipp->vt);
	chVTSet(&ftdipp->vt, MS2ST(16), _vt, ftdipp);

	ftdipp->state = USBHFTDIP_STATE_READY;
	osalMutexUnlock(&ftdipp->ftdip->mtx);
}

void usbhftdiObjectInit(USBHFTDIDriver *ftdip) {
	osalDbgCheck(ftdip != NULL);
	memset(ftdip, 0, sizeof(*ftdip));
	ftdip->info = &usbhftdiClassDriverInfo;
	osalMutexObjectInit(&ftdip->mtx);
}

void usbhftdipObjectInit(USBHFTDIPortDriver *ftdipp) {
	osalDbgCheck(ftdipp != NULL);
	memset(ftdipp, 0, sizeof(*ftdipp));
	ftdipp->vmt = &async_channel_vmt;
	ftdipp->state = USBHFTDIP_STATE_STOP;
}

#endif
