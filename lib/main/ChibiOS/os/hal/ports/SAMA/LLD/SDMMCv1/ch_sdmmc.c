#include <string.h>
#include "hal.h"

#if (SAMA_USE_SDMMC == TRUE)

#include "sama_sdmmc_lld.h"
#include "ch_sdmmc_device.h"
#include "ch_sdmmc_cmds.h"
#include "ch_sdmmc_sdio.h"
#include "ch_sdmmc_sd.h"
#include "ch_sdmmc_mmc.h"

/** SD/MMC transfer rate unit codes (10K) list */
const uint16_t sdmmcTransUnits[8] = {
	10, 100, 1000, 10000,
	0, 0, 0, 0
};

/** SD transfer multiplier factor codes (1/10) list */
const uint8_t sdTransMultipliers[16] = {
	0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80
};



uint32_t SdmmcGetMaxFreq(SdmmcDriver *drv)
{
	uint32_t rate = 0;
	sSdCard * pSd = &drv->card;

#ifndef SDMMC_TRIM_MMC
	if ((pSd->bCardType & CARD_TYPE_bmSDMMC) == CARD_TYPE_bmMMC) {
		if (pSd->bSpeedMode == SDMMC_TIM_MMC_HS200)
			rate = 200000ul;
		else if (pSd->bSpeedMode == SDMMC_TIM_MMC_HS_DDR
		    || (pSd->bSpeedMode == SDMMC_TIM_MMC_HS_SDR
		    && MMC_EXT_CARD_TYPE(pSd->EXT) & 0x2))
			rate = 52000ul;
		else if (pSd->bSpeedMode == SDMMC_TIM_MMC_HS_SDR)
			rate = 26000ul;
		else
			rate = SdmmcDecodeTransSpeed(SD_CSD_TRAN_SPEED(pSd->CSD),
			    sdmmcTransUnits, mmcTransMultipliers);
	}
#endif

	if ((pSd->bCardType & CARD_TYPE_bmSDMMC) == CARD_TYPE_bmSD) {
		rate = SdmmcDecodeTransSpeed(SD_CSD_TRAN_SPEED(pSd->CSD),
		    sdmmcTransUnits, sdTransMultipliers);
		if (pSd->bSpeedMode == SDMMC_TIM_SD_SDR104 && rate == 200000ul)
			rate = 208000ul;
		else if (pSd->bSpeedMode == SDMMC_TIM_SD_DDR50)
			rate /= 2ul;
	}

	TRACE_DEBUG_1("SdmmcGetMaxFreq rate %d\r\n",rate);
	return rate * 1000ul;
}



/**
 * Update SD/MMC information.
 * Update CSD for card speed switch.
 * Update ExtDATA for any card function switch.
 * \param pSd      Pointer to a SD card driver instance.
 * \return error code when update CSD error.
 */
void SdMmcUpdateInformation(SdmmcDriver *drv, bool csd, bool extData)
{
	uint8_t error;

	/* Update CSD for new TRAN_SPEED value */
	if (csd) {

		SdMmcSelect(drv, 0, 1);

		/* Wait for 14 usec (or more) */
		t_usleep(drv,20);

		error = Cmd9(drv);

		if (error)
			return;

		SdMmcSelect(drv, drv->card.wAddress, 1);
	}
	if (extData) {
		if ((drv->card.bCardType & CARD_TYPE_bmSDMMC) == CARD_TYPE_bmSD)
			SdGetExtInformation(drv);
#ifndef SDMMC_TRIM_MMC
		else if ((drv->card.bCardType & CARD_TYPE_bmSDMMC) == CARD_TYPE_bmMMC)
			MmcGetExtInformation(drv);
#endif
	}
}


uint8_t SDMMC_Lib_SdStart(SdmmcDriver *drv, bool * retry)
{
	uint64_t mem_size;
	uint32_t freq;
	uint32_t drv_err, status;
	uint8_t error;
	bool flag;
	sSdCard *pSd = &drv->card;

	*retry = false;

	drv->card.bSpeedMode = drv->card.bCardSigLevel == 0 ? SDMMC_TIM_SD_SDR12 : SDMMC_TIM_SD_DS;
	drv->timeout_elapsed = -1;
	HwSetHsMode(drv, drv->card.bSpeedMode);

	/* Consider switching to low signaling level, as a prerequisite to
	 * switching to any UHS-I timing mode */
	if (drv->card.bCardSigLevel == 1) {

		error = Cmd11(drv, &status);

		if (error)
			return error;

		if ((status & STATUS_STATE) != STATUS_READY) {
			TRACE_1("st %lx\n\r", status);
		}

		drv->card.bCardSigLevel = 0;
		error = HwPowerDevice(drv, SDMMC_PWR_STD_VDD_LOW_IO);

		if (error)
			return error;

		error = HwSetHsMode(drv, SDMMC_TIM_SD_SDR12);

		if (error)
			return error;

		drv->card.bSpeedMode = SDMMC_TIM_SD_SDR12;
	}

	/* The host then issues the command ALL_SEND_CID (CMD2) to the card to get
	 * its unique card identification (CID) number.
	 * Card that is unidentified (i.e. which is in Ready State) sends its CID
	 * number as the response (on the CMD line). */
	error = Cmd2(drv);
	if (error)
		return error;

	/* Thereafter, the host issues CMD3 (SEND_RELATIVE_ADDR) asks the
	 * card to publish a new relative card address (RCA), which is shorter than
	 * CID and which is used to address the card in the future data transfer
	 * mode. Once the RCA is received the card state changes to the Stand-by
	 * State. At this point, if the host wants to assign another RCA number, it
	 * can ask the card to publish a new number by sending another CMD3 command
	 * to the card. The last published RCA is the actual RCA number of the
	 * card. */
	error = Cmd3(drv);
	if (error)
		return error;
	else {
		TRACE_DEBUG_1("RCA=%u\n\r",drv->card.wAddress );
	}

	/* SEND_CSD (CMD9) to obtain the Card Specific Data (CSD register),
	 * e.g. block length, card storage capacity, etc... */
	error = Cmd9(drv);
	if (error)
		return error;

	/* Now select the card, to TRAN state */
	error = SdMmcSelect(drv, drv->card.wAddress , 0);
	if (error)
		return error;

	/* Get extended information of the card */
	SdMmcUpdateInformation(drv, true, true);

	/* Enable more bus width Mode */
	error = SdDecideBuswidth(drv);
	if (error) {
		//trace_error("Bus width %s\n\r", SD_StringifyRetCode(error));
		return SDMMC_ERR;
	}

	/* Consider HS and UHS-I timing modes */
	error = SdEnableHighSpeed(drv);
	if (error) {
		*retry = error == (SDMMC_STATE && (&drv->card.bCardSigLevel != 0));
		return error;
	}

	/* Update card information since status changed */
	flag = drv->card.bSpeedMode != SDMMC_TIM_SD_DS
	    && drv->card.bSpeedMode != SDMMC_TIM_SD_SDR12;
	if (flag || drv->card.bBusMode > 1)
		SdMmcUpdateInformation(drv, flag, true);

	/* Find out if the device supports the SET_BLOCK_COUNT command.
	 * SD devices advertise in SCR.CMD_SUPPORT whether or not they handle
	 * the SET_BLOCK_COUNT command. */
	drv->card.bSetBlkCnt = SD_SCR_CMD23_SUPPORT(pSd->SCR);
	/* Now, if the device does not support the SET_BLOCK_COUNT command, then
	 * the legacy STOP_TRANSMISSION command shall be issued, though not at
	 * the same timing. */
	if (!drv->card.bSetBlkCnt) {
		/* In case the driver does not automatically issue the
		 * STOP_TRANSMISSION command, we'll have to do it ourselves. */

		drv->control_param = 0;
		drv_err = sdmmc_device_control(drv, SDMMC_IOCTL_GET_XFERCOMPL);

		if (drv_err != SDMMC_OK || !drv->control_param)
			drv->card.bStopMultXfer = 1;
	}
	/* Ask the driver to implicitly send the SET_BLOCK_COUNT command,
	 * immediately before every READ_MULTIPLE_BLOCK and WRITE_MULTIPLE_BLOCK
	 * command. Or, if the current device does not support SET_BLOCK_COUNT,
	 * instruct the driver to stop using this command. */
	drv->control_param = pSd->bSetBlkCnt;

	drv_err = sdmmc_device_control(drv,SDMMC_IOCTL_SET_LENPREFIX);

	/* In case the driver does not support this function, we'll take it in
	 * charge. */
	if (drv->card.bSetBlkCnt && drv_err == SDMMC_OK && drv->control_param)
		drv->card.bSetBlkCnt = 0;

	/* In the case of a Standard Capacity SD Memory Card, this command sets the
	 * block length (in bytes) for all following block commands
	 * (read, write, lock).
	 * Default block length is fixed to 512 Bytes.
	 * Set length is valid for memory access commands only if partial block read
	 * operation are allowed in CSD.
	 * In the case of a High Capacity SD Memory Card, block length set by CMD16
	 * command does not affect the memory read and write commands. Always 512
	 * Bytes fixed block length is used. This command is effective for
	 * LOCK_UNLOCK command.
	 * In both cases, if block length is set larger than 512Bytes, the card sets
	 * the BLOCK_LEN_ERROR bit. */
	if (drv->card.bCardType == CARD_SD) {
		error = Cmd16(drv, SDMMC_BLOCK_SIZE);
		if (error)
			return error;
	}
	drv->card.wCurrBlockLen = SDMMC_BLOCK_SIZE;

	if (SD_CSD_STRUCTURE(pSd->CSD) >= 1) {
		drv->card.wBlockSize = 512;
		mem_size = SD_CSD_BLOCKNR_HC(pSd->CSD);
		drv->card.dwNbBlocks = mem_size >> 32 ? 0xFFFFFFFF : (uint32_t)mem_size;
		if (drv->card.dwNbBlocks >= 0x800000)
			drv->card.dwTotalSize = 0xFFFFFFFF;
		else
			drv->card.dwTotalSize = drv->card.dwNbBlocks * 512UL;
	}
	else {
		drv->card.wBlockSize = 512;
		mem_size = SD_CSD_TOTAL_SIZE(pSd->CSD);
		drv->card.dwNbBlocks = (uint32_t)(mem_size >> 9);
		drv->card.dwTotalSize = mem_size >> 32 ? 0xFFFFFFFF
		    : (uint32_t)mem_size;
	}


	/* Automatically select the max device clock frequency */
	/* Calculate transfer speed */
	freq = SdmmcGetMaxFreq(drv);

#ifndef SDMMC_TRIM_SDIO
	if (drv->card.bCardType & CARD_TYPE_bmSDIO) {
		freq = min_u32(freq, SdioGetMaxFreq(drv));
		TRACE_INFO_1("selecting sdio freq%d\r\n",freq);
	}
#endif
	error = HwSetClock(drv, &freq);
	drv->card.dwCurrSpeed = freq;
	if (error != SDMMC_OK && error != SDMMC_CHANGED) {
		TRACE_ERROR_1("error clk %s\n\r", SD_StringifyRetCode(error));
		return error;
	}

	/* Check device status and eat past exceptions, which would otherwise
	 * prevent upcoming data transaction routines from reliably checking
	 * fresh exceptions. */
	error = Cmd13(drv, &status);
	if (error)
		return error;
	status = status & ~STATUS_STATE & ~STATUS_READY_FOR_DATA  & ~STATUS_APP_CMD;

	//warning
	if (status) {
		TRACE_WARNING_1("warning st %lx\n\r", status);
	}

	return SDMMC_OK;
}



/**
 * \brief Run the SD/MMC/SDIO Mode initialization sequence.
 * This function runs the initialization procedure and the identification
 * process. Then it leaves the card in ready state. The following procedure must
 * check the card type and continue to put the card into tran(for memory card)
 * or cmd(for io card) state for data exchange.
 * \param pSd  Pointer to a SD card driver instance.
 * \return 0 if successful; otherwise returns an \ref sdmmc_rc "SD_ERROR code".
 */
uint8_t SdMmcIdentify(SdmmcDriver *drv)
{
	uint8_t error;
	bool high_capacity;
	uint8_t dev_type = CARD_UNKNOWN;
	bool sd_v2 = false;


#ifndef SDMMC_TRIM_SDIO
	/* Reset SDIO: CMD52, write 1 to RES bit in CCCR */
	{
		uint32_t status = SDIO_RES;

		error = Cmd52(drv, 1, SDIO_CIA, 0, SDIO_IOA_REG, &status);

		if ((error && error != SDMMC_NO_RESPONSE) || (!error && status & STATUS_SDIO_R5)) {
			TRACE_2("IOrst %s st %lx\n\r",
			    SD_StringifyRetCode(error), status);
		}
	}
#endif


	/* Reset MEM: CMD0 */
	error = Cmd0(drv, 0);

	t_msleep(drv,200);

	if (error) {
		TRACE_1("rst %s\n\r", SD_StringifyRetCode(error));
	}


	/* CMD8 is newly added in the Physical Layer Specification Version 2.00 to
	 * support multiple voltage ranges and used to check whether the card
	 * supports supplied voltage. The version 2.00 host shall issue CMD8 and
	 * verify voltage before card initialization.
	 * The host that does not support CMD8 shall supply high voltage range... */


	error = SdCmd8(drv, SD_IFC_VHS_27_36 >> SD_IFC_VHS_Pos);

	if (!error)
		sd_v2 = true;
	else if (error != SDMMC_NO_RESPONSE)
		return SDMMC_ERR;
	else {
		/* No response to CMD8. Wait for 130 usec (or more). */
		t_usleep(drv,200);
	}


#ifndef SDMMC_TRIM_SDIO
	/* CMD5 is newly added for SDIO initialize & power on */
	{
		uint32_t status = 0;

		error = Cmd5(drv, &status);

		if (!error && (status & SDIO_OCR_NF) > 0) {

			//int8_t elapsed;

			/* Card has SDIO function. Wait until it raises the
			 * IORDY flag, which may take up to 1s, i.e. 1000 system
			 * ticks. */
			for (drv->timeout_elapsed = 0;
			    !(status & SD_OCR_BUSYN) && !error && !drv->timeout_elapsed; ) {


				status &= SD_HOST_VOLTAGE_RANGE;
				error = Cmd5(drv, &status);
			}
			if (!(status & SD_OCR_BUSYN) && !error)
				error = SDMMC_BUSY;
			if (error) {
				TRACE_1("SDIO oc %s\n\r",SD_StringifyRetCode(error));
				return SDMMC_ERR;
			}
			TRACE("SDIO\n\r");
			dev_type = status & SDIO_OCR_MP ? CARD_SDCOMBO : CARD_SDIO;
		}
	}
#endif

	if (dev_type != CARD_SDIO) {
		/* The device should have memory (MMC or SD or COMBO).
		 * Try to initialize SD memory. */
		bool low_sig_lvl = HwIsTimingSupported(drv, SDMMC_TIM_SD_SDR12);

		high_capacity = sd_v2;
		error = Acmd41(drv, &low_sig_lvl, &high_capacity);
		if (!error) {
			TRACE_1("SD%s MEM\n\r", high_capacity ? "HC" : "");
			dev_type |= high_capacity ? CARD_SDHC : CARD_SD;
			if (drv->card.bCardSigLevel == 2 && low_sig_lvl)
				drv->card.bCardSigLevel = 1;
		}
		else if (dev_type == CARD_SDCOMBO)
			dev_type = CARD_SDIO;
	}

#ifndef SDMMC_TRIM_MMC
	if (dev_type == CARD_UNKNOWN) {
		/* Try MMC initialize */
		uint8_t count;

		for (error = SDMMC_NO_RESPONSE, count = 0;
		    error == SDMMC_NO_RESPONSE && count < 10;
		    count++)
			error = Cmd0(drv, 0);
		if (error) {
			TRACE_1("MMC rst %s\n\r",
			    SD_StringifyRetCode(error));
			return SDMMC_ERR;
		}
		high_capacity = false;
		error = Cmd1(drv, &high_capacity);
		if (error) {
			TRACE_1("MMC oc %s\n\r",
			    SD_StringifyRetCode(error));
			return SDMMC_ERR;
		}
		/* MMC card identification OK */
		TRACE("MMC\n\r");
		dev_type = high_capacity ? CARD_MMCHD : CARD_MMC;
	}
#endif

	if (dev_type == CARD_UNKNOWN) {
		TRACE("Unknown card\n\r");
		return SDMMC_ERR;
	}
	drv->card.bCardType = dev_type;
	return 0;
}



/**
 * Switch card state between STBY and TRAN (or CMD and TRAN)
 * \param pSd       Pointer to a SD card driver instance.
 * \param address   Card address to TRAN, 0 to STBY
 * \param statCheck Whether to check the status before CMD7.
 */
uint8_t SdMmcSelect(SdmmcDriver *drv, uint16_t address, uint8_t statCheck)
{
	uint8_t error;
	uint32_t status, currState;
	uint32_t targetState = address ? STATUS_TRAN : STATUS_STBY;
	uint32_t srcState = address ? STATUS_STBY : STATUS_TRAN;

	/* At this stage the Initialization and identification process is achieved
	 * The SD card is supposed to be in Stand-by State */
	while (statCheck) {
		error = Cmd13(drv, &status);
		if (error)
			return error;
		if (status & STATUS_READY_FOR_DATA) {
			currState = status & STATUS_STATE;
			if (currState == targetState)
				return 0;
			if (currState != srcState) {
				TRACE_ERROR_1("st %lx\n\r", currState);
				return SDMMC_ERR;
			}
			break;
		}
	}

	/* Switch to Transfer state. Select the current SD/MMC
	 * so that SD ACMD6 can process or EXT_CSD can read. */
	error = Cmd7(drv, address);
	return error;
}

/**
 * \brief Decode Trans Speed Value
 * \param code The trans speed code value.
 * \param unitCodes  Unit list in 10K, 0 as unused value.
 * \param multiCodes Multiplier list in 1/10, index 1 ~ 15 is valid.
 */
uint32_t SdmmcDecodeTransSpeed(uint32_t code,
		      const uint16_t * unitCodes, const uint8_t * multiCodes)
{
	uint32_t speed;
	uint8_t unitI, mulI;

	/* Unit code is valid ? */
	unitI = code & 0x7;
	if (unitCodes[unitI] == 0)
		return 0;

	/* Multi code is valid ? */
	mulI = (code >> 3) & 0xF;
	if (multiCodes[mulI] == 0)
		return 0;

	speed = (uint32_t)unitCodes[unitI] * multiCodes[mulI];
	return speed;
}

#endif

