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

#if HAL_USBH_USE_MSD

#if !HAL_USE_USBH
#error "USBHMSD needs USBH"
#endif

#include <string.h>
#include "usbh/dev/msd.h"
#include "usbh/internal.h"

//#pragma GCC optimize("Og")


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





/*===========================================================================*/
/* USB Class driver loader for MSD								 		 	 */
/*===========================================================================*/

USBHMassStorageDriver USBHMSD[HAL_USBHMSD_MAX_INSTANCES];

static usbh_baseclassdriver_t *_msd_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem);
static void _msd_unload(usbh_baseclassdriver_t *drv);

static const usbh_classdriver_vmt_t class_driver_vmt = {
	_msd_load,
	_msd_unload
};

const usbh_classdriverinfo_t usbhmsdClassDriverInfo = {
	0x08, 0x06, 0x50, "MSD", &class_driver_vmt
};

#define MSD_REQ_RESET							0xFF
#define MSD_GET_MAX_LUN							0xFE

static usbh_baseclassdriver_t *_msd_load(usbh_device_t *dev, const uint8_t *descriptor, uint16_t rem) {
	int i;
	USBHMassStorageDriver *msdp;
	uint8_t luns; // should declare it here to eliminate 'control bypass initialization' warning
	usbh_urbstatus_t stat;  // should declare it here to eliminate 'control bypass initialization' warning

	if ((rem < descriptor[0]) || (descriptor[1] != USBH_DT_INTERFACE))
		return NULL;

	const usbh_interface_descriptor_t * const ifdesc = (const usbh_interface_descriptor_t *)descriptor;

	if ((ifdesc->bAlternateSetting != 0)
			|| (ifdesc->bNumEndpoints < 2)
			|| (ifdesc->bInterfaceSubClass != 0x06)
			|| (ifdesc->bInterfaceProtocol != 0x50)) {
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
	USBH_DEFINE_BUFFER(uint8_t, buff[4]);
	stat = usbhControlRequest(dev,
			USBH_CLASSIN(USBH_REQTYPE_INTERFACE, MSD_GET_MAX_LUN, 0, msdp->ifnum),
			1, buff);
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

			osalSysLock();
			MSBLKD[i].state = BLK_ACTIVE;	/* transition directly to active, instead of BLK_STOP */
			osalSysUnlock();

			/* connect the LUN (TODO: review if it's best to leave the LUN disconnected) */
			usbhmsdLUNConnect(&MSBLKD[i]);
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

	osalMutexLock(&msdp->mtx);
	osalSysLock();
	usbhEPCloseS(&msdp->epin);
	usbhEPCloseS(&msdp->epout);
	while (lunp) {
		lunp->state = BLK_STOP;
		lunp = lunp->next;
	}
	osalSysUnlock();
	osalMutexUnlock(&msdp->mtx);

	/* now that the LUNs are idle, deinit them */
	lunp = msdp->luns;
	osalSysLock();
	while (lunp) {
		usbhmsdLUNObjectInit(lunp);
		lunp = lunp->next;
	}
	osalSysUnlock();
}


/*===========================================================================*/
/* MSD Class driver operations (Bulk-Only transport)			 		 	 */
/*===========================================================================*/



/* USB Bulk Only Transport SCSI Command block wrapper */
PACKED_STRUCT {
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
PACKED_STRUCT {
	uint32_t dCSWSignature;
	uint32_t dCSWTag;
	uint32_t dCSWDataResidue;
	uint8_t bCSWStatus;
} msd_csw_t;
#define MSD_CSW_SIGNATURE						0x53425355


typedef union {
	msd_cbw_t cbw;
	msd_csw_t csw;
} msd_transaction_t;

typedef enum {
	MSD_TRANSACTIONRESULT_OK,
	MSD_TRANSACTIONRESULT_DISCONNECTED,
	MSD_TRANSACTIONRESULT_STALL,
	MSD_TRANSACTIONRESULT_BUS_ERROR,
	MSD_TRANSACTIONRESULT_SYNC_ERROR
} msd_transaction_result_t;

typedef enum {
	MSD_COMMANDRESULT_PASSED = 0,
	MSD_COMMANDRESULT_FAILED = 1,
	MSD_COMMANDRESULT_PHASE_ERROR = 2
} msd_command_result_t;

typedef struct {
	msd_transaction_result_t tres;
	msd_command_result_t cres;
} msd_result_t;


/* ----------------------------------------------------- */
/* 			SCSI Commands 								*/
/* ----------------------------------------------------- */

/* Read 10 and Write 10 */
#define SCSI_CMD_READ_10 						0x28
#define SCSI_CMD_WRITE_10						0x2A

/* Request sense */
#define SCSI_CMD_REQUEST_SENSE 					0x03
PACKED_STRUCT {
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
PACKED_STRUCT {
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
PACKED_STRUCT {
	uint32_t last_block_addr;
	uint32_t block_size;
} scsi_readcapacity10_response_t;

/* Start/Stop Unit */
#define SCSI_CMD_START_STOP_UNIT				0x1B
PACKED_STRUCT {
	uint8_t op_code;
	uint8_t lun_immed;
	uint8_t res1;
	uint8_t res2;
	uint8_t loej_start;
	uint8_t control;
} scsi_startstopunit_request_t;

/* test unit ready */
#define SCSI_CMD_TEST_UNIT_READY				0x00

/* Other commands, TODO: use or remove them
#define SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL	0x1E
#define SCSI_CMD_VERIFY_10						0x2F
#define SCSI_CMD_SEND_DIAGNOSTIC				0x1D
#define SCSI_CMD_MODE_SENSE_6                   0x1A
*/

static inline void _prepare_cbw(msd_transaction_t *tran, USBHMassStorageLUNDriver *lunp) {
	tran->cbw.bCBWLUN = (uint8_t)(lunp - &lunp->msdp->luns[0]);
	memset(&tran->cbw.CBWCB, 0, sizeof(tran->cbw.CBWCB));
}

static msd_transaction_result_t _msd_transaction(msd_transaction_t *tran, USBHMassStorageLUNDriver *lunp, void *data) {

	uint32_t actual_len;
	usbh_urbstatus_t status;

	tran->cbw.dCBWSignature = MSD_CBW_SIGNATURE;
	tran->cbw.dCBWTag = ++lunp->msdp->tag;

	/* control phase */
	status = usbhBulkTransfer(&lunp->msdp->epout, &tran->cbw,
					sizeof(tran->cbw), &actual_len, MS2ST(1000));

	if (status == USBH_URBSTATUS_CANCELLED) {
		uerr("\tMSD: Control phase: USBH_URBSTATUS_CANCELLED");
		return MSD_TRANSACTIONRESULT_DISCONNECTED;
	} else if (status == USBH_URBSTATUS_STALL) {
		uerr("\tMSD: Control phase: USBH_URBSTATUS_STALL");
		return MSD_TRANSACTIONRESULT_STALL;
	} else if (status != USBH_URBSTATUS_OK) {
		uerrf("\tMSD: Control phase: status = %d, != OK", status);
		return MSD_TRANSACTIONRESULT_BUS_ERROR;
	} else if (actual_len != sizeof(tran->cbw)) {
		uerrf("\tMSD: Control phase: wrong actual_len = %d", actual_len);
		return MSD_TRANSACTIONRESULT_BUS_ERROR;
	}


	/* data phase */
	if (tran->cbw.dCBWDataTransferLength) {
		status = usbhBulkTransfer(
				tran->cbw.bmCBWFlags & MSD_CBWFLAGS_D2H ? &lunp->msdp->epin : &lunp->msdp->epout,
				data,
				tran->cbw.dCBWDataTransferLength,
				&actual_len, MS2ST(20000));

		if (status == USBH_URBSTATUS_CANCELLED) {
			uerr("\tMSD: Data phase: USBH_URBSTATUS_CANCELLED");
			return MSD_TRANSACTIONRESULT_DISCONNECTED;
		} else if (status == USBH_URBSTATUS_STALL) {
			uerr("\tMSD: Data phase: USBH_URBSTATUS_STALL");
			return MSD_TRANSACTIONRESULT_STALL;
		} else if (status != USBH_URBSTATUS_OK) {
			uerrf("\tMSD: Data phase: status = %d, != OK", status);
			return MSD_TRANSACTIONRESULT_BUS_ERROR;
		} else if (actual_len != tran->cbw.dCBWDataTransferLength) {
			uerrf("\tMSD: Data phase: wrong actual_len = %d", actual_len);
			return MSD_TRANSACTIONRESULT_BUS_ERROR;
		}
	}


	/* status phase */
	status = usbhBulkTransfer(&lunp->msdp->epin, &tran->csw,
			sizeof(tran->csw), &actual_len, MS2ST(1000));

	if (status == USBH_URBSTATUS_CANCELLED) {
		uerr("\tMSD: Status phase: USBH_URBSTATUS_CANCELLED");
		return MSD_TRANSACTIONRESULT_DISCONNECTED;
	} else if (status == USBH_URBSTATUS_STALL) {
		uerr("\tMSD: Status phase: USBH_URBSTATUS_STALL");
		return MSD_TRANSACTIONRESULT_STALL;
	} else if (status != USBH_URBSTATUS_OK) {
		uerrf("\tMSD: Status phase: status = %d, != OK", status);
		return MSD_TRANSACTIONRESULT_BUS_ERROR;
	} else if (actual_len != sizeof(tran->csw)) {
		uerrf("\tMSD: Status phase: wrong actual_len = %d", actual_len);
		return MSD_TRANSACTIONRESULT_BUS_ERROR;
	} else if (tran->csw.dCSWSignature != MSD_CSW_SIGNATURE) {
		uerr("\tMSD: Status phase: wrong signature");
		return MSD_TRANSACTIONRESULT_BUS_ERROR;
	} else if (tran->csw.dCSWTag != lunp->msdp->tag) {
		uerrf("\tMSD: Status phase: wrong tag (expected %d, got %d)",
				lunp->msdp->tag, tran->csw.dCSWTag);
		return MSD_TRANSACTIONRESULT_SYNC_ERROR;
	}

	if (tran->csw.dCSWDataResidue) {
		uwarnf("\tMSD: Residue=%d", tran->csw.dCSWDataResidue);
	}

	return MSD_TRANSACTIONRESULT_OK;
}


static msd_result_t scsi_inquiry(USBHMassStorageLUNDriver *lunp, scsi_inquiry_response_t *resp) {
	msd_transaction_t transaction;
	msd_result_t res;

	_prepare_cbw(&transaction, lunp);
	transaction.cbw.dCBWDataTransferLength = sizeof(scsi_inquiry_response_t);
	transaction.cbw.bmCBWFlags = MSD_CBWFLAGS_D2H;
	transaction.cbw.bCBWCBLength = 6;
	transaction.cbw.CBWCB[0] = SCSI_CMD_INQUIRY;
	transaction.cbw.CBWCB[4] = sizeof(scsi_inquiry_response_t);

	res.tres = _msd_transaction(&transaction, lunp, resp);
	if (res.tres == MSD_TRANSACTIONRESULT_OK) {
		res.cres = (msd_command_result_t) transaction.csw.bCSWStatus;
	}
	return res;
}

static msd_result_t scsi_requestsense(USBHMassStorageLUNDriver *lunp, scsi_sense_response_t *resp) {
	msd_transaction_t transaction;
	msd_result_t res;

	_prepare_cbw(&transaction, lunp);
	transaction.cbw.dCBWDataTransferLength = sizeof(scsi_sense_response_t);
	transaction.cbw.bmCBWFlags = MSD_CBWFLAGS_D2H;
	transaction.cbw.bCBWCBLength = 12;
	transaction.cbw.CBWCB[0] = SCSI_CMD_REQUEST_SENSE;
	transaction.cbw.CBWCB[4] = sizeof(scsi_sense_response_t);

	res.tres = _msd_transaction(&transaction, lunp, resp);
	if (res.tres == MSD_TRANSACTIONRESULT_OK) {
		res.cres = (msd_command_result_t) transaction.csw.bCSWStatus;
	}
	return res;
}

static msd_result_t scsi_testunitready(USBHMassStorageLUNDriver *lunp) {
	msd_transaction_t transaction;
	msd_result_t res;

	_prepare_cbw(&transaction, lunp);
	transaction.cbw.dCBWDataTransferLength = 0;
	transaction.cbw.bmCBWFlags = MSD_CBWFLAGS_D2H;
	transaction.cbw.bCBWCBLength = 6;
	transaction.cbw.CBWCB[0] = SCSI_CMD_TEST_UNIT_READY;

	res.tres = _msd_transaction(&transaction, lunp, NULL);
	if (res.tres == MSD_TRANSACTIONRESULT_OK) {
		res.cres = (msd_command_result_t) transaction.csw.bCSWStatus;
	}
	return res;
}

static msd_result_t scsi_readcapacity10(USBHMassStorageLUNDriver *lunp, scsi_readcapacity10_response_t *resp) {
	msd_transaction_t transaction;
	msd_result_t res;

	_prepare_cbw(&transaction, lunp);
	transaction.cbw.dCBWDataTransferLength = sizeof(scsi_readcapacity10_response_t);
	transaction.cbw.bmCBWFlags = MSD_CBWFLAGS_D2H;
	transaction.cbw.bCBWCBLength = 12;
	transaction.cbw.CBWCB[0] = SCSI_CMD_READ_CAPACITY_10;

	res.tres = _msd_transaction(&transaction, lunp, resp);
	if (res.tres == MSD_TRANSACTIONRESULT_OK) {
		res.cres = (msd_command_result_t) transaction.csw.bCSWStatus;
	}
	return res;
}


static msd_result_t scsi_read10(USBHMassStorageLUNDriver *lunp, uint32_t lba, uint16_t n, uint8_t *data) {
	msd_transaction_t transaction;
	msd_result_t res;

	_prepare_cbw(&transaction, lunp);
	transaction.cbw.dCBWDataTransferLength = n * lunp->info.blk_size;
	transaction.cbw.bmCBWFlags = MSD_CBWFLAGS_D2H;
	transaction.cbw.bCBWCBLength = 10;
	transaction.cbw.CBWCB[0] = SCSI_CMD_READ_10;
	transaction.cbw.CBWCB[2] = (uint8_t)(lba >> 24);
	transaction.cbw.CBWCB[3] = (uint8_t)(lba >> 16);
	transaction.cbw.CBWCB[4] = (uint8_t)(lba >> 8);
	transaction.cbw.CBWCB[5] = (uint8_t)(lba);
	transaction.cbw.CBWCB[7] = (uint8_t)(n >> 8);
	transaction.cbw.CBWCB[8] = (uint8_t)(n);

	res.tres = _msd_transaction(&transaction, lunp, data);
	if (res.tres == MSD_TRANSACTIONRESULT_OK) {
		res.cres = (msd_command_result_t) transaction.csw.bCSWStatus;
	}
	return res;
}

static msd_result_t scsi_write10(USBHMassStorageLUNDriver *lunp, uint32_t lba, uint16_t n, const uint8_t *data) {
	msd_transaction_t transaction;
	msd_result_t res;

	_prepare_cbw(&transaction, lunp);
	transaction.cbw.dCBWDataTransferLength = n * lunp->info.blk_size;
	transaction.cbw.bmCBWFlags = MSD_CBWFLAGS_H2D;
	transaction.cbw.bCBWCBLength = 10;
	transaction.cbw.CBWCB[0] = SCSI_CMD_WRITE_10;
	transaction.cbw.CBWCB[2] = (uint8_t)(lba >> 24);
	transaction.cbw.CBWCB[3] = (uint8_t)(lba >> 16);
	transaction.cbw.CBWCB[4] = (uint8_t)(lba >> 8);
	transaction.cbw.CBWCB[5] = (uint8_t)(lba);
	transaction.cbw.CBWCB[7] = (uint8_t)(n >> 8);
	transaction.cbw.CBWCB[8] = (uint8_t)(n);

	res.tres = _msd_transaction(&transaction, lunp, (uint8_t *)data);
	if (res.tres == MSD_TRANSACTIONRESULT_OK) {
		res.cres = (msd_command_result_t) transaction.csw.bCSWStatus;
	}
	return res;
}



/*===========================================================================*/
/* Block driver data/functions								 		 	 	 */
/*===========================================================================*/

USBHMassStorageLUNDriver MSBLKD[HAL_USBHMSD_MAX_LUNS];

static const struct USBHMassStorageDriverVMT blk_vmt = {
	(bool (*)(void *))usbhmsdLUNIsInserted,
	(bool (*)(void *))usbhmsdLUNIsProtected,
	(bool (*)(void *))usbhmsdLUNConnect,
	(bool (*)(void *))usbhmsdLUNDisconnect,
	(bool (*)(void *, uint32_t, uint8_t *, uint32_t))usbhmsdLUNRead,
	(bool (*)(void *, uint32_t,	const uint8_t *, uint32_t))usbhmsdLUNWrite,
	(bool (*)(void *))usbhmsdLUNSync,
	(bool (*)(void *, BlockDeviceInfo *))usbhmsdLUNGetInfo
};



static uint32_t _requestsense(USBHMassStorageLUNDriver *lunp) {
	scsi_sense_response_t sense;
	msd_result_t res;

	res = scsi_requestsense(lunp, &sense);
	if (res.tres != MSD_TRANSACTIONRESULT_OK) {
		uerr("\tREQUEST SENSE: Transaction error");
		goto failed;
	} else if (res.cres == MSD_COMMANDRESULT_FAILED) {
		uerr("\tREQUEST SENSE: Command Failed");
		goto failed;
	} else if (res.cres == MSD_COMMANDRESULT_PHASE_ERROR) {
		//TODO: Do reset, etc.
		uerr("\tREQUEST SENSE: Command Phase Error");
		goto failed;
	}

	uerrf("\tREQUEST SENSE: Sense key=%x, ASC=%02x, ASCQ=%02x",
			sense.byte[2] & 0xf, sense.byte[12], sense.byte[13]);

	return (sense.byte[2] & 0xf) | (sense.byte[12] << 8) | (sense.byte[13] << 16);

failed:
	return 0xffffffff;
}

void usbhmsdLUNObjectInit(USBHMassStorageLUNDriver *lunp) {
	osalDbgCheck(lunp != NULL);
	memset(lunp, 0, sizeof(*lunp));
	lunp->vmt = &blk_vmt;
	lunp->state = BLK_STOP;
	/* Unnecessary because of the memset:
		lunp->msdp = NULL;
		lunp->next = NULL;
		lunp->info.* = 0;
	*/
}

void usbhmsdLUNStart(USBHMassStorageLUNDriver *lunp) {
	osalDbgCheck(lunp != NULL);
	osalSysLock();
	osalDbgAssert((lunp->state == BLK_STOP) || (lunp->state == BLK_ACTIVE),
			"invalid state");
	//TODO: complete
	//lunp->state = BLK_ACTIVE;
	osalSysUnlock();
}

void usbhmsdLUNStop(USBHMassStorageLUNDriver *lunp) {
	osalDbgCheck(lunp != NULL);
	osalSysLock();
	osalDbgAssert((lunp->state == BLK_STOP) || (lunp->state == BLK_ACTIVE),
			"invalid state");
	//TODO: complete
	//lunp->state = BLK_STOP;
	osalSysUnlock();
}

bool usbhmsdLUNConnect(USBHMassStorageLUNDriver *lunp) {
	USBHMassStorageDriver *const msdp = lunp->msdp;
	msd_result_t res;

	osalDbgCheck(msdp != NULL);
	osalSysLock();
	//osalDbgAssert((lunp->state == BLK_ACTIVE) || (lunp->state == BLK_READY),
    //            "invalid state");
	if (lunp->state == BLK_READY) {
	    osalSysUnlock();
		return HAL_SUCCESS;
	} else if (lunp->state != BLK_ACTIVE) {
		osalSysUnlock();
		return HAL_FAILED;
	}
	lunp->state = BLK_CONNECTING;
    osalSysUnlock();

    osalMutexLock(&msdp->mtx);

    USBH_DEFINE_BUFFER(union {
    		scsi_inquiry_response_t inq;
    		scsi_readcapacity10_response_t cap;	}, u);

	uinfo("INQUIRY...");
	res = scsi_inquiry(lunp, &u.inq);
	if (res.tres != MSD_TRANSACTIONRESULT_OK) {
		uerr("\tINQUIRY: Transaction error");
		goto failed;
	} else if (res.cres == MSD_COMMANDRESULT_FAILED) {
		uerr("\tINQUIRY: Command Failed");
		_requestsense(lunp);
		goto failed;
	} else if (res.cres == MSD_COMMANDRESULT_PHASE_ERROR) {
		//TODO: Do reset, etc.
		uerr("\tINQUIRY: Command Phase Error");
		goto failed;
	}

	uinfof("\tPDT=%02x", u.inq.peripheral & 0x1f);
	if (u.inq.peripheral != 0) {
		uerr("\tUnsupported PDT");
		goto failed;
	}

	// Test if unit ready
	uint8_t i;
	for (i = 0; i < 10; i++) {
		uinfo("TEST UNIT READY...");
		res = scsi_testunitready(lunp);
		if (res.tres != MSD_TRANSACTIONRESULT_OK) {
			uerr("\tTEST UNIT READY: Transaction error");
			goto failed;
		} else if (res.cres == MSD_COMMANDRESULT_FAILED) {
			uerr("\tTEST UNIT READY: Command Failed");
			_requestsense(lunp);
			continue;
		} else if (res.cres == MSD_COMMANDRESULT_PHASE_ERROR) {
			//TODO: Do reset, etc.
			uerr("\tTEST UNIT READY: Command Phase Error");
			goto failed;
		}
		uinfo("\tReady.");
		break;
		// osalThreadSleepMilliseconds(200); // will raise 'code is unreachable' warning
	}
	if (i == 10) goto failed;

	// Read capacity
	uinfo("READ CAPACITY(10)...");
	res = scsi_readcapacity10(lunp, &u.cap);
	if (res.tres != MSD_TRANSACTIONRESULT_OK) {
		uerr("\tREAD CAPACITY(10): Transaction error");
		goto failed;
	} else if (res.cres == MSD_COMMANDRESULT_FAILED) {
		uerr("\tREAD CAPACITY(10): Command Failed");
		_requestsense(lunp);
		goto failed;
	} else if (res.cres == MSD_COMMANDRESULT_PHASE_ERROR) {
		//TODO: Do reset, etc.
		uerr("\tREAD CAPACITY(10): Command Phase Error");
		goto failed;
	}
	lunp->info.blk_size = __REV(u.cap.block_size);
	lunp->info.blk_num = __REV(u.cap.last_block_addr) + 1;
	uinfof("\tBlock size=%dbytes, blocks=%u (~%u MB)", lunp->info.blk_size, lunp->info.blk_num,
		(uint32_t)(((uint64_t)lunp->info.blk_size * lunp->info.blk_num) / (1024UL * 1024UL)));

	uinfo("MSD Connected.");

	osalMutexUnlock(&msdp->mtx);
	osalSysLock();
	lunp->state = BLK_READY;
    osalSysUnlock();

	return HAL_SUCCESS;

  /* Connection failed, state reset to BLK_ACTIVE.*/
failed:
	osalMutexUnlock(&msdp->mtx);
	osalSysLock();
	lunp->state = BLK_ACTIVE;
    osalSysUnlock();
	return HAL_FAILED;
}


bool usbhmsdLUNDisconnect(USBHMassStorageLUNDriver *lunp) {
	osalDbgCheck(lunp != NULL);
	osalSysLock();
	osalDbgAssert((lunp->state == BLK_ACTIVE) || (lunp->state == BLK_READY),
				"invalid state");
	if (lunp->state == BLK_ACTIVE) {
		osalSysUnlock();
		return HAL_SUCCESS;
	}
	lunp->state = BLK_DISCONNECTING;
	osalSysUnlock();

	//TODO: complete

	osalSysLock();
	lunp->state = BLK_ACTIVE;
	osalSysUnlock();
	return HAL_SUCCESS;
}

bool usbhmsdLUNRead(USBHMassStorageLUNDriver *lunp, uint32_t startblk,
                uint8_t *buffer, uint32_t n) {

	osalDbgCheck(lunp != NULL);
	bool ret = HAL_FAILED;
	uint16_t blocks;
	msd_result_t res;

	osalSysLock();
	if (lunp->state != BLK_READY) {
		osalSysUnlock();
		return ret;
	}
	lunp->state = BLK_READING;
    osalSysUnlock();

	osalMutexLock(&lunp->msdp->mtx);
	while (n) {
		if (n > 0xffff) {
			blocks = 0xffff;
		} else {
			blocks = (uint16_t)n;
		}
		res = scsi_read10(lunp, startblk, blocks, buffer);
		if (res.tres != MSD_TRANSACTIONRESULT_OK) {
			uerr("\tREAD (10): Transaction error");
			goto exit;
		} else if (res.cres == MSD_COMMANDRESULT_FAILED) {
			//TODO: request sense, and act appropriately
			uerr("\tREAD (10): Command Failed");
			_requestsense(lunp);
			goto exit;
		} else if (res.cres == MSD_COMMANDRESULT_PHASE_ERROR) {
			//TODO: Do reset, etc.
			uerr("\tREAD (10): Command Phase Error");
			goto exit;
		}
		n -= blocks;
		startblk += blocks;
		buffer += blocks * lunp->info.blk_size;
	}

	ret = HAL_SUCCESS;

exit:
	osalMutexUnlock(&lunp->msdp->mtx);
	osalSysLock();
	if (lunp->state == BLK_READING) {
		lunp->state = BLK_READY;
	} else {
		osalDbgCheck(lunp->state == BLK_STOP);
		uwarn("MSD: State = BLK_STOP");
	}
    osalSysUnlock();
	return ret;
}

bool usbhmsdLUNWrite(USBHMassStorageLUNDriver *lunp, uint32_t startblk,
                const uint8_t *buffer, uint32_t n) {

	osalDbgCheck(lunp != NULL);
	bool ret = HAL_FAILED;
	uint16_t blocks;
	msd_result_t res;

	osalSysLock();
	if (lunp->state != BLK_READY) {
		osalSysUnlock();
		return ret;
	}
	lunp->state = BLK_WRITING;
    osalSysUnlock();

	osalMutexLock(&lunp->msdp->mtx);
	while (n) {
		if (n > 0xffff) {
			blocks = 0xffff;
		} else {
			blocks = (uint16_t)n;
		}
		res = scsi_write10(lunp, startblk, blocks, buffer);
		if (res.tres != MSD_TRANSACTIONRESULT_OK) {
			uerr("\tWRITE (10): Transaction error");
			goto exit;
		} else if (res.cres == MSD_COMMANDRESULT_FAILED) {
			//TODO: request sense, and act appropriately
			uerr("\tWRITE (10): Command Failed");
			_requestsense(lunp);
			goto exit;
		} else if (res.cres == MSD_COMMANDRESULT_PHASE_ERROR) {
			//TODO: Do reset, etc.
			uerr("\tWRITE (10): Command Phase Error");
			goto exit;
		}
		n -= blocks;
		startblk += blocks;
		buffer += blocks * lunp->info.blk_size;
	}

	ret = HAL_SUCCESS;

exit:
	osalMutexUnlock(&lunp->msdp->mtx);
	osalSysLock();
	if (lunp->state == BLK_WRITING) {
		lunp->state = BLK_READY;
	} else {
		osalDbgCheck(lunp->state == BLK_STOP);
		uwarn("MSD: State = BLK_STOP");
	}
	osalSysUnlock();
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
	*bdip = lunp->info;
	return HAL_SUCCESS;
}

bool usbhmsdLUNIsInserted(USBHMassStorageLUNDriver *lunp) {
	osalDbgCheck(lunp != NULL);
	blkstate_t state;
	osalSysLock();
	state = lunp->state;
	osalSysUnlock();
	return (state >= BLK_ACTIVE);
}

bool usbhmsdLUNIsProtected(USBHMassStorageLUNDriver *lunp) {
	osalDbgCheck(lunp != NULL);
	return FALSE;
}

void usbhmsdObjectInit(USBHMassStorageDriver *msdp) {
	osalDbgCheck(msdp != NULL);
	memset(msdp, 0, sizeof(*msdp));
	msdp->info = &usbhmsdClassDriverInfo;
	osalMutexObjectInit(&msdp->mtx);
}

#endif
