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
 * Output mapping table — 10 GPIO pins in 5 slice groups.
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
 *   GP12 = PA12  GP13 = PA13  GP14 = PA14  GP15 = PA15  (dual-use: UART3/4)
 *   GP20 = PB4   GP21 = PB5
 * GP16-19 are reserved: Flash CS (GP16), Beeper (GP17), I2C1 SDA/SCL (GP18/19).
 */
timerHardware_t timerHardware[] = {
    /* slice 4 — motor group (GP8/GP9) */
    DEF_TIM(TIM4,  CH1, PA8,  TIM_USE_OUTPUT_AUTO, 0, 0),  /* GP8  */
    DEF_TIM(TIM4,  CH2, PA9,  TIM_USE_OUTPUT_AUTO, 0, 0),  /* GP9  */
    /* slice 5 — motor group (GP10/GP11) */
    DEF_TIM(TIM5,  CH1, PA10, TIM_USE_OUTPUT_AUTO, 0, 0),  /* GP10 */
    DEF_TIM(TIM5,  CH2, PA11, TIM_USE_OUTPUT_AUTO, 0, 0),  /* GP11 */
    /* slice 6 — servo group (GP12/GP13; dual-use with UART3 TX/RX on PIO1) */
    DEF_TIM(TIM6,  CH1, PA12, TIM_USE_OUTPUT_AUTO, 0, 0),  /* GP12 */
    DEF_TIM(TIM6,  CH2, PA13, TIM_USE_OUTPUT_AUTO, 0, 0),  /* GP13 */
    /* slice 7 — servo group (GP14/GP15; dual-use with UART4 TX/RX on PIO1) */
    DEF_TIM(TIM7,  CH1, PA14, TIM_USE_OUTPUT_AUTO, 0, 0),  /* GP14 */
    DEF_TIM(TIM7,  CH2, PA15, TIM_USE_OUTPUT_AUTO, 0, 0),  /* GP15 */
    /* slice 10 — servo group (GP20/GP21; dedicated, GP16-19 reserved) */
    DEF_TIM(TIM10, CH1, PB4,  TIM_USE_OUTPUT_AUTO, 0, 0),  /* GP20 */
    DEF_TIM(TIM10, CH2, PB5,  TIM_USE_OUTPUT_AUTO, 0, 0),  /* GP21 */
};
const int timerHardwareCount = 10;

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
