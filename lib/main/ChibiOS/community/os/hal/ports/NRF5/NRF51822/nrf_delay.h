/*
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
 * @file    NRF5/NRF51822/nrf_delay.h
 * @brief   NRF5 Delay routines
 *
 * @{
 */

#ifndef _NRF_DELAY_H
#define _NRF_DELAY_H

inline static void nrf_delay_us(uint32_t volatile number_of_us) __attribute__((always_inline));
inline static void nrf_delay_us(uint32_t volatile number_of_us)
{
register uint32_t delay __asm ("r0") = number_of_us;
__asm volatile (
".syntax unified\n"
    "1:\n"
    " SUBS %0, %0, #1\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " BNE 1b\n"
    ".syntax divided\n"
    : "+r" (delay));
}
#endif //__NRF_DELAY_H
