/*
* This file is part of Cleanflight.
*
* Cleanflight is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Cleanflight is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdbool.h>
#include <platform.h>
#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/pinio.h"

#include "drivers/sensor.h"
#include "drivers/timer_def_stm32f4xx.h"


/**Motors & Servos for FW & MC**/
/**See Datasheet of the WA STM32F405XG**/
/**Base of the Riza**/
/**DEF_TIM(Timer , Channle , Pin , Multirotor Motor | Fixwing Motor/Servo , 0 ,0 ))**/

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM4,  CH3,  PB8,  TIM_USE_OUTPUT_AUTO,   0, 0), // S1 D(2,7,7) UP217 / TIM10, CH1 / TIM4, CH3 /
    DEF_TIM(TIM3,  CH2,  PB5,  TIM_USE_OUTPUT_AUTO,   0, 0), // S2 D(2,2,0) UP217 / TIM3, CH2
    DEF_TIM(TIM3,  CH1,  PB4,  TIM_USE_OUTPUT_AUTO,   0, 0), // S3 D(2,6,0) UP256 /  TIM3, CH1 
    DEF_TIM(TIM2,  CH2,  PB3,  TIM_USE_OUTPUT_AUTO,   0, 0), //0,1) S4 D(2,1,6) UP256 / TIM2, CH2
    DEF_TIM(TIM1,  CH3,  PA10,  TIM_USE_OUTPUT_AUTO,   0, 0), // S5 D(1,7,3) UP173 / TIM1, CH3
    DEF_TIM(TIM1,  CH2,  PA9,  TIM_USE_OUTPUT_AUTO,   0, 0), // S6 D(1,1,3) UP173 / TIM1 ,CH2
    DEF_TIM(TIM12,  CH2,  PB15,  TIM_USE_OUTPUT_AUTO,   0, 0), // S7 D(1,6,3) UP173 / TIM1, CH3N / TIM8, CH3N / TIM12, CH2 /
    DEF_TIM(TIM12,  CH1,  PB14, TIM_USE_OUTPUT_AUTO,   0, 0), // S8 D(1,5,3) UP173 / TIM1, CH2N / TIM8, CH2N / TIM12, CH1 /
    DEF_TIM(TIM14,  CH1,  PA7, TIM_USE_OUTPUT_AUTO,   0, 0), // S9  DMA NONE / TIM8, CH1N /TIM14, CH1/TIM3, CH2/TIM1, CH1N /
    DEF_TIM(TIM13, CH1,  PA6,  TIM_USE_OUTPUT_AUTO,   0, 0), // S10 DMA NONE / TIM13, CH1 / TIM3, CH1

    DEF_TIM(TIM8, CH3N,  PB1,  TIM_USE_LED,    0, 0), // 2812LED  D(1,2,5) / TIM3, CH4 / TIM8, CH3N/ TIM1, CH3N /
    DEF_TIM(TIM11, CH1,  PB9,  TIM_USE_BEEPER, 0, 0), // BEEPER PWM / SPI2_NSS/ TIM4, CH4/ TIM11, CH1 /
    
    //DEF_TIM(TIM9,  CH2,  PA3,  TIM_USE_PPM,    0, 0), //RX2
    //DEF_TIM(TIM5,  CH3,  PA2,  TIM_USE_ANY,    0, 0), //TX2  softserial1_Tx

    //DEF_TIM(TIM1,  CH1,  PA8,  TIM_USE_ANY,    0, 0), //SSRX2
    //DEF_TIM(TIM1,  CH4,  PA11,  TIM_USE_ANY,    0, 0), //SSTX2  softserial1_Tx

   // DEF_TIM(TIM8,  CH2,  PC7,  TIM_USE_ANY,    0, 0), //SSRX1
    //DEF_TIM(TIM8,  CH1,  PC6,  TIM_USE_ANY,    0, 0), //SSTX1  softserial2_Tx

};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
