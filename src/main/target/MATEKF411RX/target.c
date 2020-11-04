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

#include <stdbool.h>
#include <platform.h>

#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/bus.h"


BUSDEV_REGISTER_SPI(busdev_rx_spi, DEVHW_RX_SPI, BUS_SPI3, RX_NSS_PIN, NONE, DEVFLAGS_USE_RAW_REGISTERS, 0);

const timerHardware_t timerHardware[] = {
    DEF_TIM(TIM9, CH2, PA3,  TIM_USE_PPM,   0, 0), // PPM/RX2

    DEF_TIM(TIM2, CH3, PB10, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR, 0, 0), // S1_OUT - DMA1_ST1
    DEF_TIM(TIM4, CH1, PB6,  TIM_USE_MC_SERVO | TIM_USE_FW_SERVO, 0, 0), // S2_OUT - DMA1_ST0
    DEF_TIM(TIM4, CH2, PB7,  TIM_USE_MC_SERVO | TIM_USE_FW_SERVO, 0, 0), // S3_OUT - DMA1_ST3
    DEF_TIM(TIM4, CH3, PB8,  TIM_USE_MC_SERVO | TIM_USE_FW_SERVO, 0, 0), // S4_OUT - DMA1_ST7

    DEF_TIM(TIM5, CH1, PA0,  TIM_USE_ANY,   0, 0), // SS1 - DMA1_ST2

    DEF_TIM(TIM9, CH1, PA2,  TIM_USE_ANY,   0, 0 ), // TX2
    DEF_TIM(TIM1, CH2, PA9,  TIM_USE_ANY,   0, 0 ), // TX1
    DEF_TIM(TIM1, CH3, PA10, TIM_USE_ANY,   0, 0 ), // RX1
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
