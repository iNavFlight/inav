/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <platform.h>
#include "target.h"

#include "common/utils.h"
#include "drivers/system.h"
#include "drivers/pwm_mapping.h"
#include "drivers/pwm_output.h"
#include "drivers/timer.h"
#include "drivers/serial.h"
#include "drivers/serial_uart.h"
#include "drivers/serial_uart_impl.h"
#include "arm_math.h"

const int timerHardwareCount = 0;
timerHardware_t timerHardware[1];

// Motor GPIO pin assignments for PIO0 DShot output.
// Analogous to timerHardware[] on STM32: target-specific pin routing data.
//   M1 → GP8  (PIO0 SM0)
//   M2 → GP9  (PIO0 SM1)
//   M3 → GP10 (PIO0 SM2)
//   M4 → GP11 (PIO0 SM3)
// GP4–7 reserved for SPI0 (gyro + flash); future motors 5/6 would use SPI1 pins.
const uint8_t rp2350MotorPins[] = {8, 9, 10, 11};
const int     rp2350MotorPinCount = 4;

// Servo GPIO pin assignments for hardware PWM slices.
//   S1 → GP16  (PWM slice 0, ch A)
//   S2 → GP17  (PWM slice 0, ch B)
//   S3 → GP18  (PWM slice 1, ch A)
//   S4 → GP19  (PWM slice 1, ch B)
// GP16–19 are free of UARTs, SPI, PIO assignments.
const uint8_t rp2350ServoPins[] = {16, 17, 18, 19};
const int     rp2350ServoPinCount = 4;

// --- Functions still stubbed (no real hardware driver yet) ---

void failureMode(failureMode_e mode)
{
    UNUSED(mode);
    while (1) {}
}

void timerInit(void)
{
    // NOP — no hardware timers configured yet
}

bool isMPUSoftReset(void)
{
    return false;
}

// Timer stub (timer.c is excluded from build)
uint8_t timer2id(const HAL_Timer_t *tim)
{
    UNUSED(tim);
    return 0;
}

// UART dispatch — hardware UARTs 1-2 via RP2350 PL011, PIO UARTs 3-4 via PIO1
serialPort_t *uartOpen(USART_TypeDef *USARTx, serialReceiveCallbackPtr rxCallback,
                        void *rxCallbackData, uint32_t baudRate,
                        portMode_t mode, portOptions_t options)
{
    uartPort_t *s = NULL;
    if      (USARTx == USART1) { s = serialUART1(baudRate, mode, options); }
    else if (USARTx == USART2) { s = serialUART2(baudRate, mode, options); }
    else if (USARTx == USART3) { s = serialUART3(baudRate, mode, options); }
    else if (USARTx == USART4) { s = serialUART4(baudRate, mode, options); }
    if (!s) { return NULL; }
    s->port.rxCallback     = rxCallback;
    s->port.rxCallbackData = rxCallbackData;
    return (serialPort_t *)s;
}

// CMSIS DSP stubs (DSP library not compiled for RP2350)
arm_status arm_rfft_fast_init_f32(arm_rfft_fast_instance_f32 *S, uint16_t fftLen)
{
    UNUSED(S);
    UNUSED(fftLen);
    return ARM_MATH_SUCCESS;
}

void arm_cfft_radix8by4_f32(arm_cfft_instance_f32 *S, float32_t *p1)
{
    UNUSED(S);
    UNUSED(p1);
}

void arm_bitreversal_32(uint32_t *pSrc, const uint16_t bitRevLen,
                         const uint16_t *pBitRevTable)
{
    UNUSED(pSrc);
    UNUSED(bitRevLen);
    UNUSED(pBitRevTable);
}

void stage_rfft_f32(arm_rfft_fast_instance_f32 *S, float32_t *p, float32_t *pOut)
{
    UNUSED(S);
    UNUSED(p);
    UNUSED(pOut);
}

void arm_cmplx_mag_f32(float32_t *pSrc, float32_t *pDst, uint32_t numSamples)
{
    UNUSED(pSrc);
    UNUSED(pDst);
    UNUSED(numSamples);
}

void arm_mult_f32(float32_t *pSrcA, float32_t *pSrcB,
                   float32_t *pDst, uint32_t blockSize)
{
    UNUSED(pSrcA);
    UNUSED(pSrcB);
    UNUSED(pDst);
    UNUSED(blockSize);
}

void arm_sub_f32(float32_t *pSrcA, float32_t *pSrcB,
                  float32_t *pDst, uint32_t blockSize)
{
    UNUSED(pSrcA);
    UNUSED(pSrcB);
    UNUSED(pDst);
    UNUSED(blockSize);
}

void arm_scale_f32(float32_t *pSrc, float32_t scale,
                    float32_t *pDst, uint32_t blockSize)
{
    UNUSED(pSrc);
    UNUSED(scale);
    UNUSED(pDst);
    UNUSED(blockSize);
}
