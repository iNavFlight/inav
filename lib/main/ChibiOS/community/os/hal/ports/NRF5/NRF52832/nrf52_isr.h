/*
    Copyright (C) 2018 Konstantin Oblaukhov
    Copyright (C) 2015 Stephen Caudle

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
 * @file    NRF52832/nrf52_isr.h
 * @brief   NRF52832 ISR handler header.
 *
 * @addtogroup NRF52832_ISR
 * @{
 */

#ifndef NRF52_ISR_H
#define NRF52_ISR_H

#if !defined(NRF5_IRQ_GPIOTE_PRIORITY) || defined(__DOXYGEN__)
#define NRF5_IRQ_GPIOTE_PRIORITY      3
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void irqInit(void);
  void irqDeinit(void);
#ifdef __cplusplus
}
#endif

#endif /* NRF52_ISR_H */

/** @} */
