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
#ifndef SAMA_CRYPTO_LLD_H_
#define SAMA_CRYPTO_LLD_H_

#if HAL_USE_CRY || defined(__DOXYGEN__)

extern void 		samaCryptoDriverInit(CRYDriver *cryp);
extern void 		samaCryptoDriverStart(CRYDriver *cryp);
extern void 		samaCryptoDriverStop(CRYDriver *cryp);
extern void 		samaCryptoDriverDisable(CRYDriver *cryp);


#define AES_PER		0x01
#define TRNG_PER	0x02
#define SHA_PER		0x04
#define TDES_PER	0x08

#define DMA_DATA_WIDTH_BYTE        	0
#define DMA_DATA_WIDTH_HALF_WORD   	1
#define DMA_DATA_WIDTH_WORD        	2
#define DMA_DATA_WIDTH_DWORD       	3

#define DMA_CHUNK_SIZE_1   			0
#define DMA_CHUNK_SIZE_2   			1
#define DMA_CHUNK_SIZE_4   			2
#define DMA_CHUNK_SIZE_8   			3
#define DMA_CHUNK_SIZE_16  			4

#define DMA_DATA_WIDTH_TO_BYTE(w)   (1 << w)

#ifndef SAMA_CRY_CRYD1_DMA_IRQ_PRIORITY
#define SAMA_CRY_CRYD1_DMA_IRQ_PRIORITY	4
#endif

#ifndef SAMA_CRY_CRYD1_IRQ_PRIORITY
#define SAMA_CRY_CRYD1_IRQ_PRIORITY	4
#endif

#ifndef SAMA_CRY_DMA_ERROR_HOOK
#define SAMA_CRY_DMA_ERROR_HOOK(cryp)		osalSysHalt("DMA failure")
#endif

#ifndef SAMA_CRY_SHA_UPDATE_LEN_MAX
#define SAMA_CRY_SHA_UPDATE_LEN_MAX 128*1024
#endif


#include "sama_aes_lld.h"
#include "sama_tdes_lld.h"
#include "sama_sha_lld.h"
#include "sama_gcm_lld.h"

#endif /* HAL_USE_CRY */

#endif //SAMA_CRYPTO_LLD_H_

/** @} */
