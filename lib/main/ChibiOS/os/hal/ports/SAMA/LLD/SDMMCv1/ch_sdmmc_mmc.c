#include <string.h>
#include "hal.h"

#if (SAMA_USE_SDMMC == TRUE)

#include "sama_sdmmc_lld.h"
#include "ch_sdmmc_device.h"
#include "ch_sdmmc_cmds.h"
#include "ch_sdmmc_sd.h"

#ifndef SDMMC_TRIM_MMC

/** Check if MMC Spec version 4 */
#define MMC_IsVer4(pSd)     ( MMC_CSD_SPEC_VERS(pSd->CSD) >= 4 )

/** Check if MMC CSD structure is 1.2 (3.1 or later) */
#define MMC_IsCSDVer1_2(pSd) \
    (  (SD_CSD_STRUCTURE(pSd->CSD)==2) \
     ||(SD_CSD_STRUCTURE(pSd->CSD)>2&&MMC_EXT_CSD_STRUCTURE(pSd->EXT)>=2) )


/** MMC transfer multiplier factor codes (1/10) list */

const uint8_t mmcTransMultipliers[16] = {
	0, 10, 12, 13, 15, 20, 26, 30, 35, 40, 45, 52, 55, 60, 70, 80
};


static uint8_t mmcSelectBuswidth(SdmmcDriver *driver, uint8_t busWidth, bool ddr)
{
	uint8_t error;
	uint32_t status;
	MmcCmd6Arg cmd6Arg = {
		.access = 0x3,   /* Write byte in the EXT_CSD register */
		.index = MMC_EXT_BUS_WIDTH_I,   /* Target byte in EXT_CSD */
		.value = MMC_EXT_BUS_WIDTH_1BIT,   /* Byte value */
	};

	if (busWidth == 8)
		cmd6Arg.value = MMC_EXT_BUS_WIDTH_8BITS
		    | (ddr ? MMC_EXT_BUS_WIDTH_DDR : 0);
	else if (busWidth == 4)
		cmd6Arg.value = MMC_EXT_BUS_WIDTH_4BITS
		    | (ddr ? MMC_EXT_BUS_WIDTH_DDR : 0);
	else if (busWidth != 1)
		return SDMMC_PARAM;

	error = MmcCmd6(driver, &cmd6Arg, &status);

	if (error)
		return SDMMC_ERR;
	else if (status & STATUS_MMC_SWITCH)
		return SDMMC_NOT_SUPPORTED;

	return SDMMC_OK;
}

static uint8_t mmcDetectBuswidth(SdmmcDriver *driver)
{
	uint8_t error, busWidth, mask = 0xff, i, len;
	sSdCard *pSd =&driver->card;
	//assert(sizeof(pSd->sandbox1) >= 8);
	//assert(sizeof(pSd->sandbox2) >= 8);

	memset(pSd->sandbox1, 0, 8);
	for (busWidth = 8; busWidth != 0; busWidth /= busWidth == 8 ? 2 : 4) {
		error = HwSetBusWidth(driver, busWidth);
		if (error)
			continue;
		switch (busWidth) {
		case 8:
			pSd->sandbox1[0] = 0x55;
			pSd->sandbox1[1] = 0xaa;
			break;
		case 4:
			pSd->sandbox1[0] = 0x5a;
			pSd->sandbox1[1] = 0;
			break;
		case 1:
			pSd->sandbox1[0] = 0x80;
			pSd->sandbox1[1] = 0;
			break;
		}
		len = (uint8_t)max_u32(busWidth, 2);
		error = Cmd19(driver, pSd->sandbox1, len, NULL);

		if (error) {
			/* Devices which do not respond to CMD19 - which results
			 * in the driver returning SDMMC_ERROR_NORESPONSE -
			 * simply do not support the bus test procedure.
			 * When the device responds to CMD19, mind the
			 * difference with other data write commands: further
			 * to host data, the device does not emit the CRC status
			 * token. Typically the peripheral reports the anomaly,
			 * and the driver is likely to return SDMMC_ERR_IO. */
			if (error != SDMMC_ERR_IO)
				return 0;
		}
		error = Cmd14(driver, pSd->sandbox2, busWidth, NULL);

		if (error)
			continue;
		if (busWidth == 1) {
			mask = 0xc0;
			pSd->sandbox2[0] &= mask;
		}
		len = busWidth == 8 ? 2 : 1;
		for (i = 0; i < len; i++) {
			if ((pSd->sandbox1[i] ^ pSd->sandbox2[i]) != mask)
				break;
		}
		if (i == len)
			break;
	}
	return busWidth;
}


uint8_t MmcGetExtInformation(SdmmcDriver *driver)
{
	sSdCard *pSd = &driver->card;
	/* MMC 4.0 Higher version */
	if (SD_CSD_STRUCTURE(pSd->CSD) >= 2 && MMC_IsVer4(pSd))
		return MmcCmd8(driver);
	else
		return SDMMC_NOT_SUPPORTED;
}


uint8_t MmcInit(SdmmcDriver *driver)
{
	MmcCmd6Arg sw_arg = {
		.access = 0x3,   /* Write byte in the EXT_CSD register */
	};
	uint64_t mem_size;
	uint32_t freq, drv_err, status;
	uint8_t error, tim_mode, pwr_class, width;
	bool flag;

	tim_mode = driver->card.bSpeedMode = SDMMC_TIM_MMC_BC;
	/* The host then issues the command ALL_SEND_CID (CMD2) to the card to get
	 * its unique card identification (CID) number.
	 * Card that is unidentified (i.e. which is in Ready State) sends its CID
	 * number as the response (on the CMD line). */
	error = Cmd2(driver);
	if (error)
		return error;

	/* Thereafter, the host issues SET_RELATIVE_ADDR (CMD3) to assign the
	 * device a dedicated relative card address (RCA), which is shorter than
	 * CID and which is used to address the card in the future data transfer
	 * mode. Once the RCA is received the card state changes to the Stand-by
	 * State. */
	error = Cmd3(driver);
	if (error)
		return error;
	//else
		TRACE_DEBUG_1("RCA=%u\n\r", driver->card.wAddress);

	/* SEND_CSD (CMD9) to obtain the Card Specific Data (CSD register),
	 * e.g. block length, card storage capacity, etc... */
	error = Cmd9(driver);
	if (error)
		return error;

	/* Calculate transfer speed */
	freq = SdmmcGetMaxFreq(driver);

	error = HwSetClock(driver, &freq);

	if (error != SDMMC_OK && error != SDMMC_CHANGED)
		return error;

	driver->card.dwCurrSpeed = freq;

	/* Now select the card, to TRAN state */
	error = SdMmcSelect(driver, driver->card.wAddress, 0);
	if (error)
		return error;

	/* If CSD:SPEC_VERS indicates v4.0 or higher, read EXT_CSD */
	error = MmcGetExtInformation(driver);
	/* Consider HS200 timing mode */
	if (error == SDMMC_OK && MMC_EXT_EXT_CSD_REV(driver->card.EXT) >= 6
	    && MMC_IsCSDVer1_2((&driver->card)) && MMC_EXT_CARD_TYPE(driver->card.EXT) & 0x10
	    && HwIsTimingSupported(driver, SDMMC_TIM_MMC_HS200))
		tim_mode = SDMMC_TIM_MMC_HS200;
	/* Consider High Speed DDR timing mode */
	else if (error == SDMMC_OK && MMC_EXT_EXT_CSD_REV(driver->card.EXT) >= 4
	    && MMC_IsCSDVer1_2((&driver->card)) && MMC_EXT_CARD_TYPE(driver->card.EXT) & 0x4
	    && HwIsTimingSupported(driver, SDMMC_TIM_MMC_HS_DDR))
		tim_mode = SDMMC_TIM_MMC_HS_DDR;
	/* Consider High Speed SDR timing mode */
	else if (error == SDMMC_OK
	    && MMC_IsCSDVer1_2((&driver->card)) && MMC_EXT_CARD_TYPE(driver->card.EXT) & 0x1
	    && HwIsTimingSupported(driver, SDMMC_TIM_MMC_HS_SDR))
		tim_mode = SDMMC_TIM_MMC_HS_SDR;
	/* Check power requirements of the device */
	if (error == SDMMC_OK) {
		if (tim_mode == SDMMC_TIM_MMC_HS200)
			pwr_class = MMC_EXT_PWR_CL_200_195(driver->card.EXT);
		else if (tim_mode == SDMMC_TIM_MMC_HS_DDR)
			pwr_class = MMC_EXT_PWR_CL_DDR_52_360(driver->card.EXT);
		else if (tim_mode == SDMMC_TIM_MMC_HS_SDR)
			pwr_class = MMC_EXT_PWR_CL_52_360(driver->card.EXT);
		else
			pwr_class = MMC_EXT_PWR_CL_26_360(driver->card.EXT);

		if (pwr_class != 0) {
			sw_arg.index = MMC_EXT_POWER_CLASS_I;
			sw_arg.value = 0xf;
			error = MmcCmd6(driver, &sw_arg, &status);
			if (error) {
				TRACE_DEBUG_1("Pwr class %s\n\r",SD_StringifyRetCode(error));
			}
		}
	}

	/* Enable High Speed SDR timing mode */
	if (tim_mode == SDMMC_TIM_MMC_HS_SDR || tim_mode == SDMMC_TIM_MMC_HS_DDR) {

		sw_arg.index = MMC_EXT_HS_TIMING_I;
		sw_arg.value = MMC_EXT_HS_TIMING_EN;

		error = MmcCmd6(driver, &sw_arg, &status);

		if (error == SDMMC_OK)
			error = HwSetHsMode(driver, SDMMC_TIM_MMC_HS_SDR);
		if (error == SDMMC_OK)
			error = Cmd13(driver, &status);
		if (error == SDMMC_OK && (status & ~STATUS_STATE
		    & ~STATUS_READY_FOR_DATA
		    || (status & STATUS_STATE) != STATUS_TRAN))
			error = SDMMC_STATE;
		if (error == SDMMC_OK) {
			driver->card.bSpeedMode = SDMMC_TIM_MMC_HS_SDR;
			freq = SdmmcGetMaxFreq(driver);
			error = HwSetClock(driver, &freq);
			driver->card.dwCurrSpeed = freq;
			error = error == SDMMC_CHANGED ? SDMMC_OK : error;
		}
		if (error != SDMMC_OK) {
			TRACE_ERROR_1("HS %s\n\r", SD_StringifyRetCode(error));
			return error;
		}
	}

	/* Consider using the widest supported data bus */
	if (MMC_IsCSDVer1_2((&driver->card)) && MMC_IsVer4((&driver->card))) {

		width = mmcDetectBuswidth(driver);

		if (width > 1) {

			error = mmcSelectBuswidth(driver, width,tim_mode == SDMMC_TIM_MMC_HS_DDR);

			if (error == SDMMC_OK)
				error = HwSetBusWidth(driver, width);

			if (error == SDMMC_OK)
				driver->card.bBusMode = width;

			if (error == SDMMC_OK && tim_mode == SDMMC_TIM_MMC_HS_DDR)
				/* Switch to High Speed DDR timing mode */
				error = HwSetHsMode(driver, tim_mode);
			if (error == SDMMC_OK)
				error = Cmd13(driver, &status);
			if (error == SDMMC_OK && (status & ~STATUS_STATE
			    & ~STATUS_READY_FOR_DATA
			    || (status & STATUS_STATE) != STATUS_TRAN))
				error = SDMMC_STATE;
			if (error == SDMMC_OK
			    && tim_mode == SDMMC_TIM_MMC_HS_DDR)
				driver->card.bSpeedMode = tim_mode;
			else if (error) {
				TRACE_ERROR_1("Width/DDR %s\n\r",SD_StringifyRetCode(error));
				return error;
			}
		}
	}

	/* Enable HS200 timing mode */
	if (tim_mode == SDMMC_TIM_MMC_HS200 && driver->card.bBusMode > 1) {
		sw_arg.index = MMC_EXT_HS_TIMING_I;
		/* Select device output Driver Type-0, i.e. 50 ohm nominal
		 * output impedance.
		 * TODO select the optimal device output Driver Type.
		 * That depends on board design. Use an oscilloscope to observe
		 * signal integrity, and among the driver types that meet rise
		 * and fall time requirements, select the weakest. */
		sw_arg.value = MMC_EXT_HS_TIMING_HS200 | MMC_EXT_HS_TIMING_50R;
		error = MmcCmd6(driver, &sw_arg, &status);
		if (error == SDMMC_OK)
			error = HwSetHsMode(driver, tim_mode);
		if (error == SDMMC_OK) {
			error = Cmd13(driver, &status);
			if (error == SDMMC_OK && (status & ~STATUS_STATE
			    & ~STATUS_READY_FOR_DATA
			    || (status & STATUS_STATE) != STATUS_TRAN))
				error = SDMMC_STATE;
		}
		if (error == SDMMC_OK) {
			driver->card.bSpeedMode = tim_mode;
			freq = SdmmcGetMaxFreq(driver);
			error = HwSetClock(driver, &freq);
			driver->card.dwCurrSpeed = freq;
			error = error == SDMMC_CHANGED ? SDMMC_OK : error;
		}
		if (error != SDMMC_OK) {
			TRACE_ERROR_1("HS200 %s\n\r", SD_StringifyRetCode(error));
			return error;
		}
	}

	/* Update card information since status changed */
	flag = driver->card.bSpeedMode >= SDMMC_TIM_MMC_HS_SDR;
	if (flag || driver->card.bBusMode > 1)
		SdMmcUpdateInformation(driver, flag, true);

	/* MMC devices have the SET_BLOCK_COUNT command part of both the
	 * block-oriented read and the block-oriented write commands,
	 * i.e. class 2 and class 4 commands.
	 * FIXME we should normally check CSD.CCC before issuing any MMC block-
	 * oriented read/write command. */
	driver->card.bSetBlkCnt = 1;
	/* Ask the driver to implicitly send the SET_BLOCK_COUNT command,
	 * immediately before every READ_MULTIPLE_BLOCK and WRITE_MULTIPLE_BLOCK
	 * command. */
	driver->control_param =  driver->card.bSetBlkCnt;
	drv_err = sdmmc_device_control(driver,SDMMC_IOCTL_SET_LENPREFIX);

	/* In case the driver does not support this function, we'll take it in
	 * charge. */
	if (driver->card.bSetBlkCnt && drv_err == SDMMC_OK && driver->control_param)
		driver->card.bSetBlkCnt = 0;

	driver->card.wCurrBlockLen = SDMMC_BLOCK_SIZE;

	if (MMC_IsCSDVer1_2((&driver->card)) && MMC_IsVer4((&driver->card))) {
		/* Get size from EXT_CSD */
		if (MMC_EXT_DATA_SECTOR_SIZE(driver->card.EXT)
		    == MMC_EXT_DATA_SECT_4KIB)
			driver->card.wBlockSize = 4096;
		else
			driver->card.wBlockSize = 512;
		driver->card.dwNbBlocks = MMC_EXT_SEC_COUNT(driver->card.EXT)
		    / (driver->card.wBlockSize / 512UL);
		/* Device density >= 4 GiB does not fit 32-bit dwTotalSize */
		driver->card.dwTotalSize = MMC_EXT_SEC_COUNT(driver->card.EXT);
		if (driver->card.dwTotalSize >= 0x800000)
			driver->card.dwTotalSize = 0xFFFFFFFF;
		else
			driver->card.dwTotalSize *= 512UL;
	}
	else {
		driver->card.wBlockSize = 512;
		mem_size = SD_CSD_TOTAL_SIZE(driver->card.CSD);
		driver->card.dwNbBlocks = (uint32_t)(mem_size >> 9);
		driver->card.dwTotalSize = mem_size >> 32 ? 0xFFFFFFFF
		    : (uint32_t)mem_size;
	}

	/* Check device status and eat past exceptions, which would otherwise
	 * prevent upcoming data transaction routines from reliably checking
	 * fresh exceptions. */
	error = Cmd13(driver, &status);
	if (error)
		return error;
	status = status & ~STATUS_STATE & ~STATUS_READY_FOR_DATA & ~STATUS_APP_CMD;
	if (status) {
		TRACE_WARNING_1("st %lx\n\r", status);
	}

	return SDMMC_OK;
}
#endif
#endif
