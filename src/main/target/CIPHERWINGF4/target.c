/*
 * Cipherwing F4 target (custom OmnibusF4 variant)
 * Motor and LED timer mapping.
 */

#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

timerHardware_t timerHardware[] = {
    // ===== Motors (8 outputs) =====
    DEF_TIM(TIM3,  CH3, PB0,  TIM_USE_OUTPUT_AUTO, 0, 0), // M1
    DEF_TIM(TIM3,  CH4, PB1,  TIM_USE_OUTPUT_AUTO, 0, 0), // M2
    DEF_TIM(TIM8,  CH3, PC8,  TIM_USE_OUTPUT_AUTO, 0, 0), // M3
    DEF_TIM(TIM8,  CH4, PC9,  TIM_USE_OUTPUT_AUTO, 0, 0), // M4
    DEF_TIM(TIM12, CH1, PB14, TIM_USE_OUTPUT_AUTO, 0, 0), // M5
    DEF_TIM(TIM1,  CH1, PA8,  TIM_USE_OUTPUT_AUTO, 0, 0), // M6
    DEF_TIM(TIM12, CH2, PB15, TIM_USE_OUTPUT_AUTO, 0, 0), // M7
    DEF_TIM(TIM4,  CH2, PB7,  TIM_USE_OUTPUT_AUTO, 0, 0), // M8

    // ===== LED strip =====
    DEF_TIM(TIM4,  CH1, PB6,  TIM_USE_LED,         0, 0),
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);

