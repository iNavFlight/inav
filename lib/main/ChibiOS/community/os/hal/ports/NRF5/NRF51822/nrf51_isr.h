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
 * @file    NRF51822/nrf51_isr.h
 * @brief   NRF51822 ISR handler header.
 *
 * @addtogroup NRF51822_ISR
 * @{
 */

#ifndef NRF51_ISR_H
#define NRF51_ISR_H

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

#endif /* NRF51_ISR_H */

/** @} */
