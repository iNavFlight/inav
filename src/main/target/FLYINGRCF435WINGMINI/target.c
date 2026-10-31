/*
 * FlyingRC F435 Wing Mini timer map.
 */

#include <stdint.h>

#include "platform.h"

#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

timerHardware_t timerHardware[] = {
    DEF_TIM(TMR5, CH1, PB12, TIM_USE_OUTPUT_AUTO, 0, 8),  // OUT1, DMA2 CH2
    DEF_TIM(TMR5, CH2, PH3,  TIM_USE_OUTPUT_AUTO, 0, 9),  // OUT2, DMA2 CH3
    DEF_TIM(TMR2, CH1, PA15, TIM_USE_OUTPUT_AUTO, 0, 0),  // OUT3, DMA1 CH1
    DEF_TIM(TMR2, CH4, PB2,  TIM_USE_OUTPUT_AUTO, 0, 6),  // OUT4 / BOOT1, DMA1 CH7
    DEF_TIM(TMR4, CH1, PB6,  TIM_USE_OUTPUT_AUTO, 0, 11), // OUT5, DMA2 CH5
    DEF_TIM(TMR4, CH2, PB7,  TIM_USE_OUTPUT_AUTO, 0, 10), // OUT6, DMA2 CH4
    DEF_TIM(TMR1, CH1, PA8,  TIM_USE_LED,         0, 5),  // WS2812, DMA1 CH6
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
