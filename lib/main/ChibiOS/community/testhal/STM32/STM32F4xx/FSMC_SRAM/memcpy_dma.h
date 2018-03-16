/*
    ChibiOS/RT - Copyright (C) 2013-2014 Uladzimir Pylinsky aka barthess

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

#ifndef MEMCPY_DMA_H_
#define MEMCPY_DMA_H_

/*
 *
 */
typedef struct {
  const stm32_dma_stream_t *dma;
} memcpy_dma_engine_t;

/*
 *
 */
#ifdef __cplusplus
extern "C" {
#endif
  void memcpy_dma_start(void);
  void memcpy_dma_stop(void);
  void memcpy_dma(void *dest, const void *src, size_t size);
#ifdef __cplusplus
}
#endif

#endif /* MEMCPY_DMA_H_ */
