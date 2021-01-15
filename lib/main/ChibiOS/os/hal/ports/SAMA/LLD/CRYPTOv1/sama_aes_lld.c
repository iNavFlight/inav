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

void sama_aes_lld_write_key(const uint32_t * key, const uint32_t * vectors,
		uint32_t len) {

	AES->AES_KEYWR[0] = key[0];
	AES->AES_KEYWR[1] = key[1];
	AES->AES_KEYWR[2] = key[2];
	AES->AES_KEYWR[3] = key[3];

	if (len >= 24) {
		AES->AES_KEYWR[4] = key[4];
		AES->AES_KEYWR[5] = key[5];
	}

	else {
		AES->AES_KEYWR[4] = 0;
		AES->AES_KEYWR[5] = 0;
		AES->AES_KEYWR[6] = 0;
		AES->AES_KEYWR[7] = 0;
	}

	if (len == 32) {
		AES->AES_KEYWR[6] = key[6];
		AES->AES_KEYWR[7] = key[7];
	}

	if (vectors != NULL) {
		AES->AES_IVR[0] = vectors[0];
		AES->AES_IVR[1] = vectors[1];
		AES->AES_IVR[2] = vectors[2];
		AES->AES_IVR[3] = vectors[3];
	}

	else {
		AES->AES_IVR[0] = 0;
		AES->AES_IVR[1] = 0;
		AES->AES_IVR[2] = 0;
		AES->AES_IVR[3] = 0;
	}

}

cryerror_t sama_aes_lld_set_key_size(size_t size) {

	uint32_t key_size = AES_MR_KEYSIZE_AES128;

	if (size == 16)
		key_size = AES_MR_KEYSIZE_AES128;
	else if (size == 24)
		key_size = AES_MR_KEYSIZE_AES192;
	else if (size == 32)
		key_size = AES_MR_KEYSIZE_AES256;
	else
		return CRY_ERR_INV_KEY_SIZE;
	//set key size
	AES->AES_MR |= (( AES_MR_KEYSIZE_Msk & (key_size)) | AES_MR_CKEY_PASSWD);

	return CRY_NOERROR;
}

void sama_aes_lld_set_input(uint32_t* data) {
	uint8_t i;
	uint8_t size = 4;

	if ((AES->AES_MR & AES_MR_OPMOD_Msk) == AES_MR_OPMOD_CFB) {
		if ((AES->AES_MR & AES_MR_CFBS_Msk) == AES_MR_CFBS_SIZE_128BIT)
			size = 4;
		else if ((AES->AES_MR & AES_MR_CFBS_Msk) == AES_MR_CFBS_SIZE_64BIT)
			size = 2;
		else
			size = 1;
	}

	for (i = 0; i < size; i++) {
		AES->AES_IDATAR[i] = data[i];
	}

}

void sama_aes_lld_get_output(uint32_t* data) {
	uint8_t i;

	for (i = 0; i < 4; i++) {
		data[i] = (AES->AES_ODATAR[i]);
	}
}

cryerror_t sama_aes_lld_process_polling(CRYDriver *cryp, aesparams *params,
		const uint8_t *in, uint8_t *out, size_t indata_len) {
	uint32_t i;
	cryerror_t ret;

	osalMutexLock(&cryp->mutex);
	//AES soft reset
	AES->AES_CR = AES_CR_SWRST;

	//AES set op mode
	AES->AES_MR |= ((AES_MR_OPMOD_Msk & (params->mode)) | AES_MR_CKEY_PASSWD);

	//AES set key size
	ret = sama_aes_lld_set_key_size(cryp->key0_size);

	if (ret == CRY_NOERROR) {

		AES->AES_MR |= (AES_MR_CFBS(cryp->config->cfbs) | AES_MR_CKEY_PASSWD);



		sama_aes_lld_write_key(cryp->key0_buffer,( const uint32_t *) params->iv, cryp->key0_size);



		if (params->encrypt)
			AES->AES_MR |= AES_MR_CIPHER;
		else
			AES->AES_MR &= ~AES_MR_CIPHER;

		AES->AES_MR |= (((AES_MR_SMOD_Msk & (AES_MR_SMOD_MANUAL_START))) | AES_MR_CKEY_PASSWD);

		//Enable aes interrupt
		AES->AES_IER = AES_IER_DATRDY;

		for (i = 0; i < indata_len; i += params->block_size) {

			sama_aes_lld_set_input((uint32_t *) ((in) + i));

			AES->AES_CR = AES_CR_START;

			while ((AES->AES_ISR & AES_ISR_DATRDY) != AES_ISR_DATRDY);

			sama_aes_lld_get_output((uint32_t *) ((out) + i));
		}

	}

	osalMutexUnlock(&cryp->mutex);

	return CRY_NOERROR;

}

cryerror_t sama_aes_lld_process_dma(CRYDriver *cryp,  aesparams *params,
		const uint8_t *in, uint8_t *out, size_t indata_len) {
#if defined(SAMA_DMA_REQUIRED)
	cryerror_t ret;

	osalDbgAssert(cryp->thread == NULL, "already waiting");
  osalDbgAssert(!((uint32_t) in & (L1_CACHE_BYTES - 1)), "in address not cache aligned");
  osalDbgAssert(!((uint32_t) out & (L1_CACHE_BYTES - 1)), "out address not cache aligned");

#if 0
  osalDbgAssert(!(indata_len & (L1_CACHE_BYTES - 1)), "size not multiple of cache line");
#endif

  /*
   * If size is not multiple of cache line, clean cache region is required.
   * TODO: remove when size assert works
   */
  if (indata_len & (L1_CACHE_BYTES - 1)) {
    cacheCleanRegion((uint8_t *) out, indata_len);
  }

	osalMutexLock(&cryp->mutex);

  cacheCleanRegion((uint8_t *) in, indata_len);

  cryp->out = out;
  cryp->in = in;
  cryp->len = indata_len;

	//set chunk size
	cryp->dmachunksize = DMA_CHUNK_SIZE_4;

	if ((cryp->config->cfbs != AES_CFBS_128))
		cryp->dmachunksize = DMA_CHUNK_SIZE_1;

	//set dma width
	cryp->dmawith = DMA_DATA_WIDTH_WORD;

	if (cryp->config->cfbs == AES_CFBS_16)
		cryp->dmawith = DMA_DATA_WIDTH_HALF_WORD;
	if (cryp->config->cfbs == AES_CFBS_8)
		cryp->dmawith = DMA_DATA_WIDTH_BYTE;

	cryp->rxdmamode = XDMAC_CC_TYPE_PER_TRAN |
			XDMAC_CC_PROT_SEC |
			XDMAC_CC_MBSIZE_SINGLE |
			XDMAC_CC_DSYNC_PER2MEM | XDMAC_CC_CSIZE(cryp->dmachunksize) |
			XDMAC_CC_DWIDTH(cryp->dmawith) |
			XDMAC_CC_SIF_AHB_IF1 |
			XDMAC_CC_DIF_AHB_IF0 |
			XDMAC_CC_SAM_FIXED_AM |
			XDMAC_CC_DAM_INCREMENTED_AM |
			XDMAC_CC_PERID(PERID_AES_RX);

	cryp->txdmamode = XDMAC_CC_TYPE_PER_TRAN |
			XDMAC_CC_PROT_SEC |
			XDMAC_CC_MBSIZE_SINGLE |
			XDMAC_CC_DSYNC_MEM2PER | XDMAC_CC_CSIZE(cryp->dmachunksize) |
			XDMAC_CC_DWIDTH(cryp->dmawith) |
			XDMAC_CC_SIF_AHB_IF0 |
			XDMAC_CC_DIF_AHB_IF1 |
			XDMAC_CC_SAM_INCREMENTED_AM |
			XDMAC_CC_DAM_FIXED_AM |
			XDMAC_CC_PERID(PERID_AES_TX);

	dmaChannelSetMode(cryp->dmarx, cryp->rxdmamode);
	dmaChannelSetMode(cryp->dmatx, cryp->txdmamode);

	// Writing channel
	dmaChannelSetSource(cryp->dmatx, in);
	dmaChannelSetDestination(cryp->dmatx, AES->AES_IDATAR);
	dmaChannelSetTransactionSize(cryp->dmatx,  ( indata_len / DMA_DATA_WIDTH_TO_BYTE(cryp->dmawith)));


	// Reading channel
	dmaChannelSetSource(cryp->dmarx, AES->AES_ODATAR);
	dmaChannelSetDestination(cryp->dmarx, out);
	dmaChannelSetTransactionSize(cryp->dmarx,  ( indata_len / DMA_DATA_WIDTH_TO_BYTE(cryp->dmawith)));

	//AES soft reset
	AES->AES_CR = AES_CR_SWRST;

	//AES set op mode
	AES->AES_MR |= ((AES_MR_OPMOD_Msk & (params->mode)) | AES_MR_CKEY_PASSWD);

	//AES set key size
	ret = sama_aes_lld_set_key_size(cryp->key0_size);

	if (ret == CRY_NOERROR) {

		AES->AES_MR |= (AES_MR_CFBS(cryp->config->cfbs) | AES_MR_CKEY_PASSWD);

		sama_aes_lld_write_key(cryp->key0_buffer,( const uint32_t *) params->iv, cryp->key0_size);

		if (params->encrypt)
			AES->AES_MR |= AES_MR_CIPHER;
		else
			AES->AES_MR &= ~AES_MR_CIPHER;

		AES->AES_MR |= (((AES_MR_SMOD_Msk & (AES_MR_SMOD_IDATAR0_START)))
				| AES_MR_CKEY_PASSWD);

		//Enable aes interrupt
		AES->AES_IER = AES_IER_DATRDY;

	}

	osalSysLock();

	dmaChannelEnable(cryp->dmarx);
	dmaChannelEnable(cryp->dmatx);

	osalThreadSuspendS(&cryp->thread);

	osalSysUnlock();

	osalMutexUnlock(&cryp->mutex);
#endif //#if defined(SAMA_DMA_REQUIRED)
	return CRY_NOERROR;

}

#endif /* HAL_USE_CRY */

/** @} */
