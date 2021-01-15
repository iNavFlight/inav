/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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

/**
 * @file    sama_sdmmc_lld.c
 * @brief   PLATFORM SDMMC driver
 *
 * @addtogroup SDMMC
 * @{
 */

#include "hal.h"
#include "ccportab.h"

#if (SAMA_USE_SDMMC == TRUE) || defined(__DOXYGEN__)
#include <string.h>
#include "sama_sdmmc_lld.h"
#include "ch_sdmmc_device.h"
#include "ch_sdmmc_sd.h"
#include "ch_sdmmc_sdio.h"
#include "ch_sdmmc_trace.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   SDMMC1 driver identifier.
 */
#if (PLATFORM_SDMMC_USE_SDMMC1 == TRUE) || defined(__DOXYGEN__)
SdmmcDriver SDMMCD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/


/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/
#if (PLATFORM_SDMMC_USE_SDMMC1 == TRUE)
OSAL_IRQ_HANDLER(SAMA_SDMMCD1_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  osalSysLockFromISR();
  sdmmc_device_poll(&SDMMCD1);
  osalSysUnlockFromISR();
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}
#endif
/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/


/**
 * @brief   Low level SDMMC driver initialization.
 *
 * @notapi
 */
void sdmmcInit(void)
{
#if PLATFORM_SDMMC_USE_SDMMC1 == TRUE
  /* Driver initialization.*/
	sdmmcObjectInit(&SDMMCD1);
#endif
}


/**
 * @brief   Configures and activates the SDMMC peripheral.
 *
 * @param[in] sdmmcp      pointer to the @p SdmmcDriver object
 *
 * @notapi
 */
void sdmmcStart(SdmmcDriver *sdmmcp, const SamaSDMMCConfig *config)
{

	uint8_t rc;

	sdmmcp->config = config;

#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX0, (ID_SDMMC0 + sdmmcp->config->slot_id) , SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */

	sdmmcp->card.EXT = sdmmcp->config->bp;
	sdmmcp->card.SSR = &sdmmcp->config->bp[EXT_SIZE];
	sdmmcp->card.SCR = &sdmmcp->config->bp[EXT_SIZE + SSR_SIZE];
	sdmmcp->card.sandbox1 =&sdmmcp->config->bp[EXT_SIZE + SSR_SIZE + SCR_SIZE];
	sdmmcp->card.sandbox2 = &sdmmcp->config->bp[EXT_SIZE + SSR_SIZE + SCR_SIZE + SB1_SIZE];


	rc = sdmmc_device_lowlevelcfg(sdmmcp);


	if (rc) {

		//initialize
		sdmmc_device_initialize(sdmmcp);

		if (!sdmmcp->use_polling) {
#if (PLATFORM_SDMMC_USE_SDMMC1 == TRUE)
			aicSetSourcePriority( (ID_SDMMC0 + sdmmcp->config->slot_id) ,SAMA_SDMMC_SDMMCDRIVER_IRQ_PRIORITY);
			aicSetSourceHandler(ID_SDMMC0 + sdmmcp->config->slot_id,SAMA_SDMMCD1_HANDLER);
			aicEnableInt( (ID_SDMMC0 + sdmmcp->config->slot_id) );
#endif
		}
		return;

	}

	TRACE_ERROR("Cannot init board for MMC\r\n");
	sdmmcp->state = MCID_INIT_ERROR;

}

/**
 * @brief   Deactivates the SDMMC peripheral.
 *
 * @param[in] sdmmcp      pointer to the @p SdmmcDriver object
 *
 * @notapi
 */

void sdmmcStop(SdmmcDriver *sdmmcp)
{


	  if (sdmmcp->state != MCID_OFF) {
	    /* Resets the peripheral.*/

	    /* Disables the peripheral.*/
		  aicDisableInt( (ID_SDMMC0 + sdmmcp->config->slot_id) );

		  	if (sdmmcp->config->slot_id == SDMMC_SLOT0) {
		  		pmcDisableSDMMC0();
		  	} else if (sdmmcp->config->slot_id == SDMMC_SLOT1) {
		  		pmcDisableSDMMC1();
		  	}
	  }
}

/**
 * @brief   sends a command .
 *
 * @param[in] sdmmcp      pointer to the @p SdmmcDriver object
 *
 * @notapi
 */
uint8_t sdmmcSendCmd(SdmmcDriver *sdmmcp)
{

		uint32_t err;

		uint8_t bRc;

		if (sdmmcp->cmd.bCmd != 55) {
			TRACE_INFO_2("Cmd%u(%lx)\n\r", sdmmcp->cmd.bCmd, sdmmcp->cmd.dwArg);
		}

		bRc = sdmmc_device_command(sdmmcp);


	{
			/* Poll command status.
			 * The driver is responsible for detecting and reporting
			 * timeout conditions. Here we only start a backup timer, in
			 * case the driver or the peripheral meets an unexpected
			 * condition. Mind that defining how long a command such as
			 * WRITE_MULTIPLE_BLOCK could take in total may only lead to an
			 * experimental value, lesser than the unrealistic theoretical
			 * maximum.
			 * Abort the command if the driver is still busy after 30s,
			 * which equals 30*1000 system ticks. */


			 sdmmcp->control_param = 1;
			 sdmmcp->timeout_elapsed = 0;
			 sdmmcp->timeout_ticks = 30*1000;
			 sdmmc_device_startTimeCount(sdmmcp);

			 do
			{
				err = sdmmc_device_control(sdmmcp, SDMMC_IOCTL_BUSY_CHECK);
				sdmmc_device_checkTimeCount(sdmmcp);
			}
			 while (sdmmcp->control_param && err == SDMMC_OK && !sdmmcp->timeout_elapsed);

			if (err != SDMMC_OK) {
				sdmmcp->cmd.bStatus = (uint8_t)err;
			}
			else if (sdmmcp->control_param) {
				sdmmcp->control_param = 0;

				sdmmc_device_control(sdmmcp, SDMMC_IOCTL_CANCEL_CMD);

				sdmmcp->cmd.bStatus = SDMMC_NO_RESPONSE;
			}

	}

	bRc = sdmmcp->cmd.bStatus;

		if (bRc == SDMMC_CHANGED)  {
			TRACE_DEBUG_2("Changed Cmd%u %s\n\r", sdmmcp->cmd.bCmd,SD_StringifyRetCode(bRc));
		}
		else if (bRc != SDMMC_OK) {
			TRACE_DEBUG_2("OK Cmd%u %s\n\r", sdmmcp->cmd.bCmd,SD_StringifyRetCode(bRc));
		}
		else if (sdmmcp->cmd.cmdOp.bmBits.respType == 1 && sdmmcp->cmd.pResp) {
			TRACE_DEBUG_2("Resp Cmd%u st %lx\n\r", sdmmcp->cmd.bCmd, *sdmmcp->cmd.pResp);
		}

		return bRc;
}


void sdmmcObjectInit(SdmmcDriver *sdmmcp)
{
	sdmmcp->state  = MCID_OFF;
	sdmmcp->timeout_elapsed = -1;
	sdmmcp->config = NULL;
}


bool sdmmcOpenDevice(SdmmcDriver *sdmmcp)
{
	uint8_t rc;

		rc = sdmmc_device_start(sdmmcp);

		if (rc != SDMMC_OK) {
			TRACE_INFO_1("SD/MMC device initialization failed: %d\n\r", rc);
			return false;
		}

		if (sdmmc_device_identify(sdmmcp) != SDMMC_OK) {
			return false;
		}
		TRACE_INFO("SD/MMC device initialization successful\n\r");
		return true;
}

bool sdmmcCloseDevice(SdmmcDriver *sdmmcp)
{
	sdmmc_device_deInit(sdmmcp);
	return true;
}

bool sdmmcShowDeviceInfo(SdmmcDriver *sdmmcp)
{
	sSdCard *pSd =&sdmmcp->card;
		TRACE_INFO("Show Device Info:\n\r");

	#ifndef SDMMC_TRIM_INFO
		const uint8_t card_type = sdmmcp->card.bCardType;
		TRACE_INFO_1("Card Type: %d\n\r", card_type);
	#endif
		TRACE_INFO("Dumping Status ... \n\r");
		SD_DumpStatus(pSd);
	#ifndef SDMMC_TRIM_INFO
		if (card_type & CARD_TYPE_bmSDMMC)
			SD_DumpCID(pSd);
		if (card_type & CARD_TYPE_bmSD) {
			SD_DumpSCR(pSd->SCR);
			SD_DumpSSR(pSd->SSR);
		}
		if (card_type & CARD_TYPE_bmSDMMC)
			SD_DumpCSD(pSd);
	#ifndef SDMMC_TRIM_MMC
		if (card_type & CARD_TYPE_bmMMC)
			SD_DumpExtCSD(pSd->EXT);
	#endif
	#ifndef SDMMC_TRIM_SDIO
		if (card_type & CARD_TYPE_bmSDIO)
			SDIO_DumpCardInformation(sdmmcp);
	#endif

	#endif
		return true;
}


bool CC_WEAK sdmmcGetInstance(uint8_t index, SdmmcDriver **sdmmcp)
{
	(void)index;
	(void)sdmmcp;
	return false;
}

#endif /* SAMA_USE_SDMMC == TRUE */

/** @} */
