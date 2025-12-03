/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 */

#include <stdbool.h>
#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

// -----------------------------------------------------------------------------
//   TIMER DEFINITIONS
// -----------------------------------------------------------------------------

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM4,   CH2, PB7,  TIM_USE_OUTPUT_AUTO,   1, 0),
    DEF_TIM(TIM4,   CH1, PB6,  TIM_USE_OUTPUT_AUTO,   1, 0),

    DEF_TIM(TIM3,   CH3, PB0,  TIM_USE_OUTPUT_AUTO,   1, 0),
    DEF_TIM(TIM3,   CH4, PB1,  TIM_USE_OUTPUT_AUTO,   1, 0),
    DEF_TIM(TIM8,   CH3, PC8,  TIM_USE_OUTPUT_AUTO,   1, 0),
    DEF_TIM(TIM8,   CH4, PC9,  TIM_USE_OUTPUT_AUTO,   1, 0),
    
    // Outputs (order corrected)
	DEF_TIM(TIM2,   CH3, PB10, TIM_USE_ANY        ,   0, 0), // s7
	DEF_TIM(TIM2,   CH4, PB11, TIM_USE_OUTPUT_AUTO,   1, 0), // s8
	DEF_TIM(TIM2,   CH1, PA15, TIM_USE_OUTPUT_AUTO,   1, 0), // s9
    DEF_TIM(TIM8,  CH2N, PB14, TIM_USE_OUTPUT_AUTO,   1, 0), // s11   
    
    
    DEF_TIM(TIM1,   CH1, PA8,  TIM_USE_LED,           0, 0),
    DEF_TIM(TIM5,   CH3, PA2,  TIM_USE_ANY,           0, 0),
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);

/*	Строка 12:     IO_t io = IOGetByTag(IO_TAG(PB14)); меняем для входа TIM_USE_ANY
	Строка 32:     IO_t io = IOGetByTag(IO_TAG(PB14));
*/


