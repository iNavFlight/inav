/*
 * RadioMaster Nexus (Original) - Target configuration defaults
 *
 * Unlike the Nexus-X/XR, the OG Nexus has no internal ELRS receiver,
 * so there is no PINIO / USER1 box configuration needed here.
 */

#include <stdint.h>
#include "platform.h"
#include "drivers/pwm_mapping.h"

void targetConfiguration(void)
{
    // Force M1/ESC (PB6/TIM4) to motor mode so the PWM auto-allocator
    // picks it as the motor output first. Without this, it walks the
    // timer table in order, converts TIM3 (S1/S2/S3) to motors via
    // pwmClaimTimer(), and leaves no timers available for servos.
    timerOverridesMutable(timer2id(TIM4))->outputMode = OUTPUT_MODE_MOTORS;
}
