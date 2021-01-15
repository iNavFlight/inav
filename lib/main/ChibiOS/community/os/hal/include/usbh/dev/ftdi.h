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

#ifndef USBH_FTDI_H_
#define USBH_FTDI_H_

#include "hal_usbh.h"

#if HAL_USE_USBH && HAL_USBH_USE_FTDI

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/
#define USBHFTDI_FRAMING_DATABITS_7    (0x7 << 0)
#define USBHFTDI_FRAMING_DATABITS_8    (0x8 << 0)
#define USBHFTDI_FRAMING_PARITY_NONE   (0x0 << 8)
#define USBHFTDI_FRAMING_PARITY_NONE   (0x0 << 8)
#define USBHFTDI_FRAMING_PARITY_ODD    (0x1 << 8)
#define USBHFTDI_FRAMING_PARITY_EVEN   (0x2 << 8)
#define USBHFTDI_FRAMING_PARITY_MARK   (0x3 << 8)
#define USBHFTDI_FRAMING_PARITY_SPACE  (0x4 << 8)
#define USBHFTDI_FRAMING_STOP_BITS_1   (0x0 << 11)
#define USBHFTDI_FRAMING_STOP_BITS_15  (0x1 << 11)
#define USBHFTDI_FRAMING_STOP_BITS_2   (0x2 << 11)

#define USBHFTDI_HANDSHAKE_NONE 		(0x0)
#define USBHFTDI_HANDSHAKE_RTS_CTS 		(0x1)
#define USBHFTDI_HANDSHAKE_DTR_DSR 		(0x2)
#define USBHFTDI_HANDSHAKE_XON_XOFF		(0x4)



/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/
typedef struct {
  uint32_t  speed;
  uint16_t  framing;
  uint8_t   handshake;
  uint8_t   xon_character;
  uint8_t	xoff_character;
} USBHFTDIPortConfig;

typedef enum {
	USBHFTDI_TYPE_A,
	USBHFTDI_TYPE_B,
	USBHFTDI_TYPE_H,
} usbhftdi_type_t;

typedef enum {
	USBHFTDIP_STATE_UNINIT = 0,
	USBHFTDIP_STATE_STOP = 1,
	USBHFTDIP_STATE_ACTIVE = 2,
	USBHFTDIP_STATE_READY = 3
} usbhftdip_state_t;


#define _ftdi_port_driver_methods                                          \
  _base_asynchronous_channel_methods

struct FTDIPortDriverVMT {
	_ftdi_port_driver_methods
};

typedef struct USBHFTDIPortDriver USBHFTDIPortDriver;
typedef struct USBHFTDIDriver USBHFTDIDriver;

struct USBHFTDIPortDriver {
	/* inherited from abstract asyncrhonous channel driver */
	const struct FTDIPortDriverVMT *vmt;
	_base_asynchronous_channel_data

	USBHFTDIDriver *ftdip;

	usbhftdip_state_t state;

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
	uint8_t ifnum;

	USBHFTDIPortDriver *next;
};

struct USBHFTDIDriver {
	/* inherited from abstract class driver */
	_usbh_base_classdriver_data

	usbhftdi_type_t type;
	USBHFTDIPortDriver *ports;

	mutex_t mtx;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/
#define usbhftdipGetState(ftdipp) ((ftdipp)->state)


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/
extern USBHFTDIDriver USBHFTDID[HAL_USBHFTDI_MAX_INSTANCES];
extern USBHFTDIPortDriver FTDIPD[HAL_USBHFTDI_MAX_PORTS];

#ifdef __cplusplus
extern "C" {
#endif
	/* FTDI port driver */
	void usbhftdipStart(USBHFTDIPortDriver *ftdipp, const USBHFTDIPortConfig *config);
	void usbhftdipStop(USBHFTDIPortDriver *ftdipp);
#ifdef __cplusplus
}
#endif


#endif

#endif /* USBH_FTDI_H_ */
