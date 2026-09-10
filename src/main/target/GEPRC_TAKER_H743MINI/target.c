/*
* This file is part of INAV Project.
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

#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/sensor.h"

// ICM42688P: DEVHW_ICM42605 driver detects both via WHO_AM_I
BUSDEV_REGISTER_SPI_TAG(busdev_icm42688, DEVHW_ICM42605, ICM42605_SPI_BUS, ICM42605_CS_PIN, ICM42605_EXTI_PIN, 0, DEVFLAGS_NONE, IMU_ICM42605_ALIGN);


timerHardware_t timerHardware[] = {

    DEF_TIM(TIM3,  CH1,  PC6,   TIM_USE_MOTOR, 0, 0),
    DEF_TIM(TIM3,  CH2,  PC7,   TIM_USE_MOTOR, 0, 1),
    DEF_TIM(TIM3,  CH3,  PC8,   TIM_USE_MOTOR, 0, 2),
    DEF_TIM(TIM3,  CH4,  PC9,   TIM_USE_MOTOR, 0, 3),

    DEF_TIM(TIM1,  CH1,  PE9,   TIM_USE_MOTOR, 0, 4),
    DEF_TIM(TIM1,  CH2,  PE11,  TIM_USE_MOTOR, 0, 5),
    DEF_TIM(TIM1,  CH3,  PE13,  TIM_USE_MOTOR, 0, 6),
    DEF_TIM(TIM1,  CH4,  PE14,  TIM_USE_MOTOR, 0, 7),

    // LED STRIP
    DEF_TIM(TIM4,  CH1,  PD12,  TIM_USE_LED, 1, 8),
};




const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
