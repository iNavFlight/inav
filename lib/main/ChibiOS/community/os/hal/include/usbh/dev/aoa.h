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

#ifndef USBH_AOA_H_
#define USBH_AOA_H_

#include "hal_usbh.h"

#if HAL_USE_USBH && HAL_USBH_USE_AOA

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

typedef enum {
	USBHAOA_CHANNEL_STATE_UNINIT = 0,
	USBHAOA_CHANNEL_STATE_STOP = 1,
	USBHAOA_CHANNEL_STATE_ACTIVE = 2,
	USBHAOA_CHANNEL_STATE_READY = 3
} usbhaoa_channel_state_t;

typedef enum {
	USBHAOA_STATE_UNINIT = 0,
	USBHAOA_STATE_STOP = 1,
	USBHAOA_STATE_ACTIVE = 2,
	USBHAOA_STATE_READY = 3
} usbhaoa_state_t;

typedef enum {
	USBHAOA_AUDIO_MODE_DISABLED = 0,
	USBHAOA_AUDIO_MODE_2CH_16BIT_PCM_44100 = 1,
} usbhaoa_audio_mode_t;

typedef struct {
	struct _aoa_channel_cfg {
		const char *manufacturer;
		const char *model;
		const char *description;
		const char *version;
		const char *uri;
		const char *serial;
	} channel;

	struct _aoa_audio_cfg {
		usbhaoa_audio_mode_t mode;
	} audio;

} USBHAOAConfig;

#define _aoa_driver_methods                                          \
  _base_asynchronous_channel_methods

struct AOADriverVMT {
	_aoa_driver_methods
};

typedef struct USBHAOAChannel USBHAOAChannel;
typedef struct USBHAOADriver USBHAOADriver;

struct USBHAOAChannel {
	/* inherited from abstract asyncrhonous channel driver */
	const struct AOADriverVMT *vmt;
	_base_asynchronous_channel_data

	usbh_ep_t epin;
	usbh_urb_t iq_urb;
	threads_queue_t	iq_waiting;
	uint32_t iq_counter;
	USBH_DECLARE_STRUCT_MEMBER(uint8_t iq_buff[64]);
	uint8_t *iq_ptr;

	usbh_ep_t epout;
	usbh_urb_t oq_urb;
	threads_queue_t	oq_waiting;
	uint32_t oq_counter;
	USBH_DECLARE_STRUCT_MEMBER(uint8_t oq_buff[64]);
	uint8_t *oq_ptr;

	virtual_timer_t vt;

	usbhaoa_channel_state_t state;
};

struct USBHAOADriver {
	/* inherited from abstract class driver */
	_usbh_base_classdriver_data

	USBHAOAChannel channel;

	usbhaoa_state_t state;

};

#define USBHAOA_ACCESSORY_STRING_MANUFACTURER   0
#define USBHAOA_ACCESSORY_STRING_MODEL          1
#define USBHAOA_ACCESSORY_STRING_DESCRIPTION    2
#define USBHAOA_ACCESSORY_STRING_VERSION        3
#define USBHAOA_ACCESSORY_STRING_URI            4
#define USBHAOA_ACCESSORY_STRING_SERIAL         5

typedef bool (*usbhaoa_filter_callback_t)(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem, USBHAOAConfig *config);

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/
#define usbhaoaStop(aoap)

#define usbhaoaGetState(aoap) ((aoap)->state)

#define usbhaoaGetChannelState(aoap) ((aoap)->channel.state)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/
extern USBHAOADriver USBHAOAD[HAL_USBHAOA_MAX_INSTANCES];

#ifdef __cplusplus
extern "C" {
#endif
	/* AOA device driver */
	void usbhaoaChannelStart(USBHAOADriver *aoap);
	void usbhaoaChannelStop(USBHAOADriver *aoap);
#ifdef __cplusplus
}
#endif


#endif

#endif /* USBH_AOA_H_ */
