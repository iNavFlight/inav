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

#ifndef HAL_USBH_LLD_H
#define HAL_USBH_LLD_H

#include "hal.h"

#if HAL_USE_USBH

#include "osal.h"
#include "stm32_otg.h"

/* TODO:
 *
 * - Implement ISO/INT OUT and test
 * - Consider DMA mode for OTG_HS, consider external PHY for HS.
 * - Implement a data pump thread, so we don't have to copy data from the ISR
 * 		This might be a bad idea for small endpoint packet sizes (the context switch
 * 		could be longer than the copy)
 */

typedef enum {
	USBH_LLD_CTRLPHASE_SETUP,
	USBH_LLD_CTRLPHASE_DATA,
	USBH_LLD_CTRLPHASE_STATUS
} usbh_lld_ctrlphase_t;

typedef enum {
	USBH_LLD_HALTREASON_NONE,
	USBH_LLD_HALTREASON_XFRC,
	USBH_LLD_HALTREASON_NAK,
	USBH_LLD_HALTREASON_STALL,
	USBH_LLD_HALTREASON_ERROR,
	USBH_LLD_HALTREASON_ABORT
} usbh_lld_halt_reason_t;


typedef struct stm32_hc_management {
	struct list_head node;

	stm32_otg_host_chn_t *hc;
	volatile uint32_t	*fifo;
	usbh_ep_t 			*ep;
	uint16_t			haintmsk;
	usbh_lld_halt_reason_t halt_reason;
} stm32_hc_management_t;


#define _usbhdriver_ll_data											\
	stm32_otg_t *otg;												\
	/* low-speed port reset bug */									\
	bool check_ls_activity;											\
	/* channels */													\
	uint8_t channels_number;										\
	stm32_hc_management_t channels[STM32_OTG2_CHANNELS_NUMBER];		\
	struct list_head ch_free[2];									\
	/* Enpoints being processed */									\
	struct list_head ep_active_lists[4];							\
	/* Pending endpoints */											\
	struct list_head ep_pending_lists[4];


#define _usbh_ep_ll_data																\
		struct list_head	*active_list;		/* shortcut to ep list */				\
		struct list_head	*pending_list;		/* shortcut to ep list */				\
		struct list_head	urb_list;			/* list of URBs queued in this EP */	\
		struct list_head	node;				/* this EP */							\
		uint32_t 			hcintmsk;													\
		uint32_t			hcchar;														\
		uint32_t 			dt_mask;			/* data-toggle mask */					\
		/* current transfer */															\
		struct {																		\
			stm32_hc_management_t *hcm;				/* assigned channel */				\
			uint32_t			len;				/* this transfer's total length */	\
			uint8_t				*buf;				/* this transfer's buffer */		\
			uint32_t			partial;			/* this transfer's partial length */\
			uint16_t			packets;			/* packets allocated */				\
			union {																		\
				uint32_t			frame_counter;		/* frame counter (for INT) */	\
				usbh_lld_ctrlphase_t	ctrl_phase;		/* control phase (for CTRL) */	\
			} u;																		\
			uint8_t				error_count;		/* error count */					\
		} xfer;





#define _usbh_port_ll_data		\
	uint16_t lld_c_status;		\
	uint16_t lld_status;

#define _usbh_device_ll_data

#define _usbh_hub_ll_data

#define _usbh_urb_ll_data		\
	struct list_head node;		\
	bool queued;


#define usbh_lld_urb_object_init(urb) 									\
		do {															\
			osalDbgAssert(((uint32_t)urb->buff & 3) == 0, 				\
				"use USBH_DEFINE_BUFFER() to declare the IO buffers"); 	\
				urb->queued = FALSE;									\
		} while (0)


#define usbh_lld_urb_object_reset(urb) 									\
		do {															\
			osalDbgAssert(urb->queued == FALSE, "wrong state");			\
			osalDbgAssert(((uint32_t)urb->buff & 3) == 0, 				\
				"use USBH_DEFINE_BUFFER() to declare the IO buffers"); 	\
		} while (0)

void usbh_lld_init(void);
void usbh_lld_start(USBHDriver *usbh);
void usbh_lld_ep_object_init(usbh_ep_t *ep);
void usbh_lld_ep_open(usbh_ep_t *ep);
void usbh_lld_ep_close(usbh_ep_t *ep);
bool usbh_lld_ep_reset(usbh_ep_t *ep);
void usbh_lld_urb_submit(usbh_urb_t *urb);
bool usbh_lld_urb_abort(usbh_urb_t *urb, usbh_urbstatus_t status);
usbh_urbstatus_t usbh_lld_root_hub_request(USBHDriver *usbh, uint8_t bmRequestType, uint8_t bRequest,
		uint16_t wvalue, uint16_t windex, uint16_t wlength, uint8_t *buf);
uint8_t usbh_lld_roothub_get_statuschange_bitmap(USBHDriver *usbh);

#ifdef __IAR_SYSTEMS_ICC__
#define USBH_LLD_DEFINE_BUFFER(var) _Pragma("data_alignment=4") var
#define USBH_LLD_DECLARE_STRUCT_MEMBER_H1(x, y) x ## y
#define USBH_LLD_DECLARE_STRUCT_MEMBER_H2(x, y) USBH_LLD_DECLARE_STRUCT_MEMBER_H1(x, y)
#define USBH_LLD_DECLARE_STRUCT_MEMBER(member)  unsigned int USBH_LLD_DECLARE_STRUCT_MEMBER_H2(dummy_align_, __COUNTER__); member
#else
#define USBH_LLD_DEFINE_BUFFER(var) var __attribute__((aligned(4)))
#define USBH_LLD_DECLARE_STRUCT_MEMBER(member) member __attribute__((aligned(4)))
#endif


#if STM32_USBH_USE_OTG1
extern USBHDriver USBHD1;
#endif

#if STM32_USBH_USE_OTG2
extern USBHDriver USBHD2;
#endif

#endif

#endif /* HAL_USBH_LLD_H */
