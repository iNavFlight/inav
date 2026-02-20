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
#include "drivers/io.h"
#include "drivers/system.h"
#include "drivers/pwm_mapping.h"
#include "drivers/pwm_output.h"
#include "drivers/timer.h"
#include "drivers/serial.h"
#include "drivers/serial_uart.h"
#include "drivers/serial_uart_impl.h"

/*
 * Output mapping table — 8 GPIO pins in 4 slice groups.
 *
 * All entries use TIM_USE_OUTPUT_AUTO so pwmMotorAndServoInit() assigns
 * them as motors or servos based on getMotorCount() and any
 * timerOverrides() the user has set in the Configurator.
 *
 * Field order matches timerHardware_t (non-AT32 build):
 *   tim, tag, channelIndex, output, ioMode, alternateFunction, usageFlags, dmaTag
 *
 * Port A (gpioid 0) = GPIO 0–15; Port B (gpioid 1) = GPIO 16–29.
 *   GP8  = PA8   GP9  = PA9   GP10 = PA10  GP11 = PA11
 *   GP16 = PB0   GP17 = PB1   GP18 = PB2   GP19 = PB3
 */
timerHardware_t timerHardware[] = {
    /* slice 4 — motor group 1 */
    DEF_TIM(TIM4, CH1, PA8,  TIM_USE_OUTPUT_AUTO, 0, 0),  /* GP8  */
    DEF_TIM(TIM4, CH2, PA9,  TIM_USE_OUTPUT_AUTO, 0, 0),  /* GP9  */
    /* slice 5 — motor group 2 */
    DEF_TIM(TIM5, CH1, PA10, TIM_USE_OUTPUT_AUTO, 0, 0),  /* GP10 */
    DEF_TIM(TIM5, CH2, PA11, TIM_USE_OUTPUT_AUTO, 0, 0),  /* GP11 */
    /* slice 8 — flexible group 1 */
    DEF_TIM(TIM8, CH1, PB0,  TIM_USE_OUTPUT_AUTO, 0, 0),  /* GP16 */
    DEF_TIM(TIM8, CH2, PB1,  TIM_USE_OUTPUT_AUTO, 0, 0),  /* GP17 */
    /* slice 9 — flexible group 2 */
    DEF_TIM(TIM9, CH1, PB2,  TIM_USE_OUTPUT_AUTO, 0, 0),  /* GP18 */
    DEF_TIM(TIM9, CH2, PB3,  TIM_USE_OUTPUT_AUTO, 0, 0),  /* GP19 */
};
const int timerHardwareCount = 8;

void failureMode(failureMode_e mode)
{
    UNUSED(mode);
    while (1) {}
}

bool isMPUSoftReset(void)
{
    return false;
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
