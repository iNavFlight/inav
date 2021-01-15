#include "hal.h"

#if (SAMA_USE_SDMMC == TRUE)

#include "sama_sdmmc_lld.h"
#include "ch_sdmmc_device.h"
#include "ch_sdmmc_sdio.h"
#include "ch_sdmmc_cmds.h"
#include "ch_sdmmc_sd.h"

#ifndef SDMMC_TRIM_SDIO





uint8_t SdioInit(SdmmcDriver *driver)
{
	uint32_t freq;
	uint8_t error;

	driver->card.bSpeedMode = SDMMC_TIM_SD_DS;

	HwSetHsMode(driver, driver->card.bSpeedMode);
	/* Thereafter, the host issues CMD3 (SEND_RELATIVE_ADDR) asks the
	 * card to publish a new relative card address (RCA), which is shorter than
	 * CID and which is used to address the card in the future data transfer
	 * mode. Once the RCA is received the card state changes to the Stand-by
	 * State. At this point, if the host wants to assign another RCA number, it
	 * can ask the card to publish a new number by sending another CMD3 command
	 * to the card. The last published RCA is the actual RCA number of the
	 * card. */
	error = Cmd3(driver);

	if (error)
		return error;

		TRACE_1("RCA=%u\n\r", driver->card.wAddress);

	/* Now select the card, to TRAN state */
	error = SdMmcSelect(driver,driver->card.wAddress, 0);
	if (error)
		return error;

	/* Enable more bus width Mode */
	error = SdDecideBuswidth(driver);
	if (error) {
		TRACE_1("Bus width %s\n\r", SD_StringifyRetCode(error));
		return SDMMC_ERR;
	}

	/* Consider High-Speed timing mode */
	error = SdEnableHighSpeed(driver);
	if (error)
		return error;

	/* Increase device clock frequency */
	freq = SdioGetMaxFreq(driver);

	error = HwSetClock(driver, &freq);
	driver->card.dwCurrSpeed = freq;
	if (error != SDMMC_OK && error != SDMMC_CHANGED) {
		TRACE_1("clk %s\n\r", SD_StringifyRetCode(error));
		return error;
	}

	return SDMMC_OK;
}



/**
 * Read one or more bytes from SDIO card, using RW_DIRECT command.
 * \param pSd         Pointer to SdCard instance.
 * \param functionNum Function number.
 * \param address     First register address to read from.
 * \param pData       Pointer to data buffer.
 * \param size        Buffer size, number of bytes to read.
 * \return 0 if successful; otherwise returns an \ref sdmmc_rc "error code".
 */
uint8_t SDIO_ReadDirect(SdmmcDriver *sdmmcp,
		uint8_t functionNum,
		uint32_t address, uint8_t * pData, uint32_t size)
{
	uint8_t error;
	uint32_t status;

	sSdCard *pSd = &sdmmcp->card;

	if (pSd->bCardType & CARD_TYPE_bmSDIO) {
		if (size == 0)
			return SDMMC_PARAM;
		while (size--) {
			status = 0;
			error =
			    Cmd52(sdmmcp, 0, functionNum, 0, address++, &status);
			if (pData)
				*pData++ = (uint8_t) status;
			if (error || status & STATUS_SDIO_R5) {
				//trace_error("IOrdRegs %luB@%lu %s st %lx\n\r",
				//    size, address, SD_StringifyRetCode(error),
				 //   status);
				return SDMMC_ERR;
			}
		}
	} else {
		return SDMMC_NOT_SUPPORTED;
	}
	return 0;
}



/**
 * Find SDIO ManfID, Fun0 tuple.
 * \param pSd         Pointer to \ref sSdCard instance.
 * \param address     Search area start address.
 * \param size        Search area size.
 * \param pAddrManfID Pointer to ManfID address value buffer.
 * \param pAddrFunc0  Pointer to Func0 address value buffer.
 */
uint8_t SdioFindTuples(SdmmcDriver *sdmmcp,
	       uint32_t address, uint32_t size,
	       uint32_t * pAddrManfID, uint32_t * pAddrFunc0)
{
	uint8_t error, tmp[3];
	uint32_t addr = address;
	uint8_t flagFound = 0;	/* 1:Manf, 2:Func0 */
	uint32_t addManfID = 0, addFunc0 = 0;
	for (; flagFound != 3;) {
		error = SDIO_ReadDirect(sdmmcp, SDIO_CIA, addr, tmp, 3);
		if (error)
			return error;
		/* End */
		if (tmp[0] == CISTPL_END)
			break;
		/* ManfID */
		else if (tmp[0] == CISTPL_MANFID) {
			flagFound |= 1;
			addManfID = addr;
		}
		/* Func0 */
		else if (tmp[0] == CISTPL_FUNCE && tmp[2] == 0x00) {
			flagFound |= 2;
			addFunc0 = addr;
		}
		/* Tuple error ? */
		if (tmp[1] == 0)
			break;
		/* Next address */
		addr += (tmp[1] + 2);
		if (addr > (address + size))
			break;
	}
	if (pAddrManfID)
		*pAddrManfID = addManfID;
	if (pAddrFunc0)
		*pAddrFunc0 = addFunc0;
	return 0;
}


uint32_t SdioGetMaxFreq(SdmmcDriver *sdmmcp)
{
	uint8_t error;
	uint32_t addr = 0, rate;
	uint8_t buf[6];

	/* Check Func0 tuple in CIS area */
	error = SDIO_ReadDirect(sdmmcp, SDIO_CIA, SDIO_CIS_PTR_REG,(uint8_t *)&addr, 3);

	if (error)
		return 0;

	error = SdioFindTuples(sdmmcp, addr, 256, NULL, &addr);

	if (error || !addr)
		return 0;

	/* Fun0 tuple: fn0_blk_siz & max_tran_speed */
	error = SDIO_ReadDirect(sdmmcp, SDIO_CIA, addr, buf, 6);

	if (error)
		return 0;

	rate = SdmmcDecodeTransSpeed(buf[5], sdmmcTransUnits,
	    sdTransMultipliers);

	if (sdmmcp->card.bSpeedMode == SDMMC_TIM_SD_SDR104 && rate == 200000ul)
		rate = 208000ul;
	else if (sdmmcp->card.bSpeedMode == SDMMC_TIM_SD_DDR50)
		rate /= 2ul;
	else if (sdmmcp->card.bSpeedMode == SDMMC_TIM_SD_HS && rate == 25000ul)
		rate *= 2ul;

	return rate * 1000ul;
}

/**
 * Display SDIO card informations (CIS, tuple ...)
 * \param pSd Pointer to \ref sSdCard instance.
 */
void SDIO_DumpCardInformation(SdmmcDriver *sdmmcp)
{
	uint32_t tmp = 0, addrCIS = 0, addrManfID = 0, addrFuncE = 0;
	uint8_t *p = (uint8_t *) & tmp;
	uint8_t buf[16];

	/* CCCR */
	_PrintTitle("CCCR");
	SDIO_ReadDirect(sdmmcp, SDIO_CIA, SDIO_CCCR_REG, p, 1);
	_PrintField("SDIO 0x%02lX", (tmp & SDIO_SDIO) >> 4);
	_PrintField("CCCR 0x%02lX", (tmp & SDIO_CCCR) >> 0);
	SDIO_ReadDirect(sdmmcp, SDIO_CIA, SDIO_SD_REV_REG, p, 1);
	_PrintField("SD 0x%02lX", (tmp & SDIO_SD) >> 0);
	SDIO_ReadDirect(sdmmcp, SDIO_CIA, SDIO_IOE_REG, p, 1);
	_PrintField("IOE 0x%02lX", (tmp & SDIO_IOE) >> 0);
	SDIO_ReadDirect(sdmmcp, SDIO_CIA, SDIO_IOR_REG, p, 1);
	_PrintField("IOR 0x%02lX", (tmp & SDIO_IOR) >> 0);
	SDIO_ReadDirect(sdmmcp, SDIO_CIA, SDIO_IEN_REG, p, 1);
	_PrintField("IEN 0x%02lX", (tmp & SDIO_IEN) >> 0);
	SDIO_ReadDirect(sdmmcp, SDIO_CIA, SDIO_INT_REG, p, 1);
	_PrintField("INT %lu",     (tmp & SDIO_INT) >> 0);
	SDIO_ReadDirect(sdmmcp, SDIO_CIA, SDIO_BUS_CTRL_REG, p, 1);
	_PrintField("CD 0x%lX",   (tmp & SDIO_CD) >> 7);
	_PrintField("SCSI 0x%lX",   (tmp & SDIO_SCSI) >> 6);
	_PrintField("ECSI 0x%lX",   (tmp & SDIO_ECSI) >> 5);
	_PrintField("BUS_WIDTH 0x%lX", (tmp & SDIO_BUSWIDTH) >> 0);
	SDIO_ReadDirect(sdmmcp, SDIO_CIA, SDIO_CAP_REG, p, 1);
	_PrintField("4BLS 0x%lX",   (tmp & SDIO_4BLS) >> 7);
	_PrintField("LSC 0x%lX",   (tmp & SDIO_LSC) >> 6);
	_PrintField("E4MI 0x%lX",   (tmp & SDIO_E4MI) >> 5);
	_PrintField("S4MI 0x%lX",   (tmp & SDIO_S4MI) >> 4);
	_PrintField("SBS 0x%lX",   (tmp & SDIO_SBS) >> 3);
	_PrintField("SRW 0x%lX",   (tmp & SDIO_SRW) >> 2);
	_PrintField("SMB 0x%lX",   (tmp & SDIO_SMB) >> 1);
	_PrintField("SDC 0x%lX",   (tmp & SDIO_SDC) >> 0);
	SDIO_ReadDirect(sdmmcp, SDIO_CIA, SDIO_CIS_PTR_REG, p, 3);
	_PrintField("CIS_PTR 0x%06lX", tmp);
	addrCIS = tmp;
	tmp = 0;
	SDIO_ReadDirect(sdmmcp, SDIO_CIA, SDIO_BUS_SUSP_REG, p, 1);
	_PrintField("BR 0x%lX",   (tmp & SDIO_BR) >> 1);
	_PrintField("BS 0x%lX",   (tmp & SDIO_BS) >> 0);
	SDIO_ReadDirect(sdmmcp, SDIO_CIA, SDIO_FUN_SEL_REG, p, 1);
	_PrintField("DF 0x%lX",   (tmp & SDIO_DF) >> 7);
	_PrintField("FS 0x%lX",   (tmp & SDIO_FS) >> 0);
	SDIO_ReadDirect(sdmmcp, SDIO_CIA, SDIO_EXEC_REG, p, 1);
	_PrintField("EX 0x%lX",   (tmp & SDIO_EX) >> 0);
	_PrintField("EXM 0x%lX",   (tmp & SDIO_EXM) >> 0);
	SDIO_ReadDirect(sdmmcp, SDIO_CIA, SDIO_READY_REG, p, 1);
	_PrintField("RF 0x%lX",   (tmp & SDIO_RF) >> 1);
	_PrintField("RFM 0x%lX",   (tmp & SDIO_RFM) >> 0);
	SDIO_ReadDirect(sdmmcp, SDIO_CIA, SDIO_FN0_BLKSIZ_REG, p, 2);
	_PrintField("FN0_SIZE %lu", tmp);
	tmp = 0;
	SDIO_ReadDirect(sdmmcp, SDIO_CIA, SDIO_POWER_REG, p, 1);
	_PrintField("EMPC 0x%lX",   (tmp & SDIO_EMPC) >> 1);
	_PrintField("SMPC 0x%lX",   (tmp & SDIO_SMPC) >> 0);
	SDIO_ReadDirect(sdmmcp, SDIO_CIA, SDIO_HS_REG, p, 1);
	_PrintField("EHS 0x%lX",   (tmp & SDIO_EHS) >> 1);
	_PrintField("SHS 0x%lX",   (tmp & SDIO_SHS) >> 0);
	/* Metaformat */
	SdioFindTuples(sdmmcp, addrCIS, 128, &addrManfID, &addrFuncE);
	if (addrManfID != 0) {
		SDIO_ReadDirect(sdmmcp, SDIO_CIA, addrManfID, buf, 6);
		_PrintTitle("CISTPL_MANFID");
		_PrintField("MANF 0x%04X", (uint16_t)buf[3] << 8 | buf[2]);
		_PrintField("CARD 0x%04X", (uint16_t)buf[5] << 8 | buf[4]);
	}
	if (addrFuncE != 0) {
		SDIO_ReadDirect(sdmmcp, SDIO_CIA, addrFuncE, buf, 6);
		_PrintTitle("CISTPL_FUNCE Fun0");
		_PrintField("BL_SIZE %u", (uint16_t)buf[4] << 8 | buf[3]);
		_PrintField("MAX_TRAN_SPD 0x%02X", buf[5]);
	}
	/* I/O function 1 */
	SDIO_ReadDirect(sdmmcp, SDIO_CIA, SDIO_FBR_ADDR(1, SDIO_FBR_CIS_PTR),
	    p, 3);
	addrFuncE = 0;
	/* TODO Augment SdioFindTuples so it finds CISTPL_FUNCE for Function 1
	 * with Extended Data 01h */
	SdioFindTuples(sdmmcp, tmp, 256, NULL, &addrFuncE);
	if (addrFuncE != 0) {
		SDIO_ReadDirect(sdmmcp, SDIO_CIA, addrFuncE, buf, 16);
		_PrintTitle("CISTPL_FUNCE Fun1");
		_PrintField("MAX_BLK_SIZE %u", (uint16_t)buf[0xf] << 8
		    | buf[0xe]);
	}
}
#endif

#endif
