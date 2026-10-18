#include <stdbool.h>
#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM3, CH4, PC9, TIM_USE_MOTOR, 0, 0),  // S1
    DEF_TIM(TIM3, CH3, PC8, TIM_USE_MOTOR, 0, 0),  // S2
    DEF_TIM(TIM3, CH2, PC7, TIM_USE_MOTOR, 0, 0),  // S3
    DEF_TIM(TIM3, CH1, PC6, TIM_USE_MOTOR, 0, 0),  // S4

    // Дополнительно — сервы подвеса (если нужно)
    DEF_TIM(TIM1, CH1, PA8,  TIM_USE_SERVO, 0, 0), // GIMBAL_PITCH
    DEF_TIM(TIM2, CH4, PB2,  TIM_USE_SERVO, 0, 0), // GIMBAL_ROLL
    DEF_TIM(TIM2, CH3, PB10, TIM_USE_SERVO, 0, 0), // GIMBAL_YAW
    DEF_TIM(TIM2, CH1, PA15, TIM_USE_SERVO, 0, 0), // GIMBAL_ZOOM
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);