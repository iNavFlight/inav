
#include <stdbool.h>
#include <stdint.h>

#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

const timerHardware_t timerHardware[] = {
    DEF_TIM(TIM2, CH2, PB3, TIM_USE_PPM, 0), // PPM IN
    
    DEF_TIM(TIM4,  CH2, PB7, TIM_USE_MC_MOTOR, 0),
    DEF_TIM(TIM4,  CH1, PB6, TIM_USE_MC_MOTOR, 0),
    DEF_TIM(TIM17, CH1, PB5, TIM_USE_MC_MOTOR, 0),
    DEF_TIM(TIM16, CH1, PB4, TIM_USE_MC_MOTOR, 0),

    DEF_TIM(TIM3,  CH3, PB0, TIM_USE_MC_MOTOR, 0),
    DEF_TIM(TIM3,  CH4, PB1, TIM_USE_MC_MOTOR, 0),

    DEF_TIM(TIM1,  CH1, PA8, TIM_USE_ANY,      0),
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
