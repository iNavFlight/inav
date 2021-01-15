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
#ifndef CRYPTOLIB_LLD_TDES_H_
#define CRYPTOLIB_LLD_TDES_H_



typedef enum  {
	TDES_MODE_ECB = 0,
	TDES_MODE_CBC,
	TDES_MODE_OFB,
	TDES_MODE_CFB
}tdes_mode_t;



typedef enum  {
	TDES_CFBS_64 = 0,
	TDES_CFBS_32,
	TDES_CFBS_16,
	TDES_CFBS_8
}tdes_cipher_size_t;

typedef struct  {
	tdes_algo_t					algo;
	tdes_mode_t					mode;
}tdes_config_t;


extern void sama_tdes_lld_init(CRYDriver *cryp);

extern cryerror_t sama_tdes_lld_polling(CRYDriver *cryp,
		tdes_config_t *params,
		bool encrypt,
		const uint8_t *data,
		size_t data_len,
		uint8_t *out,
		const uint8_t *iv);
extern cryerror_t sama_tdes_lld_dma(CRYDriver *cryp,
		tdes_config_t *params,
		bool encrypt,
		const uint8_t *data,
		size_t data_len,
		uint8_t *out,
		const uint8_t *iv);


#endif /* CRYPTOLIB_LLD_TDES_H_ */
