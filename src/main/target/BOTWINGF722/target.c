#include <stdint.h>

#include "platform.h"

#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/pinio.h"


timerHardware_t timerHardware[] = {
    DEF_TIM(TIM3, CH1, PB4,  TIM_USE_OUTPUT_AUTO, 0, 0), // S1
    DEF_TIM(TIM3, CH2, PB5,  TIM_USE_OUTPUT_AUTO, 0, 0), // S2
    DEF_TIM(TIM3, CH3, PB0,  TIM_USE_OUTPUT_AUTO, 0, 0), // S3
    DEF_TIM(TIM3, CH4, PB1,  TIM_USE_OUTPUT_AUTO, 0, 0), // S4
    DEF_TIM(TIM2, CH1, PA15, TIM_USE_OUTPUT_AUTO, 0, 0),// Servo 1
    DEF_TIM(TIM2, CH2, PB3,  TIM_USE_OUTPUT_AUTO, 0, 0),// Servo 2
    DEF_TIM(TIM4, CH1, PB6,  TIM_USE_LED, 0, 0),    
};
const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
