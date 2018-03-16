/*
    ChibiOS/RT - Copyright (C) 2014 Uladzimir Pylinsky aka barthess

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

#ifndef DMA_STORM_H_
#define DMA_STORM_H_

#ifdef __cplusplus
extern "C" {
#endif
  void dma_storm_spi_start(void);
  uint32_t dma_storm_spi_stop(void);
  void dma_storm_adc_start(void);
  uint32_t dma_storm_adc_stop(void);
  void dma_storm_uart_start(void);
  uint32_t dma_storm_uart_stop(void);
#ifdef __cplusplus
}
#endif

#endif /* DMA_STORM_H_ */
