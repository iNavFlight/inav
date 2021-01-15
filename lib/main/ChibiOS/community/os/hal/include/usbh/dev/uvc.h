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

#ifndef USBH_INCLUDE_USBH_UVC_H_
#define USBH_INCLUDE_USBH_UVC_H_

#include "hal_usbh.h"

#if HAL_USE_USBH && HAL_USBH_USE_UVC

#include "usbh/desciter.h"

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/


/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/
#define USBHUVC_MAX_STATUS_PACKET_SZ	16


/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/


typedef enum {
	UVC_CS_INTERFACE 	= 0x24,
	UVC_CS_ENDPOINT 	= 0x25
} usbh_uvc_cstype_t;

typedef enum {
	UVC_CC_VIDEO 		= 0x0e
} usbh_uvc_cctype_t;

typedef enum {
	UVC_SC_UNKNOWN = 0x00,
	UVC_SC_VIDEOCONTROL,
	UVC_SC_VIDEOSTREAMING,
	UVC_SC_VIDEO_INTERFACE_COLLECTION
} usbh_uvc_sctype_t;

typedef enum {
	UVC_VC_UNDEF = 0x00,
	UVC_VC_HEADER,
	UVC_VC_INPUT_TERMINAL,
	UVC_VC_OUTPUT_TERMINAL,
	UVC_VC_SELECTOR_UNIT,
	UVC_VC_PROCESSING_UNIT,
	UVC_VC_EXTENSION_UNIT
} usbh_uvc_vctype_t;

typedef enum {
	UVC_VS_UNDEF = 0x00,
	UVC_VS_INPUT_HEADER,
	UVC_VS_OUTPUT_HEADER,
	UVC_VS_STILL_IMAGE_FRAME,
	UVC_VS_FORMAT_UNCOMPRESSED,
	UVC_VS_FRAME_UNCOMPRESSED,
	UVC_VS_FORMAT_MJPEG,
	UVC_VS_FRAME_MJPEG,
	UVC_VS_RESERVED_0,
	UVC_VS_RESERVED_1,
	UVC_VS_FORMAT_MPEG2TS,
	UVC_VS_RESERVED_2,
	UVC_VS_FORMAT_DV,
	UVC_VS_COLOR_FORMAT,
	UVC_VS_RESERVED_3,
	UVC_VS_RESERVED_4,
	UVC_VS_FORMAT_FRAME_BASED,
	UVC_VS_FRAME_FRAME_BASED,
	UVC_VS_FORMAT_STREAM_BASED
} usbh_uvc_vstype_t;

typedef enum {
	UVC_TT_VENDOR_SPECIFIC 			= 0x0100,
	UVC_TT_STREAMING 				= 0x0101,
	UVC_ITT_VENDOR_SPECIFIC 		= 0x0200,
	UVC_ITT_CAMERA 					= 0x0201,
	UVC_ITT_MEDIA_TRANSPORT_INPUT 	= 0x0202,
	UVC_OTT_VENDOR_SPECIFIC 		= 0x0300,
	UVC_OTT_DISPLAY 				= 0x0301,
	UVC_OTT_MEDIA_TRANSPORT 		= 0x0302
} usbh_uvc_tttype_t;

typedef enum {
	UVC_SET_CUR	 =	0x01,
	UVC_GET_CUR	 =	0x81,
	UVC_GET_MIN	 =	0x82,
	UVC_GET_MAX	 =	0x83,
	UVC_GET_RES  =	0x84,
	UVC_GET_LEN  = 	0x85,
	UVC_GET_INFO =	0x86,
	UVC_GET_DEF  =	0x87
} usbh_uvc_ctrlops_t;

typedef enum {
	UVC_CTRL_VC_CONTROL_UNDEFINED  = 0x00,
	UVC_CTRL_VC_VIDEO_POWER_MODE_CONTROL = 0x01,
	UVC_CTRL_VC_REQUEST_ERROR_CODE_CONTROL = 0x02,
} usbh_uvc_ctrl_vc_interface_controls_t;

typedef enum {
	UVC_CTRL_SU_CONTROL_UNDEFINED = 0x00,
	UVC_CTRL_SU_INPUT_SELECT_CONTROL = 0x01,
} usbh_uvc_ctrl_vc_selectorunit_controls_t;

typedef enum {
	UVC_CTRL_CT_CONTROL_UNDEFINED = 0x00,
	UVC_CTRL_CT_SCANNING_MODE_CONTROL = 0x01,
	UVC_CTRL_CT_AE_MODE_CONTROL = 0x02,
	UVC_CTRL_CT_AE_PRIORITY_CONTROL = 0x03,
	UVC_CTRL_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL = 0x04,
	UVC_CTRL_CT_EXPOSURE_TIME_RELATIVE_CONTROL = 0x05,
	UVC_CTRL_CT_FOCUS_ABSOLUTE_CONTROL = 0x06,
	UVC_CTRL_CT_FOCUS_RELATIVE_CONTROL = 0x07,
	UVC_CTRL_CT_FOCUS_AUTO_CONTROL = 0x08,
	UVC_CTRL_CT_IRIS_ABSOLUTE_CONTROL = 0x09,
	UVC_CTRL_CT_IRIS_RELATIVE_CONTROL = 0x0A,
	UVC_CTRL_CT_ZOOM_ABSOLUTE_CONTROL = 0x0B,
	UVC_CTRL_CT_ZOOM_RELATIVE_CONTROL = 0x0C,
	UVC_CTRL_CT_PANTILT_ABSOLUTE_CONTROL = 0x0D,
	UVC_CTRL_CT_PANTILT_RELATIVE_CONTROL = 0x0E,
	UVC_CTRL_CT_ROLL_ABSOLUTE_CONTROL = 0x0F,
	UVC_CTRL_CT_ROLL_RELATIVE_CONTROL = 0x10,
	UVC_CTRL_CT_PRIVACY_CONTROL = 0x11
} usbh_uvc_ctrl_vc_cameraterminal_controls_t;

typedef enum {
	UVC_CTRL_PU_CONTROL_UNDEFINED = 0x00,
	UVC_CTRL_PU_BACKLIGHT_COMPENSATION_CONTROL = 0x01,
	UVC_CTRL_PU_BRIGHTNESS_CONTROL = 0x02,
	UVC_CTRL_PU_CONTRAST_CONTROL = 0x03,
	UVC_CTRL_PU_GAIN_CONTROL = 0x04,
	UVC_CTRL_PU_POWER_LINE_FREQUENCY_CONTROL = 0x05,
	UVC_CTRL_PU_HUE_CONTROL = 0x06,
	UVC_CTRL_PU_SATURATION_CONTROL = 0x07,
	UVC_CTRL_PU_SHARPNESS_CONTROL = 0x08,
	UVC_CTRL_PU_GAMMA_CONTROL = 0x09,
	UVC_CTRL_PU_WHITE_BALANCE_TEMPERATURE_CONTROL = 0x0A,
	UVC_CTRL_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL = 0x0B,
	UVC_CTRL_PU_WHITE_BALANCE_COMPONENT_CONTROL = 0x0C,
	UVC_CTRL_PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL = 0x0D,
	UVC_CTRL_PU_DIGITAL_MULTIPLIER_CONTROL = 0x0E,
	UVC_CTRL_PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL = 0x0F,
	UVC_CTRL_PU_HUE_AUTO_CONTROL = 0x10,
	UVC_CTRL_PU_ANALOG_VIDEO_STANDARD_CONTROL = 0x11,
	UVC_CTRL_PU_ANALOG_LOCK_STATUS_CONTROL = 0x12,
} usbh_uvc_ctrl_vc_processingunit_controls_t;

typedef enum {
	UVC_CTRL_VS_CONTROL_UNDEFINED = 0x00,
	UVC_CTRL_VS_PROBE_CONTROL = 0x01,
	UVC_CTRL_VS_COMMIT_CONTROL = 0x02,
	UVC_CTRL_VS_STILL_PROBE_CONTROL = 0x03,
	UVC_CTRL_VS_STILL_COMMIT_CONTROL = 0x04,
	UVC_CTRL_VS_STILL_IMAGE_TRIGGER_CONTROL = 0x05,
	UVC_CTRL_VS_STREAM_ERROR_CODE_CONTROL = 0x06,
	UVC_CTRL_VS_GENERATE_KEY_FRAME_CONTROL = 0x07,
	UVC_CTRL_VS_UPDATE_FRAME_SEGMENT_CONTROL = 0x08,
	UVC_CTRL_VS_SYNCH_DELAY_CONTROL = 0x09
} usbh_uvc_ctrl_vs_interface_controls_t;


typedef PACKED_STRUCT {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bFormatIndex;
	uint8_t bNumFrameDescriptors;
	uint8_t bmFlags;
	uint8_t bDefaultFrameIndex;
	uint8_t bAspectRatioX;
	uint8_t bAspectRatioY;
	uint8_t bmInterfaceFlags;
	uint8_t bCopyProtect;
} usbh_uvc_format_mjpeg_t;

typedef PACKED_STRUCT {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bDescriptorSubType;
	uint8_t  bFrameIndex;
	uint8_t  bmCapabilities;
	uint16_t wWidth;
	uint16_t wHeight;
	uint32_t dwMinBitRate;
	uint32_t dwMaxBitRate;
	uint32_t dwMaxVideoFrameBufferSize;
	uint32_t dwDefaultFrameInterval;
	uint8_t bFrameIntervalType;
	uint32_t dwFrameInterval[0];
} usbh_uvc_frame_mjpeg_t;


typedef PACKED_STRUCT {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bFrameIndex;
	uint8_t bmCapabilities;
	uint16_t wWidth;
	uint16_t wHeight;
	uint32_t dwMinBitRate;
	uint32_t dwMaxBitRate;
	uint32_t dwMaxVideoFrameBufferSize;
	uint32_t dwDefaultFrameInterval;
	uint8_t bFrameIntervalType;
	uint32_t dwFrameInterval[0];
} usbh_uvc_frame_uncompressed_t;

typedef PACKED_STRUCT {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bDescriptorSubType;
	uint8_t  bFormatIndex;
	uint8_t  bNumFrameDescriptors;
	uint8_t  guidFormat[16];
	uint8_t  bBitsPerPixel;
	uint8_t  bDefaultFrameIndex;
	uint8_t  bAspectRatioX;
	uint8_t  bAspectRatioY;
	uint8_t  bmInterfaceFlags;
	uint8_t  bCopyProtect;
} usbh_uvc_format_uncompressed;

typedef PACKED_STRUCT {
    uint16_t bmHint;
    uint8_t bFormatIndex;
    uint8_t bFrameIndex;
    uint32_t dwFrameInterval;
    uint16_t wKeyFrameRate;
    uint16_t wPFrameRate;
    uint16_t wCompQuality;
    uint16_t wCompWindowSize;
    uint16_t wDelay;
    uint32_t dwMaxVideoFrameSize;
    uint32_t dwMaxPayloadTransferSize;
//    uint32_t dwClockFrequency;
//    uint8_t bmFramingInfo;
//    uint8_t bPreferedVersion;
//    uint8_t bMinVersion;
//    uint8_t bMaxVersion;
} usbh_uvc_ctrl_vs_probecommit_data_t;



/* D0: Frame ID.
 * 	For frame-based formats, this bit toggles between 0 and 1 every time a new video frame begins.
 * 	For stream-based formats, this bit toggles between 0 and 1 at the start of each new codec-specific
 * 	segment. This behavior is required for frame-based payload formats (e.g., DV) and is optional
 * 	for stream-based payload formats (e.g., MPEG-2 TS). For stream-based formats, support for this
 * 	bit must be indicated via the bmFramingInfofield of the Video Probe and Commitcontrols
 * 	(see section 4.3.1.1, “Video Probe and Commit Controls”).
 *
 * D1: End of Frame.
 *  This bit is set if the following payload data marks the end of the current video or still image
 *  frame (for framebased formats), or to indicate the end of a codec-specific segment
 *  (for stream-based formats). This behavior is optional for all payload formats.
 *  For stream-based formats, support for this bit must be indicated via the bmFramingInfofield
 *  of the Video Probe and CommitControls (see section 4.3.1.1, “Video Probe and Commit Controls”).
 *
 * D2: Presentation Time.
 *  This bit is set if the dwPresentationTimefield is being sent as part of the header.
 *
 * D3: Source Clock Reference
 *  This bit is set if the dwSourceClockfield is being sent as part of the header.
 *
 * D4: Reserved
 *
 * D5: Still Image
 *  This bit is set ifthe following data is part of a still image frame, and is only used for
 *  methods 2 and 3 of still image capture.
 *
 * D6: Error
 *  This bit is set ifthere was an error in the video or still image transmission
 *  for this payload. The StreamError Code control would reflect the cause of the error.
 *
 * D7: End of header
 *  This bit is set if this is the last header group in the packet, where the
 *  header group refers to this field and any optional fields identified by the bits in this
 *  field (Defined for future extension)
*/

#define UVC_HDR_EOH				(1 << 7)	/* End of header */
#define UVC_HDR_ERR				(1 << 6)	/* Error */
#define UVC_HDR_STILL			(1 << 5)	/* Still Image */
#define UVC_HDR_SCR				(1 << 3)	/* Source Clock Reference */
#define UVC_HDR_PT				(1 << 2)	/* Presentation Time */
#define UVC_HDR_EOF				(1 << 1)	/* End of Frame */
#define UVC_HDR_FID				(1 << 0)	/* Frame ID */



typedef struct USBHUVCDriver USBHUVCDriver;

#define USBHUVC_MESSAGETYPE_STATUS	1
#define USBHUVC_MESSAGETYPE_DATA	2


#define _usbhuvc_message_base_data				\
		uint16_t type;							\
		uint16_t length;						\
		systime_t timestamp;

typedef struct {
	_usbhuvc_message_base_data
} usbhuvc_message_base_t;

typedef struct {
	_usbhuvc_message_base_data
	USBH_DECLARE_STRUCT_MEMBER(uint8_t data[0]);
} usbhuvc_message_data_t;

typedef struct {
	_usbhuvc_message_base_data
	USBH_DECLARE_STRUCT_MEMBER(uint8_t data[USBHUVC_MAX_STATUS_PACKET_SZ]);
} usbhuvc_message_status_t;


typedef enum {
	USBHUVC_STATE_UNINITIALIZED = 0,	//must call usbhuvcObjectInit
	USBHUVC_STATE_STOP	 		= 1,	//the device is disconnected
	USBHUVC_STATE_ACTIVE 		= 2,	//the device is connected
	USBHUVC_STATE_READY		 	= 3,	//the device has negotiated the parameters
	USBHUVC_STATE_STREAMING 	= 4,	//the device is streaming data
	USBHUVC_STATE_BUSY	 		= 5		//the driver is busy performing some action
} usbhuvc_state_t;


struct USBHUVCDriver {
	/* inherited from abstract class driver */
	_usbh_base_classdriver_data

	usbhuvc_state_t state;

	usbh_ep_t ep_int;
	usbh_ep_t ep_iso;

	usbh_urb_t urb_iso;
	usbh_urb_t urb_int;

	if_iterator_t ivc;
	if_iterator_t ivs;

	USBH_DECLARE_STRUCT_MEMBER(usbh_uvc_ctrl_vs_probecommit_data_t pc);
	USBH_DECLARE_STRUCT_MEMBER(usbh_uvc_ctrl_vs_probecommit_data_t pc_min);
	USBH_DECLARE_STRUCT_MEMBER(usbh_uvc_ctrl_vs_probecommit_data_t pc_max);

	mailbox_t mb;
	msg_t mb_buff[HAL_USBHUVC_MAX_MAILBOX_SZ];

	memory_pool_t mp_data;
	void *mp_data_buffer;

	memory_pool_t mp_status;
	usbhuvc_message_status_t mp_status_buffer[HAL_USBHUVC_STATUS_PACKETS_COUNT];

	mutex_t mtx;
};


/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern USBHUVCDriver USBHUVCD[HAL_USBHUVC_MAX_INSTANCES];

#ifdef __cplusplus
extern "C" {
#endif
	static inline usbhuvc_state_t usbhuvcGetState(USBHUVCDriver *uvcd) {
		return uvcd->state;
	}

	bool usbhuvcVCRequest(USBHUVCDriver *uvcdp,
			uint8_t bRequest, uint8_t entity, uint8_t control,
			uint16_t wLength, uint8_t *data);
	bool usbhuvcVSRequest(USBHUVCDriver *uvcdp,
			uint8_t bRequest, uint8_t control,
			uint16_t wLength, uint8_t *data);
	bool usbhuvcFindVSDescriptor(USBHUVCDriver *uvcdp,
			generic_iterator_t *ics,
			uint8_t bDescriptorSubtype,
			bool start);
	uint32_t usbhuvcEstimateRequiredEPSize(USBHUVCDriver *uvcdp, const uint8_t *formatdesc,
			const uint8_t *framedesc, uint32_t dwFrameInterval);

#if	USBH_DEBUG_ENABLE && USBHUVC_DEBUG_ENABLE_INFO
	void usbhuvcPrintProbeCommit(const usbh_uvc_ctrl_vs_probecommit_data_t *pc);
#else
#	define usbhuvcPrintProbeCommit(pc) do {} while(0)
#endif
	bool usbhuvcProbe(USBHUVCDriver *uvcdp);
	bool usbhuvcCommit(USBHUVCDriver *uvcdp);
	void usbhuvcResetPC(USBHUVCDriver *uvcdp);
	static inline const usbh_uvc_ctrl_vs_probecommit_data_t *usbhuvcGetPCMin(USBHUVCDriver *uvcdp) {
		return &uvcdp->pc_min;
	}
	static inline const usbh_uvc_ctrl_vs_probecommit_data_t *usbhuvcGetPCMax(USBHUVCDriver *uvcdp) {
		return &uvcdp->pc_min;
	}
	static inline usbh_uvc_ctrl_vs_probecommit_data_t *usbhuvcGetPC(USBHUVCDriver *uvcdp) {
		return &uvcdp->pc;
	}

	bool usbhuvcStreamStart(USBHUVCDriver *uvcdp, uint16_t min_ep_sz);
	bool usbhuvcStreamStop(USBHUVCDriver *uvcdp);

	static inline msg_t usbhuvcLockAndFetchS(USBHUVCDriver *uvcdp, msg_t *msg, systime_t timeout) {
		chMtxLockS(&uvcdp->mtx);
		msg_t ret = chMBFetchTimeoutS(&uvcdp->mb, msg, timeout);
		if (ret != MSG_OK)
			chMtxUnlockS(&uvcdp->mtx);
		return ret;
	}
	static inline msg_t usbhuvcLockAndFetch(USBHUVCDriver *uvcdp, msg_t *msg, systime_t timeout) {
		osalSysLock();
		msg_t ret = usbhuvcLockAndFetchS(uvcdp, msg, timeout);
		osalSysUnlock();
		return ret;
	}
	static inline void usbhuvcUnlock(USBHUVCDriver *uvcdp) {
		chMtxUnlock(&uvcdp->mtx);
	}
	static inline void usbhuvcFreeDataMessage(USBHUVCDriver *uvcdp, usbhuvc_message_data_t *msg) {
		chPoolFree(&uvcdp->mp_data, msg);
	}
	static inline void usbhuvcFreeStatusMessage(USBHUVCDriver *uvcdp, usbhuvc_message_status_t *msg) {
		chPoolFree(&uvcdp->mp_status, msg);
	}
#ifdef __cplusplus
}
#endif

#endif

#endif /* USBH_INCLUDE_USBH_UVC_H_ */
