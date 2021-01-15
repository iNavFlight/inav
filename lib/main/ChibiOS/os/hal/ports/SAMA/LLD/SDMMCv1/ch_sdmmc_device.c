#include <string.h>
#include "hal.h"

#if (SAMA_USE_SDMMC == TRUE)

#include "sama_sdmmc_lld.h"
#include "ch_sdmmc_device.h"
#include "ch_sdmmc_cmds.h"
#include "ch_sdmmc_sdio.h"
#include "ch_sdmmc_sd.h"
#include "ch_sdmmc_mmc.h"
/** A software event, never raised by the hardware, specific to this driver */
#define SDMMC_NISTR_CUSTOM_EVT        (0x1u << 13)

/** Device status */
#define STAT_ADDRESS_OUT_OF_RANGE     (1UL << 31)
#define STAT_ADDRESS_MISALIGN         (1UL << 30)
#define STAT_BLOCK_LEN_ERROR          (1UL << 29)
#define STAT_ERASE_SEQ_ERROR          (1UL << 28)
#define STAT_ERASE_PARAM              (1UL << 27)
#define STAT_WP_VIOLATION             (1UL << 26)
#define STAT_DEVICE_IS_LOCKED         (1UL << 25)
#define STAT_LOCK_UNLOCK_FAILED       (1UL << 24)
#define STAT_COM_CRC_ERROR            (1UL << 23)
#define STAT_ILLEGAL_COMMAND          (1UL << 22)
#define STAT_DEVICE_ECC_FAILED        (1UL << 21)
#define STAT_CC_ERROR                 (1UL << 20)
#define STAT_ERROR                    (1UL << 19)
#define STAT_CID_OVERWRITE            (1UL << 16)
#define STAT_ERASE_SKIP               (1UL << 15)
#define STAT_CARD_ECC_DISABLED        (1UL << 14)
#define STAT_ERASE_RESET              (1UL << 13)
#define STAT_CURRENT_STATE            (0xfUL << 9)
#define STAT_READY_FOR_DATA           (1UL << 8)
#define STAT_SWITCH_ERROR             (1UL << 7)
#define STAT_EXCEPTION_EVENT          (1UL << 6)
#define STAT_APP_CMD                  (1UL << 5)

union uint32_u {
	uint32_t word;
	uint8_t bytes[4];
};

static void calibrate_zout(Sdmmc * regs);
void reset_peripheral(SdmmcDriver *driver);
void sdmmc_set_capabilities(
		Sdmmc * regs,
		uint32_t caps0, uint32_t caps0_mask,
		uint32_t caps1, uint32_t caps1_mask);

static uint8_t HwReset(SdmmcDriver *driver);



static void sdmmc_get_response(SdmmcDriver *driver, sSdmmcCommand *cmd, bool complete, uint32_t *out);

static uint8_t sdmmc_build_dma_table( SdmmcDriver *driver );
static uint8_t unplug_device(SdmmcDriver *driver);
static uint8_t sdmmc_set_speed_mode(SdmmcDriver *driver, uint8_t mode,bool verify);
static uint8_t sdmmc_set_bus_width(SdmmcDriver *driver, uint8_t bits);


uint8_t  sdmmc_device_lowlevelcfg(SdmmcDriver *driver)
{
	uint8_t res;


	TRACE_INFO_1("Processor clock: %u MHz\r\n", ((unsigned)(SAMA_PCK / 1000000) ));
	TRACE_INFO_1("Master clock: %u MHz\r\n", ((unsigned)(SAMA_MCK / 1000000)) );

	if (driver->config->slot_id == SDMMC_SLOT0) {
		driver->regs = SDMMC0;
		pmcEnableSDMMC0();
	} else if (driver->config->slot_id == SDMMC_SLOT1) {
		driver->regs = SDMMC1;
		pmcEnableSDMMC1();
	}

	switch (driver->config->slot_id) {

	case SDMMC_SLOT0: {

		uint32_t caps0 = BOARD_SDMMC0_CAPS0;

		/* Program capabilities for SDMMC0 */
		sdmmc_set_capabilities((Sdmmc*) SDMMC0, caps0, CAPS0_MASK, 0, 0);

#if 0
		/* Configure SDMMC0 pins */

			/** SDMMC0 pin Card Detect (CD) */
		palSetGroupMode(PIOA, (1u << PIOA_PIN13), 0U,
				PAL_SAMA_FUNC_PERIPH_A | PAL_MODE_INPUT_PULLUP);

		/** SDMMC0 pin Card Clock (CK) */
		palSetGroupMode(PIOA, (1u << 0), 0U, PAL_SAMA_FUNC_PERIPH_A);

		/** SDMMC0 pin Card Command (CMD) */
		palSetGroupMode(PIOA, (1u << PIOA_PIN1), 0U,
				PAL_SAMA_FUNC_PERIPH_A | PAL_MODE_INPUT_PULLUP);

		/** SDMMC0 pin Card Reset (RSTN) */
		palSetGroupMode(PIOA, (1u << PIOA_PIN10), 0U,
				PAL_SAMA_FUNC_PERIPH_A | PAL_MODE_INPUT_PULLUP);

		/** SDMMC0 pin VDD Selection (VDDSEL) */
		palSetGroupMode(PIOA, (1u << PIOA_PIN11), 0U,
				PAL_SAMA_FUNC_PERIPH_A);

		/** SDMMC0 pin 8-bit Data (DA0-7) */
		palSetGroupMode(PIOA, 0x000003fc, 0U,
				PAL_SAMA_FUNC_PERIPH_A | PAL_MODE_INPUT_PULLUP);
#endif
		res = 1;

	}
	break;
	case SDMMC_SLOT1: {

		uint32_t caps0 = BOARD_SDMMC1_CAPS0;

		/* Program capabilities for SDMMC1 */
		sdmmc_set_capabilities(SDMMC1, caps0, CAPS0_MASK, 0, 0);

#if 0
		/* Configure SDMMC1 pins */
		/** SDMMC1 pin Card Detect (CD) */
		palSetGroupMode(PIOA, (1u << PIOA_PIN30), 0U,
				PAL_SAMA_FUNC_PERIPH_E | PAL_MODE_INPUT_PULLUP);

		/** SDMMC1 pin Card Clock (CK) */
		palSetGroupMode(PIOA, (1u << PIOA_PIN22), 0U, PAL_SAMA_FUNC_PERIPH_E);

		/** SDMMC1 pin Card Command (CMD) */
		palSetGroupMode(PIOA, (1u << PIOA_PIN28), 0U,
				PAL_SAMA_FUNC_PERIPH_E | PAL_MODE_INPUT_PULLUP);

		/** SDMMC1 pin 4-bit Data (DA0-3) */
		palSetGroupMode(PIOA, 0x003c0000, 0U,
				PAL_SAMA_FUNC_PERIPH_E | PAL_MODE_INPUT_PULLUP);
#endif

		res = 1;
	}
	break;
	default:
		res = 0;
		break;
	}

	return res;
}

bool sdmmc_device_initialize(SdmmcDriver *driver)
{

	uint32_t base_freq, power, val;
	const uint8_t max_exp = (SDMMC_TCR_DTCVAL_Msk >> SDMMC_TCR_DTCVAL_Pos) - 1;
	uint8_t exp;

	driver->use_set_blk_cnt = false;

	val = (driver->regs->SDMMC_CA0R & SDMMC_CA0R_MAXBLKL_Msk) >> SDMMC_CA0R_MAXBLKL_Pos;

	driver->blk_size = (val <= 0x2 ? (512 << val) : 512);

	/* Perform the initial I/O calibration sequence, manually.
	 * Allow tSTARTUP = 2 usec for the analog circuitry to start up.
	 * CNTVAL = fHCLOCK / (4 * (1 / tSTARTUP)) */
	val = SAMA_MCK;
	val = ROUND_INT_DIV(val, 4 * 500000UL);


	osalDbgCheck( (!(val << SDMMC_CALCR_CNTVAL_Pos & ~SDMMC_CALCR_CNTVAL_Msk)) );

	driver->regs->SDMMC_CALCR = (driver->regs->SDMMC_CALCR & ~SDMMC_CALCR_CNTVAL_Msk & ~SDMMC_CALCR_TUNDIS) | SDMMC_CALCR_CNTVAL(val);

	calibrate_zout(driver->regs);

	/* Set DAT line timeout error to occur after 500 ms waiting delay.
	 * 500 ms is the timeout value to implement when writing to SDXC cards.
	 */
	base_freq = (driver->regs->SDMMC_CA0R & SDMMC_CA0R_TEOCLKF_Msk) >> SDMMC_CA0R_TEOCLKF_Pos;
	base_freq *= driver->regs->SDMMC_CA0R & SDMMC_CA0R_TEOCLKU ? 1000000UL : 1000UL;
	/* 2 ^ (DTCVAL + 13) = TIMEOUT * FTEOCLK = FTEOCLK / 2 */
	val = base_freq / 2;
	for (exp = 31, power = 1UL << 31; !(val & power) && power != 0;
	    exp--, power >>= 1) ;
	if (power == 0) {
		TRACE_DEBUG("FTEOCLK is unknown\n\r");
		exp = max_exp;
	}
	else {
		exp = exp + 1 - 13;
		exp = (uint8_t)min_u32(exp, max_exp);
	}

	driver->regs->SDMMC_TCR = (driver->regs->SDMMC_TCR & ~SDMMC_TCR_DTCVAL_Msk) | SDMMC_TCR_DTCVAL(exp);

	TRACE_DEBUG_1("Set DAT line timeout to %lu ms\n\r", (10UL << (exp + 13UL))/ (base_freq / 100UL));

	/* Reset the peripheral. This will reset almost all registers.
	 * It doesn't affect I/O calibration however. */
	reset_peripheral(driver);
	/* As sdmmc_reset_peripheral deliberately preserves MC1R.FCD, this field
	 * has yet to be initialized. As the controller may disable outputs
	 * depending on the state of the card detection input, this input should
	 * be neutralized when the device is embedded. */
	if ( (driver->regs->SDMMC_CA0R & SDMMC_CA0R_SLTYPE_Msk) == SDMMC_CA0R_SLTYPE_EMBEDDED)
		driver->regs->SDMMC_MC1R |= SDMMC_MC1R_FCD;
	else
		driver->regs->SDMMC_MC1R &= ~SDMMC_MC1R_FCD;



	return true;
}
/**
 * Run the SDcard initialization sequence. This function runs the
 * initialisation procedure and the identification process, then it sets the
 * SD card in transfer state to set the block length and the bus width.
 * \return 0 if successful; otherwise returns an \ref sdmmc_rc "error code".
 * \param pSd  Pointer to a SD card driver instance.
 */
uint8_t sdmmc_device_start(SdmmcDriver *drv)
{
	uint32_t freq;
	uint8_t error;

	SdParamReset(&drv->card);

	/* Power the device and the bus on */
	HwPowerDevice(drv, SDMMC_PWR_STD);
	/* Reset the controller to default timing mode and data bus width */
	HwSetHsMode(drv, SDMMC_TIM_MMC_BC);

	HwSetBusWidth(drv, 1);
	/* For device identification, clock the device at fOD */
	freq = 400000ul;

	error = HwSetClock(drv, &freq);

	if (error != SDMMC_OK && error != SDMMC_CHANGED) {
		return error;
	}


	/* Initialization delay: The maximum of 1 msec, 74 clock cycles and supply
	 * ramp up time. Supply ramp up time provides the time that the power is
	 * built up to the operating level (the bus master supply voltage) and the
	 * time to wait until the SD card can accept the first command. */
	/* Power On Init Special Command */
	error = CmdPowerOn(drv);

	t_msleep(drv,200);

	if (error) {
		return error;
	}


	return SDMMC_OK;
}

uint8_t sdmmc_device_identify(SdmmcDriver *drv)
{
	uint8_t error;
	bool retry = false;

	if (drv->state != MCID_IDLE )
		return SDMMC_STATE;

	Retry:
		/* After power-on or CMD0, all cards?
		 * CMD lines are in input mode, waiting for start bit of the next command.
		 * The cards are initialized with a default relative card address
		 * (RCA=0x0000) and with a default driver stage register setting
		 * (lowest speed, highest driving current capability). */
		error = SdMmcIdentify(drv);

		if (error) {
			TRACE_ERROR_1("Identify %s\n\r", SD_StringifyRetCode(error));
			return error;
		}

			if ((drv->card.bCardType & CARD_TYPE_bmSDMMC) == CARD_TYPE_bmSD) {

			error = SDMMC_Lib_SdStart(drv, &retry);
			/* Handle the case where the both the slot and the device are
			 * UHS-I-capable, but the system doesn't support powering the
			 * card off, when SD_DeInit is called. As a result, from the
			 * device's perspective, the voltage switch sequence has been
			 * taken already. */
				if (error && retry) {
					HwPowerDevice(drv, SDMMC_PWR_STD_VDD_LOW_IO);

					drv->card.bCardSigLevel = 0;

					error = HwSetHsMode(drv, SDMMC_TIM_SD_SDR12);

					HwSetBusWidth(drv, 1);

					if (!error) {
						drv->card.bSpeedMode = SDMMC_TIM_SD_SDR12;
						goto Retry;
					}
				}
			}
	#ifndef SDMMC_TRIM_SDIO
		else if (drv->card.bCardType & CARD_TYPE_bmSDIO)
			error = SdioInit(drv);
	#endif
	#ifndef SDMMC_TRIM_MMC
		else if ((drv->card.bCardType & CARD_TYPE_bmSDMMC) == CARD_TYPE_bmMMC)
			error = MmcInit(drv);
	#endif
		else {
			TRACE_ERROR_1("Identify %s\n\r", "failed");
			return SDMMC_NOT_INITIALIZED;
		}
		if (error) {
			TRACE_ERROR_1("Init %s\n\r", SD_StringifyRetCode(error));
			return error;
		}


		drv->card.bStatus = SDMMC_OK;

		return SDMMC_OK;
}

void sdmmc_device_deInit(SdmmcDriver *drv)
{
		HwReset(drv);
		SdParamReset(&drv->card);

		memset(&drv->cmd, 0, sizeof(drv->cmd));
}


/**
 * \brief Fetch events from the SDMMC peripheral, handle them, and proceed to
 * the subsequent step, w.r.t. the SD/MMC command being processed.
 * \warning This implementation suits LITTLE ENDIAN hosts only.
 */
 void sdmmc_device_poll(SdmmcDriver *driver)

 {
 	osalDbgCheck(driver->state != MCID_OFF);

 	Sdmmc *regs = driver->regs;
 	sSdmmcCommand *cmd = &driver->cmd;
 	uint16_t events, errors, acesr;
 	bool has_data;

 	if (driver->state != MCID_CMD)
 		return;
 	//osalDbgCheck(cmd);
 	has_data = (cmd->cmdOp.bmBits.xfrData == SDMMC_CMD_TX) || (cmd->cmdOp.bmBits.xfrData == SDMMC_CMD_RX);

 Fetch:
 	/* Fetch normal events */
 	events = regs->SDMMC_NISTR;

	if (driver->expect_auto_end)
	{
		while ( chSysIsCounterWithinX(chSysGetRealtimeCounterX(),driver->start_cycles ,driver->start_cycles+driver->timeout_cycles) );
		events |= SDMMC_NISTR_CUSTOM_EVT;
	}

 	if (!events)
 		return;
 	//TRACE_1("events %08x\n\r",events);
 	/* Check the global error flag */
 	if (events & SDMMC_NISTR_ERRINT) {
 		errors = regs->SDMMC_EISTR;
 		events &= ~SDMMC_NISTR_ERRINT;
 		/* Clear error interrupts */
 		regs->SDMMC_EISTR = errors;
 		if (errors & SDMMC_EISTR_CURLIM)
 			cmd->bStatus = SDMMC_NOT_INITIALIZED;
 		else if (errors & SDMMC_EISTR_CMDCRC)
 			cmd->bStatus = SDMMC_ERR_IO;
 		else if (errors & SDMMC_EISTR_CMDTEO)
 			cmd->bStatus = SDMMC_NO_RESPONSE;
 		else if (errors & (SDMMC_EISTR_CMDEND | SDMMC_EISTR_CMDIDX))
 			cmd->bStatus = SDMMC_ERR_IO;
 		else if (errors & SDMMC_EISTR_TUNING)
 			cmd->bStatus = SDMMC_ERR_IO;
 		/* TODO upon SDMMC_EISTR_TUNING, clear HC2R:SCLKSEL, and perform
 		 * the tuning procedure */
 		/* TODO if SDMMC_NISTR_TRFC and only SDMMC_EISTR_DATTEO then
 		 * ignore SDMMC_EISTR_DATTEO */
 		else if (errors & SDMMC_EISTR_DATTEO)
 			cmd->bStatus = SDMMC_ERR_IO;
 		else if (errors & (SDMMC_EISTR_DATCRC | SDMMC_EISTR_DATEND))
 			cmd->bStatus = SDMMC_ERR_IO;
 		else if (errors & SDMMC_EISTR_ACMD) {
 			acesr = regs->SDMMC_ACESR;
 			if (acesr & SDMMC_ACESR_ACMD12NE)
 				cmd->bStatus = SDMMC_ERR;
 			else if (acesr & SDMMC_ACESR_ACMDCRC)
 				cmd->bStatus = SDMMC_ERR_IO;
 			else if (acesr & SDMMC_ACESR_ACMDTEO)
 				cmd->bStatus = SDMMC_NO_RESPONSE;
 			else if (acesr & (SDMMC_ACESR_ACMDEND | SDMMC_ACESR_ACMDIDX))
 				cmd->bStatus = SDMMC_ERR_IO;
 			else
 				cmd->bStatus = SDMMC_ERR;
 		}
 		else if (errors & SDMMC_EISTR_ADMA) {
 //#if TRACE_LEVEL >= TRACE_LEVEL_ERROR
 //			const uint32_t desc_ix = (regs->SDMMC_ASA0R -
 //			    (uint32_t)set->table) / (SDMMC_DMADL_SIZE * 4UL);
 //
 //			trace_error("ADMA error 0x%x at desc. line[%lu]\n\r",
 //			    regs->SDMMC_AESR, desc_ix);
 //#endif
 			cmd->bStatus = SDMMC_PARAM;
 		}
 		else if (errors & SDMMC_EISTR_BOOTAE)
 			cmd->bStatus = SDMMC_STATE;
 		else
 			cmd->bStatus = SDMMC_ERR;
 		driver->state = cmd->bCmd == 12 ? MCID_LOCKED : MCID_ERROR;
		//TRACE_3("CMD%u ended with error flags %04x, cmd status %s\n\r", cmd->bCmd, errors, SD_StringifyRetCode(cmd->bStatus));
 		goto End;
 	}

 	/* No error. Give priority to the low-latency event that signals the
 	 * completion of the Auto CMD12 command, hence of the whole multiple-
 	 * block data transfer. */
 	if (events & SDMMC_NISTR_CUSTOM_EVT) {
 //#ifndef NDEBUG
 //		if (!(set->regs->SDMMC_PSR & SDMMC_PSR_CMDLL))
 //			trace_warning("Auto command still ongoing\n\r");
 //#endif
 		if (cmd->pResp) {
 			//TRACE("getting resp\r\n");
 			sdmmc_get_response(driver, cmd, true, cmd->pResp);
 		}
 		goto Succeed;
 	}

 	/* First, expect completion of the command */
 	if (events & SDMMC_NISTR_CMDC) {
 //#ifndef NDEBUG
 //		if (cmd->cmdOp.bmBits.xfrData == SDMMC_CMD_TX
 //		    && !set->table && set->blk_index != cmd->wNbBlocks
 //		    && !(regs->SDMMC_PSR & SDMMC_PSR_WTACT))
 //			trace_warning("Write transfer not started\n\r");
 //		else if (cmd->cmdOp.bmBits.xfrData == SDMMC_CMD_RX
 //		    && !set->table && set->blk_index != cmd->wNbBlocks
 //		    && !(regs->SDMMC_PSR & SDMMC_PSR_RTACT))
 //			trace_warning("Read transfer not started\n\r");
 //#endif
 		/* Clear this normal interrupt */
 		regs->SDMMC_NISTR = SDMMC_NISTR_CMDC;
 		events &= ~SDMMC_NISTR_CMDC;
 		driver->cmd_line_released = true;
 		/* Retrieve command response */
 		if (cmd->pResp) {
 			//TRACE("getting resp..\r\n");
 			sdmmc_get_response(driver, cmd, driver->dat_lines_released,
 			    cmd->pResp);
 		}
 		if ((!has_data && !cmd->cmdOp.bmBits.checkBsy)
 		    || driver->dat_lines_released)
 			goto Succeed;
 	}

 	/* Expect the next incoming block of data */
 	if (events & SDMMC_NISTR_BRDRDY
 	    && cmd->cmdOp.bmBits.xfrData == SDMMC_CMD_RX && !driver->config->dma_table) {
 		/* FIXME may be optimized by looping while PSR.BUFRDEN == 1 */
 		uint8_t *in, *out, *bound;
 		union uint32_u val;
 		uint16_t count;

 		/* Clear this normal interrupt */
 		regs->SDMMC_NISTR = SDMMC_NISTR_BRDRDY;
 		events &= ~SDMMC_NISTR_BRDRDY;

 		if (driver->blk_index >= cmd->wNbBlocks) {
 //			trace_error("Excess of incoming data\n\r");
 			cmd->bStatus = SDMMC_ERR_IO;
 			driver->state = MCID_ERROR;
 			goto End;
 		}
 		out = cmd->pData + driver->blk_index * (uint32_t)cmd->wBlockSize;
 		count = cmd->wBlockSize & ~0x3;
 		for (bound = out + count; out < bound; out += 4) {
 //#ifndef NDEBUG
 //			if (!(regs->SDMMC_PSR & SDMMC_PSR_BUFRDEN))
 //				trace_error("Unexpected Buffer Read Disable status\n\r");
 //#endif
 			val.word = regs->SDMMC_BDPR;
 			out[0] = val.bytes[0];
 			out[1] = val.bytes[1];
 			out[2] = val.bytes[2];
 			out[3] = val.bytes[3];
 		}
 		if (count < cmd->wBlockSize) {
 //#ifndef NDEBUG
 //			if (!(regs->SDMMC_PSR & SDMMC_PSR_BUFRDEN))
 //				trace_error("Unexpected Buffer Read Disable status\n\r");
 //#endif
 			val.word = regs->SDMMC_BDPR;
 			count = cmd->wBlockSize - count;
 			for (in = val.bytes, bound = out + count;
 			    out < bound; in++, out++)
 				*out = *in;
 		}
 #if 0 && !defined(NDEBUG)
 		if (regs->SDMMC_PSR & SDMMC_PSR_BUFRDEN)
 			trace_warning("Renewed Buffer Read Enable status\n\r");
 #endif
 		driver->blk_index++;
 	}

 	/* Expect the Buffer Data Port to be ready to accept the next
 	 * outgoing block of data */
 	if (events & SDMMC_NISTR_BWRRDY
 	    && cmd->cmdOp.bmBits.xfrData == SDMMC_CMD_TX && !driver->config->dma_table
 	    && driver->blk_index < cmd->wNbBlocks) {
 		/* FIXME may be optimized by looping while PSR.BUFWREN == 1 */
 		uint8_t *in, *out, *bound;
 		union uint32_u val;
 		uint16_t count;

 		/* Clear this normal interrupt */
 		regs->SDMMC_NISTR = SDMMC_NISTR_BWRRDY;
 		events &= ~SDMMC_NISTR_BWRRDY;

 		in = cmd->pData + driver->blk_index * (uint32_t)cmd->wBlockSize;
 		count = cmd->wBlockSize & ~0x3;
 		for (bound = in + count; in < bound; in += 4) {
 			val.bytes[0] = in[0];
 			val.bytes[1] = in[1];
 			val.bytes[2] = in[2];
 			val.bytes[3] = in[3];
 //#ifndef NDEBUG
 //			if (!(regs->SDMMC_PSR & SDMMC_PSR_BUFWREN))
 //				trace_error("Unexpected Buffer Write Disable status\n\r");
 //#endif
 			regs->SDMMC_BDPR = val.word;
 		}
 		if (count < cmd->wBlockSize) {
 			count = cmd->wBlockSize - count;
 			for (val.word = 0, out = val.bytes, bound = in + count;
 			    in < bound; in++, out++)
 				*out = *in;
 //#ifndef NDEBUG
 //			if (!(regs->SDMMC_PSR & SDMMC_PSR_BUFWREN))
 //				trace_error("Unexpected Buffer Write Disable status\n\r");
 //#endif
 			regs->SDMMC_BDPR = val.word;
 		}
 #if 0 && !defined(NDEBUG)
 		if (regs->SDMMC_PSR & SDMMC_PSR_BUFWREN)
 			trace_warning("Renewed Buffer Write Enable status\n\r");
 #endif
 		driver->blk_index++;
 	}
 //#ifndef NDEBUG
 //	else if (events & SDMMC_NISTR_BWRRDY
 //	    && cmd->cmdOp.bmBits.xfrData == SDMMC_CMD_TX && !set->table
 //	    && set->blk_index >= cmd->wNbBlocks)
 //		trace_warning("Excess Buffer Write Ready status\n\r");
 //#endif

 	/* Expect completion of either the data transfer or the busy state. */
 	if (events & SDMMC_NISTR_TRFC) {
 		/* Deviation from the SD Host Controller Specification:
 		 * the Auto CMD12 command/response (when enabled) is still in
 		 * progress. We are on our own to figure out when CMD12 will
 		 * have completed.
 		 * In the meantime:
 		 * 1. errors affecting the CMD12 command - essentially
 		 *    SDMMC_EISTR_ACMD - have not been detected yet.
 		 * 2. SDMMC_RR[3] is not yet valid.
 		 * Our workaround here consists in generating a third event
 		 * further to Transfer Complete, after a predefined amount of
 		 * time, sufficient for CMD12 to complete.
 		 * Refer to sdmmc_send_command(), which has prepared our Timer/
 		 * Counter for this purpose. */
 		if (has_data && (cmd->bCmd == 18 || cmd->bCmd == 25)
 		    && !driver->use_set_blk_cnt) {

 			driver->expect_auto_end = true;
 //#ifndef NDEBUG
 //			if (!set->cmd_line_released)
 //				trace_warning("Command still ongoing\n\r");
 //#endif
 		}
 //#ifndef NDEBUG
 //		if (regs->SDMMC_PSR & SDMMC_PSR_WTACT)
 //			trace_error("Write transfer still active\n\r");
 //		if (regs->SDMMC_PSR & SDMMC_PSR_RTACT)
 //			trace_error("Read transfer still active\n\r");
 //#endif
 		/* Clear this normal interrupt */
 		regs->SDMMC_NISTR = SDMMC_NISTR_TRFC;
 		events &= ~SDMMC_NISTR_TRFC;
 		driver->dat_lines_released = true;
 		/* Deviation from the SD Host Controller Specification:
 		 * there are cases, notably CMD7 with address and R1b, where the
 		 * Transfer Complete interrupt precedes Command Complete. In
 		 * such cases, the command/response is still in progress, we
 		 * shall wait for Command Complete. */
 		if (driver->cmd_line_released && !driver->expect_auto_end && cmd->pResp) {
 			//TRACE("getting resp...\r\n");
 			sdmmc_get_response(driver, cmd, true, cmd->pResp);
 		}
 		if (has_data && !driver->config->dma_table
 		    && driver->blk_index != cmd->wNbBlocks) {
 			//trace_error("Incomplete data transfer\n\r");
 			cmd->bStatus = SDMMC_ERR_IO;
 			driver->state = MCID_ERROR;
 			goto End;
 		}
 		if (driver->cmd_line_released && !driver->expect_auto_end)
 			goto Succeed;
 	}

 //#ifndef NDEBUG
 //	if (events)
 //		trace_warning("Unhandled NISTR events: 0x%04x\n\r", events);
 //#endif
 	if (events)
 		regs->SDMMC_NISTR = events;
 	goto Fetch;

 Succeed:
 driver->state = MCID_LOCKED;
 End:
 	/* Clear residual normal interrupts, if any */
 	if (events)
 		regs->SDMMC_NISTR = events;
 #if 0 && !defined(NDEBUG)
 	if (set->resp_len == 1)
 		trace_debug("CMD%u got response %08lx\n\r", cmd->bCmd,
 		    cmd->pResp[0]);
 	else if (set->resp_len == 4)
 		trace_debug("CMD%u got response %08lx %08lx %08lx %08lx\n\r",
 		    cmd->bCmd, cmd->pResp[0], cmd->pResp[1], cmd->pResp[2],
 		    cmd->pResp[3]);
 #endif
 	/* Upon error, recover by resetting the CMD and DAT lines */
 	if (cmd->bStatus != SDMMC_OK && cmd->bStatus != SDMMC_CHANGED) {
 		/* Resetting DAT lines also aborts the DMA transfer - if any -
 		 * and resets the DMA circuit. */
 		regs->SDMMC_SRR |= SDMMC_SRR_SWRSTDAT | SDMMC_SRR_SWRSTCMD;
 		while (regs->SDMMC_SRR & (SDMMC_SRR_SWRSTDAT
 		    | SDMMC_SRR_SWRSTCMD)) ;
 	} else if (cmd->bCmd == 0 || (cmd->bCmd == 6
 	    && cmd->dwArg & 1ul << 31 && !cmd->cmdOp.bmBits.checkBsy)) {
 		/* Currently in the function switching period, wait for the
 		 * delay preconfigured in sdmmc_send_command(). */

 		while ( chSysIsCounterWithinX(chSysGetRealtimeCounterX(),driver->start_cycles ,driver->start_cycles+driver->timeout_cycles) );

 	}

 	/* Release this command */

 	driver->resp_len = 0;
 	driver->blk_index = 0;
 	driver->cmd_line_released = false;
 	driver->dat_lines_released = false;
 	driver->expect_auto_end = false;

 }

 void sdmmc_set_device_clock(SdmmcDriver *driver, uint32_t freq)
 {
 	osalDbgCheck(freq);

 	Sdmmc *regs = driver->regs;
 	uint32_t base_freq, div, low_freq, up_freq, new_freq;
 	uint32_t mult_freq, p_div, p_mode_freq;
 	uint16_t shval;
 	bool use_prog_mode = false;

 	freq = min_u32(freq, 120000000ul);

 	if (!(regs->SDMMC_PCR & SDMMC_PCR_SDBPWR)) {
 		TRACE_ERROR("Bus is off\n\r");
 	}
 	if (regs->SDMMC_HC2R & SDMMC_HC2R_PVALEN) {
 		TRACE_ERROR("Preset values enabled though not implemented\n\r");
 	}

 	/* In the Divided Clock Mode scenario, compute the divider */
 	base_freq = (regs->SDMMC_CA0R & SDMMC_CA0R_BASECLKF_Msk) >> SDMMC_CA0R_BASECLKF_Pos;
 	base_freq *= 1000000UL;
 	/* DIV = FBASECLK / (2 * FSDCLK) */
 	div = base_freq / (2 * freq);
 	if (div >= 0x3ff)
 		div = 0x3ff;
 	else {
 		up_freq = base_freq / (div == 0 ? 1UL : 2 * div);
 		low_freq = base_freq / (2 * (div + 1UL));
 		if (up_freq > freq && (up_freq - freq) > (freq - low_freq))
 			div += 1;
 	}
 	new_freq = base_freq / (div == 0 ? 1UL : 2 * div);

 	/* Now, in the Programmable Clock Mode scenario, compute the divider.
 	 * First, retrieve the frequency of the Generated Clock feeding this
 	 * peripheral. */
 	/* TODO fix CLKMULT value in CA1R capability register: the default value
 	 * is 32 whereas the real value is 40.5 */
 	mult_freq = (regs->SDMMC_CA1R & SDMMC_CA1R_CLKMULT_Msk) >> SDMMC_CA1R_CLKMULT_Pos;
 	if (mult_freq != 0)
 #if 0
 		mult_freq = base_freq * (mult_freq + 1);
 #else
 		mult_freq = SAMA_MCK ;// pmc_get_gck_clock(ID_SDMMC0+driver->config->slot_id);
 #endif
 	if (mult_freq != 0) {
 		/* DIV = FMULTCLK / FSDCLK - 1 */
 		p_div = CEIL_INT_DIV(mult_freq, freq);
 		if (p_div > 0x3ff)
 			p_div = 0x3ff;
 		else if (p_div != 0)
 			p_div = p_div - 1;
 		p_mode_freq = mult_freq / (p_div + 1);
 		if (ABS_DIFF(freq, p_mode_freq) < ABS_DIFF(freq, new_freq)) {
 			use_prog_mode = true;
 			div = p_div;
 			new_freq = p_mode_freq;
 		}
 	}

 	/* Stop the output clock, so we can change the frequency.
 	 * Deviation from the SD Host Controller Specification: if the internal
 	 * clock was temporarily disabled, the controller would then switch to
 	 * an irrelevant clock frequency.
 	 * This issue has been observed, notably, when trying to switch from 25
 	 * to 50 MHz. Keep the SDMMC internal clock enabled. */
 	shval = regs->SDMMC_CCR & ~SDMMC_CCR_SDCLKEN;
 	regs->SDMMC_CCR = shval;
 	driver->dev_freq = new_freq;
 	use_prog_mode = false; //no generated clock
 	/* Select the clock mode */
 	if (use_prog_mode)
 		shval |= SDMMC_CCR_CLKGSEL;
 	else
 		shval &= ~SDMMC_CCR_CLKGSEL;
 	/* Set the clock divider, and start the SDMMC internal clock - if it
 	 * wasn't started yet. */
 	shval = (shval & ~SDMMC_CCR_USDCLKFSEL_Msk & ~SDMMC_CCR_SDCLKFSEL_Msk)
 	    | SDMMC_CCR_USDCLKFSEL(div >> 8) | SDMMC_CCR_SDCLKFSEL(div & 0xff)
 	    | SDMMC_CCR_INTCLKEN;
 	regs->SDMMC_CCR = shval;
 	while (!(regs->SDMMC_CCR & SDMMC_CCR_INTCLKS)) ;
 	/* Now start the output clock */
 	regs->SDMMC_CCR |= SDMMC_CCR_SDCLKEN;
 }




 /**
  * Here is the fSdmmcSendCommand-type callback.
  * SD/MMC command.
  * \param _set  Pointer to driver instance data (struct sdmmc_set).
  * \param cmd  Pointer to the command to be sent. Owned by the caller. Shall
  * remain valid until the command is completed or stopped. For commands which
  * transfer data, mind the peripheral and DMA alignment requirements that the
  * external data buffer shall meet. Especially when DMA is used to read from the
  * device, in which case the buffer shall be aligned on entire cache lines.
  * \return Return code, from the eSDMMC_RC enumeration. If SDMMC_OK, the command
  * has been issued and the caller should:
  *   1. poll on sdmmc_is_busy(),
  *   2. once finished, check the result of the command in cmd->bStatus.
  * TODO in future when libsdmmc will set it: call sSdmmcCommand::fCallback.
  */
 uint32_t sdmmc_device_command(SdmmcDriver *driver)
 {
 	osalDbgCheck(driver->cmd.bCmd <= 63);

 	Sdmmc *regs = driver->regs;

 	const bool stop_xfer = driver->cmd.cmdOp.bmBits.xfrData == SDMMC_CMD_STOPXFR;

 	const bool has_data = (driver->cmd.cmdOp.bmBits.xfrData == SDMMC_CMD_TX) || (driver->cmd.cmdOp.bmBits.xfrData == SDMMC_CMD_RX);

 	const bool use_dma = (bool)	(driver->use_polling == false) &&
 								(driver->cmd.bCmd != 21 || driver->tim_mode >= SDMMC_TIM_SD_DS) &&
 								(driver->cmd.bCmd != 19 || driver->tim_mode < SDMMC_TIM_SD_DS);

 	const bool wait_switch = 	(driver->cmd.bCmd == 0) ||
 								(driver->cmd.bCmd == 6 && driver->cmd.dwArg & 1ul << 31 && !driver->cmd.cmdOp.bmBits.checkBsy);

 	const bool multiple_xfer = (driver->cmd.bCmd == 18 ) || ( driver->cmd.bCmd == 25 );

 	const bool blk_count_prefix = (driver->cmd.bCmd == 18 || driver->cmd.bCmd == 25) && driver->use_set_blk_cnt;

 	const bool stop_xfer_suffix = (driver->cmd.bCmd == 18 || driver->cmd.bCmd == 25) && !driver->use_set_blk_cnt;

 	uint32_t eister;
 	uint32_t mask;
 	uint32_t len;
 	uint16_t cr;
 	uint16_t tmr;

 	uint8_t mc1r;
 	uint8_t rc = SDMMC_OK;

 	TRACE_DEBUG_1("[command] start %d\r\n",driver->cmd.bCmd);

 	if (driver->state == MCID_OFF)
 		return SDMMC_STATE;

 	if (driver->cmd.cmdOp.bmBits.powerON == driver->cmd.cmdOp.bmBits.sendCmd) {
 		//trace_error("Invalid command\n\r");
 		return SDMMC_PARAM;
 	}
 	if (stop_xfer && driver->cmd.bCmd != 12 && driver->cmd.bCmd != 52) {
 		//trace_error("Inconsistent abort command\n\r");
 		return SDMMC_PARAM;
 	}
 	if (driver->cmd.cmdOp.bmBits.powerON) {
 		/* Special call, no command to send this time */
 		/* Wait for 74 SD Clock cycles, as per SD Card specification.
 		 * The e.MMC Electrical Standard specifies tRSCA >= 200 usec. */
 		if (driver->dev_freq == 0) {
 			//trace_error("Shall enable the device clock first\n\r");
 			return SDMMC_STATE;
 		}

 		return SDMMC_OK;
 	}

 	if (has_data && (driver->cmd.wNbBlocks == 0 || driver->cmd.wBlockSize == 0
 	    || driver->cmd.pData == NULL)) {
 		//trace_error("Invalid data\n\r");
 		return SDMMC_PARAM;
 	}
 	if (has_data && driver->cmd.wBlockSize > driver->blk_size) {
 		//trace_error("%u-byte data block size not supported\n\r", driver->cmd.wBlockSize);
 		return SDMMC_PARAM;
 	}

 	if (has_data && use_dma) {
 		/* Using DMA. Prepare the descriptor table. */
 		rc = sdmmc_build_dma_table(driver);

 		if (rc != SDMMC_OK && rc != SDMMC_CHANGED)
 			return rc;

 		len = (uint32_t)driver->cmd.wNbBlocks * (uint32_t)driver->cmd.wBlockSize;

 		if (driver->cmd.cmdOp.bmBits.xfrData == SDMMC_CMD_TX) {
 			/* Ensure the outgoing data can be fetched directly from
 			 * RAM */
 			cacheCleanRegion(driver->cmd.pData, len);
 		}
 		else if (driver->cmd.cmdOp.bmBits.xfrData == SDMMC_CMD_RX) {
 			/* Invalidate the corresponding data cache lines now, so
 			 * this buffer is protected against a global cache clean
 			 * operation, that concurrent code may trigger.
 			 * Warning: until the command is reported as complete,
 			 * no code should read from this buffer, nor from
 			 * variables cached in the same lines. If such
 			 * anticipated reading had to be supported, the data
 			 * cache lines would need to be invalidated twice: both
 			 * now and upon Transfer Complete. */
      if(((uint32_t) driver->cmd.pData & (L1_CACHE_BYTES - 1)) || (len & (L1_CACHE_BYTES - 1))) {
        cacheCleanInvalidateRegion(driver->cmd.pData, len);
      }
      else {
        cacheInvalidateRegion(driver->cmd.pData, len);
      }
 		}
 	}

 	///if (multiple_xfer && !has_data)
 	//	trace_warning("Inconsistent data\n\r");
 	if (sdmmc_is_busy(driver)) {
 		//trace_error("Concurrent command\n\r");
 		return SDMMC_BUSY;
 	}
 	driver->state = MCID_CMD;
 	driver->resp_len = 0;
 	driver->blk_index = 0;
 	driver->cmd_line_released = false;
 	driver->dat_lines_released = false;
 	driver->expect_auto_end = false;
 	driver->cmd.bStatus = rc;
 	TRACE_DEBUG_1("command set status %d\r\n",driver->cmd.bStatus);

 	tmr = (regs->SDMMC_TMR & ~SDMMC_TMR_MSBSEL & ~SDMMC_TMR_DTDSEL
 	    & ~SDMMC_TMR_ACMDEN_Msk & ~SDMMC_TMR_BCEN & ~SDMMC_TMR_DMAEN)
 	    | SDMMC_TMR_ACMDEN_DIS;
 	mc1r = (regs->SDMMC_MC1R & ~SDMMC_MC1R_OPD & ~SDMMC_MC1R_CMDTYP_Msk)
 	    | SDMMC_MC1R_CMDTYP_NORMAL;
 	cr = (regs->SDMMC_CR & ~SDMMC_CR_CMDIDX_Msk & ~SDMMC_CR_CMDTYP_Msk
 	    & ~SDMMC_CR_DPSEL & ~SDMMC_CR_RESPTYP_Msk)
 	    | SDMMC_CR_CMDIDX(driver->cmd.bCmd) | SDMMC_CR_CMDTYP_NORMAL
 	    | SDMMC_CR_CMDICEN | SDMMC_CR_CMDCCEN;
 	eister = SDMMC_EISTER_BOOTAE | SDMMC_EISTER_TUNING | SDMMC_EISTER_ADMA
 	    | SDMMC_EISTER_ACMD | SDMMC_EISTER_CURLIM | SDMMC_EISTER_DATEND
 	    | SDMMC_EISTER_DATCRC | SDMMC_EISTER_DATTEO | SDMMC_EISTER_CMDIDX
 	    | SDMMC_EISTER_CMDEND | SDMMC_EISTER_CMDCRC | SDMMC_EISTER_CMDTEO;

 	if (driver->cmd.cmdOp.bmBits.odON)
 		mc1r |= SDMMC_MC1R_OPD;

 	switch (driver->cmd.cmdOp.bmBits.respType) {

 	case 2:
 		cr |= SDMMC_CR_RESPTYP_RL136;
 		/* R2 response doesn't include the command index */
 		eister &= ~SDMMC_EISTER_CMDIDX;
 		break;
 	case 3:
 		/* R3 response includes neither the command index nor the CRC */
 		eister &= ~(SDMMC_EISTER_CMDIDX | SDMMC_EISTER_CMDCRC);
 	case 1:
 	case 4:
 		if (driver->cmd.cmdOp.bmBits.respType == 4 && driver->cmd.cmdOp.bmBits.ioCmd)
 			/* SDIO R4 response includes neither the command index nor the CRC */
 			eister &= ~(SDMMC_EISTER_CMDIDX | SDMMC_EISTER_CMDCRC);
 	case 5:
 	case 6:
 	case 7:
 		cr |= driver->cmd.cmdOp.bmBits.checkBsy ? SDMMC_CR_RESPTYP_RL48BUSY : SDMMC_CR_RESPTYP_RL48;
 		break;
 	default:
 		/* No response, ignore response time-out error */
 		cr |= SDMMC_CR_RESPTYP_NORESP;
 		eister &= ~SDMMC_EISTER_CMDTEO;
 		break;

 	}

 	if (stop_xfer) {
 		tmr |= SDMMC_TMR_MSBSEL | SDMMC_TMR_BCEN;
 		/* TODO consider BGCR:STPBGR (pause) */
 		/* TODO in case of SDIO consider CR:CMDTYP = ABORT */
 		/* Ignore data errors */
 		eister = eister & ~SDMMC_EISTER_ADMA & ~SDMMC_EISTER_DATEND
 		    & ~SDMMC_EISTER_DATCRC & ~SDMMC_EISTER_DATTEO;
 	}
 	else if (has_data) {
 		cr |= SDMMC_CR_DPSEL;
 		tmr |= driver->cmd.cmdOp.bmBits.xfrData == SDMMC_CMD_TX
 		    ? SDMMC_TMR_DTDSEL_WR : SDMMC_TMR_DTDSEL_RD;
 		if (blk_count_prefix)
 			tmr = (tmr & ~SDMMC_TMR_ACMDEN_Msk)
 			    | SDMMC_TMR_ACMDEN_ACMD23;
 		else if (stop_xfer_suffix)
 			tmr = (tmr & ~SDMMC_TMR_ACMDEN_Msk)
 			    | SDMMC_TMR_ACMDEN_ACMD12;
 		/* TODO check if this is fine for SDIO too (byte or block transfer) (driver->cmd.cmdOp.bmBits.ioCmd, driver->cmd.wBlockSize) */
 		if (multiple_xfer || driver->cmd.wNbBlocks > 1)
 			tmr |= SDMMC_TMR_MSBSEL | SDMMC_TMR_BCEN;
 		if (use_dma)
 			tmr |= SDMMC_TMR_DMAEN;
 	}

 	/* Wait for the CMD and DATn lines to be ready. If a previous command
 	 * is still being processed, mind the status flags it may raise. */
 	mask = SDMMC_PSR_CMDINHC;
 	if (has_data || (driver->cmd.cmdOp.bmBits.checkBsy && !stop_xfer))
 		mask |= SDMMC_PSR_CMDINHD;

 	while (regs->SDMMC_PSR & mask) ;

 	/* Enable normal interrupts */
 	regs->SDMMC_NISTER |= SDMMC_NISTER_BRDRDY | SDMMC_NISTER_BWRRDY
 	    | SDMMC_NISTER_TRFC | SDMMC_NISTER_CMDC;

 	osalDbgCheck(!(regs->SDMMC_NISTER & SDMMC_NISTR_CUSTOM_EVT));
 	/* Enable error interrupts */

 	regs->SDMMC_EISTER = eister;
 	/* Clear all interrupt status flags */
 	regs->SDMMC_NISTR = SDMMC_NISTR_ERRINT | SDMMC_NISTR_BOOTAR
 	    | SDMMC_NISTR_CINT | SDMMC_NISTR_CREM | SDMMC_NISTR_CINS
 	    | SDMMC_NISTR_BRDRDY | SDMMC_NISTR_BWRRDY | SDMMC_NISTR_DMAINT
 	    | SDMMC_NISTR_BLKGE | SDMMC_NISTR_TRFC | SDMMC_NISTR_CMDC;

 	regs->SDMMC_EISTR = SDMMC_EISTR_BOOTAE | SDMMC_EISTR_TUNING
 	    | SDMMC_EISTR_ADMA | SDMMC_EISTR_ACMD | SDMMC_EISTR_CURLIM
 	    | SDMMC_EISTR_DATEND | SDMMC_EISTR_DATCRC | SDMMC_EISTR_DATTEO
 	    | SDMMC_EISTR_CMDIDX | SDMMC_EISTR_CMDEND | SDMMC_EISTR_CMDCRC
 	    | SDMMC_EISTR_CMDTEO;

 	/* Issue the command */
 	if (has_data) {
 		if (blk_count_prefix)
 			regs->SDMMC_SSAR = SDMMC_SSAR_ARG2(driver->cmd.wNbBlocks);

 		if (use_dma)
 			regs->SDMMC_ASA0R = SDMMC_ASA0R_ADMASA((uint32_t)driver->config->dma_table);

 		regs->SDMMC_BSR = (regs->SDMMC_BSR & ~SDMMC_BSR_BLKSIZE_Msk) | SDMMC_BSR_BLKSIZE(driver->cmd.wBlockSize);
 	}

 	if (stop_xfer)
 		regs->SDMMC_BCR = SDMMC_BCR_BLKCNT(0);
 	else if (has_data && (multiple_xfer || driver->cmd.wNbBlocks > 1))
 		regs->SDMMC_BCR = SDMMC_BCR_BLKCNT(driver->cmd.wNbBlocks);

 	regs->SDMMC_ARG1R = driver->cmd.dwArg;

 	if (has_data || stop_xfer)
 		regs->SDMMC_TMR = tmr;

 	regs->SDMMC_MC1R = mc1r;
 	regs->SDMMC_CR = cr;

 	/* In the case of Auto CMD12, we'll need to generate an extra event.
 	 * Have our Timer/Counter ready for this. */
 	if (has_data && stop_xfer_suffix) {
 		/* Considering the multiple block read mode,
 		 * 1. Assuming Transfer Complete is raised upon successful
 		 *    reception of the End bit of the last data packet,
 		 * 2. A SD/eMMC protocol analyzer shows that the CMD12 command
 		 *    token is fully transmitted 1 or 2 device clock cycles
 		 *    later,
 		 * 3. The device may take up to 64 clock cycles (NCR) before
 		 *    initiating the CMD12 response token,
 		 * 4. The code length of the CMD12 response token (R1) is 48
 		 *    bits, hence 48 device clock cycles.
 		 * The sum of the above timings is the maximum time CMD12 will
 		 * take to complete. */

 		driver->timeout_cycles = 2+64+48;
 		driver->start_cycles = chSysGetRealtimeCounterX();


 	}
 	/* With SD devices, the 8-cycle function switching period will apply,
 	 * further to both SWITCH_FUNC and GO_IDLE_STATE commands.
 	 * Note that MMC devices don't require this fixed delay, but regarding
 	 * GO_IDLE_STATE we have no mean to filter the MMC requests out. */
 	else if (wait_switch) {

 		driver->timeout_ticks = 8;
 		driver->start_cycles = chSysGetRealtimeCounterX();

 	}
 	if (!driver->use_polling) {
 		regs->SDMMC_NISIER |= SDMMC_NISIER_BRDRDY | SDMMC_NISIER_BWRRDY | SDMMC_NISIER_TRFC | SDMMC_NISIER_CMDC | SDMMC_NISIER_CINT;
 		regs->SDMMC_EISIER = eister;
 	}

 	TRACE_DEBUG_1("[command] finish %d OK\r\n",driver->cmd.bCmd);
 	return SDMMC_OK;
 }



 /**
  * Here is the fSdmmcIOCtrl-type callback.
  * IO control functions.
  * \param _set  Pointer to driver instance data (struct sdmmc_set).
  * \param bCtl  IO control code.
  * \param param  IO control parameter. Optional, depends on the IO control code.
  * \return Return code, from the eSDMMC_RC enumeration.
  */
 uint32_t sdmmc_device_control(SdmmcDriver *driver, uint32_t bCtl)
 {

 	uint32_t rc = SDMMC_OK;

 	uint8_t byte;


 	if (bCtl != SDMMC_IOCTL_BUSY_CHECK && bCtl != SDMMC_IOCTL_GET_DEVICE) {
 		TRACE_DEBUG_2("SDMMC_IOCTL_%s(%lu)\n\r", SD_StringifyIOCtrl(bCtl),driver->control_param );
 	}


 	switch (bCtl) {
 	case SDMMC_IOCTL_GET_DEVICE:

 		if ((driver->regs->SDMMC_CA0R & SDMMC_CA0R_SLTYPE_Msk)  == SDMMC_CA0R_SLTYPE_EMBEDDED)
 			driver->control_param = 1;
 		else
 			driver->control_param = driver->regs->SDMMC_PSR & SDMMC_PSR_CARDINS ? 1 : 0;
 		break;

 	case SDMMC_IOCTL_GET_WP:

 		if ((driver->regs->SDMMC_CA0R & SDMMC_CA0R_SLTYPE_Msk) == SDMMC_CA0R_SLTYPE_EMBEDDED)
 			driver->control_param = 1;
 		else
 			driver->control_param = driver->regs->SDMMC_PSR & SDMMC_PSR_WRPPL   ? 1 : 0;
 		break;

 	case SDMMC_IOCTL_POWER:

 		if (driver->control_param > SDMMC_PWR_STD_VDD_LOW_IO)
 			return SDMMC_PARAM;
 		if (driver->control_param == SDMMC_PWR_OFF)
 			rc = unplug_device(driver);
 		else if (driver->control_param == SDMMC_PWR_STD_VDD_LOW_IO && !(driver->regs->SDMMC_CA0R & SDMMC_CA0R_V18VSUP))
 			return SDMMC_PARAM;
 		else {
 			/* Power the device on, or change signaling level.
 			 * This can't be done without configuring the timing
 			 * mode at the same time. */
 			byte = driver->tim_mode;
 			if ((driver->regs->SDMMC_CA0R & (SDMMC_CA0R_V18VSUP| SDMMC_CA0R_V30VSUP| SDMMC_CA0R_V33VSUP)) != SDMMC_CA0R_V18VSUP) {
 				if (driver->control_param == SDMMC_PWR_STD_VDD_LOW_IO) {
 					if (byte < SDMMC_TIM_SD_DS)
 						byte = SDMMC_TIM_MMC_HS200;
 					else if (byte < SDMMC_TIM_SD_SDR12)
 						byte = SDMMC_TIM_SD_SDR12;
 				}
 				else {
 					if (byte > SDMMC_TIM_SD_HS)
 						byte = SDMMC_TIM_SD_DS;
 					else if (byte > SDMMC_TIM_MMC_HS_DDR
 					    && byte < SDMMC_TIM_SD_DS)
 						byte = SDMMC_TIM_MMC_BC;
 				}
 			}
 			rc = sdmmc_set_speed_mode(driver, byte, true);
 		}
 		break;

 	case SDMMC_IOCTL_RESET:
 		/* Release the device. The device may have been removed. */
 		rc = unplug_device(driver);
 		break;

 	case SDMMC_IOCTL_GET_BUSMODE:
 		byte = sdmmc_get_bus_width(driver);
 		driver->control_param = byte;
 		break;

 	case SDMMC_IOCTL_SET_BUSMODE:
 		if (driver->control_param > 0xff)
 			return SDMMC_PARAM;
 		rc = sdmmc_set_bus_width(driver, driver->control_param);
 		TRACE_DEBUG_1("Using a %u-bit data bus\n\r", sdmmc_get_bus_width(driver));
 		break;

 	case SDMMC_IOCTL_GET_HSMODE:

 		if (driver->control_param > 0xff) {
 			driver->control_param = 0;
 			break;
 		}

 		byte = (uint8_t)driver->control_param;

 		if (byte == SDMMC_TIM_MMC_BC || byte == SDMMC_TIM_SD_DS)  {
 			driver->control_param = 1;
 		}
 		else if ((byte == SDMMC_TIM_MMC_HS_SDR
 		    || byte == SDMMC_TIM_MMC_HS_DDR || byte == SDMMC_TIM_SD_HS)
 		    && driver->regs->SDMMC_CA0R & SDMMC_CA0R_HSSUP)
 			driver->control_param = 1;
 		else if (byte == SDMMC_TIM_MMC_HS200
 		    && (driver->regs->SDMMC_CA0R & (SDMMC_CA0R_V18VSUP
 		    | SDMMC_CA0R_V30VSUP | SDMMC_CA0R_V33VSUP))
 		    == SDMMC_CA0R_V18VSUP
 		    && driver->regs->SDMMC_CA1R & (SDMMC_CA1R_SDR50SUP
 		    | SDMMC_CA1R_DDR50SUP | SDMMC_CA1R_SDR104SUP))
 			driver->control_param = 1;
 		/* TODO rely on platform code to get the data bus width to the
 		 * SD slot, and deny UHS-I timing modes if the DAT[3:0] signals
 		 * are not all routed. */
 		else if ((byte == SDMMC_TIM_SD_SDR12
 		    || byte == SDMMC_TIM_SD_SDR25)
 		    && driver->regs->SDMMC_CA0R & SDMMC_CA0R_V18VSUP
 		    && driver->regs->SDMMC_CA1R & (SDMMC_CA1R_SDR50SUP
 		    | SDMMC_CA1R_DDR50SUP | SDMMC_CA1R_SDR104SUP))
 			driver->control_param= 1;
 		else if (byte == SDMMC_TIM_SD_SDR50
 		    && driver->regs->SDMMC_CA0R & SDMMC_CA0R_V18VSUP
 		    && driver->regs->SDMMC_CA1R & SDMMC_CA1R_SDR50SUP)
 			driver->control_param = 1;
 		else if (byte == SDMMC_TIM_SD_DDR50
 		    && driver->regs->SDMMC_CA0R & SDMMC_CA0R_V18VSUP
 		    && driver->regs->SDMMC_CA1R & SDMMC_CA1R_DDR50SUP)
 			driver->control_param = 1;
 		else if (byte == SDMMC_TIM_SD_SDR104
 		    && driver->regs->SDMMC_CA0R & SDMMC_CA0R_V18VSUP
 		    && driver->regs->SDMMC_CA1R & SDMMC_CA1R_SDR104SUP)
 			driver->control_param = 1;
 		else
 			driver->control_param = 0;
 		break;

 	case SDMMC_IOCTL_SET_HSMODE:

 		if (driver->control_param > 0xff)
 			return SDMMC_PARAM;
 		rc = sdmmc_set_speed_mode(driver, (uint8_t)driver->control_param, false);
 		driver->control_param= driver->tim_mode;
 		break;

 	case SDMMC_IOCTL_SET_CLOCK:

 		if (driver->control_param == 0)
 			return SDMMC_PARAM;

 		sdmmc_set_device_clock(driver, driver->control_param);

 		TRACE_DEBUG_1("Clocking the device at %lu Hz\n\r", driver->dev_freq);
 		if (driver->dev_freq > 95000000ul
 		    && (driver->tim_mode == SDMMC_TIM_MMC_HS200
 		    || driver->tim_mode == SDMMC_TIM_SD_SDR104
 		    || (driver->tim_mode == SDMMC_TIM_SD_SDR50
 		    && driver->regs->SDMMC_CA1R & SDMMC_CA1R_TSDR50)))
 			rc = tuneSampling(driver);
 			/* TODO setup periodic re-tuning */
 		if (driver->dev_freq != driver->control_param) {
 			rc = rc == SDMMC_OK ? SDMMC_CHANGED : rc;
 			driver->control_param = driver->dev_freq;
 		}
 		break;

 	case SDMMC_IOCTL_SET_LENPREFIX:

 		driver->use_set_blk_cnt = driver->control_param ? true : false;
 		driver->control_param = driver->use_set_blk_cnt ? 1 : 0;
 		break;

 	case SDMMC_IOCTL_GET_XFERCOMPL:

 		driver->control_param = 1;
 		break;

 	case SDMMC_IOCTL_BUSY_CHECK:

 		if (driver->state == MCID_OFF)
 			driver->control_param = 0;
 		else
 		{

 			if (driver->use_polling) {
 					sdmmc_device_poll(driver);
 			}
 				if (driver->state == MCID_CMD) {
 					driver->control_param =1;
 				}
 				else {
 				driver->control_param = 0;
 				}

 		}
 		break;

 	case SDMMC_IOCTL_CANCEL_CMD:
 		if (driver->state == MCID_OFF)
 			rc = SDMMC_STATE;
 		else
 			rc = CancelCommand(driver);
 		break;

 	case SDMMC_IOCTL_GET_CLOCK:
 	case SDMMC_IOCTL_SET_BOOTMODE:
 	case SDMMC_IOCTL_GET_BOOTMODE:
 	default:
 		rc = SDMMC_NOT_SUPPORTED;
 		break;
 	}

 	if (rc != SDMMC_OK && rc != SDMMC_CHANGED
 	    && bCtl != SDMMC_IOCTL_BUSY_CHECK) {
 		TRACE_ERROR_2("SDMMC_IOCTL_%s ended with %s\n\r",SD_StringifyIOCtrl(bCtl), SD_StringifyRetCode(rc));
 	}

 	return rc;
 }

 void  sdmmc_device_sleep(SdmmcDriver *driver,uint32_t t,uint32_t m)
 {
 	systime_t time, end, now;
 	uint32_t f = 0;

 	(void)driver;

 	time =  chVTGetSystemTimeX();

 	if (m==1)
 		end = time + TIME_MS2I(t);
 	else if (m==2)
 		end = time + TIME_US2I(t);
 	else
 		end = time + (systime_t)t;

 	do {

 		now =  chVTGetSystemTimeX(); /* chVTTimeElapsedSinceX(time) */

 		if (now >= end) {
 			f = 1;
 		}

 	} while (!f);

 }

 void sdmmc_device_startTimeCount(SdmmcDriver *driver)
 {
 	if (driver->timeout_elapsed != -1) {
 		driver->time = chVTGetSystemTimeX();
 		driver->now = driver->time;
 	}
 }

void sdmmc_device_checkTimeCount(SdmmcDriver *driver)
{
  if (driver->timeout_elapsed != -1) {

    driver->timeout_elapsed = 0;
    driver->now = chVTTimeElapsedSinceX(driver->time);
    if (driver->now >= driver->timeout_ticks ) {
      driver->timeout_elapsed = 1;
    }

  }
}

 static void calibrate_zout(Sdmmc * regs)
 {
 	uint32_t calcr;

 	/* FIXME find out if this operation should be carried with PCR:SDBPWR
 	 * set and/or the device clock started. */

 	/* CALCR:CNTVAL has been configured by sdmmc_initialize() */
 	regs->SDMMC_CALCR |= SDMMC_CALCR_EN;
 	do
 		calcr = regs->SDMMC_CALCR;
 	while (calcr & SDMMC_CALCR_EN);
 	//trace_debug("Output Z calibr. CALN=%lu CALP=%lu\n\r",
 	//    (calcr & SDMMC_CALCR_CALN_Msk) >> SDMMC_CALCR_CALN_Pos,
 	//    (calcr & SDMMC_CALCR_CALP_Msk) >> SDMMC_CALCR_CALP_Pos);
 }

 void reset_peripheral(SdmmcDriver *driver)
 {

 	uint32_t calcr;
 	uint8_t mc1r, tcr;

 	/* First, save the few settings we'll want to restore. */
 	mc1r = driver->regs->SDMMC_MC1R;
 	tcr = driver->regs->SDMMC_TCR;
 	calcr = driver->regs->SDMMC_CALCR;

 	/* Reset our state variables to match reset values of the registers */
 	driver->tim_mode = driver->tim_mode >= SDMMC_TIM_SD_DS ? SDMMC_TIM_SD_DS
 	    : SDMMC_TIM_MMC_BC;

 	/* Reset the peripheral. This will reset almost all registers. */
 	driver->regs->SDMMC_SRR |= SDMMC_SRR_SWRSTALL;
 	while (driver->regs->SDMMC_SRR & SDMMC_SRR_SWRSTALL) ;

 	/* Restore specific register fields */
 	if (mc1r & SDMMC_MC1R_FCD)
 		driver->regs->SDMMC_MC1R |= SDMMC_MC1R_FCD;
 	driver->regs->SDMMC_TCR = (driver->regs->SDMMC_TCR & ~SDMMC_TCR_DTCVAL_Msk)
 	    | (tcr & SDMMC_TCR_DTCVAL_Msk);
 	driver->regs->SDMMC_CALCR = (driver->regs->SDMMC_CALCR & ~SDMMC_CALCR_CNTVAL_Msk
 	    & ~SDMMC_CALCR_TUNDIS) | (calcr & SDMMC_CALCR_CNTVAL_Msk);

 	/* Apply our unconditional custom settings */
 	/* When using DMA, use the 32-bit Advanced DMA 2 mode */
 	driver->regs->SDMMC_HC1R = (driver->regs->SDMMC_HC1R & ~SDMMC_HC1R_DMASEL_Msk)
 	    | SDMMC_HC1R_DMASEL_ADMA32;
 	/* Configure maximum AHB burst size */
 	driver->regs->SDMMC_ACR = (driver->regs->SDMMC_ACR & ~SDMMC_ACR_BMAX_Msk)
 	    | SDMMC_ACR_BMAX_INCR16;
 }



 void sdmmc_set_capabilities(
 		Sdmmc * regs,
 		uint32_t caps0, uint32_t caps0_mask,
 		uint32_t caps1, uint32_t caps1_mask)
 {
 	osalDbgCheck((caps0 & caps0_mask) == caps0);
 	osalDbgCheck((caps1 & caps1_mask) == caps1);

 	caps0 = (regs->SDMMC_CA0R & ~caps0_mask) | (caps0 & caps0_mask);
 	caps1 = (regs->SDMMC_CA1R & ~caps1_mask) | (caps1 & caps1_mask);

 	regs->SDMMC_CACR = SDMMC_CACR_KEY(0x46) | SDMMC_CACR_CAPWREN;
 	if (regs->SDMMC_CA0R != caps0)
 		regs->SDMMC_CA0R = caps0;
 	if (regs->SDMMC_CA1R != caps1)
 		regs->SDMMC_CA1R = caps1;
 	regs->SDMMC_CACR = SDMMC_CACR_KEY(0x46) | 0;
 }

 /**
  * \brief Retrieve command response from the SDMMC peripheral.
  */
 static void sdmmc_get_response(SdmmcDriver *driver, sSdmmcCommand *cmd, bool complete, uint32_t *out)
 {
 	//osalDbgCheck(set);
 	osalDbgCheck(cmd);
 	osalDbgCheck(cmd->cmdOp.bmBits.respType <= 7);
 	osalDbgCheck(out);

 	const bool first_call = driver->resp_len == 0;
 	const bool has_data = cmd->cmdOp.bmBits.xfrData == SDMMC_CMD_TX
 	    || cmd->cmdOp.bmBits.xfrData == SDMMC_CMD_RX;
 	uint32_t resp;
 	uint8_t ix;

 	if (first_call) {
 		switch (cmd->cmdOp.bmBits.respType) {
 		case 2:
 			/* R2 response is 120-bit long, split in
 			 * 32+32+32+24 bits this way:
 			 * RR[0] =    R[ 39:  8]
 			 * RR[1] =    R[ 71: 40]
 			 * RR[2] =    R[103: 72]
 			 * RR[3] =    R[127:104]
 			 * Shift data the way libsdmmc expects it,
 			 * that is:
 			 * pResp[0] = R[127: 96]
 			 * pResp[1] = R[ 95: 64]
 			 * pResp[2] = R[ 63: 32]
 			 * pResp[3] = R[ 31:  0]
 			 * The CRC7 and the end bit aren't provided,
 			 * just hard-code their default values. */
 			out[3] = 0x000000ff;
 			for (ix = 0; ix < 4; ix++) {
 				resp = driver->regs->SDMMC_RR[ix];
 				if (ix < 3)
 					out[2 - ix] = resp >> 24 & 0xff;
 				out[3 - ix] |= resp << 8 & 0xffffff00;
 			}
 			driver->resp_len = 4;
 			break;
 		case 1: case 3: case 4: case 5: case 6: case 7:
 			/* The nominal response is 32-bit long */
 			out[0] = driver->regs->SDMMC_RR[0];
 			driver->resp_len = 1;
 			break;
 		case 0:
 		default:
 			break;
 		}
 	}

 	if (has_data && (cmd->bCmd == 18 || cmd->bCmd == 25) && ((first_call
 	    && driver->use_set_blk_cnt) || (complete && !driver->use_set_blk_cnt))) {
 		resp =  driver->regs->SDMMC_RR[3];
 #if 0
 		trace_debug("Auto CMD%d returned status 0x%lx\n\r",
 		    set->use_set_blk_cnt ? 23 : 12, resp);
 #endif
 		if (!driver->use_set_blk_cnt)
 			/* We return a single response to the application: the
 			 * device status returned by CMD18 or CMD25, combined
 			 * with the device status just returned by Auto CMD12.
 			 * Retain the status bits from only CMD18 or CMD25, and
 			 * combine the exception bits from both. */
 			out[0] |= resp & ~STAT_DEVICE_IS_LOCKED
 			    & ~STAT_CARD_ECC_DISABLED & ~STAT_CURRENT_STATE
 			    & ~STAT_READY_FOR_DATA & ~STAT_EXCEPTION_EVENT
 			    & ~STAT_APP_CMD;
 //#ifndef NDEBUG
 //		resp = (resp & STAT_CURRENT_STATE) >> 9;
 //		if (driver->config->use_set_blk_cnt && resp != STATE_TRANSFER)
 //			trace_warning("Auto CMD23 returned state %lx\n\r", resp);
 //		else if (!driver->config->use_set_blk_cnt && cmd->bCmd == 18
 //		    && resp != STATE_SENDING_DATA)
 //			trace_warning("CMD18 switched to state %lx\n\r", resp);
 //		else if (!driver->config->use_set_blk_cnt && cmd->bCmd == 25
 ///		    && resp != STATE_RECEIVE_DATA && resp != STATE_PROGRAMMING)
 //			trace_warning("CMD25 switched to state %lx\n\r", resp);
 //#endif
 	}
 }

bool sdmmc_is_busy(SdmmcDriver *driver)
{
	//osalDbgCheck(driver->state != MCID_OFF);

	if (driver->use_polling)
		sdmmc_device_poll(driver);
	if (driver->state == MCID_CMD)
		return true;
	return false;
}


static uint8_t sdmmc_build_dma_table( SdmmcDriver *driver )
{
	//assert(set);
	//assert(set->table);
	//assert(set->table_size);
	//assert(cmd->pData);
	//assert(cmd->wBlockSize);
	//assert(cmd->wNbBlocks);
	sSdmmcCommand *cmd = &driver->cmd;
	uint32_t *line = NULL;
	uint32_t data_len = (uint32_t)cmd->wNbBlocks
	    * (uint32_t)cmd->wBlockSize;
	uint32_t ram_addr = (uint32_t)cmd->pData;
	uint32_t ram_bound = ram_addr + data_len;
	uint32_t line_ix, line_cnt;
	uint8_t rc = SDMMC_OK;

#if 0
	trace_debug("Configuring DMA for a %luB transfer %s %p\n\r",
	    data_len, cmd->cmdOp.bmBits.xfrData == SDMMC_CMD_TX ? "from" : "to",
	    cmd->pData);
#endif
	/* Verify that cmd->pData is word-aligned */
	if ((uint32_t)cmd->pData & 0x3)
		return SDMMC_PARAM;
	/* Compute the size of the descriptor table for this transfer */
	line_cnt = (data_len - 1 + SDMMC_DMADL_TRAN_LEN_MAX)/ SDMMC_DMADL_TRAN_LEN_MAX;
	/* If it won't fit into the allocated buffer, resize the transfer */
	if (line_cnt > driver->config->dma_table_size) {
		line_cnt = driver->config->dma_table_size;
		data_len = line_cnt * SDMMC_DMADL_TRAN_LEN_MAX;
		data_len /= cmd->wBlockSize;
		if (data_len == 0)
			return SDMMC_NOT_SUPPORTED;
		cmd->wNbBlocks = (uint16_t)data_len;
		data_len *= cmd->wBlockSize;
		ram_bound = ram_addr + data_len;
		rc = SDMMC_CHANGED;
	}
	/* Fill the table */
	for (line_ix = 0, line = driver->config->dma_table; line_ix < line_cnt;
	    line_ix++, line += SDMMC_DMADL_SIZE) {
		if (line_ix + 1 < line_cnt) {
			line[0] = SDMMC_DMA0DL_LEN_MAX
			    | SDMMC_DMA0DL_ATTR_ACT_TRAN
			    | SDMMC_DMA0DL_ATTR_VALID;
			line[1] = SDMMC_DMA1DL_ADDR(ram_addr);
			ram_addr += SDMMC_DMADL_TRAN_LEN_MAX;
		}
		else {
			line[0] = ram_bound - ram_addr
			    < SDMMC_DMADL_TRAN_LEN_MAX
			    ? SDMMC_DMA0DL_LEN(ram_bound - ram_addr)
			    : SDMMC_DMA0DL_LEN_MAX;
			line[0] |= SDMMC_DMA0DL_ATTR_ACT_TRAN
			    | SDMMC_DMA0DL_ATTR_END | SDMMC_DMA0DL_ATTR_VALID;
			line[1] = SDMMC_DMA1DL_ADDR(ram_addr);
		}
#if 0
		trace_debug("DMA descriptor: %luB @ 0x%lx%c\n\r",
		    (line[0] & SDMMC_DMA0DL_LEN_Msk) >> SDMMC_DMA0DL_LEN_Pos,
		    line[1], line[0] & SDMMC_DMA0DL_ATTR_END ? '.' : ' ');
#endif
	}
	return rc;
}

static uint8_t unplug_device(SdmmcDriver *driver)
{
	//osalDbgCheck(set);

	Sdmmc *regs = driver->regs;
	uint32_t usec = 0;
	uint8_t mc1r;

	//trace_debug("Release and power the device off\n\r");
	if (driver->state == MCID_CMD)
		CancelCommand(driver);

	/* Hardware-reset the e.MMC, move it to the pre-idle state.
	 * Note that this will only be effective on systems where
	 * 1) the RST_n e.MMC input is wired to the SDMMCx_RSTN PIO, and
	 * 2) the hardware reset functionality of the device has been
	 *    enabled by software (!) Refer to ECSD register byte 162. */
	/* Generate a pulse on SDMMCx_RSTN. Satisfy tRSTW >= 1 usec.
	 * The timer driver can't cope with periodic interrupts triggered as
	 * frequently as one interrupt per microsecond. Extend to 10 usec. */
	mc1r = regs->SDMMC_MC1R;
	regs->SDMMC_MC1R = mc1r | SDMMC_MC1R_RSTN;
	t_usleep(driver,10);
	regs->SDMMC_MC1R = mc1r;
	/* Wait for either tRSCA = 200 usec or 74 device clock cycles, as per
	 * the e.MMC Electrical Standard. */
	if (driver->dev_freq != 0)
		usec = ROUND_INT_DIV(74 * 1000000UL, driver->dev_freq);
	usec = max_u32(usec, 200);
	t_usleep(driver,usec);

	/* Stop both the output clock and the SDMMC internal clock */
	regs->SDMMC_CCR &= ~(SDMMC_CCR_SDCLKEN | SDMMC_CCR_INTCLKEN);
	driver->dev_freq = 0;
	/* Cut the power rail supplying signals to/from the device */
	regs->SDMMC_PCR &= ~SDMMC_PCR_SDBPWR;
	/* Reset the peripheral. This will reset almost all registers. */
	reset_peripheral(driver);

	driver->state = MCID_OFF;
	return SDMMC_OK;
}


/**
 * \brief Switch to the specified timing mode
 * \note Since HC2R:VS18EN and HC2R:UHSMS fields depend on each other, this
 * function simultaneously updates the timing mode and the electrical state of
 * host I/Os.
 * \param set  Pointer to the driver instance data
 * \param mode  The new timing mode
 * \param verify  When switching from high to low signaling level, expect
 * the host input levels driven by the device to conform to the VOLTAGE_SWITCH
 * standard sequence.
 * \return A \ref sdmmc_rc result code
 */
static uint8_t sdmmc_set_speed_mode(SdmmcDriver *driver, uint8_t mode,bool verify)
{
	//osalDbgCheck(set);

	Sdmmc *regs = driver->regs;
	const uint32_t caps = regs->SDMMC_CA0R;
	/* Deviation from the SD Host Controller Specification: we use the
	 * Voltage Support capabilities to indicate the supported signaling
	 * levels (VCCQ), rather than the power supply voltage (VCC). */
	const bool perm_low_sig = (caps & (SDMMC_CA0R_V18VSUP
	    | SDMMC_CA0R_V30VSUP | SDMMC_CA0R_V33VSUP)) == SDMMC_CA0R_V18VSUP;
	uint32_t usec = 0;
	uint16_t hc2r_prv, hc2r;
	uint8_t rc = SDMMC_OK, hc1r_prv, hc1r, mc1r_prv, mc1r, pcr_prv, pcr;
	bool toggle_sig_lvl, low_sig, dev_clk_on;

	if ((mode > SDMMC_TIM_MMC_HS200 && mode < SDMMC_TIM_SD_DS)
	    || mode > SDMMC_TIM_SD_SDR104)
		return SDMMC_PARAM;
	if ((mode == SDMMC_TIM_MMC_HS200
	    || (mode >= SDMMC_TIM_SD_SDR12 && mode <= SDMMC_TIM_SD_SDR104))
	    && !(caps & SDMMC_CA0R_V18VSUP))
		return SDMMC_PARAM;

#if 0
	/* FIXME The datasheet is unclear about CCR:DIV restriction when the MMC
	 * timing mode is High Speed DDR */
	if ((mode == SDMMC_TIM_MMC_HS_SDR || mode == SDMMC_TIM_MMC_HS_DDR
	    || mode == SDMMC_TIM_SD_HS) && !(regs->SDMMC_CCR
	    & (SDMMC_CCR_USDCLKFSEL_Msk | SDMMC_CCR_SDCLKFSEL_Msk))) {
		TRACE_ERROR("Incompatible with the current clock config\n\r");
		return SDMMC_STATE;
	}
#endif

	driver->state = (driver->state == MCID_OFF) ? MCID_IDLE : driver->state;

	mc1r = mc1r_prv = regs->SDMMC_MC1R;
	hc1r = hc1r_prv = regs->SDMMC_HC1R;
	hc2r = hc2r_prv = regs->SDMMC_HC2R;
	pcr = pcr_prv = regs->SDMMC_PCR;
	mc1r = (mc1r & ~SDMMC_MC1R_DDR)
	    | (mode == SDMMC_TIM_MMC_HS_DDR ? SDMMC_MC1R_DDR : 0);
	hc1r = (hc1r & ~SDMMC_HC1R_HSEN) | (mode == SDMMC_TIM_MMC_HS_SDR
	    || mode == SDMMC_TIM_SD_HS ? SDMMC_HC1R_HSEN : 0);
	hc2r = hc2r & ~SDMMC_HC2R_DRVSEL_Msk & ~SDMMC_HC2R_VS18EN
	    & ~SDMMC_HC2R_UHSMS_Msk;
	if (mode == SDMMC_TIM_MMC_HS200
	    || (mode >= SDMMC_TIM_SD_SDR12 && mode <= SDMMC_TIM_SD_SDR104))
		hc2r |= SDMMC_HC2R_VS18EN;
	if (mode == SDMMC_TIM_MMC_HS200 || mode == SDMMC_TIM_SD_SDR104)
		hc2r |= SDMMC_HC2R_UHSMS_SDR104;
	else if (mode == SDMMC_TIM_SD_SDR12)
		hc2r |= SDMMC_HC2R_UHSMS_SDR12;
	else if (mode == SDMMC_TIM_SD_SDR25)
		hc2r |= SDMMC_HC2R_UHSMS_SDR25;
	else if (mode == SDMMC_TIM_SD_SDR50)
		hc2r |= SDMMC_HC2R_UHSMS_SDR50;
	else if (mode == SDMMC_TIM_SD_DDR50)
		hc2r |= SDMMC_HC2R_UHSMS_DDR50;
	/* Use the fixed clock when sampling data. Except if we keep using
	 * a 100+ MHz device clock. */
	if (driver->dev_freq <= 95000000ul || (mode != SDMMC_TIM_MMC_HS200
	    && mode != SDMMC_TIM_SD_SDR104 && (mode != SDMMC_TIM_SD_SDR50
	    || !(regs->SDMMC_CA1R & SDMMC_CA1R_TSDR50))))
		hc2r &= ~SDMMC_HC2R_SCLKSEL;
	/* On SAMA5D2-XULT when using 1.8V signaling, on host outputs choose
	 * Driver Type C, i.e. 66 ohm nominal output impedance.
	 * FIXME rely on platform code to retrieve the optimal host output
	 * Driver Type. It depends on board design. An oscilloscope should be
	 * set up to observe signal integrity, then among the driver types that
	 * meet rise and fall time requirements, the weakest should be selected.
	 */
	if (hc2r & SDMMC_HC2R_VS18EN)
		hc2r |= SDMMC_HC2R_DRVSEL_TYPEC;
	pcr = (pcr & ~SDMMC_PCR_SDBVSEL_Msk) | SDMMC_PCR_SDBPWR;
	low_sig = perm_low_sig || hc2r & SDMMC_HC2R_VS18EN;
	if (low_sig)
		pcr |= SDMMC_PCR_SDBVSEL_18V;
	else
		pcr |= caps & SDMMC_CA0R_V30VSUP ? SDMMC_PCR_SDBVSEL_30V
		    : SDMMC_PCR_SDBVSEL_33V;

	if (hc2r == hc2r_prv && hc1r == hc1r_prv && mc1r == mc1r_prv
	    && pcr == pcr_prv)
		goto End;
	toggle_sig_lvl = pcr_prv & SDMMC_PCR_SDBPWR
	    && (pcr ^ pcr_prv) & SDMMC_PCR_SDBVSEL_Msk;
	if (!(pcr_prv & SDMMC_PCR_SDBPWR)) {
		TRACE_DEBUG("Power the device on\n\r");
	}
	else if (toggle_sig_lvl) {
		TRACE_DEBUG_1("Signaling level going %s\n\r",
		    hc2r & SDMMC_HC2R_VS18EN ? "low" : "high");
	}
	if (verify && toggle_sig_lvl && hc2r & SDMMC_HC2R_VS18EN) {
		/* Expect this call to follow the VOLTAGE_SWITCH command;
		 * allow 2 device clock periods before the device pulls the CMD
		 * and DAT[3:0] lines down */
		if (driver->dev_freq != 0)
			usec = ROUND_INT_DIV(2 * 1000000UL, driver->dev_freq);
		usec = max_u32(usec, 10);
		t_usleep(driver,usec);
		if (regs->SDMMC_PSR & (SDMMC_PSR_CMDLL | SDMMC_PSR_DATLL_Msk))
			rc = SDMMC_STATE;
	}
	/* Avoid generating glitches on the device clock */
	dev_clk_on = regs->SDMMC_CCR & SDMMC_CCR_SDCLKEN
	    && (toggle_sig_lvl || hc2r_prv & SDMMC_HC2R_PVALEN
	    || hc2r != hc2r_prv);
	if (dev_clk_on)
		regs->SDMMC_CCR &= ~SDMMC_CCR_SDCLKEN;
	if (toggle_sig_lvl)
		/* Drive the device clock low, turn CMD and DATx high-Z */
		regs->SDMMC_PCR = pcr & ~SDMMC_PCR_SDBPWR;

	/* Now change the timing mode */
	if (mc1r != mc1r_prv)
		regs->SDMMC_MC1R = mc1r;
	if (hc1r != hc1r_prv)
		regs->SDMMC_HC1R = hc1r;
	if (hc2r != hc2r_prv)
		regs->SDMMC_HC2R = hc2r;
	if (toggle_sig_lvl) {
		/* Changing the signaling level. The SD Host Controller
		 * Specification requires the HW to stabilize the electrical
		 * levels within 5 ms, which equals 5 system ticks.
		 * Alternative: wait for tPRUL = 25 ms */
		t_msleep(driver,5);
		if (hc2r & SDMMC_HC2R_VS18EN
		    && !(regs->SDMMC_HC2R & SDMMC_HC2R_VS18EN))
			rc = SDMMC_ERR;
	}
	if (pcr != pcr_prv)
		regs->SDMMC_PCR = pcr;
	if (verify && toggle_sig_lvl && hc2r & SDMMC_HC2R_VS18EN) {
		t_msleep(driver,1);
		if (regs->SDMMC_PSR & (SDMMC_PSR_CMDLL | SDMMC_PSR_DATLL_Msk))
			rc = SDMMC_STATE;
	}
	if (dev_clk_on || (toggle_sig_lvl && hc2r & SDMMC_HC2R_VS18EN))
		/* FIXME verify that current dev clock freq is 400 kHz */
		regs->SDMMC_CCR |= SDMMC_CCR_SDCLKEN;
	if (toggle_sig_lvl && hc2r & SDMMC_HC2R_VS18EN) {
		/* Expect the device to release the CMD and DAT[3:0] lines
		 * within 1 ms */
		t_msleep(driver,1);
		if ((regs->SDMMC_PSR & (SDMMC_PSR_CMDLL | SDMMC_PSR_DATLL_Msk))
		    != (SDMMC_PSR_CMDLL | SDMMC_PSR_DATLL_Msk) && verify)
			rc = SDMMC_STATE;
		if (!dev_clk_on)
			regs->SDMMC_CCR &= ~SDMMC_CCR_SDCLKEN;
	}
	TRACE_DEBUG_1("Using timing mode 0x%02x\n\r", mode);

	regs->SDMMC_CALCR = (regs->SDMMC_CALCR & ~SDMMC_CALCR_ALWYSON)
	    | (low_sig ? SDMMC_CALCR_ALWYSON : 0);
	if (low_sig || pcr != pcr_prv)
		/* Perform the output calibration sequence */
		calibrate_zout(driver->regs);
		/* TODO in SDR12-50/DDR50 mode, schedule periodic re-calibration */

End:
	if (rc == SDMMC_OK)
		driver->tim_mode = mode;
	return rc;
}




uint8_t sdmmc_get_bus_width(SdmmcDriver *driver)
{
	//osalDbgCheck(set);

	const uint8_t hc1r = driver->regs->SDMMC_HC1R;

	if (hc1r & SDMMC_HC1R_EXTDW)
		return 8;
	else if (hc1r & SDMMC_HC1R_DW)
		return 4;
	else
		return 1;
}

static uint8_t sdmmc_set_bus_width(SdmmcDriver *driver, uint8_t bits)
{
	//osalDbgCheck(set);

	Sdmmc *regs = driver->regs;
	uint8_t hc1r_prv, hc1r;

	if (bits != 1 && bits != 4 && bits != 8)
		return SDMMC_PARAM;
	if (bits == 8 && !(regs->SDMMC_CA0R & SDMMC_CA0R_ED8SUP)) {
		//trace_error("This slot doesn't support an 8-bit data bus\n\r");
		return SDMMC_PARAM;
	}
	/* TODO in case of SD slots, rely on platform code to get the width of
	 * the data bus actually implemented on the board. In the meantime we
	 * assume DAT[3:0] are all effectively connected to the device. */

	hc1r = hc1r_prv = regs->SDMMC_HC1R;
	if (bits == 8 && hc1r & SDMMC_HC1R_EXTDW)
		return SDMMC_OK;
	else if (bits == 8)
		hc1r |= SDMMC_HC1R_EXTDW;
	else {
		hc1r &= ~SDMMC_HC1R_EXTDW;
		if (bits == 4)
			hc1r |= SDMMC_HC1R_DW;
		else
			hc1r &= ~SDMMC_HC1R_DW;
		if (hc1r == hc1r_prv)
			return SDMMC_OK;
	}
	regs->SDMMC_HC1R = hc1r;
	return SDMMC_OK;
}


 static uint8_t HwReset(SdmmcDriver *driver)
 {
 	uint32_t rc;

 	driver->control_param = 0;

 	rc = sdmmc_device_control(driver, SDMMC_IOCTL_RESET);

 	return rc;
 }

 uint8_t HwPowerDevice(SdmmcDriver *drv, uint8_t nowSwitchOn)
 {
 	uint32_t rc;

 	drv->control_param = nowSwitchOn;

 	rc = sdmmc_device_control(drv, SDMMC_IOCTL_POWER);

 	return rc;
 }

 uint8_t HwSetHsMode(SdmmcDriver *drv, uint8_t timingMode)
 {
 	uint32_t rc;

 	drv->control_param = timingMode;

 	rc = sdmmc_device_control(drv, SDMMC_IOCTL_SET_HSMODE);

 	if ((rc == SDMMC_OK || rc == SDMMC_CHANGED)
 	    && (drv->control_param > 0xff || drv->control_param != (uint32_t)timingMode))
 		rc = SDMMC_CHANGED;
 	return rc;
 }

  uint32_t HwSetBusWidth(	SdmmcDriver *drv,uint8_t newWidth)
 {
 	uint32_t rc;

 	drv->control_param = newWidth;

 	rc = sdmmc_device_control(drv, SDMMC_IOCTL_SET_BUSMODE);

 	return rc;
 }

  bool HwIsTimingSupported(SdmmcDriver *drv, uint8_t timingMode)
  {
  	uint32_t rc;

  	drv->control_param = timingMode;

  	rc = sdmmc_device_control(drv, SDMMC_IOCTL_GET_HSMODE);

  	return rc == SDMMC_OK ? (drv->control_param ? true : false) : false;
  }

  uint8_t HwSetClock(SdmmcDriver *drv, uint32_t * pIoValClk)
 {
 	uint32_t rc;

 	drv->control_param = *pIoValClk;

 	rc = sdmmc_device_control(drv, SDMMC_IOCTL_SET_CLOCK);

 	if (rc == SDMMC_OK || rc == SDMMC_CHANGED) {

 		*pIoValClk = drv->control_param;

 		TRACE_DEBUG_1("Device clk %lu kHz\n\r", drv->control_param / 1000UL);
 	}
 	return rc;
 }



#endif
