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
#include "hal.h"

#if (HAL_USE_CRY == TRUE) || defined(__DOXYGEN__)

#include "sama_crypto_lld.h"
#include "sama_tdes_lld.h"

enum tdesd_key_mode_t {
	TDES_KEY_THREE = 0, TDES_KEY_TWO
};

void tdes_set_input(uint32_t* data0, uint32_t* data1) {
	TDES->TDES_IDATAR[0] = *data0;
	if (data1)
		TDES->TDES_IDATAR[1] = *data1;
}

void tdes_get_output(uint32_t *data0, uint32_t *data1) {
	*data0 = TDES->TDES_ODATAR[0];
	if (data1)
		*data1 = TDES->TDES_ODATAR[1];
}

cryerror_t sama_tdes_lld_polling(CRYDriver *cryp, tdes_config_t *params,
		bool encrypt, const uint8_t *data, size_t data_len, uint8_t * out,
		const uint8_t *iv) {

	uint32_t mode = 0;
	uint32_t i;
	uint8_t size = 8;
	uint32_t *vectors = (uint32_t *) iv;

	osalMutexLock(&cryp->mutex);

	//soft reset
	TDES->TDES_CR = TDES_CR_SWRST;
	//configure
	if (encrypt)
		mode |= TDES_MR_CIPHER_ENCRYPT;
	else
		mode |= TDES_MR_CIPHER_DECRYPT;

	if (cryp->key0_size == 16)
		mode |= (TDES_KEY_TWO << 4);
	else
		mode |= (TDES_KEY_THREE << 4);

	mode |= TDES_MR_TDESMOD(params->algo);

	mode |= TDES_MR_SMOD_MANUAL_START;

	mode |= TDES_MR_OPMOD(params->mode);

	if (cryp->config != NULL) {
		mode |= TDES_MR_CFBS(cryp->config->cfbs);

		if (params->mode == TDES_MODE_CFB) {
			if (cryp->config->cfbs == TDES_CFBS_32)
				size = 4;
			else if (cryp->config->cfbs == TDES_CFBS_16)
				size = 2;
			else if (cryp->config->cfbs == TDES_CFBS_8)
				size = 1;
		}

	}

	TDES->TDES_MR = mode;

	//write keys

	TDES->TDES_KEY1WR[0] = cryp->key0_buffer[0];
	TDES->TDES_KEY1WR[1] = cryp->key0_buffer[1];

	if (cryp->key0_size > 8) {
		TDES->TDES_KEY2WR[0] = cryp->key0_buffer[2];
		TDES->TDES_KEY2WR[1] = cryp->key0_buffer[3];
	} else {
		TDES->TDES_KEY2WR[0] = 0x0;
		TDES->TDES_KEY2WR[1] = 0x0;
	}
	if (cryp->key0_size > 16) {
		TDES->TDES_KEY3WR[0] = cryp->key0_buffer[4];
		TDES->TDES_KEY3WR[1] = cryp->key0_buffer[5];
	} else {
		TDES->TDES_KEY3WR[0] = 0x0;
		TDES->TDES_KEY3WR[1] = 0x0;
	}
	/* The Initialization Vector Registers apply to all modes except ECB. */
	if (params->mode != TDES_MODE_ECB && vectors != NULL) {
		TDES->TDES_IVR[0] = vectors[0];
		TDES->TDES_IVR[1] = vectors[1];
	}
	if (params->algo == TDES_ALGO_XTEA) {
		TDES->TDES_XTEA_RNDR = TDES_XTEA_RNDR_XTEA_RNDS(32);
	}

	//load 64 bit data size in tdes registers
	for (i = 0; i < data_len; i += size) {
		if (size == 8)
			tdes_set_input((uint32_t *) ((data) + i),
					(uint32_t *) ((data) + i + 4));
		else
			tdes_set_input((uint32_t *) ((data) + i), NULL);

		//Start TDES
		TDES->TDES_CR = TDES_CR_START;

		//check status
		while ((TDES->TDES_ISR & TDES_ISR_DATRDY) != TDES_ISR_DATRDY)
			;

		if (size == 8)
			tdes_get_output((uint32_t *) ((out) + i),
					(uint32_t *) ((out) + i + 4));
		else
			tdes_get_output((uint32_t *) ((out) + i), NULL);
	}

	osalMutexUnlock(&cryp->mutex);


	return CRY_NOERROR;
}

cryerror_t sama_tdes_lld_dma(CRYDriver *cryp, tdes_config_t *params,
		bool encrypt, const uint8_t *data, size_t data_len, uint8_t * out,
		const uint8_t *iv) {

  osalDbgAssert(!((uint32_t) data & (L1_CACHE_BYTES - 1)), "data address not cache aligned");
  osalDbgAssert(!((uint32_t) out & (L1_CACHE_BYTES - 1)), "out address not cache aligned");

#if 0
  osalDbgAssert(!(data_len & (L1_CACHE_BYTES - 1)), "size not multiple of cache line");
#endif

  /*
   * If size is not multiple of cache line, clean cache region is required.
   * TODO: remove when size assert works
   */
  if (data_len & (L1_CACHE_BYTES - 1)) {
    cacheCleanRegion((uint8_t *) out, data_len);
  }

	uint32_t mode = 0;
	uint32_t *vectors = (uint32_t *) iv;

	osalMutexLock(&cryp->mutex);

  cacheCleanRegion((uint8_t *) data, data_len);

  cryp->out = out;
  cryp->in = data;
  cryp->len = data_len;

	cryp->dmachunksize = DMA_CHUNK_SIZE_1;

	cryp->dmawith = DMA_DATA_WIDTH_WORD;

	if ((params->mode == TDES_MODE_CFB)) {
		if (cryp->config->cfbs == TDES_CFBS_16)
			cryp->dmawith = DMA_DATA_WIDTH_HALF_WORD;
		if (cryp->config->cfbs == TDES_CFBS_8)
			cryp->dmawith = DMA_DATA_WIDTH_BYTE;
	}

	cryp->rxdmamode = XDMAC_CC_TYPE_PER_TRAN |
	XDMAC_CC_PROT_SEC |
	XDMAC_CC_MBSIZE_SINGLE |
	XDMAC_CC_DSYNC_PER2MEM | XDMAC_CC_CSIZE(cryp->dmachunksize) |
	XDMAC_CC_DWIDTH(cryp->dmawith) |
	XDMAC_CC_SIF_AHB_IF1 |
	XDMAC_CC_DIF_AHB_IF0 |
	XDMAC_CC_SAM_FIXED_AM |
	XDMAC_CC_DAM_INCREMENTED_AM |
	XDMAC_CC_PERID(PERID_TDES_RX);

	cryp->txdmamode = XDMAC_CC_TYPE_PER_TRAN |
	XDMAC_CC_PROT_SEC |
	XDMAC_CC_MBSIZE_SINGLE |
	XDMAC_CC_DSYNC_MEM2PER | XDMAC_CC_CSIZE(cryp->dmachunksize) |
	XDMAC_CC_DWIDTH(cryp->dmawith) |
	XDMAC_CC_SIF_AHB_IF0 |
	XDMAC_CC_DIF_AHB_IF1 |
	XDMAC_CC_SAM_INCREMENTED_AM |
	XDMAC_CC_DAM_FIXED_AM |
	XDMAC_CC_PERID(PERID_TDES_TX);

	dmaChannelSetMode(cryp->dmarx, cryp->rxdmamode);
	dmaChannelSetMode(cryp->dmatx, cryp->txdmamode);

	// Writing channel
	dmaChannelSetSource(cryp->dmatx, data);
	dmaChannelSetDestination(cryp->dmatx, TDES->TDES_IDATAR);
	dmaChannelSetTransactionSize(cryp->dmatx,
			(data_len / DMA_DATA_WIDTH_TO_BYTE(cryp->dmawith)));


	// Reading channel
	dmaChannelSetSource(cryp->dmarx, TDES->TDES_ODATAR);
	dmaChannelSetDestination(cryp->dmarx, out);
	dmaChannelSetTransactionSize(cryp->dmarx,
			(data_len / DMA_DATA_WIDTH_TO_BYTE(cryp->dmawith)));

	//soft reset
	TDES->TDES_CR = TDES_CR_SWRST;

	//configure
	if (encrypt)
		mode |= TDES_MR_CIPHER_ENCRYPT;
	else
		mode |= TDES_MR_CIPHER_DECRYPT;

	if (cryp->key0_size == 16)
		mode |= (TDES_KEY_TWO << 4);
	else
		mode |= (TDES_KEY_THREE << 4);

	mode |= TDES_MR_TDESMOD(params->algo);

	mode |= TDES_MR_SMOD_IDATAR0_START;

	mode |= TDES_MR_OPMOD(params->mode);

	if (cryp->config != NULL) {
		mode |= TDES_MR_CFBS(cryp->config->cfbs);
	}

	TDES->TDES_MR = mode;



	//write keys
	TDES->TDES_KEY1WR[0] = cryp->key0_buffer[0];
	TDES->TDES_KEY1WR[1] = cryp->key0_buffer[1];

	if (cryp->key0_size > 8) {
		TDES->TDES_KEY2WR[0] = cryp->key0_buffer[2];
		TDES->TDES_KEY2WR[1] = cryp->key0_buffer[3];
	} else {
		TDES->TDES_KEY2WR[0] = 0x0;
		TDES->TDES_KEY2WR[1] = 0x0;
	}
	if (cryp->key0_size > 16) {
		TDES->TDES_KEY3WR[0] = cryp->key0_buffer[4];
		TDES->TDES_KEY3WR[1] = cryp->key0_buffer[5];
	} else {
		TDES->TDES_KEY3WR[0] = 0x0;
		TDES->TDES_KEY3WR[1] = 0x0;
	}
	//initialize vectors registers ( except ECB mode)
	if (params->mode != TDES_MODE_ECB && vectors != NULL) {
		TDES->TDES_IVR[0] = vectors[0];
		TDES->TDES_IVR[1] = vectors[1];
	}



	if (params->algo == TDES_ALGO_XTEA) {
		TDES->TDES_XTEA_RNDR = TDES_XTEA_RNDR_XTEA_RNDS(32);
	}

//enable interrutps
	TDES->TDES_IER = TDES_IER_DATRDY;

	osalSysLock();

	dmaChannelEnable(cryp->dmarx);
	dmaChannelEnable(cryp->dmatx);

	osalThreadSuspendS(&cryp->thread);
	osalSysUnlock();

	osalMutexUnlock(&cryp->mutex);

	return CRY_NOERROR;
}

#endif /* HAL_USE_CRY */

/** @} */

