/*
 * RadioMaster Nexus (Original) - Timer/PWM hardware configuration
 *
 * Timer allocation (from Rotorflight NEXUS_F7 dump):
 *   TIM3: S1 (PB4/CH1), S2 (PB5/CH2), S3 (PB0/CH3)
 *   TIM2: S4 (PB3/CH2)
 *   TIM4: M1 (PB6/CH1) - ESC output
 *
 * Note: PA2/PA3 are FREQ/PPM inputs in Rotorflight. In iNAV they
 * can be repurposed as servo outputs when UART2 is not assigned.
 */

#include <stdint.h>
#include "platform.h"
#include "drivers/io.h"
#include "drivers/timer.h"
#include "drivers/pwm_mapping.h"

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM3, CH1, PB4, TIM_USE_OUTPUT_AUTO, 0, 0),  // S1
    DEF_TIM(TIM3, CH2, PB5, TIM_USE_OUTPUT_AUTO, 0, 0),  // S2
    DEF_TIM(TIM3, CH3, PB0, TIM_USE_OUTPUT_AUTO, 0, 0),  // S3
    DEF_TIM(TIM2, CH2, PB3, TIM_USE_OUTPUT_AUTO, 0, 0),  // S4
    DEF_TIM(TIM4, CH1, PB6, TIM_USE_OUTPUT_AUTO, 0, 0),  // M1/ESC
    DEF_TIM(TIM5, CH3, PA2, TIM_USE_OUTPUT_AUTO, 0, 0),  // shared w/ UART2 TX
    DEF_TIM(TIM9, CH2, PA3, TIM_USE_OUTPUT_AUTO, 0, 0),  // shared w/ UART2 RX
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
