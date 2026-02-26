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

#include <stdbool.h>

#include "platform.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

/*
 * Timer Allocation for FrSky F405
 *
 * Connector mapping from schematic (CONN.SchDoc):
 * - CON1 (S1): T4_2 = TIM4_CH2 = PB7
 * - CON2 (S2): T4_1 = TIM4_CH1 = PB6
 * - CON3 (S3): T3_3 = TIM3_CH3 = PB0
 * - CON4 (S4): T3_4 = TIM3_CH4 = PB1
 * - CON5 (S5): T8_3 = TIM8_CH3 = PC8
 * - CON6 (S6): T8_4 = TIM8_CH4 = PC9
 * - CON7 (S7): T12_1 = TIM12_CH1 = PB14  (no DMA - no DShot)
 * - CON8 (S8): T12_2 = TIM12_CH2 = PB15  (no DMA - no DShot)
 * - CON9 (S9): T1_1  = TIM1_CH1  = PA8
 * - CON23 (LED): T2_1 = TIM2_CH1 = PA15
 */

timerHardware_t timerHardware[] = {
    // Motor outputs S1-S9 (in physical connector order from schematic)
    DEF_TIM(TIM4,  CH2,  PB7,  TIM_USE_OUTPUT_AUTO, 0, 0),  // S1 (CON1) - Motor 1
    DEF_TIM(TIM4,  CH1,  PB6,  TIM_USE_OUTPUT_AUTO, 0, 0),  // S2 (CON2) - Motor 2
    DEF_TIM(TIM3,  CH3,  PB0,  TIM_USE_OUTPUT_AUTO, 0, 0),  // S3 (CON3) - Motor 3
    DEF_TIM(TIM3,  CH4,  PB1,  TIM_USE_OUTPUT_AUTO, 0, 0),  // S4 (CON4) - Motor 4
    DEF_TIM(TIM8,  CH3,  PC8,  TIM_USE_OUTPUT_AUTO, 0, 1),  // S5 (CON5) - Motor 5  UP(2,1)
    DEF_TIM(TIM8,  CH4,  PC9,  TIM_USE_OUTPUT_AUTO, 0, 0),  // S6 (CON6) - Motor 6
    DEF_TIM(TIM12, CH1,  PB14, TIM_USE_OUTPUT_AUTO, 0, 0),  // S7 (CON7) - Motor 7  NOTE: TIM12 has no DMA - DShot not supported
    DEF_TIM(TIM12, CH2,  PB15, TIM_USE_OUTPUT_AUTO, 0, 0),  // S8 (CON8) - Motor 8  NOTE: TIM12 has no DMA - DShot not supported
    DEF_TIM(TIM1,  CH1,  PA8,  TIM_USE_OUTPUT_AUTO, 0, 0),  // S9 (CON9) - Motor 9  UP(2,5)

    // LED Strip on CON23 - PA15 (T2_1 signal on schematic)
    DEF_TIM(TIM2,  CH1,  PA15, TIM_USE_LED, 0, 0),          // LED strip (CON23)
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);

/*
 * IMPLEMENTATION NOTES:
 *
 * 1. S1/S2 order matches physical board labels (CON1=S1, CON2=S2 from schematic)
 *
 * 2. TIM12 (S7, S8): No DMA support on STM32F405.
 *    Works: Standard PWM, OneShot125, OneShot42, MultiShot
 *    Does NOT work: DShot (all variants), ProShot
 *    Use S7/S8 for non-DShot ESCs or servos
 *
 * 3. LED strip output: PA15 (T2_1) on CON23 with TIM2_CH1
 */
