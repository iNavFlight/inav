/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include "platform.h"
#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/pinio.h"
#include "drivers/sensor.h"

BUSDEV_REGISTER_SPI_TAG(busdev_ICM42605,   DEVHW_ICM42605,  ICM42605_SPI_BUS,   ICM42605_CS_PIN,   NONE,   0,  DEVFLAGS_NONE,  IMU_ICM42605_ALIGN);


timerHardware_t timerHardware[] = {

    DEF_TIM(TIM8, CH1, PC6,     TIM_USE_OUTPUT_AUTO, 0, 0),  // S1
    DEF_TIM(TIM8, CH2, PC7,     TIM_USE_OUTPUT_AUTO, 0, 1),  // S2
    DEF_TIM(TIM8, CH3, PC8,     TIM_USE_OUTPUT_AUTO, 0, 2),  // S3
    DEF_TIM(TIM8, CH4, PC9,     TIM_USE_OUTPUT_AUTO, 0, 3),  // S4
    DEF_TIM(TIM4, CH1, PB6,     TIM_USE_OUTPUT_AUTO, 0, 4),  // S5
    DEF_TIM(TIM4, CH2, PB7,     TIM_USE_OUTPUT_AUTO, 0, 5),  // S6 
    DEF_TIM(TIM4, CH3, PD14,    TIM_USE_OUTPUT_AUTO, 0, 6),  // S7
    DEF_TIM(TIM4, CH4, PD15,    TIM_USE_OUTPUT_AUTO, 0, 0),  // S8 DMA_NONE

    DEF_TIM(TIM3, CH2, PB5,  TIM_USE_LED, 0, 9),    // LED
    DEF_TIM(TIM2, CH2, PB3,  TIM_USE_ANY, 0, 0), // Camera Control
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
