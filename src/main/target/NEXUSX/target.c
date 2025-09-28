/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include "platform.h"

#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "target.h"

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM3, CH1, PB4,   TIM_USE_OUTPUT_AUTO, 0, 0),   // labelled "S1"
    DEF_TIM(TIM3, CH2, PB5,   TIM_USE_OUTPUT_AUTO, 0, 0),   // labelled "S2"
    DEF_TIM(TIM3, CH3, PB0,   TIM_USE_OUTPUT_AUTO, 0, 0),   // labelled "S3"

    DEF_TIM(TIM2, CH1, PA15,  TIM_USE_OUTPUT_AUTO, 0, 0),   // labelled "TAIL"

    DEF_TIM(TIM1, CH2, PA9,   TIM_USE_OUTPUT_AUTO, 0, 0),   // labelled "ESC"

    DEF_TIM(TIM5, CH3, PA2,   TIM_USE_OUTPUT_AUTO, 0, 0),   // labelled "RPM", clashes with UART2 TX
    DEF_TIM(TIM5, CH4, PA3,   TIM_USE_OUTPUT_AUTO, 0, 0),   // labelled "TLM", clashes with UART2 RX

#if defined(NEXUSX_9SERVOS) || defined(NEXUSX_NOI2C)
    DEF_TIM(TIM4, CH1, PB6,   TIM_USE_OUTPUT_AUTO, 0, 0),   // labelled "AUX", clashes with UART1 TX and I2C1 SCL
    DEF_TIM(TIM4, CH2, PB7,   TIM_USE_OUTPUT_AUTO, 0, 0),   // labelled "SBUS", clashes with UART1 RX and I2C1 SDA
#endif
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
