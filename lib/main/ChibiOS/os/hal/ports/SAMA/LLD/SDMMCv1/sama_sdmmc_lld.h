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
 * @file    sama_sdmmc_lld.h
 * @brief   PLATFORM SDMMC subsystem low level driver header.
 *
 * @addtogroup SDMMC
 * @{
 */

#ifndef SAMA_SDMMC_LLD_H
#define SAMA_SDMMC_LLD_H

#if (SAMA_USE_SDMMC == TRUE) || defined(__DOXYGEN__)

#include "ch_sdmmc.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    PLATFORM configuration options
 * @{
 */
/**
 * @brief   PLATFORM_SDMMC_USE_SDMMC1 driver enable switch.
 * @details If set to @p TRUE the support for PLATFORM_SDMMC_USE_SDMMC1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(PLATFORM_SDMMC_USE_SDMMC1) || defined(__DOXYGEN__)
#define PLATFORM_SDMMC_USE_SDMMC1                  FALSE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/


typedef enum {
	MCID_OFF, /**< Device not powered */
	MCID_IDLE, /**< Idle */
	MCID_LOCKED, /**< Locked for specific slot */
	MCID_CMD, /**< Processing the command */
	MCID_ERROR, /**< Command error */
	MCID_INIT_ERROR
}sdmmcstate_t;



typedef struct {

	sdmmcslots_t slot_id;

	uint8_t * bp;

	uint32_t * dma_table;
	uint32_t   dma_table_size;

} SamaSDMMCConfig;


struct SamaSDMMCDriver
{
	volatile sdmmcstate_t        		state;
	const SamaSDMMCConfig            *config;

	Sdmmc * regs;                 /* set of SDMMC hardware registers */

	uint32_t tctimer_id;               /* Timer/Counter peripheral ID (ID_TCx) */


	uint32_t dev_freq;            /* frequency of clock provided to memory
				       	   	   	   * device, in Hz */

	sSdmmcCommand  cmd;
	sSdCard  card;


	uint16_t blk_index;           /* count of data blocks tranferred already,
				       * in the context of the command and data
				       * transfer being executed */

	uint8_t resp_len;             /* size of the response, once retrieved,
				       * in the context of the command being
				       * executed, expressed in 32-bit words */
	uint8_t tim_mode;             /* timing mode aka bus speed mode */
	uint16_t blk_size;            /* max data block size, in bytes */
	bool use_polling;             /* polling mode */
	bool cmd_line_released;       /* handled the Command Complete event */
	bool dat_lines_released;      /* handled the Transfer Complete event */
	bool expect_auto_end;         /* waiting for completion of Auto CMD12 */
	bool use_set_blk_cnt;         /* implicit SET_BLOCK_COUNT command */

	uint32_t control_param;
	uint32_t timeout_ticks;
	int8_t  timeout_elapsed;
	systime_t time,now;

	rtcnt_t timeout_cycles;
	rtcnt_t start_cycles;


};

typedef struct SamaSDMMCDriver SdmmcDriver;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (PLATFORM_SDMMC_USE_SDMMC1 == TRUE)
extern SdmmcDriver SDMMCD1;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void sdmmcInit(void);
void sdmmcObjectInit(SdmmcDriver *sdmmcp);
void sdmmcStart(SdmmcDriver *sdmmcp, const SamaSDMMCConfig *config);
void sdmmcStop(SdmmcDriver *sdmmcp);
uint8_t sdmmcSendCmd(SdmmcDriver *sdmmcp);
bool sdmmcOpenDevice(SdmmcDriver *sdmmcp);
bool sdmmcCloseDevice(SdmmcDriver *sdmmcp);
bool sdmmcShowDeviceInfo(SdmmcDriver *sdmmcp);
bool sdmmcGetInstance(uint8_t index, SdmmcDriver **sdmmcp)  ;

#ifdef __cplusplus
}
#endif

#endif /* SAMA_USE_SDMMC == TRUE */

#endif /* SAMA_SDMMC_LLD_H */

/** @} */
