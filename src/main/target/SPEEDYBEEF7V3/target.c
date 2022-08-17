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

BUSDEV_REGISTER_SPI_TAG(busdev_bmi270,   DEVHW_BMI270,  BMI270_SPI_BUS,   BMI270_CS_PIN,   BMI270_EXTI_PIN,   0,  DEVFLAGS_NONE,  IMU_BMI270_ALIGN);
BUSDEV_REGISTER_SPI_TAG(busdev_mpu6000,  DEVHW_MPU6000, MPU6000_SPI_BUS,  MPU6000_CS_PIN,  MPU6000_EXTI_PIN,  0,  DEVFLAGS_NONE,  IMU_MPU6000_ALIGN);

timerHardware_t timerHardware[] = {

    DEF_TIM(TIM2, CH1, PA15,    TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR, 0, 0),  // S1
    DEF_TIM(TIM2, CH2, PB3,     TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR, 0, 0),  // S2
    DEF_TIM(TIM3, CH1, PB4,     TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),  // S3
    DEF_TIM(TIM4, CH1, PB6,     TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),  // S4
    DEF_TIM(TIM4, CH2, PB7,     TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),  // S5
    DEF_TIM(TIM3, CH2, PB5,     TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),  // S6 Clash with S2, DSHOT does not work
    DEF_TIM(TIM3, CH3, PB0,     TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),  // S7
    DEF_TIM(TIM3, CH4, PB1,     TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO, 0, 0),  // S8

    DEF_TIM(TIM8, CH3, PC8,  TIM_USE_LED, 0, 0),    // LED
    DEF_TIM(TIM5, CH1, PA0,  TIM_USE_ANY, 0, 0), // Camera Control
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);