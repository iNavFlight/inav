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

#if HAL_USBH_USE_UVC

#if !HAL_USE_USBH
#error "USBHUVC needs HAL_USE_USBH"
#endif

#if !HAL_USBH_USE_IAD
#error "USBHUVC needs HAL_USBH_USE_IAD"
#endif

#include <string.h>
#include "usbh/dev/uvc.h"
#include "usbh/internal.h"

#if USBHUVC_DEBUG_ENABLE_TRACE
#define udbgf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define udbg(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define udbgf(f, ...)  do {} while(0)
#define udbg(f, ...)   do {} while(0)
#endif

#if USBHUVC_DEBUG_ENABLE_INFO
#define uinfof(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uinfo(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uinfof(f, ...)  do {} while(0)
#define uinfo(f, ...)   do {} while(0)
#endif

#if USBHUVC_DEBUG_ENABLE_WARNINGS
#define uwarnf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uwarn(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uwarnf(f, ...)  do {} while(0)
#define uwarn(f, ...)   do {} while(0)
#endif

#if USBHUVC_DEBUG_ENABLE_ERRORS
#define uerrf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uerr(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uerrf(f, ...)  do {} while(0)
#define uerr(f, ...)   do {} while(0)
#endif


USBHUVCDriver USBHUVCD[HAL_USBHUVC_MAX_INSTANCES];

static void _uvc_init(void);
static usbh_baseclassdriver_t *_uvc_load(usbh_device_t *dev,
		const uint8_t *descriptor, uint16_t rem);
static void _uvc_unload(usbh_baseclassdriver_t *drv);

static const usbh_classdriver_vmt_t class_driver_vmt = {
	_uvc_init,
	_uvc_load,
	_uvc_unload
};
const usbh_classdriverinfo_t usbhuvcClassDriverInfo = {
	"UVC", &class_driver_vmt
};

static bool _request(USBHUVCDriver *uvcdp,
		uint8_t bRequest, uint8_t entity, uint8_t control,
		uint16_t wLength, uint8_t *data, uint8_t interf) {

	usbh_urbstatus_t res;

	if (bRequest & 0x80) {
		res = usbhControlRequest(uvcdp->dev,
				USBH_REQTYPE_CLASSIN(USBH_REQTYPE_RECIP_INTERFACE),
				bRequest,
				((control) << 8),
				(interf) | ((entity) << 8),
				wLength, data);
	} else {
		res = usbhControlRequest(uvcdp->dev,
				USBH_REQTYPE_CLASSOUT(USBH_REQTYPE_RECIP_INTERFACE),
				bRequest,
				((control) << 8),
				(interf) | ((entity) << 8),
				wLength, data);
	}

	if (res != USBH_URBSTATUS_OK)
		return HAL_FAILED;

	 return HAL_SUCCESS;
}

bool usbhuvcVCRequest(USBHUVCDriver *uvcdp,
		uint8_t bRequest, uint8_t entity, uint8_t control,
		uint16_t wLength, uint8_t *data) {
	return _request(uvcdp, bRequest, entity, control, wLength, data, if_get(&uvcdp->ivc)->bInterfaceNumber);
}

bool usbhuvcVSRequest(USBHUVCDriver *uvcdp,
		uint8_t bRequest, uint8_t control,
		uint16_t wLength, uint8_t *data) {

	return _request(uvcdp, bRequest, 0, control, wLength, data, if_get(&uvcdp->ivs)->bInterfaceNumber);
}

static bool _set_vs_alternate(USBHUVCDriver *uvcdp, uint16_t min_ep_size) {

	if (min_ep_size == 0) {
		uinfo("Selecting Alternate setting 0");
		return usbhStdReqSetInterface(uvcdp->dev, if_get(&uvcdp->ivs)->bInterfaceNumber, 0);
	}

	if_iterator_t iif = uvcdp->ivs;
	generic_iterator_t iep;
	const usbh_endpoint_descriptor_t *ep = NULL;
	uint8_t alt = 0;
	uint16_t sz = 0xffff;

	uinfof("Searching alternate setting with min_ep_size=%d", min_ep_size);

	for (; iif.valid; if_iter_next(&iif)) {
		const usbh_interface_descriptor_t *const ifdesc = if_get(&iif);

		if ((ifdesc->bInterfaceClass != UVC_CC_VIDEO)
				|| (ifdesc->bInterfaceSubClass != UVC_SC_VIDEOSTREAMING))
			continue;

		uinfof("\tScanning alternate setting=%d", ifdesc->bAlternateSetting);

		if (ifdesc->bNumEndpoints == 0)
			continue;

		for (ep_iter_init(&iep, &iif); iep.valid; ep_iter_next(&iep)) {
			const usbh_endpoint_descriptor_t *const epdesc = ep_get(&iep);
			if (((epdesc->bmAttributes & 0x03) == USBH_EPTYPE_ISO)
					&& ((epdesc->bEndpointAddress & 0x80) ==  USBH_EPDIR_IN)) {

				uinfof("\t  Endpoint wMaxPacketSize = %d", epdesc->wMaxPacketSize);

				if (epdesc->wMaxPacketSize >= min_ep_size) {
					if (epdesc->wMaxPacketSize < sz) {
						uinfo("\t    Found new optimal alternate setting");
						sz = epdesc->wMaxPacketSize;
						alt = ifdesc->bAlternateSetting;
						ep = epdesc;
					}
				}
			}
		}
	}

	if (ep && alt) {
		uinfof("\tSelecting Alternate setting %d", alt);
		if (usbhStdReqSetInterface(uvcdp->dev, if_get(&uvcdp->ivs)->bInterfaceNumber, alt) == HAL_SUCCESS) {
			usbhEPObjectInit(&uvcdp->ep_iso, uvcdp->dev, ep);
			usbhEPSetName(&uvcdp->ep_iso, "UVC[ISO ]");
			return HAL_SUCCESS;
		}
	}

	return HAL_FAILED;
}

#if	USBH_DEBUG_ENABLE && USBHUVC_DEBUG_ENABLE_INFO
void usbhuvcPrintProbeCommit(const usbh_uvc_ctrl_vs_probecommit_data_t *pc) {

	//uinfof("UVC: probe/commit data:");
	uinfof("\tbmHint=%04x", pc->bmHint);
	uinfof("\tbFormatIndex=%d, bFrameIndex=%d, dwFrameInterval=%u",
			pc->bFormatIndex, pc->bFrameIndex, pc->dwFrameInterval);
	uinfof("\twKeyFrameRate=%d, wPFrameRate=%d, wCompQuality=%u, wCompWindowSize=%u",
			pc->wKeyFrameRate, pc->wPFrameRate, pc->wCompQuality, pc->wCompWindowSize);
	uinfof("\twDelay=%d", pc->wDelay);
	uinfof("\tdwMaxVideoFrameSize=%u", pc->dwMaxVideoFrameSize);
	uinfof("\tdwMaxPayloadTransferSize=%u", pc->dwMaxPayloadTransferSize);
/*	uinfof("\tdwClockFrequency=%u", pc->dwClockFrequency);
	uinfof("\tbmFramingInfo=%02x", pc->bmFramingInfo);
	uinfof("\tbPreferedVersion=%d, bMinVersion=%d, bMaxVersion=%d",
			pc->bPreferedVersion, pc->bMinVersion, pc->bMaxVersion); */
}
#endif

static void _post(USBHUVCDriver *uvcdp, usbh_urb_t *urb, memory_pool_t *mp, uint16_t type) {
	usbhuvc_message_base_t *const msg = (usbhuvc_message_base_t *)((uint8_t *)urb->buff - offsetof(usbhuvc_message_data_t, data));
	msg->timestamp = osalOsGetSystemTimeX();

	usbhuvc_message_base_t *const new_msg = (usbhuvc_message_base_t *)chPoolAllocI(mp);
	if (new_msg != NULL) {
		/* allocated the new buffer, now try to post the message to the mailbox */
		if (chMBPostI(&uvcdp->mb, (msg_t)msg) == MSG_OK) {
			/* everything OK, complete the missing fields */
			msg->type = type;
			msg->length = urb->actualLength;

			/* change the URB's buffer to the newly allocated one */
			urb->buff = ((usbhuvc_message_data_t *)new_msg)->data;
		} else {
			/* couldn't post the message, free the newly allocated buffer */
			uerr("UVC: error, mailbox overrun");
			chPoolFreeI(&uvcdp->mp_status, new_msg);
		}
	} else {
		uerrf("UVC: error, %s pool overrun", mp == &uvcdp->mp_data ? "data" : "status");
	}
}

static void _cb_int(usbh_urb_t *urb) {
	USBHUVCDriver *uvcdp = (USBHUVCDriver *)urb->userData;

	switch (urb->status) {
	case USBH_URBSTATUS_OK:
		if (urb->actualLength >= 2) {
			_post(uvcdp, urb, &uvcdp->mp_status, USBHUVC_MESSAGETYPE_STATUS);
		} else {
			uerrf("UVC: INT IN, actualLength=%d", urb->actualLength);
		}
		break;
	case USBH_URBSTATUS_TIMEOUT:	/* the device NAKed */
		udbg("UVC: INT IN no info");
		break;
	case USBH_URBSTATUS_DISCONNECTED:
	case USBH_URBSTATUS_CANCELLED:
		uwarn("UVC: INT IN status = DISCONNECTED/CANCELLED, aborting");
		return;
	default:
		uerrf("UVC: INT IN error, unexpected status = %d", urb->status);
		break;
	}

	usbhURBObjectResetI(urb);
	usbhURBSubmitI(urb);
}

static void _cb_iso(usbh_urb_t *urb) {
	USBHUVCDriver *uvcdp = (USBHUVCDriver *)urb->userData;

	if ((urb->status == USBH_URBSTATUS_DISCONNECTED)
			|| (urb->status == USBH_URBSTATUS_CANCELLED)) {
		uwarn("UVC: ISO IN status = DISCONNECTED/CANCELLED, aborting");
		return;
	}

	if (urb->status != USBH_URBSTATUS_OK) {
		uerrf("UVC: ISO IN error, unexpected status = %d", urb->status);
	} else if (urb->actualLength >= 2) {
		const uint8_t *const buff = (const uint8_t *)urb->buff;
		if (buff[0] < 2) {
			uerrf("UVC: ISO IN, bHeaderLength=%d", buff[0]);
		} else if (buff[0] > urb->actualLength) {
			uerrf("UVC: ISO IN, bHeaderLength=%d > actualLength=%d", buff[0], urb->actualLength);
		} else {
			udbgf("UVC: ISO IN len=%d, hdr=%d, FID=%d, EOF=%d, ERR=%d, EOH=%d",
						urb->actualLength,
						buff[0],
						buff[1] & UVC_HDR_FID,
						buff[1] & UVC_HDR_EOF,
						buff[1] & UVC_HDR_ERR,
						buff[1] & UVC_HDR_EOH);

			if ((urb->actualLength > buff[0])
					|| (buff[1] & (UVC_HDR_EOF | UVC_HDR_ERR))) {
				_post(uvcdp, urb, &uvcdp->mp_data, USBHUVC_MESSAGETYPE_DATA);
			} else {
				udbgf("UVC: ISO IN skip: len=%d, hdr=%d, FID=%d, EOF=%d, ERR=%d, EOH=%d",
						urb->actualLength,
						buff[0],
						buff[1] & UVC_HDR_FID,
						buff[1] & UVC_HDR_EOF,
						buff[1] & UVC_HDR_ERR,
						buff[1] & UVC_HDR_EOH);
			}
		}
	} else if (urb->actualLength > 0) {
		uerrf("UVC: ISO IN, actualLength=%d", urb->actualLength);
	}

	usbhURBObjectResetI(urb);
	usbhURBSubmitI(urb);
}


bool usbhuvcStreamStart(USBHUVCDriver *uvcdp, uint16_t min_ep_sz) {
	bool ret = HAL_FAILED;

	osalSysLock();
	osalDbgCheck(uvcdp && (uvcdp->state != USBHUVC_STATE_UNINITIALIZED) &&
					(uvcdp->state != USBHUVC_STATE_BUSY));
	if (uvcdp->state == USBHUVC_STATE_STREAMING) {
		osalSysUnlock();
		return HAL_SUCCESS;
	}
	if (uvcdp->state != USBHUVC_STATE_READY) {
		osalSysUnlock();
		return HAL_FAILED;
	}
	uvcdp->state = USBHUVC_STATE_BUSY;
	osalSysUnlock();

	uint32_t workramsz;
	const uint8_t *elem;
	uint32_t datapackets;
	uint32_t data_sz;

	//set the alternate setting
	if (_set_vs_alternate(uvcdp, min_ep_sz) != HAL_SUCCESS)
		goto exit;

	//reserve working RAM
	data_sz = (uvcdp->ep_iso.wMaxPacketSize + sizeof(usbhuvc_message_data_t) + 3) & ~3;
	datapackets = HAL_USBHUVC_WORK_RAM_SIZE / data_sz;
	if (datapackets == 0) {
		uerr("Not enough work RAM");
		goto failed;
	}

	workramsz = datapackets * data_sz;
	uinfof("Reserving %u bytes of RAM (%d data packets of %d bytes)", workramsz, datapackets, data_sz);
	if (datapackets > (HAL_USBHUVC_MAX_MAILBOX_SZ - HAL_USBHUVC_STATUS_PACKETS_COUNT)) {
		uwarn("Mailbox may overflow, use a larger HAL_USBHUVC_MAX_MAILBOX_SZ. UVC will under-utilize the assigned work RAM.");
	}
	chMBResumeX(&uvcdp->mb);

	uvcdp->mp_data_buffer = chHeapAlloc(NULL, workramsz);
	if (uvcdp->mp_data_buffer == NULL) {
		uerr("Couldn't reserve RAM");
		goto failed;
	}

	//initialize the mempool
	chPoolObjectInit(&uvcdp->mp_data, data_sz, NULL);
	elem = (const uint8_t *)uvcdp->mp_data_buffer;
	while (datapackets--) {
		chPoolFree(&uvcdp->mp_data, (void *)elem);
		elem += data_sz;
	}

	//open the endpoint
	usbhEPOpen(&uvcdp->ep_iso);

	//allocate 1 buffer and submit the first transfer
	{
		usbhuvc_message_data_t *const msg = (usbhuvc_message_data_t *)chPoolAlloc(&uvcdp->mp_data);
		osalDbgCheck(msg);
		usbhURBObjectInit(&uvcdp->urb_iso, &uvcdp->ep_iso, _cb_iso, uvcdp, msg->data, uvcdp->ep_iso.wMaxPacketSize);
	}

	usbhURBSubmit(&uvcdp->urb_iso);

	ret = HAL_SUCCESS;
	goto exit;

failed:
	_set_vs_alternate(uvcdp, 0);
	if (uvcdp->mp_data_buffer)
		chHeapFree(uvcdp->mp_data_buffer);

exit:
	osalSysLock();
	if (ret == HAL_SUCCESS)
		uvcdp->state = USBHUVC_STATE_STREAMING;
	else
		uvcdp->state = USBHUVC_STATE_READY;
	osalSysUnlock();
	return ret;
}

bool usbhuvcStreamStop(USBHUVCDriver *uvcdp) {
	osalSysLock();
	osalDbgCheck(uvcdp && (uvcdp->state != USBHUVC_STATE_UNINITIALIZED) &&
					(uvcdp->state != USBHUVC_STATE_BUSY));
	if (uvcdp->state != USBHUVC_STATE_STREAMING) {
		osalSysUnlock();
		return HAL_SUCCESS;
	}
	uvcdp->state = USBHUVC_STATE_BUSY;

	//close the ISO endpoint
	usbhEPCloseS(&uvcdp->ep_iso);

	//purge the mailbox
	chMBResetI(&uvcdp->mb);		//TODO: the status messages are lost!!
	chMtxLockS(&uvcdp->mtx);
	osalSysUnlock();

	//free the working memory
	chHeapFree(uvcdp->mp_data_buffer);
	uvcdp->mp_data_buffer = 0;

	//set alternate setting to 0
	_set_vs_alternate(uvcdp, 0);

	osalSysLock();
	uvcdp->state = USBHUVC_STATE_READY;
	chMtxUnlockS(&uvcdp->mtx);
	osalSysUnlock();
	return HAL_SUCCESS;
}

bool usbhuvcFindVSDescriptor(USBHUVCDriver *uvcdp,
		generic_iterator_t *ics,
		uint8_t bDescriptorSubtype,
		bool start) {

	if (start)
		cs_iter_init(ics, (generic_iterator_t *)&uvcdp->ivs);
	else
		cs_iter_next(ics);

	for (; ics->valid; cs_iter_next(ics)) {
		if (ics->curr[1] != UVC_CS_INTERFACE)
			break;
		if (ics->curr[2] == bDescriptorSubtype)
			return HAL_SUCCESS;
		if (!start)
			break;
	}
	return HAL_FAILED;
}

void usbhuvcResetPC(USBHUVCDriver *uvcdp) {
	memset(&uvcdp->pc, 0, sizeof(uvcdp->pc));
}

bool usbhuvcProbe(USBHUVCDriver *uvcdp) {
//	memset(&uvcdp->pc_min, 0, sizeof(uvcdp->pc_min));
//	memset(&uvcdp->pc_max, 0, sizeof(uvcdp->pc_max));

	if (usbhuvcVSRequest(uvcdp, UVC_SET_CUR, UVC_CTRL_VS_PROBE_CONTROL, sizeof(uvcdp->pc), (uint8_t *)&uvcdp->pc) != HAL_SUCCESS)
		return HAL_FAILED;
	if (usbhuvcVSRequest(uvcdp, UVC_GET_CUR, UVC_CTRL_VS_PROBE_CONTROL, sizeof(uvcdp->pc), (uint8_t *)&uvcdp->pc) != HAL_SUCCESS)
		return HAL_FAILED;
	if (usbhuvcVSRequest(uvcdp, UVC_GET_MAX, UVC_CTRL_VS_PROBE_CONTROL, sizeof(uvcdp->pc_max), (uint8_t *)&uvcdp->pc_max) != HAL_SUCCESS)
		return HAL_FAILED;
	if (usbhuvcVSRequest(uvcdp, UVC_GET_MIN, UVC_CTRL_VS_PROBE_CONTROL, sizeof(uvcdp->pc_min), (uint8_t *)&uvcdp->pc_min) != HAL_SUCCESS)
		return HAL_FAILED;
	return HAL_SUCCESS;
}

bool usbhuvcCommit(USBHUVCDriver *uvcdp) {
	if (usbhuvcVSRequest(uvcdp, UVC_SET_CUR, UVC_CTRL_VS_COMMIT_CONTROL, sizeof(uvcdp->pc), (uint8_t *)&uvcdp->pc) != HAL_SUCCESS)
		return HAL_FAILED;

	osalSysLock();
	if (uvcdp->state == USBHUVC_STATE_ACTIVE)
		uvcdp->state = USBHUVC_STATE_READY;
	osalSysUnlock();
	return HAL_SUCCESS;
}

uint32_t usbhuvcEstimateRequiredEPSize(USBHUVCDriver *uvcdp, const uint8_t *formatdesc,
		const uint8_t *framedesc, uint32_t dwFrameInterval) {

	osalDbgCheck(framedesc);
	osalDbgCheck(framedesc[0] > 3);
	osalDbgCheck(framedesc[1] == UVC_CS_INTERFACE);
	osalDbgCheck(formatdesc);
	osalDbgCheck(formatdesc[0] > 3);
	osalDbgCheck(formatdesc[1] == UVC_CS_INTERFACE);

	uint16_t w, h, div, mul;
	uint8_t bpp;

	switch (framedesc[2]) {
	case UVC_VS_FRAME_MJPEG: {
		const usbh_uvc_frame_mjpeg_t *frame = (const usbh_uvc_frame_mjpeg_t *)framedesc;
		//const usbh_uvc_format_mjpeg_t *fmt = (const usbh_uvc_format_mjpeg_t *)formatdesc;
		w = frame->wWidth;
		h = frame->wHeight;
		bpp = 16;	//TODO: check this!!
		mul = 1;
		div = 5;	//TODO: check this estimate
	}	break;
	case UVC_VS_FRAME_UNCOMPRESSED: {
		const usbh_uvc_frame_uncompressed_t *frame = (const usbh_uvc_frame_uncompressed_t *)framedesc;
		const usbh_uvc_format_uncompressed *fmt = (const usbh_uvc_format_uncompressed *)formatdesc;
		w = frame->wWidth;
		h = frame->wHeight;
		bpp = fmt->bBitsPerPixel;
		mul = div = 1;
	}	break;
	default:
		uwarn("Unsupported format");
		return 0xffffffff;
	}

	uint32_t sz = w * h / 8 * bpp;
	sz *= 10000000UL / dwFrameInterval;
	sz /= 1000;

	if (uvcdp->dev->speed == USBH_DEVSPEED_HIGH)
		div *= 8;

	return (sz * mul) / div + 12;
}

static usbh_baseclassdriver_t *_uvc_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem) {

	USBHUVCDriver *uvcdp;
	uint8_t i;

	if (_usbh_match_descriptor(descriptor, rem, USBH_DT_INTERFACE_ASSOCIATION,
			0x0e, 0x03, 0x00) != HAL_SUCCESS)
		return NULL;

	/* alloc driver */
	for (i = 0; i < HAL_USBHUVC_MAX_INSTANCES; i++) {
		if (USBHUVCD[i].dev == NULL) {
			uvcdp = &USBHUVCD[i];
			goto alloc_ok;
		}
	}

	uwarn("Can't alloc UVC driver");

	/* can't alloc */
	return NULL;

alloc_ok:
	/* initialize the driver's variables */
	uvcdp->ivc.curr = uvcdp->ivs.curr = NULL;

	usbhEPSetName(&dev->ctrl, "UVC[CTRL]");

	const usbh_ia_descriptor_t *iad = (const usbh_ia_descriptor_t *)descriptor;
	if_iterator_t iif;
	generic_iterator_t ics;
	generic_iterator_t iep;

	iif.iad = iad;
	iif.curr = descriptor;
	iif.rem = rem;

	for (if_iter_next(&iif); iif.valid; if_iter_next(&iif)) {
		if (iif.iad != iad) break;

		const usbh_interface_descriptor_t *const ifdesc = if_get(&iif);
		if (ifdesc->bInterfaceClass != UVC_CC_VIDEO) {
			uwarnf("Skipping Interface %d (class != UVC_CC_VIDEO)",
					ifdesc->bInterfaceNumber);
			continue;
		}

		uinfof("Interface %d, Alt=%d, Class=UVC_CC_VIDEO, Subclass=%02x",
				ifdesc->bInterfaceNumber,
				ifdesc->bAlternateSetting,
				ifdesc->bInterfaceSubClass);

		switch (ifdesc->bInterfaceSubClass) {
		case UVC_SC_VIDEOCONTROL:
			if (uvcdp->ivc.curr == NULL) {
				uvcdp->ivc = iif;
			}
			for (cs_iter_init(&ics, (generic_iterator_t *)&iif); ics.valid; cs_iter_next(&ics)) {
				if (ics.curr[1] != UVC_CS_INTERFACE) {
					uwarnf("Unknown descriptor=%02X", ics.curr[1]);
					continue;
				}
				switch (ics.curr[2]) {
				case UVC_VC_HEADER:
					uinfo("  VC_HEADER"); break;
				case UVC_VC_INPUT_TERMINAL:
					uinfof("    VC_INPUT_TERMINAL, ID=%d", ics.curr[3]); break;
				case UVC_VC_OUTPUT_TERMINAL:
					uinfof("    VC_OUTPUT_TERMINAL, ID=%d", ics.curr[3]); break;
				case UVC_VC_SELECTOR_UNIT:
					uinfof("    VC_SELECTOR_UNIT, ID=%d", ics.curr[3]); break;
				case UVC_VC_PROCESSING_UNIT:
					uinfof("    VC_PROCESSING_UNIT, ID=%d", ics.curr[3]); break;
				case UVC_VC_EXTENSION_UNIT:
					uinfof("    VC_EXTENSION_UNIT, ID=%d", ics.curr[3]); break;
				default:
					uwarnf("Unknown video bDescriptorSubtype=%02x", ics.curr[2]);
					break;
				}
			}
			break;
		case UVC_SC_VIDEOSTREAMING:
			if (uvcdp->ivs.curr == NULL) {
				uvcdp->ivs = iif;
			}
			for (cs_iter_init(&ics, (generic_iterator_t *)&iif); ics.valid; cs_iter_next(&ics)) {
				if (ics.curr[1] != UVC_CS_INTERFACE) {
					uwarnf("Unknown descriptor=%02X", ics.curr[1]);
					continue;
				}
				switch (ics.curr[2]) {
				case UVC_VS_INPUT_HEADER:
					uinfo("  VS_INPUT_HEADER"); break;
				case UVC_VS_OUTPUT_HEADER:
					uinfo("  VS_OUTPUT_HEADER"); break;
				case UVC_VS_STILL_IMAGE_FRAME:
					uinfo("    VS_STILL_IMAGE_FRAME"); break;

				case UVC_VS_FORMAT_UNCOMPRESSED:
					uinfof("    VS_FORMAT_UNCOMPRESSED, bFormatIndex=%d", ics.curr[3]); break;
				case UVC_VS_FORMAT_MPEG2TS:
					uinfof("    VS_FORMAT_MPEG2TS, bFormatIndex=%d", ics.curr[3]); break;
				case UVC_VS_FORMAT_DV:
					uinfof("    VS_FORMAT_DV, bFormatIndex=%d", ics.curr[3]); break;
				case UVC_VS_FORMAT_MJPEG:
					uinfof("    VS_FORMAT_MJPEG, bFormatIndex=%d", ics.curr[3]); break;
				case UVC_VS_FORMAT_FRAME_BASED:
					uinfof("    VS_FORMAT_FRAME_BASED, bFormatIndex=%d", ics.curr[3]); break;
				case UVC_VS_FORMAT_STREAM_BASED:
					uinfof("    VS_FORMAT_STREAM_BASED, bFormatIndex=%d", ics.curr[3]); break;

				case UVC_VS_FRAME_UNCOMPRESSED:
					uinfof("      VS_FRAME_UNCOMPRESSED, bFrameIndex=%d", ics.curr[3]); break;
				case UVC_VS_FRAME_MJPEG:
					uinfof("      VS_FRAME_MJPEG, bFrameIndex=%d", ics.curr[3]); break;
				case UVC_VS_FRAME_FRAME_BASED:
					uinfof("      VS_FRAME_FRAME_BASED, bFrameIndex=%d", ics.curr[3]); break;

				case UVC_VS_COLOR_FORMAT:
					uinfo("      VS_COLOR_FORMAT"); break;
				default:
					uwarnf("Unknown video bDescriptorSubtype=%02x", ics.curr[2]);
					break;
				}
			}
			break;
		default:
			uwarnf("Unknown video bInterfaceSubClass=%02x", ifdesc->bInterfaceSubClass);
			break;
		}

		for (ep_iter_init(&iep, &iif); iep.valid; ep_iter_next(&iep)) {
			const usbh_endpoint_descriptor_t *const epdesc = ep_get(&iep);

			if ((ifdesc->bInterfaceSubClass == UVC_SC_VIDEOCONTROL)
					&& ((epdesc->bmAttributes & 0x03) == USBH_EPTYPE_INT)
					&& ((epdesc->bEndpointAddress & 0x80) ==  USBH_EPDIR_IN)) {
				/* found VC interrupt endpoint */
				uinfof("  VC Interrupt endpoint; %02x, bInterval=%d",
						epdesc->bEndpointAddress, epdesc->bInterval);
				usbhEPObjectInit(&uvcdp->ep_int, dev, epdesc);
				usbhEPSetName(&uvcdp->ep_int, "UVC[INT ]");
			} else if ((ifdesc->bInterfaceSubClass == UVC_SC_VIDEOSTREAMING)
					&& ((epdesc->bmAttributes & 0x03) == USBH_EPTYPE_ISO)
					&& ((epdesc->bEndpointAddress & 0x80) ==  USBH_EPDIR_IN)) {
				/* found VS isochronous endpoint */
				uinfof("  VS Isochronous endpoint; %02x, bInterval=%d, bmAttributes=%02x",
						epdesc->bEndpointAddress, epdesc->bInterval, epdesc->bmAttributes);
			} else {
				/* unknown EP */
				uwarnf("  <unknown endpoint>, bEndpointAddress=%02x, bmAttributes=%02x",
									epdesc->bEndpointAddress, epdesc->bmAttributes);
			}

			for (cs_iter_init(&ics, &iep); ics.valid; cs_iter_next(&ics)) {
				uinfof("    CS_ENDPOINT bLength=%d, bDescriptorType=%02X",
						ics.curr[0], ics.curr[1]);
			}
		}
	}

	if ((uvcdp->ivc.curr == NULL) || (uvcdp->ivs.curr == NULL)) {
		return NULL;
	}

//	uvcdp->dev = dev;

	_set_vs_alternate(uvcdp, 0);

	/* initialize the INT endpoint */
	chPoolObjectInit(&uvcdp->mp_status, sizeof(usbhuvc_message_status_t), NULL);
	for(i = 0; i < HAL_USBHUVC_STATUS_PACKETS_COUNT; i++)
		chPoolFree(&uvcdp->mp_status, &uvcdp->mp_status_buffer[i]);

	usbhEPOpen(&uvcdp->ep_int);

	usbhuvc_message_status_t *const msg = (usbhuvc_message_status_t *)chPoolAlloc(&uvcdp->mp_status);
	osalDbgCheck(msg);
	usbhURBObjectInit(&uvcdp->urb_int, &uvcdp->ep_int, _cb_int, uvcdp, msg->data, USBHUVC_MAX_STATUS_PACKET_SZ);
	osalSysLock();
	usbhURBSubmitI(&uvcdp->urb_int);
	uvcdp->state = USBHUVC_STATE_ACTIVE;
	osalOsRescheduleS();	/* because of usbhURBSubmitI */
	osalSysUnlock();

	dev->keepFullCfgDesc++;
	return (usbh_baseclassdriver_t *)uvcdp;
}

static void _uvc_unload(usbh_baseclassdriver_t *drv) {
	USBHUVCDriver *const uvcdp = (USBHUVCDriver *)drv;

	usbhuvcStreamStop(uvcdp);

	usbhEPClose(&uvcdp->ep_int);

	//TODO: free

	if (drv->dev->keepFullCfgDesc)
		drv->dev->keepFullCfgDesc--;

	osalSysLock();
	uvcdp->state = USBHUVC_STATE_STOP;
	osalSysUnlock();
}

static void _object_init(USBHUVCDriver *uvcdp) {
	osalDbgCheck(uvcdp != NULL);
	memset(uvcdp, 0, sizeof(*uvcdp));
	uvcdp->info = &usbhuvcClassDriverInfo;
	chMBObjectInit(&uvcdp->mb, uvcdp->mb_buff, HAL_USBHUVC_MAX_MAILBOX_SZ);
	chMtxObjectInit(&uvcdp->mtx);
	uvcdp->state = USBHUVC_STATE_STOP;
}

static void _uvc_init(void) {
	uint8_t i;
	for (i = 0; i < HAL_USBHUVC_MAX_INSTANCES; i++) {
		_object_init(&USBHUVCD[i]);
	}
}

#endif

