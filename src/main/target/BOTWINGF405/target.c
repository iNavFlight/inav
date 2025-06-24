#include <stdint.h>

#include "platform.h"

#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/pinio.h"

 
 
timerHardware_t timerHardware[] = {
    DEF_TIM(TIM1, CH2, PA9,  TIM_USE_OUTPUT_AUTO, 1, 1), // S1
    DEF_TIM(TIM1, CH1, PA8,  TIM_USE_OUTPUT_AUTO, 1, 1), // S2
    DEF_TIM(TIM3, CH4, PC9,  TIM_USE_OUTPUT_AUTO, 1, 0), // S3
    DEF_TIM(TIM3, CH3, PC8,  TIM_USE_OUTPUT_AUTO, 1, 0), // S4
    DEF_TIM(TIM2, CH2, PB3,  TIM_USE_LED, 0, 0),

};

 
const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
 