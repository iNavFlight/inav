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
#include <string.h>
#if (HAL_USE_CRY == TRUE) || defined(__DOXYGEN__)

#include "sama_crypto_lld.h"

static void incr32(uint8_t* j0)
{

    uint32_t counter = j0[15] | j0[14] << 0x8 | j0[13] << 0x10 | j0[12] << 0x18;

    counter++;

    j0[12] = (counter>>24) & 0xFF;
    j0[13] = (counter>>16) & 0xFF;
    j0[14] = (counter>>8) & 0xFF;
    j0[15] = counter & 0xFF;
}

static cryerror_t sama_gcm_lld_process_dma(CRYDriver *cryp,cgmcontext * cxt)
{
#if defined(SAMA_DMA_REQUIRED)

  osalDbgAssert(!((uint32_t) cxt->in & (L1_CACHE_BYTES - 1)), "in address not cache aligned");
  osalDbgAssert(!((uint32_t) cxt->out & (L1_CACHE_BYTES - 1)), "out address not cache aligned");
	osalDbgAssert(cryp->thread == NULL, "already waiting");

#if 0
  osalDbgAssert(!(cxt->c_size & (L1_CACHE_BYTES - 1)), "size not multiple of cache line");
#endif

  cacheCleanRegion((uint8_t *) cxt->in, cxt->c_size);

  /*
   * If size is not multiple of cache line, clean cache region is required.
   * TODO: remove when size assert works
   */
  if (cxt->c_size & (L1_CACHE_BYTES - 1)) {
    cacheCleanRegion((uint8_t *) cxt->out, cxt->c_size);
  }

  cryp->out = cxt->out;
  cryp->in = cxt->in;
  cryp->len = cxt->c_size;

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
	dmaChannelSetSource(cryp->dmatx, cxt->in);
	dmaChannelSetDestination(cryp->dmatx, AES->AES_IDATAR);
	dmaChannelSetTransactionSize(cryp->dmatx,  ( cxt->c_size / DMA_DATA_WIDTH_TO_BYTE(cryp->dmawith)));


	// Reading channel
	dmaChannelSetSource(cryp->dmarx, AES->AES_ODATAR);
	dmaChannelSetDestination(cryp->dmarx, cxt->out);
	dmaChannelSetTransactionSize(cryp->dmarx,  ( cxt->c_size / DMA_DATA_WIDTH_TO_BYTE(cryp->dmawith)));

	AES->AES_MR |= (((AES_MR_SMOD_Msk & (AES_MR_SMOD_IDATAR0_START)))
			| AES_MR_CKEY_PASSWD);

	//Enable aes interrupt
	AES->AES_IER = AES_IER_DATRDY;

	osalSysLock();

	dmaChannelEnable(cryp->dmarx);
	dmaChannelEnable(cryp->dmatx);

	osalThreadSuspendS(&cryp->thread);

	osalSysUnlock();


#endif //#if defined(SAMA_DMA_REQUIRED)
	return CRY_NOERROR;

}

cryerror_t sama_gcm_lld_process(CRYDriver *cryp,cgmcontext * cxt)
{
	cryerror_t ret;
	uint32_t *ref32;
	uint8_t i;
	uint8_t J0[16] = { 0x00 };


	osalMutexLock(&cryp->mutex);


	//AES soft reset
	AES->AES_CR = AES_CR_SWRST;

	//AES set op mode
	AES->AES_MR =((AES_MR_OPMOD_Msk & (AES_MR_OPMOD_GCM)) | AES_MR_GTAGEN | AES_MR_CKEY_PASSWD);


	//AES set key size
	ret = sama_aes_lld_set_key_size(cryp->key0_size);


	if (ret == CRY_NOERROR) {

		AES->AES_MR |= ( ((AES_MR_SMOD_Msk & (AES_MR_SMOD_MANUAL_START))) | AES_MR_CKEY_PASSWD);

		sama_aes_lld_write_key(cryp->key0_buffer,NULL, cryp->key0_size);

		AES->AES_CR = AES_CR_START;

		while ((AES->AES_ISR & AES_ISR_DATRDY) != AES_ISR_DATRDY);

		//J0

		memcpy(J0,  cxt->params.iv, 16); // copy the IV to the first 12 bytes of J0

		incr32(J0);

		// Write incr32(J0) into IV.

		ref32 = (uint32_t*)J0;
		AES->AES_IVR[0] = ref32[0];
		AES->AES_IVR[1] = ref32[1];
		AES->AES_IVR[2] = ref32[2];
		AES->AES_IVR[3] = ref32[3];


		AES->AES_AADLENR = cxt->aadsize;
		AES->AES_CLENR = cxt->c_size;

		if (cxt->params.encrypt)
			AES->AES_MR |= AES_MR_CIPHER;
		else
			AES->AES_MR &= ~AES_MR_CIPHER;

		AES->AES_MR |= AES_MR_GTAGEN| AES_MR_CKEY_PASSWD;


		for (i = 0; i < cxt->aadsize; i += cxt->params.block_size) {

			sama_aes_lld_set_input((uint32_t *) ((cxt->aad) + i));

			AES->AES_CR = AES_CR_START;

			while ((AES->AES_ISR & AES_ISR_DATRDY) != AES_ISR_DATRDY);

		}

		if (cryp->config->transfer_mode == TRANSFER_POLLING) {
			for (i = 0; i < cxt->c_size; i += cxt->params.block_size) {

				sama_aes_lld_set_input((uint32_t *) ((cxt->in) + i));

				AES->AES_CR = AES_CR_START;

				while ((AES->AES_ISR & AES_ISR_DATRDY) != AES_ISR_DATRDY);

				sama_aes_lld_get_output((uint32_t *) ((cxt->out) + i));
			}
		}
		else
		{
			sama_gcm_lld_process_dma(cryp,cxt);
		}

		while ((AES->AES_ISR & AES_ISR_TAGRDY) != AES_ISR_TAGRDY);

		ref32 = (uint32_t*)cxt->authtag;

		for (i = 0; i < 4; i++) {
			ref32[i] =AES->AES_TAGR[i];
		}


	}
	osalMutexUnlock(&cryp->mutex);

	return ret;

}


#endif
