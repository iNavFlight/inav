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

#if HAL_USBH_USE_AOA

#if !HAL_USE_USBH
#error "USBHAOA needs USBH"
#endif

#include <string.h>
#include "usbh/dev/aoa.h"
#include "usbh/internal.h"

//#pragma GCC optimize("Og")


#if USBHAOA_DEBUG_ENABLE_TRACE
#define udbgf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define udbg(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define udbgf(f, ...)  do {} while(0)
#define udbg(f, ...)   do {} while(0)
#endif

#if USBHAOA_DEBUG_ENABLE_INFO
#define uinfof(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uinfo(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uinfof(f, ...)  do {} while(0)
#define uinfo(f, ...)   do {} while(0)
#endif

#if USBHAOA_DEBUG_ENABLE_WARNINGS
#define uwarnf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uwarn(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uwarnf(f, ...)  do {} while(0)
#define uwarn(f, ...)   do {} while(0)
#endif

#if USBHAOA_DEBUG_ENABLE_ERRORS
#define uerrf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uerr(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uerrf(f, ...)  do {} while(0)
#define uerr(f, ...)   do {} while(0)
#endif


/*===========================================================================*/
/* Constants													 		 	 */
/*===========================================================================*/

#if !defined(HAL_USBHAOA_DEFAULT_MANUFACTURER)
#define HAL_USBHAOA_DEFAULT_MANUFACTURER   "ChibiOS"
#endif

#if !defined(HAL_USBHAOA_DEFAULT_MODEL)
#define HAL_USBHAOA_DEFAULT_MODEL          "USBH AOA Driver"
#endif

#if !defined(HAL_USBHAOA_DEFAULT_DESCRIPTION)
#define HAL_USBHAOA_DEFAULT_DESCRIPTION    "ChibiOS USBH AOA Driver"
#endif

#if !defined(HAL_USBHAOA_DEFAULT_VERSION)
#define HAL_USBHAOA_DEFAULT_VERSION        CH_KERNEL_VERSION
#endif

#if !defined(HAL_USBHAOA_DEFAULT_URI)
#define HAL_USBHAOA_DEFAULT_URI            NULL
#endif

#if !defined(HAL_USBHAOA_DEFAULT_SERIAL)
#define HAL_USBHAOA_DEFAULT_SERIAL         NULL
#endif

#if !defined(HAL_USBHAOA_DEFAULT_AUDIO_MODE)
#define HAL_USBHAOA_DEFAULT_AUDIO_MODE	   USBHAOA_AUDIO_MODE_DISABLED
#endif

#define AOA_GOOGLE_VID 	        			0x18D1
#define AOA_GOOGLE_PID_ACCESSORY    		0x2D00
#define AOA_GOOGLE_PID_ACCESSORY_ABD    	0x2D01
#define AOA_GOOGLE_PID_AUDIO    			0x2D02
#define AOA_GOOGLE_PID_AUDIO_ABD    		0x2D03
#define AOA_GOOGLE_PID_ACCESSORY_AUDIO    	0x2D04
#define AOA_GOOGLE_PID_ACCESSORY_AUDIO_ABD  0x2D05

#define AOA_ACCESSORY_GET_PROTOCOL          51
#define AOA_ACCESSORY_SEND_STRING           52
#define AOA_ACCESSORY_START                 53

#define AOA_SET_AUDIO_MODE					58

static bool _get_protocol(usbh_device_t *dev, uint16_t *protocol);
static bool _accessory_start(usbh_device_t *dev);
static bool _set_audio_mode(usbh_device_t *dev, uint16_t mode);
static bool _send_string(usbh_device_t *dev, uint8_t index, const char *string);


static void _stop_channelS(USBHAOAChannel *aoacp);

/*===========================================================================*/
/* USB Class driver loader for AOA								 		 	 */
/*===========================================================================*/
USBHAOADriver USBHAOAD[HAL_USBHAOA_MAX_INSTANCES];

static usbh_baseclassdriver_t *_aoa_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem);
static void _aoa_unload(usbh_baseclassdriver_t *drv);
static void _aoa_init(void);

static const usbh_classdriver_vmt_t class_driver_vmt = {
	_aoa_init,
	_aoa_load,
	_aoa_unload
};

const usbh_classdriverinfo_t usbhaoaClassDriverInfo = {
	"AOA", &class_driver_vmt
};

#if defined(HAL_USBHAOA_FILTER_CALLBACK)
extern usbhaoa_filter_callback_t HAL_USBHAOA_FILTER_CALLBACK;
#endif

static usbh_baseclassdriver_t *_aoa_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem) {
	int i;
	USBHAOADriver *aoap;

	if (dev->devDesc.idVendor != AOA_GOOGLE_VID) {
		uint16_t protocol;
		static USBHAOAConfig config = {
			{
				HAL_USBHAOA_DEFAULT_MANUFACTURER,
				HAL_USBHAOA_DEFAULT_MODEL,
				HAL_USBHAOA_DEFAULT_DESCRIPTION,
				HAL_USBHAOA_DEFAULT_VERSION,
				HAL_USBHAOA_DEFAULT_URI,
				HAL_USBHAOA_DEFAULT_SERIAL
			},

			{
				HAL_USBHAOA_DEFAULT_AUDIO_MODE,
			}
		};

		uinfo("AOA: Unrecognized VID");

#if defined(HAL_USBHAOA_FILTER_CALLBACK)
		if (!HAL_USBHAOA_FILTER_CALLBACK(dev, descriptor, rem, &config)) {
			return NULL;
		}
#endif

		uinfo("AOA: Try if it's an Android device");
		if (_get_protocol(dev, &protocol) != HAL_SUCCESS) {
			return NULL;
		}
		uinfof("AOA: Possible Android device found (protocol=%d)", protocol);

		if (config.channel.manufacturer != NULL) {
			if ((_send_string(dev, USBHAOA_ACCESSORY_STRING_MANUFACTURER, config.channel.manufacturer) != HAL_SUCCESS)
				|| (_send_string(dev, USBHAOA_ACCESSORY_STRING_MODEL, config.channel.model) != HAL_SUCCESS)
				|| (_send_string(dev, USBHAOA_ACCESSORY_STRING_DESCRIPTION, config.channel.description) != HAL_SUCCESS)
				|| (_send_string(dev, USBHAOA_ACCESSORY_STRING_VERSION, config.channel.version) != HAL_SUCCESS)
				|| (_send_string(dev, USBHAOA_ACCESSORY_STRING_URI, config.channel.uri) != HAL_SUCCESS)
				|| (_send_string(dev, USBHAOA_ACCESSORY_STRING_SERIAL, config.channel.serial) != HAL_SUCCESS)) {
				uerr("AOA: Can't send string; abort start");
				return NULL;
			}
		}

		if (protocol > 1) {
			if (_set_audio_mode(dev, (uint16_t)(config.audio.mode)) != HAL_SUCCESS) {
				uerr("AOA: Can't set audio mode; abort channel start");
				return NULL;
			}
		}

		if (_accessory_start(dev) != HAL_SUCCESS) {
			uerr("AOA: Can't start accessory; abort channel start");
		}

		return NULL;
	}

	/* AOAv2:
		0x2D00	accessory				Provides two bulk endpoints for communicating with an Android application.
		0x2D01	accessory + adb			For debugging purposes during accessory development. Available only if the user has enabled USB Debugging in the Android device settings.
		0x2D02	audio					For streaming audio from an Android device to an accessory.
		0x2D03	audio + adb
		0x2D04	accessory + audio
		0x2D05  accessory + audio + adb
	*/

	switch (dev->devDesc.idProduct) {
	case AOA_GOOGLE_PID_ACCESSORY:
	case AOA_GOOGLE_PID_ACCESSORY_ABD:
//	case AOA_GOOGLE_PID_AUDIO:
//	case AOA_GOOGLE_PID_AUDIO_ABD:
	case AOA_GOOGLE_PID_ACCESSORY_AUDIO:
	case AOA_GOOGLE_PID_ACCESSORY_AUDIO_ABD:
		break;
	default:
		uerr("AOA: Unrecognized PID");
		return NULL;
	}

	const usbh_interface_descriptor_t * const ifdesc = (const usbh_interface_descriptor_t *)descriptor;
	if ((_usbh_match_descriptor(descriptor, rem, USBH_DT_INTERFACE, 0xFF, 0xFF, 0x00) != HAL_SUCCESS)
			|| (ifdesc->bNumEndpoints < 2)) {
		uerr("AOA: This IF is not the Accessory IF");
		return NULL;
	}

	uinfof("AOA: Found Accessory Interface #%d", ifdesc->bInterfaceNumber);

	for (i = 0; i < HAL_USBHAOA_MAX_INSTANCES; i++) {
		if (USBHAOAD[i].dev == NULL) {
			aoap = &USBHAOAD[i];
			goto alloc_ok;
		}
	}

	uwarn("AOA: Can't alloc driver");

	/* can't alloc */
	return NULL;

alloc_ok:
	/* initialize the driver's variables */
	usbhEPSetName(&dev->ctrl, "AOA[CTRL]");
	aoap->state = USBHAOA_STATE_ACTIVE;

	generic_iterator_t iep;
	if_iterator_t iif;
	iif.iad = 0;
	iif.curr = descriptor;
	iif.rem = rem;

	aoap->channel.epin.status = USBH_EPSTATUS_UNINITIALIZED;
	aoap->channel.epout.status = USBH_EPSTATUS_UNINITIALIZED;

	for (ep_iter_init(&iep, &iif); iep.valid; ep_iter_next(&iep)) {
		const usbh_endpoint_descriptor_t *const epdesc = ep_get(&iep);
		if ((epdesc->bEndpointAddress & 0x80) && (epdesc->bmAttributes == USBH_EPTYPE_BULK)) {
			uinfof("AOA: BULK IN endpoint found: bEndpointAddress=%02x", epdesc->bEndpointAddress);
			usbhEPObjectInit(&aoap->channel.epin, dev, epdesc);
			usbhEPSetName(&aoap->channel.epin, "AOA[BIN ]");
		} else if (((epdesc->bEndpointAddress & 0x80) == 0)
				&& (epdesc->bmAttributes == USBH_EPTYPE_BULK)) {
			uinfof("AOA: BULK OUT endpoint found: bEndpointAddress=%02x", epdesc->bEndpointAddress);
			usbhEPObjectInit(&aoap->channel.epout, dev, epdesc);
			usbhEPSetName(&aoap->channel.epout, "AOA[BOUT]");
		} else {
			uinfof("AOA: unsupported endpoint found: bEndpointAddress=%02x, bmAttributes=%02x",
				epdesc->bEndpointAddress, epdesc->bmAttributes);
		}
	}

	if ((aoap->channel.epin.status != USBH_EPSTATUS_CLOSED)
		|| (aoap->channel.epout.status != USBH_EPSTATUS_CLOSED)) {
		uwarn("AOA: Couldn't find endpoints");
		aoap->state = USBHAOA_STATE_STOP;
		return NULL;
	}

	aoap->state = USBHAOA_STATE_READY;
	aoap->channel.state = USBHAOA_CHANNEL_STATE_ACTIVE;
	uwarn("AOA: Ready");
	return (usbh_baseclassdriver_t *)aoap;
}

static void _aoa_unload(usbh_baseclassdriver_t *drv) {
	osalDbgCheck(drv != NULL);
	USBHAOADriver *const aoap = (USBHAOADriver *)drv;
	osalSysLock();
	_stop_channelS(&aoap->channel);
	aoap->channel.state = USBHAOA_CHANNEL_STATE_STOP;
	aoap->state = USBHAOA_STATE_STOP;
	osalSysUnlock();
}

/* ------------------------------------ */
/*      Accessory data channel          */
/* ------------------------------------ */

static void _submitOutI(USBHAOAChannel *aoacp, uint32_t len) {
	udbgf("AOA: Submit OUT %d", len);
	aoacp->oq_urb.requestedLength = len;
	usbhURBObjectResetI(&aoacp->oq_urb);
	usbhURBSubmitI(&aoacp->oq_urb);
}

static void _out_cb(usbh_urb_t *urb) {
	USBHAOAChannel *const aoacp = (USBHAOAChannel *)urb->userData;
	switch (urb->status) {
	case USBH_URBSTATUS_OK:
		aoacp->oq_ptr = aoacp->oq_buff;
		aoacp->oq_counter = 64;
		chThdDequeueNextI(&aoacp->oq_waiting, Q_OK);
		chnAddFlagsI(aoacp, CHN_OUTPUT_EMPTY | CHN_TRANSMISSION_END);
		return;
	case USBH_URBSTATUS_DISCONNECTED:
		uwarn("AOA: URB OUT disconnected");
		chThdDequeueNextI(&aoacp->oq_waiting, Q_RESET);
		chnAddFlagsI(aoacp, CHN_OUTPUT_EMPTY);
		return;
	default:
		uerrf("AOA: URB OUT status unexpected = %d", urb->status);
		break;
	}
	usbhURBObjectResetI(&aoacp->oq_urb);
	usbhURBSubmitI(&aoacp->oq_urb);
}

static size_t _write_timeout(USBHAOAChannel *aoacp, const uint8_t *bp,
		size_t n, systime_t timeout) {
	chDbgCheck(n > 0U);

	size_t w = 0;
	osalSysLock();
	while (true) {
		if (aoacp->state != USBHAOA_CHANNEL_STATE_READY) {
			osalSysUnlock();
			return w;
		}
		while (usbhURBIsBusy(&aoacp->oq_urb)) {
			if (chThdEnqueueTimeoutS(&aoacp->oq_waiting, timeout) != Q_OK) {
				osalSysUnlock();
				return w;
			}
		}

		*aoacp->oq_ptr++ = *bp++;
		if (--aoacp->oq_counter == 0) {
			_submitOutI(aoacp, 64);
			osalOsRescheduleS();
		}
		osalSysUnlock(); /* Gives a preemption chance in a controlled point.*/

		w++;
		if (--n == 0U)
			return w;

		osalSysLock();
	}
}

static msg_t _put_timeout(USBHAOAChannel *aoacp, uint8_t b, systime_t timeout) {

	osalSysLock();
	if (aoacp->state != USBHAOA_CHANNEL_STATE_READY) {
		osalSysUnlock();
		return Q_RESET;
	}

	while (usbhURBIsBusy(&aoacp->oq_urb)) {
		msg_t msg = chThdEnqueueTimeoutS(&aoacp->oq_waiting, timeout);
		if (msg < Q_OK) {
			osalSysUnlock();
			return msg;
		}
	}

	*aoacp->oq_ptr++ = b;
	if (--aoacp->oq_counter == 0) {
		_submitOutI(aoacp, 64);
		osalOsRescheduleS();
	}
	osalSysUnlock();
	return Q_OK;
}

static size_t _write(USBHAOAChannel *aoacp, const uint8_t *bp, size_t n) {
	return _write_timeout(aoacp, bp, n, TIME_INFINITE);
}

static msg_t _put(USBHAOAChannel *aoacp, uint8_t b) {
	return _put_timeout(aoacp, b, TIME_INFINITE);
}

static void _submitInI(USBHAOAChannel *aoacp) {
	udbg("AOA: Submit IN");
	usbhURBObjectResetI(&aoacp->iq_urb);
	usbhURBSubmitI(&aoacp->iq_urb);
}

static void _in_cb(usbh_urb_t *urb) {
	USBHAOAChannel *const aoacp = (USBHAOAChannel *)urb->userData;
	switch (urb->status) {
	case USBH_URBSTATUS_OK:
		if (urb->actualLength == 0) {
			udbgf("AOA: URB IN no data");
		} else {
			udbgf("AOA: URB IN data len=%d", urb->actualLength);
			aoacp->iq_ptr = aoacp->iq_buff;
			aoacp->iq_counter = urb->actualLength;
			chThdDequeueNextI(&aoacp->iq_waiting, Q_OK);
			chnAddFlagsI(aoacp, CHN_INPUT_AVAILABLE);
		}
		break;
	case USBH_URBSTATUS_DISCONNECTED:
		uwarn("AOA: URB IN disconnected");
		chThdDequeueNextI(&aoacp->iq_waiting, Q_RESET);
		break;
	default:
		uerrf("AOA: URB IN status unexpected = %d", urb->status);
		_submitInI(aoacp);
		break;
	}
}

static size_t _read_timeout(USBHAOAChannel *aoacp, uint8_t *bp,
		size_t n, systime_t timeout) {
	size_t r = 0;

	chDbgCheck(n > 0U);

	osalSysLock();
	while (true) {
		if (aoacp->state != USBHAOA_CHANNEL_STATE_READY) {
			osalSysUnlock();
			return r;
		}
		while (aoacp->iq_counter == 0) {
			if (!usbhURBIsBusy(&aoacp->iq_urb))
				_submitInI(aoacp);
			if (chThdEnqueueTimeoutS(&aoacp->iq_waiting, timeout) != Q_OK) {
				osalSysUnlock();
				return r;
			}
		}
		*bp++ = *aoacp->iq_ptr++;
		if (--aoacp->iq_counter == 0) {
			_submitInI(aoacp);
			osalOsRescheduleS();
		}
		osalSysUnlock();

		r++;
		if (--n == 0U)
			return r;

		osalSysLock();
	}
}

static msg_t _get_timeout(USBHAOAChannel *aoacp, systime_t timeout) {
	uint8_t b;

	osalSysLock();
	if (aoacp->state != USBHAOA_CHANNEL_STATE_READY) {
		osalSysUnlock();
		return Q_RESET;
	}
	while (aoacp->iq_counter == 0) {
		if (!usbhURBIsBusy(&aoacp->iq_urb))
			_submitInI(aoacp);
		msg_t msg = chThdEnqueueTimeoutS(&aoacp->iq_waiting, timeout);
		if (msg < Q_OK) {
			osalSysUnlock();
			return msg;
		}
	}
	b = *aoacp->iq_ptr++;
	if (--aoacp->iq_counter == 0) {
		_submitInI(aoacp);
		osalOsRescheduleS();
	}
	osalSysUnlock();

	return (msg_t)b;
}

static msg_t _get(USBHAOAChannel *aoacp) {
	return _get_timeout(aoacp, TIME_INFINITE);
}

static size_t _read(USBHAOAChannel *aoacp, uint8_t *bp, size_t n) {
	return _read_timeout(aoacp, bp, n, TIME_INFINITE);
}

static msg_t _ctl(USBHAOAChannel *ftdipp, unsigned int operation, void *arg) {
	(void)ftdipp;
	(void)operation;
	(void)arg;
	return MSG_OK;
}

static const struct AOADriverVMT async_channel_vmt = {
	(size_t)0,
	(size_t (*)(void *, const uint8_t *, size_t))_write,
	(size_t (*)(void *, uint8_t *, size_t))_read,
	(msg_t (*)(void *, uint8_t))_put,
	(msg_t (*)(void *))_get,
	(msg_t (*)(void *, uint8_t, systime_t))_put_timeout,
	(msg_t (*)(void *, systime_t))_get_timeout,
	(size_t (*)(void *, const uint8_t *, size_t, systime_t))_write_timeout,
	(size_t (*)(void *, uint8_t *, size_t, systime_t))_read_timeout,
	(msg_t (*)(void *, unsigned int, void *))_ctl
};

static void _stop_channelS(USBHAOAChannel *aoacp) {
	if (aoacp->state != USBHAOA_CHANNEL_STATE_READY)
		return;
	uwarn("AOA: Stop channel");
	chVTResetI(&aoacp->vt);
	usbhEPCloseS(&aoacp->epin);
	usbhEPCloseS(&aoacp->epout);
	chThdDequeueAllI(&aoacp->iq_waiting, Q_RESET);
	chThdDequeueAllI(&aoacp->oq_waiting, Q_RESET);
	chnAddFlagsI(aoacp, CHN_DISCONNECTED);
	aoacp->state = USBHAOA_CHANNEL_STATE_ACTIVE;
	osalOsRescheduleS();
}

static void _vt(void *p) {
	USBHAOAChannel *const aoacp = (USBHAOAChannel *)p;
	osalSysLockFromISR();
	uint32_t len = aoacp->oq_ptr - aoacp->oq_buff;
	if (len && !usbhURBIsBusy(&aoacp->oq_urb)) {
		_submitOutI(aoacp, len);
	}
	if ((aoacp->iq_counter == 0) && !usbhURBIsBusy(&aoacp->iq_urb)) {
		_submitInI(aoacp);
	}
	chVTSetI(&aoacp->vt, OSAL_MS2I(16), _vt, aoacp);
	osalSysUnlockFromISR();
}

void usbhaoaChannelStart(USBHAOADriver *aoap) {

	osalDbgCheck(aoap);

	USBHAOAChannel *const aoacp = (USBHAOAChannel *)&aoap->channel;

	osalDbgCheck(aoap->state == USBHAOA_STATE_READY);

	osalDbgCheck((aoacp->state == USBHAOA_CHANNEL_STATE_ACTIVE)
			|| (aoacp->state == USBHAOA_CHANNEL_STATE_READY));

	if (aoacp->state == USBHAOA_CHANNEL_STATE_READY)
		return;

	usbhURBObjectInit(&aoacp->oq_urb, &aoacp->epout, _out_cb, aoacp, aoacp->oq_buff, 0);
	chThdQueueObjectInit(&aoacp->oq_waiting);
	aoacp->oq_counter = 64;
	aoacp->oq_ptr = aoacp->oq_buff;
	usbhEPOpen(&aoacp->epout);

	usbhURBObjectInit(&aoacp->iq_urb, &aoacp->epin, _in_cb, aoacp, aoacp->iq_buff, 64);
	chThdQueueObjectInit(&aoacp->iq_waiting);
	aoacp->iq_counter = 0;
	aoacp->iq_ptr = aoacp->iq_buff;
	usbhEPOpen(&aoacp->epin);
	usbhURBSubmit(&aoacp->iq_urb);

	chVTObjectInit(&aoacp->vt);
	chVTSet(&aoacp->vt, OSAL_MS2I(16), _vt, aoacp);

	aoacp->state = USBHAOA_CHANNEL_STATE_READY;

	osalEventBroadcastFlags(&aoacp->event, CHN_CONNECTED | CHN_OUTPUT_EMPTY);
}

void usbhaoaChannelStop(USBHAOADriver *aoap) {
	osalDbgCheck((aoap->channel.state == USBHAOA_CHANNEL_STATE_ACTIVE)
			|| (aoap->channel.state == USBHAOA_CHANNEL_STATE_READY));
	osalSysLock();
	_stop_channelS(&aoap->channel);
	osalSysUnlock();
}

/* ------------------------------------ */
/*      General AOA functions           */
/* ------------------------------------ */
static bool _get_protocol(usbh_device_t *dev, uint16_t *protocol) {
	USBH_DEFINE_BUFFER(uint16_t proto);

	usbh_urbstatus_t ret = usbhControlRequest(dev,
			USBH_REQTYPE_DIR_IN | USBH_REQTYPE_TYPE_VENDOR | USBH_REQTYPE_RECIP_DEVICE,
			AOA_ACCESSORY_GET_PROTOCOL,
			0,
			0,
			2,
			(uint8_t *)&proto);

	if ((ret != USBH_URBSTATUS_OK) || (proto > 2))
		return HAL_FAILED;

	*protocol = proto;
	return HAL_SUCCESS;
}

static bool _accessory_start(usbh_device_t *dev) {
	usbh_urbstatus_t ret = usbhControlRequest(dev,
			USBH_REQTYPE_DIR_OUT | USBH_REQTYPE_TYPE_VENDOR | USBH_REQTYPE_RECIP_DEVICE,
			AOA_ACCESSORY_START,
			0,
			0,
			0,
			NULL);

	if (ret != USBH_URBSTATUS_OK)
		return HAL_FAILED;

	return HAL_SUCCESS;
}

static bool _set_audio_mode(usbh_device_t *dev, uint16_t mode) {
	usbh_urbstatus_t ret = usbhControlRequest(dev,
			USBH_REQTYPE_DIR_OUT | USBH_REQTYPE_TYPE_VENDOR | USBH_REQTYPE_RECIP_DEVICE,
			AOA_SET_AUDIO_MODE,
			mode,
			0,
			0,
			NULL);

	if (ret != USBH_URBSTATUS_OK)
		return HAL_FAILED;

	return HAL_SUCCESS;
}

static bool _send_string(usbh_device_t *dev, uint8_t index, const char *string)
{
	USBH_DEFINE_BUFFER(const char nullstr[1]) = {0};
	if (string == NULL)
		string = nullstr;

	usbh_urbstatus_t ret = usbhControlRequest(dev,
			USBH_REQTYPE_DIR_OUT | USBH_REQTYPE_TYPE_VENDOR | USBH_REQTYPE_RECIP_DEVICE,
			AOA_ACCESSORY_SEND_STRING,
			0,
			index,
			strlen(string) + 1,
			(uint8_t *)string);

	if (ret != USBH_URBSTATUS_OK)
		return HAL_FAILED;

	return HAL_SUCCESS;
}

static void _object_init(USBHAOADriver *aoap) {
	osalDbgCheck(aoap != NULL);
	memset(aoap, 0, sizeof(*aoap));
	aoap->info = &usbhaoaClassDriverInfo;
	aoap->state = USBHAOA_STATE_STOP;
	aoap->channel.vmt = &async_channel_vmt;
	osalEventObjectInit(&aoap->channel.event);
	aoap->channel.state = USBHAOA_CHANNEL_STATE_STOP;
}

static void _aoa_init(void) {
	uint8_t i;
	for (i = 0; i < HAL_USBHAOA_MAX_INSTANCES; i++) {
		_object_init(&USBHAOAD[i]);
	}
}

#endif
