/*
 * This file is part of INAV.
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

#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/bus.h"

// Board hardware definitions
BUSDEV_REGISTER_SPI_TAG(busdev_mpu6000,     DEVHW_MPU6000,      MPU6000_SPI_BUS,    MPU6000_CS_PIN,     NONE,       0,  DEVFLAGS_NONE);
BUSDEV_REGISTER_SPI_TAG(busdev_mpu6500,     DEVHW_MPU6500,      MPU6500_SPI_BUS,    MPU6500_CS_PIN,     NONE,       1,  DEVFLAGS_NONE);

const timerHardware_t timerHardware[] = {
    DEF_TIM(TIM4, CH2, PB7, TIM_USE_PPM,                            0, 0), // PPM / UART1_RX

    // OUTPUT 1-4
    DEF_TIM(TIM3, CH2, PB5, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,    1, 0),  // D(1, 5, 5)
    DEF_TIM(TIM3, CH1, PB4, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,    1, 0),  // D(1, 4, 5)
//    DEF_TIM(TIM3, CH3, PB0, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,    1, 0),  // D(1, 7, 5)
	DEF_TIM(TIM3, CH3, PB0, TIM_USE_MC_MOTOR,                       1, 0), // S1_OUT    D(1,7)
    DEF_TIM(TIM1, CH2N, PB0,                    TIM_USE_FW_MOTOR,   1, 1), // S1_OUT    D(2,2,0),D(2,3,7)
	
    DEF_TIM(TIM3, CH4, PB1, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,    1, 0),  // D(1, 2, 5)

    // OUTPUT 5-6
	#ifdef GRAUPNERF7NXT
	DEF_TIM(TIM8, CH4, PC9, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,    1, 0),  // D(2, 7, 7)
    DEF_TIM(TIM8, CH3, PC8, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO,    1, 1),  // D(2, 2, 0)
//	DEF_TIM(TIM3, CH3, PC8,                    TIM_USE_FW_SERVO,    0, 0),
	
	#else
    DEF_TIM(TIM8, CH4, PC9, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,    1, 0),  // D(2, 7, 7)
    DEF_TIM(TIM8, CH3, PC8, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR,    1, 1),  // D(2, 2, 0)
	#endif

	#ifdef GRAUPNERF7NXT
	DEF_TIM(TIM2, CH1, PA0, TIM_USE_MC_SERVO | TIM_USE_FW_SERVO,    0, 0),  // S7_OUT TX4  		TIM2 or TIM5
//	DEF_TIM(TIM2, CH2, PA1, TIM_USE_MC_SERVO,                       0, 0),  // S8_OUT RX4		TIM2 or TIM5
	DEF_TIM(TIM2, CH2, PA1, TIM_USE_MC_SERVO | TIM_USE_FW_SERVO,    0, 0),  // S8_OUT RX4		TIM2 or TIM5	
	
//	DEF_TIM(TIM5, CH3, PA2, TIM_USE_MC_SERVO | TIM_USE_FW_SERVO,    0, 0),  // S9_OUT TX2		TIM2 or TIM5 or TIM9_CH1
//	DEF_TIM(TIM5, CH4, PA3, TIM_USE_MC_SERVO | TIM_USE_FW_SERVO,    0, 0),  // S10_OUT RX2		TIM2 or TIM5 or TIM9_CH2
	#endif

//	#ifdef GRAUPNERF7NXT
//	DEF_TIM(TIM1, CH2, PA9, TIM_USE_MC_SERVO | TIM_USE_FW_MOTOR,    0, 0), // S10_OUT LED Strip
//	#else
//	DEF_TIM(TIM1, CH2, PA9, TIM_USE_LED,                            1, 0), // LED strip D(1,0)	
//	#endif

    // AUXILARY pins
    DEF_TIM(TIM1, CH2, PA9, TIM_USE_LED,                            1, 0),  // LED
    DEF_TIM(TIM4, CH1, PB6, TIM_USE_ANY,                            0, 0)   // SS1 TX / UART1_TX
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
