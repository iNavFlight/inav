#include <string.h>
#include "hal.h"

#if (SAMA_USE_SDMMC == TRUE)

#include "sama_sdmmc_lld.h"
#include "ch_sdmmc_device.h"
#include "ch_sdmmc_cmds.h"
#include "ch_sdmmc_sd.h"


/** sdmmc_opc_acc SD_SEND_OP_COND command argument fields
 */
#define SD_OPC_S18R             (1ul << 24)	/**< Switching to 1.8V signaling level Request */
#define SD_OPC_XPC              (1ul << 28)	/**< SDXC Power Control */
#define SD_OPC_FB               (1ul << 29)	/**< eSD Fast Boot */
#define SD_OPC_HCS              (1ul << 30)	/**< Host Capacity Support */
/**
 *   sdmmc_cmd_op SD/MMC Command Operations
 */
#define SDMMC_CMD_bmPOWERON     (0x1      ) /**< Do Power ON sequence */
#define SDMMC_CMD_bmCOMMAND     (0x1 <<  1) /**< Send command */
#define SDMMC_CMD_bmDATAMASK    (0x3 <<  2) /**< Data operation mask */
#define SDMMC_CMD_bmNODATA      (0x0 <<  2) /**< No data transfer */
#define SDMMC_CMD_RX             0x1	    /**< data RX */
#define SDMMC_CMD_bmDATARX      (0x1 <<  2) /**< Bits for data RX */
#define SDMMC_CMD_TX             0x2	    /**< data TX */
#define SDMMC_CMD_bmDATATX      (0x2 <<  2) /**< Bits for data TX */
#define SDMMC_CMD_STOPXFR        0x3	    /**< data stop */
#define SDMMC_CMD_bmSTOPXFR     (0x3 <<  2) /**< Bits for transfer stop */
#define SDMMC_CMD_bmRESPMASK    (0x7 <<  4) /**< Bits masks response option */
#define SDMMC_CMD_bmRESP(R)     (((R)&0x7) << 4)    /**< Bits setup response type: 1 for R1, 2 for R2, ... 7 for R7 */

#define SDMMC_CMD_bmCRC         (0x1 <<  7) /**< CRC is enabled (SPI only) */
#define SDMMC_CMD_bmOD          (0x1 <<  8) /**< Open-Drain is enabled (MMC) */
#define SDMMC_CMD_bmIO          (0x1 <<  9) /**< IO function */
#define SDMMC_CMD_bmBUSY        (0x1 << 10) /**< Do busy check */

/** Cmd: Do power on initialize */
#define SDMMC_CMD_POWERONINIT   (SDMMC_CMD_bmPOWERON)
/** Cmd: Data only, read */
#define SDMMC_CMD_DATARX        (SDMMC_CMD_bmDATARX)
/** Cmd: Data only, write */
#define SDMMC_CMD_DATATX        (SDMMC_CMD_bmDATATX)
/** Cmd: Command without data */
#define SDMMC_CMD_CNODATA(R)    ( SDMMC_CMD_bmCOMMAND \
                                | SDMMC_CMD_bmRESP(R) )
/** Cmd: Command with data, read */
#define SDMMC_CMD_CDATARX(R)    ( SDMMC_CMD_bmCOMMAND \
                                | SDMMC_CMD_bmDATARX \
                                | SDMMC_CMD_bmRESP(R) )
/** Cmd: Command with data, write */
#define SDMMC_CMD_CDATATX(R)    ( SDMMC_CMD_bmCOMMAND \
                                | SDMMC_CMD_bmDATATX \
                                | SDMMC_CMD_bmRESP(R) )
/** Cmd: Send Stop command */
#define SDMMC_CMD_CSTOP         ( SDMMC_CMD_bmCOMMAND \
                                | SDMMC_CMD_bmSTOPXFR \
                                | SDMMC_CMD_bmRESP(1) )
/** Cmd: Send Stop token for SPI */
#define SDMMC_CMD_STOPTOKEN     (SDMMC_CMD_bmSTOPXFR)


#define STATUS_MMC_SWITCH ((uint32_t)( STATUS_CARD_IS_LOCKED \
                            | STATUS_COM_CRC_ERROR \
                            | STATUS_ILLEGAL_COMMAND \
                            | STATUS_CC_ERROR \
                            | STATUS_ERROR \
                            | STATUS_ERASE_RESET \
                            /*| STATUS_STATE*/ \
                            /*| STATUS_READY_FOR_DATA*/ \
                            | STATUS_SWITCH_ERROR ))


static void _ResetCmd(sSdmmcCommand * pCmd);

/**
 * Initialization delay: The maximum of 1 msec, 74 clock cycles and supply ramp
 * up time.
 * Returns the command transfer result (see SendMciCommand).
 */
uint8_t CmdPowerOn(SdmmcDriver *drv)
{
	//sSdmmcCommand *pCmd = &pSd->sdCmd;
	uint8_t bRc;

	TRACE("PwrON\n\r");

	_ResetCmd(&drv->cmd);

	/* Fill command */
	drv->cmd.cmdOp.wVal = SDMMC_CMD_POWERONINIT;
	drv->timeout_elapsed = -1;
	/* Send command */
	bRc = sdmmcSendCmd(drv);


	return bRc;
}





/**
 * Resets all cards to idle state
 * \param drv Pointer to \ref SdmmcDriver instance.
 * \param arg  Argument used.
 * \return the command transfer result (see SendMciCommand).
 */
uint8_t Cmd0(SdmmcDriver *drv, uint8_t arg)
{
	uint8_t bRc;

	_ResetCmd(&drv->cmd);

	/* Fill command */
	drv->cmd.cmdOp.wVal = SDMMC_CMD_CNODATA(0);
	drv->cmd.dwArg = arg;
	drv->timeout_elapsed = -1;
	bRc = sdmmcSendCmd(drv);


	return bRc;
}

/**
 * MMC send operation condition command.
 * Sends host capacity support information and activates the card's
 * initialization process.
 * Returns the command transfer result (see SendMciCommand).
 * \param drv Pointer to \ref SdmmcDriver instance.
 * \param hc  Upon success tells whether the device is a high capacity device.
 */
#ifndef SDMMC_TRIM_MMC
uint8_t Cmd1(SdmmcDriver *drv, bool * hc)
{
	sSdmmcCommand *pCmd = &drv->cmd;
	uint32_t arg, ocr = 0;
	uint8_t rc;
	//int8_t elapsed = -1;

	/* Tell the device that the host supports 512-byte sector addressing */
	arg = MMC_OCR_ACCESS_SECTOR;
	/* Tell the MMC device which voltage the host supplies to the VDD line
	 * (MMC card) or VCC line (e.MMC device).
	 * TODO get this board-specific value from platform code. On the
	 * SAMA5D2-XULT board, VDD is 3.3V � 1%. */
	arg |= SD_OCR_VDD_32_33 | SD_OCR_VDD_33_34;

	/* Fill command */
	_ResetCmd(&drv->cmd);

	pCmd->cmdOp.wVal = SDMMC_CMD_CNODATA(3) | SDMMC_CMD_bmOD;
	pCmd->bCmd = 1;
	pCmd->dwArg = arg;
	pCmd->pResp = &ocr;

	drv->timeout_ticks = TIME_S2I(1);
	drv->timeout_elapsed = 0;
	do {
		rc = sdmmcSendCmd(drv);

		if (rc == SDMMC_OK && !(ocr & SD_OCR_BUSYN))
			rc = SDMMC_BUSY;

	}
	while (rc == SDMMC_BUSY && !drv->timeout_elapsed);

	if (rc != SDMMC_OK)
		return rc;

	/* Analyze the final contents of the OCR Register */
#if 0
	TRACE("Device supports%s%s 3.0V:[%c%c%c%c%c%c]"
	    " 3.3V:[%c%c%c%c%c%c]\n\r",
	    ocr & MMC_OCR_VDD_170_195 ? " 1.8V" : "",
	    ocr & MMC_OCR_VDD_200_270 ? " 2.xV" : "",
	    ocr & SD_OCR_VDD_27_28 ? 'X' : '.',
	    ocr & SD_OCR_VDD_28_29 ? 'X' : '.',
	    ocr & SD_OCR_VDD_29_30 ? 'X' : '.',
	    ocr & SD_OCR_VDD_30_31 ? 'X' : '.',
	    ocr & SD_OCR_VDD_31_32 ? 'X' : '.',
	    ocr & SD_OCR_VDD_32_33 ? 'X' : '.',
	    ocr & SD_OCR_VDD_30_31 ? 'X' : '.',
	    ocr & SD_OCR_VDD_31_32 ? 'X' : '.',
	    ocr & SD_OCR_VDD_32_33 ? 'X' : '.',
	    ocr & SD_OCR_VDD_33_34 ? 'X' : '.',
	    ocr & SD_OCR_VDD_34_35 ? 'X' : '.',
	    ocr & SD_OCR_VDD_35_36 ? 'X' : '.');
#endif
	TRACE_INFO_1("Device access 0x%lx\n\r", ocr >> 29 & 0x3ul);

	*hc = (ocr & MMC_OCR_ACCESS_MODE) == MMC_OCR_ACCESS_SECTOR? true : false;

	return SDMMC_OK;
}
#endif

/**
 * Asks any card to send the CID numbers
 * on the CMD line (any card that is
 * connected to the host will respond)
 * Returns the command transfer result (see SendMciCommand).
 * \param drv Pointer to \ref SdmmcDriver instance.
 */
uint8_t Cmd2(SdmmcDriver *drv)
{
	//sSdmmcCommand *pCmd = &pSd->sdCmd;
	uint8_t bRc;

	_ResetCmd(&drv->cmd);

	/* Fill command */
	drv->cmd.cmdOp.wVal = SDMMC_CMD_CNODATA(2) | SDMMC_CMD_bmOD;
	drv->cmd.bCmd = 2;
	drv->cmd.pResp = drv->card.CID;
	drv->timeout_elapsed = -1;
	/* Send command */
	bRc = sdmmcSendCmd(drv);

	return bRc;
}

/**
 * Switches the mode of operation of the selected card.
 * CMD6 is valid under the "trans" state.
 * \return The command transfer result (see SendMciCommand).
 * \param drv Pointer to \ref SdmmcDriver instance.
 * \param  pSwitchArg  Pointer to the SWITCH_FUNC command argument.
 * \param  pStatus     Pointer to where the 512bit status is returned.
 * The buffer shall follow the peripheral and DMA alignment requirements.
 * \param  pResp       Pointer to where the response is returned.
 */
uint8_t SdCmd6(SdmmcDriver *drv, const SdCmd6Arg * pSwitchArg, uint8_t * pStatus, uint32_t * pResp)
{
	sSdmmcCommand *pCmd = &drv->cmd;
	uint8_t bRc;

	//assert(pSd);
	//assert(pSwitchArg);

	_ResetCmd(&drv->cmd);

	pCmd->bCmd = 6;
	pCmd->cmdOp.wVal = SDMMC_CMD_CDATARX(1);

	pCmd->dwArg = (pSwitchArg->set << 31)
	    | (pSwitchArg->reserved << 30)
	    | (pSwitchArg->func_grp6 << 20)
	    | (pSwitchArg->func_grp5 << 16)
	    | (pSwitchArg->pwr_limit << 12)
	    | (pSwitchArg->drv_strgth << 8)
	    | (pSwitchArg->cmd_sys << 4)
	    | (pSwitchArg->acc_mode << 0);

	if (pStatus) {
		pCmd->wBlockSize = 512 / 8;
		pCmd->wNbBlocks = 1;
		pCmd->pData = pStatus;
	}
	pCmd->pResp = pResp;
	drv->timeout_elapsed = -1;
	bRc = sdmmcSendCmd(drv);
	return bRc;
}

/**
 * Ask the SD card to publish a new relative address (RCA)
 *         or
 * Assign relative address to the MMC card
 * Returns the command transfer result (see SendMciCommand).
 * \param drv Pointer to \ref SdmmcDriver instance.
 * \param pRsp  Pointer to buffer to fill response (address on 31:16).
 */
/**
 * Sends SD Memory Card interface condition, which includes host supply
 * voltage information and asks the card whether card supports voltage.
 * Should be performed at initialization time to detect the card type.
 * \param pSd             Pointer to a SD card driver instance.
 * \param supplyVoltage   Expected supply voltage(SD).
 * \return 0 if successful;
 *         otherwise returns SD_ERROR_NORESPONSE if the card did not answer
 *         the command, or SDMMC_ERROR.
 */
uint8_t SdCmd8(SdmmcDriver *drv, uint8_t supplyVoltage)
{

	const uint32_t arg = (supplyVoltage << SD_IFC_VHS_Pos) | SD_IFC_CHK_PATTERN_STD;
	uint32_t dwResp = 0;
	uint8_t bRc;

	_ResetCmd(&drv->cmd);

	/* Fill command information */
	drv->cmd.bCmd = 8;
	drv->cmd.cmdOp.wVal = SDMMC_CMD_CNODATA(7) | SDMMC_CMD_bmOD;
	drv->cmd.dwArg = arg;
	drv->cmd.pResp = &dwResp;
	drv->timeout_elapsed = -1;
	/* Send command */
	bRc = sdmmcSendCmd(drv);

	/* Expect the R7 response, which is the card interface condition.
	 * Expect VCA to match VHS, and the check pattern to match as well. */
	if (bRc == SDMMC_OK && (dwResp & (SD_IFC_VHS_Msk | SD_IFC_CHK_PATTERN_Msk)) == arg)
		return SDMMC_OK;
	else if (bRc == SDMMC_NO_RESPONSE)
		return SDMMC_NO_RESPONSE;
	else
		return SDMMC_ERR;
}

/**
 * Command toggles a card between the
 * stand-by and transfer states or between
 * the programming and disconnect states.
 * Returns the command transfer result (see SendMciCommand).
 * \param drv Pointer to \ref SdmmcDriver instance.
 * \param cardAddr  Relative Card Address (0 deselects all).
 */
uint8_t Cmd3(SdmmcDriver *drv)
{
	sSdmmcCommand *pCmd = &drv->cmd;
	uint32_t dwResp;
	uint8_t bRc;

	_ResetCmd(&drv->cmd);

	/* Fill command */
	drv->cmd.bCmd = 3;
	drv->cmd.pResp = &dwResp;

#ifndef SDMMC_TRIM_MMC
	if (drv->card.bCardType == CARD_MMC || drv->card.bCardType == CARD_MMCHD) {
		uint16_t wNewAddr = (uint16_t)max_u32(( drv->card.wAddress + 1) & 0xffff, 2);
		pCmd->dwArg = wNewAddr << 16;

		pCmd->cmdOp.wVal = SDMMC_CMD_CNODATA(1) | SDMMC_CMD_bmOD;

		drv->timeout_elapsed = -1;
		bRc = sdmmcSendCmd(drv);
		if (bRc == SDMMC_OK) {
			drv->card.wAddress = wNewAddr;
		}
	}
	else
#endif
	{
		drv->cmd.cmdOp.wVal = SDMMC_CMD_CNODATA(6) | SDMMC_CMD_bmOD;
		drv->timeout_elapsed = -1;
		bRc = sdmmcSendCmd(drv);

		if (bRc == SDMMC_OK) {
			drv->card.wAddress = dwResp >> 16;
		}
	}
	return bRc;
}

uint8_t Cmd7(SdmmcDriver *drv, uint16_t address)
{
	//sSdmmcCommand *pCmd = &pSd->sdCmd;
	uint8_t bRc;

	_ResetCmd(&drv->cmd);

	/* Fill command */
	/* If this function is used to transition the MMC device from the
	 * Disconnected to Programming state, then busy checking is required */
	if (address)
		drv->cmd.cmdOp.wVal = SDMMC_CMD_CNODATA(1) | SDMMC_CMD_bmBUSY;
	else
		drv->cmd.cmdOp.wVal = SDMMC_CMD_CNODATA(0);
	drv->cmd.bCmd = 7;
	drv->cmd.dwArg = address << 16;
	drv->timeout_elapsed = -1;
	/* Send command */
	bRc = sdmmcSendCmd(drv);
	return bRc;
}

uint8_t Cmd9(SdmmcDriver *drv)
{
	uint8_t bRc;

	_ResetCmd(&drv->cmd);

	/* Fill command */
	drv->cmd.cmdOp.wVal = SDMMC_CMD_CNODATA(2);
	drv->cmd.bCmd = 9;
	drv->cmd.dwArg = drv->card.wAddress << 16;
	drv->cmd.pResp = drv->card.CSD;
	drv->timeout_elapsed = -1;
	/* Send command */
	bRc = sdmmcSendCmd(drv);
	return bRc;
}

uint8_t Cmd11(SdmmcDriver *drv, uint32_t * pStatus)
{
	//sSdmmcCommand *pCmd = &pSd->sdCmd;

	uint8_t bRc;

	_ResetCmd(&drv->cmd);

	/* Fill command */
	drv->cmd.bCmd = 11;
	drv->cmd.cmdOp.wVal = SDMMC_CMD_CNODATA(1);
	drv->cmd.dwArg = 0;
	drv->cmd.pResp = pStatus;
	drv->timeout_elapsed = -1;
	/* Send command */
	bRc = sdmmcSendCmd(drv);

	return bRc;
}

/**
 * Forces the card to stop transmission
 * \param pSd      Pointer to a SD card driver instance.
 * \param pStatus  Pointer to a status variable.
 */
uint8_t Cmd12(SdmmcDriver *drv, uint32_t * pStatus)
{
	sSdmmcCommand *pCmd = &drv->cmd;
	uint8_t bRc;

	_ResetCmd(pCmd);

	/* Fill command */
	pCmd->bCmd = 12;
	pCmd->cmdOp.wVal = SDMMC_CMD_CSTOP | SDMMC_CMD_bmBUSY;
	pCmd->pResp = pStatus;

	drv->timeout_elapsed = -1;
		/* Send command */
		bRc = sdmmcSendCmd(drv);
	return bRc;
}


/**
 * Addressed card sends its status register.
 * Returns the command transfer result (see SendMciCommand).
 * \param drv Pointer to \ref SdmmcDriver instance.
 * \param pStatus   Pointer to a status variable.
 */
uint8_t Cmd13(SdmmcDriver *drv, uint32_t * pStatus)
{
	uint8_t bRc;

	_ResetCmd(&drv->cmd);

	/* Fill command */
	drv->cmd.bCmd = 13;
	drv->cmd.cmdOp.wVal = SDMMC_CMD_CNODATA(1);
	drv->cmd.dwArg = drv->card.wAddress << 16;
	drv->cmd.pResp = pStatus;
	drv->timeout_elapsed = -1;
	/* Send command */
	bRc = sdmmcSendCmd(drv);
	return bRc;
}


/**
 * A host reads the reversed bus testing data pattern from a card
 * \param drv Pointer to \ref SdmmcDriver instance.
 * \param pData     Pointer to the buffer to be filled.
 * The buffer shall follow the peripheral and DMA alignment requirements.
 * \param len       Length of data in byte
 * \param pStatus   Pointer response buffer as status return.
 */
#ifndef SDMMC_TRIM_MMC
uint8_t Cmd14(SdmmcDriver *drv, uint8_t * pData, uint8_t len, uint32_t * pStatus)
{
	sSdmmcCommand *pCmd = &drv->cmd;
	uint8_t bRc;

	_ResetCmd(&drv->cmd);

	/* Fill command */
	pCmd->cmdOp.wVal = SDMMC_CMD_CDATARX(1);
	pCmd->bCmd = 14;
	pCmd->pResp = pStatus;
	pCmd->wBlockSize = len;
	pCmd->wNbBlocks = 1;
	pCmd->pData = pData;
	drv->timeout_elapsed = -1;
	/* Send command */
	bRc = sdmmcSendCmd(drv);
	return bRc;
}

/**
 * Continously transfers datablocks from card to host until interrupted by a
 * STOP_TRANSMISSION command.
 * \param pSd       Pointer to a SD card driver instance.
 * \param nbBlocks  Number of blocks to send.
 * \param pData     Pointer to the buffer to be filled.
 * The buffer shall follow the peripheral and DMA alignment requirements.
 * \param address   Data Address on SD/MMC card.
 * \param pStatus   Pointer to the response status.
 * \param fCallback Pointer to optional callback invoked on command end.
 *                  NULL:    Function return until command finished.
 *                  Pointer: Return immediately and invoke callback at end.
 *                  Callback argument is fixed to a pointer to sSdCard instance.
 */
uint8_t Cmd18(SdmmcDriver *drv,
      uint16_t * nbBlock,
      uint8_t * pData,
      uint32_t address, uint32_t * pStatus)
{
	sSdmmcCommand *pCmd = &drv->cmd;
	uint8_t bRc;

	_ResetCmd(pCmd);

	/* Fill command */
	pCmd->cmdOp.wVal = SDMMC_CMD_CDATARX(1);
	pCmd->bCmd = 18;
	pCmd->dwArg = address;
	pCmd->pResp = pStatus;
	pCmd->wBlockSize = drv->card.wCurrBlockLen;
	pCmd->wNbBlocks = *nbBlock;
	pCmd->pData = pData;
	drv->timeout_elapsed = -1;
		/* Send command */
		bRc = sdmmcSendCmd(drv);
	if (bRc == SDMMC_CHANGED)
		*nbBlock = pCmd->wNbBlocks;
	return bRc;
}

/**
 * A host sends the bus test data pattern to a card.
 * \param drv Pointer to \ref SdmmcDriver instance.
 * \param pData     Pointer to the buffer to be filled.
 * The buffer shall follow the peripheral and DMA alignment requirements.
 * \param len       Length of data in byte
 * \param pStatus   Pointer response buffer as status return.
*/
 uint8_t Cmd19(SdmmcDriver *drv, uint8_t * pData, uint8_t len, uint32_t * pStatus)
{
	sSdmmcCommand *pCmd = &drv->cmd;
	uint8_t bRc;

	_ResetCmd(&drv->cmd);

	/* Fill command */
	pCmd->cmdOp.wVal = SDMMC_CMD_CDATATX(1);
	pCmd->bCmd = 19;
	pCmd->pResp = pStatus;
	pCmd->wBlockSize = len;
	pCmd->wNbBlocks = 1;
	pCmd->pData = pData;
	drv->timeout_elapsed = -1;
	/* Send command */
	bRc = sdmmcSendCmd(drv);
	return bRc;
}
#endif

uint8_t Cmd16(SdmmcDriver *drv, uint16_t blkLen)
{
	//sSdmmcCommand *pCmd = &pSd->sdCmd;
	uint8_t bRc;

	_ResetCmd(&drv->cmd);

	/* Fill command */
	drv->cmd.cmdOp.wVal = SDMMC_CMD_CNODATA(1);
	drv->cmd.bCmd = 16;
	drv->cmd.dwArg = blkLen;
	drv->timeout_elapsed = -1;
	/* Send command */
	bRc = sdmmcSendCmd(drv);

	return bRc;
}


/**
 * Read single block command
 * \param pSd  Pointer to a SD card driver instance.
 * \param pData     Pointer to the buffer to be filled.
 * The buffer shall follow the peripheral and DMA alignment requirements.
 * \param address   Data Address on SD/MMC card.
 * \param pStatus   Pointer response buffer as status return.
 * \param fCallback Pointer to optional callback invoked on command end.
 *                  NULL:    Function return until command finished.
 *                  Pointer: Return immediately and invoke callback at end.
 *                  Callback argument is fixed to a pointer to sSdCard instance.
 */
uint8_t Cmd17(SdmmcDriver *drv,
      uint8_t * pData,
      uint32_t address, uint32_t * pStatus)
{
	sSdmmcCommand *pCmd = &drv->cmd;
	uint8_t bRc;

	_ResetCmd(pCmd);

	/* Fill command */
	pCmd->cmdOp.wVal = SDMMC_CMD_CDATARX(1);
	pCmd->bCmd = 17;
	pCmd->dwArg = address;
	pCmd->pResp = pStatus;
	pCmd->wBlockSize =drv->card.wCurrBlockLen;
	pCmd->wNbBlocks = 1;
	pCmd->pData = pData;


	drv->timeout_elapsed = -1;
		/* Send command */
		bRc = sdmmcSendCmd(drv);
	return bRc;
}


/**
 * Defines the number of blocks (read/write) and the reliable writer parameter
 * (write) for a block read or write command
 * data (CSD) on the CMD line.
 * Returns the command transfer result (see SendMciCommand).
 * \param pSd       Pointer to a SD card driver instance.
 * \param write     Write Request parameter.
 * \param blocks    number of blocks.
 */
uint8_t Cmd23(SdmmcDriver *drv, uint8_t write, uint32_t blocks, uint32_t * pStatus)
{
	sSdmmcCommand *pCmd = &drv->cmd;
	uint8_t bRc;

	_ResetCmd(pCmd);

	/* Fill command */
	pCmd->cmdOp.wVal = SDMMC_CMD_CNODATA(1);
	pCmd->bCmd = 23;
	pCmd->wNbBlocks = 0;
	pCmd->dwArg = write << 31 | blocks;
	pCmd->pResp = pStatus;

	drv->timeout_elapsed = -1;
	/* Send command */
	bRc = sdmmcSendCmd(drv);
	return bRc;
}

/**
 * Write single block command
 * \param pSd  Pointer to a SD card driver instance.
 * \param blockSize Block size (shall be set to 512 in case of high capacity).
 * \param pData     Pointer to the buffer to be filled.
 * The buffer shall follow the peripheral and DMA alignment requirements.
 * \param address   Data Address on SD/MMC card.
 * \param pStatus   Pointer to response buffer as status.
 * \param fCallback Pointer to optional callback invoked on command end.
 *                  NULL:    Function return until command finished.
 *                  Pointer: Return immediately and invoke callback at end.
 *                  Callback argument is fixed to a pointer to sSdCard instance.
 */
uint8_t Cmd24(SdmmcDriver *drv,
      uint8_t * pData,
      uint32_t address, uint32_t * pStatus)
{
	sSdmmcCommand *pCmd = &drv->cmd;
	uint8_t bRc;

	_ResetCmd(pCmd);

	/* Fill command */
	pCmd->cmdOp.wVal = SDMMC_CMD_CDATATX(1);
	pCmd->bCmd = 24;
	pCmd->dwArg = address;
	pCmd->pResp = pStatus;
	pCmd->wBlockSize =drv->card.wCurrBlockLen;
	pCmd->wNbBlocks = 1;
	pCmd->pData = pData;
	drv->timeout_elapsed = -1;
	/* Send command */
	bRc = sdmmcSendCmd(drv);
	return bRc;
}

/**
 * Write multiple block command
 * \param pSd  Pointer to a SD card driver instance.
 * \param blockSize Block size (shall be set to 512 in case of high capacity).
 * \param nbBlock   Number of blocks to send.
 * \param pData     Pointer to the buffer to be filled.
 * The buffer shall follow the peripheral and DMA alignment requirements.
 * \param address   Data Address on SD/MMC card.
 * \param pStatus   Pointer to the response buffer as status.
 * \param fCallback Pointer to optional callback invoked on command end.
 *                  NULL:    Function return until command finished.
 *                  Pointer: Return immediately and invoke callback at end.
 *                  Callback argument is fixed to a pointer to sSdCard instance.
 */
uint8_t Cmd25(SdmmcDriver *drv,
      uint16_t * nbBlock,
      uint8_t * pData,
      uint32_t address, uint32_t * pStatus)
{
	sSdmmcCommand *pCmd = &drv->cmd;
	uint8_t bRc;

	_ResetCmd(pCmd);

	/* Fill command */
	pCmd->cmdOp.wVal = SDMMC_CMD_CDATATX(1);
	pCmd->bCmd = 25;
	pCmd->dwArg = address;
	pCmd->pResp = pStatus;
	pCmd->wBlockSize =drv->card.wCurrBlockLen;
	pCmd->wNbBlocks = *nbBlock;
	pCmd->pData = pData;

	drv->timeout_elapsed = -1;
	/* Send command */
	bRc = sdmmcSendCmd(drv);
	if (bRc == SDMMC_CHANGED)
		*nbBlock = pCmd->wNbBlocks;
	return bRc;
}


/**
 * SDIO IO_RW_DIRECT command, response R5.
 * \return the command transfer result (see SendMciCommand).
 * \param drv Pointer to \ref SdmmcDriver instance.
 * \param pIoData   Pointer to input argument (\ref SdioRwDirectArg) and
 *                  response (\ref SdmmcR5) buffer.
 * \param fCallback Pointer to optional callback invoked on command end.
 *                  NULL:    Function return until command finished.
 *                  Pointer: Return immediately and invoke callback at end.
 *                  Callback argument is fixed to a pointer to sSdCard instance.
 */
uint8_t Cmd52(SdmmcDriver *drv,
      uint8_t wrFlag,
      uint8_t funcNb, uint8_t rdAfterWr, uint32_t addr, uint32_t * pIoData)
{
	SdioCmd52Arg *pArg52 = (SdioCmd52Arg *) pIoData;
	sSdmmcCommand *pCmd = &drv->cmd;
	uint8_t bRc;

	pArg52->rwFlag = wrFlag;
	pArg52->functionNum = funcNb;
	pArg52->rawFlag = rdAfterWr;
	pArg52->regAddress = addr;

	_ResetCmd(&drv->cmd);
	/* Fill command */
	pCmd->cmdOp.wVal = SDMMC_CMD_CNODATA(5) | SDMMC_CMD_bmIO;
	pCmd->bCmd = 52;
	pCmd->dwArg = *pIoData;
	pCmd->pResp = pIoData;
	drv->timeout_elapsed = -1;
	/* Send command */
	bRc = sdmmcSendCmd(drv);
	return bRc;
}
/**
 * Indicates to the card that the next command is an application specific
 * command rather than a standard command.
 * \return the command transfer result (see SendMciCommand).
 * \param drv Pointer to \ref SdmmcDriver instance.
 * \param cardAddr  Card Relative Address.
 */
uint8_t Cmd55(SdmmcDriver *drv, uint16_t cardAddr)
{
	sSdmmcCommand *pCmd = &drv->cmd;
	uint32_t dwResp;
	uint8_t bRc;

	_ResetCmd(&drv->cmd);

	/* Fill command information */
	pCmd->bCmd = 55;
	pCmd->cmdOp.wVal = SDMMC_CMD_CNODATA(1)| (cardAddr ? 0 : SDMMC_CMD_bmOD);
	pCmd->dwArg = cardAddr << 16;
	pCmd->pResp = &dwResp;
	drv->timeout_elapsed = -1;
	/* Send command */
	bRc = sdmmcSendCmd(drv);
	return bRc;
}


/**
 * Defines the data bus width (00=1bit or 10=4 bits bus) to be used for data
 * transfer.
 * The allowed data bus widths are given in SCR register.
 * \param drv Pointer to \ref SdmmcDriver instance.
 * \param busWidth  Bus width in bits (4 or 1).
 * \return the command transfer result (see SendCommand).
 */
uint8_t Acmd6(SdmmcDriver *drv, uint8_t busWidth)
{
	sSdmmcCommand *pCmd = &drv->cmd;
	uint8_t error;

	TRACE_INFO_1("Acmd%u\n\r", 6);

	error = Cmd55(drv, drv->card.wAddress);

	if (!error) {
		_ResetCmd(&drv->cmd);

		pCmd->bCmd = 6;
		pCmd->cmdOp.wVal = SDMMC_CMD_CNODATA(1);
		pCmd->dwArg = (busWidth == 4) ? SD_SSR_DATA_BUS_WIDTH_4BIT : SD_SSR_DATA_BUS_WIDTH_1BIT;
		drv->timeout_elapsed = -1;
		error = sdmmcSendCmd(drv);
	}
	else {
		if (error) {
			TRACE_ERROR_2("Acmd%u %s\n\r", 6, SD_StringifyRetCode(error));
		}
	}
	return error;
}


/**
 * From the selected card get its SD Status Register (SSR).
 * ACMD13 is valid under the Transfer state.
 * \param drv Pointer to \ref SdmmcDriver instance.
 * \param pSSR  Pointer to a 64-byte buffer receiving the contents of the SSR.
 * The buffer shall follow the peripheral and DMA alignment requirements.
 * \param pResp  Pointer to where the response is returned.
 * \return The command transfer result (see SendCommand).
 */
uint8_t Acmd13(SdmmcDriver *drv, uint8_t * pSSR, uint32_t * pResp)
{
	sSdmmcCommand *pCmd = &drv->cmd;
	uint8_t error;

	//assert(pSd);

	TRACE_INFO_1("Acmd%u\n\r", 13);

	error = Cmd55(drv, drv->card.wAddress);

	if (!error) {

		_ResetCmd(&drv->cmd);

		pCmd->bCmd = 13;
		pCmd->cmdOp.wVal = SDMMC_CMD_CDATARX(1);
		if (pSSR) {
			pCmd->wBlockSize = 512 / 8;
			pCmd->wNbBlocks = 1;
			pCmd->pData = pSSR;
		}
		pCmd->pResp = pResp;
		drv->timeout_elapsed = -1;
		error = sdmmcSendCmd(drv);

	} else {
		if (error) {
			TRACE_ERROR_2("Acmd%u %s\n\r", 13, SD_StringifyRetCode(error));
		}
	}
	return error;
}

/**
 * Asks to all cards to send their operations conditions.
 * Returns the command transfer result (see SendCommand).
 * \param drv Pointer to \ref SdmmcDriver instance.
 * \param low_sig_lvl  In: tells whether the host supports UHS-I timing modes.
 * Out: tells whether the device may switch to low signaling level.
 * \param hc  In: tells whether the device has replied to SEND_IF_COND.
 * Out: tells whether the device is a high capacity device.
 */
uint8_t Acmd41(SdmmcDriver *drv, bool * low_sig_lvl, bool * hc)
{

	sSdmmcCommand *pCmd = &drv->cmd;
	/* TODO get this board-specific value from platform code. On the
	 * SAMA5D2-XULT board, VDD is 3.3V ± 1%. */
	const uint32_t vdd_range = SD_OCR_VDD_32_33 | SD_OCR_VDD_33_34;
	uint32_t arg, ocr = 0;
	uint8_t rc;
	//int8_t elapsed = -1;

	//trace_debug("Acmd%u\n\r", 41);
	/* Provided the device has answered the SEND_IF_COND command, raise the
	 * Host Capacity Support flag. Also, set the SDXC Power Control flag.
	 * TODO assign XPC depending on board capabilities. */
	arg = *hc ? SD_OPC_HCS | SD_OPC_XPC : 0;
	/* Preparing UHS-I timing modes, ask the device whether it's in a
	 * position to switch to low signaling voltage. */
	arg |= *low_sig_lvl ? SD_OPC_S18R : 0;
	/* Tell the SD device which voltage the host supplies to the VDD line */
	arg |= vdd_range;


	drv->timeout_ticks = TIME_S2I(1);
	drv->timeout_elapsed = 0;

	do {
		rc = Cmd55(drv, 0);

		if (rc != SDMMC_OK)
			break;

		_ResetCmd(&drv->cmd);

		pCmd->bCmd = 41;
		pCmd->cmdOp.wVal = SDMMC_CMD_CNODATA(3);
		pCmd->dwArg = arg;
		pCmd->pResp = &ocr;
		drv->timeout_elapsed = -1;
		rc =  sdmmcSendCmd(drv);

		if (rc != SDMMC_OK)
			break;


	} while (!(ocr & SD_OCR_BUSYN) && !drv->timeout_elapsed);

	if (!(ocr & SD_OCR_BUSYN) && rc == SDMMC_OK)
		/* Supply voltage range is incompatible */
		rc = SDMMC_BUSY;
	if (rc != SDMMC_OK) {
		TRACE_ERROR_2("Acmd%u %s\n\r", 41, SD_StringifyRetCode(rc));
	}
	else {
#if 0
		/* Analyze the final contents of the OCR Register */
		trace_info("Device supports%s%s 3.0V:[%c%c%c%c%c%c]"
		    " 3.3V:[%c%c%c%c%c%c]\n\r",
		    ocr & SD_OCR_VDD_LOW ? " 1.xV" : "", "",
		    ocr & SD_OCR_VDD_27_28 ? 'X' : '.',
		    ocr & SD_OCR_VDD_28_29 ? 'X' : '.',
		    ocr & SD_OCR_VDD_29_30 ? 'X' : '.',
		    ocr & SD_OCR_VDD_30_31 ? 'X' : '.',
		    ocr & SD_OCR_VDD_31_32 ? 'X' : '.',
		    ocr & SD_OCR_VDD_32_33 ? 'X' : '.',
		    ocr & SD_OCR_VDD_30_31 ? 'X' : '.',
		    ocr & SD_OCR_VDD_31_32 ? 'X' : '.',
		    ocr & SD_OCR_VDD_32_33 ? 'X' : '.',
		    ocr & SD_OCR_VDD_33_34 ? 'X' : '.',
		    ocr & SD_OCR_VDD_34_35 ? 'X' : '.',
		    ocr & SD_OCR_VDD_35_36 ? 'X' : '.');
#endif
		/* Verify that arg[23:15] range fits within OCR[23:15] range */
		if ((ocr & vdd_range) != vdd_range)
			rc = SDMMC_NOT_SUPPORTED;
		if (*low_sig_lvl)
			*low_sig_lvl = ocr & SD_OCR_S18A ? true : false;
		*hc = ocr & SD_OCR_CCS ? true : false;
	}
	return rc;

}


/**
 * From the selected card get its SD CARD Configuration Register (SCR).
 * ACMD51 is valid under the Transfer state.
 * \param drv Pointer to \ref SdmmcDriver instance.
 * \param pSCR  Pointer to an 8-byte buffer receiving the contents of the SCR.
 * The buffer shall follow the peripheral and DMA alignment requirements.
 * \param pResp  Pointer to where the response is returned.
 * \return The command transfer result (see SendCommand).
 */
uint8_t Acmd51(SdmmcDriver *drv, uint8_t * pSCR, uint32_t * pResp)
{
	sSdmmcCommand *pCmd = &drv->cmd;
	uint8_t error;


	TRACE_INFO_1("Acmd%u\n\r", 51);

	error = Cmd55(drv, drv->card.wAddress);

	if (!error) {

		_ResetCmd(pCmd);

		pCmd->bCmd = 51;
		pCmd->cmdOp.wVal = SDMMC_CMD_CDATARX(1);

		if (pSCR) {
			pCmd->wBlockSize = 64 / 8;
			pCmd->wNbBlocks = 1;
			pCmd->pData = pSCR;
		}
		pCmd->pResp = pResp;
		drv->timeout_elapsed = -1;
		error = sdmmcSendCmd(drv);

	} else {
		if (error) {
			TRACE_ERROR_2("Acmd%u %s\n\r", 51, SD_StringifyRetCode(error));
		}
	}
	return error;
}

/**
 * SEND_EXT_CSD, to get EXT_CSD register as a block of data.
 * Valid under "trans" state.
 * \param drv Pointer to \ref SdmmcDriver instance.
 * \param pEXT  512 byte buffer pointer for EXT_CSD data.
 * The buffer shall follow the peripheral and DMA alignment requirements.
 * \return 0 if successful;
 *         otherwise returns SD_ERROR_NORESPONSE if the card did not answer
 *         the command, or SDMMC_ERROR.
 */
#ifndef SDMMC_TRIM_MMC

/**
 * Switches the mode of operation of the selected card or
 * Modifies the EXT_CSD registers.
 * CMD6 is valid under the "trans" state.
 * \return The command transfer result (see SendMciCommand).
 * \param  pSd         Pointer to a SD/MMC card driver instance.
 * \param  pSwitchArg  Pointer to a MmcCmd6Arg instance.
 * \param  pResp       Pointer to where the response is returned.
 */
uint8_t MmcCmd6(SdmmcDriver *drv, const void *pSwitchArg, uint32_t * pResp)
{
	sSdmmcCommand *pCmd = &drv->cmd;
	uint8_t bRc;
	MmcCmd6Arg *pMmcSwitch;

	//assert(pSd);
//	assert(pSwitchArg);

	_ResetCmd(&drv->cmd);

	pMmcSwitch = (MmcCmd6Arg *) pSwitchArg;
	pCmd->bCmd = 6;
	pCmd->cmdOp.wVal = SDMMC_CMD_CNODATA(1) | SDMMC_CMD_bmBUSY;
	pCmd->dwArg = (pMmcSwitch->access << 24)
	    | (pMmcSwitch->index << 16)
	    | (pMmcSwitch->value << 8)
	    | (pMmcSwitch->cmdSet << 0);
	pCmd->pResp = pResp;
	drv->timeout_elapsed = -1;
	bRc = sdmmcSendCmd(drv);

	if (!bRc && pResp && *pResp & STATUS_MMC_SWITCH) {
		TRACE_INFO_1("st %lx\n\r", *pResp);
	}
	return bRc;
}

uint8_t MmcCmd8(SdmmcDriver *drv)
{
	sSdmmcCommand *pCmd = &drv->cmd;
	uint8_t bRc;

	_ResetCmd(&drv->cmd);

	/* Fill command */
	pCmd->bCmd = 8;
	pCmd->cmdOp.wVal = SDMMC_CMD_CDATARX(1);
	pCmd->wBlockSize = 512;
	pCmd->wNbBlocks = 1;
	pCmd->pData = drv->card.EXT;
	drv->timeout_elapsed = -1;
	/* Send command */
	bRc = sdmmcSendCmd(drv);

	return bRc;
}
#endif

/**
 * SDIO SEND OPERATION CONDITION (OCR) command.
 * Sends host capacity support information and acrivates the card's
 * initialization process.
 * \return The command transfer result (see SendMciCommand).
 * \param drv Pointer to \ref SdmmcDriver instance.
 * \param pIo     Pointer to data sent as well as response buffer (32bit).
 */
#ifndef SDMMC_TRIM_SDIO
uint8_t Cmd5(SdmmcDriver *drv, uint32_t * pIo)
{
	sSdmmcCommand *pCmd = &drv->cmd;
	uint8_t bRc;

	_ResetCmd(&drv->cmd);

	/* Fill command */
	pCmd->cmdOp.wVal = SDMMC_CMD_CNODATA(4) | SDMMC_CMD_bmIO
	    | SDMMC_CMD_bmOD;
	pCmd->bCmd = 5;
	pCmd->dwArg = *pIo;
	pCmd->pResp = pIo;
	drv->timeout_elapsed = -1;
	/* Send command */
	bRc = sdmmcSendCmd(drv);
	return bRc;
}
#endif


uint8_t CancelCommand(SdmmcDriver *driver)
{
	//assert(set);
	osalDbgCheck(driver->state != MCID_OFF);

	Sdmmc *regs = driver->regs;
	sSdmmcCommand *cmd = &driver->cmd;
	uint32_t response;   /* The R1 response is 32-bit long */
	uint32_t usec, rc;

	cmd->pResp = &response;
	cmd->cmdOp.wVal  = SDMMC_CMD_CSTOP | SDMMC_CMD_bmBUSY;
	cmd->bCmd = 12;

	if (driver->state != MCID_CMD && driver->state != MCID_ERROR)
		return SDMMC_STATE;
	//trace_debug("Requested to cancel CMD%u\n\r", set->cmd ? set->cmd->bCmd : 99);
	if (driver->state == MCID_ERROR) {
		driver->state = MCID_LOCKED;
		return SDMMC_OK;
	}
	//assert(cmd);
	/* Asynchronous Abort, if a data transfer has been started */
	if (cmd->cmdOp.bmBits.xfrData == SDMMC_CMD_TX
	    || cmd->cmdOp.bmBits.xfrData == SDMMC_CMD_RX) {
		/* May the CMD line still be busy, reset it */
		if (regs->SDMMC_PSR & SDMMC_PSR_CMDINHC) {
			regs->SDMMC_SRR |= SDMMC_SRR_SWRSTCMD;
			while (regs->SDMMC_SRR & SDMMC_SRR_SWRSTCMD) ;
		}
		/* Issue the STOP_TRANSMISSION command. */
		driver->state = MCID_LOCKED;
		driver->resp_len = 0;
		driver->blk_index = 0;
		driver->cmd_line_released = false;
		driver->dat_lines_released = false;
		driver->expect_auto_end = false;

		rc = sdmmc_device_command(driver);

		if (rc == SDMMC_OK) {
			for (usec = 0; driver->state == MCID_CMD && usec < 500000; usec+= 10) {

				t_usleep(driver,10);

				if (driver->use_polling)  {
					sdmmc_device_poll(driver);
				}
			}
		}
	}
	/* Reset CMD and DATn lines */
	regs->SDMMC_SRR |= SDMMC_SRR_SWRSTDAT | SDMMC_SRR_SWRSTCMD;
	while (regs->SDMMC_SRR & (SDMMC_SRR_SWRSTDAT | SDMMC_SRR_SWRSTCMD)) ;

	/* Release command */
	cmd->bStatus = SDMMC_USER_CANCEL;

	driver->state = MCID_LOCKED;

	driver->resp_len = 0;
	driver->blk_index = 0;
	driver->cmd_line_released = false;
	driver->dat_lines_released = false;
	driver->expect_auto_end = false;

	return SDMMC_OK;
}


uint8_t tuneSampling(SdmmcDriver *driver)
{
	//osalDbgCheck(set);
	osalDbgCheck(driver->state != MCID_OFF && driver->state != MCID_CMD);

	Sdmmc *regs = driver->regs;
	uint32_t response;   /* The R1 response is 32-bit long */

	//test command
	driver->cmd.pData = (uint8_t *)&response;
	driver->cmd.wBlockSize = 128;
			driver->cmd.wNbBlocks = 1;
			driver->cmd.pResp = &response;
			driver->cmd.dwArg = 0;
			driver->cmd.cmdOp.wVal = SDMMC_CMD_CDATARX(1);
			driver->cmd.bCmd = 21;
	uint16_t hc2r;
	uint8_t rc = SDMMC_OK, ix;

	if (driver->tim_mode != SDMMC_TIM_MMC_HS200)
		driver->cmd.bCmd = 19;
	ix = sdmmc_get_bus_width(driver);
	if (ix == 4)
		driver->cmd.wBlockSize = 64;
	else if (ix != 8)
		return SDMMC_PARAM;
	/* Start the tuning procedure */
	regs->SDMMC_HC2R |= SDMMC_HC2R_EXTUN;
	hc2r = regs->SDMMC_HC2R;
	for (ix = 0; hc2r & SDMMC_HC2R_EXTUN && ix < 40; ix++) {
		/* Issue the SEND_TUNING_BLOCK command */
		driver->state = MCID_LOCKED;
		driver->resp_len = 0;
		driver->blk_index = 0;
		driver->cmd_line_released = false;
		driver->dat_lines_released = false;
		driver->expect_auto_end = false;
		rc = sdmmc_device_command(driver);

		if (rc != SDMMC_OK)
			break;

		/* While tuning the position of the sampling point, usual
		 * interrupts do not occur. Expect NISTR:BRDRDY only. */
		while (!(regs->SDMMC_NISTR & SDMMC_NISTR_BRDRDY)) ;
		regs->SDMMC_NISTR = SDMMC_NISTR_BRDRDY;
		//driver->cmd = NULL;
		hc2r = regs->SDMMC_HC2R;
	}
	if (hc2r & SDMMC_HC2R_EXTUN) {
		/* Abort the tuning procedure */
		regs->SDMMC_HC2R = hc2r & ~SDMMC_HC2R_EXTUN;
		/* Reset the tuning circuit. Use the fixed clock when sampling
		 * data. */
		regs->SDMMC_HC2R = hc2r & ~SDMMC_HC2R_SCLKSEL
		    & ~SDMMC_HC2R_EXTUN;
		rc = SDMMC_ERR;
	}
	else if (!(hc2r & SDMMC_HC2R_SCLKSEL))
		rc = SDMMC_ERR;
	/* Clear residual interrupts, if any */
	if (regs->SDMMC_NISTR & SDMMC_NISTR_ERRINT)
		regs->SDMMC_EISTR = regs->SDMMC_EISTR;
	regs->SDMMC_NISTR = regs->SDMMC_NISTR;
	driver->state = MCID_LOCKED;
	driver->resp_len = 0;
	driver->blk_index = 0;
	driver->cmd_line_released = false;
	driver->dat_lines_released = false;
	driver->expect_auto_end = false;
	//trace_debug("%u tuning blocks. %s.\n\r", ix, SD_StringifyRetCode(rc));
	return rc;
}

static void _ResetCmd(sSdmmcCommand * pCmd)
{
	memset(pCmd, 0, sizeof (sSdmmcCommand));
}

#endif
