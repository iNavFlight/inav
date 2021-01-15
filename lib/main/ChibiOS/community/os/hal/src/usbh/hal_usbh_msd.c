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

#if HAL_USBH_USE_MSD

#if !HAL_USE_USBH
#error "USBHMSD needs USBH"
#endif

#include <string.h>
#include "usbh/dev/msd.h"
#include "usbh/internal.h"

#if USBHMSD_DEBUG_ENABLE_TRACE
#define udbgf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define udbg(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define udbgf(f, ...)  do {} while(0)
#define udbg(f, ...)   do {} while(0)
#endif

#if USBHMSD_DEBUG_ENABLE_INFO
#define uinfof(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uinfo(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uinfof(f, ...)  do {} while(0)
#define uinfo(f, ...)   do {} while(0)
#endif

#if USBHMSD_DEBUG_ENABLE_WARNINGS
#define uwarnf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uwarn(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uwarnf(f, ...)  do {} while(0)
#define uwarn(f, ...)   do {} while(0)
#endif

#if USBHMSD_DEBUG_ENABLE_ERRORS
#define uerrf(f, ...)  usbDbgPrintf(f, ##__VA_ARGS__)
#define uerr(f, ...)  usbDbgPuts(f, ##__VA_ARGS__)
#else
#define uerrf(f, ...)  do {} while(0)
#define uerr(f, ...)   do {} while(0)
#endif

static void _lun_object_deinit(USBHMassStorageLUNDriver *lunp);

/*===========================================================================*/
/* USB Class driver loader for MSD                                           */
/*===========================================================================*/

struct USBHMassStorageDriver {
	/* inherited from abstract class driver */
	_usbh_base_classdriver_data

	usbh_ep_t epin;
	usbh_ep_t epout;
	uint8_t ifnum;
	uint8_t max_lun;
	uint32_t tag;

	USBHMassStorageLUNDriver *luns;
};

static USBHMassStorageDriver USBHMSD[HAL_USBHMSD_MAX_INSTANCES];

static void _msd_init(void);
static usbh_baseclassdriver_t *_msd_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem);
static void _msd_unload(usbh_baseclassdriver_t *drv);

static const usbh_classdriver_vmt_t class_driver_vmt = {
	_msd_init,
	_msd_load,
	_msd_unload
};

const usbh_classdriverinfo_t usbhmsdClassDriverInfo = {
	"MSD", &class_driver_vmt
};

#define MSD_REQ_RESET							0xFF
#define MSD_GET_MAX_LUN							0xFE

static usbh_baseclassdriver_t *_msd_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem) {
	int i;
	USBHMassStorageDriver *msdp;
	uint8_t luns;
	usbh_urbstatus_t stat;

	if (_usbh_match_descriptor(descriptor, rem, USBH_DT_INTERFACE,
			0x08, 0x06, 0x50) != HAL_SUCCESS)
		return NULL;

	const usbh_interface_descriptor_t * const ifdesc = (const usbh_interface_descriptor_t *)descriptor;

	if ((ifdesc->bAlternateSetting != 0)
			|| (ifdesc->bNumEndpoints < 2)) {
		return NULL;
	}

	/* alloc driver */
	for (i = 0; i < HAL_USBHMSD_MAX_INSTANCES; i++) {
		if (USBHMSD[i].dev == NULL) {
			msdp = &USBHMSD[i];
			goto alloc_ok;
		}
	}

	uwarn("Can't alloc MSD driver");

	/* can't alloc */
	return NULL;

alloc_ok:
	/* initialize the driver's variables */
	msdp->epin.status = USBH_EPSTATUS_UNINITIALIZED;
	msdp->epout.status = USBH_EPSTATUS_UNINITIALIZED;
	msdp->max_lun = 0;
	msdp->tag = 0;
	msdp->luns = 0;
	msdp->ifnum = ifdesc->bInterfaceNumber;
	usbhEPSetName(&dev->ctrl, "MSD[CTRL]");

	/* parse the configuration descriptor */
	if_iterator_t iif;
	generic_iterator_t iep;
	iif.iad = 0;
	iif.curr = descriptor;
	iif.rem = rem;
	for (ep_iter_init(&iep, &iif); iep.valid; ep_iter_next(&iep)) {
		const usbh_endpoint_descriptor_t *const epdesc = ep_get(&iep);
		if ((epdesc->bEndpointAddress & 0x80) && (epdesc->bmAttributes == USBH_EPTYPE_BULK)) {
			uinfof("BULK IN endpoint found: bEndpointAddress=%02x", epdesc->bEndpointAddress);
			usbhEPObjectInit(&msdp->epin, dev, epdesc);
			usbhEPSetName(&msdp->epin, "MSD[BIN ]");
		} else if (((epdesc->bEndpointAddress & 0x80) == 0)
				&& (epdesc->bmAttributes == USBH_EPTYPE_BULK)) {
			uinfof("BULK OUT endpoint found: bEndpointAddress=%02x", epdesc->bEndpointAddress);
			usbhEPObjectInit(&msdp->epout, dev, epdesc);
			usbhEPSetName(&msdp->epout, "MSD[BOUT]");
		} else {
			uinfof("unsupported endpoint found: bEndpointAddress=%02x, bmAttributes=%02x",
					epdesc->bEndpointAddress, epdesc->bmAttributes);
		}
	}
	if ((msdp->epin.status != USBH_EPSTATUS_CLOSED) || (msdp->epout.status != USBH_EPSTATUS_CLOSED)) {
		goto deinit;
	}

	/* read the number of LUNs */
	uinfo("Reading Max LUN:");
	USBH_DEFINE_BUFFER(uint8_t buff[4]);
	stat = usbhControlRequest(dev,
			USBH_REQTYPE_CLASSIN(USBH_REQTYPE_RECIP_INTERFACE),
			MSD_GET_MAX_LUN, 0, msdp->ifnum, 1, buff);
	if (stat == USBH_URBSTATUS_OK) {
		msdp->max_lun = buff[0] + 1;
		uinfof("\tmax_lun = %d", msdp->max_lun);
		if (msdp->max_lun > HAL_USBHMSD_MAX_LUNS) {
			msdp->max_lun = HAL_USBHMSD_MAX_LUNS;
			uwarnf("\tUsing max_lun = %d", msdp->max_lun);
		}
	} else if (stat == USBH_URBSTATUS_STALL) {
		uwarn("\tStall, max_lun = 1");
		msdp->max_lun = 1;
	} else {
		uerr("\tError");
		goto deinit;
	}

	/* open the bulk IN/OUT endpoints */
	usbhEPOpen(&msdp->epin);
	usbhEPOpen(&msdp->epout);

	/* Alloc one block device per logical unit found */
	luns = msdp->max_lun;
	for (i = 0; (luns > 0) && (i < HAL_USBHMSD_MAX_LUNS); i++) {
		if (MSBLKD[i].msdp == NULL) {
			/* link the new block driver to the list */
			MSBLKD[i].next = msdp->luns;
			msdp->luns = &MSBLKD[i];
			MSBLKD[i].msdp = msdp;
			MSBLKD[i].state = BLK_ACTIVE;
			luns--;
		}
	}

	return (usbh_baseclassdriver_t *)msdp;

deinit:
	/* Here, the enpoints are closed, and the driver is unlinked */
	return NULL;
}

static void _msd_unload(usbh_baseclassdriver_t *drv) {
	osalDbgCheck(drv != NULL);
	USBHMassStorageDriver *const msdp = (USBHMassStorageDriver *)drv;
	USBHMassStorageLUNDriver *lunp = msdp->luns;

	/* disconnect all LUNs */
	while (lunp) {
		usbhmsdLUNDisconnect(lunp);
		_lun_object_deinit(lunp);
		lunp = lunp->next;
	}

	usbhEPClose(&msdp->epin);
	usbhEPClose(&msdp->epout);
}


/*===========================================================================*/
/* MSD Class driver operations (Bulk-Only transport)                         */
/*===========================================================================*/

/* USB Bulk Only Transport SCSI Command block wrapper */
typedef PACKED_STRUCT {
	uint32_t dCBWSignature;
	uint32_t dCBWTag;
	uint32_t dCBWDataTransferLength;
	uint8_t bmCBWFlags;
	uint8_t bCBWLUN;
	uint8_t bCBWCBLength;
	uint8_t CBWCB[16];
} msd_cbw_t;
#define MSD_CBW_SIGNATURE						0x43425355
#define MSD_CBWFLAGS_D2H						0x80
#define MSD_CBWFLAGS_H2D						0x00

/* USB Bulk Only Transport SCSI Command status wrapper */
typedef PACKED_STRUCT {
	uint32_t dCSWSignature;
	uint32_t dCSWTag;
	uint32_t dCSWDataResidue;
	uint8_t bCSWStatus;
} msd_csw_t;
#define MSD_CSW_SIGNATURE						0x53425355

typedef struct {
	msd_cbw_t *cbw;
	uint8_t csw_status;
	uint32_t data_processed;
} msd_transaction_t;

typedef enum {
	MSD_BOTRESULT_OK,
	MSD_BOTRESULT_DISCONNECTED,
	MSD_BOTRESULT_ERROR
} msd_bot_result_t;

typedef enum {
	MSD_RESULT_OK = MSD_BOTRESULT_OK,
	MSD_RESULT_DISCONNECTED = MSD_BOTRESULT_DISCONNECTED,
	MSD_RESULT_TRANSPORT_ERROR = MSD_BOTRESULT_ERROR,
	MSD_RESULT_FAILED
} msd_result_t;


#define	CSW_STATUS_PASSED		0
#define	CSW_STATUS_FAILED		1
#define	CSW_STATUS_PHASE_ERROR	2

static bool _msd_bot_reset(USBHMassStorageDriver *msdp) {

	usbh_urbstatus_t res;
	res = usbhControlRequest(msdp->dev,
			USBH_REQTYPE_CLASSOUT(USBH_REQTYPE_RECIP_INTERFACE),
			0xFF, 0, msdp->ifnum, 0, NULL);
	if (res != USBH_URBSTATUS_OK) {
		return FALSE;
	}

	osalThreadSleepMilliseconds(100);

	return usbhEPReset(&msdp->epin) && usbhEPReset(&msdp->epout);
}

static msd_bot_result_t _msd_bot_transaction(msd_transaction_t *tran, USBHMassStorageLUNDriver *lunp, void *data) {

	uint32_t data_actual_len, actual_len;
	usbh_urbstatus_t status;
	USBH_DEFINE_BUFFER(msd_csw_t csw);

	tran->cbw->bCBWLUN = (uint8_t)(lunp - &lunp->msdp->luns[0]);
	tran->cbw->dCBWSignature = MSD_CBW_SIGNATURE;
	tran->cbw->dCBWTag = ++lunp->msdp->tag;
	tran->data_processed = 0;

	/* control phase */
	status = usbhBulkTransfer(&lunp->msdp->epout, tran->cbw,
					sizeof(*tran->cbw), &actual_len, OSAL_MS2I(1000));

	if (status == USBH_URBSTATUS_CANCELLED) {
		uerr("\tMSD: Control phase: USBH_URBSTATUS_CANCELLED");
		return MSD_BOTRESULT_DISCONNECTED;
	}

	if ((status != USBH_URBSTATUS_OK) || (actual_len != sizeof(*tran->cbw))) {
		uerrf("\tMSD: Control phase: status = %d (!= OK), actual_len = %d (expected to send %d)",
				status, actual_len, sizeof(*tran->cbw));
		_msd_bot_reset(lunp->msdp);
		return MSD_BOTRESULT_ERROR;
	}


	/* data phase */
	data_actual_len = 0;
	if (tran->cbw->dCBWDataTransferLength) {
		usbh_ep_t *const ep = tran->cbw->bmCBWFlags & MSD_CBWFLAGS_D2H ? &lunp->msdp->epin : &lunp->msdp->epout;
		status = usbhBulkTransfer(
				ep,
				data,
				tran->cbw->dCBWDataTransferLength,
				&data_actual_len, OSAL_MS2I(20000));

		if (status == USBH_URBSTATUS_CANCELLED) {
			uerr("\tMSD: Data phase: USBH_URBSTATUS_CANCELLED");
			return MSD_BOTRESULT_DISCONNECTED;
		}

		if (status == USBH_URBSTATUS_STALL) {
			uerrf("\tMSD: Data phase: USBH_URBSTATUS_STALL, clear halt");
			status = (usbhEPReset(ep) == HAL_SUCCESS) ? USBH_URBSTATUS_OK : USBH_URBSTATUS_ERROR;
		}

		if (status != USBH_URBSTATUS_OK) {
			uerrf("\tMSD: Data phase: status = %d (!= OK), resetting", status);
			_msd_bot_reset(lunp->msdp);
			return MSD_BOTRESULT_ERROR;
		}
	}


	/* status phase */
	status = usbhBulkTransfer(&lunp->msdp->epin, &csw,
				sizeof(csw), &actual_len, OSAL_MS2I(1000));

	if (status == USBH_URBSTATUS_STALL) {
		uwarn("\tMSD: Status phase: USBH_URBSTATUS_STALL, clear halt and retry");

		status = (usbhEPReset(&lunp->msdp->epin) == HAL_SUCCESS) ? USBH_URBSTATUS_OK : USBH_URBSTATUS_ERROR;

		if (status == USBH_URBSTATUS_OK) {
			status = usbhBulkTransfer(&lunp->msdp->epin, &csw,
						sizeof(csw), &actual_len, OSAL_MS2I(1000));
		}
	}

	if (status == USBH_URBSTATUS_CANCELLED) {
		uerr("\tMSD: Status phase: USBH_URBSTATUS_CANCELLED");
		return MSD_BOTRESULT_DISCONNECTED;
	}

	if (status != USBH_URBSTATUS_OK) {
		uerrf("\tMSD: Status phase: status = %d (!= OK), resetting", status);
		_msd_bot_reset(lunp->msdp);
		return MSD_BOTRESULT_ERROR;
	}

	/* validate CSW */
	if ((actual_len != sizeof(csw))
		|| (csw.dCSWSignature != MSD_CSW_SIGNATURE)
		|| (csw.dCSWTag != lunp->msdp->tag)
		|| (csw.bCSWStatus >= CSW_STATUS_PHASE_ERROR)) {
		/* CSW is not valid */
		uerrf("\tMSD: Status phase: Invalid CSW: len=%d, dCSWSignature=%x, dCSWTag=%x (expected %x), bCSWStatus=%d, resetting",
				actual_len,
				csw.dCSWSignature,
				csw.dCSWTag,
				lunp->msdp->tag,
				csw.bCSWStatus);
		_msd_bot_reset(lunp->msdp);
		return MSD_BOTRESULT_ERROR;
	}

	/* check if CSW is meaningful */
	if ((csw.bCSWStatus != CSW_STATUS_PHASE_ERROR)
			&& (csw.dCSWDataResidue > tran->cbw->dCBWDataTransferLength)) {
		/* CSW is not meaningful */
		uerrf("\tMSD: Status phase: CSW not meaningful: bCSWStatus=%d, dCSWDataResidue=%u, dCBWDataTransferLength=%u, resetting",
				csw.bCSWStatus,
				csw.dCSWDataResidue,
				tran->cbw->dCBWDataTransferLength);
		_msd_bot_reset(lunp->msdp);
		return MSD_BOTRESULT_ERROR;
	}

	if (csw.bCSWStatus == CSW_STATUS_PHASE_ERROR) {
		uerr("\tMSD: Status phase: Phase error, resetting");
		_msd_bot_reset(lunp->msdp);
		return MSD_BOTRESULT_ERROR;
	}

	tran->data_processed = tran->cbw->dCBWDataTransferLength - csw.dCSWDataResidue;
	if (data_actual_len < tran->data_processed) {
		tran->data_processed = data_actual_len;
	}

	tran->csw_status = csw.bCSWStatus;

	return MSD_BOTRESULT_OK;
}


/* ----------------------------------------------------- */
/* SCSI Commands                                         */
/* ----------------------------------------------------- */

/* Read 10 and Write 10 */
#define SCSI_CMD_READ_10 						0x28
#define SCSI_CMD_WRITE_10						0x2A

/* Request sense */
#define SCSI_CMD_REQUEST_SENSE 					0x03
typedef PACKED_STRUCT {
	uint8_t byte[18];
} scsi_sense_response_t;

#define SCSI_SENSE_KEY_GOOD                     0x00
#define SCSI_SENSE_KEY_RECOVERED_ERROR          0x01
#define SCSI_SENSE_KEY_NOT_READY                0x02
#define SCSI_SENSE_KEY_MEDIUM_ERROR             0x03
#define SCSI_SENSE_KEY_HARDWARE_ERROR           0x04
#define SCSI_SENSE_KEY_ILLEGAL_REQUEST          0x05
#define SCSI_SENSE_KEY_UNIT_ATTENTION           0x06
#define SCSI_SENSE_KEY_DATA_PROTECT             0x07
#define SCSI_SENSE_KEY_BLANK_CHECK              0x08
#define SCSI_SENSE_KEY_VENDOR_SPECIFIC          0x09
#define SCSI_SENSE_KEY_COPY_ABORTED             0x0A
#define SCSI_SENSE_KEY_ABORTED_COMMAND          0x0B
#define SCSI_SENSE_KEY_VOLUME_OVERFLOW          0x0D
#define SCSI_SENSE_KEY_MISCOMPARE               0x0E
#define SCSI_ASENSE_NO_ADDITIONAL_INFORMATION   0x00
#define SCSI_ASENSE_LOGICAL_UNIT_NOT_READY      0x04
#define SCSI_ASENSE_INVALID_FIELD_IN_CDB        0x24
#define SCSI_ASENSE_NOT_READY_TO_READY_CHANGE   0x28
#define SCSI_ASENSE_WRITE_PROTECTED             0x27
#define SCSI_ASENSE_FORMAT_ERROR                0x31
#define SCSI_ASENSE_INVALID_COMMAND             0x20
#define SCSI_ASENSE_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE 0x21
#define SCSI_ASENSE_MEDIUM_NOT_PRESENT                 0x3A
#define SCSI_ASENSEQ_NO_QUALIFIER                      0x00
#define SCSI_ASENSEQ_FORMAT_COMMAND_FAILED             0x01
#define SCSI_ASENSEQ_INITIALIZING_COMMAND_REQUIRED     0x02
#define SCSI_ASENSEQ_OPERATION_IN_PROGRESS             0x07

/* Inquiry */
#define SCSI_CMD_INQUIRY 						0x12
typedef PACKED_STRUCT {
	uint8_t peripheral;
	uint8_t removable;
	uint8_t version;
	uint8_t response_data_format;
	uint8_t additional_length;
	uint8_t sccstp;
	uint8_t bqueetc;
	uint8_t cmdque;
	uint8_t vendorID[8];
	uint8_t productID[16];
	uint8_t productRev[4];
} scsi_inquiry_response_t;

/* Read Capacity 10 */
#define SCSI_CMD_READ_CAPACITY_10				0x25
typedef PACKED_STRUCT {
	uint32_t last_block_addr;
	uint32_t block_size;
} scsi_readcapacity10_response_t;

/* Start/Stop Unit */
#define SCSI_CMD_START_STOP_UNIT				0x1B
typedef PACKED_STRUCT {
	uint8_t op_code;
	uint8_t lun_immed;
	uint8_t res1;
	uint8_t res2;
	uint8_t loej_start;
	uint8_t control;
} scsi_startstopunit_request_t;

/* test unit ready */
#define SCSI_CMD_TEST_UNIT_READY				0x00

static msd_result_t scsi_requestsense(USBHMassStorageLUNDriver *lunp, scsi_sense_response_t *resp);

static msd_result_t _scsi_perform_transaction(USBHMassStorageLUNDriver *lunp,
		msd_transaction_t *transaction, void *data) {

	msd_bot_result_t res;
	res = _msd_bot_transaction(transaction, lunp, data);
	if (res != MSD_BOTRESULT_OK) {
		return (msd_result_t)res;
	}

	if (transaction->csw_status == CSW_STATUS_FAILED) {
		if (transaction->cbw->CBWCB[0] != SCSI_CMD_REQUEST_SENSE) {
			/* do auto-sense (except for SCSI_CMD_REQUEST_SENSE!) */
			uwarn("\tMSD: Command failed, auto-sense");
			USBH_DEFINE_BUFFER(scsi_sense_response_t sense);
			if (scsi_requestsense(lunp, &sense) == MSD_RESULT_OK) {
				uwarnf("\tMSD: REQUEST SENSE: Sense key=%x, ASC=%02x, ASCQ=%02x",
						sense.byte[2] & 0xf, sense.byte[12], sense.byte[13]);

				return MSD_RESULT_OK;
			}
		}
		return MSD_RESULT_FAILED;
	}

	return MSD_RESULT_OK;
}

static msd_result_t scsi_inquiry(USBHMassStorageLUNDriver *lunp, scsi_inquiry_response_t *resp) {
	USBH_DEFINE_BUFFER(msd_cbw_t cbw);
	msd_transaction_t transaction;
	msd_result_t res;

	memset(cbw.CBWCB, 0, sizeof(cbw.CBWCB));
	cbw.dCBWDataTransferLength = sizeof(scsi_inquiry_response_t);
	cbw.bmCBWFlags = MSD_CBWFLAGS_D2H;
	cbw.bCBWCBLength = 6;
	cbw.CBWCB[0] = SCSI_CMD_INQUIRY;
	cbw.CBWCB[4] = sizeof(scsi_inquiry_response_t);
	transaction.cbw = &cbw;

	res = _scsi_perform_transaction(lunp, &transaction, resp);
	if (res == MSD_RESULT_OK) {
		//transaction is OK; check length
		if (transaction.data_processed < cbw.dCBWDataTransferLength) {
			res = MSD_RESULT_TRANSPORT_ERROR;
		}
	}

	return res;
}

static msd_result_t scsi_requestsense(USBHMassStorageLUNDriver *lunp, scsi_sense_response_t *resp) {
	USBH_DEFINE_BUFFER(msd_cbw_t cbw);
	msd_transaction_t transaction;
	msd_result_t res;

	memset(cbw.CBWCB, 0, sizeof(cbw.CBWCB));
	cbw.dCBWDataTransferLength = sizeof(scsi_sense_response_t);
	cbw.bmCBWFlags = MSD_CBWFLAGS_D2H;
	cbw.bCBWCBLength = 12;
	cbw.CBWCB[0] = SCSI_CMD_REQUEST_SENSE;
	cbw.CBWCB[4] = sizeof(scsi_sense_response_t);
	transaction.cbw = &cbw;

	res = _scsi_perform_transaction(lunp, &transaction, resp);
	if (res == MSD_RESULT_OK) {
		//transaction is OK; check length
		if (transaction.data_processed < cbw.dCBWDataTransferLength) {
			res = MSD_RESULT_TRANSPORT_ERROR;
		}
	}

	return res;
}

static msd_result_t scsi_testunitready(USBHMassStorageLUNDriver *lunp) {
	USBH_DEFINE_BUFFER(msd_cbw_t cbw);
	msd_transaction_t transaction;

	memset(cbw.CBWCB, 0, sizeof(cbw.CBWCB));
	cbw.dCBWDataTransferLength = 0;
	cbw.bmCBWFlags = MSD_CBWFLAGS_D2H;
	cbw.bCBWCBLength = 6;
	cbw.CBWCB[0] = SCSI_CMD_TEST_UNIT_READY;
	transaction.cbw = &cbw;

	return _scsi_perform_transaction(lunp, &transaction, NULL);
}

static msd_result_t scsi_readcapacity10(USBHMassStorageLUNDriver *lunp, scsi_readcapacity10_response_t *resp) {
	USBH_DEFINE_BUFFER(msd_cbw_t cbw);
	msd_transaction_t transaction;
	msd_result_t res;

	memset(cbw.CBWCB, 0, sizeof(cbw.CBWCB));
	cbw.dCBWDataTransferLength = sizeof(scsi_readcapacity10_response_t);
	cbw.bmCBWFlags = MSD_CBWFLAGS_D2H;
	cbw.bCBWCBLength = 12;
	cbw.CBWCB[0] = SCSI_CMD_READ_CAPACITY_10;
	transaction.cbw = &cbw;

	res = _scsi_perform_transaction(lunp, &transaction, resp);
	if (res == MSD_RESULT_OK) {
		//transaction is OK; check length
		if (transaction.data_processed < cbw.dCBWDataTransferLength) {
			res = MSD_RESULT_TRANSPORT_ERROR;
		}
	}

	return res;
}


static msd_result_t scsi_read10(USBHMassStorageLUNDriver *lunp, uint32_t lba, uint16_t n, uint8_t *data, uint32_t *actual_len) {
	USBH_DEFINE_BUFFER(msd_cbw_t cbw);
	msd_transaction_t transaction;
	msd_result_t res;

	memset(cbw.CBWCB, 0, sizeof(cbw.CBWCB));
	cbw.dCBWDataTransferLength = n * lunp->info.blk_size;
	cbw.bmCBWFlags = MSD_CBWFLAGS_D2H;
	cbw.bCBWCBLength = 10;
	cbw.CBWCB[0] = SCSI_CMD_READ_10;
	cbw.CBWCB[2] = (uint8_t)(lba >> 24);
	cbw.CBWCB[3] = (uint8_t)(lba >> 16);
	cbw.CBWCB[4] = (uint8_t)(lba >> 8);
	cbw.CBWCB[5] = (uint8_t)(lba);
	cbw.CBWCB[7] = (uint8_t)(n >> 8);
	cbw.CBWCB[8] = (uint8_t)(n);
	transaction.cbw = &cbw;

	res = _scsi_perform_transaction(lunp, &transaction, data);
	if (actual_len) {
		*actual_len = transaction.data_processed;
	}
	if (res == MSD_RESULT_OK) {
		//transaction is OK; check length
		if (transaction.data_processed < cbw.dCBWDataTransferLength) {
			res = MSD_RESULT_TRANSPORT_ERROR;
		}
	}

	return res;
}

static msd_result_t scsi_write10(USBHMassStorageLUNDriver *lunp, uint32_t lba, uint16_t n, const uint8_t *data, uint32_t *actual_len) {
	USBH_DEFINE_BUFFER(msd_cbw_t cbw);
	msd_transaction_t transaction;
	msd_result_t res;

	memset(cbw.CBWCB, 0, sizeof(cbw.CBWCB));
	cbw.dCBWDataTransferLength = n * lunp->info.blk_size;
	cbw.bmCBWFlags = MSD_CBWFLAGS_H2D;
	cbw.bCBWCBLength = 10;
	cbw.CBWCB[0] = SCSI_CMD_WRITE_10;
	cbw.CBWCB[2] = (uint8_t)(lba >> 24);
	cbw.CBWCB[3] = (uint8_t)(lba >> 16);
	cbw.CBWCB[4] = (uint8_t)(lba >> 8);
	cbw.CBWCB[5] = (uint8_t)(lba);
	cbw.CBWCB[7] = (uint8_t)(n >> 8);
	cbw.CBWCB[8] = (uint8_t)(n);
	transaction.cbw = &cbw;

	res = _scsi_perform_transaction(lunp, &transaction, (void *)data);
	if (actual_len) {
		*actual_len = transaction.data_processed;
	}
	if (res == MSD_RESULT_OK) {
		//transaction is OK; check length
		if (transaction.data_processed < cbw.dCBWDataTransferLength) {
			res = MSD_RESULT_TRANSPORT_ERROR;
		}
	}

	return res;
}



/*===========================================================================*/
/* Block driver data/functions                                               */
/*===========================================================================*/

USBHMassStorageLUNDriver MSBLKD[HAL_USBHMSD_MAX_LUNS];

static const struct USBHMassStorageDriverVMT blk_vmt = {
	(size_t)0,
	(bool (*)(void *))usbhmsdLUNIsInserted,
	(bool (*)(void *))usbhmsdLUNIsProtected,
	(bool (*)(void *))usbhmsdLUNConnect,
	(bool (*)(void *))usbhmsdLUNDisconnect,
	(bool (*)(void *, uint32_t, uint8_t *, uint32_t))usbhmsdLUNRead,
	(bool (*)(void *, uint32_t,	const uint8_t *, uint32_t))usbhmsdLUNWrite,
	(bool (*)(void *))usbhmsdLUNSync,
	(bool (*)(void *, BlockDeviceInfo *))usbhmsdLUNGetInfo
};

static void _lun_object_deinit(USBHMassStorageLUNDriver *lunp) {
	osalDbgCheck(lunp != NULL);
	chSemWait(&lunp->sem);
	lunp->msdp = NULL;
	lunp->next = NULL;
	memset(&lunp->info, 0, sizeof(lunp->info));
	lunp->state = BLK_STOP;
	chSemSignal(&lunp->sem);
}

static void _lun_object_init(USBHMassStorageLUNDriver *lunp) {
	osalDbgCheck(lunp != NULL);
	memset(lunp, 0, sizeof(*lunp));
	lunp->vmt = &blk_vmt;
	lunp->state = BLK_STOP;
	chSemObjectInit(&lunp->sem, 1);
	/* Unnecessary because of the memset:
		lunp->msdp = NULL;
		lunp->next = NULL;
		lunp->info.* = 0;
	*/
}

bool usbhmsdLUNConnect(USBHMassStorageLUNDriver *lunp) {
	osalDbgCheck(lunp != NULL);
	osalDbgCheck(lunp->msdp != NULL);
	msd_result_t res;

	chSemWait(&lunp->sem);
	osalDbgAssert((lunp->state == BLK_READY) || (lunp->state == BLK_ACTIVE), "invalid state");
	if (lunp->state == BLK_READY) {
		chSemSignal(&lunp->sem);
		return HAL_SUCCESS;
	}
	lunp->state = BLK_CONNECTING;

    {
		USBH_DEFINE_BUFFER(scsi_inquiry_response_t inq);
		uinfo("INQUIRY...");
		res = scsi_inquiry(lunp, &inq);
		if (res == MSD_RESULT_DISCONNECTED) {
			goto failed;
		} else if (res == MSD_RESULT_TRANSPORT_ERROR) {
			//retry?
			goto failed;
		} else if (res == MSD_RESULT_FAILED) {
			//retry?
			goto failed;
		}

		uinfof("\tPDT=%02x", inq.peripheral & 0x1f);
		if (inq.peripheral != 0) {
			uerr("\tUnsupported PDT");
			goto failed;
		}
	}

	// Test if unit ready
    uint8_t i;
	for (i = 0; i < 10; i++) {
		uinfo("TEST UNIT READY...");
		res = scsi_testunitready(lunp);
		if (res == MSD_RESULT_DISCONNECTED) {
			goto failed;
		} else if (res == MSD_RESULT_TRANSPORT_ERROR) {
			//retry?
			goto failed;
		} else if (res == MSD_RESULT_FAILED) {
			uinfo("\tTEST UNIT READY: Command Failed, retry");
			osalThreadSleepMilliseconds(200);
			continue;
		}
		uinfo("\tReady.");
		break;
	}
	if (i == 10) goto failed;

	{
		USBH_DEFINE_BUFFER(scsi_readcapacity10_response_t cap);
		// Read capacity
		uinfo("READ CAPACITY(10)...");
		res = scsi_readcapacity10(lunp, &cap);
		if (res == MSD_RESULT_DISCONNECTED) {
			goto failed;
		} else if (res == MSD_RESULT_TRANSPORT_ERROR) {
			//retry?
			goto failed;
		} else if (res == MSD_RESULT_FAILED) {
			//retry?
			goto failed;
		}

		lunp->info.blk_size = __REV(cap.block_size);
		lunp->info.blk_num = __REV(cap.last_block_addr) + 1;
	}

	uinfof("\tBlock size=%dbytes, blocks=%u (~%u MB)", lunp->info.blk_size, lunp->info.blk_num,
		(uint32_t)(((uint64_t)lunp->info.blk_size * lunp->info.blk_num) / (1024UL * 1024UL)));

	uinfo("MSD Connected.");
	lunp->state = BLK_READY;
	chSemSignal(&lunp->sem);
	return HAL_SUCCESS;

  /* Connection failed, state reset to BLK_ACTIVE.*/
failed:
	uinfo("MSD Connect failed.");
	lunp->state = BLK_ACTIVE;
	chSemSignal(&lunp->sem);
	return HAL_FAILED;
}

bool usbhmsdLUNDisconnect(USBHMassStorageLUNDriver *lunp) {
	osalDbgCheck(lunp != NULL);

	chSemWait(&lunp->sem);
	osalDbgAssert((lunp->state == BLK_READY) || (lunp->state == BLK_ACTIVE), "invalid state");
	if (lunp->state == BLK_ACTIVE) {
		chSemSignal(&lunp->sem);
		return HAL_SUCCESS;
	}
	lunp->state = BLK_DISCONNECTING;

	//TODO: complete: sync, etc.

	lunp->state = BLK_ACTIVE;
	chSemSignal(&lunp->sem);

	return HAL_SUCCESS;
}

bool usbhmsdLUNRead(USBHMassStorageLUNDriver *lunp, uint32_t startblk,
                uint8_t *buffer, uint32_t n) {

	osalDbgCheck(lunp != NULL);
	bool ret = HAL_FAILED;
	uint16_t blocks;
	msd_result_t res;
	uint32_t actual_len;

	chSemWait(&lunp->sem);
	if (lunp->state != BLK_READY) {
		chSemSignal(&lunp->sem);
		return ret;
	}
	lunp->state = BLK_READING;

	while (n) {
		if (n > 0xffff) {
			blocks = 0xffff;
		} else {
			blocks = (uint16_t)n;
		}
		res = scsi_read10(lunp, startblk, blocks, buffer, &actual_len);
		if (res == MSD_RESULT_DISCONNECTED) {
			goto exit;
		} else if (res == MSD_RESULT_TRANSPORT_ERROR) {
			//retry?
			goto exit;
		} else if (res == MSD_RESULT_FAILED) {
			//retry?
			goto exit;
		}
		n -= blocks;
		startblk += blocks;
		buffer += blocks * lunp->info.blk_size;
	}

	ret = HAL_SUCCESS;

exit:
	lunp->state = BLK_READY;
	chSemSignal(&lunp->sem);
	return ret;
}

bool usbhmsdLUNWrite(USBHMassStorageLUNDriver *lunp, uint32_t startblk,
                const uint8_t *buffer, uint32_t n) {

	osalDbgCheck(lunp != NULL);
	bool ret = HAL_FAILED;
	uint16_t blocks;
	msd_result_t res;
	uint32_t actual_len;

	chSemWait(&lunp->sem);
	if (lunp->state != BLK_READY) {
		chSemSignal(&lunp->sem);
		return ret;
	}
	lunp->state = BLK_WRITING;

	while (n) {
		if (n > 0xffff) {
			blocks = 0xffff;
		} else {
			blocks = (uint16_t)n;
		}
		res = scsi_write10(lunp, startblk, blocks, buffer, &actual_len);
		if (res == MSD_RESULT_DISCONNECTED) {
			goto exit;
		} else if (res == MSD_RESULT_TRANSPORT_ERROR) {
			//retry?
			goto exit;
		} else if (res == MSD_RESULT_FAILED) {
			//retry?
			goto exit;
		}
		n -= blocks;
		startblk += blocks;
		buffer += blocks * lunp->info.blk_size;
	}

	ret = HAL_SUCCESS;

exit:
	lunp->state = BLK_READY;
	chSemSignal(&lunp->sem);
	return ret;
}

bool usbhmsdLUNSync(USBHMassStorageLUNDriver *lunp) {
	osalDbgCheck(lunp != NULL);
	(void)lunp;
	//TODO: Do SCSI Sync
	return HAL_SUCCESS;
}

bool usbhmsdLUNGetInfo(USBHMassStorageLUNDriver *lunp, BlockDeviceInfo *bdip) {
	osalDbgCheck(lunp != NULL);
	osalDbgCheck(bdip != NULL);

	osalSysLock();
	if (lunp->state >= BLK_READY) {
		*bdip = lunp->info;
		osalSysUnlock();
		return HAL_SUCCESS;
	}
	osalSysUnlock();
	return HAL_FAILED;
}

bool usbhmsdLUNIsInserted(USBHMassStorageLUNDriver *lunp) {
	osalDbgCheck(lunp != NULL);
	return (lunp->state >= BLK_ACTIVE);
}

bool usbhmsdLUNIsProtected(USBHMassStorageLUNDriver *lunp) {
	osalDbgCheck(lunp != NULL);
	//TODO: Implement
	return FALSE;
}

static void _msd_object_init(USBHMassStorageDriver *msdp) {
	osalDbgCheck(msdp != NULL);
	memset(msdp, 0, sizeof(*msdp));
	msdp->info = &usbhmsdClassDriverInfo;
}

static void _msd_init(void) {
	uint8_t i;
	for (i = 0; i < HAL_USBHMSD_MAX_INSTANCES; i++) {
		_msd_object_init(&USBHMSD[i]);
	}
	for (i = 0; i < HAL_USBHMSD_MAX_LUNS; i++) {
		_lun_object_init(&MSBLKD[i]);
	}
}
#endif
